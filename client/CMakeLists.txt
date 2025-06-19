cmake_minimum_required(VERSION 3.16)
project(FantasyLegend_Client VERSION 1.0.0 LANGUAGES CXX)

# 设置Qt6查找路径（根据你本地环境修改）
set(CMAKE_PREFIX_PATH "/home/pck/tools/qtlib")

# 查找Qt6组件
find_package(Qt6 REQUIRED COMPONENTS
    Core Gui Widgets
    Quick QuickControls2
    Network
)

# 自动扫描所有源文件和头文件
file(GLOB_RECURSE SOURCE_FILES
    "src/*.cpp"
    "src/*/*.cpp"
    "src/*/*/*.cpp"
)

file(GLOB_RECURSE HEADER_FILES
    "include/*.h"
    "include/*/*.h"
    "include/*/*/*.h"
)

# 创建可执行文件目标
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

# 设置资源文件默认前缀（用于 .qrc）
set_target_properties(${PROJECT_NAME} PROPERTIES
    QT_RESOURCE_PREFIX "/assets"
)

# 添加 Qt 资源支持（如果存在 resources.qrc）
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc")
    qt_add_resources(${PROJECT_NAME} resources.qrc)
endif()

# 包含目录设置
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/common/include
)

# 链接 Qt 库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
)

# 启用 Qt 特性：MOC, UIC, RCC
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# 输出目录设置
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 复制 assets 到构建目录（方便运行时加载 QML 等资源）
file(COPY assets/
     DESTINATION ${CMAKE_BINARY_DIR}/client/assets
)

# 编译选项优化
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)