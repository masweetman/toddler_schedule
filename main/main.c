#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "bsp/esp-bsp.h"
#include "lvgl.h"
#include "schedule_config.h"
#include "esp_codec_dev.h"

static const char *TAG = "toddler_schedule";

// ─── App State ────────────────────────────────────────────────────────────
static int                    current_step        = 0;
static bool                   celebration_showing = false;
static const schedule_step_t *active_steps        = NULL;
static int                    active_num_steps    = 0;

// ─── LVGL Object Handles ──────────────────────────────────────────────────
static lv_obj_t *scr_home;
static lv_obj_t *scr_main;
static lv_obj_t *img_step;
static lv_obj_t *lbl_step_name;
static lv_obj_t *lbl_tap_hint;
static lv_obj_t *bar_progress;
static lv_obj_t *lbl_progress;
static lv_obj_t *lbl_cheer;
static lv_obj_t *scr_celebration;

// ─── Play a short "tada" tone ─────────────────────────────────────────────
static esp_codec_dev_handle_t s_speaker = NULL;

static void play_tada(void)
{
    if (!s_speaker) return;

    const int sample_rate = 16000;
    const int duration_ms = 300;
    const int num_samples = (sample_rate * duration_ms) / 1000;

    int16_t *buf = heap_caps_malloc(num_samples * 2 * sizeof(int16_t), MALLOC_CAP_DEFAULT);
    if (!buf) return;

    for (int i = 0; i < num_samples; i++) {
        int freq       = (i < num_samples / 2) ? 880 : 1100;
        float t        = (float)i / sample_rate;
        int16_t sample = (int16_t)(8000 * (((int)(t * freq * 2) % 2 == 0) ? 1 : -1));
        buf[i * 2]     = sample;
        buf[i * 2 + 1] = sample;
    }

    esp_codec_dev_write(s_speaker, buf, num_samples * 2 * sizeof(int16_t));
    free(buf);
}

// ─── All-done celebration screen ─────────────────────────────────────────
static void show_celebration(void)
{
    celebration_showing = true;
    lv_scr_load(scr_celebration);
    play_tada();
    play_tada();
}

// ─── Refresh the main screen for the current step ────────────────────────
static void refresh_screen(void)
{
    if (current_step >= active_num_steps) {
        show_celebration();
        return;
    }

    const schedule_step_t *step = &active_steps[current_step];

    lv_img_set_src(img_step, step->image);
    lv_obj_align(img_step, LV_ALIGN_CENTER, 0, -30);
    lv_label_set_text(lbl_step_name, step->label);

    int progress = (current_step * 100) / active_num_steps;
    lv_bar_set_value(bar_progress, progress, LV_ANIM_ON);

    char prog_str[32];
    snprintf(prog_str, sizeof(prog_str), "Step %d of %d",
             current_step + 1, active_num_steps);
    lv_label_set_text(lbl_progress, prog_str);

    lv_obj_add_flag(lbl_cheer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(lbl_tap_hint, LV_OBJ_FLAG_HIDDEN);
}

// ─── Touch callback ───────────────────────────────────────────────────────
static void advance_step_cb(lv_timer_t *t)
{
    refresh_screen();
    lv_timer_del(t);
}

static void on_screen_touched(lv_event_t *e)
{
    if (celebration_showing) {
        celebration_showing = false;
        current_step = 0;
        lv_scr_load(scr_home);
        return;
    }

    if (current_step >= active_num_steps) return;

    const schedule_step_t *step = &active_steps[current_step];
    lv_label_set_text(lbl_cheer, step->cheer);
    lv_obj_clear_flag(lbl_cheer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(lbl_tap_hint, LV_OBJ_FLAG_HIDDEN);

    play_tada();
    current_step++;

    lv_timer_create(advance_step_cb, 1500, NULL);
}

// ─── Home screen button callbacks ────────────────────────────────────────
static void on_morning_pressed(lv_event_t *e)
{
    active_steps     = MORNING_STEPS;
    active_num_steps = NUM_MORNING_STEPS;
    current_step     = 0;
    lv_scr_load(scr_main);
    refresh_screen();
}

static void on_evening_pressed(lv_event_t *e)
{
    active_steps     = EVENING_STEPS;
    active_num_steps = NUM_EVENING_STEPS;
    current_step     = 0;
    lv_scr_load(scr_main);
    refresh_screen();
}

// ─── Build all UI elements ────────────────────────────────────────────────
static void build_ui(void)
{
    // ── Home screen ──────────────────────────────────────────
    scr_home = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_home, lv_color_hex(0x000000), 0);
    lv_obj_set_style_pad_all(scr_home, 0, 0);
    lv_obj_clear_flag(scr_home, LV_OBJ_FLAG_SCROLLABLE);

    // Morning panel (left half)
    lv_obj_t *btn_morning = lv_obj_create(scr_home);
    lv_obj_set_size(btn_morning, 160, 240);
    lv_obj_set_pos(btn_morning, 0, 0);
    lv_obj_set_style_bg_color(btn_morning, lv_color_hex(0xE8A020), 0);
    lv_obj_set_style_radius(btn_morning, 0, 0);
    lv_obj_set_style_border_width(btn_morning, 0, 0);
    lv_obj_set_style_pad_all(btn_morning, 0, 0);
    lv_obj_clear_flag(btn_morning, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(btn_morning, on_morning_pressed, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lbl_morning_icon = lv_label_create(btn_morning);
    lv_label_set_text(lbl_morning_icon, LV_SYMBOL_CHARGE);
    lv_obj_set_style_text_font(lbl_morning_icon, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(lbl_morning_icon, lv_color_hex(0x1A1A1A), 0);
    lv_obj_align(lbl_morning_icon, LV_ALIGN_CENTER, 0, -20);

    lv_obj_t *lbl_morning_txt = lv_label_create(btn_morning);
    lv_label_set_text(lbl_morning_txt, "Morning");
    lv_obj_set_style_text_font(lbl_morning_txt, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(lbl_morning_txt, lv_color_hex(0x1A1A1A), 0);
    lv_obj_align(lbl_morning_txt, LV_ALIGN_CENTER, 0, 20);

    // Bedtime panel (right half)
    lv_obj_t *btn_evening = lv_obj_create(scr_home);
    lv_obj_set_size(btn_evening, 160, 240);
    lv_obj_set_pos(btn_evening, 160, 0);
    lv_obj_set_style_bg_color(btn_evening, lv_color_hex(0x1A2050), 0);
    lv_obj_set_style_radius(btn_evening, 0, 0);
    lv_obj_set_style_border_width(btn_evening, 0, 0);
    lv_obj_set_style_pad_all(btn_evening, 0, 0);
    lv_obj_clear_flag(btn_evening, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(btn_evening, on_evening_pressed, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lbl_evening_icon = lv_label_create(btn_evening);
    lv_label_set_text(lbl_evening_icon, LV_SYMBOL_BELL);
    lv_obj_set_style_text_font(lbl_evening_icon, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(lbl_evening_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(lbl_evening_icon, LV_ALIGN_CENTER, 0, -20);

    lv_obj_t *lbl_evening_txt = lv_label_create(btn_evening);
    lv_label_set_text(lbl_evening_txt, "Bedtime");
    lv_obj_set_style_text_font(lbl_evening_txt, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(lbl_evening_txt, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(lbl_evening_txt, LV_ALIGN_CENTER, 0, 20);

    // ── Main screen ──────────────────────────────────────────
    scr_main = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_main, lv_color_hex(0x000000), 0);
    lv_obj_add_event_cb(scr_main, on_screen_touched, LV_EVENT_CLICKED, NULL);

    // Progress bar
    bar_progress = lv_bar_create(scr_main);
    lv_obj_set_size(bar_progress, 280, 18);
    lv_obj_align(bar_progress, LV_ALIGN_TOP_MID, 0, 8);
    lv_bar_set_range(bar_progress, 0, 100);
    lv_bar_set_value(bar_progress, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bar_progress, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_color(bar_progress, lv_color_hex(0xFF8C42),
                              LV_PART_INDICATOR);
    lv_obj_set_style_radius(bar_progress, 9, 0);
    lv_obj_set_style_radius(bar_progress, 9, LV_PART_INDICATOR);

    // "Step X of Y" label
    lbl_progress = lv_label_create(scr_main);
    lv_obj_align(lbl_progress, LV_ALIGN_TOP_MID, 0, 32);
    lv_obj_set_style_text_font(lbl_progress, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl_progress, lv_color_hex(0xFFFFFF), 0);

    // Step image
    img_step = lv_img_create(scr_main);
    lv_obj_align(img_step, LV_ALIGN_CENTER, 0, -30);

    // Step name
    lbl_step_name = lv_label_create(scr_main);
    lv_obj_align(lbl_step_name, LV_ALIGN_CENTER, 0, 80);
    lv_obj_set_style_text_font(lbl_step_name, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(lbl_step_name, lv_color_hex(0xFFFFFF), 0);

    // Tap hint
    lbl_tap_hint = lv_label_create(scr_main);
    lv_label_set_text(lbl_tap_hint, "Tap when done!");
    lv_obj_align(lbl_tap_hint, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_text_font(lbl_tap_hint, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(lbl_tap_hint, lv_color_hex(0xFFFFFF), 0);

    // Cheer message (hidden until tap)
    lbl_cheer = lv_label_create(scr_main);
    lv_obj_align(lbl_cheer, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_text_font(lbl_cheer, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(lbl_cheer, lv_color_hex(0xFFFFFF), 0);
    lv_obj_add_flag(lbl_cheer, LV_OBJ_FLAG_HIDDEN);

    // ── Celebration screen ───────────────────────────────────
    scr_celebration = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_celebration, lv_color_hex(0x000000), 0);
    lv_obj_add_event_cb(scr_celebration, on_screen_touched,
                        LV_EVENT_CLICKED, NULL);

    lv_obj_t *lbl_star = lv_label_create(scr_celebration);
    lv_label_set_text(lbl_star, "ALL DONE!");
    lv_obj_set_style_text_font(lbl_star, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(lbl_star, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(lbl_star, LV_ALIGN_CENTER, 0, -30);

    lv_obj_t *lbl_well_done = lv_label_create(scr_celebration);
    lv_label_set_text(lbl_well_done, "Amazing job tonight!");
    lv_obj_set_style_text_font(lbl_well_done, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(lbl_well_done, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(lbl_well_done, LV_ALIGN_CENTER, 0, 20);

    lv_obj_t *lbl_restart = lv_label_create(scr_celebration);
    lv_label_set_text(lbl_restart, "Tap to start again");
    lv_obj_set_style_text_font(lbl_restart, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl_restart, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(lbl_restart, LV_ALIGN_BOTTOM_MID, 0, -15);

    // ── Load home screen ─────────────────────────────────────
    lv_scr_load(scr_home);
}

// ─── Entry point ──────────────────────────────────────────────────────────
void app_main(void)
{
    ESP_LOGI(TAG, "Toddler Schedule starting...");

    bsp_display_start();
    bsp_display_backlight_on();
    s_speaker = bsp_audio_codec_speaker_init();
    esp_codec_dev_sample_info_t fs = { .sample_rate = 16000, .channel = 2, .bits_per_sample = 16 };
    esp_codec_dev_open(s_speaker, &fs);
    esp_codec_dev_set_out_vol(s_speaker, 50);   // ← ADD THIS LINE (0-100)


    bsp_display_lock(0);
    build_ui();
    bsp_display_unlock();

    ESP_LOGI(TAG, "UI ready.");
}