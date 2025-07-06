// Debug helper
#include <stdio.h>
#include "nones_api.h"
#include "nones.h"
#include <stdlib.h>
#include "cart.h"
#include <string.h>

#include <SDL3/SDL.h>
#include <stdatomic.h>

// Global emulator instance
static Nones g_nones;

static atomic_bool g_realtime_running = false;
static SDL_Thread *g_realtime_thread = NULL;

// Thread function for real-time emulation loop
static int realtime_emulation_thread(void *data) {
    printf("[nones_run_realtime] SDL3 real-time emulation thread started.\n");
    const double frame_time = 1000.0 / 60.0; // ms per frame
    while (atomic_load(&g_realtime_running)) {
        uint64_t start = SDL_GetTicks();
        // printf("[nones_run_realtime] Advancing frame...\n");
        nones_advance_frame();
        // printf("[nones_run_realtime] Frame advanced.\n");
        uint64_t elapsed = SDL_GetTicks() - start;
        if (elapsed < frame_time) {
            SDL_Delay((Uint32)(frame_time - elapsed));
        }
    }
    printf("[nones_run_realtime] SDL3 real-time emulation thread exiting.\n");
    return 0;
}

// Start real-time emulation loop in a background thread
void nones_run_realtime() {
    if (g_realtime_thread) return; // Already running
    atomic_store(&g_realtime_running, true);
    g_realtime_thread = SDL_CreateThread(realtime_emulation_thread, "nones_realtime", NULL);
}

// Stop the real-time emulation loop
void nones_stop_realtime() {
    if (!g_realtime_thread) return;
    atomic_store(&g_realtime_running, false);
    SDL_WaitThread(g_realtime_thread, NULL);
    g_realtime_thread = NULL;
}

// Advance the emulator by one frame (for DLL/headless use)
void nones_advance_frame() {
    if (!g_nones.system) {
        printf("[nones_advance_frame] g_nones.system is NULL!\n");
        return;
    }
    // printf("[nones_advance_frame] SystemRun called.\n");
    SystemRun(g_nones.system, false, false, true);
    // printf("[nones_advance_frame] SystemRun finished.\n");
}

// Return pointer to current video frame (RGBA8888), set width/height
const uint8_t* nones_get_video_frame(uint32_t* width, uint32_t* height) {
    if (!g_nones.system || !g_nones.system->ppu) return NULL;
    if (width) *width = SCREEN_WIDTH;
    if (height) *height = SCREEN_HEIGHT;
    // Front buffer is buffers[1], RGBA8888 (uint32_t per pixel)
    return (const uint8_t*)g_nones.system->ppu->buffers[1];
}

// Fill buffer with up to max_samples of 16-bit PCM audio, return samples written
size_t nones_get_audio_samples(int16_t* buffer, size_t max_samples) {
    if (!g_nones.system || !g_nones.system->apu || !buffer) return 0;
    // outbuffer is int16_t[735] (mono, 44100Hz, 1 frame worth)
    size_t available = 735; // LOW_RATE_SAMPLES
    size_t to_copy = (max_samples < available) ? max_samples : available;
    memcpy(buffer, g_nones.system->apu->outbuffer, to_copy * sizeof(int16_t));
    return to_copy;
}

// Initialize the emulator (without loading a ROM)
int nones_init() {
    memset(&g_nones, 0, sizeof(g_nones));
    g_nones.arena = ArenaCreate(1024 * 1024 * 3);
    if (!g_nones.arena) return 1;
    g_nones.system = SystemCreate(g_nones.arena);
    if (!g_nones.system) {
        ArenaDestroy(g_nones.arena);
        return 2;
    }
    // SDL and window/renderer/audio are not initialized here (for headless use)
    return 0;
}

// Load a ROM file into the already-initialized emulator
int nones_load_rom(const char* path) {
    if (!g_nones.arena || !g_nones.system) return 1;
    // Load the ROM using SystemLoadCart
    int result = SystemLoadCart(g_nones.arena, g_nones.system, path);
    if (result == 0) {
        // Set up PPU/APU/CPU and video buffers
        static uint32_t* buffers[2] = {NULL, NULL};
        // Allocate two frame buffers if not already done
        if (!buffers[0]) buffers[0] = malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
        if (!buffers[1]) buffers[1] = malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
        SystemInit(g_nones.system, buffers);
    }
    return result;
}
