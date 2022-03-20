#include "constants.h"
#include "ui.h"

namespace UI {

void drawWaveform(SDL_Renderer* renderer, Oscillator::Fn oscFn, double freqHz, bool isPlaying) {
    SDL_SetRenderDrawColor(renderer, 50, 205, 50, 255); // lime green

    int padding = (int)(0.1 * (double)WINDOW_WIDTH);
    int drawingWidth = WINDOW_WIDTH - 2 * padding;

    // Draw the waveform from left to right, as if starting
    // from the waveform point at t = 0.
    const double periodS = 1.0 / DEFAULT_FREQ;
    for (double t = 0.0; t < periodS; t += dt) {
        double y = oscFn(t, freqHz);
        // Convert (t,y) to 2-D coord (px,py)
        int px = padding + (int)((t / periodS) * drawingWidth);
        int py = padding + (int)((drawingWidth / 2) * (1.0 - y));
        int radius = (isPlaying ? 4 : 1);
        drawCircle(renderer, px, py, radius);
    }
}

void drawCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
            }
        }
    }
}

} // namespace UI
