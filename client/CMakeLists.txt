cmake_minimum_required(VERSION 3.16)
project(FantasyLegend_Client VERSION 1.0.0 LANGUAGES CXX)

# 设置Qt6路径
set(CMAKE_PREFIX_PATH "/home/pck/tools/qtlib")

# 查找Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Quick QuickControls2)

# 设置源文件
set(SOURCES
    src/main.cpp
    src/ui/windows/MainWindow.cpp
    src/ui/components/MainMenu.cpp
    src/ui/components/PauseMenu.cpp
    src/ui/scenes/GameScene.cpp
    src/ui/managers/UIManager.cpp
    src/controllers/GameController.cpp
    src/network/NetworkManager.cpp
    src/utils/Logger.cpp
    src/utils/FileUtils.cpp
)

set(HEADERS
    include/ui/windows/MainWindow.h
    include/ui/components/MainMenu.h
    include/ui/components/PauseMenu.h
    include/ui/scenes/GameScene.h
    include/ui/managers/UIManager.h
    include/controllers/GameController.h
    include/network/NetworkManager.h
    include/utils/Logger.h
    include/utils/FileUtils.h
)

# 创建可执行文件
add_executable(FantasyLegend_Client ${SOURCES} ${HEADERS})

# 包含目录
target_include_directories(FantasyLegend_Client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/common/include
)

# 链接Qt库
target_link_libraries(FantasyLegend_Client
    Qt6::Core
    Qt6::Widgets
    Qt6::Quick
    Qt6::QuickControls2
)

# 设置包含目录
target_include_directories(FantasyLegend_Client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 复制资源文件
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/client)

# 启用Qt MOC
set_target_properties(FantasyLegend_Client PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# 设置输出目录
set_target_properties(FantasyLegend_Client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 设置C++标准
set_target_properties(FantasyLegend_Client PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# 编译选项
target_compile_options(FantasyLegend_Client PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
) 