# Layer Word

**Layer Word** works similarly to Caps Word, but for layers. When activated, it keeps a layer active until a word-breaking key is pressed. For example, the Layer Word configured for my Numbers layer remains active as long as I type numbers and mathematical operators. The keycodes that keep each Layer Word active are fully customizable.

&nbsp;</br>
## Setup

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
#define LT_NUMW LT(_NUMBERS, KC_0)

uint8_t get_layerword_layer_from_trigger(uint16_t keycode) {

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

Activating a Layer Word automatically deactivates any previously active Layer Word, to avoid messing up with the layer stack. 

&nbsp;</br>
### Layer Lock-like behavior

Like [Layer Lock](https://docs.qmk.fm/features/layer_lock), Layer Word can keep a momentarily activated layer active after its activation key is released.

For example, suppose you access your Navigation layer with a layer-tap key, and have a `NAVWORD` key on that layer. While holding the layer-tap key, pressing `NAVWORD` activates Layer Word on the Navigation layer. You can then release the layer-tap key, and the layer will remain active until a word-breaking key is pressed.

This also works with Momentary (MO), Layer Tap (LT), Layer Tap Toggle (TT), Layer Mod (LM), One Shot Layer (OSL) keys and [One Shot Layer on Steroids](https://github.com/Kawamashi/qmk_modules/tree/main/oneshots_on_steroids) keys.

&nbsp;</br>
### Idle timeout

Layer Word is automatically deactivated after a period of keyboard inactivity. The default is 5000 (5 seconds). Configure the timeout duration in `config.h`, for instance:
```c
#define LAYERWORD_TIMEOUT 3000    // 3 seconds
```
For more granular control, you can add the following callback to your `keymap.c` to customize the timeout for each Layer Word:
```c
uint16_t get_layerword_timeout(uint8_t layer) {

  switch (layer) {
    case _FUNCTIONS:
        return 30000;
    case _NUMBERS:
        return LAYERWORD_TIMEOUT - 1000;
    default:
        return LAYERWORD_TIMEOUT;
  }
}
```
Setting `LAYERWORD_TIMEOUT` to 0 configures Layer Word to never time out. Layer Word then remains active indefinitely until a word-breaking key is pressed.


You can find my Layer Word configuration [here](https://github.com/Kawamashi/qmk_krypton_keymaps/blob/c67da7fb580399b95aec1556312d4c30397f01df/users/Kawamashi/customization.c#L565).

&nbsp;</br>
## Functions
You can manipulate Layer Word with these functions:


| Function                    | Description                                  | 
| --------------------------- | -------------------------------------------- | 
| `get_layerword_layer()`     | Returns the layer of the active Layer Word   | 
| `enable_layerword(layer)`   | Turns on the Layer Word of `layer`           | 
| `disable_layerword()`       | Turns off Layer Word                         | 
| `toggle_layerword(keycode)` | Toggles the Layer Word triggered by `keycode`|
