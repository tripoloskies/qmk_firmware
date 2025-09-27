// Copyright 2025 tripoloskies (https://github.com/tripoloskies)
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include "quantum.h"
#include <stdint.h>


bool process_record_arrow_swap(uint16_t keycode, keyrecord_t *record);
bool process_record_system(uint16_t keycode, keyrecord_t *record);
bool process_record_rgb_controls(uint16_t keycode, keyrecord_t *record);
