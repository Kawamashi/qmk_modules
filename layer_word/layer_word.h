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
 * for some time. This is useful to prevent unexpected behaviours.
 * To be called from `housekeeping_task_user`.
*/
//void housekeeping_task_layerword(void);

// Each layerword timeout timer can be defined individually (in ms).
uint16_t layerword_exit_timeout(uint8_t layer);

// Returns the active Layerword layer
uint8_t get_layerword_layer(void);

// Turns Layerword on
void enable_layerword(uint8_t layer);

// Turns Layerword off
void disable_layerword(uint8_t layer);

// Toggles Layerword
void toggle_layerword(uint16_t keycode);

// Handler for keys triggering layerwords
bool process_layerword_triggers(uint16_t keycode, keyrecord_t *record);

// Handler function for Layerword
// To be called from `process_record_user`
//bool process_record_layerword(uint16_t keycode, keyrecord_t *record);

// Returns the layerword number of a key
// Keys triggering layerwords must be declared here
// and associated with their layer 
uint8_t get_layerword_layer_from_trigger(uint16_t keycode);

/* Callback function, called on each key press while Layerword is active.
 *
 * This function is used to determine whether a key should continue Layerword.
 * Returning true will continue Layerword and stay on layer after key processing.
 * Returning false will end layerword and exit layer after key processing.
 * If you want to exit layer before key processing, call `disable_layerword()` before returning false.
*/
bool should_continue_layerword(uint8_t layer, uint16_t keycode, keyrecord_t *record);

#ifdef __cplusplus
}
#endif