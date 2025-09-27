// Copyright 2025 tripoloskies (https://github.com/tripoloskies)
// Copyright 2024 Su (@isuua)
// SPDX-License-Identifier: GPL-2.0-or-later


#include <stdlib.h>
#include "_wait.h"
#include "hal_usb.h"
#include "host.h"
#include "host_driver.h"
#include "usb_device_state.h"
#include "quantum.h"
#include "wireless.h"
#include "module.h"
#include "usb_main.h"
#include "transport.h"

#ifndef USB_POWER_DOWN_DELAY
#    define USB_POWER_DOWN_DELAY 30000
#endif

static transport_t transport = TRANSPORT_USB;

void driver_release_all_keys(host_driver_t *driver) {
    driver->send_keyboard(NULL);
    driver->send_nkro(NULL);
    driver->send_extra(NULL);
    driver->send_mouse(NULL);
}

void wls_transport_enable(bool enable) __attribute__((weak));
void wls_transport_enable(bool enable) {

    if (enable) {
        wls_transport_attempt:
        if (host_get_driver() != &wireless_driver) {
            host_set_driver(&wireless_driver);
        }
        else {
            goto wls_transport_attempt;
        }
    } else {
        if (*md_getp_state() == MD_STATE_CONNECTED) {
            driver_release_all_keys(&wireless_driver);
        }
    }
}

/* Control USB device connection and disconnection by
 * controlling the power supply of the USB DP pull-up resistor.
 * Overwrite these two functions. */
void usb_power_connect(void) __attribute__((weak));
void usb_power_connect(void) {}

void usb_power_disconnect(void) __attribute__((weak));
void usb_power_disconnect(void) {}

void usb_transport_enable(bool enable) __attribute__((weak));
void usb_transport_enable(bool enable) {

    if (enable) {
        usb_transport_attempt:
        if (host_get_driver() != &chibios_driver) {
            extern bool last_suspend_state;
            /* This flag is not set to 1 with probability after usb restart */
            last_suspend_state = true;
    #if !defined(KEEP_USB_CONNECTION_IN_WIRELESS_MODE)
            usb_power_connect();
            restart_usb_driver(&USBD1);
    #endif
            host_set_driver(&chibios_driver);
        }
        else {
            goto usb_transport_attempt;
        }
    } else {
        if (USB_DRIVER.state == USB_ACTIVE) {
            driver_release_all_keys(&chibios_driver);
        }

#if !defined(KEEP_USB_CONNECTION_IN_WIRELESS_MODE)
        usbStop(&USBD1);
        usbDisconnectBus(&USBD1);
        usb_power_disconnect();
#endif
    }
}

void set_transport(transport_t new_transport) {
    if (transport != new_transport) {
        transport = new_transport;

        switch (transport) {
            case TRANSPORT_USB: {
                usb_transport_enable(true);
                wls_transport_enable(false);
            } break;
            case TRANSPORT_WLS: {
                wls_transport_enable(true);
                usb_transport_enable(false);
            } break;
            default:
                break;
        }
    }
}

transport_t get_transport(void) {
    return transport;
}

void usb_remote_wakeup(void) {

    static uint32_t suspend_timer = 0x00;

    if (USB_DRIVER.state == USB_SUSPENDED) {
        if (!suspend_timer) suspend_timer = sync_timer_read32();
        if (sync_timer_elapsed32(suspend_timer) >= USB_POWER_DOWN_DELAY) {
            suspend_timer = 0x00;
            suspend_power_down();
        }
    } else {
        suspend_timer = 0x00;
    }
}

void usb_remote_host(void) {

    if (USB_DRIVER.state == USB_SUSPENDED) {
        if ((USB_DRIVER.status & 2U) && suspend_wakeup_condition()) {
            usbWakeupHost(&USB_DRIVER);
#    if USB_SUSPEND_WAKEUP_DELAY > 0
            // Some hubs, kvm switches, and monitors do
            // weird things, with USB device state bouncing
            // around wildly on wakeup, yielding race
            // conditions that can corrupt the keyboard state.
            //
            // Pause for a while to let things settle...
            wait_ms(USB_SUSPEND_WAKEUP_DELAY);
#    endif
        }

#    if USB_POWER_DOWN_DELAY
        suspend_wakeup_init();
#    endif
    }
}

bool pre_process_record_kb(uint16_t keycode, keyrecord_t *record) {
    (void)keycode;
    (void)record;

    if (get_transport() == TRANSPORT_USB){
        usb_remote_host();
    }

    return true;
}
