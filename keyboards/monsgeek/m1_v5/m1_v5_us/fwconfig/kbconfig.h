// Copyright 2025 tripoloskies (https://github.com/tripoloskies)
// Copyright 2024 yangzheng20003 (@yangzheng20003)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef WIRELESS_ENABLE
#    include "wireless.h"
#endif

typedef union {
    uint32_t raw;
    struct {
        uint8_t flag : 1;
        uint8_t devs : 3;
        uint8_t arrow_swap_flag : 1;
        uint8_t ctrl_app_flag : 1;
    };
} kbconfig_t;


extern kbconfig_t kbconfig;

void kbconfig_update(void);
void kbconfig_init(void);
