#ifndef AUDIO_SYNTH_H
#define AUDIO_SYNTH_H

#include <stdbool.h>

void audio_init(void);
void audio_close(void);

void audio_play_key_tap(void);
void audio_play_tile_pop(void);
void audio_play_tile_flip(void);
void audio_play_victory_chime(void);

#endif // AUDIO_SYNTH_H
