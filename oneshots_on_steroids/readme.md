# One Shots on Steroids

*Highly customizable instant-activation one shot keys for QMK*

This is my take on how one shot keys should work: the modifier or layer is activated as soon as the key is pressed, and remains active while the key is held. If the one shot key is released before the One Shot Term without another key having been pressed in between, the modifier or layer remains active until the next key press, after which it’s deactivated.

One Shot on Steroids (OSoS) keys are *snappy*: the mod or layer is activated immediately on key down, without waiting for the tap-hold resolution like regular one shot keys. For example, with mouse, using an OSoS Shift key feels just as natural as using a regular shift key.

One Shot on Steroids keys are *forgiving*: tap an OSoS key again to cancel it. An optional timeout can also be defined, to automatically deactivate OSoS after a period of keyboard inactivity.

One Shot on Steroids keys are *flexible*: an OSoS key can be used to activate a layer alongside one or more modifiers. 

One Shot on Steroids keys are *versatile*: three optional behaviors can be enabled, individually or in combination, to better suit your needs.

&nbsp;</br>
* [Set up](#Set-up)
* [How One Shots on Steroids work](#How-One-Shots-on-Steroids-work)
   * [One Shot Term](#One-Shot-Term)
   * [Different modifiers, different behaviors](#Different-modifiers-different-behaviors)
* [Cancelling One Shots on Steroids](#Cancelling-One-Shots-on-Steroids)
* [Optional behaviors](#Optional-behaviors)
   * [One Shot Layers freed from the layer stack](#One-Shot-Layers-freed-from-the-layer-stack)
   * [Mod-absorbing One Shot Layers](#Mod-absorbing-One-Shot-Layers)
   * [Split Trigger and Hold Keys](#Split-Trigger-and-Hold-Keys)
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

First, in `config.h`, define how many One Shot on Steroids keys you’ll use and the One Shot Term (in milliseconds):
```c
#define OS_STEROIDS_COUNT 4
#define OS_STEROIDS_TERM 200
```

Then, in `keymap.c`, define custom keycodes you’ll use for One Shot on Steroids keys and their specifications in the `oneshot_os` array:
```c
enum custom_keycodes {
    // Custom keycodes for OSoS
    OS_SHFT = SAFE_RANGE,
    OS_NUM,
    OS_WNUM,
    // Other custom keycodes
};

const oneshot_on_steroids_t oneshot_os[] = {
 // {OS(trigger key, modifier,         layer   )}
    {OS(OS_SHFT,     MOD_BIT(KC_LSFT), 0       )},
    {OS(OS_NUM,      0,                _NUMBERS)},
    {OS(OS_NAV,      0,                _NAV    )},
    {OS(OS_WNUM,     MOD_BIT(KC_LGUI), _NUMBERS)}
};
```
Each entry of this array must use the one shot-type wrapper `OS(keycode, modifier, layer)`. The `modifier` field uses the `MOD_BIT()` macro (not `MOD_xxx` constants), or `0` for layer-only one shot keys. For modifier-only one shot keys, simply use `0` for the `layer` field.

&nbsp;</br>
## How One Shots on Steroids work

### One Shot Term

The modifier or layer is activated as soon as the OSoS key is pressed, and remains active while the key is held.

<img src="png/OSoS 2.png" width="600">

Therefore, the output of this sequence is `BA`:

<img src="png/OSoS 1.png" width="600">

If another key is pressed before the OSoS key is released, it behaves like a regular modifier:

<img src="png/OSoS 3.png" width="600">

if the OSoS key is released after the One Shot Term, it behaves like a regular modifier:

<img src="png/OSoS 14.png" width="600">

Otherwise, the modifier or layer remains active until the next key press, after which it’s deactivated:

<img src="png/OSoS 4.png" width="600">

If the One Shot Term is not defined, the Tapping Term is applied by default.\
For more granular control, you can add the following callback to your `keymap.c` to customize the One Shot Term for each key:
```c
uint16_t get_oneshot_on_steroids_term(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case OS_SHFT:
            return OS_STEROIDS_TERM + 125;
        case OS_NUM:
            return 130;
        default:
            return OS_STEROIDS_TERM;
    }
}
```

&nbsp;</br>
### Different modifiers, different behaviors

#### Modifiers to be held after the One Shot Term

To prevent mouse interaction, OSoS keys involving Shift and/or Ctrl unregister their modifier as soon as they are released. If the one shot behavior is triggered, the modifier will be sent alongside the other key. 

<img src="png/OSoS 5.png" width="600">

For OSoS modifiers using neither Shift nor Ctrl, the modifier remains registered. 

<img src="png/OSoS 6.png" width="600">

To modify these behaviors, you can add the following function to your `keymap.c`, and customize it:
```c
bool should_mod_be_held_after_oneshot_release(uint8_t mod, uint16_t keycode) {
    // Shift and Ctrl are not kept registered after the OSoS key is released,
    // to avoid interfering with mouse usage. If the one shot behavior is triggered,
    // `add_oneshot_mods()` is used instead.
    if (mod & (MOD_MASK_CTRL | MOD_MASK_SHIFT)) { return false; }
    return true;
}
```

#### Solution to the problem of flashing modifiers  

OSoS keys involving GUI or left Alt might cause the “flashing modifiers” problem: using such modifiers without other keys may trigger unwanted application or OS actions, like GUI opening the start menu. If you use OSoS keys involving these modifiers, I strongly recommend that you define a `DUMMY_MOD_NEUTRALIZER_KEYCODE` in your `config.h` as a keycode to which no keyboard shortcuts are bound. For example:
```c
#define DUMMY_MOD_NEUTRALIZER_KEYCODE KC_F18
```
This key will be sent in between the modifier register and unregister events of an OSoS key. This prevents programs from interpreting the modifier release as a lone modifier tap when a one shot is cancelled, avoiding unwanted OS or application actions.

By default, only left Alt and left GUI are neutralized. If you want to change the list of modifiers requiring intervention, you may also define `MODS_TO_NEUTRALIZE`.

More information can be found [here](https://docs.qmk.fm/features/key_overrides#neutralize-flashing-modifiers).

&nbsp;</br>
## Cancelling One Shots on Steroids

If you tapped an OSoS key by mistake, just tap it again to cancel it. 

A timeout can be defined in `config.h` to automatically deactivate OSoS keys after a period of keyboard inactivity:
```c
#define OS_STEROIDS_TIMEOUT 3000    // 3 seconds
```
For more granular control, you can add the following callback to your `keymap.c` to customize the timeout for each OSoS key:
```c
uint16_t get_oneshot_on_steroids_timeout(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case OS_SHFT:
            return OS_STEROIDS_TIMEOUT - 1000;
        case OS_NUM:
            return 5000; 
        default:
            return OS_STEROIDS_TIMEOUT;
    }
}
```

&nbsp;</br>
Some keys can be configured as *cancel keys*. When pressed, they immediately deactivate any active One Shot on Steroids.\
To do so, add the following callback to your `keymap.c`. In this example, `KC_ESC` and a custom keycode `CANCEL` are defined as cancel keys:
```c
bool is_oneshot_on_steroids_cancel_key(uint16_t keycode) {
    switch (keycode) {
        case KC_ESC:
        case CANCEL:  // CANCEL is a custom keycode.
            return true;
        default:
            return false;
    }
}
```

If you want a macro to cancel One Shots on Steroids, you can use some [functions](#Functions) to do that.

&nbsp;</br>
## Optional behaviors

### One Shot Layers freed from the layer stack

Imagine you have a layer for symbols you can access with a layer-tap key, and a layer for numbers. Sometimes you want to use the symbol layer while inputting numbers, and sometimes you want to insert a number while inputting symbols. If the number layer index is lower than the symbol layer one, the latter use-case is impossible. 

<img src="png/OSoS 15.png" width="600">

With the option `OS_STEROIDS_FREE_LAYER_STACK`, an OSoS layer key temporarily deactivates the layer it was pressed from, allowing it to bypass the usual layer stack restrictions. The original layer is automatically restored when the OSoS layer is deactivated. With this option, you can place OSoS layer keys on any layer without worrying about the layer stack.

<img src="png/OSoS 7.png" width="600">

If the one shot behavior is triggered, the associated layer remains active until the next key press, the symbol layer will be reactivated afterwards.

<img src="png/OSoS 8.png" width="600">

If the symbol layer is deactivated while the one shot layer on steroids is active, it won’t be reactivated.

<img src="png/OSoS 9.png" width="600">

To enable this behavior, add the following to your `config.h`:
```c
#define OS_STEROIDS_FREE_LAYER_STACK
```

For finer control, you can add the following callback to your `keymap.c`. You can customize the behavior based on the originating layer, the OSoS key being pressed, or both. 
In this example, the `_NUMBERS` layer can’t be deactivated by OSoS layer keys, and `OS_WNUM` doesn’t deactivate the layer it comes from:
```c
bool should_oneshot_on_steroids_deactivate_layer(uint16_t keycode, uint8_t layer) {
    // Prevent _NUMBERS layer from being deactivated
    if (layer == _NUMBERS) { return false; }

    switch (keycode) {
        case OS_WNUM:    
            return false;    // OS_WNUM doesn't deactivate layers
        default:
            return true;     // Other OSoS 
    }
}
```

&nbsp;</br>
### Mod-absorbing One Shot Layers

Imagine you have a navigation layer. Using this layer while holding `GUI` for window‑management can be cumbersome. With the option `OS_STEROIDS_ABSORB_MODS`, an active modifier when an OSoS layer key is pressed remains registered as long as the layer is active.

If an OSoS layer key is pressed after a one shot modifier (vanilla or OSoS), it absorbs that modifier:

<img src="png/OSoS 10.png" width="600">

If an OSoS layer key is pressed while a modifier (basic, mod-tap, one shot, etc.) is held, it absorbs that modifier:

<img src="png/OSoS 11.png" width="600">

If another key is pressed before the modifier key is released, the modifier is considered used and is deactivated when released:

<img src="png/OSoS 12.png" width="600">

To enable this behaviour,  add the following to your `config.h`:
```c
#define OS_STEROIDS_ABSORB_MODS
```

If you need further customization, you can add the following callback to your `keymap.c`, and modify it to suit your needs. For example:

```c
bool should_osl_on_steroids_absorb_mods(uint16_t keycode) {
    const uint8_t mods = get_mods() | get_oneshot_mods();

    switch (keycode) {
        case OS_NAV:
            // OS_NAV doesn't absorb Alt-Gr
            if (mods & MOD_BIT(KC_ALGR)) { return false; }
            break;

        case OS_NUM:
            // OS_NUM only absorbs Shift
            if (mods & ~MOD_MASK_SHIFT) { return false; }
            break;

        case OS_WNUM:
            // OS_WNUM doesn't absorb mods
            return false;

        default:
            // Other OSoS layer keys absorb mods
            return true;
    }
}
```

Similar to `get_mods()`, `get_absorbed_mods()` returns the modifiers currently absorbed by an OSoS layer.

&nbsp;</br>
### Split Trigger and Hold Keys
*Two keys for One Shot: inspired by [Lobre’s Shaka gesture](https://github.com/lobre/shaka34/blob/main/gesture/README.md)*

Imagine you want to access your tertiary layers, such as function or media layers, with OSoS keys. There is no room for them on your base layer, so they will be on a secondary layer. But your secondary layers are pretty crowded too, leaving only less ergonomic key positions. Holding keys outside of the home‑row can be tiring, so why not use the comfortable thumb key you’re already holding to access the secondary layer ? If you can’t picture this use case, I strongly encourage you to read [Lobre’s write-up](https://github.com/lobre/shaka34/blob/main/gesture/README.md).

Defining `OS_STEROIDS_SPLIT_TRIGGER_HOLD` in your `config.h` allows you to separate the key that triggers the one shot from the key that keeps it active:
```c
#define OS_STEROIDS_SPLIT_TRIGGER_HOLD
```
The one shot-type wrapper now uses another parameter: `OS(trigger key, holding key, modifier, layer)`. You may have to modify existing OSoS array accordingly:
```c
const oneshot_on_steroids_t oneshot_os[] = {
 // {OS(trigger key, holding key, modifier,         layer   )}
 // "Regular" OSoS keys: the trigger key and the holding key are the same
    {OS(OS_SHFT,     OS_SHFT,     MOD_BIT(KC_LSFT), 0       )},
    {OS(OS_NUM,      OS_NUM,      0,                _NUMBERS)},
    {OS(OS_NAV,      OS_NAV       0,                _NAV    )},
 // OSoS keys with a holding key different from the trigger key
    {OS(OS_WINM,     LT_NAV,      0,                _WINMGMT)},
    {OS(OS_WNUM,     LT_NUM,      MOD_BIT(KC_LGUI), _NUMBERS)},
};
```


On my navigation layer, I have `OS_WINM`, a OSoS layer key to activate my windows-management layer. I access the nav layer with `LT_NAV`, a thumb layer-tap key which is also used to hold the OSoS.

<img src="png/OSoS 13.png" width="1100">

- To access the windows-management layer, I press `LT_NAV` then `OS_WINM`.
- I can release `OS_WINM` at any time, it doesn’t matter.
- To trigger the one shot behavior, I release `LT_NAV` within the One Shot Term. The navigation layer is deactivated, while the windows-management layer remains active until the next key press.
- To stay longer on the windows-management layer, I continue to hold `LT_NAV`. Releasing `LT_NAV` deactivates both the navigation and the windows‑management layers. 

It’s possible to go back and forth between a secondary and a tertiary layer by placing the same OSoS layer key at the same position on both layers. Pressing this key on the tertiary layer cancels the OSoS, thereby deactivating the associated layer. If `LT_NAV` is still held, this brings you back to the navigation layer.

&nbsp;</br>
## Other customization options

When I developed One Shots on Steroids, I spent a lot of time determining whether modifier keys, layer-changer keys, one shot keys (vanilla or OSoS) should “consume” a one shot on steroids. The default setting should suit most use cases. However, there is an exception: if you use an OSoS layer key to access OSoS modifier keys as Callum modifiers, add the following to your `config.h`:
```c
#define OS_MOD_SHOULD_LEAVE_OS_LAYER
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

    if (!is_mod_key && !is_layer_key) { return false; }

    // Mod or layer-change key pressed after an OSoS key
    if (is_oneshot_layer_on_steroids(oneshot)) {
        // If a layer-change key is pressed after an OSL, the OSL must be reset.
        if (is_layer_key) { return false; }
        // keycode is not a layer key, it’s a mod key.
#           ifdef OS_MOD_SHOULD_LEAVE_OS_LAYER
        // When using OSM as Callum mods, an OSL tapped before must be reset.
        if (is_oneshot_mod_on_steroids(keycode)) { return false; }
#           endif  // OS_MOD_SHOULD_LEAVE_OS_LAYER
        // Standard behavior, like any mod key after an OSL
        return true;
    } else {
        // one shot is OSM on steroids
#           ifdef OS_STEROIDS_ABSORB_MODS
        if (is_oneshot_layer_on_steroids(keycode)) {
            if (should_oneshot_on_steroids_absorb_mods(keycode)) { return false; }
        }
#           endif  // OS_STEROIDS_ABSORB_MODS
        // OSM on steroids should stay pressed
        // whether keycode is a mod or a layer-change key.
        return true;
    }
}
```
&nbsp;</br>

If you want an OSoS key to have completely customized behavior, add the following callback to your `keymap.c`. It is called before the OSoS logic is executed. Returning `false` skips the default One Shot on Steroids behavior and prevents further processing of the key event.

Here is an example of two use-cases: completely replacing the behavior of an OSoS key, and modifying the context before processing the key normally.

```c
bool is_oneshot_on_steroids_custom_behavior(uint16_t keycode, keyrecord_t* record) {
    switch (keycode) {

        case OS_NAV:
            // If OS_NAV is pressed while OS_SHFT is active,
            // deactivate OS_SHFT and turn caps word on
            // instead of normal processing of OS_NAV.
            if (get_oneshot_on_steroids_state(OS_SHFT) > 0) {
                clear_oneshot_mods_on_steroids();
                caps_word_on();
                return false;
            }
            break;

#   ifdef OS_STEROIDS_ABSORB_MODS
        case OS_NUM:
            // If OS_NUM is pressed while caps word is active,
            // turn it off and send a one shot shift so that OS_NUM absorbs it.
            // Process OS_NUM normally afterwards.
            if (is_caps_word_on()) {
                caps_word_off();
                add_oneshot_mods(MOD_BIT(KC_LSFT));
            }
            return true;
#   endif  // OS_STEROIDS_ABSORB_MODS

        default:
            // Process other OSoS keys normally.
            return true;
    }
}
```

&nbsp;</br>
## Functions
You can manipulate One Shot on Steroids with these functions:


| Function                                 | Description                                                                | 
| ---------------------------------------- | -------------------------------------------------------------------------- | 
| `get_oneshot_on_steroids_state(keycode)` | If `keycode` is an OSoS key, returns its state. Otherwise, returns -1.     |
| `get_oneshot_layer_on_steroids()`        | If there is an active OSoS layer, returns the layer. Otherwise, returns 0. |
| `is_oneshot_on_steroids(keycode)`        | Returns whether `keycode` is an OSoS key.                                  |
| `is_oneshot_layer_on_steroids(keycode)`  | Returns whether `keycode` is an OSoS layer key.                            |
| `is_oneshot_mod_on_steroids(keycode)`    | Returns whether `keycode` is a modifier-only OSoS key.                     |
| `is_oneshot_layer_on_steroids_active()`  | Returns whether an OSoS layer is active.                                   |
| `cancel_oneshot_on_steroids(keycode)`    | Deactivates the OSoS key triggered by `keycode`.                           |
| `clear_oneshots_on_steroids()`           | Deactivates all OSoS keys.                                                 |
| `reset_oneshot_layer_on_steroids()`      | Deactivates the active OSoS layer key.                                     |
| `del_oneshot_mods_on_steroids(mods)`     | Deactivates all OSoS keys using `mods`.                                    |
| `clear_oneshot_mods_on_steroids()`       | Deactivates all OSoS keys using any modifier.                              |
