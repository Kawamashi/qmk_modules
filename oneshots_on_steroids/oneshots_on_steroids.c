/* Copyright 2025 @Kawamashi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "oneshots_on_steroids.h"

ASSERT_COMMUNITY_MODULES_MIN_API_VERSION(1, 0, 0);

#ifdef NO_ACTION_ONESHOT
#error "Oneshots on Steroids can't work when NO_ACTION_ONESHOT is defined. Please undefined NO_ACTION_ONESHOT."
#endif  // NO_ACTION_ONESHOT

oneshot_state_t oneshot_state[OS_STEROIDS_COUNT] = { [0 ... OS_STEROIDS_COUNT - 1] = os_idle };

static uint16_t idle_timer = 0;

uint16_t oneshot_holding_time[OS_STEROIDS_COUNT] = { [0 ... OS_STEROIDS_COUNT - 1] = 0 };
uint8_t oneshot_origin_layer[OS_STEROIDS_COUNT] = { [0 ... OS_STEROIDS_COUNT - 1] = 0 };

#ifdef ONESHOT_TIMEOUT
    void housekeeping_task_oneshots_on_steroids(void) {
        if (idle_timer && timer_expired(timer_read(), idle_timer)) {
            clear_all_oneshots_on_steroids();
            idle_timer = 0;
        }
    }
#endif  // ONESHOT_TIMEOUT

void clear_oneshot_on_steroids(uint8_t index) {
    oneshot_state[index] = os_idle;
    if (oneshot[index].modifier != KC_NO) { del_oneshot_mods(oneshot[index].modifier); }
    if (oneshot[index].layer != 0) { layer_off(oneshot[index].layer); }
}

void clear_all_oneshots_on_steroids(void) {
    for (uint8_t i = 0; i < OS_STEROIDS_COUNT; i++) {
        if (oneshot_state[i] == os_up_queued) { clear_oneshot_on_steroids(i); }
    }
}

void clear_oneshot_layer_on_steroids(uint8_t layer) {
    for (uint8_t i = 0; i < OS_STEROIDS_COUNT; i++) {
        if (oneshot[i].layer == layer && oneshot_state[i] != os_idle) {
            clear_oneshot_on_steroids(i);
        }
    }
}

void clear_all_oneshot_mod_on_steroids() {
    for (uint8_t i = 0; i < OS_STEROIDS_COUNT; i++) {
        if (oneshot[i].modifier != KC_NO && oneshot_state[i] != os_idle) {
            clear_oneshot_on_steroids(i);
        }
    }
}

bool is_oneshot_on_steroids(uint16_t keycode) {
    for (uint8_t i = 0; i < OS_STEROIDS_COUNT; i++) {
        if (keycode == oneshot[i].trigger) { return true; }
    }
    return false;
}


bool process_record_oneshots_on_steroids(uint16_t keycode, keyrecord_t *record){
    
    bool should_continue_processing = true;

    for (uint8_t i = 0; i < OS_STEROIDS_COUNT; i++) {

        if (record->event.pressed) {

            if (keycode == oneshot[i].trigger) {
                // Trigger keydown
                if (oneshot_state[i] == os_idle) {
                    if (oneshot[i].modifier != KC_NO) { register_mods(oneshot[i].modifier); }
                    if (oneshot[i].layer != 0) { 
                        uint8_t key_layer = read_source_layers_cache(record->event.key);
                        uint8_t default_layer = get_highest_layer(default_layer_state);
                        if (key_layer != default_layer) {
                            oneshot_origin_layer[i] = key_layer;
                            layer_off(key_layer);
                        } 
                        layer_on(oneshot[i].layer);
                    }
                    oneshot_holding_time[i] = timer_read();
                    oneshot_state[i] = os_down_unused;
                } else {
                    clear_oneshot_on_steroids(i);
                    if (oneshot_origin_layer[i] != 0) { layer_on(oneshot_origin_layer[i]); }
                }
                return false;
            }

        } else {

            if (oneshot_state[i] != os_idle) {

                if (keycode == oneshot[i].suppressor){

                    if (oneshot[i].modifier != KC_NO) { unregister_mods(oneshot[i].modifier); }

                    if (oneshot_state[i] == os_down_unused && timer_elapsed(oneshot_holding_time[i]) < TAPPING_TERM) {
                        oneshot_state[i] = os_up_queued;
                        if (oneshot[i].modifier != KC_NO) { set_oneshot_mods(oneshot[i].modifier); }
                        #ifdef ONESHOT_TIMEOUT
                          idle_timer = (record->event.time + ONESHOT_TIMEOUT) | 1;
                        #endif  // ONESHOT_TIMEOUT
                    } else {
                        oneshot_state[i] = os_idle;
                        if (oneshot[i].layer != 0) {layer_off(oneshot[i].layer); }
                    }
                    should_continue_processing = false;
                    continue;
                }
            }
            if (keycode == oneshot[i].trigger) { return false; }
        }
    }
    if (!should_continue_processing) { return false; }

    for (uint8_t i = 0; i < OS_STEROIDS_COUNT; i++) {
        if (oneshot_state[i] == os_idle) { continue; }

        if (is_oneshot_on_steroids_cancel_key(keycode)) {
            if (record->event.pressed) {
                // Cancel oneshot on press of specific keys.
                clear_oneshot_on_steroids(i);
            }
            continue;
        }

        if (should_oneshot_on_steroids_stay_pressed(keycode)) {
            if (record->event.pressed) {
                #ifdef ONESHOT_TIMEOUT
                  idle_timer = (record->event.time + ONESHOT_TIMEOUT) | 1;
                #endif  // ONESHOT_TIMEOUT
            }
            continue;
        }

        if (record->event.pressed) {
            // Regular key pressed
            switch (oneshot_state[i]) {
                case os_down_unused:
                    // When the mod key is still pressed
                    oneshot_state[i] = os_down_used;
                    break;
                case os_up_queued:
                    oneshot_state[i] = os_up_queued_used;
                    break;
                default:
                    break;
            }

        }
    }
    return true;
}

void post_process_record_oneshots_on_steroids(uint16_t keycode, keyrecord_t *record){

    // In post_process_record instead of process_record because of a bug
    // when rolling two keys, the first one affected by a custom oneshot
    // and the second one being a mod-tap on base layer.

    for (uint8_t i = 0; i < OS_STEROIDS_COUNT; i++) {
        if (oneshot_state[i] == os_up_queued_used) {
            oneshot_state[i] = os_idle;
            if (oneshot[i].layer != 0) { layer_off(oneshot[i].layer); }
            continue;
        }
    }
}

__attribute__((weak)) bool is_oneshot_on_steroids_cancel_key(uint16_t keycode) {
    switch (keycode) {

        default:
            return false;
    }
}

__attribute__((weak)) bool should_oneshot_on_steroids_stay_pressed(uint16_t keycode) {
    switch (keycode) {

        default:
            return false;
  }
}