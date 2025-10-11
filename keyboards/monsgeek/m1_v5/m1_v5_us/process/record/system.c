#include QMK_KEYBOARD_H
#include "rgb/control.h"
#include "record.h"
#include "module.h"

bool process_record_system(uint16_t keycode, keyrecord_t *record) {


#ifdef NKRO_ENABLE
    switch (keycode) {
        case NK_TOGG: {
            if (record->event.pressed) {
                rgb_nkro_toggle();
            }
        } break;
    }
#endif

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

        case EE_CLR: {
            factory_reset_timer = record->event.pressed ? timer_read32() : 0;
            return false;
        } break;
    }
    return true;
}
