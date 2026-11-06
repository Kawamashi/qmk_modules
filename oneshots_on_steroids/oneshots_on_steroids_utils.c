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


static int8_t get_oneshot_on_steroids_index(uint16_t keycode) {
    // If the keycode is an OSoS key, returns its index in the array of OSoS
    // Otherwise, returns -1
    for (uint8_t i = 0; i < OS_STEROIDS_COUNT; i++) {
        if (keycode == oneshot_os[i].trigger) { return i; }
    }
    return -1;
}

int8_t get_oneshot_on_steroids_state(uint16_t keycode) {
    for (uint8_t i = 0; i < OS_STEROIDS_COUNT; i++) {
        if (keycode == oneshot_os[i].trigger) { return get_oneshot_state(i); }
    }
    return -1;
}

uint8_t get_oneshot_layer_on_steroids(void) {
    const int8_t act_osl_index = get_active_osl_index();
    if (act_osl_index == -1) { return 0; }
    return oneshot_os[act_osl_index].layer;
}

bool is_oneshot_on_steroids(uint16_t keycode) {
    for (uint8_t i = 0; i < OS_STEROIDS_COUNT; i++) {
        if (keycode == oneshot_os[i].trigger) { return true; }
    }
    return false;
}

bool is_oneshot_layer_on_steroids(uint16_t keycode) {
    for (uint8_t i = 0; i < OS_STEROIDS_COUNT; i++) {
        if (keycode == oneshot_os[i].trigger && oneshot_os[i].layer != 0) { return true; }
    }
    return false;
}

bool is_oneshot_mod_on_steroids(uint16_t keycode) {
    return is_oneshot_on_steroids(keycode) && !is_oneshot_layer_on_steroids(keycode);
}

bool is_oneshot_layer_on_steroids_active(void) {
    return get_active_osl_index() != -1;
}

void cancel_oneshot_on_steroids(uint16_t keycode) {
    const int8_t index = get_oneshot_on_steroids_index(keycode);
    if (index != -1) { deactivate_oneshot_on_steroids(index, true); }
}

void clear_oneshots_on_steroids(void) {
    for (uint8_t i = 0; i < OS_STEROIDS_COUNT; i++) {
        deactivate_oneshot_on_steroids(i, true);
    }
}

void reset_oneshot_layer_on_steroids(void) {
    const int8_t act_osl_index = get_active_osl_index();
    if (act_osl_index == -1) { return; }
    deactivate_oneshot_on_steroids(act_osl_index, true);
}

void del_oneshot_mods_on_steroids(uint8_t mods) {
    for (uint8_t i = 0; i < OS_STEROIDS_COUNT; i++) {
        if (get_oneshot_state(i) == os_idle) { continue; }
        if ((oneshot_os[i].modifier & mods) != 0) {
            deactivate_oneshot_on_steroids(i, true);
#           ifdef OS_STEROIDS_ABSORB_MODS
        } else {
            const uint8_t absorbed_mods = get_absorbed_mods();
            if ((absorbed_mods & mods) != 0) {
                // Case of modifiers absorbed by OSL
                // In this case, we must remove modifiers w/o cancelling the OSL.
                unregister_mods_on_steroids(mods);
                set_absorbed_mods(absorbed_mods & ~mods);
            }
#           endif  // OS_STEROIDS_ABSORB_MODS
        }
    }
}

void clear_oneshot_mods_on_steroids(void) {
    for (uint8_t i = 0; i < OS_STEROIDS_COUNT; i++) {
        if (get_oneshot_state(i) == os_idle) { continue; }
        if (oneshot_os[i].modifier != 0) {
            deactivate_oneshot_on_steroids(i, true);
#           ifdef OS_STEROIDS_ABSORB_MODS
        } else {
            const uint8_t absorbed_mods = get_absorbed_mods();
            if (absorbed_mods != 0) {
                // Case of modifiers absorbed by OSL
                // In this case, we must remove modifiers w/o cancelling the OSL.
                unregister_mods_on_steroids(absorbed_mods);
                set_absorbed_mods(0);
            }
#           endif  // OS_STEROIDS_ABSORB_MODS    
        }
    }
}


__attribute__((weak)) bool is_oneshot_on_steroids_custom_behavior(uint16_t keycode, keyrecord_t* record) {
    if (record->event.pressed) {
        switch (keycode) {

            default:
                break;
        }
    } else {

    }
    return true;
}

__attribute__((weak)) bool is_oneshot_on_steroids_cancel_key(uint16_t keycode) {
    switch (keycode) {

        default:
            return false;
    }
}

__attribute__((weak)) bool should_oneshot_on_steroids_ignore_key(uint16_t keycode, uint16_t oneshot, keyrecord_t* record) {

    bool is_mod_key = is_oneshot_mod_on_steroids(keycode);
    bool is_layer_key = is_oneshot_layer_on_steroids(keycode);

    switch (keycode) {
        // mod keys.
        case QK_MOD_TAP ... QK_MOD_TAP_MAX:
            if (record->tap.count) { break; }
        case KC_LCTL ... KC_RGUI:
        case KC_HYPR:
        case KC_MEH:
        case QK_ONE_SHOT_MOD ... QK_ONE_SHOT_MOD_MAX:
            is_mod_key = true;
            break;

        // layer switch keys.
        case QK_LAYER_TAP ... QK_LAYER_TAP_MAX:
            if (record->tap.count) { break; }
        case QK_LAYER_TAP_TOGGLE ... QK_LAYER_TAP_TOGGLE_MAX:
        case QK_MOMENTARY ... QK_MOMENTARY_MAX:
        case QK_LAYER_MOD ... QK_LAYER_MOD_MAX:
        case QK_ONE_SHOT_LAYER ... QK_ONE_SHOT_LAYER_MAX:
        case QK_TO ... QK_TO_MAX:
        case QK_TOGGLE_LAYER ... QK_TOGGLE_LAYER_MAX:
        case QK_TRI_LAYER_LOWER ... QK_TRI_LAYER_UPPER:
            is_layer_key = true;
            break;
    }

    if (!is_mod_key && !is_layer_key) { return false; }

    // Mod or layer-change key pressed after an OSoS key
    if (is_oneshot_layer_on_steroids(oneshot)) {
        // If a layer-change key is pressed after an OSL, the OSL must be reset.
        if (is_layer_key) { return false; }
        // keycode is not a layer key, it’s a mod key.
#           ifdef OS_MOD_SHOULD_LEAVE_OS_LAYER
        // When using OSM as Callum mods, an OSL tapped before must be reset.
        if (is_oneshot_mod_on_steroids(keycode)) { return false; }
#           endif  // OS_MOD_SHOULD_LEAVE_OS_LAYER
        // Standard behavior, like any mod key after an OSL
        return true;
    } else {
        // oneshot is OSoS mod key
#           ifdef OS_STEROIDS_ABSORB_MODS
        if (is_oneshot_layer_on_steroids(keycode)) {
            if (should_oneshot_on_steroids_absorb_mods(keycode)) { return false; }
        }
#           endif  // OS_STEROIDS_ABSORB_MODS
        // OSoS mod keys should stay pressed
        // whether keycode is a mod or a layer-change key.
        return true;
    }
}


__attribute__((weak)) bool should_mod_be_held_after_oneshot_release(uint8_t mod, uint16_t keycode) {
    // Shift and Ctrl are not kept registered after the OSoS key is released,
    // to avoid interfering with mouse usage. If the one shot behavior is triggered,
    // `add_oneshot_mods()` is used instead.
    if (mod & (MOD_MASK_CTRL | MOD_MASK_SHIFT)) { return false; }
    return true;
}

__attribute__((weak)) uint16_t get_oneshot_on_steroids_term(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
    
    default:
        return OS_STEROIDS_TERM;
    }
}

#   if OS_STEROIDS_TIMEOUT > 0
__attribute__((weak)) uint16_t get_oneshot_on_steroids_timeout(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
    
    default:
        return OS_STEROIDS_TIMEOUT;
    }
}
#   endif  // OS_STEROIDS_TIMEOUT > 0

#ifdef OS_STEROIDS_ABSORB_MODS
__attribute__((weak)) bool should_oneshot_on_steroids_absorb_mods(uint16_t keycode) {
    switch (keycode) {

        default:
            return true;
    }
}
#endif  // OS_STEROIDS_ABSORB_MODS

#ifdef OS_STEROIDS_FREE_LAYER_STACK
__attribute__((weak)) bool should_oneshot_on_steroids_deactivate_layer(uint16_t keycode, uint8_t layer) {
    switch (keycode) {

        default:
            return true;
    }
}
#endif  // OS_STEROIDS_FREE_LAYER_STACK

#ifdef OS_STEROIDS_AUTOMATIC_RELEASE
__attribute__((weak)) bool should_automatically_release_oneshot_on_steroids(uint16_t keycode, uint16_t oneshot, keyrecord_t* record) {
    switch (keycode) {

        default:
            return true;
    }
}
#endif  // OS_STEROIDS_AUTOMATIC_RELEASE