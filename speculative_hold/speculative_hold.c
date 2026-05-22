

#include "speculative_hold.h"

// Events bypass Speculative Hold when there are unsettled LT keys in
// action_tapping's waiting_queue.
static uint16_t settle_timer = 0;

typedef struct {
  keypos_t key;
  uint8_t mods;
} held_key_t;

#define HELD_KEYS_SIZE 8
static held_key_t held_keys[HELD_KEYS_SIZE] = {};
static uint8_t num_held_keys = 0;
static uint8_t speculative_mods = 0;

static void held_keys_add(keypos_t key, uint8_t mods) {
  if (num_held_keys >= HELD_KEYS_SIZE) {  // Overflow!
    clear_keyboard();
    num_held_keys = 0;
  }

  held_keys[num_held_keys] = (held_key_t){
      .key = key,
      .mods = mods,
  };
  ++num_held_keys;
}

static int8_t held_keys_find(keypos_t key) {
  for (int8_t i = 0; i < num_held_keys; ++i) {
    if (KEYEQ(held_keys[i].key, key)) {
      return i;
    }
  }
  return -1;
}

static void held_keys_del_index(int8_t i) {
  if (i < num_held_keys) {
    --num_held_keys;
    for (int8_t j = i; j < num_held_keys; ++j) {
      held_keys[j] = held_keys[j + 1];
    }
  }
}

void housekeeping_task_speculative_hold(void) {
  if (settle_timer && timer_expired(timer_read(), settle_timer)) {
    settle_timer = 0;
  }
}

static uint8_t unpack_mods(uint16_t keycode) {
  const uint8_t mods5 = QK_MOD_TAP_GET_MODS(keycode);
  return (mods5 & 0x10) != 0 ? (mods5 << 4) : mods5;
}

// This hook is called for every key event, *before* QMK core processes it.
bool pre_process_record_speculative_hold(uint16_t keycode, keyrecord_t* record) {

  // Ignore events other than MT press events.
  if (record->event.pressed && (IS_QK_MOD_TAP(keycode) || IS_QK_LAYER_TAP(keycode))) {

    if (IS_QK_LAYER_TAP(keycode)) {
      // If this is an LT key, track when it will settle.
      const uint16_t term = GET_TAPPING_TERM(keycode, record);
      const uint16_t now = timer_read();
      if (!settle_timer || term > TIMER_DIFF_16(settle_timer, now)) { settle_timer = (now + term) | 1; }

    } else if (IS_KEYEVENT(record->event)) {
      // If this is an MT key and not part of a combo...
      const keypos_t key = record->event.key;
      const uint8_t mods = unpack_mods(keycode);

      if (!settle_timer && (~(get_mods() | speculative_mods) & mods) != 0 && get_speculative_hold(keycode, record)) {
        // If the mods are not already applied and get_speculative_hold() is true, then apply mods speculatively.

        held_keys_add(key, mods);

        speculative_mods |= mods;
        add_weak_mods(speculative_mods);
        send_keyboard_report();
      } else if (num_held_keys > 0) {
        held_keys_add(key, 0);
      }
    }
  }

  return true;
}

// This hook is called for every key event, *after* the core tapping processing,
// but before most other feature handlers.
bool process_record_speculative_hold(uint16_t keycode, keyrecord_t* record) {

  if (num_held_keys == 0 || !record->event.pressed) {
    return true;
  }

  add_weak_mods(speculative_mods);
  int8_t i = held_keys_find(record->event.key);

  if (IS_QK_MOD_TAP(keycode) && record->tap.count == 0) {  // MT hold press.
    if (i != -1) {

      const uint8_t cleared_mods = held_keys[i].mods;
      held_keys_del_index(i);

      // Clear the speculatively-held mod. The hold press handling will apply the mod again.
      speculative_mods &= ~cleared_mods;
      del_weak_mods(cleared_mods);
    }
  } else {  // Tap press event; cancel speculatively-held mod.
    uint8_t cleared_mods = 0;

    if (i == -1) { i = 0; }

    // Clear mods for the ith key and all keys that follow.
    for (int8_t j = num_held_keys - 1; j >= i; --j) {
      cleared_mods |= held_keys[j].mods;
    }

    num_held_keys = i;

    if (cleared_mods) {
#ifdef REPEAT_KEY_ENABLE
      // Repeat Key's process record handler runs before modules. This means the
      // current tap press event was already recorded while the speculative mods
      // were still applied. We correct for that here.
      set_last_mods(get_last_mods() & ~cleared_mods);
#endif  // REPEAT_KEY_ENABLE

      speculative_mods &= ~cleared_mods;
      del_weak_mods(cleared_mods);
      send_keyboard_report();
    }
  }

  return true;
}

__attribute__((weak)) bool get_speculative_hold(uint16_t keycode,
                                                keyrecord_t* record) {
  return (QK_MOD_TAP_GET_MODS(keycode) & (MOD_LALT | MOD_LGUI)) == 0;
}