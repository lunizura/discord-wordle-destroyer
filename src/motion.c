#include "motion.h"
#include <math.h>

float ease_linear(float t) {
    return (t < 0.0f) ? 0.0f : (t > 1.0f) ? 1.0f : t;
}

float ease_in_cubic(float t) {
    t = ease_linear(t);
    return t * t * t;
}

float ease_out_cubic(float t) {
    t = ease_linear(t);
    float inv = 1.0f - t;
    return 1.0f - (inv * inv * inv);
}

float ease_in_out_cubic(float t) {
    t = ease_linear(t);
    return (t < 0.5f) ? (4.0f * t * t * t) : (1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f);
}

float ease_out_back(float t) {
    t = ease_linear(t);
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    float inv = t - 1.0f;
    return 1.0f + c3 * inv * inv * inv + c1 * inv * inv;
}

float ease_out_expo(float t) {
    t = ease_linear(t);
    return (t >= 1.0f) ? 1.0f : 1.0f - powf(2.0f, -10.0f * t);
}

void tween_start(Tween *tw, float start, float target, float duration, float delay) {
    tw->start = start;
    tw->current = start;
    tw->target = target;
    tw->duration = (duration <= 0.0001f) ? 0.0001f : duration;
    tw->elapsed = 0.0f;
    tw->delay = delay;
    tw->active = true;
    tw->finished = false;
}

void tween_update(Tween *tw, float dt) {
    if (!tw->active || tw->finished) return;

    if (tw->delay > 0.0f) {
        tw->delay -= dt;
        if (tw->delay > 0.0f) return;
        // Delay elapsed, remainder goes into animation
        dt = -tw->delay;
        tw->delay = 0.0f;
    }

    tw->elapsed += dt;
    if (tw->elapsed >= tw->duration) {
        tw->elapsed = tw->duration;
        tw->current = tw->target;
        tw->finished = true;
        tw->active = false;
    }
}

float tween_get_value_cubic(const Tween *tw) {
    if (tw->delay > 0.0f) return tw->start;
    float progress = tw->elapsed / tw->duration;
    return tw->start + (tw->target - tw->start) * ease_out_cubic(progress);
}

float tween_get_value_pop(const Tween *tw) {
    if (tw->delay > 0.0f) return tw->start;
    float progress = tw->elapsed / tw->duration;
    return tw->start + (tw->target - tw->start) * ease_out_back(progress);
}

void spring_init(Spring *sp, float initial_val, float tension, float damping) {
    sp->value = initial_val;
    sp->velocity = 0.0f;
    sp->target = initial_val;
    sp->tension = tension;
    sp->damping = damping;
}

void spring_set_target(Spring *sp, float target) {
    sp->target = target;
}

void spring_update(Spring *sp, float dt) {
    // Semi-implicit Euler integration for stable spring physics
    if (dt > 0.05f) dt = 0.05f; // Clamp to avoid explosive instabilities on huge frame drops
    float force = -sp->tension * (sp->value - sp->target);
    float damping_force = -sp->damping * sp->velocity;
    float acceleration = force + damping_force;

    sp->velocity += acceleration * dt;
    sp->value += sp->velocity * dt;
}
