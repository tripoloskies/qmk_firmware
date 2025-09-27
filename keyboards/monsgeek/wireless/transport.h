// Copyright 2025 tripoloskies (https://github.com/tripoloskies)
// Copyright 2024 Su (@isuua)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdbool.h>
#include "host_driver.h"

typedef enum {
    TRANSPORT_NONE,
    TRANSPORT_USB,
    TRANSPORT_WLS,
} transport_t;

extern host_driver_t chibios_driver;
extern host_driver_t wireless_driver;


void driver_release_all_keys(host_driver_t* driver);
void wls_transport_enable(bool enable);
void usb_transport_enable(bool enable);
void set_transport(transport_t new_transport);
transport_t get_transport(void);
void usb_power_connect(void);
void usb_power_disconnect(void);
void usb_remote_wakeup(void);
void usb_wakeup_trigger(void);
