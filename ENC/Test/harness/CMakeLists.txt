cmake_minimum_required(VERSION 3.20)
project(ns_test C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 产物路径: ENC/Test/bin/<Config>/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/$<CONFIG>")

if(MSVC)
    set(NS_TEST_C_FLAGS /W3 /wd4068 /utf-8)
else()
    set(NS_TEST_C_FLAGS -Wall -Wno-unknown-pragmas)
endif()

# 引入算法组件(复用同一份源码与 CMakeLists, 五个变体库在其内部定义)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../../NS_Component"
                 "${CMAKE_BINARY_DIR}/ns_component")

set(NS_HARNESS_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/wav_io.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/ns_runner.c")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/wav_io.c" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ns_runner.c")
    # 五个变体各自一个可执行文件: 五个库导出同名符号集, 单个进程只能链接其中一个。
    # 可执行文件名 ns_runner_<variant>.exe 与命令行 <variant> 参数必须一致,
    # 由 ns_runner 启动时用 NS_GetVariantName()/NS_GetPatchFlags() 交叉校验。
    foreach(v BASELINE D1 D1D5 D1D5D6 OPTIMIZED)
        string(TOLOWER "${v}" vlow)
        add_executable(ns_runner_${vlow} ${NS_HARNESS_SRC})
        target_link_libraries(ns_runner_${vlow} PRIVATE ns_core_${v})
        target_compile_options(ns_runner_${vlow} PRIVATE ${NS_TEST_C_FLAGS})
        target_include_directories(ns_runner_${vlow} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    endforeach()
else()
    message(STATUS "harness 源码尚未生成(T3 交付), 跳过 ns_runner target")
endif()
