cmake_minimum_required(VERSION 3.16)
project(HighPerfWebServer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT MSVC)
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

include_directories(include)

find_package(Threads REQUIRED)

# Server Core Library (Added src/thread_pool.cpp)
add_library(server_lib
    src/server.cpp
    src/thread_pool.cpp
)

target_link_libraries(server_lib PUBLIC Threads::Threads)

if(WIN32)
    target_link_libraries(server_lib PUBLIC ws2_32)
endif()

# Executable App
add_executable(web_server src/main.cpp)
target_link_libraries(web_server PRIVATE server_lib)