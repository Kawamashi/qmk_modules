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

/* Idle timeout:
 *
 * Layerword can be configured to deactivate if the keyboard is idle
 * for some time. This is useful to prevent unexpected behaviors.
*/
#ifndef LAYERWORD_TIMEOUT
#    define LAYERWORD_TIMEOUT 5000 // Default timeout of 5 seconds.
#endif

#   if LAYERWORD_TIMEOUT > 0
// Returns the idle timeout for the specified layer (in milliseconds)
uint16_t get_layerword_timeout(uint8_t layer);
#   endif  // LAYERWORD_TIMEOUT > 0

// If there is an active Layer Word, returns the layer
// Otherwise, returns 0
uint8_t get_layerword_layer(void);

// Returns whether a Layer Word is active
bool is_layerword_active(void);

// Activates Layer Word on the specified layer
void enable_layerword(uint8_t layer);

// Deactivate Layer Word
void disable_layerword(void);

// Toggles Layer Word based on the triggering keycode
void toggle_layerword(uint16_t keycode);

// Handler for Layer Word trigger keys
bool process_layerword_triggers(uint16_t keycode, keyrecord_t *record);

// Returns the layer associated with a trigger keycode, or 0 if none
uint8_t get_layerword_layer_from_trigger(uint16_t keycode);

/* Callback function, called on each key press while Layer Word is active.
 *
 * Returns true to keep Layer Word active, or false to deactivate it
 * after the key has been processed.
 * 
 * To deactivate Layer Word before processing the key, call
 * disable_layerword() before returning false.
*/
bool should_continue_layerword(uint8_t layer, uint16_t keycode, keyrecord_t *record);

#ifdef __cplusplus
}
#endif