cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "opencl-tutorial-part-2")

project(${PROJECT_NAME})

add_executable(${PROJECT_NAME} Main.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS NO
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

include_directories("lib/OpenCL/include")

find_library(OpenCL OpenCL.lib "lib/OpenCL/lib")

target_link_libraries(${PROJECT_NAME} debug ${OpenCL} optimized ${OpenCL})
