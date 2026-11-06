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

#include "flow_tap.h"


// Events bypass flow_tap when there are unsettled LT keys in action_tapping's
// waiting_queue. Particularly, supposing an LT settles as held, the layer state
// will change and buffered events following the LT will be reconsidered as keys
// on that layer. That may change whether flow_tap is enabled or the timeout
// to use on those keys. We don't know in advance how the LT will settle.
static uint16_t settle_timer = 0;
static uint8_t is_tapped[(MATRIX_ROWS * MATRIX_COLS + 7) / 8] = {0};

void housekeeping_task_flow_tap(void) {
  if (settle_timer && timer_expired(timer_read(), settle_timer)) {
    settle_timer = 0;
  }
}

bool is_tap_hold_event(uint16_t keycode, keyrecord_t* record, keypos_t pos) {
  // The event is not a combo
  if (!IS_KEYEVENT(record->event)) { return false; }
  // The key is a mod-tap or a layer-tap
  if (!IS_QK_MOD_TAP(keycode) && !IS_QK_LAYER_TAP(keycode)) { return false; }
  // The keyrecord has a valid matrix position
  if (pos.row >= MATRIX_ROWS || pos.col >= MATRIX_COLS) { return false; }

  return true;
}

bool pre_process_record_flow_tap(uint16_t keycode, keyrecord_t* record) {
  const keypos_t pos = record->event.key;

  if (is_tap_hold_event(keycode, record, pos)) {
    // The event is on an MT or LT with a valid matrix position.
    const uint16_t tap_keycode = tap_hold_extractor(keycode);

    // Determine the key's index in the bit arrays.
    const uint16_t index = pos.row * MATRIX_COLS + pos.col;
    const uint16_t array_index = index / 8;
    const uint8_t bit_mask = UINT8_C(1) << (index % 8);

    if (record->event.pressed) {  // On press.

      if (IS_QK_MOD_TAP(keycode) && !settle_timer && get_idle_time() < TAP_INTERVAL) {
        // Rewrite the event as a press of the tap keycode. This way, it
        // bypasses the usual action_tapping logic.
        record->keycode = tap_keycode;
        // Record this key as tapped.
        is_tapped[array_index] |= bit_mask;

      } else if (IS_QK_LAYER_TAP(keycode)) {
        // Otherwise if this is an LT key, track when it will settle according to its tapping term.
        // NOTE: To be precise, the key could settle before the tapping term. This is an approximation.
        const uint16_t term = GET_TAPPING_TERM(keycode, record);
        const uint16_t now = timer_read();
        if (!settle_timer || term > TIMER_DIFF_16(settle_timer, now)) {
          settle_timer = (now + term) | 1;
        }
      }

    } else if ((is_tapped[array_index] & bit_mask) != 0) {  // On tap release.

      // Rewrite the event as a release of the tap keycode.
      record->keycode = tap_keycode;
      // Record the key as released.
      is_tapped[array_index] &= ~bit_mask;
    }
  }
  
  return true;
}