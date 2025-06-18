cmake_minimum_required(VERSION 3.16)
project(FantasyLegend_Server VERSION 1.0.0 LANGUAGES CXX)

# 查找Python
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

# 设置第三方库目录
set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party")

# 添加 stb 库
set(STB_DIR "${THIRD_PARTY_DIR}/stb")
if(NOT EXISTS "${STB_DIR}")
    message(FATAL_ERROR "STB library not found. Please run scripts/setup_third_party.sh first.")
endif()
include_directories(${STB_DIR})

# 添加 spdlog 库
set(SPDLOG_DIR "${THIRD_PARTY_DIR}/spdlog")
if(NOT EXISTS "${SPDLOG_DIR}")
    message(FATAL_ERROR "spdlog library not found. Please run scripts/setup_third_party.sh first.")
endif()
add_subdirectory(${SPDLOG_DIR} ${CMAKE_BINARY_DIR}/spdlog)

# 添加 nlohmann_json 库
set(JSON_DIR "${THIRD_PARTY_DIR}/json")
if(NOT EXISTS "${JSON_DIR}")
    message(FATAL_ERROR "nlohmann_json library not found. Please run scripts/setup_third_party.sh first.")
endif()
add_subdirectory(${JSON_DIR} ${CMAKE_BINARY_DIR}/json)

# 添加 pybind11 库
set(PYBIND11_DIR "${THIRD_PARTY_DIR}/pybind11")
if(NOT EXISTS "${PYBIND11_DIR}")
    message(FATAL_ERROR "pybind11 library not found. Please run scripts/setup_third_party.sh first.")
endif()
add_subdirectory(${PYBIND11_DIR} ${CMAKE_BINARY_DIR}/pybind11)

# 设置编译选项
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 定义资源管理系统相关的编译选项
add_compile_definitions(
    FANTASY_RESOURCE_SYSTEM
    FANTASY_ENABLE_COMPRESSION
    FANTASY_ENABLE_PACKAGING
    FANTASY_ENABLE_VERSION_CONTROL
    FANTASY_ENABLE_PERFORMANCE_MONITORING
)

# 添加源文件
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/*.h"
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接库
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    pybind11::module
    pthread
    dl
)

# 设置包含目录
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${THIRD_PARTY_DIR}
)

# 包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/common/include
    ${Python3_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(${PROJECT_NAME}
    FantasyLegend_Common
    ${Python3_LIBRARIES}
)

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 复制配置文件和脚本
file(COPY config DESTINATION ${CMAKE_BINARY_DIR}/server)
file(COPY scripts DESTINATION ${CMAKE_BINARY_DIR}/server)

# 创建资源目录
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/server/resources)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/server/resources/textures)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/server/resources/models)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/server/resources/audio)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/server/resources/fonts)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/server/resources/scripts)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/server/resources/maps)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/server/resources/configs) 