## Layer Word

**Layer Word** is a concept similar to Caps Word, but applied to layers. When a Layer Word is enabled, the corresponding layer is activated, and stays active until the user types a word-breaking character. For example, the Layer Word bound to my number layer remains active as long as I type numeric symbols. The characters that allow each Layer Word to continue are defined by the user.

Only one Layer Word can be active at a time to avoid messing up with the layer stack. Layer Words can automatically deactivate after a period of keyboard inactivity.

Setting up a Layer Word is very simple. First, in keymap.c, you need to define a custom keycode, for example, `NAVWORD`. Layer Word also works with layer-tap keys, allowing you to momentarily activate a layer on hold and triggering a layer word on tap. Then, you link these keycodes to layers. Layer Word's timeout can also be defined:


[Layer Word](keyboards/splitkb/kyria/rev1/base/keymaps/Kawamashi/features/layerword.c) est un concept similaire à Caps Word, mais qui s’applique aux couches. Quand on active un Layer Word, on active la couche correspondante, et elle restera activée tant que l’utilisateur ne tape pas de caractère “word breaking”. Par exemple, le Layer Word lié à ma couche de nombres restera activé tant que je taperai des symboles numériques. Les caractères permettant de continuer chaque Layer Word sont à définir par l’utilisateur.


Il ne peut y avoir qu’un seul Layer Word actif à la fois. Les Layer Word peuvent se désactiver automatiquement au bout d’un certain temps d’inactivité du clavier.


La mise en place d’un Layer Word est très simple. Il faut d’abord définir un keycode custom, `NUMWORD` par exemple. Layer Word fonctionne également avec des layer-tap, pour activer momentanément une couche . Dans ce cas, le comportement hold n’est pas modifié
On lie ensuite ce keycode à une couche et on peut définir le timeout du Layerword : 

```c
// custom keycodes for NAVWORD and FUNWORD
enum custom_keycodes {
  NAVWORD = SAFE_RANGE,
  FUNWORD,
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
    case FUNWORD: return _FUNCAPPS;
    default: return 0;
  }
}

uint16_t layerword_exit_timeout(uint8_t layer) {

  switch (layer) {
    case _NUMBERS:
    case _SHORTNAV:
        return 3000;
    case _FUNCAPPS:
        return 30000;
    default:
        return 0;
  }
}
```

Il ne reste plus qu’à définir les caractères qui continuent le Layer Word :

```c
bool should_continue_layerword(uint8_t layer, uint16_t keycode, keyrecord_t *record) {

  switch (layer) {

    case _NUMBERS:
      switch (keycode) {
        // Keycodes that should not disable num word.
        // Numpad keycodes
        case KC_1 ... KC_0:
        case KC_PDOT:
        case PG_MOIN:
        case PG_ASTX: 
        case PG_PLUS:
        case PG_SLSH:
        case PG_EGAL:
        case PG_EXP:
        case PG_IND:
        case PG_H:
        case PG_2PTS:

        // Misc
        case KC_BSPC:
        case PG_1DK:   // Not to exit Numword when chording it with 1DK
            return true; 
        default:
            return false;
      }

    case _SHORTNAV:
      switch (keycode) {
        case SEL_WORD:
        case SEL_LINE:
          return true;
      }
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

    case _FUNCAPPS:
      switch (keycode) {
        case KC_F8:
            return true;
        default:
            disable_layerword(_FUNCAPPS);
            return false;
      }

    // Other Layerwords
  }
  return false;
}
```

Tout autre symbole de la couche sera saisi, mais le Layer Word se terminera et la couche sera désactivée quand la touche sera relâchée.  
Le paramétrage de mes Layer Words se trouve [ici](https://github.com/Kawamashi/qmk_userspace/blob/main/keyboards/splitkb/kyria/rev1/base/keymaps/Kawamashi/word_conf.c).

&nbsp;</br>
