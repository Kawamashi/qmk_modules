# One-Shots on Steroids

[WIP]

This is my attempt for one-shot keys that work as I think they should: the modifier or the layer is sent as soon as you press the key, and as long as you hold it. If the one-shot key is released before the One‑Shot Term without another key having been pressed in between, then it behaves as a one‑shot key for the next keypress.

One-Shot on Steroids (OSoS) are *snappy*: the mod or layer is applied immediately on key down, without waiting for the tap-hold resolution like regular one-shot keys. For example, with mouse, using a OSoS Shift fells as natural as a regular shift key.

One-Shot on Steroids are *forgiving*: tap again a OSoS key to cancel it. An optional timeout can also be defined, to automatically deactivate OSoS after a period of keyboard inactivity.

One-Shot on Steroids are *nimble*: a OSoS key can be used to activate a layer alongside one or several modifier(s). 

One-Shot on Steroids are *versatile*: three optional behaviours can be activated (and potentially combined) to better suit your needs.

* [Set up](#Set-up)
* [How One-Shots on Steroids work](#How-One-Shots-on-Steroids-work)
* [Cancelling One-Shots on Steroids](#Cancelling-One-Shots-on-Steroids)
* [Optional behaviours](#Optional-behaviours)
* [Layer stack](#One-Shot-Layers-freed-from-the-layer-stack)
* [Further customization](#Further-customization)


&nbsp;</br>
## Set up

Add the following to the list of modules in your `keymap.json` to enable this module:
```json
{
    "modules": ["Kawamashi/oneshots_on_steroids"]
}
```
&nbsp;</br>

First, in `config.h`, declare the number of One-Shot on Steroids, the one-shot term and the optional timeout:
```c
#define OS_STEROIDS_COUNT 3
#define OS_STEROIDS_TERM 200
#define OS_STEROIDS_TIMEOUT 3000
```

Then, in `keymap.c`, define custom keycodes you’ll use for One-Shot on Steroids and their specifications in the `oneshot` array:
```c
// custom keycodes for OSoS
enum custom_keycodes {
  OS_SHFT = SAFE_RANGE,
  OS_NUMR,
  OS_WNUM,
  // Other custom keycodes
};

const oneshot_t oneshot[] = {
  {OS(OS_SHFT, MOD_BIT(KC_LSFT),  0     )},
  {OS(OS_NUMR, 0,                _NUMROW)},
  {OS(OS_WNUM, MOD_BIT(KC_LGUI), _NUMROW)}
}
```
Each line of this array must use the oneshot-type wrapper `OS(keycode, modifier, layer)`. The `modifier` field uses the `MOD_BIT()` macro, or `0` for layer-only oneshots. It’s the same for mods-only oneshots, with `0` in the `layer` field.

&nbsp;</br>
## How One-Shots on Steroids work

### One-Shot Term

One-Shots on Steroids work simply: the modifier or the layer is sent as soon as you press the key, and as long as you hold it.

<img src="png/OSoS 2.png" width="600">

Therefore, the output of this sequence is `BA`:

<img src="png/OSoS 1.png" width="600">

If the one-shot is held for less time than the One-Shot Term, but a keycode is tapped in between, it behave like a regular modifier. 

<img src="png/OSoS 3.png" width="600">

If the one-shot key is released before the One-Shot Term without another key having been pressed in between, then it behaves as a one‑shot key for the next keypress. 

<img src="png/OSoS 4.png" width="600">

If the one-shot term is not defined, the Tapping Term is applied by default. For more granular control, you can add the following to your `config.h`:
```c
#define OS_STEROIDS_TERM_PER_KEY
```

You can then add the following function to your `keymap.c` to customise the one-shot term for each key:
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
### Different modifiers, different behaviours

#### Modifiers to be held after the One-Shot Term

To prevent mouse interaction, One-Shot mods on Steroids using Shift and/or Ctrl stop sending their modifier(s) as soon as they are released. If the one-shot behaviour is triggered, the modifier will be sent alongside the other key. 

<img src="png/OSoS 5.png" width="600">

For One-Shot mods on Steroids using neither Shift or Ctrl, the modifier is sent all the way. 

<img src="png/OSoS 6.png" width="600">

To modify these behaviours, you can add the following function to your `keymap.c`, and customise it:
```c
bool should_mod_be_held_after_oneshot_term(uint8_t mod, uint16_t trigger) {
    // shift and ctrl shouldn't be held after the one-shot term,
    // using `add_oneshot_mods()` instead, not to interfere with the mouse
    if (mod & (MOD_MASK_CTRL | MOD_MASK_SHIFT)) { return false; }
    return true;
}
```

#### Solution to the problem of flashing modifiers  

One-Shot Mods on Steroids involving GUI or left Alt might cause the “flashing modifiers” problem: using such modifiers without other keys may trigger application actions, like GUI opening the start menu when it is not desired. If you use One-Shot Mods on Steroids involving these modifiers, I strongly recommand that you define a `DUMMY_MOD_NEUTRALIZER_KEYCODE` in your `config.h` as a keycode to which no keyboard shortcuts are bound. This key will be sent in between the register and unregister events of a One-Shot on Steroids key. That way, the programs on your computer will no longer interpret the mod suppression induced by cancellation of a one‑shot as a lone tap of a modifier key and will thus not falsely trigger the undesired action.

By default, only left Alt and left GUI are neutralized. If you want to change the list of mods requiring intervention, you may also define `MODS_TO_NEUTRALIZE`.

You can find more information [here](https://docs.qmk.fm/features/key_overrides#neutralize-flashing-modifiers).

&nbsp;</br>
## Cancelling One-Shots on Steroids

&nbsp;</br>
## Optional behaviours

### One-Shot Layers freed from the layer stack

Imagine you have a layer for symbols you can access with a layer-tap key, and a layer for numbers. Sometimes you want to use the symbol layer while inputting numbers, and sometimes you want to insert a number while inputting symbols. If the number layer index is lower than the symbol layer one, the latter use-case is impossible. 

If you add `OS_STEROIDS_FREE_LAYER_STACK` to your `config.h`, One-Shot Layer on Steroids disables the layer it comes from, not to be limited by the layer stack. This layer is reactivated as soon as the one-shot layer is deactivated. With this option, you can use a One-Shot Layer on Steroids on your symbol layer without need to worry about the layer stack anymore.

<img src="png/OSoS 7.png" width="600">

If the one-shot behaviour is triggered, the associated layer remains active until another key is pressed, the symbol layer will be reactivated afterwards.

<img src="png/OSoS 8.png" width="600">

If the symbol layer is deactivated while the one-shot layer on steroids is active, it won’t be reactivated.

<img src="png/OSoS 9.png" width="600">

If you need further customization, you can add `OS_STEROIDS_FREE_LAYER_STACK_PER_KEY` to your `config.h`, and customize this function on your `keymap.c`:

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

Imagine you have a navigation layer. Regularly, you use this layer while holding `GUI` for window‑management, which can be cumbersome. If you add `OSL_STEROIDS_ABSORB_MODS` to your `config.h`, an ongoing modifier when triggering a One-Shot Layer on Steroids will be applied as long as the layer is active. It works with a one-shot modifier (vanilla or on steroids):

<img src="png/OSoS 10.png" width="600">

It also works if a modifier (basic, mod-tap, one-shot, etc.) is held when the one-shot layer on steroids is tapped:

<img src="png/OSoS 11.png" width="600">

If a key is pressed before the modifier key is released, the modifier is considered to have been used: therefore it’s deactivated as soon as it’s released.

<img src="png/OSoS 12.png" width="600">

If you need further customization, you can add and customize this function on your `keymap.c`:

```c
bool should_osl_on_steroids_absorb_mods(uint16_t keycode) {
    switch (keycode) {

        default:
            return true;
    }
}
```

## Further customization
is_oneshot_on_steroids_custom_behaviour
should_oneshot_on_steroids_ignore_key
