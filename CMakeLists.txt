cmake_minimum_required(VERSION 3.10)
project(TEST)

add_definitions(-std=c++23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/main.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(test ${SOURCE_FILES})

target_include_directories(test PUBLIC src)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(DEBUG)
endif()


# target_compile_options(test PRIVATE 
#     $<$<CONFIG:Release>:-O3 -g0>
#     $<$<CONFIG:Debug>:-Og -g3>
# )
#
# target_link_options(test PRIVATE 
#     $<$<CONFIG:Release>:-O3 -g0>
#     $<$<CONFIG:Debug>:-Og -g3>
# )
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto -O3 -g0 -DNDEBUG")

if(EXISTS "${CMAKE_BINARY_DIR}/compile_commands.json" AND CMAKE_BUILD_TYPE MATCHES Debug)
    file(COPY "${CMAKE_BINARY_DIR}/compile_commands.json" DESTINATION "${CMAKE_SOURCE_DIR}")
endif()
