cmake_minimum_required(VERSION 3.20)
project(ns_core C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# 产物路径: build/bin/<Config>/ 与 build/lib/<Config>/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/$<CONFIG>")

set(NS_COMPONENT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

# 算法源码: src/*.c 自动收集(CONFIGURE_DEPENDS 让新增文件自动触发重配置)
file(GLOB NS_ALGO_SRC CONFIGURE_DEPENDS "${NS_COMPONENT_ROOT}/src/*.c")
set(NS_LIB_SRC ${NS_ALGO_SRC})
if(EXISTS "${NS_COMPONENT_ROOT}/port/arm_math_compat.c")
    list(APPEND NS_LIB_SRC "${NS_COMPONENT_ROOT}/port/arm_math_compat.c")
endif()

set(NS_PUBLIC_INCLUDES
    "${NS_COMPONENT_ROOT}/api"
    "${NS_COMPONENT_ROOT}/src"
    "${NS_COMPONENT_ROOT}/port")

# MSVC 说明:
#   /W3      —— 计划书 T2 验收要求"无警告"
#   /wd4068  —— 源码含 #pragma unroll / #pragma O0 / #pragma push|pop(ARMCC 语义),
#               MSVC 不识别。屏蔽 C4068(unknown pragma), 不改动源码。
#   /utf-8   —— 源码为 UTF-8 中文注释, 必须显式指定源/执行字符集。
if(MSVC)
    set(NS_C_FLAGS /W3 /wd4068 /utf-8)
else()
    set(NS_C_FLAGS -Wall -Wno-unknown-pragmas)
endif()

function(ns_add_variant name d1 d5 d6 d8)
    add_library(${name} STATIC ${NS_LIB_SRC})
    target_include_directories(${name} PUBLIC ${NS_PUBLIC_INCLUDES})
    target_compile_definitions(${name} PRIVATE
        NS_PATCH_D1=${d1}
        NS_PATCH_D5=${d5}
        NS_PATCH_D6=${d6}
        NS_PATCH_D8=${d8}
        ENC_SAMPLE_DEPTH=24)
    target_compile_options(${name} PRIVATE ${NS_C_FLAGS})
endfunction()

# 冻结源固有的两类警告(见执行记录 B-26):
#   histograms.c / quantile_noise_estimator.c -> C4244 (float/double -> int32_t 隐式转换)
#   noise_suppressor.c                        -> C4101 (InstRfftF32 未使用;
#                                                计划书 2.3 节 E 注明确要求"保留原样")
# 计划书 13.2 禁止改动算法源码, 故本项与 /wd4068 完全同性质: 只在编译选项层豁免,
# 不做全局屏蔽(/wd 只作用于上述具名文件), 不改动任何源码字符。
if(MSVC)
    set_source_files_properties(
        "${NS_COMPONENT_ROOT}/src/histograms.c"
        "${NS_COMPONENT_ROOT}/src/quantile_noise_estimator.c"
        PROPERTIES COMPILE_OPTIONS "/wd4244")
    set_source_files_properties(
        "${NS_COMPONENT_ROOT}/src/noise_suppressor.c"
        PROPERTIES COMPILE_OPTIONS "/wd4101")
endif()

# 消融五组（D8 由执行记录 B-24 确认为新缺陷，经用户决策把 Q1 的修复范围由三项扩为四项）
ns_add_variant(ns_core_BASELINE   0 0 0 0)   # patch_flags = 0
ns_add_variant(ns_core_D1         1 0 0 0)   # patch_flags = 1
ns_add_variant(ns_core_D1D5       1 1 0 0)   # patch_flags = 3
ns_add_variant(ns_core_D1D5D6     1 1 1 0)   # patch_flags = 7
ns_add_variant(ns_core_OPTIMIZED  1 1 1 1)   # patch_flags = 15

# 兼容层自测程序(独立可执行, 由 T1 产出源码)
if(EXISTS "${NS_COMPONENT_ROOT}/port/arm_math_compat_test.c")
    add_executable(arm_math_compat_test
        "${NS_COMPONENT_ROOT}/port/arm_math_compat_test.c"
        "${NS_COMPONENT_ROOT}/port/arm_math_compat.c")
    target_include_directories(arm_math_compat_test PRIVATE ${NS_PUBLIC_INCLUDES})
    target_compile_options(arm_math_compat_test PRIVATE ${NS_C_FLAGS})
else()
    message(STATUS "arm_math_compat_test.c 尚未生成(T1 交付), 跳过自测程序 target")
endif()
