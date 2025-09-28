// Copyright 2025 tripoloskies (https://github.com/tripoloskies)
// Copyright 2024 yangzheng20003 (@yangzheng20003)
// SPDX-License-Identifier: GPL-2.0-or-later
#include QMK_KEYBOARD_H
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "m1_v5_us.h"
#include "config.h"
#include "module.h"

#include "fwconfig/fwconfig.h"
#include "process/process.h"


#ifdef RGB_MATRIX_ENABLE
#    include "rgb/rgb.h"
#endif

#ifdef WIRELESS_ENABLE
#    include "wireless.h"
#    include "usb_main.h"
#    include "lowpower.h"
#endif




enum layers {
    _BL = 0,
    _FL,
    _MBL,
    _MFL,
    _FBL,
};



static uint32_t post_init_timer = 0x00;

bool enable_bat_indicators = true;
static bool battery_low_flag = false;

uint32_t ee_clr_timer = 0;
uint32_t bat_indicator_cnt   = true;
uint32_t hs_ct_time;
RGB rgb_test_open;


////////////////////////////////////////////////////////////////////////////
///     QMK Events After Firmware Loaded                                 ///
////////////////////////////////////////////////////////////////////////////

/*
    keyboard_post_init_kb()

    This is where the essential functions (configurations from EEPROM, including console [if enable], backlights, wireless connection, and USB ports)
    activate. It only runs after the firmware was loaded.
*/
void keyboard_post_init_kb(void) {

#ifdef CONSOLE_ENABLE
    debug_enable = true;
    debug_matrix = true;
    debug_keyboard = true;
#endif

    kbconfig_init();

#ifdef LED_POWER_EN_PIN
    gpio_set_pin_output(LED_POWER_EN_PIN);
    if (rgb_matrix_get_val() != 0) {
        gpio_write_pin_high(LED_POWER_EN_PIN);
    }
    gpio_set_pin_output(HS_LED_BOOSTING_PIN);
    gpio_write_pin_high(HS_LED_BOOSTING_PIN);
#endif

#ifdef USB_POWER_EN_PIN
    usb_power_connect();
    gpio_set_pin_output(USB_POWER_EN_PIN);
#endif

#ifdef HS_BAT_CABLE_PIN
    setPinInput(HS_BAT_CABLE_PIN);
#endif

#ifdef BAT_FULL_PIN
    setPinInputHigh(BAT_FULL_PIN);
#endif
    setPinInputHigh(SYSTEM_WIN_PIN);
    setPinInputHigh(SYSTEM_MAC_PIN);

#ifdef WIRELESS_ENABLE
    wireless_init();
    post_init_timer = timer_read32();
#endif
    rgb_control_init();
    keyboard_post_init_user();
}



////////////////////////////////////////////////////////////////////////////
///     Wireless-Specific Functions                                      ///
////////////////////////////////////////////////////////////////////////////

#ifdef WIRELESS_ENABLE

void usb_power_connect(void) {
#    ifdef USB_POWER_EN_PIN
        gpio_write_pin_low(USB_POWER_EN_PIN);
#    endif
}

void usb_power_disconnect(void) {
#   ifdef USB_POWER_EN_PIN
        gpio_write_pin_high(USB_POWER_EN_PIN);
#   endif
}

void suspend_power_down_kb(void) {
#   ifdef LED_POWER_EN_PIN
    gpio_write_pin_low(LED_POWER_EN_PIN);
#   endif

    suspend_power_down_user();
}


void suspend_wakeup_init_kb(void) {
#ifdef LED_POWER_EN_PIN
    rgb_control_brightness_state();
#endif
    suspend_wakeup_init_user();
}


// [Bugfix/Hacks] for wireless mode not working when keypress after rebooting the keyboard.
// Give some time while initializing something tri-mode related (wls drivers, usb drivers, suichan drivers*, etc...) before switching to the current devs.
// and to set sleep duration for all wireless modes.
void wireless_post_task(void) {
    if (post_init_timer && timer_elapsed32(post_init_timer) >= 300) {
        md_send_devctrl(MD_SND_CMD_DEVCTRL_FW_VERSION);   // get the module fw version.
        md_send_devctrl(MD_SND_CMD_DEVCTRL_SLEEP_BT_EN);  // timeout 30min to sleep in bt mode, enable
        md_send_devctrl(MD_SND_CMD_DEVCTRL_SLEEP_2G4_EN); // timeout 30min to sleep in 2.4g mode, enable
        wireless_devs_change(!kbconfig.devs, kbconfig.devs, false);
        post_init_timer = 0;
    }
}


/*
    wls_process_long_press()

    (CALLBACK FUNCTION)
    Enters wireless pairing mode when the use long-presses bluetooth or 2.4G wireless key.
*/
uint32_t wls_process_long_press(uint32_t trigger_time, void *cb_arg) {
    uint16_t keycode = *((uint16_t *)cb_arg);

    switch (keycode) {
        case KC_BT1:
            wireless_devs_change(wireless_get_current_devs(), DEVS_BT1, true);
        break;
        case KC_BT2:
            wireless_devs_change(wireless_get_current_devs(), DEVS_BT2, true);
        break;
        case KC_BT3:
            wireless_devs_change(wireless_get_current_devs(), DEVS_BT3, true);
        break;
        case KC_2G4:
            wireless_devs_change(wireless_get_current_devs(), DEVS_2G4, true);
        break;
    }

    return 0;
}




void wls_process_long_press_countdown(uint16_t *keycode, uint8_t wls_device, bool pressed) {
    static uint16_t keycode_shadow = 0x00;
    static deferred_token wls_process_long_press_token = INVALID_DEFERRED_TOKEN;

    keycode_shadow = *keycode;

    if (pressed) {
        if (wireless_get_current_devs() != wls_device)
            wireless_devs_change(wireless_get_current_devs(), wls_device, false);

        if (wls_process_long_press_token == INVALID_DEFERRED_TOKEN) {
            wls_process_long_press_token = defer_exec(WLS_KEYCODE_PAIR_TIME, wls_process_long_press, &keycode_shadow);
        }
    } else {
        cancel_deferred_exec(wls_process_long_press_token);
        wls_process_long_press_token = INVALID_DEFERRED_TOKEN;
    }

}


bool process_record_wls(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case KC_BT1:
            wls_process_long_press_countdown(&keycode, DEVS_BT1, record->event.pressed);
        break;
        case KC_BT2:
            wls_process_long_press_countdown(&keycode, DEVS_BT2, record->event.pressed);
        break;
        case KC_BT3:
            wls_process_long_press_countdown(&keycode, DEVS_BT3, record->event.pressed);
        break;
        case KC_2G4:
            wls_process_long_press_countdown(&keycode, DEVS_2G4, record->event.pressed);
        break;
        case KC_USB:
            wls_process_long_press_countdown(&keycode, DEVS_USB, record->event.pressed);
        break;
        default:
            return true;
    }
    return false;
}

#endif


////////////////////////////////////////////////////////////////////////////
///     QMK Key-press hooks                                              ///
////////////////////////////////////////////////////////////////////////////

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {

#ifdef WIRELESS_ENABLE
    if (process_record_wls(keycode, record) != true) {
        return false;
    }
#endif
    if (
    process_record_arrow_swap(keycode, record) != true ||
    process_record_rgb_controls(keycode, record) != true
    ) {
        return false;
    }



    switch (keycode) {
        case QK_BOOT: {
            if (record->event.pressed) {
                dprintf("into boot!!!\r\n");
                eeconfig_disable();
                bootloader_jump();
            }
        } break;

        case BT_TEST: {
            if (record->event.pressed) {
                md_send_devctrl(MD_SND_CMD_DEVCTRL_FORCED_PAIRING_BT);
            }
            return false;
        } break;
        case NK_TOGG: {
#ifdef NKRO_ENABLE
            if (record->event.pressed) {
                rgb_nkro_toggle();
            }
#endif
        } break;
        case EE_CLR: {
            if (record->event.pressed) {
                ee_clr_timer = timer_read32();
            } else {
                ee_clr_timer = 0;
            }

            return false;
        } break;

        case KC_LCMD: {
            if (keymap_is_mac_system()) {
                if (keymap_config.no_gui) {
                    if (record->event.pressed) {
                        register_code16(KC_LCMD);
                    } else {
                        unregister_code16(KC_LCMD);
                    }
                }
            }

            return true;
        } break;
        case KC_RCMD: {
            if (keymap_is_mac_system()) {
                if (keymap_config.no_gui) {
                    if (record->event.pressed) {
                        register_code16(KC_RCMD);
                    } else {
                        unregister_code16(KC_RCMD);
                    }
                }
            }

            return true;
        } break;
        case HS_BATQ: {
            bat_req_flag = (kbconfig.devs != DEVS_USB) && record->event.pressed;
            return false;
        } break;
        case HS_DIR: {
            if (record->event.pressed) {
                kbconfig.arrow_swap_flag = !kbconfig.arrow_swap_flag;
                rgb_test_open     = hsv_to_rgb((HSV){.h = 0, .s = 0, .v = RGB_MATRIX_VAL_STEP * 5});
                rgb_queue_frame_queue(INDICATOR_QUEUE_MAIN, 0xFF, (rgb_t){rgb_test_open.r, rgb_test_open.g, rgb_test_open.b}, 250, 1, NULL);
                kbconfig_update();
            }
            return false;
        } break;
        case HS_CT_A: {
            if (record->event.pressed) {
                hs_ct_time = timer_read32();
            } else {
                hs_ct_time = 0;
            }
            return false;
        } break;
        case KC_RCTL: {
            if (kbconfig.ctrl_app_flag) {
                if (record->event.pressed) {
                    register_code16(KC_APP);
                } else {
                    unregister_code16(KC_APP);
                }
                return false;
            } else {
                return true;
            }
        } break;
        case HS_SIRI: {
            if (record->event.pressed) {
                uprintf("You called Siri?\n");
                register_code(KC_LCMD);
                register_code(KC_SPC);
                wait_ms(20);
            } else {
                unregister_code(KC_SPC);
                unregister_code(KC_LCMD);
            }
            return false;
        } break;
        case KC_MCTL: {
            if (record->event.pressed) {
                register_code(KC_LCTL);
                register_code(KC_UP);

            } else {
                unregister_code(KC_LCTL);
                unregister_code(KC_UP);
            }
            return false;
        } break;
        default:
            break;
    }

    return true;

}




////////////////////////////////////////////////////////////////////////////
///     QMK Housekeeping Function                                        ///
////////////////////////////////////////////////////////////////////////////
void housekeeping_task_user(void) {


    uint8_t hs_now_mode;
    static uint32_t hs_current_time;
    static uint32_t rgb_control_sync_time;
    bool usb_insert_flag;
    bool bat_full_flag;


    usb_insert_flag = readPin(HS_BAT_CABLE_PIN);
    bat_full_flag = readPin(BAT_FULL_PIN);

    if (usb_insert_flag && bat_full_flag) {
        hs_now_mode = MD_SND_CMD_DEVCTRL_CHARGING_DONE;
    } else if (usb_insert_flag) {
        hs_now_mode = MD_SND_CMD_DEVCTRL_CHARGING;
    } else {
        hs_now_mode = MD_SND_CMD_DEVCTRL_CHARGING_STOP;
    }

    // Synchronizes in-built RGB-related configurations and/or VIA rgb-related configurations to RGB control configurations.
    if (!rgb_control_sync_time || timer_elapsed32(rgb_control_sync_time > 50)) {
        rgb_control_sync_time = timer_read32();
        rgb_control_sync();
    }

    if (!hs_current_time || timer_elapsed32(hs_current_time) > 1000) {
        hs_current_time = timer_read32();
        md_send_devctrl(hs_now_mode);
        md_send_devctrl(MD_SND_CMD_DEVCTRL_INQVOL);
    }

    writePin(HS_LED_BOOSTING_PIN, (usb_insert_flag) ? 0 : 1);

    if (timer_elapsed32(hs_ct_time) > 3000 && hs_ct_time) {
        kbconfig.ctrl_app_flag = !kbconfig.ctrl_app_flag;
        rgb_test_open          = hsv_to_rgb((HSV){.h = 0, .s = 0, .v = RGB_MATRIX_VAL_STEP * 5});
        rgb_queue_frame_queue(INDICATOR_QUEUE_MAIN, 0xFF, (rgb_t){rgb_test_open.r, rgb_test_open.g, rgb_test_open.b}, 250, 1, NULL);
        kbconfig_update();
        hs_ct_time = 0;
    }


    if (!post_init_timer && wireless_get_current_devs() == DEVS_USB) {
        wireless_devs_change(wireless_get_current_devs(), wireless_get_current_devs(), usb_insert_flag);
    }


    if ((readPin(SYSTEM_WIN_PIN) != 0) && (readPin(SYSTEM_MAC_PIN) == 0)) { // mac system
        if (!keymap_is_mac_system()) {
            set_single_persistent_default_layer(_MBL);
            layer_move(0);
        }
    } else { // win system
        if (keymap_is_mac_system()) {
            set_single_persistent_default_layer(_BL);
            layer_move(0);
        }
    }
}

#ifdef RGB_MATRIX_ENABLE
#ifdef WIRELESS_ENABLE

bool wls_rgb_indicator_reset = false;

void wireless_devs_change_kb(uint8_t old_devs, uint8_t new_devs, bool reset) {

    wls_rgb_indicator_reset = reset;

    if (post_init_timer) {
        return;
    }

    if (kbconfig.devs != new_devs) {
        kbconfig.devs = new_devs;
        kbconfig_update();
    }
}


void wls_indicator_timeout_callback(indicator_queue_t* queue) {
    int8_t current_state = *md_getp_state();
    if (current_state != MD_STATE_CONNECTED) {
        if (wireless_get_current_devs() != DEVS_USB || USB_DRIVER.state != USB_ACTIVE) {
            queue->loop = true;
            return;
        }
    }
    led_wakeup();
    queue->loop = false;
}


void bat_indicators_timeout_callback(indicator_queue_t* queue) {
    queue->loop = battery_low_flag;
}

/*
    bat_indicators()

    Checks the charging/discharging state of the battery,
    Full charge or discharging = None
    Charging = Fn backlight turns to GREEN,
    Low battery (15%) = Fn Baclight blinks red every 250ms,
    Empty battery charge (0%) =
*/
void bat_indicators(void) {
    bool usb_insert_flag;
    bool bat_full_flag;
    static uint32_t battery_process_time = 0;
    static bool prev_battery_low_flag = false;


    usb_insert_flag = readPin(HS_BAT_CABLE_PIN);
    bat_full_flag = readPin(BAT_FULL_PIN);

    // Full Charge
    if (usb_insert_flag && bat_full_flag) {
        battery_low_flag = false;
        battery_process_time = 0;
    }
    // Charging
    else if (usb_insert_flag) {
        battery_process_time = 0;
        battery_low_flag = false;
        rgb_matrix_set_color(HS_MATRIX_BLINK_INDEX_BAT, 0x00, 0xFF, 0x00);
    }

    // Low Battery Capacity
    else if (*md_getp_bat() <= BATTERY_CAPACITY_LOW) {

        battery_low_flag = true;

        if (prev_battery_low_flag != battery_low_flag) {
            rgb_queue_frame_queue(
                INDICATOR_QUEUE_BAT,
                HS_MATRIX_BLINK_INDEX_BAT,
                (rgb_t){0xFF, 0x00, 0x00},
                250,
                1,
                bat_indicators_timeout_callback
            );
        }

        // Empty charge or no battery detected
        if (*md_getp_bat() <= BATTERY_CAPACITY_STOP) {
            if (!battery_process_time) {
                battery_process_time = timer_read32();
            }
            if (battery_process_time && timer_elapsed32(battery_process_time) > 60000) {
                battery_process_time = 0;
                lower_sleep          = true;
                lpwr_set_timeout_manual(true);
            }
        }
    } else {
        battery_low_flag = false;
        battery_process_time = 0;
    }
    prev_battery_low_flag = battery_low_flag;
}

uint32_t wls_indicator_interval_set(uint8_t state, bool reset) {
    if (state == MD_STATE_CONNECTED) {
        return 2000;
    }
    else {
        return (reset) ? 250 : 500;
    }
}

rgb_t wls_indicator_color_set(uint8_t devs, uint8_t state, bool reset) {
    switch (devs) {
        case DEVS_USB:
            return (reset) ? (rgb_t){HS_DISCOVER_COLOR_USB} : (rgb_t){HS_LBACK_COLOR_USB};
        break;
        case DEVS_BT1:
            if (state == MD_STATE_CONNECTED) {
                return (rgb_t) {HS_CONNECTED_COLOR_BT1};
            }
            else {
                return (reset) ? (rgb_t){HS_DISCOVER_COLOR_BT1} : (rgb_t){HS_LBACK_COLOR_BT1};
            }
        break;
        case DEVS_BT2:
            if (state == MD_STATE_CONNECTED) {
                return (rgb_t) {HS_CONNECTED_COLOR_BT2};
            }
            else {
                return (reset) ? (rgb_t){HS_DISCOVER_COLOR_BT2} : (rgb_t){HS_LBACK_COLOR_BT2};
            }
        break;
        case DEVS_BT3:
            if (state == MD_STATE_CONNECTED) {
                return (rgb_t) {HS_CONNECTED_COLOR_BT3};
            }
            else {
                return (reset) ? (rgb_t){HS_DISCOVER_COLOR_BT3} : (rgb_t){HS_LBACK_COLOR_BT3};
            }
        break;
        case DEVS_2G4:
            if (state == MD_STATE_CONNECTED) {
                return (rgb_t) {HS_CONNECTED_COLOR_2G4};
            }
            else {
                return (reset) ? (rgb_t){HS_DISCOVER_COLOR_2G4} : (rgb_t){HS_LBACK_COLOR_2G4};
            }
        break;
        default:
            return (rgb_t) {RGB_BLACK};
    }

}


void wls_indicator_trigger(void) {
    uint8_t indicator_index;
    static bool reset = false;
    static uint8_t devs = 0xFF;
    static uint8_t wls_state = 0xFF;

    if (wls_state == MD_STATE_CONNECTED) {
        wls_rgb_indicator_reset = false;
    }

    if (devs == kbconfig.devs && reset == wls_rgb_indicator_reset && wls_state == *md_getp_state()) {
        return;
    }

    wls_state = *md_getp_state();
    devs = kbconfig.devs;
    reset = wls_rgb_indicator_reset;

    switch (devs) {
        case DEVS_USB:
            indicator_index = HS_RGB_BLINK_INDEX_USB;
        break;
        case DEVS_BT1:
            indicator_index = HS_RGB_BLINK_INDEX_BT1;
        break;
        case DEVS_BT2:
            indicator_index = HS_RGB_BLINK_INDEX_BT2;
        break;
        case DEVS_BT3:
            indicator_index = HS_RGB_BLINK_INDEX_BT3;
        break;
        case DEVS_2G4:
            indicator_index = HS_RGB_BLINK_INDEX_2G4;
        break;
        default:
            return;
    }


    rgb_queue_frame_queue(
        INDICATOR_QUEUE_WLS,
        indicator_index,
        wls_indicator_color_set(devs, wls_state, reset),
        wls_indicator_interval_set(wls_state, reset),
        (wls_state == MD_STATE_CONNECTED) ? 0 : 1,
        (wls_state == MD_STATE_CONNECTED) ? NULL : wls_indicator_timeout_callback
    );
}

#endif
#endif



bool rgb_matrix_indicators_advanced_kb(uint8_t led_min, uint8_t led_max) {


    if (post_init_timer) {
        rgb_queue_execute();
        return true;
    }


    switch (lpwr_get_state()) {
        case LPWR_NORMAL:
            rgb_matrix_set_color(10, 0xFF, 0x00, 0x00);
        break;
        case LPWR_STOP:
            rgb_matrix_set_color(10, 0x00, 0xFF, 0x00);
        break;
        default:
            rgb_matrix_set_color(10, 0x00, 0xFF, 0xFF);
    }

    if (ee_clr_timer && timer_elapsed32(ee_clr_timer) > 3000) {
        factory_reset();
        enable_bat_indicators = false;
        ee_clr_timer = 0;
    }

    if (host_keyboard_led_state().caps_lock)
        rgb_matrix_set_color(HS_RGB_INDEX_CAPS, 0x20, 0x20, 0x20);

    if (!keymap_is_mac_system() && keymap_config.no_gui)
        rgb_matrix_set_color(HS_RGB_INDEX_WIN_LOCK, 0x20, 0x20, 0x20);


#ifdef WIRELESS_ENABLE

    wls_indicator_trigger();

    // Stop doing battery indicator monitoring if there's an ongoing factory reset.
    if (enable_bat_indicators) {
        bat_indicators();
        bat_indicator_cnt = timer_read32();
    }
    else {
        if (timer_elapsed32(bat_indicator_cnt) > 2000) {
            enable_bat_indicators = true;
            bat_indicator_cnt     = timer_read32();
        }
    }

#endif

    rgb_queue_execute();
    rgb_req_bat_percentage();

    return true;
}

