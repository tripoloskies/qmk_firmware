// Copyright 2025 tripoloskies (https://github.com/tripoloskies)
// Copyright 2024 yangzheng20003 (@yangzheng20003)
// SPDX-License-Identifier: GPL-2.0-or-later
#include "control.h"
#include "rgb_matrix.h"
#include QMK_KEYBOARD_H
#include "info_config.h"
#include "quantum.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "fwconfig/fwconfig.h"
#include "rgb.h"


// Flag to display battery capacity status
bool bat_req_flag;

// Flag for initialized RGB control
static bool initialized_flag = false;


static bool rgb_control_modified_host = false;

void rgb_config_load_current(void) {
    uint8_t index = rgb_control_conf.mode;
    rgbconfig_animation_t *current_config = &rgb_control_conf.matrix_animation_settings[index];
    rgb_control_modified_host = true;
    rgb_matrix_sethsv_noeeprom(current_config->color.h, current_config->color.s, current_config->color.v);
    rgb_matrix_set_speed_noeeprom(current_config->speed);
    rgb_control_brightness_state();
}


void rgb_blink_dir(void) {
    rgb_queue_frame_queue(INDICATOR_QUEUE_MAIN, 0xFF, (rgb_t){0, 0, 0}, 250, 1, NULL);
}

void factory_reset_indicator(void) {
    rgb_queue_frame_queue(INDICATOR_QUEUE_MAIN, 0xFF, (rgb_t){255, 255, 255}, 150, 3, NULL);
}

void rgb_nkro_toggle(void) {
    rgb_queue_frame_queue(INDICATOR_QUEUE_MAIN, 0xFF, (rgb_t){255, 0, 0}, 250, 0, NULL);
    rgb_queue_frame_queue(INDICATOR_QUEUE_MAIN,0xFF, (rgb_t){0, 255, 0}, 250, 0, NULL);
    rgb_queue_frame_queue(INDICATOR_QUEUE_MAIN,0xFF, (rgb_t){0, 0, 255}, 250, 0, NULL);
}



void rgb_req_bat_percentage(void) {
    uint8_t mi_index[10] = RGB_MATRIX_BAT_INDEX_MAP;
    if (bat_req_flag) {
        rgb_matrix_set_color_all(0x00, 0x00, 0x00);
        for (uint8_t i = 0; i < 10; i++) {
            if ((i < (*md_getp_bat() / 10)) || (i < 1)) {
                if (*md_getp_bat() >= (IM_BAT_REQ_LEVEL1_VAL)) {
                    rgb_matrix_set_color(mi_index[i], IM_BAT_REQ_LEVEL1_COLOR);
                } else if (*md_getp_bat() >= (IM_BAT_REQ_LEVEL2_VAL)) {
                    rgb_matrix_set_color(mi_index[i], IM_BAT_REQ_LEVEL2_COLOR);
                } else {
                    rgb_matrix_set_color(mi_index[i], IM_BAT_REQ_LEVEL3_COLOR);
                }
            } else {
                rgb_matrix_set_color(mi_index[i], 0x00, 0x00, 0x00);
            }
        }
    }
}


void rgb_control_brightness_state(void) {
    uint8_t index = rgb_control_conf.mode;
    rgbconfig_animation_t *current_config = &rgb_control_conf.matrix_animation_settings[index];

    if (current_config->color.v > 0) {
        gpio_write_pin_high(LED_POWER_EN_PIN);
    }
    else {
        gpio_write_pin_low(LED_POWER_EN_PIN);
    }
}


void rgb_animation_switch(void) {
    if (rgb_control_conf.mode >= RGB_ANIMATION_MODES_SIZE - 1) {
        rgb_control_conf.mode = 0;
    }
    else {
        rgb_control_conf.mode++;
    }

    rgb_matrix_mode(RGB_ANIMATION_MODES[rgb_control_conf.mode]);
    uprintf("Config mode from RGB Matrix: %s\n", rgb_matrix_get_mode_name(RGB_ANIMATION_MODES[rgb_control_conf.mode]));
    rgb_config_load_current();
}


void rgb_control_set_speed(bool increase) {
    uint8_t index = rgb_control_conf.mode;
    rgbconfig_animation_t *current_config = &rgb_control_conf.matrix_animation_settings[index];

    rgb_control_modified_host = true;

    if (increase) {
        if ((uint16_t)(current_config->speed + RGB_MATRIX_SPD_STEP) >= RGB_CONTROL_MAX_SPEED) {
            current_config->speed = RGB_CONTROL_MAX_SPEED;
        }
        else {
            current_config->speed += RGB_MATRIX_SPD_STEP;
        }
    }
    else {
        if ((int16_t)(current_config->speed - RGB_MATRIX_SPD_STEP) <= 0) {
            current_config->speed = 0;
        }
        else {
            current_config->speed -= RGB_MATRIX_SPD_STEP;
        }
    }


    rgb_matrix_set_speed_noeeprom(current_config->speed);
    rgbconfig_update();
}


void rgb_control_set_hue(bool increase) {
    uint8_t index = rgb_control_conf.mode;
    rgbconfig_animation_t *current_config = &rgb_control_conf.matrix_animation_settings[index];
    HSV *color_conf = &current_config->color;

    rgb_control_modified_host = true;

    if (increase) {
        rgb_matrix_increase_hue_noeeprom();
    }
    else {
        rgb_matrix_decrease_hue_noeeprom();
    }

    *color_conf = rgb_matrix_get_hsv();
    rgbconfig_update();
}

void rgb_control_set_brightness(bool increase) {
    uint8_t index = rgb_control_conf.mode;
    rgbconfig_animation_t *current_config = &rgb_control_conf.matrix_animation_settings[index];
    HSV *color_conf = &current_config->color;

    rgb_control_modified_host = true;
    if (increase) {
        rgb_matrix_increase_val_noeeprom();
    }
    else {
        rgb_matrix_decrease_val_noeeprom();
    }
    *color_conf = rgb_matrix_get_hsv();
    rgb_control_brightness_state();
    rgbconfig_update();
}

void rgb_matrix_speed_low(void) {
    rgb_control_set_speed(false);
}

void rgb_matrix_speed_high(void) {
    rgb_control_set_speed(true);
}

void rgb_matrix_hue_shift_left(void) {
    rgb_control_set_hue(false);
}

void rgb_matrix_hue_shift_right(void) {
    rgb_control_set_hue(true);
}

void rgb_matrix_brightness_down(void) {
    rgb_control_set_brightness(false);
}

void rgb_matrix_brightness_up(void) {
    rgb_control_set_brightness(true);
}


void rgb_control_sync(void) {
    static rgb_config_t prev_rgb_matrix_config;

    if (!prev_rgb_matrix_config.raw) {
        prev_rgb_matrix_config.raw = rgb_matrix_config.raw;
        return;
    }

    if (rgb_control_modified_host) {
        rgb_control_modified_host = false;
        return;
    }

    if (memcmp(&prev_rgb_matrix_config, &rgb_matrix_config, sizeof(rgb_matrix_config))) {
        uint8_t index;
        rgbconfig_animation_t *current_config;

        prev_rgb_matrix_config.raw = rgb_matrix_config.raw;

        rgb_control_conf.mode = rgbconfig_find_animation_mode(rgb_matrix_get_mode());
        index = rgb_control_conf.mode;

        current_config = &rgb_control_conf.matrix_animation_settings[index];
        current_config->color = rgb_matrix_get_hsv();
        current_config->speed = rgb_matrix_get_speed();
        rgbconfig_update();
    }
}


void rgb_control_init(void) {
    rgb_matrix_enable_noeeprom();
    rgbconfig_read();

    if (!initialized_flag) {
        rgb_queue_init();
        initialized_flag = true;
    }

    rgb_config_load_current();
}
