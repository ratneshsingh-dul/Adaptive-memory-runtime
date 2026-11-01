cmake_minimum_required(VERSION 3.20)

project(AdaptiveMemoryRuntime
    VERSION 0.1.0
    DESCRIPTION "A measured, adaptive C++ memory allocator"
    LANGUAGES CXX
)

add_library(amr INTERFACE)
target_include_directories(amr INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(amr INTERFACE cxx_std_20)

add_executable(amr_demo src/main.cpp)
target_link_libraries(amr_demo PRIVATE amr)

option(AMR_BUILD_BASELINE "Build the dependency-free system allocator baseline" ON)
if(AMR_BUILD_BASELINE)
    add_executable(amr_baseline benchmarks/baseline.cpp)
    target_link_libraries(amr_baseline PRIVATE amr)
endif()

if(MSVC)
    target_compile_options(amr_demo PRIVATE /W4 /permissive-)
    if(AMR_BUILD_BASELINE)
        target_compile_options(amr_baseline PRIVATE /W4 /permissive-)
    endif()
else()
    target_compile_options(amr_demo PRIVATE -Wall -Wextra -Wpedantic)
    if(AMR_BUILD_BASELINE)
        target_compile_options(amr_baseline PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()
