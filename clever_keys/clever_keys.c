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

#include "clever_keys.h"

ASSERT_COMMUNITY_MODULES_MIN_API_VERSION(1, 0, 0);

static uint16_t recent[CK_BUFFER_SIZE] = {KC_NO};
static uint8_t bkspc_countdown = CK_BUFFER_SIZE + 1;

static bool processingCK = false;

static uint16_t last_keypress_time = 0;

#   if CLEVER_KEYS_TIMEOUT > 0
static uint16_t idle_timer = 0;

void housekeeping_task_clever_keys(void) {
  if (idle_timer && timer_expired(timer_read(), idle_timer)) {
    clear_recent_keys();  // Timed out; clear the buffer.
  }
}

// Update the Clever Keys timeout
static void set_clever_keys_idle_timer(uint16_t keycode, keyrecord_t* record) {
    uint16_t timeout = get_clever_keys_timeout(keycode);
    if (timeout) {
      idle_timer = (record->event.time + timeout) | 1;
    } else {
      idle_timer = 0;
    }
}
#endif  // CLEVER_KEYS_TIMEOUT > 0

uint16_t get_idle_time(void) {
  return timer_elapsed(last_keypress_time);
}

uint16_t get_recent_keycode(int8_t i) {
  return recent[CK_BUFFER_SIZE + i];
}

bool was_keycode_replaced(void) {
  return processingCK;
}

void update_bkspc_countdown(uint8_t i) {
  bkspc_countdown = i;
}

void clear_recent_keys(void) {
  memset(recent, 0, sizeof(recent));  // Set all zeros (KC_NO).
  bkspc_countdown = CK_BUFFER_SIZE + 1;
  idle_timer = 0;
}


uint16_t get_ongoing_keycode(uint16_t keycode, keyrecord_t* record) {

  uint8_t mods = get_mods() | get_weak_mods() | get_oneshot_mods();

  if (mods & ~(MOD_MASK_SHIFT | MOD_BIT(KC_ALGR))) {
    clear_recent_keys();  // Avoid interfering with ctrl, alt and gui.
    return KC_NO;
  }

  if (IS_QK_MODS(keycode)) { 
    switch (QK_MODS_GET_MODS(keycode)) {
      case MOD_LSFT:
      case MOD_RSFT:
        mods |= MOD_BIT(KC_LSFT);
        break;
      case MOD_RALT:
        mods |= MOD_BIT(KC_ALGR);
        break;
      
      default:
        clear_recent_keys();  // Avoid interfering with ctrl, alt and gui.
        return KC_NO;
    }
  }

  switch (keycode) {
    // Ignore mod keys.
    case KC_LCTL ... KC_RGUI:
    //case KC_HYPR:
    //case KC_MEH:
    // Sticky keys don't type anything on their own.
    case QK_ONE_SHOT_MOD ... QK_ONE_SHOT_MOD_MAX:
    // Ignore MO, TO, TG, TT, and OSL layer switch keys.
    //case QK_LAYER_TAP_TOGGLE ... QK_LAYER_TAP_TOGGLE_MAX:
    case QK_ONE_SHOT_LAYER ... QK_ONE_SHOT_LAYER_MAX:
    case QK_MOMENTARY ... QK_MOMENTARY_MAX:
/*     case QK_TO ... QK_TO_MAX:
    case QK_TOGGLE_LAYER ... QK_TOGGLE_LAYER_MAX:
    case QK_TRI_LAYER_LOWER ... QK_TRI_LAYER_UPPER: */
        return KC_NO;
  }

  // Extract keycode from regular tap-hold keys.
  if (IS_QK_MOD_TAP(keycode) || IS_QK_LAYER_TAP(keycode)) {
      if (record->tap.count == 0) { return KC_NO; }
      // Get tapping keycode.
      keycode &= 0xff;
  }

  // Handle backspace.
  if (keycode == KC_BSPC) {
      bkspc_countdown--;
      if (bkspc_countdown == 0) {
        // Clear the key buffers.
        clear_recent_keys();
      } else {
        // Rewind the key buffers.
        memmove(recent + 1, recent, (CK_BUFFER_SIZE - 1) * sizeof(uint16_t));
        recent[0] = KC_NO;
      }
  }

  // Handles custom keycodes.
  uint16_t custom_keycode = get_ongoing_keycode_user(keycode, record);
  if (custom_keycode != KC_TRNS) { return custom_keycode; }


  uint16_t basic_keycode = QK_MODS_GET_BASIC_KEYCODE(keycode);

  switch (basic_keycode) {
    // These keys type letters, digits, symbols.
    case KC_A ... KC_0:
    case KC_SPC ... KC_SLASH:
    case KC_KP_1 ... KC_NUBS:

      if (mods & MOD_BIT(KC_ALGR)) { 
        basic_keycode = ALGR(basic_keycode);
        keycode = ALGR(keycode);
      }

      // Handle keys with embedded modifier, for ex on symbols layer
      if (basic_keycode != keycode) { return keycode; }

      if (is_letter(basic_keycode)) { return keycode; }

      if (mods & MOD_MASK_SHIFT) { return S(keycode); }
      
      // Handle shifted symbols (ex shift + '-' = '!')
      // Convert 8-bit mods to the 5-bit format used in keycodes. This is lossy: if
      // left and right handed mods were mixed, they all become right handed.
      //mods = ((mods & 0xf0) ? /* set right hand bit */ 0x10 : 0)
            // Combine right and left hand mods.
/*             | (((mods >> 4) | mods) & 0xf);
      // Combine basic keycode with mods.
      keycode = (mods << 8) | basic_keycode; */
      return keycode;
  }

  // Avoid acting otherwise, particularly on navigation keys.
  clear_recent_keys();
  return KC_NO;
}

__attribute__((weak)) uint16_t get_ongoing_keycode_user(uint16_t keycode, keyrecord_t* record) {

  switch (keycode) {
    case KC_BSPC:
      return KC_NO;
    
    default:
      return KC_TRNS;
  }
}

void store_keycode(uint16_t keycode, keyrecord_t* record) {
  // Slide the buffer left by one element.
  memmove(recent, recent + 1, (CK_BUFFER_SIZE - 1) * sizeof(*recent));
  recent[CK_BUFFER_SIZE - 1] = keycode;
  bkspc_countdown++;
}

void invoke_key(uint16_t keycode, keyrecord_t* record) {
  // Copy of the record argument for the clever key.
  static keyrecord_t mod_record;
  mod_record = *record;
  mod_record.keycode = keycode;
  // Send the `keycode` key down event
  process_record(&mod_record);
  // Send the `keycode` key up event
  mod_record.event.pressed = false;
  process_record(&mod_record);
}

void replace_ongoing_key(uint16_t clever_keycode, uint16_t* ongoing_keycode, keyrecord_t* record) {
  record->keycode = clever_keycode;
  *ongoing_keycode = clever_keycode;
  set_last_keycode(clever_keycode);
  processingCK = true;
}

void process_word(uint16_t keycodes[], uint8_t num_keycodes, keyrecord_t* record) {
  for (int i = 0; i < num_keycodes; ++i) {
    invoke_key(keycodes[i], record);  // tap_code doesn't work with caps word.
  }
}

void finish_word(uint16_t keycodes[], uint8_t num_keycodes, uint16_t* ongoing_keycode, keyrecord_t* record) {
  process_word(keycodes, num_keycodes - 1, record);
  bkspc_countdown = num_keycodes - 1;
  replace_ongoing_key(keycodes[num_keycodes - 1], ongoing_keycode, record);
}

/* void finish_magic(uint16_t keycodes[], uint8_t num_keycodes, uint16_t* ongoing_keycode, keyrecord_t* record) {
  process_word(keycodes, num_keycodes - 1, record);
  replace_ongoing_key(keycodes[num_keycodes - 1], ongoing_keycode, record);
} */


void process_clever_keys(uint16_t keycode, keyrecord_t* record) {

  if (record->event.pressed) {
    uint16_t ongoing_keycode = get_ongoing_keycode(keycode, record);

    if (ongoing_keycode != KC_NO) {
      get_clever_keycode(&ongoing_keycode, record);
      store_keycode(ongoing_keycode, record);
      set_clever_keys_idle_timer(ongoing_keycode, record);
      last_keypress_time = record->event.time;

      // Global quick tap implementation for combos.
      // IS_KEYEVENT prevents combos from updating last_keypress_time, to allow combos to be chained.
      // if (IS_KEYEVENT(record->event)) { last_keypress_time = record->event.time; }
    }

  } else if (processingCK) {    // On keyrelease
    processingCK = false;
    record->keycode = recent[CK_BUFFER_SIZE - 1];
  }
}

#ifndef CK_EXPLICIT_HOOK
    bool process_record_clever_keys(uint16_t keycode, keyrecord_t* record) {
        process_clever_keys(keycode, record);
        return true;
    }
#endif  // CK_EXPLICIT_HOOK

__attribute__((weak)) bool is_letter(uint16_t keycode) {

  switch (keycode) {
    case KC_A ... KC_Z:
      return true;
    
    default:
      return false;
  }
}

#   if CLEVER_KEYS_TIMEOUT > 0
__attribute__((weak)) uint16_t get_clever_keys_timeout(uint16_t keycode) {
    switch (keycode) {

        default:
            return CLEVER_KEYS_TIMEOUT;
    }
}
#   endif  // CLEVER_KEYS_TIMEOUT > 0