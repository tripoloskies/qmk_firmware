// Copyright 2025 tripoloskies (https://github.com/tripoloskies)
// Copyright 2024 yangzheng20003 (@yangzheng20003)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "quantum.h"
#include "module.h"
#include "rgb/rgb.h"
#include "fwconfig/fwconfig.h"
#include "wireless.h"
#include "lowpower.h"


static ioline_t col_pins[MATRIX_COLS] = MATRIX_COL_PINS;
bool lower_sleep;

void lpwr_wakeup_reinstate_devs(void) {
    *md_getp_state() = MD_STATE_DISCONNECTED;
    *md_getp_indicator() = 0;
    driver_release_all_keys(&chibios_driver);
    md_devs_change(wireless_get_current_devs(), false);
}

void lpwr_presleep_hook(void) {
    md_devs_change(DEVS_USB, false);
}

void lpwr_wakeup_hook(void) {

    lpwr_wakeup_reinstate_devs();
    rgb_control_brightness_state();
    gpio_write_pin_high(HS_LED_BOOSTING_PIN);
}

bool lpwr_is_allow_timeout_hook(void) {
    return !(wireless_get_current_devs() == DEVS_USB);
}


void lpwr_exti_init_hook(void) {
    if (lower_sleep) {
        #if DIODE_DIRECTION == ROW2COL
                for (uint8_t i = 0; i < ARRAY_SIZE(col_pins); i++) {
                    if (col_pins[i] != NO_PIN) {
                        setPinOutput(col_pins[i]);
                        writePinHigh(col_pins[i]);
                    }
                }
        #endif
    }
    setPinInput(HS_BAT_CABLE_PIN);
    waitInputPinDelay();
    palEnableLineEvent(HS_BAT_CABLE_PIN, PAL_EVENT_MODE_RISING_EDGE);
}

void palcallback_cb(uint8_t line) {
    switch (line) {
        case PAL_PAD(HS_BAT_CABLE_PIN): {
            lpwr_set_sleep_wakeupcd(LPWR_WAKEUP_CABLE);
        } break;
    }
}

void lpwr_stop_hook_pre(void) {

    gpio_write_pin_low(LED_POWER_EN_PIN);
    gpio_write_pin_low(A9);
    gpio_write_pin_low(HS_LED_BOOSTING_PIN);

    if (lower_sleep) {
        lpwr_set_sleep_wakeupcd(LPWR_WAKEUP_UART);
    }
}

void lpwr_stop_hook_post(void) {
    if (lower_sleep) {
        switch (lpwr_get_sleep_wakeupcd()) {
            case LPWR_WAKEUP_USB:
            case LPWR_WAKEUP_CABLE: {
                lower_sleep = false;
                lpwr_set_state(LPWR_WAKEUP);
            } break;
            default: {
                lpwr_set_state(LPWR_STOP);
            } break;
        }
    }
}
