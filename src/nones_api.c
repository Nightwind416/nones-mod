#include "nones_api.h"
#include "nones.h"
#include "cart.h"
#include <string.h>

// Global emulator instance
static Nones g_nones;

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
    return result;
}
