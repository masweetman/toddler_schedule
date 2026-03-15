#pragma once
#include "lvgl.h"

// ─── Forward-declare image descriptors ───────────────────────────────────
LV_IMG_DECLARE(brush_teeth);
LV_IMG_DECLARE(pajamas);
LV_IMG_DECLARE(story_time);
LV_IMG_DECLARE(breakfast);
LV_IMG_DECLARE(get_dressed);

// ─── Step type ────────────────────────────────────────────────────────────
typedef struct {
    const char         *label;
    const lv_img_dsc_t *image;
    const char         *cheer;
} schedule_step_t;

// ─── Morning sequence ─────────────────────────────────────────────────────
#define NUM_MORNING_STEPS 3

static const schedule_step_t MORNING_STEPS[NUM_MORNING_STEPS] = {
    {
        .label = "Brush Teeth",
        .image = &brush_teeth,
        .cheer = "Great brushing! " LV_SYMBOL_OK
    },
    {
        .label = "Eat Breakfast",
        .image = &breakfast,
        .cheer = "Yummy breakfast! " LV_SYMBOL_OK
    },
    {
        .label = "Get Dressed",
        .image = &get_dressed,
        .cheer = "Looking great! " LV_SYMBOL_OK
    },
};

// ─── Evening sequence ─────────────────────────────────────────────────────
#define NUM_EVENING_STEPS 3

static const schedule_step_t EVENING_STEPS[NUM_EVENING_STEPS] = {
    {
        .label = "Brush Teeth",
        .image = &brush_teeth,
        .cheer = "Great brushing! " LV_SYMBOL_CHARGE
    },
    {
        .label = "Put on PJs",
        .image = &pajamas,
        .cheer = "PJs on! So cozy!"
    },
    {
        .label = "Story Time",
        .image = &story_time,
        .cheer = "All done! Goodnight!"
    },
};