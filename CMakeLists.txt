cmake_minimum_required(VERSION 3.15)

project(code LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 输出 compile_commands.json（给 VSCode 提供智能补全）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#可执行文件
file(GLOB_RECURSE SRC_FILES
    src/*.cpp
    main.cpp
)
add_executable(myapp ${SRC_FILES})

# 头文件目录
target_include_directories(myapp PRIVATE include)


# 多线程支持
find_package(Threads REQUIRED)
target_link_libraries(myapp PRIVATE Threads::Threads)

# 更严格的编译器警告
target_compile_options(myapp PRIVATE
    -Wall -Wextra -Wpedantic
)

# 测试性能可执行文件
file(GLOB_RECURSE PERF_SRC_FILES
    src/*.cpp
    test/test_performance.cpp
)
add_executable(test_performance ${PERF_SRC_FILES})
target_include_directories(test_performance PRIVATE include)
target_link_libraries(test_performance PRIVATE Threads::Threads)
target_compile_options(test_performance PRIVATE
    -Wall -Wextra -Wpedantic
)

# Release 模式启用 O3 优化
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
