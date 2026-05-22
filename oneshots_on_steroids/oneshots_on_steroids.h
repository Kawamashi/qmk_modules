#pragma once

#include "quantum.h"

#ifdef __cplusplus
extern "C" {
#endif

/*Custom one-shot key structure.
 * 
 * The `trigger` field is the keycode that triggers the modifier or the layer-change.
 * The `suppressor` field is the keycode that 
 * Trigger and suppressor can be the same key, but they can be different, to drift between layers.
 * 
 * 
 * For one-shot mods, the `modifier` field must use the MOD_* prefix.
 * Modifiers can be combined, for ex. MOD_LCTL | MOD_LSFT
 * KC_NO must be used if no modifiers are triggered by the one-shot key.
 * The `target_layer` field is used for one-shot layers.
 * `_BASE` must be used 
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
void clear_oneshot(uint8_t index);

// Deactivate all one-shot keys
void clear_oneshots(void);

// Returns whether a keycode is a custom one-shot key or not
bool is_custom_oneshot(uint16_t keycode);

//void post_process_oneshot_on_steroids(uint16_t keycode, keyrecord_t *record);

// Custom one-shot keys implementation that doesn't rely on timers.
// If a mod is used while it is held it will be unregistered on keyup as normal.
// Otherwise it will be queued and only released after the next non-mod keyup.
//bool process_oneshot_on_steroids(uint16_t keycode, keyrecord_t *record);

// To be implemented by the user. Defines keys to cancel one-shot mods and layers.
bool is_oneshot_cancel_key(uint16_t keycode);

// To be implemented by the user. 
// Defines keys not to release the modifier when determining
// whether a one-shot mod has been used
// Setting this to layer change keys allows carrying one-shot modifiers between layers.
bool should_oneshot_stay_pressed(uint16_t keycode);

#ifdef __cplusplus
}
#endif
