
#ifndef NONES_API_H
#define NONES_API_H

#include <stdint.h>
#include <stddef.h>

#ifdef _WIN32
#define NONES_API __declspec(dllexport)
#else
#define NONES_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the emulator. Returns 0 on success, nonzero on failure.
NONES_API int nones_init();

// Load a ROM from the given path. Returns 0 on success, nonzero on failure.
NONES_API int nones_load_rom(const char* path);

// Get a pointer to the current video frame in RGBA8888 format. Sets width and height.
// Returns pointer to internal buffer (do not free or modify).
NONES_API const uint8_t* nones_get_video_frame(uint32_t* width, uint32_t* height);

// Fill the provided buffer with up to max_samples of 16-bit PCM audio (mono, 44100Hz).
// Returns the number of samples written.
NONES_API size_t nones_get_audio_samples(int16_t* buffer, size_t max_samples);

// Advance the emulator by one frame (run emulation for one frame)
NONES_API void nones_advance_frame();

// Run the emulator at real-time (60Hz) in a background loop using SDL3 timing. Returns immediately; loop runs until stopped.
NONES_API void nones_run_realtime();

// Stop the real-time emulation loop started by nones_run_realtime.
NONES_API void nones_stop_realtime();

#ifdef __cplusplus
}
#endif

#endif // NONES_API_H
