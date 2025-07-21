cmake_minimum_required(VERSION 3.10)

project(fecs)

add_library(fecs INTERFACE)

target_include_directories(fecs INTERFACE "include")

target_compile_definitions(fecs INTERFACE $<$<CONFIG:Debug>:FECS_LOGGING>)