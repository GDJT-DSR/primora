cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(primora DESCRIPTION "a verifier and generator of prime" LANGUAGES C)

file(GLOB SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/*.c")

# add_compile_options(-fsanitize=address -g)

add_executable(primora "${SOURCES}")
target_compile_options(primora PRIVATE
    $<$<CONFIG:Release>:-O3 -march=native -funroll-loops -flto>
    $<$<CONFIG:Debug>:-fsanitize=address -fno-omit-frame-pointer -g>
)

target_link_options(primora PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=address>
    $<$<CONFIG:Release>:-flto>
)
