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

// Number of keys in the buffer of recent keys.
#ifndef CK_BUFFER_SIZE
#   define CK_BUFFER_SIZE 8
#endif  // CK_BUFFER_SIZE

/* Idle timeout:
 *
 * The Clever Keys buffer can be configured to be emptied if the keyboard is idle
 * for some time. This is useful to prevent unexpected behaviors.
*/
#ifndef CLEVER_KEYS_TIMEOUT
#  define CLEVER_KEYS_TIMEOUT 5000
#endif  // CLEVER_KEYS_TIMEOUT

#   if CLEVER_KEYS_TIMEOUT > 0
// Callback to customise the timeout for each key
uint16_t get_clever_keys_timeout(uint16_t keycode);
#   endif  // CLEVER_KEYS_TIMEOUT > 0

// Returns the n-th key of the buffer
// ex: get_recent_keycode(-1) returns the latest key,
// get_recent_keycode(-2) the key before, etc.
uint16_t get_recent_keycode(int8_t n);

// Returns whether the last key was replaced
bool was_keycode_replaced(void);

/**
 * Backspace countdown:
 * When a clever key is found, it supersede the ongoing key.
 * Sometimes, it's is not the desired behavior.
 * In this case, just delete the clever(s) key(s) and type again the desired letter.
 * To make it work, registering a clever key settles a "backspace countdown".
 * When it reaches 0, the buffer of recent keys is cleared.
 * This function updates the countdown.
*/ 
void update_bkspc_countdown(uint8_t i);

// Clears the buffer by replacing all keys by KC_NO
void clear_recent_keys(void);

/**
 * Idle timeout:
 * A timeout can be configured to clear the buffer if the keyboard is idle
 * for some time. This is useful to prevent unexpected behaviors.
 * To be called from `housekeeping_task_user`.
 */
//void recent_keys_task(void);

/**
 * For each keypress, this function determines whether the key should be further processed by `Clever keys`.
 *
 * `Clever keys` shouldn't interfere with hotkeys, navigation keys, etc.
 * With these keys, the function returns KC_NO and the process doesn't go any further.
 * Only keypresses which produce actual characters should be processed and buffered.
 * In this case, the function returns the extracted keycode.
 * Tap-hold keys and backspace must be handled properly.
 */
uint16_t get_ongoing_keycode(uint16_t keycode, keyrecord_t* record);

/**
 * User callback to customize the outcome of `get_ongoing_keycode`,
 * depending on layers for ex.
 * Returns KC_NO to stop `Clever keys` processing
 * Returns KC_TRNS to continue default processing
 * Returns `keycode` to process and buffer it directly
*/
uint16_t get_ongoing_keycode_user(uint16_t keycode, keyrecord_t* record);

// Function to append `keycode` to the buffer
// This function also update `bkspc_countdown`
// and the idle timeout.
void store_keycode(uint16_t keycode, keyrecord_t* record);

// Generate a keyrecord and plumb it into the event pipeline.
void invoke_key(uint16_t keycode, keyrecord_t* record);

// Replaces the ongoing key by another one, for "adaptative keys".
void replace_ongoing_key(uint16_t clever_keycode, uint16_t* ongoing_keycode, keyrecord_t* record);

// Plumb a sequence of keys into the event pipeline.
void process_word(uint16_t keycodes[], uint8_t num_keycodes, keyrecord_t* record);

// Plumb a sequence of keys into the event pipeline and replaces the ongoing key by another one.
void finish_word(uint16_t keycodes[], uint8_t num_keycodes, uint16_t* ongoing_keycode, keyrecord_t* record);

//void finish_magic(uint16_t keycodes[], uint8_t num_keycodes, uint16_t* ongoing_keycode, keyrecord_t* record);


// Handler function for Clever Keys
// To be called from `process_record_user`
void process_clever_keys(uint16_t keycode, keyrecord_t* record);

// Looks for a clever key to supersede the ongoing key
void get_clever_keycode(uint16_t* ongoing_keycode, keyrecord_t* record);

// Returns the time elapsed since the last key was pressed to produce a character.
// Useful to implement mandatory idle time before triggering events (ex: combos)
uint16_t get_idle_time(void);

// Returns whether a keycode is a letter or not
bool is_letter(uint16_t keycode);

#ifdef __cplusplus
}
#endif
