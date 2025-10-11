// Copyright 2025 tripoloskies (https://github.com/tripoloskies)
// Copyright 2024 yangzheng20003 (@yangzheng20003)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdint.h>

#define keymap_is_mac_system() ((get_highest_layer(default_layer_state) == _MBL) || (get_highest_layer(default_layer_state) == _MFL))
#define keymap_is_base_layer() ((get_highest_layer(default_layer_state) == _BL) || (get_highest_layer(default_layer_state) == _FL))

extern uint32_t post_init_timer;
extern uint32_t factory_reset_timer;
extern uint32_t bat_indicator_timer;

void nkro_toggle_config(void);
void nkro_read_config(void);
void usb_power_connect(void);
void usb_power_disconnect(void);
