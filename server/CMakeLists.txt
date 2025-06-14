cmake_minimum_required(VERSION 3.16)
project(FantasyLegend_Server VERSION 1.0.0 LANGUAGES CXX)

# 查找Python
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

# 查找pybind11
find_package(pybind11 REQUIRED)

# 设置源文件
set(SOURCES
    src/main.cpp
    src/core/GameEngine.cpp
    src/core/characters/Character.cpp
    src/core/combat/Combat.cpp
    src/core/levels/Level.cpp
    src/data/database/DatabaseManager.cpp
    src/data/config/ConfigManager.cpp
    src/utils/Logger.cpp
    src/utils/FileUtils.cpp
)

set(HEADERS
    include/core/GameEngine.h
    include/core/characters/Character.h
    include/core/combat/Combat.h
    include/core/levels/Level.h
    include/data/database/DatabaseManager.h
    include/data/config/ConfigManager.h
    include/utils/Logger.h
    include/utils/FileUtils.h
)

# 创建可执行文件
add_executable(FantasyLegend_Server ${SOURCES} ${HEADERS})

# 包含目录
target_include_directories(FantasyLegend_Server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/common/include
    ${Python3_INCLUDE_DIRS}
    ${pybind11_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(FantasyLegend_Server
    FantasyLegend_Common
    pybind11::embed
    ${Python3_LIBRARIES}
)

# 设置输出目录
set_target_properties(FantasyLegend_Server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 复制配置文件和脚本
file(COPY config DESTINATION ${CMAKE_BINARY_DIR}/server)
file(COPY scripts DESTINATION ${CMAKE_BINARY_DIR}/server) 