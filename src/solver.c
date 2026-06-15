#include "solver.h"
#include "words.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>

void solver_init(SolverState *state, SolverMode mode) {
    state->mode = mode;
    state->total_words = 0;
    state->active_count = 0;
    state->rec_count = 0;

    int count = (mode == MODE_NYT) ? NYT_WORD_COUNT : TOTAL_WORD_COUNT;
    const char **dict = (mode == MODE_NYT) ? NYT_WORDS : TOTAL_WORDS;

    for (int i = 0; i < count && i < MAX_WORDS; i++) {
        strncpy(state->words[i].word, dict[i], WORD_LEN);
        state->words[i].word[WORD_LEN] = '\0';
        
        uint32_t mask = 0;
        for (int c = 0; c < WORD_LEN; c++) {
            mask |= (1 << (state->words[i].word[c] - 'a'));
        }
        state->words[i].char_mask = mask;
        state->words[i].active = true;
        state->words[i].is_answer = (mode == MODE_NYT) ? true : false;
        state->total_words++;
    }
    state->active_count = state->total_words;

    solver_calculate_recommendations(state);
}

void solver_reset(SolverState *state) {
    for (int i = 0; i < state->total_words; i++) {
        state->words[i].active = true;
    }
    state->active_count = state->total_words;
    solver_calculate_recommendations(state);
}

void solver_set_mode(SolverState *state, SolverMode mode) {
    if (state->mode != mode) {
        solver_init(state, mode);
    }
}

void solver_get_feedback(const char *secret, const char *guess, char *out_feedback) {
    bool used_s[WORD_LEN] = {false};
    bool used_g[WORD_LEN] = {false};

    for (int i = 0; i < WORD_LEN; i++) {
        out_feedback[i] = 'B';
    }

    // Pass 1: Greens (Exact matches)
    for (int i = 0; i < WORD_LEN; i++) {
        if (guess[i] == secret[i]) {
            out_feedback[i] = 'G';
            used_s[i] = true;
            used_g[i] = true;
        }
    }

    // Pass 2: Yellows (Misplaced matches)
    for (int i = 0; i < WORD_LEN; i++) {
        if (used_g[i]) continue;
        for (int j = 0; j < WORD_LEN; j++) {
            if (!used_s[j] && guess[i] == secret[j]) {
                out_feedback[i] = 'Y';
                used_s[j] = true;
                break;
            }
        }
    }
    out_feedback[WORD_LEN] = '\0';
}

int solver_pattern_index(const char *feedback) {
    int idx = 0;
    int multiplier = 1;
    for (int i = 0; i < WORD_LEN; i++) {
        int val = 0; // 'B'
        if (feedback[i] == 'Y' || feedback[i] == 'y') val = 1;
        else if (feedback[i] == 'G' || feedback[i] == 'g') val = 2;
        idx += val * multiplier;
        multiplier *= 3;
    }
    return idx;
}

bool solver_is_consistent(const char *candidate, const char *guess, const char *feedback) {
    char simulated[WORD_LEN + 1];
    solver_get_feedback(candidate, guess, simulated);
    return strcmp(simulated, feedback) == 0;
}

int solver_filter(SolverState *state, const char *guess, const char *feedback) {
    int count = 0;
    for (int i = 0; i < state->total_words; i++) {
        if (!state->words[i].active) continue;

        if (!solver_is_consistent(state->words[i].word, guess, feedback)) {
            state->words[i].active = false;
        } else {
            count++;
        }
    }
    state->active_count = count;
    solver_calculate_recommendations(state);
    return count;
}

// Comparison function for sorting recommendations by entropy descending
static int compare_recs(const void *a, const void *b) {
    const Recommendation *ra = (const Recommendation *)a;
    const Recommendation *rb = (const Recommendation *)b;
    
    // Primary sort: Entropy descending
    if (rb->entropy > ra->entropy + 0.001f) return 1;
    if (ra->entropy > rb->entropy + 0.001f) return -1;
    
    // Secondary sort: Prefer possible answers
    if (rb->is_possible_answer && !ra->is_possible_answer) return 1;
    if (ra->is_possible_answer && !rb->is_possible_answer) return -1;

    // Tertiary: Smaller expected remaining pool
    return ra->expected_remaining - rb->expected_remaining;
}

void solver_calculate_recommendations(SolverState *state) {
    state->rec_count = 0;
    if (state->active_count == 0) return;

    // If only 1 or 2 words remain, they are the obvious answers
    if (state->active_count <= 2) {
        for (int i = 0; i < state->total_words; i++) {
            if (state->words[i].active) {
                Recommendation *r = &state->top_recs[state->rec_count++];
                strncpy(r->word, state->words[i].word, WORD_LEN);
                r->word[WORD_LEN] = '\0';
                r->entropy = (state->active_count == 1) ? 0.0f : 1.0f;
                r->expected_remaining = 1;
                r->is_possible_answer = true;
                if (state->rec_count >= MAX_RECOMMENDATIONS) break;
            }
        }
        return;
    }

    // Default top openers cache for Round 1 to ensure instant startup
    if (state->active_count == state->total_words) {
        const char *openers[] = {"soare", "roate", "raise", "slate", "crane", "salet", "trace", "adieu", "audio", "stare"};
        float entropies[]     = { 5.89f,   5.88f,   5.87f,   5.86f,   5.84f,   5.83f,   5.82f,   5.76f,   5.75f,   5.74f};
        int remaining[]       = {    62,      63,      64,      65,      66,      67,      69,      74,      75,      76};

        for (int i = 0; i < 10; i++) {
            Recommendation *r = &state->top_recs[state->rec_count++];
            strncpy(r->word, openers[i], WORD_LEN);
            r->word[WORD_LEN] = '\0';
            r->entropy = entropies[i];
            r->expected_remaining = remaining[i];
            r->is_possible_answer = true;
        }
        return;
    }

    // For rounds 2-6: Calculate Shannon Entropy across active pool
    Recommendation candidate_list[MAX_WORDS];
    int candidate_count = 0;

    // Evaluate all active words
    for (int i = 0; i < state->total_words; i++) {
        if (!state->words[i].active) continue;

        int pattern_counts[NUM_PATTERNS] = {0};
        const char *guess = state->words[i].word;

        // Group active words into 243 pattern buckets
        for (int j = 0; j < state->total_words; j++) {
            if (!state->words[j].active) continue;
            char fb[WORD_LEN + 1];
            solver_get_feedback(state->words[j].word, guess, fb);
            pattern_counts[solver_pattern_index(fb)]++;
        }

        // Calculate Shannon Entropy: H = - sum( p * log2(p) )
        float entropy = 0.0f;
        int expected_rem_sum = 0;
        float total_f = (float)state->active_count;

        for (int p = 0; p < NUM_PATTERNS; p++) {
            if (pattern_counts[p] > 0) {
                float p_prob = (float)pattern_counts[p] / total_f;
                entropy -= p_prob * (logf(p_prob) / 0.69314718f); // log2(x) = ln(x) / ln(2)
                expected_rem_sum += pattern_counts[p] * pattern_counts[p];
            }
        }

        Recommendation *cand = &candidate_list[candidate_count++];
        strncpy(cand->word, guess, WORD_LEN);
        cand->word[WORD_LEN] = '\0';
        cand->entropy = entropy;
        cand->expected_remaining = (int)roundf((float)expected_rem_sum / total_f);
        cand->is_possible_answer = true;

        // Safety limit for candidate pool evaluation
        if (candidate_count >= 500) break;
    }

    // Sort candidates by entropy
    qsort(candidate_list, candidate_count, sizeof(Recommendation), compare_recs);

    // Pick top recommendations
    int take = (candidate_count < MAX_RECOMMENDATIONS) ? candidate_count : MAX_RECOMMENDATIONS;
    for (int i = 0; i < take; i++) {
        state->top_recs[state->rec_count++] = candidate_list[i];
    }
}
