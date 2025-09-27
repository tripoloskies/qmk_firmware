// Copyright 2025 tripoloskies (https://github.com/tripoloskies)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "color.h"

// Default RGB animation speed index
#define RGB_CONTROL_SPEED_LEVELS_DEFAULT 2

// Flags for RGB control config
#define RGB_ANIMATION_CONFIG_NEW_FLAG 0x0E
#define RGB_ANIMATION_CONFIG_MODIFIED_FLAG 0x0F

// Size of RGB_ANIMATION_MODES (make SURE that the value matches the size of RGB_ANIMATION_MODES)
#define RGB_ANIMATION_MODES_SIZE 16

// EEPROM Memory Address RGB Control Coniguration
#define EEPROM_RGBCONTROL_ADDR EECONFIG_USER_DATABLOCK


// Structs for RGB animation settings
typedef union PACKED {
    uint64_t raw;
    struct {
        uint8_t flag;
        hsv_t color;
        uint8_t speed;
    };
} rgbconfig_animation_t;

// Structs for RGB control settings
typedef struct {
    int8_t flag;
    int8_t mode;
    rgbconfig_animation_t matrix_animation_settings[RGB_ANIMATION_MODES_SIZE];
} rgbconfig_t;

// List of RGB Matrix animations allowed by the manufacturer
extern const uint8_t RGB_ANIMATION_MODES[16];

// List of RGB Matrix animations that are not allowed by the manufacturer to change colors
extern const uint8_t RGB_ANIMATION_MODES_HUE_LOCK[7];

// List of speed levels
extern const uint8_t RGB_CONTROL_SPEED_LEVELS[5];

extern rgbconfig_t rgb_control_conf;

void rgbconfig_update(void);
uint8_t rgbconfig_find_animation_mode(uint8_t mode);
void rgbconfig_read(void);
