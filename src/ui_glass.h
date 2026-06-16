#ifndef UI_GLASS_H
#define UI_GLASS_H

#include "raylib.h"

// Color Palette Tokens
#define COLOR_VOID_BG      (Color){ 11, 11, 30, 255 }     // Deepest indigo void #0B0B1E
#define COLOR_GLASS_PANEL  (Color){ 18, 24, 48, 175 }    // Translucent smoked glass
#define COLOR_GLASS_BORDER (Color){ 255, 255, 255, 40 }   // 1px specular glass edge
#define COLOR_GLASS_TOPLIP (Color){ 255, 255, 255, 75 }   // Top light-catching bevel
#define COLOR_TILE_EMPTY   (Color){ 255, 255, 255, 10 }   // Glass placeholder
#define COLOR_TILE_FILLED  (Color){ 255, 255, 255, 25 }   // Typed active letter
#define COLOR_TILE_GRAY    (Color){ 40, 48, 68, 220 }     // Absent slate
#define COLOR_TILE_YELLOW  (Color){ 217, 119, 6, 230 }    // Misplaced amber
#define COLOR_TILE_GREEN   (Color){ 16, 185, 129, 235 }   // Correct emerald
#define COLOR_TEXT_LIGHT   (Color){ 248, 250, 252, 255 }  // Crisp white
#define COLOR_TEXT_MUTED   (Color){ 148, 163, 184, 255 }  // Slate secondary

typedef struct {
    float x;
    float y;
    float radius;
    Color color;
    float speed_x;
    float speed_y;
    float phase;
} LightOrb;

typedef struct {
    LightOrb orbs[4];
    float global_time;
    Font font_title;
    Font font_board;
    Font font_ui;
} GlassRenderer;

void glass_init(GlassRenderer *gr);
void glass_update(GlassRenderer *gr, float dt);
void glass_draw_background(const GlassRenderer *gr, int screen_w, int screen_h);
void glass_draw_panel(Rectangle rect, float roundness, Color bg, Color border);
void glass_draw_tile(Rectangle rect, char letter, Color fill_color, float flip_progress, float scale_pop, bool is_current);
void glass_draw_button(Rectangle rect, const char *text, bool hovered, bool active, Color accent);

#endif // UI_GLASS_H
