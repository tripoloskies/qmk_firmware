// Copyright 2025 tripoloskies (https://github.com/tripoloskies)
// SPDX-License-Identifier: GPL-2.0-or-later
#include QMK_KEYBOARD_H
#include "quantum.h"
#include <stdint.h>
#include "rgb/rgb.h"
#include "record.h"

bool process_record_rgb_controls(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case RM_SPDU: {
            if (record->event.pressed) {
                rgb_matrix_speed_high();
            }
        } break;
        case RM_SPDD: {
            if (record->event.pressed) {
                rgb_matrix_speed_low();
            }
        } break;
        case RM_VALU: {
            if (record->event.pressed) {
                rgb_matrix_brightness_up();
            }
        } break;
        case RM_VALD: {
            if (record->event.pressed) {
                rgb_matrix_brightness_down();
            }
        } break;
        case RM_HUED: {
            if (record->event.pressed) {
                rgb_matrix_hue_shift_left();
            }
        } break;
        case RM_HUEU: {
            if (record->event.pressed) {
                rgb_matrix_hue_shift_right();
            }
        } break;
        case RM_TOGG: {
            if (record->event.pressed) {
                rgb_animation_switch();
            }
        }break;
        default: {
            return true;
        }
    }
    return false;
}
