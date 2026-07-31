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

#pragma once

#include "quantum.h"

#ifdef __cplusplus
extern "C" {
#endif

/* One Shot on Steroids key structure.
 * 
 * The `trigger` field is the keycode that is pressed to initiate the One Shot on Steroids.
 * The `suppressor` field is the keycode that is held to continue the One Shot on Steroids.
 * Trigger and suppressor are usually the same key.
 * In some cases, instead of holding down the trigger key, it’s easier to use another key, hence the suppressor key.
 * 
 * For OSoS mod keys, the `modifier` field must use the MOD_BIT() macro.
 * Modifiers can be combined, for ex. MOD_BIT(KC_LCTL) | MOD_BIT(KC_LSFT)
 * `0` must be used for layer-only OSoS key.
 * The `layer` field is used for OSoS layer keys.
 * `0` must be used for modifier-only OSoS keys.
 */ 
typedef struct {
  uint16_t trigger;
  uint16_t suppressor;
  uint8_t modifier;
  uint8_t layer;
} oneshot_on_steroids_t;

// Array of One Shot on Steroids keys.
// Each OSoS key must also be declared in `custom_keycodes`.
extern const oneshot_on_steroids_t oneshot_os[];

// Represents the five states an OSoS key can be in
typedef enum {
    os_idle,
    os_down_unused,
    os_down_used,
    os_up_queued,
    os_up_queued_used,
} oneshot_on_steroids_state_t;

// Oneshot-type wrapper
#ifdef OS_STEROIDS_SPLIT_TRIGGER_HOLD
#  define OS(key1, key2, mods, layer) key1, key2, mods, layer
#else
#  define OS(key, mods, layer) key, key, mods, layer
#endif  // OS_STEROIDS_SPLIT_TRIGGER_HOLD

#if !defined OS_STEROIDS_TERM
#   define OS_STEROIDS_TERM TAPPING_TERM
#endif

// If the keycode is an OSoS key, returns its state
// Otherwise, returns -1
int8_t get_oneshot_on_steroids_state(uint16_t keycode);

// If there is an active OSoS layer, returns the layer
// Otherwise, returns 0
uint8_t get_oneshot_layer_on_steroids(void);

#   if defined OS_STEROIDS_ABSORB_MODS
// Returns the modifiers absorbed by an OSoS layer key
uint8_t get_absorbed_mods(void);
#   endif  // OS_STEROIDS_ABSORB_MODS

// Returns whether a keycode is an OSoS key
bool is_oneshot_on_steroids(uint16_t keycode);

// Returns whether a keycode is an OSoS layer key
bool is_oneshot_layer_on_steroids(uint16_t keycode);

// Returns whether a keycode is a modifier-only OSoS key
bool is_oneshot_mod_on_steroids(uint16_t keycode);

// Returns whether an OSoS layer is active
bool is_oneshot_layer_on_steroids_active(void);

// Deactivates a specific OSoS key (by keycode)
void cancel_oneshot_on_steroids(uint16_t keycode);

// Deactivates all OSoS keys
void clear_oneshots_on_steroids(void);

// Deactivates the active OSoS layer key
void reset_oneshot_layer_on_steroids(void);

// Deactivates all OSoS keys using a specific modifier
void del_oneshot_mods_on_steroids(uint8_t mods);

// Deactivates all OSoS keys using any modifier
void clear_oneshot_mods_on_steroids(void);



//*******************************************  Callbacks  ********************************************//

// Callback to customise the One Shot Term for each OSoS keys.
uint16_t get_oneshot_on_steroids_term(uint16_t keycode, keyrecord_t *record);

// Handles OSoS custom behavior, before their standard processing
bool is_oneshot_on_steroids_custom_behavior(uint16_t keycode, keyrecord_t* record);

// Defines keys to cancel One Shots on Steroids.
bool is_oneshot_on_steroids_cancel_key(uint16_t keycode);

// Determines whether a key press should be ignored by an active OSoS key.
bool should_oneshot_on_steroids_ignore_key(uint16_t keycode, uint16_t trigger, keyrecord_t* record);

// When triggering the one shot effect, define which mods should be released
// to send one shot mods instead, not to interfere with the mouse.
// By default, the function returns true if mods include shift or ctrl.
bool should_mod_be_held_after_oneshot_release(uint8_t mod, uint16_t keycode);

#   ifdef OS_STEROIDS_TIMEOUT
// // Callback to customise the timeout for each OSoS keys.
uint16_t get_oneshot_on_steroids_timeout(uint16_t keycode, keyrecord_t *record);
#   endif  // OS_STEROIDS_TIMEOUT

#   if defined OS_STEROIDS_ABSORB_MODS
// Callback to customise which OSoS layer keys should absorb modifiers.
bool should_oneshot_on_steroids_absorb_mods(uint16_t keycode);
#   endif  // OS_STEROIDS_ABSORB_MODS

#   ifdef OS_STEROIDS_FREE_LAYER_STACK
// Callback to customise which OSoS layer keys should deactivate their origin layer.
bool should_oneshot_on_steroids_deactivate_layer(uint16_t keycode, uint8_t layer);
#   endif  // OS_STEROIDS_FREE_LAYER_STACK

#ifdef __cplusplus
}
#endif
