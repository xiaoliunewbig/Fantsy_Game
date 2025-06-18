#!/bin/bash

# Fantasy Legend Server 构建脚本
# 作者: [pengchengkang]
# 日期: 2025.06.17

set -e  # 遇到错误时退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 打印带颜色的消息
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 显示帮助信息
show_help() {
    echo "Fantasy Legend Server 构建脚本"
    echo ""
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  -h, --help              显示此帮助信息"
    echo "  -c, --clean             清理构建目录"
    echo "  -d, --debug             构建调试版本"
    echo "  -r, --release           构建发布版本"
    echo "  -t, --test              构建并运行测试"
    echo "  -e, --examples          构建示例程序"
    echo "  -j, --jobs N            使用N个并行任务 (默认: 4)"
    echo "  -v, --verbose           详细输出"
    echo "  --install               安装到系统"
    echo "  --format                格式化代码"
    echo "  --analyze               静态代码分析"
    echo ""
    echo "示例:"
    echo "  $0 -d -j 8              # 构建调试版本，使用8个并行任务"
    echo "  $0 -r -t                # 构建发布版本并运行测试"
    echo "  $0 --clean              # 清理构建目录"
}

# 检查依赖
check_dependencies() {
    print_info "检查构建依赖..."
    
    # 检查CMake
    if ! command -v cmake &> /dev/null; then
        print_error "CMake 未找到，请安装 CMake 3.16 或更高版本"
        exit 1
    fi
    
    # 检查编译器
    if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
        print_error "未找到C++编译器，请安装 g++ 或 clang++"
        exit 1
    fi
    
    # 检查SQLite3
    if ! pkg-config --exists sqlite3 2>/dev/null && ! ldconfig -p | grep -q libsqlite3; then
        print_warning "SQLite3 未找到，某些功能可能不可用"
    fi
    
    # 检查JSON库
    if ! pkg-config --exists jsoncpp 2>/dev/null && ! ldconfig -p | grep -q libjsoncpp; then
        print_warning "JSONCPP 未找到，某些功能可能不可用"
    fi
    
    print_success "依赖检查完成"
}

# 清理构建目录
clean_build() {
    print_info "清理构建目录..."
    if [ -d "build" ]; then
        rm -rf build
        print_success "构建目录已清理"
    else
        print_info "构建目录不存在，无需清理"
    fi
}

# 创建构建目录
create_build_dir() {
    print_info "创建构建目录..."
    mkdir -p build
    cd build
}

# 配置CMake
configure_cmake() {
    local build_type=$1
    local verbose=$2
    
    print_info "配置CMake (构建类型: $build_type)..."
    
    local cmake_args="-DCMAKE_BUILD_TYPE=$build_type"
    
    if [ "$verbose" = "true" ]; then
        cmake_args="$cmake_args -DCMAKE_VERBOSE_MAKEFILE=ON"
    fi
    
    cmake .. $cmake_args
    
    if [ $? -eq 0 ]; then
        print_success "CMake配置完成"
    else
        print_error "CMake配置失败"
        exit 1
    fi
}

# 编译项目
compile_project() {
    local jobs=$1
    local verbose=$2
    
    print_info "编译项目 (并行任务: $jobs)..."
    
    local make_args="-j$jobs"
    if [ "$verbose" = "true" ]; then
        make_args="$make_args VERBOSE=1"
    fi
    
    make $make_args
    
    if [ $? -eq 0 ]; then
        print_success "编译完成"
    else
        print_error "编译失败"
        exit 1
    fi
}

# 运行测试
run_tests() {
    print_info "运行测试..."
    
    if [ -f "FantasyLegendTests" ]; then
        ./FantasyLegendTests
        if [ $? -eq 0 ]; then
            print_success "测试通过"
        else
            print_error "测试失败"
            exit 1
        fi
    else
        print_warning "测试可执行文件未找到"
    fi
}

# 运行示例
run_examples() {
    print_info "运行示例..."
    
    if [ -f "FantasyLegendExamples" ]; then
        ./FantasyLegendExamples
        if [ $? -eq 0 ]; then
            print_success "示例运行成功"
        else
            print_error "示例运行失败"
            exit 1
        fi
    else
        print_warning "示例可执行文件未找到"
    fi
}

# 安装
install_project() {
    print_info "安装项目..."
    
    sudo make install
    
    if [ $? -eq 0 ]; then
        print_success "安装完成"
    else
        print_error "安装失败"
        exit 1
    fi
}

# 格式化代码
format_code() {
    print_info "格式化代码..."
    
    if command -v clang-format &> /dev/null; then
        find ../src ../include -name "*.cpp" -o -name "*.h" | xargs clang-format -i
        print_success "代码格式化完成"
    else
        print_warning "clang-format 未找到，跳过代码格式化"
    fi
}

# 静态分析
static_analysis() {
    print_info "运行静态代码分析..."
    
    if command -v cppcheck &> /dev/null; then
        cppcheck --enable=all --std=c++17 --language=c++ --suppress=missingIncludeSystem ../src
        print_success "静态分析完成"
    else
        print_warning "cppcheck 未找到，跳过静态分析"
    fi
}

# 主函数
main() {
    local clean=false
    local debug=false
    local release=false
    local test=false
    local examples=false
    local jobs=4
    local verbose=false
    local install=false
    local format=false
    local analyze=false
    
    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -c|--clean)
                clean=true
                shift
                ;;
            -d|--debug)
                debug=true
                shift
                ;;
            -r|--release)
                release=true
                shift
                ;;
            -t|--test)
                test=true
                shift
                ;;
            -e|--examples)
                examples=true
                shift
                ;;
            -j|--jobs)
                jobs="$2"
                shift 2
                ;;
            -v|--verbose)
                verbose=true
                shift
                ;;
            --install)
                install=true
                shift
                ;;
            --format)
                format=true
                shift
                ;;
            --analyze)
                analyze=true
                shift
                ;;
            *)
                print_error "未知选项: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    # 如果没有指定构建类型，默认使用Release
    if [ "$debug" = "false" ] && [ "$release" = "false" ]; then
        release=true
    fi
    
    # 确定构建类型
    if [ "$debug" = "true" ] && [ "$release" = "true" ]; then
        print_error "不能同时指定调试和发布模式"
        exit 1
    fi
    
    local build_type="Release"
    if [ "$debug" = "true" ]; then
        build_type="Debug"
    fi
    
    print_info "开始构建 Fantasy Legend Server..."
    print_info "构建类型: $build_type"
    print_info "并行任务: $jobs"
    
    # 检查依赖
    check_dependencies
    
    # 清理构建目录
    if [ "$clean" = "true" ]; then
        clean_build
        if [ "$format" = "false" ] && [ "$analyze" = "false" ]; then
            exit 0
        fi
    fi
    
    # 格式化代码
    if [ "$format" = "true" ]; then
        format_code
        if [ "$analyze" = "false" ] && [ "$debug" = "false" ] && [ "$release" = "false" ]; then
            exit 0
        fi
    fi
    
    # 静态分析
    if [ "$analyze" = "true" ]; then
        static_analysis
        if [ "$debug" = "false" ] && [ "$release" = "false" ]; then
            exit 0
        fi
    fi
    
    # 创建构建目录
    create_build_dir
    
    # 配置CMake
    configure_cmake "$build_type" "$verbose"
    
    # 编译项目
    compile_project "$jobs" "$verbose"
    
    # 运行测试
    if [ "$test" = "true" ]; then
        run_tests
    fi
    
    # 运行示例
    if [ "$examples" = "true" ]; then
        run_examples
    fi
    
    # 安装
    if [ "$install" = "true" ]; then
        install_project
    fi
    
    print_success "构建完成！"
    print_info "可执行文件位置:"
    print_info "  主服务器: build/FantasyLegendServer"
    if [ -f "FantasyLegendTests" ]; then
        print_info "  测试程序: build/FantasyLegendTests"
    fi
    if [ -f "FantasyLegendExamples" ]; then
        print_info "  示例程序: build/FantasyLegendExamples"
    fi
}

# 运行主函数
main "$@" 