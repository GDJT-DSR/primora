cmake_minimum_required(VERSION 3.15)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

project(primora DESCRIPTION "a verifier and generator of prime" LANGUAGES C)

file(GLOB SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/*.c")

# add_compile_options(-fsanitize=address -g)

add_executable(primora "${SOURCES}")
if(MSVC)
    # 屏蔽 sscanf/fgets 等 CRT 函数的 C4996 警告
    target_compile_definitions(primora PRIVATE _CRT_SECURE_NO_WARNINGS)

    target_compile_options(primora PRIVATE
        /utf-8                                   # 源文件是 UTF-8，里面有中文字符串
        $<$<CONFIG:Release>:/O2 /GL>
        $<$<CONFIG:Debug>:/Od /Zi>
    )

    target_link_options(primora PRIVATE
        $<$<CONFIG:Release>:/LTCG>
    )
else()
    target_compile_options(primora PRIVATE
        $<$<CONFIG:Release>:-O3 -march=native -funroll-loops -flto>
        $<$<CONFIG:Debug>:-g -fno-omit-frame-pointer>
    )

    target_link_options(primora PRIVATE
        $<$<CONFIG:Release>:-flto>
    )
endif()
