# One-Shots on Steroids

*Highly customizable instant-activation one-shot keys for QMK*

[WIP]

This is my take on how one-shot keys should work: the modifier or the layer is activated as soon as you press the key, and as long as you hold it. If the one-shot key is released before the One‑Shot Term without another key having been pressed in between, then it behaves as a one‑shot key for the next keypress.

One-Shot on Steroids (OSoS) keys are *snappy*: the mod or layer is activated immediately on key down, without waiting for the tap-hold resolution like regular one-shot keys. For example, with mouse, using an OSoS Shift key feels just as natural as using a regular shift key.

One-Shot on Steroids keys are *forgiving*: tap an OSoS key again to cancel it. An optional timeout can also be defined, to automatically deactivate OSoS after a period of keyboard inactivity.

One-Shot on Steroids keys are *flexible*: an OSoS key can be used to activate a layer alongside one or more modifiers. 

One-Shot on Steroids keys are *versatile*: three optional behaviors can be activated (and potentially combined) to better suit your needs.

* [Set up](#Set-up)
* [How One-Shots on Steroids work](#How-One-Shots-on-Steroids-work)
   * [One-Shot Term](#One-Shot-Term)
   * [Different modifiers, different behaviors](#Different-modifiers,different-behaviors)
* [Cancelling One-Shots on Steroids](#Cancelling-One-Shots-on-Steroids)
* [Optional behaviors](#Optional-behaviors)
   * [One-Shot Layers freed from the layer stack](#One-shot-Layers-freed-from-the-layer-stack)
   * [Mod-absorbing One-Shot Layers](#Mod-absorbing-One-Shot-Layers)
   * [Two keys for one-shot](#Two-keys-for-one-shot)
* [Other customization options](#Other-customization-options)
* [Functions](#Functions)


&nbsp;</br>
## Set up

Add the following to the list of modules in your `keymap.json` to enable this module:
```json
{
    "modules": ["Kawamashi/oneshots_on_steroids"]
}
```
&nbsp;</br>

First, in `config.h`, define how many One-Shot on Steroids keys you’ll use and the One‑Shot Term:
```c
#define OS_STEROIDS_COUNT 3
#define OS_STEROIDS_TERM 200
```

Then, in `keymap.c`, define custom keycodes you’ll use for One-Shot on Steroids keys and their specifications in the `oneshot` array:
```c
// custom keycodes for OSoS
enum custom_keycodes {
    OS_SHFT = SAFE_RANGE,
    OS_NUMR,
    OS_WNUM,
    // Other custom keycodes
};

const oneshot_t oneshot[] = {
 // {OS(trigger key, modifier,         layer  )}
    {OS(OS_SHFT,     MOD_BIT(KC_LSFT), 0      )},
    {OS(OS_NUMR,     0,                _NUMROW)},
    {OS(OS_WNUM,     MOD_BIT(KC_LGUI), _NUMROW)}
};
```
Each line of this array must use the oneshot-type wrapper `OS(keycode, modifier, layer)`. The `modifier` field uses the `MOD_BIT()` macro, or `0` for layer-only oneshots. It’s the same for modifier-only oneshots, with `0` in the `layer` field.

&nbsp;</br>
## How One-Shots on Steroids work

### One-Shot Term

One-Shot on Steroids keys work simply: the modifier or the layer is activated as soon as you press the key, and remains active while the key is held.

<img src="png/OSoS 2.png" width="600">

Therefore, the output of this sequence is `BA`:

<img src="png/OSoS 1.png" width="600">

If the OSoS key is released before the One-Shot Term, but a keycode is tapped in between, it behaves like a regular modifier. 

<img src="png/OSoS 3.png" width="600">

If the OSoS key is released before the One-Shot Term without another key having been pressed in between, then it behaves as a one‑shot key for the next keypress. 

<img src="png/OSoS 4.png" width="600">

If the One-Shot Term is not defined, the Tapping Term is applied by default. For more granular control, you can add the following to your `config.h`:
```c
#define OS_STEROIDS_TERM_PER_KEY
```

You can then add the following function to your `keymap.c` to customize the one-shot term for each key:
```c
uint16_t get_oneshot_on_steroids_term(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case OS_SHFT:
            return OS_STEROIDS_TERM + 125;
        case OS_NUMR:
            return 130;
        default:
            return OS_STEROIDS_TERM;
    }
}
```

&nbsp;</br>
### Different modifiers, different behaviors

#### Modifiers to be held after the One-Shot Term

To prevent mouse interaction, OSoS modifiers using Shift and/or Ctrl unregister their modifier as soon as they are released. If the one-shot behavior is triggered, the modifier will be sent alongside the other key. 

<img src="png/OSoS 5.png" width="600">

For OSoS modifiers using neither Shift nor Ctrl, the modifier remains registered. 

<img src="png/OSoS 6.png" width="600">

To modify these behaviors, you can add the following function to your `keymap.c`, and customize it:
```c
bool should_mod_be_held_after_oneshot_term(uint8_t mod, uint16_t trigger) {
    // shift and ctrl shouldn't be held after the one-shot term,
    // using `add_oneshot_mods()` instead, not to interfere with the mouse
    if (mod & (MOD_MASK_CTRL | MOD_MASK_SHIFT)) { return false; }
    return true;
}
```

#### Solution to the problem of flashing modifiers  

OSoS modifiers involving GUI or left Alt might cause the “flashing modifiers” problem: using such modifiers without other keys may trigger application or OS actions, like GUI opening the start menu when it is not desired. If you use OSoS keys involving these modifiers, I strongly recommend that you define a `DUMMY_MOD_NEUTRALIZER_KEYCODE` in your `config.h` as a keycode to which no keyboard shortcuts are bound. This key will be sent in between the register and unregister events of an OSoS key. That way, the programs on your computer will no longer interpret the mod suppression induced by cancellation of a one‑shot as a lone tap of a modifier key and will thus not falsely trigger the undesired action.

By default, only left Alt and left GUI are neutralized. If you want to change the list of modifiers requiring intervention, you may also define `MODS_TO_NEUTRALIZE`.

You can find more information [here](https://docs.qmk.fm/features/key_overrides#neutralize-flashing-modifiers).

&nbsp;</br>
## Cancelling One-Shots on Steroids

Cancelling One-Shots on Steroids is easy. First, a timeout can be defined in your `config.h`, to automatically deactivate one-shot keys after a period of keyboard inactivity:
```c
#define OS_STEROIDS_TIMEOUT 3000
```
If you activate an OSoS key by mistake, just tap it again to cancel it. 

Some keys can be configured as *cancel keys*. For that, add the following to your `config.h`:
```c
#define OS_STEROIDS_CANCEL_KEY
```
Then, add the following function to your `keymap.c`, and customize it:
```c
bool is_oneshot_on_steroids_cancel_key(uint16_t keycode) {
    switch (keycode) {

        default:
            return false;
    }
}
```

If you want a macro to cancel One-Shots on Steroids, you can use some [functions](#Functions) to do that.

&nbsp;</br>
## Optional behaviors

### One-Shot Layers freed from the layer stack

Imagine you have a layer for symbols you can access with a layer-tap key, and a layer for numbers. Sometimes you want to use the symbol layer while inputting numbers, and sometimes you want to insert a number while inputting symbols. If the number layer index is lower than the symbol layer one, the latter use-case is impossible. 

If you define `OS_STEROIDS_FREE_LAYER_STACK` in your `config.h`, an OSoS layer key temporarily disables the layer it comes from, not to be limited by the layer stack. This layer is reactivated as soon as the one-shot layer is deactivated. With this option, you can use an OSoS layer key on your symbol layer without needing to worry about the layer stack anymore.

<img src="png/OSoS 7.png" width="600">

If the one-shot behavior is triggered, the associated layer remains active until another key is pressed, the symbol layer will be reactivated afterwards.

<img src="png/OSoS 8.png" width="600">

If the symbol layer is deactivated while the one-shot layer on steroids is active, it won’t be reactivated.

<img src="png/OSoS 9.png" width="600">

If you need further customization, you can add the following to your `config.h`:
```c
#define OS_STEROIDS_FREE_LAYER_STACK_PER_KEY
```
Then, add the following function to your `keymap.c`, and modify it to suit your needs:
```c
bool should_oneshot_on_steroids_deactivate_layer(uint16_t keycode, uint8_t layer, keyrecord_t* record) {
    switch (layer) {

        default:
            return true;
    }
}
```

&nbsp;</br>
### Mod-absorbing One-Shot Layers

Imagine you have a navigation layer. Using this layer while holding `GUI` for window‑management can be cumbersome. If you define `OSL_STEROIDS_ABSORB_MODS` in your `config.h`, an active modifier when triggering an OSoS layer key will be applied as long as the layer is active. It works with a one‑shot modifier (vanilla or OSoS):

<img src="png/OSoS 10.png" width="600">

It also works if a modifier (basic, mod-tap, one-shot, etc.) is held when the OSoS layer key is tapped:

<img src="png/OSoS 11.png" width="600">

If a key is pressed before the modifier key is released, the modifier is considered to have been used: therefore it’s deactivated as soon as it’s released.

<img src="png/OSoS 12.png" width="600">

To check if a modifier has been absorbed by a one-shot layer on steroids, you can use this function: `has_mod_been_absorbed_by_osl(mod)`

If you need further customization, you can add the following function to your `keymap.c`, and modify it to suit your needs:

```c
bool should_osl_on_steroids_absorb_mods(uint16_t keycode) {
    switch (keycode) {

        default:
            return true;
    }
}
```

&nbsp;</br>
### Two keys for one-shot
*Inspired by [Lobre’s Shaka gesture](https://github.com/lobre/shaka34/blob/main/gesture/README.md)*

Imagine you want to access your tertiary layers, such as function or media layers, with OSoS keys. There is no room for them on your base layer, so they will be on a secondary layer. But your secondary layers are pretty crowded too, leaving only less ergonomic positions. Holding keys outside of the home‑row can be tiring, so why not use another key instead ? Like the comfortable thumb key you’re already holding to access your secondary layer! If you can’t picture this use case, I strongly encourage you to read [Lobre’s write-up](https://github.com/lobre/shaka34/blob/main/gesture/README.md).

Defining `OS_STEROIDS_HOLD_WITH_OTHER_KEY` in your `config.h` allows you to separate the key that triggers the one-shot from the key that keeps it active. The oneshot-type wrapper now uses another parameter: `OS(trigger key, holding key, modifier, layer)`. You may have to modify existing OSoS array accordingly:
```c
const oneshot_t oneshot[] = {
 // {OS(trigger key, holding key, modifier,         layer  )}
 // "Regular" OSoS keys: the trigger key and the holding key are the same
    {OS(OS_SHFT,     OS_SHFT,     MOD_BIT(KC_LSFT), _BASE    )},
    {OS(OS_NUMR,     OS_NUMR,     0,                _NUMROW  )},
 // OSoS keys with a holding key different from the trigger key
    {OS(OS_WINM,     LT_NAV,      0,                _FUNCAPPS)},
    {OS(OS_WNUM,     LT_NUM,      MOD_BIT(KC_LGUI), _NUMROW  )},
};
```
<img src="png/OSoS 13.png" width="1100">

On my navigation layer, I have `OS_WINM`, a OSoS layer key to activate my windows-management layer. I access the nav layer with `LT_NAV`, a thumb layer-tap key which is also used to hold the OSoS. This way:
- To access the windows-management layer, I press `LT_NAV` then `OS_WINM`.
- I can release `OS_WINM` at any time, it doesn’t matter.
- To trigger the one-shot behavior, I release `LT_NAV` within the One-Shot Term. It deactivates the navigation layer. The windows‑management layer remains active until the next keypress.
- To stay longer on the windows-management layer, I continue to hold `LT_NAV`. Releasing `LT_NAV` deactivates the navigation and the windows‑management layers. 

It’s possible to go back and forth between a secondary and a tertiary layer by placing the same OSoS layer key at the same spot on both layers. Pressing this key on the tertiary layer cancels the OSoS, thereby deactivating the associated layer. If `LT_NAV` is still held, this brings you back to the navigation layer.

&nbsp;</br>
## Other customization options

When I developped One-Shots on Steroids, I spent a lot of time determining whether modifier keys, layer-changer keys, one-shot keys (vanilla or OSoS) should “consume” a one-shot on steroids. The default setting should be suitable for the vast majority of use cases. However, there is an exception: if you use an OSoS layer key to access OSoS modifier keys as Callum modifiers, add the following to your `config.h`:
```c
#define OSM_SHOULD_LEAVE_OSL_LAYER
```

If you need further customization, you can add the following function to your `keymap.c`, and modify it to suit your needs:

```c
bool should_oneshot_on_steroids_ignore_key(uint16_t keycode, uint16_t oneshot, keyrecord_t* record) {

    bool is_mod_key = is_oneshot_mod_on_steroids(keycode);
    bool is_layer_key = is_oneshot_layer_on_steroids(keycode);

    switch (keycode) {
        // mod keys.
        case QK_MOD_TAP ... QK_MOD_TAP_MAX:
            if (record->tap.count) { break; }
        case KC_LCTL ... KC_RGUI:
        case KC_HYPR:
        case KC_MEH:
        case QK_ONE_SHOT_MOD ... QK_ONE_SHOT_MOD_MAX:
            is_mod_key = true;
            break;

        // layer switch keys.
        case QK_LAYER_TAP ... QK_LAYER_TAP_MAX:
            if (record->tap.count) { break; }
        case QK_LAYER_TAP_TOGGLE ... QK_LAYER_TAP_TOGGLE_MAX:
        case QK_MOMENTARY ... QK_MOMENTARY_MAX:
        case QK_ONE_SHOT_LAYER ... QK_ONE_SHOT_LAYER_MAX:
        case QK_TO ... QK_TO_MAX:
        case QK_TOGGLE_LAYER ... QK_TOGGLE_LAYER_MAX:
        case QK_TRI_LAYER_LOWER ... QK_TRI_LAYER_UPPER:
            is_layer_key = true;
            break;
    }

    // Mod or layer-change key applied after one-shot on steroids
    if (is_mod_key || is_layer_key) {
        if (is_oneshot_layer_on_steroids(oneshot)) {
            // If a layer-change key is pressed after an OSL, the OSL must be reset.
            if (is_layer_key) { return false; }
            // keycode is not a layer key, it’s a mod key.
#               ifdef OSM_SHOULD_LEAVE_OSL_LAYER
            // When using OSM as Callum mods, an OSL tapped before must be reset.
            if (is_oneshot_mod_on_steroids(keycode)) { return false; }
#               endif  // OSM_SHOULD_LEAVE_OSL_LAYER
            // Standard behavior, like any mod key after an OSL
            return true;
        } else {
            // one-shot is OSM on steroids
#               ifdef OSL_STEROIDS_ABSORB_MODS
            if (is_oneshot_layer_on_steroids(keycode)) {
                if (should_osl_on_steroids_absorb_mods(keycode)) { return false; }
            }
#               endif  // OSL_STEROIDS_ABSORB_MODS
            // OSM on steroids should stay pressed
            // whether keycode is a mod or a layer-change key.
            return true;
        }
    }
    return false;
}
```
&nbsp;</br>

If you want an OSoS key to have completely customized behavior, there is a callback called at the very beginning of the code, before the one‑shot logic is executed. For that, add the following function to your `keymap.c`, and modify it to suit your needs:
```c
bool is_oneshot_on_steroids_custom_behavior(uint16_t keycode, keyrecord_t* record) {
    switch (keycode) {

        default:
            return true;
    }
}
```
If the function returns `false`, QMK will stop processing the key event.

&nbsp;</br>
## Functions
You can manipulate One-Shot on Steroids with these functions:


| Function                                 | Description                                                                | 
| ---------------------------------------- | -------------------------------------------------------------------------- | 
| `cancel_oneshot_on_steroids(keycode)`    | Deactivates the OSoS key triggered by `keycode`.                           |
| `get_oneshot_on_steroids_state(keycode)` | If `keycode` is an OSoS key, returns its state. Otherwise, returns -1.     |
| `get_oneshot_on_steroids_index(keycode)` | If `keycode` is an OSoS key, returns its index. Otherwise, returns -1.     |
| `get_oneshot_layer_on_steroids()`        | If there is an active OSoS layer, returns the layer. Otherwise, returns 0. |
| `is_oneshot_on_steroids(keycode)`        | Returns whether `keycode` is an OSoS key.                                  |
| `is_oneshot_layer_on_steroids(keycode)`  | Returns whether `keycode` is an OSoS layer key.                            |
| `is_oneshot_mod_on_steroids(keycode)`    | Returns whether `keycode` is a modifier-only OSoS key.                     |
| `is_oneshot_layer_on_steroids_active()`  | Returns whether an OSoS layer is active.                                   |
| `clear_oneshots_on_steroids()`           | Deactivates all OSoS keys.                                                 |
| `reset_oneshot_layer_on_steroids()`      | Deactivates the active OSoS layer key.                                     |
| `del_oneshot_mods_on_steroids(mods)`     | Deactivates all OSoS keys using `mods`.                                    |
| `clear_oneshot_mods_on_steroids()`       | Deactivates all OSoS keys using any modifier.                              |
