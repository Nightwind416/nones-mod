#include <stdio.h>
#include <string.h>
#include <SDL3/SDL.h>
#include "panel.h"

// Define these locally to avoid including nones.h (circular dependency)
#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240

// Get default panel configuration
PanelConfig PanelGetDefaultConfig(void)
{
    PanelConfig config = {
        .panels = {
            [PANEL_POS_TOP_LEFT] = PANEL_TYPE_GAME,
            [PANEL_POS_TOP_RIGHT] = PANEL_TYPE_MAP,
            [PANEL_POS_BOTTOM_LEFT] = PANEL_TYPE_STATUS,
            [PANEL_POS_BOTTOM_RIGHT] = PANEL_TYPE_EDITOR
        },
        .panel_width = SCREEN_WIDTH,
        .panel_height = SCREEN_HEIGHT
    };
    return config;
}

// Get panel type name for display
const char* PanelGetTypeName(PanelType type)
{
    switch (type) {
        case PANEL_TYPE_GAME:   return "Game Window";
        case PANEL_TYPE_MAP:    return "Map Viewer";
        case PANEL_TYPE_STATUS: return "Status Window";
        case PANEL_TYPE_EDITOR: return "Map Editor";
        default:                return "Unknown";
    }
}

// Initialize the panel system
void PanelSystemInit(PanelSystem *panel_system, SDL_Window *window, SDL_Renderer *renderer)
{
    memset(panel_system, 0, sizeof(*panel_system));
    
    panel_system->window = window;
    panel_system->renderer = renderer;
    panel_system->config = PanelGetDefaultConfig();
    panel_system->show_settings = false;
    
    // Initialize individual panels
    for (int i = 0; i < PANEL_POS_COUNT; i++) {
        panel_system->panels[i].type = panel_system->config.panels[i];
        panel_system->panels[i].position = (PanelPosition)i;
        panel_system->panels[i].visible = true;
    }
    
    PanelSystemUpdateLayout(panel_system);
}

// Update panel layout based on configuration
void PanelSystemUpdateLayout(PanelSystem *panel_system)
{
    int panel_w = panel_system->config.panel_width;
    int panel_h = panel_system->config.panel_height;
    
    // Set up 2x2 grid layout
    panel_system->panels[PANEL_POS_TOP_LEFT].rect = (SDL_FRect){0, 0, (float)panel_w, (float)panel_h};
    panel_system->panels[PANEL_POS_TOP_RIGHT].rect = (SDL_FRect){(float)panel_w, 0, (float)panel_w, (float)panel_h};
    panel_system->panels[PANEL_POS_BOTTOM_LEFT].rect = (SDL_FRect){0, (float)panel_h, (float)panel_w, (float)panel_h};
    panel_system->panels[PANEL_POS_BOTTOM_RIGHT].rect = (SDL_FRect){(float)panel_w, (float)panel_h, (float)panel_w, (float)panel_h};
    
    // Update window size to fit all panels
    SDL_SetWindowSize(panel_system->window, panel_w * 2, panel_h * 2);
}

// Render the game panel
void PanelRenderGame(Panel *panel, SDL_Renderer *renderer, void *nones_data)
{
    // Extract needed fields - we get System and SDL_Texture
    // Since we can't include nones.h due to circular dependency,
    // we'll need to pass these separately or restructure
    
    // For now, just render a placeholder since we need to refactor
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &panel->rect);
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
    SDL_RenderDebugText(renderer, panel->rect.x + 10, panel->rect.y + 10, "Game Window");
}

// Render the map panel
void PanelRenderMap(Panel *panel, SDL_Renderer *renderer, void *nones_data)
{
    (void)nones_data;  // Unused for now
    
    // Set background
    SDL_SetRenderDrawColor(renderer, 20, 20, 40, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &panel->rect);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 100, 100, 200, SDL_ALPHA_OPAQUE);
    SDL_RenderRect(renderer, &panel->rect);
    
    // Draw title
    SDL_SetRenderDrawColor(renderer, 200, 200, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDebugText(renderer, panel->rect.x + 10, panel->rect.y + 10, "Map Viewer");
    
    // Placeholder text
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, SDL_ALPHA_OPAQUE);
    SDL_RenderDebugText(renderer, panel->rect.x + 10, panel->rect.y + 30, "(Nametable viewer)");
}

// Render the status panel
void PanelRenderStatus(Panel *panel, SDL_Renderer *renderer, void *nones_data)
{
    (void)nones_data;  // Unused for now
    
    // Set background
    SDL_SetRenderDrawColor(renderer, 20, 40, 20, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &panel->rect);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 100, 200, 100, SDL_ALPHA_OPAQUE);
    SDL_RenderRect(renderer, &panel->rect);
    
    // Draw title
    SDL_SetRenderDrawColor(renderer, 200, 255, 200, SDL_ALPHA_OPAQUE);
    SDL_RenderDebugText(renderer, panel->rect.x + 10, panel->rect.y + 10, "Game Status");
    
    // Placeholder text
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, SDL_ALPHA_OPAQUE);
    SDL_RenderDebugText(renderer, panel->rect.x + 10, panel->rect.y + 30, "(CPU/PPU state)");
}

// Render the editor panel
void PanelRenderEditor(Panel *panel, SDL_Renderer *renderer, void *nones_data)
{
    // Set background
    SDL_SetRenderDrawColor(renderer, 40, 20, 20, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &panel->rect);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 200, 100, 100, SDL_ALPHA_OPAQUE);
    SDL_RenderRect(renderer, &panel->rect);
    
    // Draw title
    SDL_SetRenderDrawColor(renderer, 255, 200, 200, SDL_ALPHA_OPAQUE);
    SDL_RenderDebugText(renderer, panel->rect.x + 10, panel->rect.y + 10, "Map Editor");
    
    // Placeholder text
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, SDL_ALPHA_OPAQUE);
    SDL_RenderDebugText(renderer, panel->rect.x + 10, panel->rect.y + 40, "(Editor tools)");
}

// Render all panels
void PanelSystemRender(PanelSystem *panel_system, void *nones_data)
{
    if (!panel_system || !panel_system->renderer) {
        return;
    }
    
    SDL_Renderer *renderer = panel_system->renderer;
    
    // Render each panel based on its type
    for (int i = 0; i < PANEL_POS_COUNT; i++) {
        Panel *panel = &panel_system->panels[i];
        if (!panel->visible) continue;
        
        switch (panel->type) {
            case PANEL_TYPE_GAME:
                PanelRenderGame(panel, renderer, nones_data);
                break;
            case PANEL_TYPE_MAP:
                PanelRenderMap(panel, renderer, nones_data);
                break;
            case PANEL_TYPE_STATUS:
                PanelRenderStatus(panel, renderer, nones_data);
                break;
            case PANEL_TYPE_EDITOR:
                PanelRenderEditor(panel, renderer, nones_data);
                break;
            default:
                break;
        }
    }
    
    // Render settings UI if visible
    if (panel_system->show_settings) {
        PanelRenderSettings(panel_system, renderer);
    }
}

// Render settings UI
void PanelRenderSettings(PanelSystem *panel_system, SDL_Renderer *renderer)
{
    // Create a semi-transparent overlay
    int window_w, window_h;
    SDL_GetWindowSize(panel_system->window, &window_w, &window_h);
    
    SDL_FRect overlay = {
        (float)(window_w / 4), 
        (float)(window_h / 4), 
        (float)(window_w / 2), 
        (float)(window_h / 2)
    };
    
    // Draw semi-transparent background
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    SDL_RenderFillRect(renderer, &overlay);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderRect(renderer, &overlay);
    
    // Draw title
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDebugText(renderer, overlay.x + 10, overlay.y + 10, "Panel Settings");
    
    // Draw panel configuration info
    int y_offset = 40;
    for (int i = 0; i < PANEL_POS_COUNT; i++) {
        char text[128];
        const char *pos_name = "";
        switch (i) {
            case PANEL_POS_TOP_LEFT: pos_name = "Top Left:"; break;
            case PANEL_POS_TOP_RIGHT: pos_name = "Top Right:"; break;
            case PANEL_POS_BOTTOM_LEFT: pos_name = "Bottom Left:"; break;
            case PANEL_POS_BOTTOM_RIGHT: pos_name = "Bottom Right:"; break;
        }
        
        snprintf(text, sizeof(text), "%s %s", pos_name, 
                PanelGetTypeName(panel_system->config.panels[i]));
        SDL_RenderDebugText(renderer, overlay.x + 10, overlay.y + y_offset, text);
        y_offset += 20;
    }
    
    // Draw instructions
    SDL_RenderDebugText(renderer, overlay.x + 10, overlay.y + overlay.h - 30, 
                       "Press F3 to close settings");
}
