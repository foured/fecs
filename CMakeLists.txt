cmake_minimum_required(VERSION 3.12)

project(fecs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Static linking c++ runtime library
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


if(RELEASE_BUILD)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
endif()

if(MSVS)
    add_compile_options(/arch:AVX2)
    add_compile_options(/MP)
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# add libs dirs

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(${CMAKE_PROJECT_NAME})

# if(RELEASE_BUILD)

# else()
# 	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC LOGGING) 
# endif()

target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${SRC_FILES})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")