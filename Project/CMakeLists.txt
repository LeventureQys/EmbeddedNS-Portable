# 设置 CMake 最低版本
cmake_minimum_required(VERSION 3.10)

# 项目名称和版本
project(ENC_Demo VERSION 1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
# 添加子项目 ENC_Core
add_subdirectory(ENC_Core)

# 添加可执行文件
add_executable(ENC_Demo main.cpp)

# 链接 ENC_Core 库
target_link_libraries(ENC_Demo PRIVATE ENC_Core)

# 包含 ENC_Core 的头文件
target_include_directories(ENC_Demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ENC_Core/AudioProcess)

# 根据构建类型设置输出目录
if(CMAKE_CONFIGURATION_TYPES)  # 多配置生成器(如Visual Studio)
    # 设置可执行文件和库的输出目录
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)
        set_target_properties(ENC_Demo PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin/${OUTPUTCONFIG}
        )
        set_target_properties(ENC_Core PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin/${OUTPUTCONFIG}
            ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin/${OUTPUTCONFIG}
        )
    endforeach()
else()  # 单配置生成器(如Makefile)
    set_target_properties(ENC_Demo PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    set_target_properties(ENC_Core PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()

# 在Windows平台下，添加一个后构建步骤将DLL复制到可执行文件目录
if(WIN32)
    add_custom_command(TARGET ENC_Demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:ENC_Core>
            $<TARGET_FILE_DIR:ENC_Demo>
    )
endif()