// Debug helper
#include <stdio.h>
#include "nones_api.h"
#include "nones.h"
#include <stdlib.h>
#include "cart.h"
#include <string.h>
#include <stddef.h>

#include <SDL3/SDL.h>
#include <stdatomic.h>

// Global emulator instance
static Nones g_nones;

// Threading and synchronization
static atomic_bool g_realtime_running = false;
static SDL_Thread *g_realtime_thread = NULL;
static SDL_Mutex *g_audio_mutex = NULL;
static SDL_Mutex *g_video_mutex = NULL;

// Audio ring buffer for smooth playback - increased size for better buffering
#define AUDIO_RING_SIZE (44100 * 4)  // 4 seconds of audio buffer for better stability
static int16_t g_audio_ring[AUDIO_RING_SIZE];
static atomic_int g_audio_write_pos = 0;
static atomic_int g_audio_read_pos = 0;

// Video frame management
static atomic_bool g_new_frame_available = false;
static uint32_t g_frame_counter = 0;
static uint32_t g_last_frame_retrieved = 0;

// Performance tracking
static uint64_t g_last_fps_time = 0;
static uint32_t g_fps_counter = 0;
static float g_current_fps = 0.0f;
static uint32_t g_audio_underruns = 0;

// Controller input state
static uint8_t g_controller_state[2] = {0, 0};

// Helper function to get available audio samples in ring buffer
static size_t get_audio_samples_available() {
    int write_pos = atomic_load(&g_audio_write_pos);
    int read_pos = atomic_load(&g_audio_read_pos);
    if (write_pos >= read_pos) {
        return write_pos - read_pos;
    } else {
        return AUDIO_RING_SIZE - read_pos + write_pos;
    }
}

// Helper function to write audio samples to ring buffer
static void write_audio_samples(const int16_t* samples, size_t count) {
    if (!g_audio_mutex) return;

    SDL_LockMutex(g_audio_mutex);

    int write_pos = atomic_load(&g_audio_write_pos);
    for (size_t i = 0; i < count; i++) {
        g_audio_ring[write_pos] = samples[i];
        write_pos = (write_pos + 1) % AUDIO_RING_SIZE;
    }

    atomic_store(&g_audio_write_pos, write_pos);

    SDL_UnlockMutex(g_audio_mutex);
}

// Thread function for real-time emulation loop
static int realtime_emulation_thread(void *data) {
    (void)data; // Suppress unused parameter warning
    printf("[nones_run_realtime] SDL3 real-time emulation thread started.\n");

    // Set high thread priority for better timing (SDL3 function name)
    SDL_SetCurrentThreadPriority(SDL_THREAD_PRIORITY_TIME_CRITICAL);

    // Use proper NES timing - 60 FPS for emulation, not the 500 FPS cap
    const uint64_t frame_time_ns = 1000000000ULL / 60; // ~16.67ms per frame

    double previous_time = 0;
    double current_time = 0;
    double accumulator = 0;

    while (atomic_load(&g_realtime_running)) {
        uint64_t start_time = SDL_GetTicksNS();
        previous_time = current_time;
        current_time = start_time;
        double delta_time = current_time - previous_time;
        accumulator += delta_time;

        // Run emulation frames using accumulator (same as standalone)
        while (accumulator >= frame_time_ns) {
            // Run one frame of emulation
            nones_advance_frame();

            // Copy audio samples to ring buffer
            if (g_nones.system && g_nones.system->apu) {
                write_audio_samples(g_nones.system->apu->outbuffer, 735);
            }

            // Mark new frame as available
            atomic_store(&g_new_frame_available, true);
            g_frame_counter++;

            accumulator -= frame_time_ns;
        }

        // Update FPS counter
        g_fps_counter++;
        if (current_time - g_last_fps_time >= 1000000000ULL) { // 1 second
            g_current_fps = (float)g_fps_counter;
            g_fps_counter = 0;
            g_last_fps_time = current_time;
        }

        // Proper frame limiting - limit to 60 FPS, not 500 FPS
        double frame_time = SDL_GetTicksNS() - start_time;
        if (frame_time < frame_time_ns) {
            SDL_DelayNS(frame_time_ns - frame_time);
        }
    }

    printf("[nones_run_realtime] SDL3 real-time emulation thread exiting.\n");
    return 0;
}

// Start real-time emulation loop in a background thread
void nones_run_realtime() {
    if (g_realtime_thread) return; // Already running

    // Initialize mutexes if not already done
    if (!g_audio_mutex) {
        g_audio_mutex = SDL_CreateMutex();
        g_video_mutex = SDL_CreateMutex();
    }

    // Reset timing counters
    g_last_fps_time = SDL_GetTicksNS();
    g_fps_counter = 0;
    g_current_fps = 0.0f;
    g_audio_underruns = 0;

    // Clear audio buffer for clean start
    atomic_store(&g_audio_write_pos, 0);
    atomic_store(&g_audio_read_pos, 0);

    atomic_store(&g_realtime_running, true);
    g_realtime_thread = SDL_CreateThread(realtime_emulation_thread, "nones_realtime", NULL);
}

// Stop the real-time emulation loop
void nones_stop_realtime() {
    if (!g_realtime_thread) return;

    atomic_store(&g_realtime_running, false);
    SDL_WaitThread(g_realtime_thread, NULL);
    g_realtime_thread = NULL;

    // Cleanup mutexes
    if (g_audio_mutex) {
        SDL_DestroyMutex(g_audio_mutex);
        g_audio_mutex = NULL;
    }
    if (g_video_mutex) {
        SDL_DestroyMutex(g_video_mutex);
        g_video_mutex = NULL;
    }
}

// Advance the emulator by one frame (for DLL/headless use)
void nones_advance_frame() {
    if (!g_nones.system) {
        printf("[nones_advance_frame] g_nones.system is NULL!\n");
        return;
    }

    // Update controller input before running frame
    if (g_nones.system) {
        // Convert our button state to the format expected by the emulator
        bool buttons[16] = {0};

        // Controller 1 mapping (A, B, Select, Start, Up, Down, Left, Right)
        buttons[0] = (g_controller_state[0] & 0x01) != 0; // A
        buttons[1] = (g_controller_state[0] & 0x02) != 0; // B
        buttons[7] = (g_controller_state[0] & 0x04) != 0; // Select
        buttons[6] = (g_controller_state[0] & 0x08) != 0; // Start
        buttons[2] = (g_controller_state[0] & 0x10) != 0; // Up
        buttons[3] = (g_controller_state[0] & 0x20) != 0; // Down
        buttons[4] = (g_controller_state[0] & 0x40) != 0; // Left
        buttons[5] = (g_controller_state[0] & 0x80) != 0; // Right

        // Controller 2 mapping (if needed)
        buttons[8]  = (g_controller_state[1] & 0x01) != 0; // A
        buttons[9]  = (g_controller_state[1] & 0x02) != 0; // B
        buttons[15] = (g_controller_state[1] & 0x04) != 0; // Select
        buttons[14] = (g_controller_state[1] & 0x08) != 0; // Start
        buttons[10] = (g_controller_state[1] & 0x10) != 0; // Up
        buttons[11] = (g_controller_state[1] & 0x20) != 0; // Down
        buttons[12] = (g_controller_state[1] & 0x40) != 0; // Left
        buttons[13] = (g_controller_state[1] & 0x80) != 0; // Right

        SystemUpdateJPButtons(g_nones.system, buttons);
    }

    // Reset PPU frame_finished flag to ensure we run a full frame
    if (g_nones.system->ppu) {
        g_nones.system->ppu->frame_finished = false;
    }

    // Track cycles at start of frame
    uint64_t start_cycles = g_nones.system->cpu->cycles;

    // Run one frame of emulation
    SystemRun(g_nones.system, false, false, true);

    // Calculate cycles executed this frame
    uint64_t executed_cycles = g_nones.system->cpu->cycles - start_cycles;

    // Only log and fix cycle count if not running in real-time thread
    if (executed_cycles < 29780) {
        uint64_t missing_cycles = 29780 - executed_cycles;
        // Only log if significant difference (>100 cycles)
        if (missing_cycles > 100) {
            printf("[nones_advance_frame] Adding %llu missing cycles\n", missing_cycles);
        }
        // Add missing cycles and update APU accordingly
        SystemAddCpuCycles(missing_cycles);
    }
}

// Return pointer to current video frame (RGBA8888), set width/height
const uint8_t* nones_get_video_frame(uint32_t* width, uint32_t* height) {
    if (!g_nones.system || !g_nones.system->ppu) return NULL;

    if (g_video_mutex) {
        SDL_LockMutex(g_video_mutex);
    }

    if (width) *width = SCREEN_WIDTH;
    if (height) *height = SCREEN_HEIGHT;

    // Mark that this frame has been retrieved
    g_last_frame_retrieved = g_frame_counter;

    // Front buffer is buffers[1], RGBA8888 (uint32_t per pixel)
    const uint8_t* result = (const uint8_t*)g_nones.system->ppu->buffers[1];

    if (g_video_mutex) {
        SDL_UnlockMutex(g_video_mutex);
    }

    return result;
}

// Get the current audio buffer fill level (0.0 to 1.0)
float nones_get_audio_buffer_level() {
    size_t available = get_audio_samples_available();
    return (float)available / (float)AUDIO_RING_SIZE;
}

// Check if new video frame is available since last call
int nones_has_new_frame() {
    return g_frame_counter != g_last_frame_retrieved;
}

// Set controller input state (8 buttons: A, B, Select, Start, Up, Down, Left, Right)
void nones_set_controller_input(int controller, uint8_t buttons) {
    if (controller >= 0 && controller < 2) {
        g_controller_state[controller] = buttons;
    }
}

// Get emulation timing statistics
void nones_get_timing_stats(float* fps, float* audio_underruns) {
    if (fps) *fps = g_current_fps;
    if (audio_underruns) *audio_underruns = (float)g_audio_underruns;
}

// Flush/clear the audio ring buffer (useful for seeking or reset)
void nones_flush_audio_buffer() {
    if (g_audio_mutex) {
        SDL_LockMutex(g_audio_mutex);
    }

    atomic_store(&g_audio_write_pos, 0);
    atomic_store(&g_audio_read_pos, 0);
    memset(g_audio_ring, 0, sizeof(g_audio_ring));

    if (g_audio_mutex) {
        SDL_UnlockMutex(g_audio_mutex);
    }
}

// Get audio latency information
void nones_get_audio_latency_info(float* buffer_ms, int* samples_available) {
    size_t available = get_audio_samples_available();

    if (buffer_ms) {
        *buffer_ms = (float)available / 44100.0f * 1000.0f; // Convert to milliseconds
    }
    if (samples_available) {
        *samples_available = (int)available;
    }
}

// Fill buffer with up to max_samples of 16-bit PCM audio, return samples written
size_t nones_get_audio_samples(int16_t* buffer, size_t max_samples) {
    if (!buffer || max_samples == 0) return 0;

    if (!g_audio_mutex) return 0;

    SDL_LockMutex(g_audio_mutex);

    size_t available = get_audio_samples_available();
    size_t to_copy = (max_samples < available) ? max_samples : available;

    if (to_copy == 0) {
        // Audio underrun - fill with silence
        memset(buffer, 0, max_samples * sizeof(int16_t));
        g_audio_underruns++;
        SDL_UnlockMutex(g_audio_mutex);
        return max_samples; // Return max_samples to indicate buffer was filled with silence
    }

    int read_pos = atomic_load(&g_audio_read_pos);

    // Copy samples from ring buffer
    for (size_t i = 0; i < to_copy; i++) {
        buffer[i] = g_audio_ring[read_pos];
        read_pos = (read_pos + 1) % AUDIO_RING_SIZE;
    }

    // Fill remaining buffer with silence if needed
    if (to_copy < max_samples) {
        memset(&buffer[to_copy], 0, (max_samples - to_copy) * sizeof(int16_t));
    }

    atomic_store(&g_audio_read_pos, read_pos);

    SDL_UnlockMutex(g_audio_mutex);

    return max_samples;
}

// Initialize the emulator (without loading a ROM)
int nones_init() {
    memset(&g_nones, 0, sizeof(g_nones));

    // Initialize SDL for timing functions (SDL3 doesn't require specific subsystems for timing)
    if (!SDL_Init(0)) {
        printf("[nones_init] SDL_Init failed: %s\n", SDL_GetError());
        return 3;
    }

    g_nones.arena = ArenaCreate(1024 * 1024 * 3);
    if (!g_nones.arena) return 1;

    g_nones.system = SystemCreate(g_nones.arena);
    if (!g_nones.system) {
        ArenaDestroy(g_nones.arena);
        return 2;
    }

    // Initialize audio ring buffer
    memset(g_audio_ring, 0, sizeof(g_audio_ring));
    atomic_store(&g_audio_write_pos, 0);
    atomic_store(&g_audio_read_pos, 0);

    // Initialize frame tracking
    atomic_store(&g_new_frame_available, false);
    g_frame_counter = 0;
    g_last_frame_retrieved = 0;

    // Initialize controller state
    memset(g_controller_state, 0, sizeof(g_controller_state));

    // Initialize performance counters
    g_current_fps = 0.0f;
    g_audio_underruns = 0;

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
