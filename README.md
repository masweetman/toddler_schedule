# Toddler Bedtime Schedule

A touchscreen kiosk app for the **ESP32-S3-BOX-3** that guides a toddler through their bedtime routine step by step.

## How it works

1. The device powers on and displays the first step with an image, a step name, and a progress bar.
2. The toddler taps the screen when they finish a step — a cheer message appears and a short "tada" tone plays.
3. After 1.5 seconds the app automatically advances to the next step.
4. When all steps are complete a celebration screen appears ("ALL DONE!").
5. Tapping the celebration screen restarts from step 1.

## Steps

| # | Label | Cheer |
|---|-------|-------|
| 1 | Brush Teeth | "Great brushing! ✨" |
| 2 | Put on PJs | "PJs on! So cozy! 🌙" |
| 3 | Story Time | "All done! Goodnight! ⭐" |

Steps are defined in [`main/schedule_config.h`](main/schedule_config.h). Adding or changing steps only requires editing that file and placing a matching image C array in `main/images/`.

## Hardware

- **Board:** Espressif ESP32-S3-BOX-3
- **Display:** 320×240 ILI9341 (handled by BSP)
- **Touch:** GT911 capacitive touch (handled by BSP)
- **Speaker:** Built-in codec via `esp_codec_dev`

## Project structure

```
main/
  main.c              # Application logic and UI
  schedule_config.h   # Step definitions (labels, images, cheer text)
  images/
    brush_teeth.c     # LVGL image array
    pajamas.c
    story_time.c
```

## Dependencies

| Component | Version |
|-----------|---------|
| ESP-IDF | ≥ 5.0.0 |
| espressif/esp-box-3 (BSP) | ≥ 1.2.0 |
| lvgl/lvgl | ≥ 8.3.6, < 9.0.0 |

Dependencies are managed by the ESP-IDF Component Manager and declared in [`main/idf_component.yml`](main/idf_component.yml).

## Building and flashing

Open an ESP-IDF terminal (or source the ESP-IDF environment), then:

```bash
# Install/update managed components
idf.py update-dependencies

# Build
idf.py build

# Flash and monitor (replace PORT with your device, e.g. /dev/cu.usbserial-*)
idf.py -p PORT flash monitor
```

## Adding a new step

1. Convert your image to an LVGL C array (e.g. with [LVGL's online image converter](https://lvgl.io/tools/imageconverter)) and save it as `main/images/<name>.c`.
2. In `main/schedule_config.h`:
   - Add `LV_IMG_DECLARE(<name>);`
   - Add an entry to `STEPS[]` with `.label`, `.image`, and `.cheer`.
   - Increment `NUM_STEPS`.
