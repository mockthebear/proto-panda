# Protopanda

🇺🇸 English | [🇧🇷 Português](README.pt-br.md)

<p align="center">
  <img src="doc/logoprotopanda.png" alt="Protopanda">
</p>

<p align="center">
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-AGPL--3.0-blue.svg" alt="License: AGPL-3.0"></a>
  <img src="https://img.shields.io/badge/firmware-3.3.5-orange.svg" alt="Firmware version">
  <img src="https://img.shields.io/badge/board-ESP32--S3--N16R8-informational.svg" alt="Board: ESP32-S3-N16R8">
  <a href="https://t.me/mockdiodes"><img src="https://img.shields.io/badge/Telegram-Channel-26A5E4.svg?logo=telegram&logoColor=white" alt="Telegram channel"></a>
  <a href="https://t.me/protopandachat"><img src="https://img.shields.io/badge/Telegram-Chat-26A5E4.svg?logo=telegram&logoColor=white" alt="Telegram chat"></a>
</p>

Protopanda is an open source platform (firmware and hardware) for controlling protogens. The idea is to be simple enough that all you need is a bit of tech savviness to make it work, but at the same time flexible enough that a person with minimal knowledge of Lua can make amazing things.

1. [Features](#features)
2. [3D Models](#3d-models)
3. [Guides](#guides)
4. [FAQ](#faq)
5. [Powering](#powering)
6. [Panels](#panels)
7. [Face and Expressions](#face-and-expressions)
8. [Compiling and flashing firmware](./doc/flashing-guide.md)
9. [LED Strips](#led-strips)
10. [Bluetooth](#bluetooth)
11. [Hardware](#hardware)
12. [DIY](#diy)
13. [Printing and assembling guide](./doc/print-guide.md)
14. [Programming in Lua](#programming-in-lua)

## Features

- Built on the ESP32-S3 N16R8. Easily available and cheap
- 60+ FPS animations
- Support for HUB75 panels, MAX7219 LED matrices or WS2812 matrices
- RGB 16-bit color depth
- Support for WS2812 LED strips
- Customization using Lua
- Facial expressions are just `.PNG` files
- Uses an SD card with easy to configure settings
- BLE support for remote controlling, or IR
- USB-C powered
- Internal screen for menus
- Wi-Fi mode where you can change configurations
- Improved Wi-Fi web editor and compact folder-first directory listing
- Support for keyframe animation with vectorial models
- Built-in FFT and mouth animations based on sound
- Open source and open hardware
- Has games!
- gay 🏳️‍🌈

## 3D Models

All 3D models are on Thingiverse.
**Note that the front frame is on a different page!**

**Head:** https://www.thingiverse.com/thing:7188042

**Front frame:** https://www.thingiverse.com/thing:7188045

[Check the FAQ here](./doc/faq.md)

## Guides

There are several guides with images and all!

* [Printing and assembling guide](./doc/print-guide.md)
* [Building your own protopanda (DIY)](./doc/diy-guide.md)
* [Assembling the front frame with all parts](./doc/front-frame-guide.md)
* [Flashing and compiling the firmware](./doc/flashing-guide.md)
* [Configuring your protogen](./doc/configuring.md)
* [Lua function reference](doc/lua-doc.md)

## FAQ

[Check the FAQ here](./doc/faq.md)

## Powering

__TL;DR: Use a power bank with at least 20W of PD and USB-C.__

There are two modes: one powering 5V directly from USB, and another that has some power management (a buck converter), which needs anywhere from 6.5V up to 12V. This second mode is enabled only via hardware changes on the PCB.

Each HUB75 panel can consume up to 2A at maximum brightness, so powering directly from USB at 5V can be problematic. This is why the version with the regulator triggers PD on the USB, requesting 9V at 3A — plenty of power to light up both panels, though this version consumes considerably more power overall.

Since you usually won't be running them at full brightness or with all LEDs set to white, the 5V version is recommended. But some power banks can't handle the power spike on startup, so choosing a version with PD is also a safe choice.

## Panels

The recommended option is to use HUB75 panels. They're driven by [mrcodetastic's HUB75 lib](https://github.com/mrcodetastic/ESP32-HUB75-MatrixPanel-DMA), and these are the [recommended panels](https://pt.aliexpress.com/item/4000002686894.html).

![HUB75 panels](doc/panels.jpg "HUB75 panels")

They're multiplexed, which means only a few LEDs are on at a given time. It's fast enough that it can't be seen by the eye, but under direct sunlight it's hard to take a good photo without screen tearing.

![Screen tearing caught on camera](doc/tearing.jpg "Screen tearing caught on camera")

The resolution is 64 pixels wide and 32 pixels tall. With two panels side by side, the total area is 128x32px. The color depth is 16 bits, in RGB565 format, meaning red (0-32), green (0-64), and blue (0-32).

You can also use MAX7219 matrices or addressable LED matrices!

To prevent another type of tearing when a frame is being drawn while it's being updated, double buffering is used. This means we draw pixels to a frame, but they won't appear on the screen immediately — instead, we're drawing in memory. When we call `flipPanelBuffer()`, the buffer we drew is sent to the DMA to be constantly drawn on the panel, and the buffer we draw to swaps. This increases memory usage, but it's a price worth paying.

## Face and Expressions

Protopanda uses images from the SD card and a few JSON files to construct the animation sequences. All images must be in `.PNG` format; later, they're decoded to a raw format and stored in the [frame bulk file](#bulk-file).

- [Loading Frames](#loading-frames)
- [Expressions](#expressions)
- [Overlays](#overlays)
- [Expression Stack](#expression-stack)
- [Bulk File](#bulk-file)
- [Managed Mode](#managed-mode)

### Loading Frames

To load frames, add them to the SD card and specify their locations in the `animation.json` file:

```json
{
  "frames": [
    {"pattern": "/expressions/angry/angry%d.png","flip_left": false,"flip_right": true,"from": 5,"to": 9,"name": "frames_angry"},
    {"pattern": "/expressions/angry/angry%d transition.png","flip_left": false,"flip_right": true,"from": 1,"to": 4,"name": "frames_angry_transition"},
    {"pattern": "/expressions/blink/blink%d.png","flip_left": false,"flip_right": true,"from": 1,"to": 8,"name": "frames_blink"},
  ]
}
```

> **Note:** Modifying `animation.json` (adding/removing files) forces the system to rebuild the [frame bulk file](#bulk-file).

Each entry in the `frames` array can be either:
- A file path, **or**
- An object describing multiple files.
  *(Tip: Use [this tool](https://onlinetexttools.com/printf-text) for `printf`-style patterns.)*

#### Frame Object Properties

- **`pattern`** (string)
  Uses `%d` as a placeholder for numbers (like `printf`). Requires `from` and `to` fields.
  **Example:**
  ```json
  {"pattern": "/bolinha/input-onlinegiftools-%d.png", "from": 1, "to": 155}
  ```
  Loads frames from `/bolinha/input-onlinegiftools-1.png` to `...-155.png`.

- **`flip_left`** (boolean)
  Flips the left-side frame horizontally (useful for panel orientation).

- **`flip_right`** (boolean)
  Flips the right-side frame horizontally (useful for panel orientation).

- **`name`** (string)
  Assigns an identifier to a frame or group. The name refers to the first frame in the `pattern`.
  *Why?* Hardcoding frame orders (e.g., `[1, 2, 3]`) becomes problematic if you need to insert a new frame later on. Names act as offsets for flexibility.

- **`color_scheme_left`** (string)
  Flips specific color channels if needed.
  Use any permutation of "rgb", "bgr", "rbg".

### Expressions

After loading frames, [Lua scripts](#programming-in-lua) manage expressions. These are defined in `/expressions.json`:

```json
{
  "frames": [],
  "expressions": [
    {
      "name": "normal",
      "frames": "frames_normal",
      "animation": [1, 2, 1, 2, 1, 2, 3, 4, 3],
      "duration": 250,
      "overlay": "mouth"
    },
    {
      "name": "sus",
      "frames": "frames_amogus",
      "animation": "auto",
      "duration": 200
    },
    {
      "name": "noise",
      "frames": "frames_noise",
      "animation": "loop",
      "duration": 5,
      "onEnter": "ledsStackCurrentBehavior(); ledsSegmentBehavior(0, BEHAVIOR_NOISE); ledsSegmentBehavior(1, BEHAVIOR_NOISE)",
      "onLeave": "ledsPopBehavior()"
    },
    {
      "name": "boop",
      "frames": "frames_boop",
      "animation": [1, 2, 3, 2],
      "duration": 250
    },
    {
      "name": "boop_begin",
      "frames": "frames_boop_transition",
      "animation": [1, 2, 3],
      "duration": 250,
      "transition": true
    },
    {
      "name": "boop_end",
      "frames": "frames_boop_transition",
      "animation": [3, 2, 1],
      "duration": 250,
      "transition": true
    }
  ],
  "scripts": [],
  "boop": {}
}
```

#### Expression Properties

- **`name`** (string, *optional*)
  Identifies the animation (e.g., for menus or scripting).

- **`frames`** (string)
  References a frame group from `animation.json`.

- **`animation`** (int[] or `"auto"`)
  - `int[]`: Explicit frame order (e.g., `[1, 2, 3]`).
  - `"loop"`: Sequential frames (e.g., `1, 2, 3...`).
  - `"pingpong"`: Sequential frames then reversed (e.g., `1, 2, 3... ...3, 2, 1`).
  - `"loop_backwards"`: Sequential frames, backwards (e.g., `...3, 2, 1`).

- **`duration`** (int)
  Frame display time (in milliseconds).

- **`hidden`** (string)
  Hides the expression from menu selection.

- **`intro`** (string)
  The name of another animation, which MUST have `transition=true`. It will play whenever this expression is entered.

- **`outro`** (string)
  The name of another animation, which MUST have `transition=true`. It will play whenever this expression stops running.

- **`transition`** (boolean)
  If `true`, the animation plays once and reverts to the previous state. This forces the animation to stack on top of the previous one instead of replacing it.

- **`repeats`** (int, default 1)
  If the animation is of type `transition`, you can set this to force it to repeat N times.

- **`overlay`** (string)
  Name of the overlay to be used in that animation.

- **`onEnter`** (string, Lua code)
  Executes when the animation starts.

- **`onLeave`** (string, Lua code)
  Executes when the animation ends (either due to `transition=true` or interruption).

### Overlays

Sometimes you want something with a little more swag — like a mouth that moves as you speak, some stars, or even something that reacts to an accelerometer. For that, you can create overlays:

```json
{
"overlays"   : [
    {
      "name"    : "stars",
      "elements": [
        {
          "sprites"           : [
            "/expressions/overlays/star.png"
          ],
          "transparency": true,
          "transparency_color": "#ff00ff",
          "animation"         : {
            "mode": "random_flashing",
            "alive_duration": 50,
            "interval_min": 100,
            "interval_max": 500,
            "min_x": 0,
            "max_x": 64,
            "min_y": 0,
            "max_y": 64
          }
        }
      ]
    }
    {
      "name"    : "mouth",
      "elements": [
        {
          "sprites"           : [
            "/expressions/overlays/mouth0.png",
            "/expressions/overlays/mouth1.png",
            "/expressions/overlays/mouth2.png",
            "/expressions/overlays/mouth3.png",
            "/expressions/overlays/mouth4.png"
          ],
          "transparency": false,
          "transparency_color": "#000000",
          "animation"         : {
            "mode": "fft",
            "x": 11,
            "y": 19,
            "band_start": 2,
            "band_end": 8,
            "attack": 0.05,
            "release": 0.2,
            "min_energy": 50000,
            "max_energy": 200000,
            "frist_frame_threshold": 60000
          }
        }
      ]
    }
  ]
}
```

### Expression Stack

Expressions are stored in a stack. When you add an animation that doesn't repeat, it pauses the current animation and runs until the end of the new animation. If you add two at the same time, the last one will be executed; when it finishes, the previous one resumes.

### Bulk File

Even with the SD card, changing frames is not quite fast — the SD card interface isn't fast enough. To make it faster, images are PNG-decoded into raw pixel data stored in RGB565 format inside the internal flash. All frames are stored in a single file called the `bulk file`. This is done so that frames are stored sequentially, and by keeping the file open, transfer speed is accelerated, achieving 60fps.

Every time you add or modify a frame, this file must be rebuilt. This can be done from the menu or by calling the Lua function `composeBulkFile`.

### Managed Mode

Animations are processed by Core 0, so you don't have to waste any precious time updating them from [Lua scripts](#programming-in-lua). It's possible to change the frame using Lua scripts too, but that's also wasteful — so leave it to the other core, and you only have to worry about selecting which expressions you want!

During managed mode, frame drawing is handled by Core 0.

![Managed mode](doc/managed.png "Managed mode")

## Compiling

Full guide here: [Compiling and flashing firmware](./doc/flashing-guide.md)

## LED Strips

Protopanda supports the WS2812B addressable LED protocol and provides a simple, no-frills system for defining a few behaviors for the strip/matrices.

![LED strip](doc/A7301542.JPG)

![LED strip diagram](doc/ewm.drawio.png)

You can define them inside `hardware.json`:
```json
{
  "leds": { 
    "pin_mode": "double",
    "_comment": "Modes allowed are: 'double' and 'single'. If using extra led strips, they'll all attach at the right led pin if double is set",
    "groups":[
      {
        "_comment": "If pin_side is undefined, it defaults to 'left'",
        "pin_side": "left",
        "led_count": 64,
        "mode": "pride"
      },
      {
        "pin_side": "right",
        "led_count": 64,
        "mode": "pride"
      }
    ]
  }
}
```

### Available Modes and Parameters

| Mode | Description | Parameters |
|------|-------------|------------|
| `none` | LEDs remain off | None |
| `pride` | Rainbow pride flag animation | None |
| `rotate` | Rotating color along the strip | `speed` (ms) - rotation speed |
| `random_color` | Each LED flashes random colors | None |
| `fade_cycle` | Gradual color cycling | `hue` (0-255), `speed` (ms), `min_brightness` (0-255) |
| `rotate_fade_cycle` | Fade cycle with rotation | `hue`, `speed`, `min_brightness`, `rotate_speed` (ms) |
| `color_rgb` | Static RGB color | `r` (0-255), `g` (0-255), `b` (0-255) |
| `color_hsv` | Static HSV color | `h` (0-255), `s` (0-255), `v` (0-255) |
| `random_blink` | LEDs blink randomly | `base_hue` (0-255), `hue_variance` (0-255), `brightness` (0-255), `blink_speed` (ms) |
| `icon_x` | Display an "X" pattern | None |
| `icon_y` | Display a "Y" pattern | None |
| `icon_v` | Display a "V" pattern | None |
| `rotate_sine_v` | Sine wave brightness variation | `hue` (0-255), `saturation` (0-255), `speed` (ms) |
| `rotate_sine_s` | Sine wave saturation variation | `hue` (0-255), `brightness` (0-255), `speed` (ms) |
| `rotate_sine_h` | Sine wave hue variation | `sat` (0-255), `brightness` (0-255), `speed` (ms) |
| `fade_in` | Gradual fade-in effect | `hue` (0-255), `saturation` (0-255), `step` (0-255), `delay` (ms) |
| `noise` | Random noise effect | `step` (0-255), `delay` (ms) |

## Bluetooth

Since version 2.0, Protopanda supports almost any kind of BLE device that has HID. All you need to do is adapt an existing driver if needed, or write a new one. Currently, the devices supported are:
* https://github.com/mockthebear/ble-fursuit-paw
* https://pt.aliexpress.com/item/1005008459884910.html
* https://pt.aliexpress.com/item/1005009845485445.html

A BLE joystick works best.

### Keybind

Currently, the default keybinds are:
```json
{
  "keybinds":{
    "joystick.right_hat=5": "BUTTON_LEFT",
    "joystick.right_hat=3": "BUTTON_DOWN",
    "joystick.right_hat=1": "BUTTON_RIGHT",
    "joystick.right_hat=7": "BUTTON_UP",
    "joystick.buttons.4": "BUTTON_CONFIRM",
    "joystick.buttons.1": "BUTTON_BACK",

    "beauty.buttons.4": "BUTTON_LEFT",
    "beauty.buttons.1": "BUTTON_DOWN",
    "beauty.buttons.3": "BUTTON_RIGHT",
    "beauty.buttons.2": "BUTTON_UP",
    "beauty.buttons.5": "BUTTON_CONFIRM",
    "beauty.buttons.6": "BUTTON_BACK"

  }
}
```
They all map by default to the BLE fursuit paw.

## Hardware

Protopanda is designed to run on the ESP32-S3-N16R8, which is a version with 16MB flash, 384KB ROM, 512KB RAM, and 8MB octal PSRAM.

This specific version is required because its extra space and PSRAM provide enough RAM to run the panels, BLE, and [Lua](#programming-in-lua) together.

On the hardware, there's a port for the HUB75 data, an SD card connector, two screw terminals for the 5V out, the power in pins, one I2C port, and the LED strip pin.

### Diagram

![Diagram](doc/noitegrama.png "Diagram")

### Ports

![Ports](doc/ports.png "Ports")

### Schematic

![Schematic](doc/schematic.png "Schematic")

### Two Cores

Protopanda uses (and abuses) both cores of the ESP32.

* **Core 0**
  By default, Core 0 is primarily designed to manage Bluetooth. When not doing so, it manages the animations, and when [Managed mode](#managed-mode) is active, it also handles the LED screen updates.

* **Core 1**
  The second core handles non-screen-related tasks. It runs the routine that checks the [power level](#powering), updates inputs, reads sensors, and calls the Lua function `onLoop`.

### DIY

We know not everyone can build a PCB from scratch, so there's a way to build your own reduced version of Protopanda.

Check out the [guide for making your own Protopanda!](./doc/diy-guide.md)

![Electronics schematic](doc/diy-schematic.png "Electronics schematic")

#### Remote Controller

To control it, you can:
* Use a Protopanda controller built with an NRF52832.
* Use an IR controller and write a driver for it.
* Write your own solution using the two extra GPIOs left.
* Buy a BLE HID device that's compatible.

##### Compatible Devices

[**Mocute controller**](https://pt.aliexpress.com/item/32962516758.html). The best alternative, input-wise. It works as a mouse, keyboard, or joystick. Use it in mouse mode or keyboard mode.

[**Lazy TikTok scrolling keypad**](https://pt.aliexpress.com/item/1005008459884910.html). High latency. It's a dumb device that simulates mouse and keyboard movements to scroll TikTok on your phone. Luckily, those movements have been mapped to Protopanda input. Best size/form factor.

[**VRPARK**](https://pt.aliexpress.com/item/32792719676.html). The cheapest and crappiest of them all. Easiest to find.

If you find a generic BLE HID device, like a mouse or keyboard, it should be compatible in some way.

## Printing and Assembling Guide

[Guide here](./doc/print-guide.md)

## Programming in Lua

__[Lua functions reference](doc/lua-doc.md)__

- [Minimum Lua Script](#minimum-lua-script)
- [Cycle Expressions Each Second](#cycle-expressions-each-second)

### Minimum Lua Script
```lua
--Minimum lua script on init.lua

function onSetup()
  --Function is called once, here you may start the BLE, begin scanning, configure panel, set power mode, load lib and prepare led strips and even power on
  --All calls here are called from SETUP, running on core 0
end

function onPreflight()
  --Upon here, the all lua calls are called from core 1. You can even leave this function in blank.
  --Core 0 will only start managing after 100ms (the final beep)
end

function onLoop(dt)
  --This function will be called in loop. 
  --The dt parameter is the difference in MS from the begin of the last frame and current one. Useful for storing elapsed time
end
```
### Cycle Expressions Each Second
```lua
local expressions = dofile("/lualib/expressions.lua")
local changeExpressionTimer = 1000 --1 second

function onSetup()
  setPanelMaxBrightness(64)
  panelPowerOn() --Brightness always start at 0
  gentlySetPanelBrightness(64)
end

function onPreflight()
  setPanelManaged(true)
  expressions.Next()
end

function onLoop(dt)
  changeExpressionTimer = changeExpressionTimer - dt 
  if changeExpressionTimer <= 0 then 
    changeExpressionTimer = 1000 --1 second
    expressions.Next()
  end
end
```
