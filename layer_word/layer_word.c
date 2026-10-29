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

#include "layer_word.h"

ASSERT_COMMUNITY_MODULES_MIN_API_VERSION(1, 0, 0);

#ifdef OS_STEROIDS_COUNT
  #include "../oneshots_on_steroids/oneshots_on_steroids.h"
#endif  // OS_STEROIDS_COUNT


static uint8_t layerword_layer = 0;
static bool continue_layerword = false;

#   if LAYERWORD_TIMEOUT > 0
static uint16_t idle_timer = 0;

// Layer Words can be configured to deactivate if the keyboard is idle for some time.
// This is useful to prevent unexpected behaviors.
// In config.h, define `LAYERWORD_TIMEOUT` with a time in milliseconds.
void housekeeping_task_layer_word(void) {
    if (idle_timer && timer_expired(timer_read(), idle_timer)) {
        disable_layerword();
    }
}

// Update the Layer Word timeout
static void set_layerword_idle_timer(void) {
    uint16_t timeout = get_layerword_timeout(layerword_layer);
    if (timeout) { idle_timer = (timer_read() + timeout) | 1; }
}
#   endif  // LAYERWORD_TIMEOUT > 0

uint8_t get_layerword_layer(void) {
    return layerword_layer;
}

bool is_layerword_active(void) {
    return layerword_layer != 0;
}

void enable_layerword(uint8_t layer) {
    if (layer == 0) { return; }
    if (layerword_layer != 0) { layer_off(layerword_layer); }   // Only one Layer Word can be active at a time

#       ifndef NO_ACTION_ONESHOT
    if (layer == get_oneshot_layer()) {
        reset_oneshot_layer();  // Reset OSL to prevent it to turn the layer off.
    }
#       endif  // NO_ACTION_ONESHOT

#       ifdef OS_STEROIDS_COUNT
    if (layer == get_oneshot_layer_on_steroids()) {
        reset_oneshot_layer_on_steroids();
    }
#       endif  // OS_STEROIDS_COUNT

    layer_on(layer);
    layerword_layer = layer;
    continue_layerword = true;
#       if LAYERWORD_TIMEOUT > 0
    set_layerword_idle_timer();
#       endif  // LAYERWORD_TIMEOUT > 0
}
  
void disable_layerword(void) {
    if (layerword_layer == 0) { return; }
    layer_off(layerword_layer);
    layerword_layer = 0;
#       if LAYERWORD_TIMEOUT > 0
    idle_timer = 0;
#       endif  // LAYERWORD_TIMEOUT > 0
}

void toggle_layerword(uint16_t keycode) {
    uint8_t layerword_target = get_layerword_layer_from_trigger(keycode);
    if (layerword_target == 0) { return; }

    if (layerword_layer != layerword_target) {
        // Activate target layer
        enable_layerword(layerword_target);
    } else {
        // Press the Layer Word key again to deactivate it
        disable_layerword();
    }
}
  
bool process_layerword_triggers(uint16_t keycode, keyrecord_t *record) {

#       ifndef NO_ACTION_TAPPING
    // Process normally when held
    if (IS_QK_MOD_TAP(keycode) || IS_QK_LAYER_TAP(keycode)) {
        if (record->tap.count == 0) { return true; }
    }
#       endif  // NO_ACTION_TAPPING

    // Tap action
    if (record->event.pressed) {
        toggle_layerword(keycode);
    }
    return false;
}

// Handles `MO` and `TT` keys.
static bool mo_and_tt_handling(uint8_t mo_tt_layer, keyrecord_t* record) {
    if (mo_tt_layer != layerword_layer) { return true; }
    
    if (record->event.pressed) {
        // On press, deactivate Layer Word
        disable_layerword();
    }
    return false;  // Skip default handling.
}

bool process_record_layer_word(uint16_t keycode, keyrecord_t *record) {
    // Handle the custom keycodes that go with this feature
    if (get_layerword_layer_from_trigger(keycode)) { return process_layerword_triggers(keycode, record); }

    // Other than the custom keycodes, nothing else in this feature will activate
    // if the behavior is not on, so allow QMK to handle the event as usual.
    if (layerword_layer == 0) { return true; }

    if (IS_QK_MOMENTARY(keycode)) {  // `MO` keys
        return mo_and_tt_handling(QK_MOMENTARY_GET_LAYER(keycode), record);
    }
    if (IS_QK_LAYER_TAP_TOGGLE(keycode)) {  // `TT` keys
        return mo_and_tt_handling(QK_LAYER_TAP_TOGGLE_GET_LAYER(keycode), record);
    }
    if (IS_QK_LAYER_MOD(keycode)) {  // `LM` keys
        uint8_t lm_layer = QK_LAYER_MOD_GET_LAYER(keycode);
        if (lm_layer != layerword_layer) { return true; }

        if (record->event.pressed) {  // On press, deactivate Layer Word
            disable_layerword();
        } else {  // On release, clear the mod.
            unregister_mods(QK_LAYER_MOD_GET_MODS(keycode));
        }
        return false;  // Skip default handling.
    }


    if (record->event.pressed) {

      #ifndef NO_ACTION_TAPPING
        // Get the base keycode of a mod or layer tap key
        switch (keycode) {
            case QK_MOD_TAP ... QK_MOD_TAP_MAX:
            case QK_LAYER_TAP ... QK_LAYER_TAP_MAX:
                // Return early if the key has not been registered as tapped yet
                if (record->tap.count == 0) { return true; }
                keycode &= 0xff;
                break;
        }
      #endif  // NO_ACTION_TAPPING

        continue_layerword = should_continue_layerword(layerword_layer, keycode, record);
#           if LAYERWORD_TIMEOUT > 0
        if (continue_layerword) { set_layerword_idle_timer(); }
#           endif  // LAYERWORD_TIMEOUT > 0

#       ifndef NO_ACTION_TAPPING
    } else {  // On keyrelease
        if (IS_QK_LAYER_TAP(keycode) && record->tap.count == 0) {
            // Release event on a held layer-tap key when Layer Word is on.
            // Skip default handling so that layer stays on.
            if (QK_LAYER_TAP_GET_LAYER(keycode) == layerword_layer) { return false; }
        }
#       endif  // NO_ACTION_TAPPING
    }
    return true;
}

void post_process_record_layer_word(uint16_t keycode, keyrecord_t *record){
    // Exit Layer Word just after a breaking character

    // In post_process_record instead of process_record because of a bug
    // when rolling two keys with the Layer Word exit after the first one
    // and the second one being a mod-tap on base layer.

    if (!continue_layerword) { disable_layerword(); }
}

__attribute__((weak)) uint8_t get_layerword_layer_from_trigger(uint16_t keycode) {
    switch (keycode) {

        default:
            return 0;
    }
}

#   if LAYERWORD_TIMEOUT > 0
__attribute__((weak)) uint16_t get_layerword_timeout(uint8_t layer) {
    switch (layer) {

        default:
            return LAYERWORD_TIMEOUT;
    }
}
#   endif  // LAYERWORD_TIMEOUT > 0

__attribute__((weak)) bool should_continue_layerword(uint8_t layer, uint16_t keycode, keyrecord_t *record) {
    switch (layer) {

    }
    return false;
}
