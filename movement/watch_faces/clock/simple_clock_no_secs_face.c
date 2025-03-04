/*
 * MIT License
 *
 * Copyright (c) 2024 Bill Bateman
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

#include <stdlib.h>
#include <string.h>
#include "simple_clock_no_secs_face.h"
#include "watch.h"
#include "watch_utility.h"
#include "watch_private_display.h"

static void _update_alarm_indicator(bool settings_alarm_enabled, simple_clock_no_secs_state_t *state) {
    state->alarm_enabled = settings_alarm_enabled;
    if (state->alarm_enabled) watch_set_indicator(WATCH_INDICATOR_BELL);
    else watch_clear_indicator(WATCH_INDICATOR_BELL);
}

void simple_clock_no_secs_face_setup(movement_settings_t *settings, uint8_t watch_face_index, void ** context_ptr) {
    (void) settings;
    (void) watch_face_index;
    if (*context_ptr == NULL) {
        // only happens at boot
        *context_ptr = malloc(sizeof(simple_clock_no_secs_state_t));
        memset(*context_ptr, 0, sizeof(simple_clock_no_secs_state_t));
        simple_clock_no_secs_state_t *state = (simple_clock_no_secs_state_t *)*context_ptr;
        state->show_seconds = true;
    }
    // whenever watch wakes from deep sleep
    // ...
}

void simple_clock_no_secs_face_activate(movement_settings_t *settings, void *context) {
    (void) settings;
    simple_clock_no_secs_state_t *state = (simple_clock_no_secs_state_t *)context;

    if (watch_tick_animation_is_running()) watch_stop_tick_animation();
    if (settings->bit.clock_mode_24h) watch_set_indicator(WATCH_INDICATOR_24H);

    // show alarm indicator if there is an active alarm
    _update_alarm_indicator(settings->bit.alarm_enabled, state);
    watch_set_colon();

    // this ensures that none of the timestamp fields will match, so we can re-render them all.
    state->previous_date_time = 0xFFFFFFFF;
}

bool simple_clock_no_secs_face_loop(movement_event_t event, movement_settings_t *settings, void *context) {
    simple_clock_no_secs_state_t *state = (simple_clock_no_secs_state_t *)context;

    char buf[11];
    uint8_t pos;
    watch_date_time date_time;
    uint32_t previous_date_time;

    switch (event.event_type) {
        case EVENT_ACTIVATE:
        case EVENT_TICK:
        case EVENT_LOW_ENERGY_UPDATE:
            // update display
            date_time = watch_rtc_get_date_time();
            previous_date_time = state->previous_date_time;
            state->previous_date_time = date_time.reg;

            if ((date_time.reg >> 6) == (previous_date_time >> 6) && event.event_type != EVENT_LOW_ENERGY_UPDATE && state->show_seconds) {
                // everything before seconds is the same, don't waste cycles setting those segments.
                watch_display_character_lp_seconds('0' + date_time.unit.second / 10, 8);
                watch_display_character_lp_seconds('0' + date_time.unit.second % 10, 9);
                break;
            } else if ((date_time.reg >> 12) == (previous_date_time >> 12) && event.event_type != EVENT_LOW_ENERGY_UPDATE && state->show_seconds) {
                // everything before minutes is the same.
                pos = 6;
                sprintf(buf, "%02d%02d", date_time.unit.minute, date_time.unit.second);
            } else if ((date_time.reg >> 12) == (previous_date_time >> 12) && (event.event_type == EVENT_LOW_ENERGY_UPDATE || !state->show_seconds)) {
                // everything before minutes is the same, and we're not showing seconds
                pos = 6;
                sprintf(buf, "%02d  ", date_time.unit.minute);
            } else {
                // other stuff changed; let's do it all.
                if (!settings->bit.clock_mode_24h) {
                    // if we are in 12 hour mode, do some cleanup.
                    if (date_time.unit.hour < 12) {
                        watch_clear_indicator(WATCH_INDICATOR_PM);
                    } else {
                        watch_set_indicator(WATCH_INDICATOR_PM);
                    }
                    date_time.unit.hour %= 12;
                    if (date_time.unit.hour == 0) date_time.unit.hour = 12;
                }
                pos = 0;
                if (event.event_type == EVENT_LOW_ENERGY_UPDATE || !state->show_seconds) {
                    sprintf(buf, "%s%2d%2d%02d  ", watch_utility_get_weekday(date_time), date_time.unit.day, date_time.unit.hour, date_time.unit.minute);
                } else {
                    sprintf(buf, "%s%2d%2d%02d%02d", watch_utility_get_weekday(date_time), date_time.unit.day, date_time.unit.hour, date_time.unit.minute, date_time.unit.second);
                }
            }
            // only show tick animation if we are low energy and want to show seconds
            if (event.event_type == EVENT_LOW_ENERGY_UPDATE && state->show_seconds) {
                if (!watch_tick_animation_is_running()) watch_start_tick_animation(500);
            } else {
                if (watch_tick_animation_is_running()) watch_stop_tick_animation();
            }
            watch_display_string(buf, pos);
            
            // update alarm indicator
            if (state->alarm_enabled != settings->bit.alarm_enabled) _update_alarm_indicator(settings->bit.alarm_enabled, state);
            
            // show signal when asleep
            if (event.event_type == EVENT_LOW_ENERGY_UPDATE) watch_set_indicator(WATCH_INDICATOR_SIGNAL);
            else watch_clear_indicator(WATCH_INDICATOR_SIGNAL);

            break;
        case EVENT_ALARM_BUTTON_UP:
            // toggle seconds
            state->show_seconds = !state->show_seconds;
            
            date_time = watch_rtc_get_date_time();
            pos = 8;
            if (state->show_seconds) sprintf(buf, "%02d", date_time.unit.second);
            else sprintf(buf, "  ");
            watch_display_string(buf, pos);
            break;
        case EVENT_ALARM_LONG_UP:
            // go to sleep
            movement_force_sleep();
            break;
        default:
            return movement_default_loop_handler(event, settings);
    }

    return true;
}

void simple_clock_no_secs_face_resign(movement_settings_t *settings, void *context) {
    (void) settings;
    (void) context;
}

