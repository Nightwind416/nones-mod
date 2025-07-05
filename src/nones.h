#ifndef NONES_H
#define NONES_H

#include <stdint.h>
#include <stdbool.h>
#include "arena.h"
#include "system.h"
#include <SDL3/SDL.h>

//#define SCREEN_WIDTH 340
//#define SCREEN_HEIGHT 260
#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240
#define FRAMERATE 60
#define FRAMECAP 500
//#define FRAMERATE 60.098477556112265
#define FRAME_TIME_MS (1000.0 / FRAMERATE)
#define FRAME_CAP_MS (1000.0 / FRAMECAP)
#define FRAME_TIME_NS (1000000000.0 / FRAMERATE)
#define FRAME_CAP_NS (1000000000.0 / FRAMECAP)

typedef struct {
    Arena *arena;
    System *system;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    bool buttons[16];
    int num_gamepads;
    SDL_Gamepad *gamepad1;
    SDL_Gamepad *gamepad2;
    SDL_JoystickID *gamepads;
    bool quit;
} Nones;

void NonesRun(Nones *nones, const char *path);
void NonesPutSoundData(Apu *apu);

#endif
