// Copyright 2025 tripoloskies (https://github.com/tripoloskies)
// Copyright 2024 yangzheng20003 (@yangzheng20003)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "color.h"


extern bool bat_req_flag;


#define RGB_CONTROL_MAX_SPEED   255
#define RGB_MATRIX_BAT_VAL      150
#define IM_BAT_REQ_LEVEL1_VAL   50
#define IM_BAT_REQ_LEVEL1_COLOR 0x00, RGB_MATRIX_BAT_VAL, 0x00

#define IM_BAT_REQ_LEVEL2_VAL   30
#define IM_BAT_REQ_LEVEL2_COLOR 0x00, RGB_MATRIX_BAT_VAL, 0x00

#define IM_BAT_REQ_LEVEL3_COLOR RGB_MATRIX_BAT_VAL, 0x00, 0x00


void rgb_control_init(void);
void rgb_blink_dir(void);
bool hs_reset_settings_user(void);
void rgb_req_bat_percentage(void);

void rgb_nkro_toggle(void);


void rgb_matrix_hue_shift_left(void);
void rgb_matrix_hue_shift_right(void);

void rgb_animation_switch(void);
void rgb_matrix_speed_low(void);
void rgb_matrix_speed_high(void);
void rgb_matrix_hue_shift_left(void);
void rgb_matrix_hue_shift_right(void);

void rgb_control_brightness_state(void);
void rgb_matrix_brightness_up(void);
void rgb_matrix_brightness_down(void);

void rgb_control_sync(void);
