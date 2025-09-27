// Copyright 2025 tripoloskies (https://github.com/tripoloskies)
// SPDX-License-Identifier: GPL-2.0-or-later
#include "quantum.h"
#include "nkro.h"
#include "fwconfig/fwconfig.h"

/*
    nkro_toggle_config()

    Toggles nkro flag from both eeprom config AND QMK internal keymap config.
    It automatically saves the whole shenanigans (configuration) into the EEPROM.
    It is an alternative way of toggling NKRO because the default NKRO toggle is buggy.
*/
void nkro_toggle_config(void) {
    bool is_nkro_active = keymap_config.nkro;

    kbconfig.nkro_flag = !is_nkro_active;
    keymap_config.nkro = !is_nkro_active;

    kbconfig_update();
}

void nkro_read_config(void) {
    keymap_config.nkro = kbconfig.nkro_flag;
}
