#include "audio_synth.h"
#include "raylib.h"
#include <math.h>
#include <stdlib.h>

static Sound snd_key_tap;
static Sound snd_tile_pop;
static Sound snd_tile_flip;
static Sound snd_victory_chime;
static bool audio_ready = false;

#define SAMPLE_RATE 44100

// Helper to synthesize a mono 16-bit PCM wave from custom math
static Wave generate_wave(int sample_count, float (*gen_func)(float t, float duration)) {
    Wave wave = {0};
    wave.frameCount = sample_count;
    wave.sampleRate = SAMPLE_RATE;
    wave.sampleSize = 16;
    wave.channels = 1;

    short *samples = (short *)malloc(sample_count * sizeof(short));
    float duration = (float)sample_count / (float)SAMPLE_RATE;

    for (int i = 0; i < sample_count; i++) {
        float t = (float)i / (float)SAMPLE_RATE;
        float s = gen_func(t, duration);
        if (s > 1.0f) s = 1.0f;
        if (s < -1.0f) s = -1.0f;
        samples[i] = (short)(s * 32767.0f);
    }

    wave.data = samples;
    return wave;
}

// 1. Crisp mechanical keycap tap (15ms sine burst with rapid exponential decay)
static float synth_key_tap(float t, float dur) {
    (void)dur;
    float freq = 1200.0f;
    float env = expf(-t * 220.0f);
    return sinf(2.0f * PI * freq * t) * env * 0.35f;
}

// 2. Tile color cycle pop (25ms frequency sweep 600Hz -> 950Hz)
static float synth_tile_pop(float t, float dur) {
    float progress = t / dur;
    float freq = 600.0f + progress * 350.0f;
    float env = sinf(progress * PI) * expf(-progress * 2.5f);
    return sinf(2.0f * PI * freq * t) * env * 0.40f;
}

// 3. 3D Tile flip swoosh (120ms dual frequency resonant sweep)
static float synth_tile_flip(float t, float dur) {
    float progress = t / dur;
    float freq1 = 280.0f + sinf(progress * PI) * 180.0f;
    float freq2 = 420.0f + cosf(progress * PI) * 120.0f;
    float env = sinf(progress * PI) * 0.30f;
    return (sinf(2.0f * PI * freq1 * t) + 0.5f * sinf(2.0f * PI * freq2 * t)) * env;
}

// 4. Wordle Victory Chime (C-E-G major harmonic bell chord with long resonant decay)
static float synth_victory_chime(float t, float dur) {
    (void)dur;
    float c5 = 523.25f;
    float e5 = 659.25f;
    float g5 = 783.99f;
    float c6 = 1046.50f;

    float env = expf(-t * 5.0f);
    float s = (sinf(2.0f * PI * c5 * t) * 0.4f +
               sinf(2.0f * PI * e5 * t) * 0.3f +
               sinf(2.0f * PI * g5 * t) * 0.25f +
               sinf(2.0f * PI * c6 * t) * 0.15f) * env;
    return s * 0.5f;
}

void audio_init(void) {
    InitAudioDevice();
    if (!IsAudioDeviceReady()) return;

    Wave w_tap   = generate_wave((int)(SAMPLE_RATE * 0.018f), synth_key_tap);
    Wave w_pop   = generate_wave((int)(SAMPLE_RATE * 0.030f), synth_tile_pop);
    Wave w_flip  = generate_wave((int)(SAMPLE_RATE * 0.120f), synth_tile_flip);
    Wave w_chime = generate_wave((int)(SAMPLE_RATE * 0.450f), synth_victory_chime);

    snd_key_tap       = LoadSoundFromWave(w_tap);
    snd_tile_pop      = LoadSoundFromWave(w_pop);
    snd_tile_flip     = LoadSoundFromWave(w_flip);
    snd_victory_chime = LoadSoundFromWave(w_chime);

    UnloadWave(w_tap);
    UnloadWave(w_pop);
    UnloadWave(w_flip);
    UnloadWave(w_chime);

    audio_ready = true;
}

void audio_close(void) {
    if (!audio_ready) return;
    UnloadSound(snd_key_tap);
    UnloadSound(snd_tile_pop);
    UnloadSound(snd_tile_flip);
    UnloadSound(snd_victory_chime);
    CloseAudioDevice();
    audio_ready = false;
}

void audio_play_key_tap(void) {
    if (audio_ready) PlaySound(snd_key_tap);
}

void audio_play_tile_pop(void) {
    if (audio_ready) PlaySound(snd_tile_pop);
}

void audio_play_tile_flip(void) {
    if (audio_ready) PlaySound(snd_tile_flip);
}

void audio_play_victory_chime(void) {
    if (audio_ready) PlaySound(snd_victory_chime);
}
