# 测试目录的CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# 设置测试可执行文件
add_executable(test_data_persistence test_data_persistence.cpp)

# 链接必要的库
target_link_libraries(test_data_persistence
    PRIVATE
    FantasyCore
    FantasyDatabase
    FantasyData
    ${CMAKE_THREAD_LIBS_INIT}
)

# 包含目录
target_include_directories(test_data_persistence
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# 编译选项
target_compile_options(test_data_persistence
    PRIVATE
    -std=c++17
    -Wall
    -Wextra
    -O2
)

# 添加测试
add_test(NAME DataPersistenceTest COMMAND test_data_persistence)

# 设置测试属性
set_tests_properties(DataPersistenceTest PROPERTIES
    TIMEOUT 300
    ENVIRONMENT "FANTASY_TEST_MODE=1"
) 