#ifndef NONES_API_H
#define NONES_API_H

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

#ifdef __cplusplus
}
#endif

#endif // NONES_API_H
