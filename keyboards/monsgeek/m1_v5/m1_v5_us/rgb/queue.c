// Copyright 2025 tripoloskies (https://github.com/tripoloskies)
// SPDX-License-Identifier: GPL-2.0-or-later
#include QMK_KEYBOARD_H
#include <stdint.h>
#include <stdbool.h>
#include "quantum.h"
#include <stdlib.h>
#include "queue.h"

static indicator_queue_t indicator_queue[MAX_INDICATOR_QUEUE_SLOTS];


static bool is_queue_already_executed = false;


bool rgb_queue_is_full(indicator_queue_t* queue) {
    return queue->rear >= MAX_INDICATOR_QUEUE;
}
bool rgb_queue_is_empty(indicator_queue_t* queue) {
    return queue->front == queue->rear - 1;
}

void rgb_queue_enqueue(indicator_queue_t* queue, indicator_frame_t frame) {
    if (rgb_queue_is_full(queue)) {
        return;
    }
    queue->frames[queue->rear] = frame;
    queue->rear++;
}

void rgb_queue_dequeue(indicator_queue_t* queue) {
    if (rgb_queue_is_empty(queue)) {
        return;
    }
    queue->front++;
}

indicator_frame_t* rgb_queue_peek(indicator_queue_t* queue) {
    if (rgb_queue_is_empty(queue)) {
        return 0x00;
    }
    return &queue->frames[queue->front + 1];
}



void rgb_queue_frame_init(indicator_frame_t* frame) {
    frame->active = false;
    frame->timer = -1;
    frame->interval = 0;
}

void rgb_queue_queue_init(indicator_queue_t* queue) {

    if (queue != NULL) {
        queue->front = -1;
        queue->rear = 0;
        queue->callback = NULL;
        queue->loop = false;
        rgb_queue_frame_init(queue->frames);
    }
    else {
        for (uint8_t x = 0; x < MAX_INDICATOR_QUEUE_SLOTS; x++) {
            indicator_queue[x].front = -1;
            indicator_queue[x].rear = 0;

            for (uint8_t i = 0; i < MAX_INDICATOR_QUEUE; i++) {
                rgb_queue_frame_init(&indicator_queue[x].frames[i]);
            }
        }
    }

}

void rgb_queue_queue_reuse(indicator_queue_t* queue) {
    queue->front = -1;
    for (uint8_t i = 0; i < queue->rear; i++) {
        queue->frames[i].active = true;
        queue->frames[i].timer = -1;
    }
}

void rgb_queue_init() {
    rgb_queue_queue_init(NULL);
}

void rgb_queue_frame_queue(enum indicator_slots slot_index, uint8_t index, rgb_t color, uint32_t interval, uint8_t blink_times, void (*callback)(indicator_queue_t*)) {
    indicator_frame_t dat_frame = {
        .active = true,
        .color = color,
        .index = index,
        .interval = interval,
        .timer = -1,
    };
    indicator_frame_t dat_frame_blink = {
        .active = true,
        .color = (rgb_t) {0, 0, 0},
        .index = index,
        .interval = interval,
        .timer = -1
    };

    if (slot_index < 0 || slot_index > MAX_INDICATOR_QUEUE_SLOTS) {
        return;
    }

    if (is_queue_already_executed) {
        rgb_queue_queue_init(&indicator_queue[slot_index]);
    }


    indicator_queue[slot_index].callback = callback;

    if (!blink_times) {
        rgb_queue_enqueue(&indicator_queue[slot_index], dat_frame);

    }
    else {
        for (uint8_t i = 0; i <= blink_times; i++) {
            rgb_queue_enqueue(&indicator_queue[slot_index], dat_frame);
            rgb_queue_enqueue(&indicator_queue[slot_index], dat_frame_blink);
        }
    }
    is_queue_already_executed = false;

}


void rgb_queue_render(indicator_queue_t* queue) {

    is_queue_already_executed = true;

    if (rgb_queue_is_empty(queue)) {
        if (queue->rear > 0) {
            if (queue->loop) {
                rgb_matrix_set_color(0x00, 0xFF, 0, 0);
                rgb_queue_queue_reuse(queue);
            }
            else {

                rgb_matrix_set_color(0x00, 0x00, 0xFF, 0);
                rgb_queue_queue_init(queue);
            }
        }
        return;
    }

    indicator_frame_t* latest_queue = rgb_queue_peek(queue);

    if (latest_queue == 0x00) {
        return;
    }

    if (!latest_queue->active) {
        rgb_queue_frame_init(latest_queue);
        rgb_queue_dequeue(queue);
        return;
    }


    if (latest_queue->timer == -1) {
        latest_queue->timer = timer_read32();
    }


    if (timer_elapsed32(latest_queue->timer) >= latest_queue->interval) {
        if (queue->callback != NULL) {
            (*queue->callback)(queue);
        }
        latest_queue->active = false;
        latest_queue->timer = 0;
        rgb_queue_dequeue(queue);
    }

    rgb_matrix_set_color(2, 0xFF, 0, 0xFF);
    if (latest_queue->index == 0xFF) {
        rgb_matrix_set_color_all(latest_queue->color.r, latest_queue->color.g, latest_queue->color.b);
    }
    else {
        rgb_matrix_set_color(latest_queue->index, latest_queue->color.r, latest_queue->color.g, latest_queue->color.b);
    }

}

void rgb_queue_execute(void) {
    for (uint8_t i = 0; i < MAX_INDICATOR_QUEUE_SLOTS; i++) {
        rgb_queue_render(&indicator_queue[i]);
    }
}
