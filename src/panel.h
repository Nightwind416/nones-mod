#ifndef PANEL_H
#define PANEL_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL3/SDL.h>

// Panel types that can be displayed
typedef enum {
    PANEL_TYPE_GAME,        // Main game window
    PANEL_TYPE_MAP,         // Map viewer
    PANEL_TYPE_STATUS,      // Game status/debug info
    PANEL_TYPE_EDITOR,      // Map editor
    PANEL_TYPE_COUNT
} PanelType;

// Panel position in the grid
typedef enum {
    PANEL_POS_TOP_LEFT,
    PANEL_POS_TOP_RIGHT,
    PANEL_POS_BOTTOM_LEFT,
    PANEL_POS_BOTTOM_RIGHT,
    PANEL_POS_COUNT
} PanelPosition;

// Configuration for the panel layout
typedef struct {
    PanelType panels[PANEL_POS_COUNT];  // Which panel type to show in each position
    int panel_width;                     // Width of each panel
    int panel_height;                    // Height of each panel
} PanelConfig;

// Individual panel state
typedef struct {
    PanelType type;
    PanelPosition position;
    SDL_FRect rect;                     // Position and size in the window
    bool visible;
} Panel;

// Panel system state
typedef struct {
    Panel panels[PANEL_POS_COUNT];
    PanelConfig config;
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool show_settings;                 // Whether to show settings UI
} PanelSystem;

// Forward declarations for context
typedef struct System System;

// Panel rendering context - contains data needed to render panels
typedef struct {
    SDL_Texture *game_texture;
    System *system;
} PanelRenderContext;

// Initialize the panel system
void PanelSystemInit(PanelSystem *panel_system, SDL_Window *window, SDL_Renderer *renderer);

// Update panel layout based on window size
void PanelSystemUpdateLayout(PanelSystem *panel_system);

// Render all panels
void PanelSystemRender(PanelSystem *panel_system, void *nones_data);

// Get default panel configuration
PanelConfig PanelGetDefaultConfig(void);

// Render individual panel types
void PanelRenderGame(Panel *panel, SDL_Renderer *renderer, void *nones_data);
void PanelRenderMap(Panel *panel, SDL_Renderer *renderer, void *nones_data);
void PanelRenderStatus(Panel *panel, SDL_Renderer *renderer, void *nones_data);
void PanelRenderEditor(Panel *panel, SDL_Renderer *renderer, void *nones_data);

// Settings UI
void PanelRenderSettings(PanelSystem *panel_system, SDL_Renderer *renderer);

// Get panel type name
const char* PanelGetTypeName(PanelType type);

#endif // PANEL_H
