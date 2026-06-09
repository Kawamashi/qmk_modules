# One-Shots on Steroids

[WIP]

## One-Shot on Steroids philosophy

This is my attempt for one-shot keys that work as I think they should: the modifier or the layer is sent as soon as you press the key, and as long as you hold it. If the one-shot key is released before Tapping Term without another key having been pressed in between, then it behaves as a one‑shot key for the next keypress.

One-Shot on Steroids (OSoS) are *snappy*: the mod or layer is applied immediately on key down, without waiting for the tap-hold resolution like regular one-shot keys. For example, with mouse, using a OSoS Shift is as natural as a regular shift key.

One-Shot on Steroids are *forgiving*: tap again a OSoS key to cancel it. An optional timeout can also be defined, to automatically deactivate OSoS after a period of keyboard inactivity.

One-Shot on Steroids are *versatile*: a OSoS key can be used to activate a layer alongside one or several modifier(s). 

One-Shot on Steroids are *customizable*: three optional behaviours can be activated (and possibly combined) to better suit your needs

&nbsp;</br>
## Set up

Add the following to the list of modules in your `keymap.json` to enable this module:
```json
{
    "modules": ["Kawamashi/oneshots_on_steroids"]
}
```
&nbsp;</br>

First, in `config.h`, declare the number of One-Shot on Steroids, and the optional timeout:
```c
#define OS_STEROIDS_COUNT 3
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
