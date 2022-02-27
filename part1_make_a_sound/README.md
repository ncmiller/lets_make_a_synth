# Part 1 - Making a Sound

For this first part, the goal is simple: generate a sine wave tone in C++
that can be heard on my PC speakers.

Sounds easy enough, let's dive in!

## CMake, basic C++ project

Since we're starting with a blank slate, we need to get
a basic executable to build, something like "Hello, World".

I'll be using CMake to build the project.

Here's the CMakeLists.txt:

```cmake
cmake_minimum_required(VERSION 3.0)
project(lets_make_synth)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

include_directories(
    include
)

set(SOURCES
    src/main.cpp
)

string(JOIN " " CXX_FLAGS_LIST
    -O2

    # https://github.com/lefticus/cppbestpractices/blob/master/02-Use_the_Tools_Available.md#gcc%E2%80%94clang
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

add_executable(synth ${SOURCES})
```

And here's main.cpp:

```cpp
#include <stdio.h>

int main(int argc, char* argv[]) {
    printf("__cplusplus: %ld\n", __cplusplus);
    return 0;
}
```

I've also got a top-level `build.sh` script that simply calls cmake:

```sh
mkdir -p build
cd build
cmake ..
make
```

Now to build it:

```
$ ./build.sh
-- Configuring done
-- Generating done
-- Build files have been written to: /users/nick.miller/src/lets_make_a_synth/build
Consolidate compiler generated dependencies of target synth
[ 50%] Building CXX object CMakeFiles/synth.dir/src/main.cpp.o
[100%] Linking CXX executable synth
[100%] Built target synth
```

And finally, run it:

```
$ build/synth
__cplusplus: 201703
```

Great, now we've got a basic C++ project compiling and running.

## Research - How Do I Send Audio to My Sound Device?

* Don't want to waste time designing things from scratch not related to synth
* Don't want to work with too low or too high level of API
* Want to get sound working as quickly as possible
* Want free, open-source, cross-platform, real-time PCM stream, small, simple
* Googled "Real time generate sound C" - lots of options
* Options
    - SDL2
        - I've used SDL2 before
        - Example code: https://github.com/lundstroem/synth-samples-sdl2/blob/master/src/synth_samples_sdl2_1.c
        - Can easily pair with an SDL2 graphical UI
    - PortAudio
        - Used by many popular projects, like Audacity and VLC
        - Good tutorial docs: http://files.portaudio.com/docs/v19-doxydocs/writing_a_callback.html
    - openal
        - Too low level
* Decision: SDL2 best meets my criteria

## A basic SDL2 project

A small diversion...
If you want to know more, see SDL2 tutorials

* Installing SDL2

MacOS:

```sh
brew install sdl2
```

Ubuntu:

```sh
sudo apt-get install libsdl2-dev
```

* CMake updates
    - SDL finder scripts
        - FindSDL*.cmake scripts from Kitware
        - Official source (I think): https://github.com/Kitware/CMake/blob/master/Modules/FindSDL.cmake
    - Main CMakeLists.txt
        - find_package
        - include_directories
        - target_link_libraries

* main.cpp: Create simple SDL2 window, initialize audio subsystem

## Generating a sine wave

* Review equation for sine wave
* Review callback specifics
