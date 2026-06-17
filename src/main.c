#include "raylib.h"
#include "audio_synth.h"
#include "ui_board.h"
#include "icon_data.h"

#define WINDOW_WIDTH  1040
#define WINDOW_HEIGHT 700

int main(void) {
    // Enable anti-aliasing and V-Sync for butter-smooth rendering
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Wordle Prism - Desktop C Engine");
    SetTargetFPS(120);

    // Set pixel art prism icon
    Image icon_img = {
        .data = (void *)PRISM_ICON_PIXELS,
        .width = PRISM_ICON_WIDTH,
        .height = PRISM_ICON_HEIGHT,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };
    SetWindowIcon(icon_img);

    // Initialize procedural micro-audio engine
    audio_init();

    // Initialize Wordle board & solver state
    WordleBoard board;
    board_init(&board);
    board.icon_texture = LoadTextureFromImage(icon_img);

    // Main Engine Loop
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (dt > 0.05f) dt = 0.05f; // Frame clamping for smooth animations

        // 1. Process Input
        board_handle_input(&board);

        // 2. Update Physics & Tweens
        board_update(&board, dt);

        // 3. Render Glass UI
        BeginDrawing();
        board_draw(&board, WINDOW_WIDTH, WINDOW_HEIGHT);
        EndDrawing();
    }

    // Clean up
    UnloadTexture(board.icon_texture);
    audio_close();
    CloseWindow();

    return 0;
}
