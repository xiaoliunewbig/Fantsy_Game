cmake_minimum_required(VERSION 3.16)
project(FantasyLegendServer VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# 查找所有源文件
file(GLOB_RECURSE SERVER_SOURCES 
    "src/*.cpp"
    "src/*/*.cpp"
    "src/*/*/*.cpp"
    "src/*/*/*/*.cpp"
)

# 查找所有头文件
file(GLOB_RECURSE SERVER_HEADERS 
    "include/*.h"
    "include/*/*.h"
    "include/*/*/*.h"
    "include/*/*/*/*.h"
)

# 查找测试文件
# file(GLOB_RECURSE TEST_SOURCES 
#     "test/*.cpp"
#     "test/*/*.cpp"
# )

# 查找示例文件
# file(GLOB_RECURSE EXAMPLE_SOURCES 
#     "examples/*.cpp"
#     "examples/*/*.cpp"
# )

# 创建主服务器可执行文件
add_executable(FantasyLegendServer ${SERVER_SOURCES} ${SERVER_HEADERS})

# 设置包含目录
target_include_directories(FantasyLegendServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# 设置编译定义
target_compile_definitions(FantasyLegendServer PRIVATE
    FANTASY_SERVER_BUILD
    FANTASY_VERSION="${PROJECT_VERSION}"
)

# 查找并链接必要的库
find_package(Threads REQUIRED)
target_link_libraries(FantasyLegendServer PRIVATE Threads::Threads)

# 查找SQLite3
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(SQLITE3 QUIET sqlite3)
    if(SQLITE3_FOUND)
        target_link_libraries(FantasyLegendServer PRIVATE ${SQLITE3_LIBRARIES})
        target_include_directories(FantasyLegendServer PRIVATE ${SQLITE3_INCLUDE_DIRS})
        target_compile_options(FantasyLegendServer PRIVATE ${SQLITE3_CFLAGS_OTHER})
    endif()
endif()

# 如果没有找到SQLite3，尝试使用系统默认
if(NOT SQLITE3_FOUND)
    find_library(SQLITE3_LIB sqlite3)
    if(SQLITE3_LIB)
        target_link_libraries(FantasyLegendServer PRIVATE ${SQLITE3_LIB})
    endif()
endif()

# 查找JSON库
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(JSONCPP QUIET jsoncpp)
    if(JSONCPP_FOUND)
        target_link_libraries(FantasyLegendServer PRIVATE ${JSONCPP_LIBRARIES})
        target_include_directories(FantasyLegendServer PRIVATE ${JSONCPP_INCLUDE_DIRS})
        target_compile_options(FantasyLegendServer PRIVATE ${JSONCPP_CFLAGS_OTHER})
    endif()
endif()

# 如果没有找到jsoncpp，尝试使用系统默认
if(NOT JSONCPP_FOUND)
    find_library(JSONCPP_LIB jsoncpp)
    if(JSONCPP_LIB)
        target_link_libraries(FantasyLegendServer PRIVATE ${JSONCPP_LIB})
    endif()
endif()

# 创建测试可执行文件
if(TEST_SOURCES)
    add_executable(FantasyLegendTests ${TEST_SOURCES} ${SERVER_SOURCES} ${SERVER_HEADERS})
    target_include_directories(FantasyLegendTests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    )
    target_compile_definitions(FantasyLegendTests PRIVATE
        FANTASY_SERVER_BUILD
        FANTASY_VERSION="${PROJECT_VERSION}"
    )
    target_link_libraries(FantasyLegendTests PRIVATE Threads::Threads)
    if(SQLITE3_FOUND)
        target_link_libraries(FantasyLegendTests PRIVATE ${SQLITE3_LIBRARIES})
        target_include_directories(FantasyLegendTests PRIVATE ${SQLITE3_INCLUDE_DIRS})
        target_compile_options(FantasyLegendTests PRIVATE ${SQLITE3_CFLAGS_OTHER})
    elseif(SQLITE3_LIB)
        target_link_libraries(FantasyLegendTests PRIVATE ${SQLITE3_LIB})
    endif()
    if(JSONCPP_FOUND)
        target_link_libraries(FantasyLegendTests PRIVATE ${JSONCPP_LIBRARIES})
        target_include_directories(FantasyLegendTests PRIVATE ${JSONCPP_INCLUDE_DIRS})
        target_compile_options(FantasyLegendTests PRIVATE ${JSONCPP_CFLAGS_OTHER})
    elseif(JSONCPP_LIB)
        target_link_libraries(FantasyLegendTests PRIVATE ${JSONCPP_LIB})
    endif()
endif()

# 创建示例可执行文件
if(EXAMPLE_SOURCES)
    add_executable(FantasyLegendExamples ${EXAMPLE_SOURCES} ${SERVER_SOURCES} ${SERVER_HEADERS})
    target_include_directories(FantasyLegendExamples PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    )
    target_compile_definitions(FantasyLegendExamples PRIVATE
        FANTASY_SERVER_BUILD
        FANTASY_VERSION="${PROJECT_VERSION}"
    )
    target_link_libraries(FantasyLegendExamples PRIVATE Threads::Threads)
    if(SQLITE3_FOUND)
        target_link_libraries(FantasyLegendExamples PRIVATE ${SQLITE3_LIBRARIES})
        target_include_directories(FantasyLegendExamples PRIVATE ${SQLITE3_INCLUDE_DIRS})
        target_compile_options(FantasyLegendExamples PRIVATE ${SQLITE3_CFLAGS_OTHER})
    elseif(SQLITE3_LIB)
        target_link_libraries(FantasyLegendExamples PRIVATE ${SQLITE3_LIB})
    endif()
    if(JSONCPP_FOUND)
        target_link_libraries(FantasyLegendExamples PRIVATE ${JSONCPP_LIBRARIES})
        target_include_directories(FantasyLegendExamples PRIVATE ${JSONCPP_INCLUDE_DIRS})
        target_compile_options(FantasyLegendExamples PRIVATE ${JSONCPP_CFLAGS_OTHER})
    elseif(JSONCPP_LIB)
        target_link_libraries(FantasyLegendExamples PRIVATE ${JSONCPP_LIB})
    endif()
endif()

# 安装规则
install(TARGETS FantasyLegendServer
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# 安装头文件
install(DIRECTORY include/
    DESTINATION include/fantasy
    FILES_MATCHING PATTERN "*.h"
)

# 安装配置文件
install(DIRECTORY config/
    DESTINATION share/fantasy/config
    FILES_MATCHING PATTERN "*.json" PATTERN "*.xml" PATTERN "*.ini"
)

# 安装文档
install(DIRECTORY docs/
    DESTINATION share/fantasy/docs
    FILES_MATCHING PATTERN "*.md" PATTERN "*.txt"
)

# 创建构建信息
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config/build_info.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/build_info.h"
)

# 打印构建信息
message(STATUS "Fantasy Legend Server Build Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Source Files: ${SERVER_SOURCES}")
message(STATUS "  Test Files: ${TEST_SOURCES}")
message(STATUS "  Example Files: ${EXAMPLE_SOURCES}")
message(STATUS "  SQLite3: ${SQLITE3_FOUND}")
message(STATUS "  JSONCPP: ${JSONCPP_FOUND}")

# 添加自定义目标
add_custom_target(clean-all
    COMMAND ${CMAKE_BUILD_TOOL} clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeCache.txt
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Cleaning all build files"
)

# 添加格式化目标
find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -i ${SERVER_SOURCES} ${SERVER_HEADERS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Formatting source code with clang-format"
    )
endif()

# 添加静态分析目标
find_program(CPPCHECK "cppcheck")
if(CPPCHECK)
    add_custom_target(static-analysis
        COMMAND ${CPPCHECK} --enable=all --std=c++17 --language=c++ 
                --suppress=missingIncludeSystem ${SERVER_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running static analysis with cppcheck"
    )
endif() 