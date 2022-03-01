cmake_minimum_required(VERSION 3.0)
project(lets_make_synth)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

find_package(SDL2 REQUIRED)

include_directories(
    ${SDL2_INCLUDE_DIR}
)

string(JOIN " " CXX_FLAGS_LIST
    -O2
    -Wall
    -Wextra
    -Wshadow
    -Wnon-virtual-dtor
    -Wconversion
    -Wno-unused-parameter
    -Wnull-dereference
    -Wdouble-promotion
    -Wformat=2
    -pedantic
    -Werror
)
set(CMAKE_CXX_FLAGS "${CXX_FLAGS_LIST}")

add_executable(synth
    main.cpp
)
target_link_libraries(synth PRIVATE
    ${SDL2_LIBRARY}
    m
)
