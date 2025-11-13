cmake_minimum_required(VERSION 3.10) # 修正: 与主项目保持一致
project(ENC_Core  VERSION 0.1.0 LANGUAGES C ) # 指明纯C项目

# 收集 AudioProcessing 目录下的所有 .cpp 文件
file(GLOB_RECURSE SOURCES AudioProcess/*.cc AudioProcess/*.c AudioProcess/*.h)

# 打印找到的源文件列表（可选，用于调试）
message(STATUS "Found source files:")
foreach(SOURCE ${SOURCES})
    message(STATUS "  ${SOURCE}")
endforeach()
add_definitions(-DENCV)
message("FILES: ${SOURCES}") # 打印 FILES 变量

# 现在是生成库
add_library(ENC_Core SHARED
    ${SOURCES}
)

# 设置 ENC_Core 库的包含目录 (PUBLIC，因为 ENC_Demo 需要使用)
target_include_directories(ENC_Core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/AudioProcess # 修正: 使用 target_include_directories,  PUBLIC 作用域，使用更明确的路径
)