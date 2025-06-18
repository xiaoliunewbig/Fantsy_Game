#!/bin/bash

# Fantasy Legend Server 启动脚本
# 作者: [pengchengkang]
# 日期: 2025.06.17

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

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

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SERVER_DIR="$(dirname "$SCRIPT_DIR")"

# 默认配置
CONFIG_FILE="$SERVER_DIR/config/server.conf"
LOG_DIR="$SERVER_DIR/logs"
DATA_DIR="$SERVER_DIR/data"
PID_FILE="$SERVER_DIR/fantasy_legend.pid"

# 显示帮助信息
show_help() {
    echo "Fantasy Legend Server 启动脚本"
    echo ""
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  -h, --help              显示此帮助信息"
    echo "  -c, --config FILE       指定配置文件 (默认: config/server.conf)"
    echo "  -d, --daemon            以守护进程模式运行"
    echo "  -f, --foreground        前台运行 (默认)"
    echo "  -l, --log-level LEVEL   设置日志级别 (debug, info, warn, error)"
    echo "  -p, --port PORT         设置端口号"
    echo "  --stop                  停止服务器"
    echo "  --restart               重启服务器"
    echo "  --status                查看服务器状态"
    echo ""
    echo "示例:"
    echo "  $0                      # 前台启动服务器"
    echo "  $0 -d                   # 守护进程模式启动"
    echo "  $0 --stop               # 停止服务器"
    echo "  $0 --restart            # 重启服务器"
}

# 检查服务器是否正在运行
is_server_running() {
    if [ -f "$PID_FILE" ]; then
        local pid=$(cat "$PID_FILE")
        if kill -0 "$pid" 2>/dev/null; then
            return 0
        else
            rm -f "$PID_FILE"
        fi
    fi
    return 1
}

# 启动服务器
start_server() {
    local daemon_mode=$1
    
    print_info "启动 Fantasy Legend Server..."
    
    # 检查服务器是否已经运行
    if is_server_running; then
        print_warning "服务器已经在运行中 (PID: $(cat "$PID_FILE"))"
        return 1
    fi
    
    # 创建必要的目录
    mkdir -p "$LOG_DIR" "$DATA_DIR"
    
    # 检查可执行文件
    local executable="$SERVER_DIR/build/FantasyLegendServer"
    if [ ! -f "$executable" ]; then
        print_error "服务器可执行文件未找到: $executable"
        print_info "请先运行 ./build.sh -r 构建项目"
        return 1
    fi
    
    # 设置环境变量
    export FANTASY_LEGEND_CONFIG="$CONFIG_FILE"
    export FANTASY_LEGEND_LOG_DIR="$LOG_DIR"
    export FANTASY_LEGEND_DATA_DIR="$DATA_DIR"
    
    # 启动服务器
    if [ "$daemon_mode" = "true" ]; then
        print_info "以守护进程模式启动..."
        nohup "$executable" > "$LOG_DIR/server.log" 2>&1 &
        echo $! > "$PID_FILE"
        sleep 2
        
        if is_server_running; then
            print_success "服务器已启动 (PID: $(cat "$PID_FILE"))"
            print_info "日志文件: $LOG_DIR/server.log"
        else
            print_error "服务器启动失败"
            return 1
        fi
    else
        print_info "前台启动服务器..."
        print_info "按 Ctrl+C 停止服务器"
        "$executable"
    fi
}

# 停止服务器
stop_server() {
    print_info "停止 Fantasy Legend Server..."
    
    if [ -f "$PID_FILE" ]; then
        local pid=$(cat "$PID_FILE")
        if kill -0 "$pid" 2>/dev/null; then
            print_info "发送停止信号到进程 $pid..."
            kill "$pid"
            
            # 等待进程结束
            local count=0
            while kill -0 "$pid" 2>/dev/null && [ $count -lt 30 ]; do
                sleep 1
                count=$((count + 1))
            done
            
            if kill -0 "$pid" 2>/dev/null; then
                print_warning "进程未响应，强制终止..."
                kill -9 "$pid"
            fi
            
            rm -f "$PID_FILE"
            print_success "服务器已停止"
        else
            print_warning "服务器未运行"
            rm -f "$PID_FILE"
        fi
    else
        print_warning "PID文件不存在，服务器可能未运行"
    fi
}

# 重启服务器
restart_server() {
    print_info "重启 Fantasy Legend Server..."
    stop_server
    sleep 2
    start_server "false"
}

# 查看服务器状态
show_status() {
    if is_server_running; then
        local pid=$(cat "$PID_FILE")
        print_success "服务器正在运行 (PID: $pid)"
        
        # 显示进程信息
        if command -v ps &> /dev/null; then
            echo ""
            print_info "进程信息:"
            ps -p "$pid" -o pid,ppid,cmd,etime,pcpu,pmem
        fi
        
        # 显示端口信息
        if command -v netstat &> /dev/null; then
            echo ""
            print_info "端口信息:"
            netstat -tlnp 2>/dev/null | grep "$pid" || echo "未找到端口信息"
        fi
    else
        print_warning "服务器未运行"
    fi
}

# 主函数
main() {
    local daemon_mode=false
    local action="start"
    
    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -c|--config)
                CONFIG_FILE="$2"
                shift 2
                ;;
            -d|--daemon)
                daemon_mode=true
                shift
                ;;
            -f|--foreground)
                daemon_mode=false
                shift
                ;;
            -l|--log-level)
                export FANTASY_LEGEND_LOG_LEVEL="$2"
                shift 2
                ;;
            -p|--port)
                export FANTASY_LEGEND_PORT="$2"
                shift 2
                ;;
            --stop)
                action="stop"
                shift
                ;;
            --restart)
                action="restart"
                shift
                ;;
            --status)
                action="status"
                shift
                ;;
            *)
                print_error "未知选项: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    # 切换到服务器目录
    cd "$SERVER_DIR"
    
    # 执行相应操作
    case $action in
        start)
            start_server "$daemon_mode"
            ;;
        stop)
            stop_server
            ;;
        restart)
            restart_server
            ;;
        status)
            show_status
            ;;
    esac
}

# 运行主函数
main "$@" 