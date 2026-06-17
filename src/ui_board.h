#ifndef UI_BOARD_H
#define UI_BOARD_H

#include "raylib.h"
#include "solver.h"
#include "motion.h"
#include "ui_glass.h"

#define BOARD_ROWS 6
#define BOARD_COLS 5

typedef enum {
    TILE_COLOR_EMPTY = 0,
    TILE_COLOR_FILLED = 1,
    TILE_COLOR_GRAY = 2,
    TILE_COLOR_YELLOW = 3,
    TILE_COLOR_GREEN = 4
} TileColor;

typedef struct {
    char letter;
    TileColor color;
    Tween flip_tween;
    Tween pop_tween;
} BoardTile;

typedef enum {
    GAME_PLAYING = 0,
    GAME_SOLVED = 1,
    GAME_FAILED = 2
} GameStatus;

typedef struct {
    BoardTile grid[BOARD_ROWS][BOARD_COLS];
    int current_row;
    int current_col;
    GameStatus status;
    SolverState solver;
    GlassRenderer glass;
    Texture2D icon_texture;
    
    // Sidebar recommendation hover springs
    Spring rec_lifts[MAX_RECOMMENDATIONS];
    int hovered_rec;
    
    // UI bounds
    Rectangle board_panel_rect;
    Rectangle sidebar_panel_rect;
    Rectangle reset_btn_rect;
    Rectangle mode_btn_rect;
    Rectangle fill_best_btn_rect;
    Rectangle submit_btn_rect;

    // Victory celebratory particles
    float victory_timer;
} WordleBoard;

void board_init(WordleBoard *b);
void board_update(WordleBoard *b, float dt);
void board_draw(WordleBoard *b, int screen_w, int screen_h);

void board_handle_input(WordleBoard *b);
void board_type_char(WordleBoard *b, char c);
void board_backspace(WordleBoard *b);
void board_cycle_tile_color(WordleBoard *b, int row, int col);
void board_submit_row(WordleBoard *b);
void board_reset(WordleBoard *b);
void board_toggle_mode(WordleBoard *b);
void board_fill_word(WordleBoard *b, const char *word);

#endif // UI_BOARD_H
