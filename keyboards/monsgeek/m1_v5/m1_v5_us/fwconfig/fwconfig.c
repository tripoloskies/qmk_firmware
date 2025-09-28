// Copyright 2025 tripoloskies (https://github.com/tripoloskies)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "quantum.h"
#include "rgb/rgb.h"
#include "fwconfig.h"


bool factory_settings_flag = false;

void factory_reset(void) {
    factory_settings_flag = true;
    factory_reset_indicator();
    eeconfig_init();
    eeconfig_update_rgb_matrix_default();
    keyboard_post_init_kb();
}
