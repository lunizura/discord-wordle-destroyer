#include "ui_glass.h"
#include <math.h>
#include <stdio.h>

void glass_init(GlassRenderer *gr) {
    gr->global_time = 0.0f;

    // Orb 1: Violet/Magenta
    gr->orbs[0] = (LightOrb){
        .x = 240.0f, .y = 180.0f, .radius = 280.0f,
        .color = (Color){ 168, 85, 247, 55 },
        .speed_x = 0.35f, .speed_y = 0.45f, .phase = 0.0f
    };

    // Orb 2: Luminous Cyan
    gr->orbs[1] = (LightOrb){
        .x = 800.0f, .y = 260.0f, .radius = 320.0f,
        .color = (Color){ 6, 182, 212, 45 },
        .speed_x = -0.40f, .speed_y = 0.30f, .phase = 1.57f
    };

    // Orb 3: Warm Golden Amber
    gr->orbs[2] = (LightOrb){
        .x = 520.0f, .y = 520.0f, .radius = 240.0f,
        .color = (Color){ 245, 158, 11, 35 },
        .speed_x = 0.25f, .speed_y = -0.35f, .phase = 3.14f
    };

    // Orb 4: Deep Indigo Bloom
    gr->orbs[3] = (LightOrb){
        .x = 920.0f, .y = 620.0f, .radius = 300.0f,
        .color = (Color){ 99, 102, 241, 40 },
        .speed_x = -0.30f, .speed_y = -0.25f, .phase = 4.71f
    };
}

void glass_update(GlassRenderer *gr, float dt) {
    gr->global_time += dt;

    // Drifting Lissajous curves for ambient fluid motion
    for (int i = 0; i < 4; i++) {
        LightOrb *orb = &gr->orbs[i];
        float t = gr->global_time;
        orb->x += sinf(t * orb->speed_x + orb->phase) * 0.75f;
        orb->y += cosf(t * orb->speed_y + orb->phase) * 0.65f;
    }
}

void glass_draw_background(const GlassRenderer *gr, int screen_w, int screen_h) {
    // 1. Base deep void
    ClearBackground(COLOR_VOID_BG);

    // 2. Subtle gradient wash across entire background
    DrawRectangleGradientV(0, 0, screen_w, screen_h, 
                           (Color){ 14, 16, 42, 255 }, 
                           (Color){ 7, 8, 20, 255 });

    // 3. Ambient Chromatic Light Orbs with soft radial falloff
    for (int i = 0; i < 4; i++) {
        const LightOrb *orb = &gr->orbs[i];
        DrawCircleGradient((Vector2){ orb->x, orb->y }, orb->radius, orb->color, (Color){ 0, 0, 0, 0 });
    }
}

void glass_draw_panel(Rectangle rect, float roundness, Color bg, Color border) {
    // Soft drop shadow
    Rectangle shadow_rect = (Rectangle){ rect.x + 3.0f, rect.y + 6.0f, rect.width, rect.height };
    DrawRectangleRounded(shadow_rect, roundness, 8, (Color){ 0, 0, 0, 65 });

    // Translucent glass body
    DrawRectangleRounded(rect, roundness, 8, bg);

    // Outer 1px specular border
    DrawRectangleRoundedLines(rect, roundness, 8, border);

    // Top specular highlight edge (light catches the top bevel)
    Rectangle top_line = (Rectangle){ rect.x + rect.width * 0.1f, rect.y + 1.0f, rect.width * 0.8f, 1.0f };
    DrawRectangleRounded(top_line, 0.5f, 4, COLOR_GLASS_TOPLIP);
}

void glass_draw_tile(Rectangle rect, char letter, Color fill_color, float flip_progress, float scale_pop, bool is_current) {
    // Calculate 3D perspective projection during flip
    // Progress 0.0 -> 1.0: tile shrinks along X to 0 at 0.5, then expands with new color
    float flip_scale_x = 1.0f;
    Color active_fill = fill_color;

    if (flip_progress > 0.0f && flip_progress < 1.0f) {
        float angle = flip_progress * PI;
        flip_scale_x = fabsf(cosf(angle));
        if (flip_progress < 0.5f) {
            // First half of flip shows previous state
            active_fill = COLOR_TILE_FILLED;
        }
    }

    // Apply scale pop on click/entry
    float width = rect.width * flip_scale_x * scale_pop;
    float height = rect.height * scale_pop;
    float center_x = rect.x + rect.width * 0.5f;
    float center_y = rect.y + rect.height * 0.5f;

    Rectangle target = (Rectangle){
        center_x - width * 0.5f,
        center_y - height * 0.5f,
        width,
        height
    };

    // Ambient tile shadow
    Rectangle shadow = (Rectangle){ target.x + 1.0f, target.y + 3.0f, target.width, target.height };
    DrawRectangleRounded(shadow, 0.18f, 6, (Color){ 0, 0, 0, 50 });

    // Draw tile body
    DrawRectangleRounded(target, 0.18f, 6, active_fill);

    // Border highlight
    Color border_color = is_current ? (Color){ 147, 197, 253, 140 } : COLOR_GLASS_BORDER;
    DrawRectangleRoundedLines(target, 0.18f, 6, border_color);

    // Specular sheen flash at the apex of flip
    if (flip_progress > 0.40f && flip_progress < 0.60f) {
        float flash_alpha = 1.0f - fabsf(flip_progress - 0.5f) * 10.0f;
        if (flash_alpha > 0.0f) {
            DrawRectangleRounded(target, 0.18f, 6, (Color){ 255, 255, 255, (unsigned char)(flash_alpha * 90) });
        }
    }

    // Letter rendering
    if (letter != '\0' && width > 12.0f) {
        char str[2] = { letter, '\0' };
        int font_size = (int)(height * 0.52f);
        int text_w = MeasureText(str, font_size);
        int text_x = (int)(center_x - text_w * 0.5f);
        int text_y = (int)(center_y - font_size * 0.5f);

        // Subtle text drop shadow
        DrawText(str, text_x + 1, text_y + 2, font_size, (Color){ 0, 0, 0, 100 });
        // Crisp white text
        DrawText(str, text_x, text_y, font_size, COLOR_TEXT_LIGHT);
    }
}

void glass_draw_button(Rectangle rect, const char *text, bool hovered, bool active, Color accent) {
    Color bg = active ? (Color){ accent.r, accent.g, accent.b, 180 } :
               hovered ? (Color){ 255, 255, 255, 28 } :
                         (Color){ 255, 255, 255, 12 };

    Color border = hovered ? (Color){ accent.r, accent.g, accent.b, 200 } : COLOR_GLASS_BORDER;

    glass_draw_panel(rect, 0.25f, bg, border);

    int font_size = 14;
    int tw = MeasureText(text, font_size);
    int tx = (int)(rect.x + (rect.width - tw) * 0.5f);
    int ty = (int)(rect.y + (rect.height - font_size) * 0.5f);

    Color tc = active ? COLOR_TEXT_LIGHT : (hovered ? (Color){ 255, 255, 255, 255 } : COLOR_TEXT_MUTED);
    DrawText(text, tx, ty, font_size, tc);
}
