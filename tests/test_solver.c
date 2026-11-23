#include "solver.h"
#include "words.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_tests_run = 0;
static int g_tests_failed = 0;

#define ASSERT_TRUE(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "[FAIL] %s:%d: assertion failed: %s\n", __FILE__, __LINE__, #cond); \
        g_tests_failed++; \
        return; \
    } \
} while (0)

#define ASSERT_INT_EQ(expected, actual) do { \
    int _e = (expected); \
    int _a = (actual); \
    if (_e != _a) { \
        fprintf(stderr, "[FAIL] %s:%d: expected %d, got %d (%s)\n", __FILE__, __LINE__, _e, _a, #actual); \
        g_tests_failed++; \
        return; \
    } \
} while (0)

#define ASSERT_STR_EQ(expected, actual) do { \
    const char *_e = (expected); \
    const char *_a = (actual); \
    if (strcmp(_e, _a) != 0) { \
        fprintf(stderr, "[FAIL] %s:%d: expected \"%s\", got \"%s\" (%s)\n", __FILE__, __LINE__, _e, _a, #actual); \
        g_tests_failed++; \
        return; \
    } \
} while (0)

#define RUN_TEST(fn) do { \
    int _before = g_tests_failed; \
    g_tests_run++; \
    fn(); \
    if (g_tests_failed == _before) { \
        printf("[PASS] %s\n", #fn); \
    } \
} while (0)

static bool is_valid_lowercase_word(const char *w) {
    if (w == NULL || strlen(w) != WORD_LEN) {
        return false;
    }
    for (int i = 0; i < WORD_LEN; i++) {
        if (w[i] < 'a' || w[i] > 'z') {
            return false;
        }
    }
    return true;
}

static int cmp_word_ptr(const void *a, const void *b) {
    const char *const *pa = (const char *const *)a;
    const char *const *pb = (const char *const *)b;
    return strcmp(*pa, *pb);
}

static void assert_unique_wordlist(const char **words, int count) {
    const char **copy = (const char **)malloc((size_t)count * sizeof(const char *));
    ASSERT_TRUE(copy != NULL);
    for (int i = 0; i < count; i++) {
        copy[i] = words[i];
    }
    qsort(copy, (size_t)count, sizeof(const char *), cmp_word_ptr);
    for (int i = 1; i < count; i++) {
        if (strcmp(copy[i - 1], copy[i]) == 0) {
            fprintf(stderr, "[FAIL] duplicate word in wordlist: \"%s\"\n", copy[i]);
            g_tests_failed++;
            free(copy);
            return;
        }
    }
    free(copy);
}

static void test_wordlists_integrity(void) {
    ASSERT_INT_EQ(2315, NYT_WORD_COUNT);
    ASSERT_INT_EQ(14855, TOTAL_WORD_COUNT);

    for (int i = 0; i < NYT_WORD_COUNT; i++) {
        ASSERT_TRUE(is_valid_lowercase_word(NYT_WORDS[i]));
    }
    for (int i = 0; i < TOTAL_WORD_COUNT; i++) {
        ASSERT_TRUE(is_valid_lowercase_word(TOTAL_WORDS[i]));
    }

    assert_unique_wordlist(NYT_WORDS, NYT_WORD_COUNT);
    assert_unique_wordlist(TOTAL_WORDS, TOTAL_WORD_COUNT);
}

static void test_feedback_exact_and_duplicates(void) {
    char fb[WORD_LEN + 1];

    solver_get_feedback("crane", "crane", fb);
    ASSERT_STR_EQ("GGGGG", fb);

    solver_get_feedback("crane", "plumy", fb);
    ASSERT_STR_EQ("BBBBB", fb);

    solver_get_feedback("eerie", "speed", fb);
    ASSERT_STR_EQ("BBYYB", fb);

    solver_get_feedback("eerie", "tepee", fb);
    ASSERT_STR_EQ("BGBYG", fb);

    solver_get_feedback("speed", "steep", fb);
    ASSERT_STR_EQ("GBGGY", fb);

    solver_get_feedback("babes", "abbey", fb);
    ASSERT_STR_EQ("YYGGB", fb);

    solver_get_feedback("apple", "ppppp", fb);
    ASSERT_STR_EQ("BGGBB", fb);

    solver_get_feedback("robot", "otter", fb);
    ASSERT_STR_EQ("YYBBY", fb);

    solver_get_feedback("alloy", "llama", fb);
    ASSERT_STR_EQ("YGYBB", fb);

    solver_get_feedback("llama", "ababa", fb);
    ASSERT_STR_EQ("BBGBG", fb);

    solver_get_feedback("llama", "aabba", fb);
    ASSERT_STR_EQ("YBBBG", fb);

    solver_get_feedback("mamma", "maxim", fb);
    ASSERT_STR_EQ("GGBBY", fb);
}

static void test_pattern_index_bijection(void) {
    ASSERT_INT_EQ(0, solver_pattern_index("BBBBB"));
    ASSERT_INT_EQ(242, solver_pattern_index("GGGGG"));
    ASSERT_INT_EQ(1, solver_pattern_index("YBBBB"));
    ASSERT_INT_EQ(3, solver_pattern_index("BYBBB"));
    ASSERT_INT_EQ(9, solver_pattern_index("BBYBB"));
    ASSERT_INT_EQ(27, solver_pattern_index("BBBYB"));
    ASSERT_INT_EQ(81, solver_pattern_index("BBBBY"));

    /* Lowercase input compatibility */
    ASSERT_INT_EQ(0, solver_pattern_index("bbbbb"));
    ASSERT_INT_EQ(solver_pattern_index("GYBYG"), solver_pattern_index("gybyg"));

    bool seen[NUM_PATTERNS] = {false};
    const char chars[3] = {'B', 'Y', 'G'};
    char fb[WORD_LEN + 1];
    fb[WORD_LEN] = '\0';

    for (int a = 0; a < 3; a++) {
        for (int b = 0; b < 3; b++) {
            for (int c = 0; c < 3; c++) {
                for (int d = 0; d < 3; d++) {
                    for (int e = 0; e < 3; e++) {
                        fb[0] = chars[a];
                        fb[1] = chars[b];
                        fb[2] = chars[c];
                        fb[3] = chars[d];
                        fb[4] = chars[e];
                        int idx = solver_pattern_index(fb);
                        ASSERT_TRUE(idx >= 0 && idx < NUM_PATTERNS);
                        ASSERT_TRUE(!seen[idx]);
                        seen[idx] = true;
                    }
                }
            }
        }
    }

    for (int i = 0; i < NUM_PATTERNS; i++) {
        ASSERT_TRUE(seen[i]);
    }
}

static void test_consistency_checks(void) {
    ASSERT_TRUE(solver_is_consistent("babes", "abbey", "YYGGB"));
    ASSERT_TRUE(!solver_is_consistent("babes", "abbey", "YYYYB"));
    ASSERT_TRUE(solver_is_consistent("eerie", "speed", "BBYYB"));
    ASSERT_TRUE(!solver_is_consistent("eerie", "speed", "BBGYB"));
    ASSERT_TRUE(solver_is_consistent("eerie", "tepee", "BGBYG"));
    ASSERT_TRUE(!solver_is_consistent("paper", "speed", "BBYYB"));
    ASSERT_TRUE(solver_is_consistent("crane", "crane", "GGGGG"));
}

static void test_nyt_filter_recommendations_and_reset(void) {
    SolverState *state = (SolverState *)calloc(1, sizeof(SolverState));
    ASSERT_TRUE(state != NULL);

    solver_init(state, MODE_NYT);
    ASSERT_INT_EQ(MODE_NYT, state->mode);
    ASSERT_INT_EQ(NYT_WORD_COUNT, state->total_words);
    ASSERT_INT_EQ(NYT_WORD_COUNT, state->active_count);
    ASSERT_INT_EQ(MAX_RECOMMENDATIONS, state->rec_count);

    /* Same-mode call should preserve state */
    int rem = solver_filter(state, "crane", "BBBBB");
    ASSERT_TRUE(rem > 0 && rem < NYT_WORD_COUNT);
    ASSERT_INT_EQ(rem, state->active_count);
    ASSERT_TRUE(state->rec_count > 0 && state->rec_count <= MAX_RECOMMENDATIONS);

    solver_set_mode(state, MODE_NYT);
    ASSERT_INT_EQ(rem, state->active_count);

    for (int i = 0; i < state->rec_count; i++) {
        ASSERT_TRUE(is_valid_lowercase_word(state->top_recs[i].word));
        ASSERT_TRUE(state->top_recs[i].entropy >= 0.0f);
        ASSERT_TRUE(state->top_recs[i].expected_remaining >= 1);
        if (i > 0) {
            ASSERT_TRUE(state->top_recs[i - 1].entropy + 0.002f >= state->top_recs[i].entropy);
        }
    }

    /* Exact match narrows to 1 candidate with 0 entropy */
    solver_reset(state);
    ASSERT_INT_EQ(NYT_WORD_COUNT, state->active_count);
    ASSERT_INT_EQ(MAX_RECOMMENDATIONS, state->rec_count);

    int solved_rem = solver_filter(state, "crane", "GGGGG");
    ASSERT_INT_EQ(1, solved_rem);
    ASSERT_INT_EQ(1, state->rec_count);
    ASSERT_STR_EQ("crane", state->top_recs[0].word);
    ASSERT_TRUE(fabsf(state->top_recs[0].entropy) < 1e-5f);

    /* Two-candidate corner case: "slate" with "BGGGG" leaves {"elate", "plate"} */
    solver_reset(state);
    int two_rem = solver_filter(state, "slate", "BGGGG");
    ASSERT_INT_EQ(2, two_rem);
    ASSERT_INT_EQ(2, state->rec_count);
    ASSERT_TRUE(fabsf(state->top_recs[0].entropy - 1.0f) < 1e-5f);
    ASSERT_TRUE(fabsf(state->top_recs[1].entropy - 1.0f) < 1e-5f);

    /* Zero remaining words corner case */
    solver_reset(state);
    int empty_rem = solver_filter(state, "zzzzz", "GGGGG");
    ASSERT_INT_EQ(0, empty_rem);
    ASSERT_INT_EQ(0, state->active_count);
    ASSERT_INT_EQ(0, state->rec_count);

    free(state);
}

static void test_multi_turn_filtering_and_invariants(void) {
    SolverState *state = (SolverState *)calloc(1, sizeof(SolverState));
    ASSERT_TRUE(state != NULL);

    solver_init(state, MODE_NYT);

    /* Simulate solving secret "abbey" across sequential turns without resetting */
    int rem1 = solver_filter(state, "crane", "BBYBY");
    ASSERT_TRUE(rem1 > 1 && rem1 < NYT_WORD_COUNT);
    ASSERT_INT_EQ(rem1, state->active_count);

    int active_seen = 0;
    for (int i = 0; i < state->total_words; i++) {
        bool consistent = solver_is_consistent(state->words[i].word, "crane", "BBYBY");
        ASSERT_TRUE(state->words[i].active == consistent);
        if (state->words[i].active) {
            active_seen++;
        }
    }
    ASSERT_INT_EQ(rem1, active_seen);

    /* Turn 2: "abbey" vs "salet" -> "BYBGB" */
    int rem2 = solver_filter(state, "salet", "BYBGB");
    ASSERT_TRUE(rem2 >= 1 && rem2 < rem1);
    ASSERT_INT_EQ(rem2, state->active_count);
    ASSERT_TRUE(state->rec_count > 0 && state->rec_count <= MAX_RECOMMENDATIONS);

    for (int i = 0; i < state->total_words; i++) {
        bool consistent_both =
            solver_is_consistent(state->words[i].word, "crane", "BBYBY") &&
            solver_is_consistent(state->words[i].word, "salet", "BYBGB");
        ASSERT_TRUE(state->words[i].active == consistent_both);
    }

    /* Turn 3: exact solve */
    int rem3 = solver_filter(state, "abbey", "GGGGG");
    ASSERT_INT_EQ(1, rem3);
    ASSERT_INT_EQ(1, state->active_count);
    ASSERT_INT_EQ(1, state->rec_count);
    ASSERT_STR_EQ("abbey", state->top_recs[0].word);
    ASSERT_TRUE(fabsf(state->top_recs[0].entropy) < 1e-5f);

    /* Turn 4: contradictory feedback narrows to 0; subsequent filter on empty pool stays 0 */
    int rem4 = solver_filter(state, "crane", "GGGGG");
    ASSERT_INT_EQ(0, rem4);
    ASSERT_INT_EQ(0, state->active_count);
    ASSERT_INT_EQ(0, state->rec_count);

    int rem5 = solver_filter(state, "slate", "BBBBB");
    ASSERT_INT_EQ(0, rem5);
    ASSERT_INT_EQ(0, state->active_count);
    ASSERT_INT_EQ(0, state->rec_count);

    free(state);
}

static void test_discord_mode_filtering_and_roundtrip(void) {
    SolverState *state = (SolverState *)calloc(1, sizeof(SolverState));
    ASSERT_TRUE(state != NULL);

    solver_init(state, MODE_NYT);
    solver_set_mode(state, MODE_DISCORD);
    ASSERT_INT_EQ(MODE_DISCORD, state->mode);
    ASSERT_INT_EQ(TOTAL_WORD_COUNT, state->total_words);
    ASSERT_INT_EQ(TOTAL_WORD_COUNT, state->active_count);
    ASSERT_INT_EQ(MAX_RECOMMENDATIONS, state->rec_count);

    int rem1 = solver_filter(state, "tares", "GGGYB");
    ASSERT_TRUE(rem1 > 0 && rem1 < TOTAL_WORD_COUNT);
    ASSERT_TRUE(state->rec_count > 0 && state->rec_count <= MAX_RECOMMENDATIONS);
    for (int i = 0; i < state->rec_count; i++) {
        ASSERT_TRUE(is_valid_lowercase_word(state->top_recs[i].word));
        ASSERT_TRUE(state->top_recs[i].entropy >= 0.0f);
        ASSERT_TRUE(state->top_recs[i].expected_remaining >= 1);
    }

    int rem2 = solver_filter(state, "taken", "GGGBB");
    ASSERT_TRUE(rem2 >= 0 && rem2 <= rem1);

    /* Switching back to MODE_NYT after filtering must fully reset pool & mode */
    solver_set_mode(state, MODE_NYT);
    ASSERT_INT_EQ(MODE_NYT, state->mode);
    ASSERT_INT_EQ(NYT_WORD_COUNT, state->total_words);
    ASSERT_INT_EQ(NYT_WORD_COUNT, state->active_count);
    ASSERT_INT_EQ(MAX_RECOMMENDATIONS, state->rec_count);

    free(state);
}

int main(void) {
    RUN_TEST(test_wordlists_integrity);
    RUN_TEST(test_feedback_exact_and_duplicates);
    RUN_TEST(test_pattern_index_bijection);
    RUN_TEST(test_consistency_checks);
    RUN_TEST(test_nyt_filter_recommendations_and_reset);
    RUN_TEST(test_multi_turn_filtering_and_invariants);
    RUN_TEST(test_discord_mode_filtering_and_roundtrip);

    printf("\nSummary: %d test(s) run, %d failure(s).\n", g_tests_run, g_tests_failed);
    return (g_tests_failed == 0) ? 0 : 1;
}
