# Layer Word

**Layer Word** is similar in concept to Caps Word, but applied to layers. When a Layer Word is enabled, the matching layer is activated, and remains active until a word-breaking character is tapped. For example, the Layer Word related to my number layer remains active as long as I type numeric symbols. The keycodes that allow each Layer Word to continue are user-defined.

&nbsp;</br>
## Set up

Add the following to the list of modules in your `keymap.json` to enable this module:
```json
{
    "modules": ["Kawamashi/layer_word"]
}
```
&nbsp;</br>

Setting up a Layer Word is very simple. First, in `keymap.c`, define a custom keycode you’ll use to initiate a Layer Word, for example, `NAVWORD`. Layer-tap keys can also be used as triggers, allowing you to momentarily activate a layer on hold and triggering a Layer Word on tap. Then, simply configure the function `layerword_layer_from_trigger` to bind these keycodes to layers: 

```c
// custom keycode for NAVWORD
enum custom_keycodes {
  NAVWORD = SAFE_RANGE,
  // Other custom keycodes
};

// `LT_NUMW` momentarily activates _NUMBERS layer on hold,
// and triggers NUMWORD on tap.
// `KC_0` is just a placeholder, it will never be sent.
#define LT_NUMW LT(_NUMROW, KC_0)

uint8_t layerword_layer_from_trigger(uint16_t keycode) {

  switch (keycode) {
    case LT_NUMW: return _NUMBERS;
    case NAVWORD: return _SHORTNAV;
    default: return 0;
  }
}
```

After that, for each layer involving a Layer Word, define the keys that should continue it:

```c
bool should_continue_layerword(uint8_t layer, uint16_t keycode, keyrecord_t *record) {

  switch (layer) {

    case _NUMBERS:
      switch (keycode) {
        // Keycodes that should not disable num word.
        case KC_1 ... KC_0:
        case KC_P1 ... KC_P0:
        case KC_PSLS ... KC_PPLS:
        case KC_PDOT:
        case KC_DOT:
        case KC_MINS:
        case KC_SLSH:
        case KC_EQL:
        // Misc
        case KC_BSPC:
            return true; 
        default:
            return false;
      }

    case _SHORTNAV:
      keycode = QK_MODS_GET_BASIC_KEYCODE(keycode);
      switch (keycode) {
        case KC_LEFT:
        case KC_RIGHT:
        case KC_DOWN:
        case KC_UP:
        case KC_PGUP:
        case KC_PGDN:
        case KC_HOME:
        case KC_END:
            return true;
        default:
            return false;
      }

    // Other Layerwords
  }
  return false;
}
```

Any other symbol from the layer will be processed, but the Layer Word will end afterwards and the layer will be deactivated.

The activation of a Layer Word automatically disables the other ones, to avoid messing up with the layer stack. A timeout can also be defined, to automatically deactivate Layer Words after a period of keyboard inactivity:

```c
uint16_t layerword_exit_timeout(uint8_t layer) {

  switch (layer) {
    case _NUMBERS:
    case _SHORTNAV:
        return 3000;
    default:
        return 0;
  }
}
```
&nbsp;</br>

When a trigger is located on the layer to which it is linked, Layer Word prevents Layer Taps, One-shot Layers, etc. to exit this layer, like [Layer Lock](https://docs.qmk.fm/features/layer_lock#layer-lock). For example, I access my Navigation layer with a Layer Tap key. On this layer, I have my `NAVWORD` key. When I press it, the navigation layer will stay on, even when the LT key is released. The layer will be automatically deactivated when a word-breaking character is tapped. 

My Layer Words settings are located [here](https://github.com/Kawamashi/qmk_userspace/blob/2eda8743e5bc084ae41fbe80c64a765cb540efa8/users/Kawamashi/conf_words.c#L230).

&nbsp;</br>
## Functions
You can manipulate Layer Word with these functions:


| Function                    | Description                                  | 
| --------------------------- | -------------------------------------------- | 
| `get_layerword_layer()`     | Returns the layer of the active Layer Word   | 
| `enable_layerword(layer)`   | Turns on the Layer Word of `layer`           | 
| `disable_layerword(layer)`  | Turns off the Layer Word of `layer`          | 
| `toggle_layerword(keycode)` | Toggles the Layer Word triggered by `keycode`|
