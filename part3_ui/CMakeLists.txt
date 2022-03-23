cmake_minimum_required(VERSION 3.0)
project(lets_make_synth)

set(SYNTH_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
set(SYNTH_DEPS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../dependencies)
list(APPEND CMAKE_MODULE_PATH ${SYNTH_CMAKE_DIR})

include(${SYNTH_CMAKE_DIR}/CxxFlags.cmake)

find_package(SDL2 REQUIRED)
add_subdirectory(${SYNTH_DEPS_DIR}/nanovg ${CMAKE_CURRENT_BINARY_DIR}/nanovg)

include_directories(
    ${SDL2_INCLUDE_DIR}
)

add_executable(synth
    sdlwrapper.cpp
    oscillator.cpp
    ui.cpp
    main.cpp
)
target_link_libraries(synth PRIVATE
    ${SDL2_LIBRARY}
    m
    nanovg
)
