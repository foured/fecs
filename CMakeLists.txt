cmake_minimum_required(VERSION 3.10)

project(fecs)

add_library(${CMAKE_PROJECT_NAME} INTERFACE)

target_include_directories(${CMAKE_PROJECT_NAME} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_compile_definitions(${CMAKE_PROJECT_NAME} INTERFACE $<$<CONFIG:Debug>:FECS_LOGGING>)