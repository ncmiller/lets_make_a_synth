# Part 1 - Making a Sound

* Goal: write C++ code that will generate a sine wave and hear it through my speakers

## CMake, basic C++ project

* Print C++ version and return

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
brew install sdl2 sdl2_image sdl2_ttf sdl2_mixer
```

Ubuntu:

```sh
sudo apt-get install libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev libsdl2-mixer-dev
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
