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

// Applies the HRM modifier before the tap-hold decision is made.
// It's especially usefull when using mod-tap keys with a mouse.
//void pre_process_speculative_hold(uint16_t keycode, keyrecord_t* record);

// Clears the speculatively-held mod once the tap-hold decision is made.
//bool process_speculative_hold(uint16_t keycode, keyrecord_t* record);

/**
 * Optional callback to define per-key where Speculative Hold applies.
 *
 * @param keycode    Keycode of the mod-tap key.
 * @param record     keyrecord_t for the event.
 * @return Whether to perform speculative holding for this key.
 */
bool get_speculative_hold(uint16_t keycode, keyrecord_t* record);

#ifdef __cplusplus
}
#endif