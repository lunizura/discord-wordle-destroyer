#include "ui_board.h"
#include "audio_synth.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

void board_init(WordleBoard *b) {
    memset(b, 0, sizeof(WordleBoard));
    b->current_row = 0;
    b->current_col = 0;
    b->status = GAME_PLAYING;
    b->victory_timer = 0.0f;
    b->hovered_rec = -1;

    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            b->grid[r][c].letter = '\0';
            b->grid[r][c].color = TILE_COLOR_EMPTY;
        }
    }

    for (int i = 0; i < MAX_RECOMMENDATIONS; i++) {
        spring_init(&b->rec_lifts[i], 0.0f, 220.0f, 16.0f);
    }

    // Initialize layout rectangles (Screen size 1040 x 700)
    b->board_panel_rect = (Rectangle){ 40.0f, 85.0f, 480.0f, 575.0f };
    b->sidebar_panel_rect = (Rectangle){ 550.0f, 85.0f, 450.0f, 575.0f };

    b->mode_btn_rect = (Rectangle){ 820.0f, 25.0f, 180.0f, 38.0f };
    b->reset_btn_rect = (Rectangle){ 55.0f, 605.0f, 130.0f, 40.0f };
    b->fill_best_btn_rect = (Rectangle){ 195.0f, 605.0f, 140.0f, 40.0f };
    b->submit_btn_rect = (Rectangle){ 345.0f, 605.0f, 155.0f, 40.0f };

    glass_init(&b->glass);
    solver_init(&b->solver, MODE_NYT);
}

void board_update(WordleBoard *b, float dt) {
    glass_update(&b->glass, dt);

    if (b->status == GAME_SOLVED) {
        b->victory_timer += dt;
    }

    // Update tile animations
    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            BoardTile *tile = &b->grid[r][c];
            tween_update(&tile->flip_tween, dt);
            tween_update(&tile->pop_tween, dt);
        }
    }

    // Update recommendation spring physics
    Vector2 mouse = GetMousePosition();
    b->hovered_rec = -1;

    float rec_start_y = b->sidebar_panel_rect.y + 110.0f;
    float rec_h = 42.0f;
    float rec_gap = 6.0f;

    for (int i = 0; i < b->solver.rec_count && i < 8; i++) {
        Rectangle card_rect = (Rectangle){
            b->sidebar_panel_rect.x + 20.0f,
            rec_start_y + i * (rec_h + rec_gap),
            b->sidebar_panel_rect.width - 40.0f,
            rec_h
        };

        if (CheckCollisionPointRec(mouse, card_rect)) {
            b->hovered_rec = i;
            spring_set_target(&b->rec_lifts[i], -3.5f);
        } else {
            spring_set_target(&b->rec_lifts[i], 0.0f);
        }
        spring_update(&b->rec_lifts[i], dt);
    }
}

void board_type_char(WordleBoard *b, char c) {
    if (b->status != GAME_PLAYING) return;
    if (b->current_row >= BOARD_ROWS) return;

    if (b->current_col < BOARD_COLS) {
        BoardTile *tile = &b->grid[b->current_row][b->current_col];
        tile->letter = (char)toupper(c);
        tile->color = TILE_COLOR_GRAY; // Default to Gray when typed
        
        // Tactile scale pop on entry
        tween_start(&tile->pop_tween, 1.15f, 1.0f, 0.12f, 0.0f);
        audio_play_key_tap();
        
        b->current_col++;
    }
}

void board_backspace(WordleBoard *b) {
    if (b->status != GAME_PLAYING) return;
    if (b->current_row >= BOARD_ROWS) return;

    if (b->current_col > 0) {
        b->current_col--;
        BoardTile *tile = &b->grid[b->current_row][b->current_col];
        tile->letter = '\0';
        tile->color = TILE_COLOR_EMPTY;
        audio_play_key_tap();
    }
}

void board_cycle_tile_color(WordleBoard *b, int row, int col) {
    if (row < 0 || row >= BOARD_ROWS || col < 0 || col >= BOARD_COLS) return;
    BoardTile *tile = &b->grid[row][col];
    if (tile->letter == '\0') return;

    // Cycle: Gray (2) -> Yellow (3) -> Green (4) -> Gray (2)
    if (tile->color == TILE_COLOR_GRAY) {
        tile->color = TILE_COLOR_YELLOW;
    } else if (tile->color == TILE_COLOR_YELLOW) {
        tile->color = TILE_COLOR_GREEN;
    } else {
        tile->color = TILE_COLOR_GRAY;
    }

    tween_start(&tile->pop_tween, 1.12f, 1.0f, 0.10f, 0.0f);
    audio_play_tile_pop();
}

void board_fill_word(WordleBoard *b, const char *word) {
    if (b->status != GAME_PLAYING) return;
    if (b->current_row >= BOARD_ROWS) return;

    for (int c = 0; c < BOARD_COLS; c++) {
        BoardTile *tile = &b->grid[b->current_row][c];
        tile->letter = (char)toupper(word[c]);
        tile->color = TILE_COLOR_GRAY;
        tween_start(&tile->pop_tween, 1.15f, 1.0f, 0.12f, c * 0.03f);
    }
    b->current_col = BOARD_COLS;
    audio_play_key_tap();
}

void board_submit_row(WordleBoard *b) {
    if (b->status != GAME_PLAYING) return;
    if (b->current_row >= BOARD_ROWS) return;
    if (b->current_col < BOARD_COLS) return; // Row must be full

    // Extract guess and feedback string
    char guess[BOARD_COLS + 1];
    char feedback[BOARD_COLS + 1];
    int green_count = 0;

    for (int c = 0; c < BOARD_COLS; c++) {
        BoardTile *tile = &b->grid[b->current_row][c];
        guess[c] = (char)tolower(tile->letter);

        if (tile->color == TILE_COLOR_GREEN) {
            feedback[c] = 'G';
            green_count++;
        } else if (tile->color == TILE_COLOR_YELLOW) {
            feedback[c] = 'Y';
        } else {
            feedback[c] = 'B';
        }

        // Staggered 3D Flip animation (70ms stagger per tile)
        tween_start(&tile->flip_tween, 0.0f, 1.0f, 0.28f, c * 0.07f);
    }
    guess[BOARD_COLS] = '\0';
    feedback[BOARD_COLS] = '\0';

    audio_play_tile_flip();

    // Check Victory
    if (green_count == BOARD_COLS) {
        b->status = GAME_SOLVED;
        audio_play_victory_chime();
        return;
    }

    // Filter solver candidates
    solver_filter(&b->solver, guess, feedback);

    // Advance to next row
    b->current_row++;
    b->current_col = 0;

    if (b->current_row >= BOARD_ROWS && b->status != GAME_SOLVED) {
        b->status = GAME_FAILED;
    }
}

void board_reset(WordleBoard *b) {
    b->current_row = 0;
    b->current_col = 0;
    b->status = GAME_PLAYING;
    b->victory_timer = 0.0f;

    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            b->grid[r][c].letter = '\0';
            b->grid[r][c].color = TILE_COLOR_EMPTY;
            b->grid[r][c].flip_tween.active = false;
            b->grid[r][c].pop_tween.active = false;
        }
    }

    solver_reset(&b->solver);
    audio_play_tile_pop();
}

void board_toggle_mode(WordleBoard *b) {
    SolverMode new_mode = (b->solver.mode == MODE_NYT) ? MODE_DISCORD : MODE_NYT;
    solver_set_mode(&b->solver, new_mode);
    board_reset(b);
}

void board_handle_input(WordleBoard *b) {
    Vector2 mouse = GetMousePosition();
    bool mouse_clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    // 1. Keyboard text input
    int key = GetKeyPressed();
    while (key > 0) {
        if (key >= KEY_A && key <= KEY_Z) {
            board_type_char(b, (char)('A' + (key - KEY_A)));
        } else if (key == KEY_BACKSPACE) {
            board_backspace(b);
        } else if (key == KEY_ENTER || key == KEY_KP_ENTER) {
            board_submit_row(b);
        } else if (key == KEY_ESCAPE) {
            board_reset(b);
        }
        key = GetKeyPressed();
    }

    // 2. Mouse click on tiles to cycle color
    if (mouse_clicked) {
        float start_x = b->board_panel_rect.x + 35.0f;
        float start_y = b->board_panel_rect.y + 45.0f;
        float tile_size = 72.0f;
        float tile_gap = 10.0f;

        for (int r = 0; r < BOARD_ROWS; r++) {
            for (int c = 0; c < BOARD_COLS; c++) {
                Rectangle t_rect = (Rectangle){
                    start_x + c * (tile_size + tile_gap),
                    start_y + r * (tile_size + tile_gap),
                    tile_size,
                    tile_size
                };

                if (CheckCollisionPointRec(mouse, t_rect)) {
                    board_cycle_tile_color(b, r, c);
                }
            }
        }

        // Button clicks
        if (CheckCollisionPointRec(mouse, b->reset_btn_rect)) {
            board_reset(b);
        } else if (CheckCollisionPointRec(mouse, b->mode_btn_rect)) {
            board_toggle_mode(b);
        } else if (CheckCollisionPointRec(mouse, b->fill_best_btn_rect)) {
            if (b->solver.rec_count > 0) {
                board_fill_word(b, b->solver.top_recs[0].word);
            }
        } else if (CheckCollisionPointRec(mouse, b->submit_btn_rect)) {
            board_submit_row(b);
        }

        // Sidebar card clicks (auto-fill selected recommendation)
        float rec_start_y = b->sidebar_panel_rect.y + 110.0f;
        float rec_h = 42.0f;
        float rec_gap = 6.0f;
        for (int i = 0; i < b->solver.rec_count && i < 8; i++) {
            Rectangle card_rect = (Rectangle){
                b->sidebar_panel_rect.x + 20.0f,
                rec_start_y + i * (rec_h + rec_gap),
                b->sidebar_panel_rect.width - 40.0f,
                rec_h
            };
            if (CheckCollisionPointRec(mouse, card_rect)) {
                board_fill_word(b, b->solver.top_recs[i].word);
            }
        }
    }
}

void board_draw(WordleBoard *b, int screen_w, int screen_h) {
    // 1. Ambient Background & Drifting Orbs
    glass_draw_background(&b->glass, screen_w, screen_h);

    Vector2 mouse = GetMousePosition();

    // 2. Header Bar
    if (b->icon_texture.id > 0) {
        Rectangle src = { 0.0f, 0.0f, (float)b->icon_texture.width, (float)b->icon_texture.height };
        Rectangle dest = { 40.0f, 20.0f, 38.0f, 38.0f };
        DrawTexturePro(b->icon_texture, src, dest, (Vector2){ 0, 0 }, 0.0f, WHITE);
    }
    DrawText("WORDLE PRISM", 88, 26, 26, COLOR_TEXT_LIGHT);
    DrawText("C99 INTELLIGENCE ENGINE", 295, 34, 13, (Color){ 168, 85, 247, 230 });

    // Mode Toggle Button
    const char *mode_str = (b->solver.mode == MODE_NYT) ? "MODE: NYT DAILY" : "MODE: DISCORD WIDE";
    bool mode_hov = CheckCollisionPointRec(mouse, b->mode_btn_rect);
    Color mode_accent = (b->solver.mode == MODE_NYT) ? (Color){ 16, 185, 129, 255 } : (Color){ 6, 182, 212, 255 };
    glass_draw_button(b->mode_btn_rect, mode_str, mode_hov, false, mode_accent);

    // 3. Left Panel: Wordle Board Glass Cockpit
    glass_draw_panel(b->board_panel_rect, 0.05f, COLOR_GLASS_PANEL, COLOR_GLASS_BORDER);

    // Board Tiles
    float start_x = b->board_panel_rect.x + 40.0f;
    float start_y = b->board_panel_rect.y + 40.0f;
    float tile_size = 70.0f;
    float tile_gap = 10.0f;

    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            BoardTile *tile = &b->grid[r][c];
            Rectangle rect = (Rectangle){
                start_x + c * (tile_size + tile_gap),
                start_y + r * (tile_size + tile_gap),
                tile_size,
                tile_size
            };

            Color fill = COLOR_TILE_EMPTY;
            if (tile->color == TILE_COLOR_FILLED) fill = COLOR_TILE_FILLED;
            else if (tile->color == TILE_COLOR_GRAY) fill = COLOR_TILE_GRAY;
            else if (tile->color == TILE_COLOR_YELLOW) fill = COLOR_TILE_YELLOW;
            else if (tile->color == TILE_COLOR_GREEN) fill = COLOR_TILE_GREEN;

            float flip_p = tile->flip_tween.active ? tween_get_value_cubic(&tile->flip_tween) : 0.0f;
            float pop_s = tile->pop_tween.active ? tween_get_value_pop(&tile->pop_tween) : 1.0f;
            bool is_curr = (r == b->current_row && c == b->current_col && b->status == GAME_PLAYING);

            glass_draw_tile(rect, tile->letter, fill, flip_p, pop_s, is_curr);
        }
    }

    // Board Control Buttons
    bool reset_hov = CheckCollisionPointRec(mouse, b->reset_btn_rect);
    glass_draw_button(b->reset_btn_rect, "RESET [ESC]", reset_hov, false, (Color){ 239, 68, 68, 255 });

    bool fill_hov = CheckCollisionPointRec(mouse, b->fill_best_btn_rect);
    glass_draw_button(b->fill_best_btn_rect, "USE TOP GUESS", fill_hov, false, (Color){ 6, 182, 212, 255 });

    bool submit_hov = CheckCollisionPointRec(mouse, b->submit_btn_rect);
    bool row_full = (b->current_col == BOARD_COLS && b->status == GAME_PLAYING);
    Color submit_accent = row_full ? (Color){ 16, 185, 129, 255 } : (Color){ 148, 163, 184, 180 };
    glass_draw_button(b->submit_btn_rect, "SUBMIT GUESS [↵]", submit_hov, row_full, submit_accent);

    // 4. Right Panel: Recommendations & Analytics Sidebar
    glass_draw_panel(b->sidebar_panel_rect, 0.05f, COLOR_GLASS_PANEL, COLOR_GLASS_BORDER);

    DrawText("OPTIMAL RECOMMENDATIONS", (int)(b->sidebar_panel_rect.x + 22.0f), (int)(b->sidebar_panel_rect.y + 22.0f), 18, COLOR_TEXT_LIGHT);
    DrawText("Ranked by Shannon Entropy (Information Gain)", (int)(b->sidebar_panel_rect.x + 22.0f), (int)(b->sidebar_panel_rect.y + 46.0f), 12, COLOR_TEXT_MUTED);

    // Remaining Words Badge
    char pool_str[64];
    snprintf(pool_str, sizeof(pool_str), "REMAINING: %d WORDS", b->solver.active_count);
    DrawText(pool_str, (int)(b->sidebar_panel_rect.x + 22.0f), (int)(b->sidebar_panel_rect.y + 75.0f), 14, (Color){ 6, 182, 212, 255 });

    // Recommendation Cards
    float rec_start_y = b->sidebar_panel_rect.y + 105.0f;
    float rec_h = 44.0f;
    float rec_gap = 7.0f;

    for (int i = 0; i < b->solver.rec_count && i < 8; i++) {
        const Recommendation *rec = &b->solver.top_recs[i];
        float lift = b->rec_lifts[i].value;

        Rectangle card = (Rectangle){
            b->sidebar_panel_rect.x + 20.0f,
            rec_start_y + i * (rec_h + rec_gap) + lift,
            b->sidebar_panel_rect.width - 40.0f,
            rec_h
        };

        bool hovered = (b->hovered_rec == i);
        Color card_bg = hovered ? (Color){ 255, 255, 255, 30 } : (Color){ 255, 255, 255, 12 };
        Color card_border = hovered ? (Color){ 168, 85, 247, 220 } : COLOR_GLASS_BORDER;

        glass_draw_panel(card, 0.20f, card_bg, card_border);

        // Word text
        char upper_word[8];
        for (int c = 0; c < 5; c++) upper_word[c] = (char)toupper(rec->word[c]);
        upper_word[5] = '\0';

        char rank_word[16];
        snprintf(rank_word, sizeof(rank_word), "%d. %s", i + 1, upper_word);
        DrawText(rank_word, (int)(card.x + 16.0f), (int)(card.y + 13.0f), 17, COLOR_TEXT_LIGHT);

        // Entropy score
        char ent_str[32];
        snprintf(ent_str, sizeof(ent_str), "%.2f bits", rec->entropy);
        int ew = MeasureText(ent_str, 13);
        DrawText(ent_str, (int)(card.x + card.width - ew - 16.0f), (int)(card.y + 15.0f), 13, (Color){ 245, 158, 11, 240 });

        // Expected pool reduction
        char exp_str[32];
        snprintf(exp_str, sizeof(exp_str), "~%d left", rec->expected_remaining);
        DrawText(exp_str, (int)(card.x + card.width - ew - 75.0f), (int)(card.y + 16.0f), 11, COLOR_TEXT_MUTED);
    }

    // Bottom Guide / Instructions
    DrawText("TIP: Click tiles to cycle color (Gray -> Yellow -> Green)", (int)(b->sidebar_panel_rect.x + 22.0f), (int)(b->sidebar_panel_rect.y + 515.0f), 12, COLOR_TEXT_MUTED);
    DrawText("Press [ENTER] to lock guess. Click any recommendation to auto-fill.", (int)(b->sidebar_panel_rect.x + 22.0f), (int)(b->sidebar_panel_rect.y + 535.0f), 12, (Color){ 148, 163, 184, 200 });

    // Victory celebratory banner if solved
    if (b->status == GAME_SOLVED) {
        Rectangle banner = (Rectangle){
            b->board_panel_rect.x + 20.0f,
            b->board_panel_rect.y + b->board_panel_rect.height * 0.40f,
            b->board_panel_rect.width - 40.0f,
            90.0f
        };
        glass_draw_panel(banner, 0.20f, (Color){ 16, 185, 129, 230 }, (Color){ 255, 255, 255, 180 });

        const char *vic_title = "PUZZLE SOLVED!";
        int vtw = MeasureText(vic_title, 26);
        DrawText(vic_title, (int)(banner.x + (banner.width - vtw) * 0.5f), (int)(banner.y + 20.0f), 26, COLOR_TEXT_LIGHT);

        char vic_sub[64];
        snprintf(vic_sub, sizeof(vic_sub), "Solved in %d turn%s! Press ESC to restart.", b->current_row + 1, (b->current_row == 0) ? "" : "s");
        int vsw = MeasureText(vic_sub, 14);
        DrawText(vic_sub, (int)(banner.x + (banner.width - vsw) * 0.5f), (int)(banner.y + 55.0f), 14, (Color){ 240, 253, 244, 255 });
    }
}
