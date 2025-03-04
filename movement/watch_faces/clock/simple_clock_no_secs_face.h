/*
 * MIT License
 *
 * Copyright (c) 2024 Bill Batemman
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef SIMPLE_CLOCK_NO_SECS_FACE_H_
#define SIMPLE_CLOCK_NO_SECS_FACE_H_

#include "movement.h"

/*
 * SIMPLE CLOCK NO SECONDS FACE
 *
 * Displays the current time, matching the original operation of the watch.
 * Based on simple clock face. No hourly chime.
 *
 * Press ALARM to toggle seconds/tick.
 */

typedef struct {
    uint32_t previous_date_time;
    bool show_seconds;
    bool alarm_enabled;
} simple_clock_no_secs_state_t;

void simple_clock_no_secs_face_setup(movement_settings_t *settings, uint8_t watch_face_index, void ** context_ptr);
void simple_clock_no_secs_face_activate(movement_settings_t *settings, void *context);
bool simple_clock_no_secs_face_loop(movement_event_t event, movement_settings_t *settings, void *context);
void simple_clock_no_secs_face_resign(movement_settings_t *settings, void *context);

#define simple_clock_no_secs_face ((const watch_face_t){ \
    simple_clock_no_secs_face_setup, \
    simple_clock_no_secs_face_activate, \
    simple_clock_no_secs_face_loop, \
    simple_clock_no_secs_face_resign, \
    NULL, \
})

#endif // SIMPLE_CLOCK_NO_SECS_FACE_H_

