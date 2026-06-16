#ifndef MOTION_H
#define MOTION_H

#include <stdbool.h>

typedef struct {
    float current;
    float target;
    float start;
    float duration;
    float elapsed;
    float delay;
    bool active;
    bool finished;
} Tween;

typedef struct {
    float value;
    float velocity;
    float target;
    float tension;   // Spring stiffness (e.g. 180.0f)
    float damping;   // Friction damping (e.g. 12.0f)
} Spring;

// Easing functions
float ease_linear(float t);
float ease_in_cubic(float t);
float ease_out_cubic(float t);
float ease_in_out_cubic(float t);
float ease_out_back(float t); // Overshoot bounce (10-15%)
float ease_out_expo(float t);

// Tween lifecycle
void tween_start(Tween *tw, float start, float target, float duration, float delay);
void tween_update(Tween *tw, float dt);
float tween_get_value_cubic(const Tween *tw);
float tween_get_value_pop(const Tween *tw);

// Spring physics lifecycle
void spring_init(Spring *sp, float initial_val, float tension, float damping);
void spring_set_target(Spring *sp, float target);
void spring_update(Spring *sp, float dt);

#endif // MOTION_H
