cmake_minimum_required(VERSION 3.0)
project(lets_make_synth)

set(SYNTH_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
list(APPEND CMAKE_MODULE_PATH ${SYNTH_CMAKE_DIR})
include(${SYNTH_CMAKE_DIR}/CxxFlags.cmake)
find_package(SDL2 REQUIRED)

include_directories(
    ${SDL2_INCLUDE_DIR}
)

add_executable(synth
    main.cpp
)
target_link_libraries(synth PRIVATE
    ${SDL2_LIBRARY}
    m
)
