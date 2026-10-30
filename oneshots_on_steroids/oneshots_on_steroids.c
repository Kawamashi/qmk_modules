/* Copyright 2026 @Kawamashi
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
#error "One Shots on Steroids need vanilla One Shot Keys to work. Please undefine NO_ACTION_ONESHOT."
#endif  // NO_ACTION_ONESHOT


static oneshot_on_steroids_state_t oneshot_state[OS_STEROIDS_COUNT] = { [0 ... OS_STEROIDS_COUNT - 1] = os_idle };
static uint16_t oneshot_tap_time[OS_STEROIDS_COUNT] = { [0 ... OS_STEROIDS_COUNT - 1] = 0 };
static int8_t active_osl_index = -1;

oneshot_on_steroids_state_t get_oneshot_state(uint8_t index) {
    return oneshot_state[index];
}

int8_t get_active_osl_index(void) {
    return active_osl_index;
}


#   if defined OS_STEROIDS_ABSORB_MODS
static uint8_t oneshot_pressed_mods = 0;
static uint8_t oneshot_absorbed_mods = 0;

static bool should_unregister_mod(uint8_t index, uint8_t mod) {
    if (mod & oneshot_pressed_mods) {
        oneshot_pressed_mods &= ~mod;
        switch (oneshot_state[index]) {
            case os_down_unused:
            case os_up_queued:
                oneshot_absorbed_mods |= mod;
                return false;
            default:
                return true;
        }
    }
    return true;
}

static bool should_process_mod_release(uint8_t index, uint16_t keycode, keyrecord_t *record) {

    uint8_t mod_being_released = 0;
    if (IS_MODIFIER_KEYCODE(keycode)) {
        mod_being_released = MOD_BIT(keycode);

    } else if (IS_QK_MOD_TAP(keycode) && !record->tap.count) {
        mod_being_released = QK_MOD_TAP_GET_MODS(keycode);
        if ((mod_being_released & 0x10) != 0) { mod_being_released <<= 4; }

    } else if (IS_QK_ONE_SHOT_MOD(keycode) && !record->tap.count) {
        mod_being_released = QK_ONE_SHOT_MOD_GET_MODS(keycode);
        if ((mod_being_released & 0x10) != 0) { mod_being_released <<= 4; }

    } else if (IS_QK_LAYER_MOD(keycode)) {
        mod_being_released = QK_LAYER_MOD_GET_MODS(keycode);
        if ((mod_being_released & 0x10) != 0) { mod_being_released <<= 4; }
        layer_off(QK_LAYER_MOD_GET_LAYER(keycode));
    }

    if (mod_being_released) {
        return should_unregister_mod(index, mod_being_released);
    }
    return true;
}

uint8_t get_absorbed_mods(void) {
    return oneshot_absorbed_mods;
}

void set_absorbed_mods(uint8_t mods) {
    oneshot_absorbed_mods = mods;
}
#   endif  // OS_STEROIDS_ABSORB_MODS


#   ifdef OS_STEROIDS_FREE_LAYER_STACK
static uint8_t oneshot_origin_layer = 0;

// Handles `LT`, `MO`, `LM` and `TT` keys.
static bool should_process_layer_off(uint8_t key_layer) {
    if (key_layer == oneshot_origin_layer) {
        oneshot_origin_layer = 0;
        return false;  // Skip default handling.
    }
    return true;
}

static bool should_process_layer_release(uint8_t index, uint16_t keycode, keyrecord_t *record) {

    if (IS_QK_MOMENTARY(keycode)) {  // `MO` keys
        return should_process_layer_off(QK_MOMENTARY_GET_LAYER(keycode));
    }
    if (IS_QK_LAYER_TAP_TOGGLE(keycode)) {  // `TT` keys
        return should_process_layer_off(QK_LAYER_TAP_TOGGLE_GET_LAYER(keycode));
    }
    if (IS_QK_LAYER_TAP(keycode) && !record->tap.count) {  // `LT` keys
        return should_process_layer_off(QK_LAYER_TAP_GET_LAYER(keycode));
    }
    if (IS_QK_LAYER_MOD(keycode)) {  // `LM` keys
        if (!should_process_layer_off(QK_LAYER_MOD_GET_LAYER(keycode))) {
            unregister_mods(QK_LAYER_MOD_GET_MODS(keycode));
            return false;
        }
    }
    return true;
}
#   endif  // OS_STEROIDS_FREE_LAYER_STACK


#   if OS_STEROIDS_TIMEOUT > 0
static uint16_t idle_timer = 0;
static int8_t timed_oneshot_index = -1;

// One Shot on Steroids can be configured to deactivate if the keyboard is idle for some time.
// This is useful to prevent unexpected behaviors.
// In config.h, define `OS_STEROIDS_TIMEOUT` with a time in milliseconds.
void housekeeping_task_oneshots_on_steroids(void) {
    if (idle_timer && timer_expired(timer_read(), idle_timer)) {
        clear_oneshots_on_steroids();
    }
}

// Update the timeout of an OSoS key
static void set_oneshot_on_steroids_idle_timer(uint16_t keycode, keyrecord_t *record) {
    uint16_t timeout = get_oneshot_on_steroids_timeout(keycode, record);
    if (timeout) { idle_timer = (record->event.time + timeout) | 1; }
}
#   endif  // OS_STEROIDS_TIMEOUT > 0


void unregister_mods_on_steroids(uint8_t mods) {
#       ifdef DUMMY_MOD_NEUTRALIZER_KEYCODE
     neutralize_flashing_modifiers(mods);
#       endif // DUMMY_MOD_NEUTRALIZER_KEYCODE
    unregister_mods(mods);
}

static bool should_unregister_osm_on_steroids(int8_t index) {
#       ifdef OS_STEROIDS_ABSORB_MODS
    if (active_osl_index != -1 && index != active_osl_index) {
        return should_unregister_mod(active_osl_index, oneshot_os[index].modifier);
    }
#       endif  // OS_STEROIDS_ABSORB_MODS
    return true;
}

static bool automatic_release_after_other_keypress(uint16_t keycode, uint16_t oneshot, keyrecord_t* record) {
#       ifdef OS_STEROIDS_AUTOMATIC_RELEASE
    return should_automatically_release_oneshot_on_steroids(keycode, oneshot, record);
#       else
    return false;
#       endif
}

static bool should_deactivate_layer(int8_t index) {
#       ifdef OS_STEROIDS_FREE_LAYER_STACK
    if (active_osl_index != -1 && index != active_osl_index) {
        return should_process_layer_off(oneshot_os[index].layer);
    }
#       endif  // OS_STEROIDS_FREE_LAYER_STACK
    return true;
}

void deactivate_oneshot_on_steroids(int8_t index, bool called_by_user) {
    // Deactivate a specific OSoS key (by index)
    if (oneshot_state[index] == os_idle) { return; }

    if (oneshot_os[index].modifier != 0) {
        if (should_unregister_osm_on_steroids(index)) {

            switch (oneshot_state[index]) {
                case os_down_unused:
                    unregister_mods_on_steroids(oneshot_os[index].modifier);
                    break;
                case os_down_used:
                    if (called_by_user) {
                        unregister_mods_on_steroids(oneshot_os[index].modifier);
                    } else {
                        unregister_mods(oneshot_os[index].modifier);
                    }
                    break;
                case os_up_queued:
                    if (should_mod_be_held_after_oneshot_release(oneshot_os[index].modifier, oneshot_os[index].trigger)) {
                        unregister_mods_on_steroids(oneshot_os[index].modifier);
                    } else {
                        del_oneshot_mods(oneshot_os[index].modifier);
                    }
                    break;
                case os_finished:
                    if (should_mod_be_held_after_oneshot_release(oneshot_os[index].modifier, oneshot_os[index].trigger)) {
                        unregister_mods(oneshot_os[index].modifier);
                    } else if (called_by_user) {
                        del_oneshot_mods(oneshot_os[index].modifier);
                    }
                    break;
                default:
                    break;
            }
        }
    }

    if (oneshot_os[index].layer != 0) {
        if (should_deactivate_layer(index)) { layer_off(oneshot_os[index].layer); }

        if (index == active_osl_index) {
            active_osl_index = -1;
            // If there is no other OSL active,
            // restore oneshot_origin_layer
            // and unregister the mods absorbed by the OSL.
#               ifdef OS_STEROIDS_FREE_LAYER_STACK
            if (oneshot_origin_layer != 0) {
                layer_on(oneshot_origin_layer);
                oneshot_origin_layer = 0;
            }
#               endif  // OS_STEROIDS_FREE_LAYER_STACK
#               ifdef OS_STEROIDS_ABSORB_MODS
            if (oneshot_absorbed_mods != 0) {
                unregister_mods_on_steroids(oneshot_absorbed_mods);
                oneshot_pressed_mods = 0;
                oneshot_absorbed_mods = 0;
            }
#               endif  // OS_STEROIDS_ABSORB_MODS
        }
    }
    oneshot_state[index] = os_idle;
#       if OS_STEROIDS_TIMEOUT > 0
    if (index == timed_oneshot_index) {
        idle_timer = 0;
        timed_oneshot_index = -1;
    }
#       endif  // OS_STEROIDS_TIMEOUT > 0
}


static void process_trigger_press(uint8_t index, keyrecord_t *record) {

    if (oneshot_state[index] == os_idle) {

        if (oneshot_os[index].modifier != 0) { register_mods(oneshot_os[index].modifier); }
    
        if (oneshot_os[index].layer != 0) {
#               ifdef OS_STEROIDS_FREE_LAYER_STACK
            if (!is_oneshot_layer_active() && (active_osl_index == -1 || oneshot_state[active_osl_index] < os_up_queued)) {
            // OSL on steroids can deactivate another layer only if there is no ongoing oneshot layer,
            // not to mess up with the layer stack.
                const uint8_t key_layer = read_source_layers_cache(record->event.key);
                const uint8_t default_layer = get_highest_layer(default_layer_state);
                if (should_oneshot_on_steroids_deactivate_layer(oneshot_os[index].trigger, key_layer)
                    && key_layer > oneshot_os[index].layer && key_layer != default_layer
                ) {
                    oneshot_origin_layer = key_layer;
                    layer_off(key_layer);
                }
            }
#               endif  // OS_STEROIDS_FREE_LAYER_STACK

#               ifdef OS_STEROIDS_ABSORB_MODS
            if (should_oneshot_on_steroids_absorb_mods(oneshot_os[index].trigger)) {
#                   if OS_STEROIDS_TIMEOUT > 0
                if (timed_oneshot_index != -1) {
                    if ((get_mods() | get_oneshot_mods()) & oneshot_os[timed_oneshot_index].modifier) {
                        // preventing OSM tapped before from idling
                        idle_timer = 0;
                        timed_oneshot_index = -1;
                    }
                }
#                   endif  // OS_STEROIDS_TIMEOUT > 0
                // removing the oneshot mod of `mods`
                const uint8_t mods = get_mods() & ~oneshot_os[index].modifier;
                if (mods) {
                    oneshot_pressed_mods |= mods;
                }
                const uint8_t oneshot_mods = get_oneshot_mods() & ~oneshot_os[index].modifier;
                if (oneshot_mods) {
                    del_oneshot_mods(oneshot_mods);
                    oneshot_absorbed_mods |= oneshot_mods;
                    register_mods(oneshot_mods);
                }
            }
#               endif  // OS_STEROIDS_ABSORB_MODS

            layer_on(oneshot_os[index].layer);
            active_osl_index = index;
        }
        oneshot_tap_time[index] = timer_read();
        oneshot_state[index] = os_down_unused;
    } else {
        // The oneshot key is being tapped twice, without another key having being tapped:
        // cancel the oneshot.
        deactivate_oneshot_on_steroids(index, false);
    }
}

static void process_suppressor_release(uint8_t index, uint16_t keycode, keyrecord_t *record) {

    if (oneshot_state[index] == os_down_unused && timer_elapsed(oneshot_tap_time[index]) < get_oneshot_on_steroids_term(keycode, record)) {
        // The oneshot key has been released earlier than the One Shot Term,
        // without any other key being pressed in-between:
        // triggering the oneshot behavior.
        oneshot_state[index] = os_up_queued;
        if (oneshot_os[index].modifier != 0) {
            if (!should_mod_be_held_after_oneshot_release(oneshot_os[index].modifier, oneshot_os[index].trigger)) {
                unregister_mods(oneshot_os[index].modifier);
                add_oneshot_mods(oneshot_os[index].modifier);
            }
        }
#           if OS_STEROIDS_TIMEOUT > 0
        set_oneshot_on_steroids_idle_timer(oneshot_os[index].trigger, record);
        if (idle_timer) { timed_oneshot_index = index; }
#           endif  // OS_STEROIDS_TIMEOUT > 0
    } else {
        // The oneshot key has been released after the One Shot Term
        // or a key was tapped when the oneshot key was held:
        // cancel the oneshot.
        deactivate_oneshot_on_steroids(index, false);
    }
}

static void process_other_key_press(uint8_t index, uint16_t keycode, keyrecord_t *record) {

    if (is_oneshot_on_steroids_cancel_key(keycode)) {
        // Cancel oneshot on press of specific keys.
        deactivate_oneshot_on_steroids(index, false);
        return;
    }

    if (should_oneshot_on_steroids_ignore_key(keycode, oneshot_os[index].trigger, record)) {
#           if OS_STEROIDS_TIMEOUT > 0
        if (idle_timer) { set_oneshot_on_steroids_idle_timer(oneshot_os[index].trigger, record); }
#           endif  // OS_STEROIDS_TIMEOUT > 0
        return;
    }
    
    // Regular key pressed
    switch (oneshot_state[index]) {
        case os_down_unused:
            // When the oneshot key is still pressed
            if (!automatic_release_after_other_keypress(keycode, oneshot_os[index].trigger, record)) {
                oneshot_state[index] = os_down_used;
                break;
            }
        case os_up_queued:
            if (is_oneshot_on_steroids(keycode)) {
                // because process_record_oneshots_on_steroids() will return false,
                // halting all further processing,
                // post_process_record_oneshots_on_steroids will never be reached.
                deactivate_oneshot_on_steroids(index, false);
            } else {
                oneshot_state[index] = os_finished;
            }
            break;
        default:
            break;
    }
}


bool process_record_oneshots_on_steroids(uint16_t keycode, keyrecord_t *record) {
    
    if (!is_oneshot_on_steroids_custom_behavior(keycode, record)) { return false; }

    bool should_continue_processing = true;

    // Processing triggers and suppressors
    for (uint8_t i = 0; i < OS_STEROIDS_COUNT; i++) {

        if (record->event.pressed) {    // On press

            if (keycode == oneshot_os[i].trigger) {
                // Trigger keydown
                process_trigger_press(i, record);
                should_continue_processing = false;
            }

        } else {    // On release

            if (oneshot_state[i] == os_idle) {
                // Case of canceled oneshot (OSoS key tapped twice)
                if (keycode == oneshot_os[i].trigger) { should_continue_processing = false; }
                continue;
            }
            if (keycode == oneshot_os[i].suppressor) { process_suppressor_release(i, keycode, record); }
            // Trigger released
            if (keycode == oneshot_os[i].trigger) { should_continue_processing = false; }
        }
    }

    // Processing other keys
    for (uint8_t i = 0; i < OS_STEROIDS_COUNT; i++) {

        if (oneshot_state[i] == os_idle) { continue; }
        // Triggers has been processed before
        if (keycode == oneshot_os[i].trigger) { continue; }

        if (record->event.pressed) {    // On press
            process_other_key_press(i, keycode, record);

        } else if (i == active_osl_index) {
            // Regular key release when an OSL is active

#               ifdef OS_STEROIDS_ABSORB_MODS
            if (!should_process_mod_release(i, keycode, record)) {
                should_continue_processing = false;
                continue;
            }
#               endif  // OS_STEROIDS_ABSORB_MODS

#               ifdef OS_STEROIDS_FREE_LAYER_STACK
            if (!should_process_layer_release(i, keycode, record)) {
                should_continue_processing = false;
            }
#               endif  // OS_STEROIDS_FREE_LAYER_STACK
        }
    }
    return should_continue_processing;
}

void post_process_record_oneshots_on_steroids(uint16_t keycode, keyrecord_t *record) {

    // In post_process_record instead of process_record because of a bug
    // when rolling two keys, the first one affected by a custom oneshot
    // and the second one being a mod-tap on base layer.

    for (uint8_t i = 0; i < OS_STEROIDS_COUNT; i++) {
        if (oneshot_state[i] == os_finished) {
            // The oneshot has been used.
            deactivate_oneshot_on_steroids(i, false);
            continue;
        }
    }
}