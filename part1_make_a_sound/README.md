# Part 1 - Making a Sound

For this first part, the goal is simple - generate a sine wave tone
in C++ that can be heard on PC speakers.

Sounds easy enough, so let's dive in!

## CMake, basic C++ project

Since we're starting with a blank slate, we need to get
a basic C++ executable to build, something like "Hello, World".

I'll be using CMake to build the project.

Here's the CMakeLists.txt:

```cmake
cmake_minimum_required(VERSION 3.0)
project(lets_make_a_synth)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

string(JOIN " " CXX_FLAGS_LIST
    -O2
    -Wall
    -Werror
)
set(CMAKE_CXX_FLAGS "${CXX_FLAGS_LIST}")
add_executable(synth main.cpp)
```

And here's main.cpp:

```cpp
#include <stdio.h>

int main(int argc, char* argv[]) {
    printf("__cplusplus: %ld\n", __cplusplus);
    return 0;
}
```

This simply prints the C++ version to `stdout` and returns.

I've also created a top-level `build.sh` script for convenience,
that simply invokes cmake and make:

```sh
mkdir -p build
cd build
cmake ..
make
```

And now, we can build it:

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

And we can run it:

```
$ build/synth
__cplusplus: 201703
```

Great! Now we've got a basic C++ project compiling and running.

## Research - How Do I Send Audio to My Sound Device?

So, how exactly do sound devices work? How do we send audio data to the
card?

For this, I turned to the Internet. Searching "Real time generate sound C"
turned up some good hits. After following some links and reading a few
StackOverflow links, I learned a few things:

* There's no easy, cross-platform way to send raw audio data to the sound
    card. We'll need an audio library.
* There are a bajillion audio libraries to choose from.

At this point, I think it makes sense to take a step back and think about the goals
of this project:

* Learn about digital synthesizers.
* Learn about real-time audio in C++.
* Spend majority of coding time on the synthesizer. Don't
  spend a lot of time implementing things from scratch not related to the
  synthesizer.
* We want to hear a sine wave from our speakers as fast and easily as
    possible.

Given these general goals, we can turn our focus to selecting an audio
library. Here are the rough criteria I came up with:

* Needs to be free, open-source, cross-platform, and support real-time audio
* Can't be too low level, as to be difficult to use.
* Can't be too high level, as to take away the joy
    of learning about real-time audio, digital signal processing,
    and digital synthesizers.

Out of the hundreds of possible audio libraries out there, there were three
that jumped out as front-runners in my quick search:

* Option 1: SDL2
    - Pro: I've used SDL2 before
    - Pro: there is [example audio code online](https://github.com/lundstroem/synth-samples-sdl2/blob/master/src/synth_samples_sdl2_1.c)
    - Pro: Can easily pair with an SDL2 graphical UI, if/when we need that in the future
    - Con: Not sure how reliable it is for real-time audio
- Option 2: PortAudio
    - Pro: Used by many popular projects, like Audacity and VLC
    - Pro: Good tutorial docs: http://files.portaudio.com/docs/v19-doxydocs/writing_a_callback.html
    - Con: More of a learning curve for me personally, compared to SDL2
- Option 3: openal
    - Con: too low level

At this early stage of the project, I chose SDL2 for the audio library.
It's possible this may change to PortAudio in the future if SDL audio is
not up to the task, but I hope that doesn't happen.

## A basic SDL2 project

We already have a basic C++ project, but now we need to add SDL2 audio.
For now, we just want to get a basic SDL window to show, and we want
to initialize the SDL audio subsystem.

First, we must install SDL2.

MacOS:

```sh
brew install sdl2
```

Ubuntu:

```sh
sudo apt-get install libsdl2-dev
```

Here's the new CMakeLists.txt, that includes SDL2:

```cmake
cmake_minimum_required(VERSION 3.0)
project(lets_make_a_synth)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# https://github.com/Kitware/CMake/blob/master/Modules/FindSDL.cmake
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

string(JOIN " " CXX_FLAGS_LIST
    -O2
    -Wall
    -Werror
)
set(CMAKE_CXX_FLAGS "${CXX_FLAGS_LIST}")
add_executable(synth main.cpp)
target_link_libraries(synth PRIVATE ${SDL2_LIBRARY})
```

And, finally, we've got a much larger `main.cpp` that creates a window with
a dark gray background.

```cpp
#include <SDL.h>
#include <stdio.h>

// Window Configuration
#define WINDOW_WIDTH 360
#define WINDOW_HEIGHT 640

// Audio Configuration
#define SAMPLE_RATE_HZ 48000
#define NUM_SOUND_CHANNELS 2
// Signed 16-bit samples, in system byte order
#define SAMPLE_FORMAT AUDIO_S16SYS
// Smaller buffer == lower latency, but too low will cause crackling
// For low-latency real-time audio, 512 samples should be good.
// Assuming 44.1KHz, this will be (512 / 44100) == 11.61 ms of latency.
#define SAMPLES_PER_BUFFER 512

static SDL_Renderer* _renderer;
static SDL_Window* _window;
static SDL_AudioDeviceID _audioDevice;

static void close(void) {
    SDL_Log("Closing");
    if (_renderer) {
        SDL_DestroyRenderer(_renderer);
    }
    if (_window) {
        SDL_DestroyWindow(_window);
    }
    SDL_CloseAudioDevice(_audioDevice);
    SDL_Quit();
}

static void audioCallback(void* userdata, Uint8* stream, int len) {
    memset(stream, 0, (size_t)len);
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (0 != SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return -1;
    }

    // Initialize Audio
    SDL_AudioSpec desired = {};
    desired.freq = SAMPLE_RATE_HZ;
    desired.format = SAMPLE_FORMAT;
    desired.channels = NUM_SOUND_CHANNELS;
    desired.samples = SAMPLES_PER_BUFFER;
    desired.callback = audioCallback;

    SDL_AudioSpec actual = {};
    _audioDevice = SDL_OpenAudioDevice(NULL, 0, &desired, &actual, 0);
    if (_audioDevice <= 0) {
        SDL_Log("Could not open audio device: %s", SDL_GetError());
        return -2;
    }
    if (desired.format != actual.format) {
        SDL_Log("Could not get desired audio format");
        return -2;
    }

    SDL_Log("-------------------");
    SDL_Log("sample rate: %d", actual.freq);
    SDL_Log("channels:    %d", actual.channels);
    SDL_Log("samples:     %d", actual.samples);
    SDL_Log("size:        %d", actual.size);
    SDL_Log("------------------");

    SDL_PauseAudioDevice(_audioDevice, 0);

    // Create a window
    _window = SDL_CreateWindow(
        "Synth",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        0
    );
    if (_window == nullptr) {
        SDL_Log("Could not create window: %s", SDL_GetError());
        close();
        return -3;
    }

    // Create a renderer
    _renderer = SDL_CreateRenderer(
            _window,
            -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (_renderer == nullptr) {
        SDL_Log("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        close();
        return -4;
    }
    SDL_RenderSetLogicalSize(_renderer, WINDOW_WIDTH, WINDOW_HEIGHT);

    bool loopShouldStop = false;
    SDL_Event event;

    // Main SDL Loop
    SDL_Log("Starting main loop");
    while (!loopShouldStop) {
        // Check for events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                loopShouldStop = true;
            }
        }

        // Render frame with dark gray background
        SDL_RenderClear(_renderer);
        SDL_SetRenderDrawColor(_renderer, 25, 25, 25, 255);
        SDL_RenderPresent(_renderer);
    }

    close();
    return 0;
}
```

This is not meant to be a series about SDL, so if you want to learn more
about the SDL library, I can recommend the [Lazy Foo' SDL
Tutorials](https://lazyfoo.net/tutorials/SDL/).

I do however want to draw your attention to a few snippets of the code.

### Audio Device Configuration

The SDL audio device is initialized with these lines:

```cpp
    // Initialize Audio
    SDL_AudioSpec desired = {};
    desired.freq = SAMPLE_RATE_HZ;
    desired.format = SAMPLE_FORMAT;
    desired.channels = NUM_SOUND_CHANNELS;
    desired.samples = SAMPLES_PER_BUFFER;
    desired.callback = audioCallback;

    SDL_AudioSpec actual = {};
    _audioDevice = SDL_OpenAudioDevice(NULL, 0, &desired, &actual, 0);
```

Here, we're setting up our desired sample rate (48000 Hz), sample format (signed,
16-bit samples), number of channels (2 for stereo), and samples per buffer
(256).

The way SDL audio works is - there is a separate audio thread that runs
in SDL, and when it needs new audio data to send to hardware, it will call
our `audioCallback` function with a buffer that needs to be filled by us with
sample data. The size of this buffer will depend on our `SAMPLES_PER_BUFFER`
configuration. We've set it to 256, meaning there are 256 samples per buffer.
Each sample consist of two 16-bit values (one for left channel, one for
right), so the total buffer size in bytes is 256 * 2 * 2 = 1024 bytes.

With 256 samples per buffer and a sample rate of 48 KHz, this means the buffer
will hold a total of (256 / 48000) = .005333 seconds = 5.333 ms of audio data.
That also means our callback will be called roughly every 5.333 ms. That means
our callback can't waste any time - it needs to fill the buffer as fast as it
can and return as soon as possible.

The samples per buffer also determines the audio latency.
Most musicians playing a live instrument need 20 ms or less of latency, or else they'll start to notice.
We have an audio
latency of about 5.333 ms. Or in other words, from the time the sound is
triggered, it will take about 5.333 ms to reach the hardware. In reality, it might
actually be twice this amount, but it should be good enough for now.

I think (though I'm not certain) that SDL uses a front and back buffer,
such that it will call our callback to fill the back buffer while the hardware is accessing the front
buffer. Then when the front buffer runs out, it swaps front and back, then
calls our callback to fill the back buffer again.

If we look at our audio callback, it's not doing much right now:

```cpp
static void audioCallback(void* userdata, Uint8* stream, int len) {
    memset(stream, 0, (size_t)len);
}
```

Since we're dealing with signed 16-bit samples, the range of each sample
will be -32678 to 32767. For now, we just set
all samples to zero, which will result in _bone-chilling_ silence, which
pairs well with our dark window background.

## Generating a sine wave

Okay, now for the fun part. We're going to generate a sine wave which will
ultimately push some waves out into the air with a frequency of 440 Hz that
our human ears are sensitive to, and which will be perceived as "sound".

It's been a while....what's the equation for a sine wave again? Let's look at
[Wikipedia](https://en.wikipedia.org/wiki/Sine_wave).

Okay, cool, the equation is a function of time:

```
y(t) = A * sin(2 * PI * f * t + shift)
```

We won't use any phase shift, so we can leave that at 0. Okay, so how is time
going to work?

Well, we know with a sample rate of 48 KHz, each sample accounts for (1
/ 48000) = 2.083e-05 seconds. So when our audio callback gets called, we'll
just start putting in samples one at a time, and advancing time by this
miniscule amount.

In code, that will look something like this:

```cpp
// Max volume, scaling factor from 0.0 to 1.0
#define VOLUME 0.025 // about -32 dB

static void audioCallback(void* userdata, Uint8* stream, int len) {
    constexpr double twoPi = 2.0 * M_PI;
    constexpr double secondsPerSample = 1.0 / SAMPLE_RATE_HZ;

    // Sine wave, A440 Hz
    constexpr double freqHz = 440;
    constexpr double periodS = 1.0 / freqHz;
    // For s16, range is [-32768, 32767]
    constexpr double maxAmp = 32767.0 * VOLUME;

    static double t = 0.0;
    while (len > 0) {
        // Equation for a sine wave:
        //    y(t) = A * sin(2 * PI * f * t + shift)
        double y = maxAmp * sin(twoPi * freqHz * t);

        int16_t* left = (int16_t*)(stream);
        int16_t* right = (int16_t*)(stream + 2);
        *left = (int16_t)y;
        *right = (int16_t)y;

        t += secondsPerSample;
        if (t > periodS) {
             t -= periodS;
        }
        stream += 4;
        len -= 4;
    }
}
```

We don't want our ears to start bleeding, so the `VOLUME` scaling factor
reduces the sine wave amplitude to a comfortable listening level.

After compiling and running the program again, we get a glorious tone
at 440 Hz. We won't be winning Grammy's anytime soon, but it feels good
to have basic audio working.
