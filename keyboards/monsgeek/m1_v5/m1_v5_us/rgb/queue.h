#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "color.h"
#include "util.h"

#define MAX_INDICATOR_QUEUE 12
#define MAX_INDICATOR_QUEUE_SLOTS 3

typedef struct indicator_queue_s indicator_queue_t;

typedef struct{
    bool active;
    uint32_t interval;
    uint8_t index;
    rgb_t color;
    int32_t timer;
}indicator_frame_t;


struct indicator_queue_s {
    int8_t front;
    int8_t rear;
    bool loop;
    void (*callback)(indicator_queue_t*);
    indicator_frame_t frames[MAX_INDICATOR_QUEUE];
};

enum indicator_slots {
    INDICATOR_QUEUE_MAIN,
    INDICATOR_QUEUE_BAT,
    INDICATOR_QUEUE_WLS = MAX_INDICATOR_QUEUE_SLOTS - 1
};

void rgb_queue_init(void);
void rgb_queue_frame_queue(enum indicator_slots slot_index, uint8_t index, rgb_t color, uint32_t interval, uint8_t blink_times, void (*callback)(indicator_queue_t*));
void rgb_queue_execute(void);
