/* Copyright 2025 @Kawamashi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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
#include "kawamashi.h"

#ifdef __cplusplus
extern "C" {
#endif

//void housekeeping_task_flow_tap(void);

// Checks if the event is on a home-row MT with a valid matrix position.
//bool is_tap_hold_event(uint16_t keycode, keyrecord_t* record, keypos_t pos);

// Disables the hold behaviour of HRM during typing.
//bool pre_process_record_flow_tap(uint16_t keycode, keyrecord_t* record);

#ifdef __cplusplus
}
#endif
