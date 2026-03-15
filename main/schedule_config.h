#pragma once
#include "lvgl.h"

// ─── Forward-declare your image descriptors ───────────────────────────────
LV_IMG_DECLARE(brush_teeth);
LV_IMG_DECLARE(pajamas);
LV_IMG_DECLARE(story_time);

// ─── Define each step ─────────────────────────────────────────────────────
typedef struct {
    const char         *label;
    const lv_img_dsc_t *image;
    const char         *cheer;
} schedule_step_t;

#define NUM_STEPS 3

static const schedule_step_t STEPS[NUM_STEPS] = {
    {
        .label = "Brush Teeth",
        .image = &brush_teeth,
        .cheer = "Great brushing! ✨"
    },
    {
        .label = "Put on PJs",
        .image = &pajamas,
        .cheer  = "PJs on! So cozy! 🌙"
    },
    {
        .label = "Story Time",
        .image = &story_time,
        .cheer  = "All done! Goodnight! ⭐"
    },
};