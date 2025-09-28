// Copyright 2025 tripoloskies (https://github.com/tripoloskies)
// Copyright 2024 yangzheng20003 (@yangzheng20003)
// SPDX-License-Identifier: GPL-2.0-or-later
#include "quantum.h"
#include "fwconfig.h"

kbconfig_t kbconfig;

void kbconfig_update(void) {
    eeconfig_update_kb(kbconfig.raw);
}

uint32_t kbconfig_read(void) {
    return eeconfig_read_kb();
}

void kbconfig_default(void) {
    kbconfig.flag             = true;
    kbconfig.arrow_swap_flag  = 0;
    kbconfig.ctrl_app_flag    = 0;

#ifdef WIRELESS_ENABLE
    kbconfig.devs = DEVS_USB;
#endif
    kbconfig_update();
}

void kbconfig_init(void) {
    kbconfig.raw = kbconfig_read();
    if (!kbconfig.raw) {
        kbconfig_default();
    }
}

