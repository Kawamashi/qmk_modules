# @Kawamashi’s QMK community modules

This repo contains my [QMK](docs.qmk.fm) community modules, which I use myself in [my keymap](https://github.com/Kawamashi/qmk_userspace/tree/main/users/Kawamashi).

Each child directory is a separate module, and has instructions on how to add it to your build.

| Module                                                                                            | Status                        |
| ------------------------------------------------------------------------------------------------- | ----------------------------- | 
| [Flow Tap](https://github.com/Kawamashi/qmk_modules/tree/main/flow_tap)                           | 2026-05-30 – Personnal module | 
| [Layer Word](https://github.com/Kawamashi/qmk_modules/tree/main/layer_word)                       | 2026-05-30 – Published        | 
| [One-Shots on Steroids](https://github.com/Kawamashi/qmk_modules/tree/main/oneshots_on_steroids)  | 2026-05-30 – WIP              | 
| [Speculative Hold](https://github.com/Kawamashi/qmk_modules/tree/main/speculative_hold)           | 2026-05-30 – Personnal module |

I'm no developer, my code is probably naive! 😅

&nbsp;</br>

## Installation
QMK must be version 0.28.0 (released on 2025-02-27) or above. There must be a `modules` folder inside your `qmk_firmware` folder. 

### 1) Download modules
On a terminal (or `QMK MSYS`), run these shell commands to download the modules, replacing /path/to/qmk_firmware with the path of your "`qmk_firmware`" folder, or with the path of your [external userspace](https://docs.qmk.fm/newbs_external_userspace) folder if you use one.

```shell
cd /path/to/qmk_firmware
mkdir -p modules
git submodule add https://github.com/Kawamashi/qmk-modules.git modules/Kawamashi
git submodule update --init --recursive
```

If you don't want to use git, [download a .zip of this repo](https://github.com/Kawamashi/qmk_modules/archive/refs/heads/main.zip) into the modules folder. Unzip it, then rename the resulting `qmk_modules-main` folder to `Kawamashi`.

In any case, the installed directory structure is like this:
```
<QMK_FIRMWARE or QMK_USERSPACE>
└── modules
    └── Kawamashi
        ├── clever_keys
        ├── layer_word
        ├── oneshots_on_steroids
        └── ...
```

### 2) Add modules to your keymap
Add a module to your keymap by writing a file `keymap.json` in your keymap folder with the content:
```json
{
  "modules": ["Kawamashi/layer_word"]
}
```
If there is already a  `keymap.json` file, merge the `"modules"` line into it. Add multiple modules like:
```json
{
  "modules": ["Kawamashi/layer_word", "Kawamashi/oneshots_on_steroids"]
}
```

### 3) Compile a new firmware
Compile and flash the firmware as usual. If there are build errors, try running `qmk clean` and compiling again for a clean build.
