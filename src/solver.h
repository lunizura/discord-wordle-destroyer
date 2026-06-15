#ifndef SOLVER_H
#define SOLVER_H

#include <stdbool.h>
#include <stdint.h>

#define WORD_LEN 5
#define MAX_WORDS 15000
#define NUM_PATTERNS 243 // 3^5 possible color combinations
#define MAX_RECOMMENDATIONS 10

typedef enum {
    MODE_NYT = 0,
    MODE_DISCORD = 1
} SolverMode;

typedef struct {
    char word[WORD_LEN + 1];
    float entropy;
    int expected_remaining;
    bool is_possible_answer;
} Recommendation;

typedef struct {
    char word[WORD_LEN + 1];
    uint32_t char_mask;
    bool active;
    bool is_answer; // true if in curated NYT answers list
} WordEntry;

typedef struct {
    WordEntry words[MAX_WORDS];
    int total_words;
    int active_count;
    SolverMode mode;
    Recommendation top_recs[MAX_RECOMMENDATIONS];
    int rec_count;
} SolverState;

// Initializes the solver state with embedded wordlists
void solver_init(SolverState *state, SolverMode mode);

// Resets active words and state back to initial dictionary
void solver_reset(SolverState *state);

// Changes dictionary mode (NYT vs Discord) and re-evaluates
void solver_set_mode(SolverState *state, SolverMode mode);

// Simulates Wordle feedback: 'G' (green), 'Y' (yellow), 'B' (gray)
void solver_get_feedback(const char *secret, const char *guess, char *out_feedback);

// Computes pattern index 0..242 from 5-char feedback string
int solver_pattern_index(const char *feedback);

// Checks if a candidate word is consistent with previous feedback
bool solver_is_consistent(const char *candidate, const char *guess, const char *feedback);

// Filters the active pool given a guess and feedback received
int solver_filter(SolverState *state, const char *guess, const char *feedback);

// Calculates Shannon Entropy and computes the top recommendations
void solver_calculate_recommendations(SolverState *state);

#endif // SOLVER_H
