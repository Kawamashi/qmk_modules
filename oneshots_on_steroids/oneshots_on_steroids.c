#include "oneshots_on_steroids.h"

oneshot_state_t oneshot_state[OS_COUNT] = { [0 ... OS_COUNT - 1] = os_idle };

static uint16_t idle_timer = 0;
uint16_t oneshot_holding_time[OS_COUNT] = { [0 ... OS_COUNT - 1] = 0 };
uint8_t oneshot_origin_layer[OS_COUNT] = { [0 ... OS_COUNT - 1] = 0 };

void oneshot_task(void) {
    if (idle_timer && timer_expired(timer_read(), idle_timer)) {
        clear_oneshots();
        idle_timer = 0;
    }
}

void clear_oneshot(uint8_t index) {
    oneshot_state[index] = os_idle;
    if (oneshot[index].modifier != KC_NO) { del_oneshot_mods(oneshot[index].modifier); }
    if (oneshot[index].layer != 0) { layer_off(oneshot[index].layer); }
}

void clear_oneshots(void) {
    for (uint8_t i = 0; i < OS_COUNT; i++) {
        if (oneshot_state[i] == os_up_queued) { clear_oneshot(i); }
    }
}

bool is_custom_oneshot(uint16_t keycode) {
    for (uint8_t i = 0; i < OS_COUNT; i++) {
        if (keycode == oneshot[i].trigger) { return true; }
    }
    return false;
}


bool process_record_oneshots_on_steroids(uint16_t keycode, keyrecord_t *record){
    
    bool should_continue_processing = true;

    for (uint8_t i = 0; i < OS_COUNT; i++) {

        if (record->event.pressed) {

            if (keycode == oneshot[i].trigger) {
                // Trigger keydown
                if (oneshot_state[i] == os_idle) {
                    if (oneshot[i].modifier != KC_NO) { register_mods(oneshot[i].modifier); }
                    if (oneshot[i].layer != 0) { 
                        uint8_t key_layer = read_source_layers_cache(record->event.key);
                        uint8_t default_layer = get_highest_layer(default_layer_state);
                        if (key_layer != default_layer) {
                            oneshot_origin_layer[i] = key_layer;
                            layer_off(key_layer);
                        } 
                        layer_on(oneshot[i].layer);
                    }
                    oneshot_holding_time[i] = timer_read();
                    oneshot_state[i] = os_down_unused;
                } else {
                    clear_oneshot(i);
                    if (oneshot_origin_layer[i] != 0) { layer_on(oneshot_origin_layer[i]); }
                }
                return false;
            }

        } else {

            if (oneshot_state[i] != os_idle) {

                if (keycode == oneshot[i].suppressor){

                    if (oneshot[i].modifier != KC_NO) { unregister_mods(oneshot[i].modifier); }

                    if (oneshot_state[i] == os_down_unused && timer_elapsed(oneshot_holding_time[i]) < TAPPING_TERM) {
                        oneshot_state[i] = os_up_queued;
                        if (oneshot[i].modifier != KC_NO) { set_oneshot_mods(oneshot[i].modifier); }
                        idle_timer = (record->event.time + ONESHOT_TIMEOUT) | 1;
                    } else {
                        oneshot_state[i] = os_idle;
                        if (oneshot[i].layer != 0) {layer_off(oneshot[i].layer); }
                    }
                    should_continue_processing = false;
                    continue;
                }
            }
            if (keycode == oneshot[i].trigger) { return false; }
        }
    }
    if (!should_continue_processing) { return false; }

    for (uint8_t i = 0; i < OS_COUNT; i++) {
        if (oneshot_state[i] == os_idle) { continue; }

        if (is_oneshot_cancel_key(keycode)) {
            if (record->event.pressed) {
                // Cancel oneshot on press of specific keys.
                clear_oneshot(i);
            }
            continue;
        }

        if (should_oneshot_stay_pressed(keycode)) {
            if (record->event.pressed) {
                idle_timer = (record->event.time + ONESHOT_TIMEOUT) | 1;
            }
            continue;
        }

        if (record->event.pressed) {
            // Regular key pressed
            switch (oneshot_state[i]) {
                case os_down_unused:
                    // When the mod key is still pressed
                    oneshot_state[i] = os_down_used;
                    break;
                case os_up_queued:
                    oneshot_state[i] = os_up_queued_used;
                    break;
                default:
                    break;
            }

        }
    }
    return true;
}

void post_process_record_oneshots_on_steroids(uint16_t keycode, keyrecord_t *record){
    for (uint8_t i = 0; i < OS_COUNT; i++) {
        // Regular key released / roll between two regular keys
        if (oneshot_state[i] == os_up_queued_used) {
            oneshot_state[i] = os_idle;
            if (oneshot[i].layer != 0) { layer_off(oneshot[i].layer); }
            continue;
        }
    }
}
