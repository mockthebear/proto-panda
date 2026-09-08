# Topics

1. [Lua Functions](#lua-functions)
2. [Lua Constants](#lua-constants)
3. [LED Behaviors](#led-behaviors)
4. [Bluetooth Interface](#bluetooth-interface)

# Lua functions

- [Power](#power)
- [System](#system)
- [Sensors](#sensors)
- [Led panel Drawing](#led-panel-drawing)
- [Dictionary Functions](#dictionary-functions)
- [Servo Control](#servo-control)
- [Image Decoding](#image-decoding)
- [Internal Screen](#internal-screen)
- [LED Strips](#led-strips-1)
- [FFT / Audio Analysis](#fft--audio-analysis)
- [Arduino Core](#arduino-core)
- [IR Remote](#ir-remote)
- [2D Models](#2d-models)
- [Sprites](#sprites)
- [Keyframe Animations](#keyframe-animations)

## Power

[↑ Back to top](#topics)

#### `panelPowerOn()`
Turns on the regulator, powering the panel and the 5V output.
> **⚠️ Requires `PIN_ENABLE_REGULATOR`**: If this pin is not defined in the hardware config, this function does nothing.
- **Returns**: `nil`

#### `panelPowerOff()`
Turns off the regulator.
> **⚠️ Requires `PIN_ENABLE_REGULATOR`**: If this pin is not defined in the hardware config, this function does nothing.
- **Returns**: `nil`

#### `getBatteryVoltage()`
Returns the battery/USB voltage in volts.
- **Returns**: `float`

#### `getAvgBatteryVoltage()`
Returns the avg battery/USB voltage in volts.
- **Returns**: `float`

#### `setAutoCheckPowerLevel(bool)`
Sets automatic power check. If the power goes under the `setVoltageStopThreshold` and the hardware is configured using a buck converter, the voltage is cut off.
- **Returns**: `nil`

#### `setVoltageStopThreshold(voltage)`
Sets the voltage threshold at which the system will automatically shut down to prevent damage from low battery voltage.
- **Parameters**:
  - `voltage` (int): The voltage threshold in volts.
- **Returns**: `nil`

#### `setVoltageStartThreshold(voltage)`
Sets the voltage threshold at which the system will automatically power on after being shut down due to low battery voltage.
- **Parameters**:
  - `voltage` (int): The voltage threshold in volts.
- **Returns**: `nil`

#### `setPoweringMode(mode)`
Sets the powering mode of the system. The mode can be `POWER_MODE_5V_PD`, `POWER_MODE_USB_9V`, or `POWER_MODE_BATTERY`.
- **Parameters**:
  - `mode` (int): The powering mode to set.
- **Returns**: `nil`

## System

[↑ Back to top](#topics)

#### `log(msg)`
Logs a message to the system logger (appears in serial output and log file).
- **Parameters**:
  - `msg` (string): The message to log.
- **Returns**: `nil`

#### `setHaltOnError(halt)`
Controls whether the system halts (freezes with error display) when a Lua error occurs. Useful for debugging; disable in production to recover gracefully.
- **Parameters**:
  - `halt` (bool): `true` to halt on error (default), `false` to continue.
- **Returns**: `nil`

### `getFreePsram()`
Returns the amount of free PSRAM (Pseudo Static RAM) available in bytes.
- **Returns**: `int` (The amount of free PSRAM in bytes).

### `getFreeHeap()`
Returns the amount of free heap memory available in bytes.
- **Returns**: `int` (The amount of free heap memory in bytes).

#### `getLuaFps()`
Returns the current frames-per-second as seen from the Lua loop (Core 1).
- **Returns**: `float`

#### `getFps()`
Returns the auto-measured FPS of the system.
- **Returns**: `float`

#### `restart()`
Restarts the ESP32 microcontroller.
- **Returns**: `nil`

#### `getResetReason()`
Returns the reason for the last reset.
- **Returns**: `int` (One of the ESP_RST_* constants)

#### `setBrownoutDetection(bool enable)`
Enables or disables brownout detection.
- **Parameters**:
  - `enable` (bool): `true` to enable, `false` to disable
- **Returns**: `nil`

#### `listFiles(path, recursive)`
Lists files in directory on the SD card.
- **Parameters**:
  - `path` (string): Directory path
  - `recursive` (bool): Whether to list recursively
- **Returns**: `string array`

#### `listFilesInFolder(path)`
Lists files in a folder using the internal storage helper.
- **Parameters**:
  - `path` (string): Directory path
- **Returns**: `string array`

#### `moveFile(src, dest)`
Moves/renames a file on the SD card.
- **Parameters**:
  - `src` (string): Source path
  - `dest` (string): Destination path
- **Returns**: `bool` (Success)

#### `removeFile(path)`
Deletes a file from the SD card.
- **Parameters**:
  - `path` (string): File path
- **Returns**: `bool` (Success)

#### `createDir(path)`
Creates a directory on the SD card.
- **Parameters**:
  - `path` (string): Directory path to create
- **Returns**: `bool` (Success)

#### `fileExists(path)`
Checks whether a file or directory exists on the SD card.
- **Parameters**:
  - `path` (string): File path
- **Returns**: `bool`

#### `formatFFAT(full)`
Formats the internal FFat (flash) filesystem.
- **Parameters**:
  - `full` (bool): If `true`, performs a full format.
- **Returns**: `bool` (Success)

#### `deleteBulkFile()`
Deletes the `frames.bulk` file, forcing a rebuild on next boot.
- **Returns**: `nil`

#### `composeBulkFile()`
Manually triggers a rebuild of the `frames.bulk` file from the PNG frames on the SD card.
- **Returns**: `nil`

#### `dumpStackToSerial()`
Debug utility. Dumps the current Lua stack contents to the serial port.
- **Returns**: `nil`

## Sensors

[↑ Back to top](#topics)

#### `readButtonStatus(int)`
Reads the status of the remote control button. There are a total of 5 buttons. Possible button states:
```
BUTTON_RELEASED = 0
BUTTON_JUST_PRESSED = 1
BUTTON_PRESSED = 2
BUTTON_JUST_RELEASED = 3
```
- **Parameters**:
  - `button` (int): The button ID to check.
- **Returns**: `int` (The button state).

#### `readAccelerometerX([device])`
Returns the raw value of the remote control's accelerometer sensor, X axis only.
- **Parameters**:
  - `device` (int, optional): The ID of the remote control device. Default is `0`.
- **Returns**: `float`

#### `readAccelerometerY([device])`
Returns the raw value of the remote control's accelerometer sensor, Y axis only.
- **Parameters**:
  - `device` (int, optional): The ID of the remote control device. Default is `0`.
- **Returns**: `float`

#### `readAccelerometerZ([device])`
Returns the raw value of the remote control's accelerometer sensor, Z axis only.
- **Parameters**:
  - `device` (int, optional): The ID of the remote control device. Default is `0`.
- **Returns**: `float`

#### `readGyroX([device])`
Returns the raw value of the remote control's gyroscope sensor, X axis only.
- **Parameters**:
  - `device` (int, optional): The ID of the remote control device. Default is `0`.
- **Returns**: `int`

#### `readGyroY([device])`
Returns the raw value of the remote control's gyroscope sensor, Y axis only.
- **Parameters**:
  - `device` (int, optional): The ID of the remote control device. Default is `0`.
- **Returns**: `int`

#### `readGyroZ([device])`
Returns the raw value of the remote control's gyroscope sensor, Z axis only.
- **Parameters**:
  - `device` (int, optional): The ID of the remote control device. Default is `0`.
- **Returns**: `int`

#### `hasLidar()`
Checks for the presence of a lidar sensor.
- **Returns**: `bool` (`true` if a lidar is present, otherwise `false`)

#### `readLidar()`
Returns the distance in mm from the lidar.
- **Returns**: `int` (The distance in millimeters).

#### `getInternalButtonStatus()`
Returns the status of the internal button.
- **Returns**: `int` (1 is pressed and 0 is released).

#### `i2cScan()`
Scans the I2C bus and returns addresses of detected devices.
- **Returns**: `int array` (List of detected I2C addresses)

## Led panel Drawing

[↑ Back to top](#topics)

#### `flipPanelBuffer()`
The panels have two buffers. One for drawing and the other for rendering. When the drawing is finished in one buffer, this call flips them so you can draw into the other.
- **Returns**: `nil`

#### `drawPanelPixel(x, y, color)`
Draws a pixel at the specified coordinates with the given color.
- **Parameters**:
  - `x` (int): The X coordinate.
  - `y` (int): The Y coordinate.
  - `color` (int): The color to draw (use `color565` or `color444` to generate the color).
- **Returns**: `nil`

#### `drawPanelPixels(x, y, pixels)`
Draws multiple pixels at once starting at the given coordinates.
- **Parameters**:
  - `x` (int): Starting X coordinate.
  - `y` (int): Starting Y coordinate.
  - `pixels` (int array): Array of color values in `color565` format.
- **Returns**: `nil`

#### `drawPanelCircle(x, y, radius, color)`
Draws a circle outline at the specified coordinates.
- **Parameters**:
  - `x` (int): The X coordinate of the center.
  - `y` (int): The Y coordinate of the center.
  - `radius` (int): The radius of the circle.
  - `color` (int): The color to draw.
- **Returns**: `nil`

#### `drawPanelChar(x, y, c, color, [bg, [size]])`
Draws a character on the panel at the specified coordinates.
- **Parameters**:
  - `x` (int): The X coordinate.
  - `y` (int): The Y coordinate.
  - `c` (char): The character to draw.
  - `color` (int): The color of the character.
  - `bg` (int, optional): The background color. Default is `0`.
  - `size` (int, optional): The size multiplier. Default is `1`.
- **Returns**: `nil`

#### `drawPanelFillCircle(x, y, radius, color)`
Draws a filled circle at the specified coordinates.
- **Parameters**:
  - `x` (int): The X coordinate of the center.
  - `y` (int): The Y coordinate of the center.
  - `radius` (int): The radius of the circle.
  - `color` (int): The fill color.
- **Returns**: `nil`

#### `drawPanelLine(x0, y0, x1, y1, color)`
Draws a line between two points.
- **Parameters**:
  - `x0` (int): Starting X coordinate.
  - `y0` (int): Starting Y coordinate.
  - `x1` (int): Ending X coordinate.
  - `y1` (int): Ending Y coordinate.
  - `color` (int): The color to draw.
- **Returns**: `nil`

#### `drawPanelRect(x, y, width, height, color)`
Draws a rectangle outline.
- **Parameters**:
  - `x` (int): The X coordinate of the top-left corner.
  - `y` (int): The Y coordinate of the top-left corner.
  - `width` (int): The width of the rectangle.
  - `height` (int): The height of the rectangle.
  - `color` (int): The color to draw.
- **Returns**: `nil`

#### `drawPanelFillRect(x, y, width, height, color)`
Draws a filled rectangle.
- **Parameters**:
  - `x` (int): The X coordinate of the top-left corner.
  - `y` (int): The Y coordinate of the top-left corner.
  - `width` (int): The width of the rectangle.
  - `height` (int): The height of the rectangle.
  - `color` (int): The fill color.
- **Returns**: `nil`

#### `drawPanelFillTriangle(x0, y0, x1, y1, x2, y2, color)`
Draws a filled triangle defined by three vertices.
- **Parameters**:
  - `x0, y0` (int): First vertex.
  - `x1, y1` (int): Second vertex.
  - `x2, y2` (int): Third vertex.
  - `color` (int): The fill color.
- **Returns**: `nil`

#### `color444(r, g, b)`
Converts RGB888 values to RGB444 format.
- **Parameters**:
  - `r` (int): Red (0-255).
  - `g` (int): Green (0-255).
  - `b` (int): Blue (0-255).
- **Returns**: `int` (RGB444 color value)

#### `color565(r, g, b)`
Converts RGB888 values to RGB565 format.
- **Parameters**:
  - `r` (int): Red (0-255).
  - `g` (int): Green (0-255).
  - `b` (int): Blue (0-255).
- **Returns**: `int` (RGB565 color value)

#### `clearPanelBuffer()`
Clears the screen buffer.
- **Returns**: `nil`

#### `drawPanelFace(faceId)`
Draws a specific face (frame). This face must already be loaded beforehand.
- **Parameters**:
  - `faceId` (int): The ID of the face to draw.
- **Returns**: `nil`

#### `setPanelAnimation(frames, duration, [repeat, [drop, [storage]]])`
If the panel is in managed mode, sets an animation to run on it.
- **Parameters**:
  - `frames` (int array): The IDs of each frame in the animation.
  - `duration` (int): The duration of each frame in milliseconds.
  - `repeat` (int, optional): Number of times the animation repeats. Default is `-1` (infinite).
  - `drop` (bool, optional): If `true`, all stacked animations are erased and this becomes the only one.
  - `storage` (int, optional): Storage ID queryable via `getCurrentAnimationStorage()`.
- **Returns**: `nil`

#### `setPanelModelAnimation(modelAnimId, [repeat, [drop]])`
If the panel is in managed mode, sets a 3D model animation to run on it.
- **Parameters**:
  - `modelAnimId` (int): The ID of the model animation to play.
  - `repeat` (int, optional): Number of repeats. Default is `-1` (infinite).
  - `drop` (bool, optional): If `true`, clears the animation stack first.
- **Returns**: `nil`

#### `setInterruptFrames(frames, frameDuration)`
Sets frames that play as an interrupt animation (e.g. triggered by a sensor).
- **Parameters**:
  - `frames` (int array): Frame IDs to display.
  - `frameDuration` (int): Duration of each frame in milliseconds.
- **Returns**: `nil`

#### `setInterruptAnimationPin(pin)`
Configures the GPIO pin that triggers the interrupt animation.
- **Parameters**:
  - `pin` (int): The GPIO pin number.
- **Returns**: `nil`

#### `setAnimationShader(shader, [intensity])`
Sets a shader to apply over the panel animation.
- **Parameters**:
  - `shader` (int): Shader type (e.g. `SHADER_RAINBOW`, `SHADER_FIRE`).
  - `intensity` (float, optional): Shader intensity. Default is `1.0`.
- **Returns**: `nil`

#### `setPanelColorMode(mode)`
Sets the color channel order for the panel (useful if your panel has a non-standard wiring).
- **Parameters**:
  - `mode` (int): One of `COLOR_MODE_RGB`, `COLOR_MODE_RBG`, `COLOR_MODE_GRB`, `COLOR_MODE_GBR`, `COLOR_MODE_BRG`, `COLOR_MODE_BGR`.
- **Returns**: `nil`

#### `loadFrameAsTexture(frameId)`
Loads a panel frame into the texture slot for use by the 3D model renderer.
- **Parameters**:
  - `frameId` (int): The frame ID to load as texture.
- **Returns**: `nil`

### `getCurrentAnimationStorage()`
Returns the ID of the current animation storage being used.
- **Returns**: `int`

#### `setPanelManaged(managed)`
Enables or disables managed mode. In managed mode, rendering is handled asynchronously on Core 0.
- **Parameters**:
  - `managed` (bool): `true` to enable, `false` to disable.
- **Returns**: `nil`

#### `isPanelManaged()`
Returns whether the panel is in managed mode.
- **Returns**: `bool`

#### `getPanelCurrentFace()`
Returns the current frame ID being displayed.
- **Returns**: `int`

#### `drawPanelCurrentFrame()`
Draws the current frame immediately.
- **Returns**: `nil`

#### `getAnimationStackSize()`
Returns the number of animations currently stacked.
- **Returns**: `int`

#### `popPanelAnimation()`
Removes the current animation from the stack.
- **Returns**: `nil`

#### `setPanelBrightness(brightness)`
Sets the brightness of the panel immediately.
- **Parameters**:
  - `brightness` (int): Brightness level (0-255, where 255 is 100%).
- **Returns**: `nil`

#### `getPanelBrightness()`
Returns the current panel brightness level.
- **Returns**: `int` (0-255)

#### `gentlySetPanelBrightness(brightness, [rate])`
Gradually adjusts the panel brightness to the specified level.
- **Parameters**:
  - `brightness` (int): Target brightness level (0-255).
  - `rate` (int, optional): Speed of the transition. Default is `4`.
- **Returns**: `nil`

#### `setRainbowShader(enabled)`
Enables or disables the rainbow shader, which converts pixels to a rainbow pattern.
- **Parameters**:
  - `enabled` (bool): `true` to enable the shader, `false` to disable.
- **Returns**: `nil`

#### `getFrameOffsetByName(name)`
Returns the frame offset associated with the given name.
- **Parameters**:
  - `name` (string): The frame name.
- **Returns**: `int`

#### `getFrameCountByName(name)`
Returns the number of frames in a given frame group.
- **Parameters**:
  - `name` (string): The frame name.
- **Returns**: `int`

#### `clearAllOverlaySprites()`
Removes all sprites currently registered as animation overlays.
- **Returns**: `nil`

#### `setOverlaySprite(sprite)`
Registers a `Sprite` object to be drawn as an overlay on top of the current panel animation.
- **Parameters**:
  - `sprite` (Sprite): The sprite object (from `Sprite()`) to overlay.
- **Returns**: `bool`

#### `clearOverlaySprite(sprite)`
Removes a specific sprite overlay.
- **Returns**: `bool`

#### `forceRedrawEachFrame(enabled)`
Forces the panel to redraw every frame instead of only when content changes (useful when overlay sprites or shaders animate independently of the base animation).
- **Parameters**:
  - `enabled` (bool): `true` to force redraw every frame.
- **Returns**: `nil`

#### `setFFTOverlay(enable)`
Links the FFT audio analysis output to an animation overlay (e.g. to drive a spectrum visualization on the panel).
- **Returns**: `nil`

## Image Decoding

[↑ Back to top](#topics)

### `decodePng(filename)`
Decodes a PNG image file from the SD card and returns the raw pixel data as an RGB565 table.
- **Parameters**:
  - `filename` (string): The path to the PNG file on the SD card.
- **Returns**: `uint16 table`

## Dictionary Functions

[↑ Back to top](#topics)

#### `dictGet(key)`
Gets a value from persistent dictionary storage.
- **Parameters**:
  - `key` (string): The key to lookup.
- **Returns**: `string`

#### `dictSet(key, value)`
Sets a value in persistent dictionary storage.
- **Parameters**:
  - `key` (string): The key to store.
  - `value` (string): The value to store.
- **Returns**: `nil`

#### `dictDet(key)`
Deletes a key from persistent dictionary storage.
> **Note**: The registered function name is `dictDet` (not `dictDel`).
- **Parameters**:
  - `key` (string): The key to delete.
- **Returns**: `nil`

#### `dictSave()`
Saves the dictionary to persistent storage.
- **Returns**: `nil`

#### `dictLoad()`
Loads the dictionary from persistent storage.
- **Returns**: `nil`

#### `dictFormat()`
Formats/clears the dictionary storage.
- **Returns**: `nil`

## Servo Control

[↑ Back to top](#topics)

> **⚠️ Requires `USE_SERVO`**: `servoPause()`, `servoResume()`, and `servoMove()` are only registered when the firmware is compiled with servo support. `hasServo()` is always available.

#### `servoPause(servoId)`
Pauses the specified servo.
- **Parameters**:
  - `servoId` (int): The ID of the servo to pause.
- **Returns**: `bool`

#### `servoResume(servoId)`
Resumes the specified servo.
- **Parameters**:
  - `servoId` (int): The ID of the servo to resume.
- **Returns**: `bool`

#### `servoMove(servoId, angle)`
Moves the specified servo to the given angle.
- **Parameters**:
  - `servoId` (int): The ID of the servo to move.
  - `angle` (float): The target angle for the servo.
- **Returns**: `bool`

#### `hasServo()`
Checks if the system has a servo connected.
- **Returns**: `bool`

## Internal Screen

[↑ Back to top](#topics)

### `oledDrawPixel(x, y, color)`
Draws a single pixel on the OLED screen.
- **Parameters**:
  - `x` (int): The X coordinate.
  - `y` (int): The Y coordinate.
  - `color` (int): 1 for white, 0 for black.
- **Returns**: `nil`

### `oledDrawBottomBar()`
Draws the bottom bar.
- **Returns**: `nil`

### `oledSetTextColor(fg[, bg])`
Sets the text color for drawing on the OLED screen.
- **Parameters**:
  - `fg` (int): Foreground color (1 for white, 0 for black).
  - `bg` (int, optional): Background color. Default is `1`.
- **Returns**: `nil`

### `oledSetFontSize(size)`
Sets the text size.
- **Parameters**:
  - `size` (int): Text size multiplier. Default is `1`.
- **Returns**: `nil`

#### `oledSetCursor(x, y)`
Sets the cursor position on the OLED screen.
- **Parameters**:
  - `x` (int): The X coordinate.
  - `y` (int): The Y coordinate.
- **Returns**: `nil`

#### `oledFaceToScreen(x, y)`
Draws the current panel image to the internal OLED screen at the specified position.
- **Parameters**:
  - `x` (int): The X coordinate.
  - `y` (int): The Y coordinate.
- **Returns**: `nil`

#### `oledDrawTopBar()`
Draws the top bar with a few utilities.
- **Returns**: `nil`

#### `oledClearScreen()`
Clears the OLED screen buffer.
- **Returns**: `nil`

#### `oledDisplay()`
Sends the buffer to the OLED screen.
- **Returns**: `nil`

#### `oledDrawText(msg)`
Draws text at the current cursor position.
- **Parameters**:
  - `msg` (string): The text to draw.
- **Returns**: `nil`

#### `oledDrawRect(x, y, width, height, color)`
Draws a rectangle outline on the OLED screen.
- **Parameters**:
  - `x` (int): X coordinate.
  - `y` (int): Y coordinate.
  - `width` (int): Width of the rectangle.
  - `height` (int): Height of the rectangle.
  - `color` (int): 1 for white, 0 for black.
- **Returns**: `nil`

#### `oledDrawFilledRect(x, y, width, height, color)`
Draws a filled rectangle on the OLED screen.
- **Parameters**:
  - `x` (int): X coordinate.
  - `y` (int): Y coordinate.
  - `width` (int): Width of the rectangle.
  - `height` (int): Height of the rectangle.
  - `color` (int): 1 for white, 0 for black.
- **Returns**: `nil`

#### `oledDrawLine(x, y, x2, y2, color)`
Draws a line on the OLED screen.
- **Parameters**:
  - `x, y` (int): Starting point.
  - `x2, y2` (int): Ending point.
  - `color` (int): 1 for white, 0 for black.
- **Returns**: `nil`

#### `oledDrawFastHLine(x, y, w, color)`
Draws a fast horizontal line on the OLED screen.
- **Parameters**:
  - `x` (int): Starting X coordinate.
  - `y` (int): Y coordinate.
  - `w` (int): Width.
  - `color` (int): 1 for white, 0 for black.
- **Returns**: `nil`

#### `oledDrawFastVLine(x, y, w, color)`
Draws a fast vertical line on the OLED screen.
- **Parameters**:
  - `x` (int): X coordinate.
  - `y` (int): Starting Y coordinate.
  - `w` (int): Height.
  - `color` (int): 1 for white, 0 for black.
- **Returns**: `nil`

#### `oledDrawCircle(x, y, r, color)`
Draws a circle outline on the OLED screen.
- **Parameters**:
  - `x` (int): Center X coordinate.
  - `y` (int): Center Y coordinate.
  - `r` (int): Radius.
  - `color` (int): 1 for white, 0 for black.
- **Returns**: `nil`

#### `oledDrawFilledCircle(x, y, r, color)`
Draws a filled circle on the OLED screen.
- **Parameters**:
  - `x` (int): Center X coordinate.
  - `y` (int): Center Y coordinate.
  - `r` (int): Radius.
  - `color` (int): 1 for white, 0 for black.
- **Returns**: `nil`

#### `oledCreateIcon(width, height, data)`
Creates an icon from binary data and returns its ID.
- **Parameters**:
  - `width` (int): Width of the icon.
  - `height` (int): Height of the icon.
  - `data` (int array): Binary pixel data for the icon.
- **Returns**: `int` (The icon ID)

#### `oledDrawIcon(x, y, iconId)`
Draws a previously created icon at the specified position.
- **Parameters**:
  - `x` (int): X coordinate.
  - `y` (int): Y coordinate.
  - `iconId` (int): The icon ID returned by `oledCreateIcon`.
- **Returns**: `nil`

## Led Strips

[↑ Back to top](#topics)

#### `ledsBegin(led_count, [max_brightness])`
Initializes the LED strip.
- **Parameters**:
  - `led_count` (int): The number of LEDs in the strip.
  - `max_brightness` (int, optional): Maximum brightness (0-255). Default is `128`.
- **Returns**: `bool`

#### `ledsBeginDual(led_count, led_count2, [max_brightness])`
Initializes two individual LED strips.
- **Parameters**:
  - `led_count` (int): Number of LEDs in the first strip.
  - `led_count2` (int): Number of LEDs in the second strip.
  - `max_brightness` (int, optional): Maximum brightness (0-255). Default is `128`.
- **Returns**: `bool`

#### `ledsIsManaged()`
Checks if the LEDs are in managed mode.
- **Returns**: `bool`

#### `ledsGentlySeBrightness(brightness, [rate, [startAmount]])`
Slowly ramps up the brightness to a target value.
- **Parameters**:
  - `brightness` (int): Target brightness.
  - `rate` (int, optional): Units of brightness increased per frame. Default is `1`.
  - `startAmount` (int, optional): Starting brightness. Default is `0`.
- **Returns**: `nil`

#### `ledsStackCurrentBehavior()`
Saves the current LED behavior state to a stack, useful for temporary behavior changes.
- **Returns**: `int`

#### `ledsPopBehavior()`
Restores the LED behavior from the top of the stack.
- **Returns**: `int`

#### `ledsSegmentRange(id, from, to)`
Defines a segment of the LED strip for independent control.
- **Parameters**:
  - `id` (int): Segment ID (0-15).
  - `from` (int): Starting LED index.
  - `to` (int): Ending LED index.
- **Returns**: `nil`

#### `ledsSetManaged(managed)`
Enables or disables managed mode.
- **Parameters**:
  - `managed` (bool): `true` to enable, `false` to disable.
- **Returns**: `nil`

#### `ledsSetBrightness(brightness)`
Sets the LED strip brightness.
- **Parameters**:
  - `brightness` (int): Brightness (0-255).
- **Returns**: `nil`

#### `ledsGetBrightness()`
Returns the current LED strip brightness.
- **Returns**: `int`

#### `ledsDisplay()`
Pushes LED changes to the strip. Use when managed mode is disabled.
- **Returns**: `nil`

#### `ledsSegmentBehavior(id, behavior, [p1, p2, p3, p4])`
Sets the behavior for a specific LED segment.
- **Parameters**:
  - `id` (int): Segment ID (0-15).
  - `behavior` (int): Behavior constant (e.g. `BEHAVIOR_PRIDE`, `BEHAVIOR_ROTATE`).
  - `p1..p4` (int, optional): Behavior-specific parameters.
- **Returns**: `nil`

#### `ledsSegmentTweenBehavior(id, behavior, [p1, p2, p3, p4])`
Transitions smoothly to a new behavior for a specific LED segment.
- **Parameters**:
  - `id` (int): Segment ID (0-15).
  - `behavior` (int): Target behavior constant.
  - `p1..p4` (int, optional): Behavior-specific parameters.
- **Returns**: `nil`

#### `ledsSegmentTweenSpeed(id, speed)`
Sets the tween transition speed for a specific LED segment.
- **Parameters**:
  - `id` (int): Segment ID (0-15).
  - `speed` (int): Tween speed (higher = slower transition).
- **Returns**: `nil`

#### `ledsSetColor(id, r, g, b)`
Sets the color of an individual LED. Best used outside managed mode.
- **Parameters**:
  - `id` (int): Individual LED index.
  - `r, g, b` (int): RGB color components (0-255).
- **Returns**: `nil`

#### `ledsSegmentColor(id, r, g, b)`
Sets the color of a LED segment. Best used outside managed mode.
- **Parameters**:
  - `id` (int): Segment ID.
  - `r, g, b` (int): RGB color components (0-255).
- **Returns**: `nil`

## FFT / Audio Analysis

[↑ Back to top](#topics)

These functions expose a microphone/audio FFT (Fast Fourier Transform) analyzer, for audio-reactive stuff.

#### `beginFft(gpio, num_samples, samplingFreq = 44100, noiseThreshold = 2000, bandCount = 16)`
Initializes the FFT/audio analyzer.
- **Returns**: unknown

#### `stopFft()`
Stops the FFT/audio analyzer.
- **Returns**: unknown

#### `deinitFft()`
De-initializes the FFT/audio analyzer, releasing associated resources.
- **Returns**: unknown

#### `updateFft()`
Updates/reads the FFT analysis for the current frame. Likely needs to be called once per loop when not managed.
- **Returns**: unknown

#### `setManaged(managed)`
Enables or disables managed mode for the FFT analyzer (kinda like `ledsSetManaged`/`setPanelManaged`).
- **Parameters**:
  - `managed` (bool, presumed)
- **Returns**: unknown

#### `isManagedFft()`
Returns whether the FFT analyzer is in managed mode.
- **Returns**: `bool` (presumed)

#### `isRunningFft()`
Returns whether the FFT analyzer is currently running.
- **Returns**: `bool` (presumed)

#### `setNoiseThreshold(threshold)`
Sets the noise floor threshold for the FFT analyzer.
- **Parameters**:
  - `threshold` (int)
- **Returns**: unknown

#### `getBandCountFft()`
Returns the number of frequency bands available from the FFT analyzer.
- **Returns**: `int` 

#### `getBandValueFft(band)`
Returns the current value of a specific frequency band.
- **Parameters**:
  - `band` (int): Band index.
- **Returns**: int

## Arduino Core

[↑ Back to top](#topics)

#### `tone(frequency)`
Generates a tone on the buzzer.
- **Parameters**:
  - `frequency` (int): Tone frequency in Hz.
- **Returns**: `nil`

#### `toneDuration(frequency, duration)`
Generates a tone with a set duration.
- **Parameters**:
  - `frequency` (int): Tone frequency in Hz.
  - `duration` (int): Duration in milliseconds.
- **Returns**: `nil`

#### `noTone()`
Stops tone generation.
- **Returns**: `nil`

#### `millis()`
Returns the number of milliseconds the system has been running.
- **Returns**: `int`

#### `delay(ms)`
Delays execution for the specified number of milliseconds.
- **Parameters**:
  - `ms` (int): Milliseconds to delay.
- **Returns**: `nil`

#### `delayMicroseconds(us)`
Delays execution for the specified number of microseconds.
- **Parameters**:
  - `us` (int): Microseconds to delay.
- **Returns**: `nil`

#### `vTaskDelay(ticks)`
Delays using the FreeRTOS task delay (yields to other tasks).
- **Parameters**:
  - `ticks` (int): Number of FreeRTOS ticks to delay.
- **Returns**: `nil`

#### `digitalWrite(pin, value)`
Writes a digital value to a pin.
- **Parameters**:
  - `pin` (int): Pin number.
  - `value` (int): `HIGH` or `LOW`.
- **Returns**: `nil`

#### `digitalRead(pin)`
Reads a digital value from a pin.
- **Parameters**:
  - `pin` (int): Pin number.
- **Returns**: `int` (`HIGH` or `LOW`)

#### `analogRead(pin)`
Reads an analog value from a pin.
- **Parameters**:
  - `pin` (int): Pin number.
- **Returns**: `int`

#### `pinMode(pin, mode)`
Sets the mode of a pin.
- **Parameters**:
  - `pin` (int): Pin number.
  - `mode` (int): `INPUT`, `OUTPUT`, `INPUT_PULLUP`, etc.
- **Returns**: `nil`

### Serial Communication

SerialIo uses pins IO1 and IO2 as TX and RX.

#### `beginSerialIo([baud])`
Initializes the secondary serial port (IO1/IO2 pins).
- **Parameters**:
  - `baud` (int, optional): Baud rate. Default is `115200`.
- **Returns**: `nil`

#### `setTimeoutSerialIo(timeout)`
Sets the read timeout for the secondary serial port.
- **Parameters**:
  - `timeout` (int): Timeout in milliseconds.
- **Returns**: `nil`

#### `serialIoAvaliable()`
Returns the number of bytes available to read from the secondary serial port.
- **Returns**: `int`

#### `serialAvaliable()`
Returns the number of bytes available to read from the primary serial port.
- **Returns**: `int`

#### `serialIoReadStringUntil([terminator])`
Reads characters from the secondary serial port until the terminator is found.
- **Parameters**:
  - `terminator` (char, optional): Terminating character. Default is `'\n'`.
- **Returns**: `string`

#### `serialReadStringUntil([terminator])`
Reads characters from the primary serial port until the terminator is found.
- **Parameters**:
  - `terminator` (char, optional): Terminating character. Default is `'\n'`.
- **Returns**: `string`

#### `serialIoRead()`
Reads one byte from the secondary serial port.
- **Returns**: `int` (Byte read, or -1 if none available)

#### `serialRead()`
Reads one byte from the primary serial port.
- **Returns**: `int` (Byte read, or -1 if none available)

#### `serialIoWrite(data)`
Writes a single byte to the secondary serial port.
- **Parameters**:
  - `data` (int): Byte to write (0-255).
- **Returns**: `int` (Bytes written)

#### `serialWrite(data)`
Writes a single byte to the primary serial port.
- **Parameters**:
  - `data` (int): Byte to write (0-255).
- **Returns**: `int` (Bytes written)

#### `serialIoWriteString(data)`
Writes a string to the secondary serial port.
- **Parameters**:
  - `data` (string): The string to write.
- **Returns**: `int` (Bytes written)

#### `serialWriteString(data)`
Writes a string to the primary serial port.
- **Parameters**:
  - `data` (string): The string to write.
- **Returns**: `int` (Bytes written)

#### `serialIoAvailableForWrite()`
Returns the number of bytes that can be written without blocking to the secondary serial port.
- **Returns**: `int`

#### `serialAvailableForWrite()`
Returns the number of bytes that can be written without blocking to the primary serial port.
- **Returns**: `int`

### I2C Communication

#### `wireBegin(addr)`
Initializes the I2C bus. Pass `0` for master mode.
- **Parameters**:
  - `addr` (uint8_t): 7-bit slave address, or `0` for master mode.
- **Returns**: `bool`

#### `wireAvailable()`
Returns the number of bytes available for reading from the I2C bus.
- **Returns**: `int`

#### `wireBeginTransmission(addr)`
Begins a transmission to the specified I2C address.
- **Parameters**:
  - `addr` (uint8_t): 7-bit device address.
- **Returns**: `nil`

#### `wireEndTransmission([sendStop])`
Ends the I2C transmission.
- **Parameters**:
  - `sendStop` (bool, optional): Send stop condition. Default is `true`.
- **Returns**: `uint8_t` (Transmission status)

#### `wireRequestFrom(address, size, sendStop)`
Requests bytes from a specific I2C device.
- **Parameters**:
  - `address` (uint16_t): 7-bit device address.
  - `size` (int): Number of bytes to request.
  - `sendStop` (bool): Whether to send stop condition.
- **Returns**: `uint8_t` (Number of bytes received)

#### `wireRead()`
Reads one byte from the I2C buffer.
- **Returns**: `int`

#### `wireReadBytes(length)`
Reads multiple bytes from the I2C buffer.
- **Parameters**:
  - `length` (int): Number of bytes to read.
- **Returns**: `table` (Array of bytes)

#### `wirePeek()`
Peeks at the next byte in the I2C buffer without consuming it.
- **Returns**: `int`

#### `wireFlush()`
Flushes the I2C buffer.
- **Returns**: `nil`

#### `wireParseFloat()`
Parses a float from the I2C buffer.
- **Returns**: `float`

#### `wireParseInt()`
Parses an integer from the I2C buffer.
- **Returns**: `int`

#### `wireSetTimeout(timeout)`
Sets the I2C operation timeout.
- **Parameters**:
  - `timeout` (uint32_t): Timeout in milliseconds.
- **Returns**: `nil`

#### `wireGetTimeout()`
Gets the current I2C operation timeout.
- **Returns**: `uint32_t`

## IR Remote

[↑ Back to top](#topics)

These functions allow receiving commands from infrared remote controls via a VS1838B or compatible receiver.

#### `startIR()`
Initializes the IR receiver.
- **Returns**: `bool` (`true` if successful)

#### `hasIRStarted()`
Returns whether the IR receiver has been initialized.
- **Returns**: `bool`

#### `setIRInterruptPin(pin)`
Sets the GPIO pin connected to the IR receiver.
- **Parameters**:
  - `pin` (uint16_t): The GPIO pin number.
- **Returns**: `nil`

#### `enableIRInterrupt([mode])`
Enables the interrupt on the IR receiver pin.
- **Parameters**:
  - `mode` (int, optional): Interrupt mode (e.g. `CHANGE`, `RISING`, `FALLING`). Default is `CHANGE`.
- **Returns**: `nil`

#### `disableIRInterrupt()`
Disables the interrupt on the IR receiver pin.
- **Returns**: `nil`

#### `hasIRCommand()`
Returns whether a new IR command has been received.
- **Returns**: `bool`

#### `getLastIRCommand()`
Returns the last received IR command as an `IrCommand` object.
- **Returns**: `IrCommand`

## 2D Models

[↑ Back to top](#topics)

These functions expose the 2D model rendering system added in v3.0.0. Models are rendered on the panel using the keyframe animation system. Model frame IDs start at `MODEL_FRAME_ID_OFFSET`.

#### `loadModel(modelData, name)`
Loads a 2D model from a JSON data string or from a file and registers it under the given name.
- **Parameters**:
  - `modelData` (string): Model data or file path.
  - `name` (string, optional): Identifier name for the model. Default is `""`.
- **Returns**: `Model` object

### `Model` Class

Model objects are returned by `loadModel()`. They cannot be created directly.

#### `model:Recalculate()`
Recalculates the model's geometry (e.g. after modifying points).
- **Returns**: `nil`

#### `model:Reset()`
Resets the model to its original state.
- **Returns**: `nil`

#### `model:GetId()`
Returns the internal ID of the model.
- **Returns**: `int`

#### `model:CopyToRaster()`
Copies the current model state to the rasterizer for rendering.
- **Returns**: `nil`

#### `model:AddPointGroup()`
Adds a new point group to the model.
- **Returns**: `nil`

#### `model:SetTriangle(index, p1, p2, p3)`
Defines a triangle by referencing three point indices.
- **Parameters**:
  - `index` (int): Triangle index.
  - `p1, p2, p3` (int): Point indices.
- **Returns**: `nil`

#### `model:GetTriangle(index)`
Gets the triangle data at the given index.
- **Parameters**:
  - `index` (int): Triangle index.
- **Returns**: `table`

#### `model:SetBatchOperations(enabled)`
Enables or disables batch operations mode for faster bulk transformations.
- **Parameters**:
  - `enabled` (bool)
- **Returns**: `nil`

#### `model:SetAccumulativeOperations(enabled)`
Enables or disables accumulative mode, where transforms stack rather than replace.
- **Parameters**:
  - `enabled` (bool)
- **Returns**: `nil`

#### `model:SetPointPosition(pointId, x, y, z)`
Sets the absolute position of a single point.
- **Parameters**:
  - `pointId` (int): Point index.
  - `x, y, z` (float): New position.
- **Returns**: `nil`

#### `model:TranslatePoint(pointId, x, y, z)`
Moves a single point by a delta.
- **Parameters**:
  - `pointId` (int): Point index.
  - `x, y, z` (float): Translation delta.
- **Returns**: `nil`

#### `model:SetPointsPosition(x, y, z)`
Sets all points to the given position.
- **Parameters**:
  - `x, y, z` (float): Position.
- **Returns**: `nil`

#### `model:ScalePoints(x, y, z)`
Scales all points by the given factors.
- **Parameters**:
  - `x, y, z` (float): Scale factors.
- **Returns**: `nil`

#### `model:TranslatePoints(x, y, z)`
Translates all points by a delta.
- **Parameters**:
  - `x, y, z` (float): Translation delta.
- **Returns**: `nil`

#### `model:Scale(x, y, z)`
Scales the whole model.
- **Parameters**:
  - `x, y, z` (float): Scale factors.
- **Returns**: `nil`

#### `model:Rotate(x, y, z)`
Rotates the model.
- **Parameters**:
  - `x, y, z` (float): Rotation angles.
- **Returns**: `nil`

#### `model:Translate(x, y, z)`
Translates the model.
- **Parameters**:
  - `x, y, z` (float): Translation delta.
- **Returns**: `nil`

#### `model:GetCenter()`
Returns the center point of the model.
- **Returns**: `float, float, float` (x, y, z)

## Sprites

[↑ Back to top](#topics)

Sprites are 2D images that can be drawn over the screen. They work like objects, which have position, rotation and their own frames. 
Frames can be created from an empty square, or loading a PNG. They should be used as overlays in managed mode.

#### `Sprite()`
Creates a new, empty `Sprite` object and registers it in the animation's sprite pool.
- **Returns**: `Sprite` object

### `Sprite` Class

#### `sprite:GetId()`
Returns the internal ID of the sprite.
- **Returns**: `int`

#### `sprite:CreateEmptyTexture(width, height)`
Allocates a blank pixel buffer for the sprite of the given size.
This will increase in 1 the number of frames.
- **Parameters**:
  - `width` (int)
  - `height` (int)
- **Returns**: `nil`

#### `sprite:LoadFromPng(filename)`
Loads sprite pixel data from a PNG file on the SD card.
- **Parameters**:
  - `filename` (string)
- **Returns**: `bool`

#### `sprite:CropSprite(x, y, width, height)`
Crops the sprite to a sub-region. This does not remove data from the texture, only limits the drawing space.
- **Parameters**:
  - `x, y` (int): Top-left of the crop region.
  - `width, height` (int): Size of the crop region.
- **Returns**: `nil`

#### `sprite:SetPixelColor(id, x, y, color)`
Sets the color of an individual pixel of a specific frame in the sprite.
- **Parameters**:
  - `id` (int) Frame id
  - `x, y` (int) position
  - `color` (int) color RGB565
- **Returns**: `nil`

#### `sprite:GetWidth()`
Returns the sprite's width in pixels.
- **Returns**: `int`

#### `sprite:GetHeight()`
Returns the sprite's height in pixels.
- **Returns**: `int`

#### `sprite:SetPosition(x, y)`
Sets the sprite's draw position.
- **Parameters**:
  - `x, y` (int)
- **Returns**: `nil`

#### `sprite:SetRotation(angle)`
Sets the sprite's rotation.
- **Parameters**:
  - `angle` (float) rad
- **Returns**: `nil`

#### `sprite:SetFrameId(frameId)`
Sets the current frame index that will be drawn.
- **Parameters**:
  - `frameId` (int)
- **Returns**: `nil`

#### `sprite:GetFrameId()`
Returns the current frame index.
- **Returns**: `int`

#### `sprite:GetFrameCount()`
Returns the total number of frames in the sprite.
- **Returns**: `int`

#### `sprite:SetTransparencyColor([color])`
Sets a color to be treated as transparent when drawing the sprite.
- **Parameters**:
  - `color` (int, optional): Default is `-1` (disabled/no transparency key).
- **Returns**: `nil`

#### `sprite:UseCustomShader(enabled)`
Enables or disables the use of a custom shader on this sprite.
- **Parameters**:
  - `enabled` (bool)
- **Returns**: `nil`

#### `sprite:SetShader(shader, [intensity])`
Sets a shader to apply to this sprite specifically (mirrors `setAnimationShader`).
- **Parameters**:
  - `shader` (int): Shader constant, e.g. `SHADER_RAINBOW`.
  - `intensity` (float, optional): Default is `1.0`.
- **Returns**: `nil`

#### `sprite:Clone()`
Creates and returns a duplicate of this sprite. This does not clone the textures itself. The new sprite will reuse the same textures. So if you edit the texture of the original sprite, it will reflect on the new one. But loading and adding frames does not affect the clone or the parent sprite.
- **Returns**: `Sprite` object

#### `sprite:Draw(x, y, [scale, [shader, [flipConfig]]])`
Draws the sprite to the panel at the given position.
- **Parameters**:
  - `x, y` (int): Draw position.
  - `shaderStrenght` (float): Default is `1.0`.
  - `shader` (int): Default is `SHADER_NONE`.
  - `flipConfig` (FlipConfig): Horizontal/vertical flip configuration. Default is a default-constructed `{flipLeft = false, flipRight=false, modeLeft=COLOR_MODE_RGB, modeRight=COLOR_MODE_RGB}`.
- **Returns**: `nil`

## Keyframe Animations

[↑ Back to top](#topics)

Keyframe animations drive 2D model transformations over time. They are created with `newKeyframeAnimation()` and played via `setPanelModelAnimation()`.

#### `newKeyframeAnimation(duration)`
Creates a new keyframe animation of the given duration.
- **Parameters**:
  - `duration` (int): Total duration in milliseconds.
- **Returns**: `KeyframeAnimation` object

### `KeyframeAnimation` Class

#### `anim:Reset()`
Resets the animation to the beginning.
- **Returns**: `nil`

#### `anim:GetId()`
Returns the internal ID of this animation (used with `setPanelModelAnimation`).
- **Returns**: `int`

#### `anim:AddTrack(track)`
Adds a `KeyframeTrack` to this animation.
- **Parameters**:
  - `track` (KeyframeTrack): The track to add.
- **Returns**: `nil`

### `KeyframeTrack` Class

KeyframeTrack objects can be created directly with `KeyframeTrack()`.

#### `KeyframeTrack()`
Creates a new, empty keyframe track.
- **Returns**: `KeyframeTrack` object

#### `track:Reset()`
Resets the track.
- **Returns**: `nil`

#### `track:SetResource(modelId)`
Associates this track with a model by ID.
- **Parameters**:
  - `modelId` (int): The model ID to control.
- **Returns**: `nil`

#### `track:AddKeyFrame(keyframe)`
Adds a keyframe to this track.
- **Parameters**:
  - `keyframe` (Keyframe): The keyframe to add.
- **Returns**: `nil`

### `KeyFrame(time, value)`
Creates a new keyframe. Parameters depend on the type of operation being animated.
- **Parameters**:
  - `time` (uint16): Time offset in ms.
  - `value` (Vec2f or similar): The value at this keyframe.
- **Returns**: `Keyframe` object

---

# Lua Constants

[↑ Back to top](#topics)

- [Engine Related](#engine-related)
- [Input](#input)
- [LED Behavior](#led-behavior)
- [Pins and GPIO](#pins-and-gpio)
- [Color Modes](#color-modes)
- [Shaders](#shaders)
- [Keyframe Types](#keyframe-types)
- [BLE Power Levels](#ble-power-levels)
- [Interrupt Modes](#interrupt-modes)
- [ESP32 Reset Reason](#esp32-reset-reason)

## Engine Related

- `PANDA_VERSION`: String representing the current firmware version.
- `VCC_THRESHOLD_START`: Minimum voltage required for the system to start.
- `VCC_THRESHOLD_HALT`: Voltage below which the system halts to prevent damage.
- `OLED_SCREEN_WIDTH`: Width of the internal OLED screen in pixels.
- `OLED_SCREEN_HEIGHT`: Height of the internal OLED screen in pixels.
- `CANVAS_WIDTH`: Width of the HUB75 panel in pixels.
- `CANVAS_HEIGHT`: Height of the HUB75 panel in pixels.
- `PANEL_WIDTH`: Alias for `CANVAS_WIDTH`.
- `PANEL_HEIGHT`: Alias for `CANVAS_HEIGHT`.
- `DEFAULT_CANVAS_WIDTH`: Default/fallback canvas width used when no custom canvas size is configured.
- `DEFAULT_CANVAS_HEIGHT`: Default/fallback canvas height used when no custom canvas size is configured.
- `POWER_MODE_NONE`: Ignore any powering behavior.
- `POWER_MODE_USB_5V`: Power mode for USB 5V input.
- `POWER_MODE_USB_9V`: Power mode for USB 9V PD input.
- `POWER_MODE_BATTERY`: Power mode for battery input.
- `BUILT_IN_POWER_MODE`: The power mode set at compile time.
- `SERVO_COUNT`: The number of servos.
- `MODEL_FRAME_ID_OFFSET`: Frame ID offset used for model-based frames. Model frame IDs start at this value.
- `EDIT_MODE_PIN`: The GPIO pin number used for entering edit mode.
- `EDIT_ENABLE_LOGIC_LEVEL`: The logic level that triggers edit mode.
- `ENABLE_EDIT_MODE`: `1` if edit mode is compiled in.
- `PIN_ENABLE_REGULATOR`: GPIO pin that controls the voltage regulator (`-1` if not defined).
- `PIN_USB_BATTERY_IN`: GPIO pin for battery/USB voltage reading (`-1` if not defined).
- `USE_PIN_BATTERY_IN`: `1` if battery voltage sensing is enabled.
- `RESISTOR_DIVIDER_R8`, `RESISTOR_DIVIDER_R9`: Resistor values for the voltage divider.
- `V_REF`: ADC reference voltage.
- `BLACK`: OLED color constant (`1`).
- `WHITE`: OLED color constant (`0`).
- `MAX_LED_GROUPS`: Number of available LED groups/segments.

## Input

- `BUTTON_RELEASED`: Button is not pressed.
- `BUTTON_JUST_PRESSED`: Button was just pressed this frame.
- `BUTTON_PRESSED`: Button is currently held.
- `BUTTON_JUST_RELEASED`: Button was just released this frame.
- `DEVICE_X_BUTTON_LEFT/RIGHT/UP/DOWN/CONFIRM/AUX_A/AUX_B/BACK`: Button mappings for device X (0 to `MAX_BLE_CLIENTS`).
- `BUTTON_LEFT/RIGHT/UP/DOWN/CONFIRM/AUX_A/AUX_B/BACK`: Shorthand aliases for device 0 buttons.

## Led Behavior

- `BEHAVIOR_NONE`: No behavior; LEDs remain off or unchanged.
- `BEHAVIOR_PRIDE`: Rainbow cycling color pattern.
- `BEHAVIOR_ROTATE`: Rotates a color along the segment.
- `BEHAVIOR_RANDOM_COLOR`: Each LED gets a random color.
- `BEHAVIOR_FADE_CYCLE`: Fades brightness in a cycle.
- `BEHAVIOR_ROTATE_FADE_CYCLE`: Combines rotation and fading.
- `BEHAVIOR_STATIC_RGB`: Static RGB color.
- `BEHAVIOR_STATIC_HSV`: Static HSV color.
- `BEHAVIOR_RANDOM_BLINK`: Randomly blinks LEDs.
- `BEHAVIOR_ROTATE_SINE_V`: Sine wave on brightness (V in HSV).
- `BEHAVIOR_ROTATE_SINE_S`: Sine wave on saturation (S in HSV).
- `BEHAVIOR_ROTATE_SINE_H`: Sine wave on hue (H in HSV).
- `BEHAVIOR_FADE_IN`: Fades LEDs in from off to a specified color.
- `BEHAVIOR_NOISE`: Noise/static effect.
- `BEHAVIOR_ICON_X`: Displays an X icon pattern on the segment.
- `BEHAVIOR_ICON_I`: Displays an I icon pattern on the segment.
- `BEHAVIOR_ICON_V`: Displays a V icon pattern on the segment.
- `MAX_LED_GROUPS`: Total number of available LED groups.

## Pins and GPIO

- `D1`, `D2`: External IO pins (IO1 and IO2).
- `HIGH`, `LOW`: Digital pin states.
- `INPUT`, `OUTPUT`: Pin modes.
- `INPUT_PULLUP`, `INPUT_PULLDOWN`: Input modes with internal resistors.
- `ANALOG`: Analog pin mode.
- `OUTPUT_OPEN_DRAIN`, `OPEN_DRAIN`: Open-drain output mode.
- `PULLDOWN`: Pull-down mode.

## Color Modes

Used with `setPanelColorMode()` to handle panels with non-standard RGB channel wiring.

- `COLOR_MODE_RGB`
- `COLOR_MODE_RBG`
- `COLOR_MODE_GRB`
- `COLOR_MODE_GBR`
- `COLOR_MODE_BRG`
- `COLOR_MODE_BGR`

## Shaders

Used with `setAnimationShader()`.

- `SHADER_NONE`: No shader.
- `SHADER_RAINBOW`: Rainbow overlay.
- `SHADER_FIRE`: Fire effect.
- `SHADER_TEXTURE`: Texture-based shader.
- `SHADER_TRANS`: Transgender flag color shader 🏳️‍⚧️
- `SHADER_LAST`: Alias for the last available shader.

## Keyframe Types

Used when building `KeyframeTrack` animations.

- `KEYFRAME_TRANSLATE`: Translation keyframe.
- `KEYFRAME_ROTATE`: Rotation keyframe.
- `KEYFRAME_SCALE`: Scale keyframe.
- `KEYFRAME_RESET`: Resets the model transform.
- `KEYFRAME_COLOR`: Color change keyframe.
- `KEYFRAME_VISIBILITY`: Visibility toggle keyframe.
- `KEYFRAME_SINE`: Sine wave motion keyframe.
- `KEYFRAME_SHADER`: Shader change keyframe.

## BLE Power Levels

Used with `startBLERadio(powerLevel)`.

- `ESP_PWR_LVL_N24` through `ESP_PWR_LVL_P21`: BLE TX power levels in dBm, from -24 dBm (lowest) to +21 dBm (highest).

## Interrupt Modes

Used with `enableIRInterrupt([mode])` and `attachInterrupt`.

- `RISING`: Trigger on rising edge.
- `FALLING`: Trigger on falling edge.
- `CHANGE`: Trigger on any change.
- `ONLOW`: Trigger when pin is low.
- `ONHIGH`: Trigger when pin is high.
- `ONLOW_WE`: Trigger when low (wake-up capable).
- `ONHIGH_WE`: Trigger when high (wake-up capable).

## ESP32 Reset Reason

Returned by `getResetReason()`.

- `ESP_RST_UNKNOWN`
- `ESP_RST_POWERON`
- `ESP_RST_EXT`
- `ESP_RST_SW`
- `ESP_RST_PANIC`
- `ESP_RST_INT_WDT`
- `ESP_RST_TASK_WDT`
- `ESP_RST_WDT`
- `ESP_RST_DEEPSLEEP`
- `ESP_RST_BROWNOUT`
- `ESP_RST_SDIO`

---

# Led Behaviors

[↑ Back to top](#topics)

## Parameter Truth Table

All parameter values are in the range 0–255 unless marked as **ms** (milliseconds).

| Mode                  | param1       | param2         | param3          | param4         |
|-----------------------|--------------|----------------|-----------------|----------------|
| `none`                | -            | -              | -               | -              |
| `pride`               | -            | -              | -               | -              |
| `rotate`              | -            | -              | -               | speed (ms)     |
| `random_color`        | -            | -              | -               | -              |
| `fade_cycle`          | hue          | speed (ms)     | min_brightness  | -              |
| `rotate_fade_cycle`   | hue          | speed (ms)     | min_brightness  | rotate_speed (ms) |
| `color_rgb`           | red          | green          | blue            | -              |
| `color_hsv`           | hue          | saturation     | value           | -              |
| `random_blink`        | base_hue     | hue_variance   | brightness      | blink_speed (ms) |
| `icon_x`              | -            | -              | -               | -              |
| `icon_i`              | -            | -              | -               | -              |
| `icon_v`              | -            | -              | -               | -              |
| `rotate_sine_v`       | hue          | saturation     | speed (ms)      | -              |
| `rotate_sine_s`       | hue          | brightness     | speed (ms)      | -              |
| `rotate_sine_h`       | saturation   | brightness     | speed (ms)      | -              |
| `fade_in`             | hue          | saturation     | step            | delay (ms)     |
| `noise`               | -            | -              | step            | delay (ms)     |

Parameters are passed positionally to `ledsSegmentBehavior(id, behavior, param1, param2, param3, param4)`. Unused positions should be `0`.

---

* `BEHAVIOR_PRIDE`
  - **Description**: Rainbow cycling pattern.
  - **Parameters**: None.

* `BEHAVIOR_ROTATE`
  - **Description**: Rotates a single color through the segment.
  - **Parameters**:
    - `4`: Speed of rotation (delay in ms).

* `BEHAVIOR_RANDOM_COLOR`
  - **Description**: Each LED gets a random color.
  - **Parameters**: None.

* `BEHAVIOR_FADE_CYCLE`
  - **Description**: Fades brightness up and down in a cycle.
  - **Parameters**:
    - `1`: Hue value (0-255).
    - `2`: Speed of the fade cycle.
    - `3`: Minimum brightness value.

* `BEHAVIOR_ROTATE_FADE_CYCLE`
  - **Description**: Combines rotation and fading.
  - **Parameters**:
    - `1`: Hue value (0-255).
    - `2`: Speed of the fade cycle.
    - `3`: Minimum brightness value.
    - `4`: Speed of rotation (delay in ms).

* `BEHAVIOR_STATIC_RGB`
  - **Description**: Sets all LEDs in the segment to a static RGB color.
  - **Parameters**:
    - `1`: Red (0-255).
    - `2`: Green (0-255).
    - `3`: Blue (0-255).

* `BEHAVIOR_STATIC_HSV`
  - **Description**: Sets all LEDs in the segment to a static HSV color.
  - **Parameters**:
    - `1`: Hue (0-255).
    - `2`: Saturation (0-255).
    - `3`: Brightness (0-255).

* `BEHAVIOR_RANDOM_BLINK`
  - **Description**: Randomly blinks LEDs with random colors and brightness.
  - **Parameters**:
    - `1`: Base hue (0-255).
    - `2`: Hue variation range.
    - `3`: Maximum brightness.
    - `4`: Delay between blinks (ms).

* `BEHAVIOR_ROTATE_SINE_V`
  - **Description**: Sine wave varying brightness rotates through the segment.
  - **Parameters**:
    - `1`: Base hue (0-255).
    - `2`: Base saturation (0-255).
    - `3`: Speed of the sine wave (ms).

* `BEHAVIOR_ROTATE_SINE_S`
  - **Description**: Sine wave varying saturation rotates through the segment.
  - **Parameters**:
    - `1`: Base hue (0-255).
    - `2`: Base brightness (0-255).
    - `3`: Speed of the sine wave (ms).

* `BEHAVIOR_ROTATE_SINE_H`
  - **Description**: Sine wave varying hue rotates through the segment.
  - **Parameters**:
    - `1`: Base saturation (0-255).
    - `2`: Base brightness (0-255).
    - `3`: Speed of the sine wave (ms).

* `BEHAVIOR_FADE_IN`
  - **Description**: Gradually fades LEDs in to a specified HSV color.
  - **Parameters**:
    - `1`: Hue (0-255).
    - `2`: Saturation (0-255).
    - `3`: Step size.
    - `4`: Delay between steps (ms).

* `BEHAVIOR_NOISE`
  - **Description**: Noise/static flickering effect across the segment.
  - **Parameters**:
    - `3`: Step size.
    - `4`: Delay between steps (ms).

* `BEHAVIOR_ICON_X`, `BEHAVIOR_ICON_I`, `BEHAVIOR_ICON_V`
  - **Description**: Display a simple icon pattern (X, I, or V shape) on the segment.
  - **Parameters**: None.

* `BEHAVIOR_NONE`
  - **Description**: No behavior applied. LEDs remain off or unchanged.
  - **Parameters**: None.

---

# Bluetooth Interface

[↑ Back to top](#topics)

## Radio and BLE functions

#### `startBLE()`
Initializes the BLE environment but does not start the radio yet.
- **Returns**: `bool`

#### `hasBLEStarted()`
Returns whether BLE has been initialized.
- **Returns**: `bool`

#### `startBLERadio(powerLevel)`
Starts the BLE radio at the specified power level.
- **Parameters**:
  - `powerLevel` (int): One of the `ESP_PWR_LVL_*` constants.
- **Returns**: `bool`

#### `getRRSI(connId)`
Returns the RSSI of a given connection.
- **Parameters**:
  - `connId` (int): The connection ID.
- **Returns**: `int`

#### `getClientIdFromControllerId(id)`
Returns the connection ID for a given controller ID.
- **Parameters**:
  - `id` (int): Controller ID.
- **Returns**: `int`

#### `getConnectedRemoteControls()`
Returns the number of currently connected remote controls.
- **Returns**: `int`

#### `isElementIdConnected(id)`
Checks if a remote control with the given ID is connected.
- **Parameters**:
  - `id` (int): The controller ID.
- **Returns**: `bool`

#### `beginBleScanning()`
Starts scanning for BLE devices.
- **Returns**: `nil`

#### `setLogDiscoveredBleDevices(bool)`
When `true`, each scanned device will be saved to the log file.
- **Returns**: `nil`

#### `setMaximumControls(count)`
Sets the maximum number of simultaneously connected remote controls.
- **Parameters**:
  - `count` (int): Maximum number of devices.
- **Returns**: `nil`

#### `getCharacteristicsFromService(connectionId, uuid, refresh)`
Gets all characteristics of a given service on a connected device.
- **Parameters**:
  - `connectionId` (int): Connection ID.
  - `uuid` (string): Service UUID.
  - `refresh` (bool): Whether to re-query the device.
- **Returns**: `nil`

## Handling BLE connections

Here's an example of how we accept connections from a mouse/keyboard/joystick:
```lua
drivers.mouseHandler = BleServiceHandler("00001812-0000-1000-8000-00805f9b34fb")
drivers.mouseHandler:SetOnConnectCallback(drivers.onConnectHID)
drivers.mouseHandler:SetOnDisconnectCallback(drivers.onDisconnectHID)
drivers.mouseListener = drivers.mouseHandler:AddCharacteristics("2a4d")
drivers.mouseListener:SetSubscribeCallback(drivers.onMouseCallback) 
drivers.mouseListener:SetCallbackModeStream(false)
```

First we create a service handler and pass a UUID. That specific UUID belongs to a Human Interface Device service, so any device advertising mouse/keyboard/joystick capabilities will match.

The moment it is registered, any scanned device advertising that service UUID will be attempted to connect.

Connect callback:
```lua
function drivers.onConnectHID(connectionId, controllerId, address, name)
    log("Connected conId="..connectionId.." controller="..controllerId.." addr=\""..address.."\" name=["..name.."]")
end
```

The `connectionId` is unique per connection. The `controllerId` is assigned by Protopanda — the first connected device is `0`, the second is `1`, etc. IDs are freed on disconnect and can be reused.

Disconnect callback:
```lua
function drivers.onDisconnectHID(connectionId, controllerId, reason)
    log("Disconnected "..connectionId.." due ".. reason)
end
``` 

Attaching a listener to a characteristic:
```lua
drivers.mouseListener = drivers.mouseHandler:AddCharacteristics("2a4d")
drivers.mouseListener:SetSubscribeCallback(drivers.onMouseCallback) 
drivers.mouseListener:SetRequired(true)
drivers.mouseListener:SetCallbackModeStream(false)
```

Characteristics callback:
```lua
function drivers.onMouseCallback(connectionId, controllerId, data)
  -- data is an array of integers 0-255
end
```

## Handling BLE functions

### `BleServiceHandler(uuid)`
Creates a service handler object. Keep this in a global or long-lived variable — the Lua GC won't destroy the C++ reference but will destroy the Lua reference if it goes out of scope.
- **Parameters**:
  - `uuid` (string): Full 128-bit UUID string.
- **Returns**: `BleServiceHandlerObject`

### `BleServiceHandler::ReadFromCharacteristics(clientId, uuid)`
Reads data from a specific characteristic on a connected client.
- **Parameters**:
  - `clientId` (int): Connection ID.
  - `uuid` (string): 16-bit or full UUID.
- **Returns**: `int array`

### `BleServiceHandler::GetServices(clientId)`
Returns the available services on the connected client.
- **Parameters**:
  - `clientId` (int): Connection ID.
- **Returns**: `string array`

### `BleServiceHandler::GetRSSI(clientId)`
Returns the RSSI for a specific connected client.
- **Parameters**:
  - `clientId` (int): Connection ID.
- **Returns**: `int`

### `BleServiceHandler::GetClientIdFromControllerId(id)`
Returns the connection ID for a given controller ID.
- **Parameters**:
  - `id` (int): Controller ID.
- **Returns**: `int`

### `BleServiceHandler::AddAddressRequired(address)`
Restricts connections to only devices with this MAC address. Can be called multiple times to allow a list of addresses. If both `AddAddressRequired` and `AddNameRequired` are set, both conditions must match.
```lua
Handler:AddAddressRequired("AA:BB:CC:DD:EE")
Handler:AddAddressRequired("FF:00:11:22:33")
```
- **Parameters**:
  - `address` (string): MAC address string.

### `BleServiceHandler::AddNameRequired(name)`
Restricts connections to only devices with this advertised name. Can be called multiple times.
```lua
Handler:AddNameRequired("VR-PARK")
Handler:AddNameRequired("Beauty-r1")
```
- **Parameters**:
  - `name` (string): Device name.

### `BleServiceHandler::GetCharacteristics(clientId)`
Returns the available characteristics in the current service for the connected client.
- **Parameters**:
  - `clientId` (int): Connection ID.
- **Returns**: `string array`

### `BleServiceHandler::WriteToCharacteristics(message, clientId, uuid[, reply])`
Writes a message to a specific characteristic.
- **Parameters**:
  - `message` (int array): Bytes to write.
  - `clientId` (int): Connection ID.
  - `uuid` (string): 16-bit or full UUID.
  - `reply` (bool, optional): If `true`, waits for a response.
- **Returns**: `bool`

### `BleServiceHandler::SetOnDisconnectCallback(callback)`
Sets the callback for when a device disconnects.
- **Parameters**:
  - `callback` (function): `function(connectionId, controllerId, reason)`

### `BleServiceHandler::SetOnConnectCallback(callback)`
Sets the callback for when a device connects.
- **Parameters**:
  - `callback` (function): `function(connectionId, controllerId, address, name)`

### `BleServiceHandler::AddCharacteristics(uuid)`
Returns a handler for the specified characteristic UUID.
- **Parameters**:
  - `uuid` (string): 16-bit or full UUID.
- **Returns**: `BleCharacteristicsHandlerObject`

### `BleCharacteristicsHandler::SetSubscribeCallback(callback)`
Sets the callback for incoming messages on this characteristic.
- **Parameters**:
  - `callback` (function): `function(connectionId, controllerId, data)`

### `BleCharacteristicsHandler::SetCallbackModeStream(stream)`
When `true`, processes one message per loop (messages queue up). When `false`, processes all queued messages per loop.
- **Parameters**:
  - `stream` (bool)

### `BleCharacteristicsHandler::SetRequired(req)`
If `true`, a device is disconnected if this characteristic is not present.
- **Parameters**:
  - `req` (bool)