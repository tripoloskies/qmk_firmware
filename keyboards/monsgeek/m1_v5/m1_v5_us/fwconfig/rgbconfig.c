// Copyright 2025 tripoloskies (https://github.com/tripoloskies)
// SPDX-License-Identifier: GPL-2.0-or-later
#include "quantum.h"
#include "rgbconfig.h"




// RGB Control Configuration
rgbconfig_t rgb_control_conf;

// List of RGB Matrix animations allowed by the manufacturer
const uint8_t RGB_ANIMATION_MODES[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

// List of RGB Matrix animations that are not allowed by the manufacturer to change colors
const uint8_t RGB_ANIMATION_MODES_HUE_LOCK[7]  = {6, 13, 15, 16, 25, 26, 34};




////////////////////////////////////////////////////////////////////////////
///     Configuration Actions                                            ///
////////////////////////////////////////////////////////////////////////////

void rgbconfig_update(void) {
    eeconfig_update_user_datablock(&rgb_control_conf, 0, sizeof(rgb_control_conf));
}


uint8_t rgbconfig_find_animation_mode(uint8_t mode) {
    for (uint8_t i = 0; i < RGB_ANIMATION_MODES_SIZE; i++) {
        if (RGB_ANIMATION_MODES[i] == mode) {
            return i;
        }
    }
    // Returns default value
    return 0;
}

void rgbconfig_defaults(void) {
    rgb_control_conf.flag = RGB_ANIMATION_CONFIG_NEW_FLAG;
    rgb_control_conf.mode = 0;

#ifdef RGB_MATRIX_DEFAULT_MODE
    rgb_control_conf.mode = rgbconfig_find_animation_mode(RGB_MATRIX_DEFAULT_MODE);
#endif

    for (int i = 0; i < RGB_ANIMATION_MODES_SIZE; i++) {
        rgbconfig_animation_t *animation_config = &rgb_control_conf.matrix_animation_settings[i];
        animation_config->flag = RGB_ANIMATION_CONFIG_NEW_FLAG;
        animation_config->color = (HSV) {RGB_MATRIX_DEFAULT_HUE, RGB_MATRIX_DEFAULT_SAT, RGB_MATRIX_DEFAULT_VAL};
        animation_config->speed = RGB_MATRIX_DEFAULT_SPD;
    }

    rgbconfig_update();
}


void rgbconfig_read(void) {
    eeconfig_read_user_datablock(&rgb_control_conf, 0, sizeof(rgb_control_conf));

    if (rgb_control_conf.flag != RGB_ANIMATION_CONFIG_NEW_FLAG) {
            rgbconfig_defaults();
    }
}
