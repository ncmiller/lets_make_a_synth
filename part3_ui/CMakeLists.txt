cmake_minimum_required(VERSION 3.0)
project(lets_make_synth)

set(SYNTH_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
set(SYNTH_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../third_party)
list(APPEND CMAKE_MODULE_PATH ${SYNTH_CMAKE_DIR})

find_package(SDL2 REQUIRED)
add_subdirectory(${SYNTH_THIRD_PARTY_DIR}/nanovg ${CMAKE_CURRENT_BINARY_DIR}/nanovg)
add_subdirectory(${SYNTH_THIRD_PARTY_DIR}/glad ${CMAKE_CURRENT_BINARY_DIR}/glad)
include_directories(
    ${SDL2_INCLUDE_DIR}
)

include(${SYNTH_CMAKE_DIR}/CxxFlags.cmake)

add_executable(synth
    sdlwrapper.cpp
    oscillator.cpp
    ui.cpp
    utility.cpp
    audio.cpp
    input.cpp
    main.cpp
)

target_link_libraries(synth PRIVATE
    ${SDL2_LIBRARY}
    m
    nanovg
    glad
)
