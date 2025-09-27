// Copyright 2025 tripoloskies (https://github.com/tripoloskies)
// Copyright 2024 yangzheng20003 (@yangzheng20003)
// SPDX-License-Identifier: GPL-2.0-or-later
#include "record.h"
#include "fwconfig/fwconfig.h"

bool process_record_arrow_swap(uint16_t keycode, keyrecord_t *record) {


    if (!kbconfig.arrow_swap_flag) {
        return true;
    }

    switch (keycode) {
        case KC_W:
            (record->event.pressed) ? register_code16(KC_UP) : unregister_code16(KC_UP);
        break;
        case KC_A:
            (record->event.pressed) ? register_code16(KC_LEFT) : unregister_code16(KC_LEFT);
        break;
        case KC_S:
            (record->event.pressed) ? register_code16(KC_DOWN) : unregister_code16(KC_DOWN);
        break;
        case KC_D:
            (record->event.pressed) ? register_code16(KC_RIGHT) : unregister_code16(KC_RIGHT);
        break;
        case KC_UP:
            (record->event.pressed) ? register_code16(KC_W) : unregister_code16(KC_W);
        break;
        case KC_LEFT:
            (record->event.pressed) ? register_code16(KC_A) : unregister_code16(KC_A);
        break;
        case KC_DOWN:
            (record->event.pressed) ? register_code16(KC_S) : unregister_code16(KC_S);
        break;
        case KC_RIGHT:
            (record->event.pressed) ? register_code16(KC_D) : unregister_code16(KC_D);
        break;

        default:
            return true;
    }

    return false;
}
