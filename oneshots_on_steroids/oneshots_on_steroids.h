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

#pragma once

#include "quantum.h"

#ifdef __cplusplus
extern "C" {
#endif

/*Custom one-shot key structure.
 * 
 * The `trigger` field is the keycode that triggers the modifier or the layer-change.
 * The `suppressor` field is the keycode that 
 * Trigger and suppressor can be the same key, but they can be different, to "bounce" between layers.
 * 
 * 
 * For one-shot mods, the `modifier` field must use the MOD_* prefix.
 * Modifiers can be combined, for ex. MOD_LCTL | MOD_LSFT
 * KC_NO must be used if no modifiers are triggered by the one-shot key.
 * The `layer` field is used for one-shot layers.
 * `0` must be used 
 */ 
typedef struct {
  uint16_t trigger;
  uint16_t suppressor;
  uint8_t modifier;
  uint8_t layer;
} oneshot_t;

// Table of custom one-shot modifiers.
// Each custom OSM must also be declared in custom_keycodes.
extern const oneshot_t oneshot[];

// Represents the five states a one-shot key can be in
typedef enum {
    os_idle,
    os_up_queued,
    os_up_queued_used,
    os_down_unused,
    os_down_used,
} oneshot_state_t;

/* Idle timeout:
 * Custom one-shot mods are configured to deactivate if the keyboard is idle for some time.
 * This is useful to prevent unexpected behaviours.
 * In config.h, define `ONESHOT_TIMEOUT` with a time in milliseconds.
 */
//void oneshot_task(void);

// Deactivate a specific one-shot key 
void clear_oneshot_on_steroids(uint8_t index);

// Deactivate all one-shot keys
void clear_all_oneshots_on_steroids(void);

// Deactivate one-shot keys involving a layer
void clear_oneshot_layer_on_steroids(uint8_t layer);

// Deactivate all one-shot keys involving a modifier
void clear_all_oneshot_mod_on_steroids(void);

// Returns whether a keycode is a custom one-shot key or not
bool is_oneshot_on_steroids(uint16_t keycode);

//void post_process_oneshot_on_steroids(uint16_t keycode, keyrecord_t *record);

// Custom one-shot keys implementation that doesn't rely on timers.
// If a mod is used while it is held it will be unregistered on keyup as normal.
// Otherwise it will be queued and only released after the next non-mod keyup.
//bool process_oneshot_on_steroids(uint16_t keycode, keyrecord_t *record);

// To be implemented by the user. Defines keys to cancel one-shot mods and layers.
bool is_oneshot_on_steroids_cancel_key(uint16_t keycode);

// To be implemented by the user. 
// Defines keys not to release the modifier when determining
// whether a one-shot mod has been used
// Setting this to layer change keys allows carrying one-shot modifiers between layers.
bool should_oneshot_on_steroids_stay_pressed(uint16_t keycode);

#ifdef __cplusplus
}
#endif
