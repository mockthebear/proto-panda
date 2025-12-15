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
- [Drawing](#drawing)
- [Image Decoding](#image-decoding)
- [Internal Screen](#internal-screen)
- [Remote Control](#remote-control)
- [LED Strips](#led-strips-1)
- [Arduino Core](#arduino-core)

## Power
#### `waitForPower()`
Forces the process to wait until the battery or USB voltage is above a threshold.
- **Returns**: `nil`

#### `panelPowerOn()`
Turns on the regulator, powering the panel and the 5V output.
- **Returns**: `nil`

#### `panelPowerOff()`
Turns off the regulator.
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
Sets the powering mode of the system. The mode can be `POWER_MODE_USB_5V`, `POWER_MODE_USB_9V`, or `POWER_MODE_BATTERY`.
- **Parameters**:
  - `mode` (int): The powering mode to set.
- **Returns**: `nil`

## System

### `getFreePsram()`
Returns the amount of free PSRAM (Pseudo Static RAM) available in bytes.
- **Returns**: `int` (The amount of free PSRAM in bytes).

### `getFreeHeap()`
Returns the amount of free heap memory available in bytes.
- **Returns**: `int` (The amount of free heap memory in bytes).

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
Lists files in directory.
- **Parameters**:
  - `path` (string): Directory path
  - `recursive` (bool): Whether to list recursively
- **Returns**: `string array`

#### `moveFile(src, dest)`
Moves/renames file.
- **Parameters**:
  - `src` (string): Source path
  - `dest` (string): Destination path
- **Returns**: `bool` (Success)

#### `removeFile(path)`
Deletes file.
- **Parameters**:
  - `path` (string): File path
- **Returns**: `bool` (Success)

#### `deleteBulkFile()`
Deletes frames.bulk file.
- **Returns**: `nil`

## Sensors
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
- **Returns**: `float` (The raw accelerometer value on the X axis).

#### `readAccelerometerY([device])`
Returns the raw value of the remote control's accelerometer sensor, Y axis only.
- **Parameters**:
  - `device` (int, optional): The ID of the remote control device. Default is `0`.
- **Returns**: `float` (The raw accelerometer value on the Y axis).

#### `readAccelerometerZ([device])`
Returns the raw value of the remote control's accelerometer sensor, Z axis only.
- **Parameters**:
  - `device` (int, optional): The ID of the remote control device. Default is `0`.
- **Returns**: `float` (The raw accelerometer value on the Z axis).

#### `readGyroX([device])`
Returns the raw value of the remote control's gyroscope sensor, X axis only.
- **Parameters**:
  - `device` (int, optional): The ID of the remote control device. Default is `0`.
- **Returns**: `int` (The raw gyroscope value on the X axis).

#### `readGyroY([device])`
Returns the raw value of the remote control's gyroscope sensor, Y axis only.
- **Parameters**:
  - `device` (int, optional): The ID of the remote control device. Default is `0`.
- **Returns**: `int` (The raw gyroscope value on the Y axis).

#### `readGyroZ([device])`
Returns the raw value of the remote control's gyroscope sensor, Z axis only.
- **Parameters**:
  - `device` (int, optional): The ID of the remote control device. Default is `0`.
- **Returns**: `int` (The raw gyroscope value on the Z axis).

#### `hasLidar()`
Checks for the presence of a lidar.
- **Returns**: `bool` (True if a lidar is present, otherwise False).

#### `readLidar()`
Returns the distance in mm from the lidar.
- **Returns**: `int` (The distance in millimeters).

#### `getInternalButtonStatus()`
Returns the status of the internal button.
- **Returns**: `int` (1 is pressed and 0 is released).

## Led panel Drawing

#### `flipPanelBuffer()`
The panels have two buffers. One for drawing and the other for rendering. When the drawing is finished in one buffer, this call flips them so you can draw into the other.
- **Returns**: `nil`

#### `DrawPixel(x, y, color)`
Draws a pixel at the specified coordinates with the given color.
- **Parameters**:
  - `x` (int): The X coordinate.
  - `y` (int): The Y coordinate.
  - `color` (int): The color to draw (use `color565` or `color444` to generate the color).
- **Returns**: `nil`

#### `drawPanelCircle(x, y, radius, color)`
Draws a circle outline at the specified coordinates with the given radius and color.
- **Parameters**:
  - `x` (int): The X coordinate of the center.
  - `y` (int): The Y coordinate of the center.
  - `radius` (int): The radius of the circle.
  - `color` (int): The color to draw.
- **Returns**: `nil`

### `drawPanelChar(x, y, c, color, bg, size)`
Draws a character on the panel at the specified coordinates with the given color and background.
- **Parameters**:
  - `x` (int): The X coordinate.
  - `y` (int): The Y coordinate.
  - `c` (char): The character to draw.
  - `color` (int): The color of the character (use `color565` or `color444` to generate the color).
  - `bg` (int): The background color (use `color565` or `color444` to generate the color).
  - `size` (int): The size of the character (1 for normal size, larger values for scaling).
- **Returns**: `nil`

#### `drawPanelFillCircle(x, y, radius, color)`
Draws a filled circle at the specified coordinates with the given radius and color.
- **Parameters**:
  - `x` (int): The X coordinate of the center.
  - `y` (int): The Y coordinate of the center.
  - `radius` (int): The radius of the circle.
  - `color` (int): The color to fill.
- **Returns**: `nil`

#### `drawPanelLine(x0, y0, x1, y1, color)`
Draws a line between two points with the specified color.
- **Parameters**:
  - `x0` (int): The X coordinate of the starting point.
  - `y0` (int): The Y coordinate of the starting point.
  - `x1` (int): The X coordinate of the ending point.
  - `y1` (int): The Y coordinate of the ending point.
  - `color` (int): The color to draw.
- **Returns**: `nil`

#### `drawPanelRect(x, y, width, height, color)`
Draws a rectangle outline at the specified position with the given width, height, and color.
- **Parameters**:
  - `x` (int): The X coordinate of the top-left corner.
  - `y` (int): The Y coordinate of the top-left corner.
  - `width` (int): The width of the rectangle.
  - `height` (int): The height of the rectangle.
  - `color` (int): The color to draw.
- **Returns**: `nil`

#### `drawPanelFillRect(x, y, width, height, color)`
Draws a filled rectangle at the specified position with the given width, height, and color.
- **Parameters**:
  - `x` (int): The X coordinate of the top-left corner.
  - `y` (int): The Y coordinate of the top-left corner.
  - `width` (int): The width of the rectangle.
  - `height` (int): The height of the rectangle.
  - `color` (int): The color to fill.
- **Returns**: `nil`

#### `color444(r, g, b)`
Converts RGB888 color values to RGB444 format.
- **Parameters**:
  - `r` (int): The red component (0-255).
  - `g` (int): The green component (0-255).
  - `b` (int): The blue component (0-255).
- **Returns**: `int` (The RGB444 color value).

#### `color565(r, g, b)`
Converts RGB888 color values to RGB565 format.
- **Parameters**:
  - `r` (int): The red component (0-255).
  - `g` (int): The green component (0-255).
  - `b` (int): The blue component (0-255).
- **Returns**: `int` (The RGB565 color value).

#### `clearPanelBuffer()`
Clears the screen buffer.
- **Returns**: `nil`

#### `drawPanelFace(int)`
Draws a specific face. This face must already be loaded beforehand.
- **Parameters**:
  - `faceId` (int): The ID of the face to draw.
- **Returns**: `nil`

#### `setPanelAnimation(frames int_array, duration, [repeat, [drop, [storage]]])`
If the panel is in managed mode, this will set an animation to run on it.
- **Parameters**:
  - `frames` (int array): The IDs of each frame in the animation.
  - `duration` (int): The duration of each frame in milliseconds.
  - `repeat` (int, optional): The number of times the animation should repeat. Default is `-1` (infinite).
  - `drop` (bool, optional): If `true`, all stacked animations will be erased, and this will be the only animation.
  - `storage` (int, optional): When defined, you can query this sotrage trough `getCurrentAnimationStorage()`. Can be useful when animations share the same frames
- **Returns**: `nil`

### `getCurrentAnimationStorage()`
Returns the ID of the current animation storage being used.
- **Returns**: `int` (The current animation storage ID).

#### `setPanelManaged(bomanagedol)`
Enables or disables managed mode. In managed mode, rendering is handled asynchronously, and you only need to define animations.
- **Parameters**:
  - `managed` (bool): `true` to enable managed mode, `false` to disable.
- **Returns**: `nil`

#### `isPanelManaged()`
Returns whether the panel is in managed mode.
- **Returns**: `bool` (`true` if managed mode is enabled, otherwise `false`).

#### `getPanelCurrentFace()`
Returns the current frame ID being displayed.
- **Returns**: `int` (The current frame ID).

#### `drawPanelCurrentFrame()`
Draws the current frame.
- **Returns**: `nil`

#### `getAnimationStackSize()`
Returns the number of animations currently stacked.
- **Returns**: `int` (The number of stacked animations).

#### `popPanelAnimation()`
Removes the current animation from the stack.
- **Returns**: `nil`

#### `setPanelBrightness(brightness)`
Sets the brightness of the panel.
- **Parameters**:
  - `brightness` (int): The brightness level (0-255, where 255 is 100%).
- **Returns**: `nil`


#### `gentlySetPanelBrightness(brightness, rate)`
Gradually adjusts the panel brightness to the specified level at the given rate. This function is useful for smooth transitions in brightness.
- **Parameters**:
  - `brightness` (int): The target brightness level (0-255).
  - `rate` (int): The speed at which the brightness changes (higher values mean slower transitions).
- **Returns**: `nil`

#### `setSpeakingFrames(frames, frameDuration)`
Sets the frames to display when the speak sensor is triggered.
- **Parameters**:
  - `frames` (int array): The IDs of the frames to display.
  - `frameDuration` (int): The duration of each frame in milliseconds.
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
- **Returns**: `int` (The frame offset).

#### `getFrameCountByName(name)`
Returns the amount of frames a given frame group has
- **Parameters**:
  - `name` (string): The frame name.
- **Returns**: `int` (The frame count).



## Image Decoding

### `decodePng(filename)`
Decodes a PNG image file from the SD card and returns the raw pixel data.
- **Parameters**:
  - `filename` (string): The path to the PNG file on the SD card.
- **Returns**: `uint16` table 

## Dictionary Functions
#### `dictGet(key)`
Gets a value from persistent dictionary storage.
- **Parameters**:
  - `key` (string): The key to lookup
- **Returns**: `string` (The stored value)

#### `dictSet(key, value)`
Sets a value in persistent dictionary storage.
- **Parameters**:
  - `key` (string): The key to store
  - `value` (string): The value to store
- **Returns**: `nil`

#### `dictDel(key)`
Deletes a key from persistent dictionary storage.
- **Parameters**:
  - `key` (string): The key to delete
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

#### `servoPause(servoId)`
Pauses the specified servo.
- **Parameters**:
  - `servoId` (int): The ID of the servo to pause.
- **Returns**: `bool` (`true` if the servo was paused successfully, otherwise `false`).

#### `servoResume(servoId)`
Resumes the specified servo.
- **Parameters**:
  - `servoId` (int): The ID of the servo to resume.
- **Returns**: `bool` (`true` if the servo was resumed successfully, otherwise `false`).

#### `servoMove(servoId, angle)`
Moves the specified servo to the given angle.
- **Parameters**:
  - `servoId` (int): The ID of the servo to move.
  - `angle` (float): The target angle for the servo.
- **Returns**: `bool` (`true` if the servo was moved successfully, otherwise `false`).

#### `hasServo()`
Checks if the system has a servo connected.
- **Returns**: `bool` (`true` if a servo is present, otherwise `false`).

## Internal screen


### `oledDrawPixel(x, y, color)`
Draws a single pixel on the OLED screen at the specified coordinates with the given color.
- **Parameters**:
  - `x` (int): The X coordinate.
  - `y` (int): The Y coordinate.
  - `color` (int): The color to draw (1 for white, 0 for black).
- **Returns**: `nil`

### `oledDrawBottomBar()`
Draws the bottom bar
- **Returns**: `nil`

### `oledSetTextColor(fg[, bg])`
Sets the text color for drawing on the OLED screen.
- **Parameters**:
  - `fg` (int): The foreground color (1 for white, 0 for black).
  - `bg` (int, optional): The background color (1 for white, 0 for black).
- **Returns**: `nil`

### `oledSetFontSize(size)`
Sets the text size, default is 1
- **Parameters**:
  - `size` (int): Default is 1
- **Returns**: `nil`

#### `oledSetCursor(x, y)`
Sets the cursor position on the OLED screen.
- **Parameters**:
  - `x` (int): The X coordinate.
  - `y` (int): The Y coordinate.
- **Returns**: `nil`

#### `oledFaceToScreen(x, y)`
Draws the current image being displayed on the outer panels to the internal screen at the specified position.
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

#### `oledDrawText(msg string)`
Draws text at the current cursor position.
- **Parameters**:
  - `msg` (string): The text to draw.
- **Returns**: `nil`

#### `oledDrawRect(x, y, width, height, color)`
Draws a rectangle on the OLED screen.
- **Parameters**:
  - `x` (int): The X coordinate.
  - `y` (int): The Y coordinate.
  - `width` (int): The width of the rectangle.
  - `height` (int): The height of the rectangle.
  - `color` (int): The color to draw (1 for white, 0 for black).
- **Returns**: `nil`

#### `oledCreateIcon(width, height, data)`
Creates an icon from the provided data and returns its ID.
- **Parameters**:
  - `width` (int): The width of the icon.
  - `height` (int): The height of the icon.
  - `data` (int array): The binary data for the icon.
- **Returns**: `int` (The icon ID).

#### `oledDrawIcon(x, y, iconId)`
Draws an icon at the specified position.
- **Parameters**:
  - `x` (int): The X coordinate.
  - `y` (int): The Y coordinate.
  - `iconId` (int): The ID of the icon to draw.
- **Returns**: `nil`

## Led strips

#### `ledsBegin(led_count, max_brightness])`
Initializes the LED strip with the specified number of LEDs and optional maximum brightness.
- **Parameters**:
  - `led_count` (int): The number of LEDs in the strip.
  - `max_brightness` (int, optional): The maximum brightness level (0-255). Default is 128.
- **Returns**: `bool` (`true` if successful, otherwise `false`).

#### `ledsBeginDual(led_count, led_count2, max_brightness])`
Initializes two individual leds strips.
- **Parameters**:
  - `led_count` (int): The number of LEDs in the strip.
  - `led_count2` (int): The number of LEDs in the second strip.
  - `max_brightness` (int, optional): The maximum brightness level (0-255). Default is 128.
- **Returns**: `bool` (`true` if successful, otherwise `false`).

#### `ledsIsManaged()`
Check if the leds are in managed mode
- **Returns**: `bool`.

#### `ledsGentlySeBrightness(brightness, [rate, [startAmount]])`
Slowly ramp up the brightness to a certain value
- **Parameters**:
  - `brightness` (int): Max brightness to be ramped up
  - `led_count2` (int, optional): How many units of brightness to be increased every frame. Default is 1
  - `max_brightness` (int, optional): Starting brigtness. Default is 0

#### `ledsStackCurrentBehavior()`
Store current led behavior state in a stack. Is useful to st temporary behaviors
- **Returns**: `int`

#### `ledsPopBehavior()`
Update current led behavior with the stored behavior on the stack.
- **Returns**: `int`

#### `ledsSegmentRange(id int, from int, to int)`
Defines a segment of the LED strip for independent control.
- **Parameters**:
  - `id` (int): The segment ID (0-15).
  - `from` (int): The starting LED index.
  - `to` (int): The ending LED index.
- **Returns**: `nil`

#### `ledsSetManaged(managed)`
Enables or disabled managed mode
- **Returns**: `nil`

#### `ledsSetBrightness(brightness)`
- **Parameters**:
  - `id` (int): Brightness (0-255).
- **Returns**: `nil`

#### `ledsGetBrightness()`
- **Returns**: `int`

#### `ledsDisplay()`
Make the changes on the leds to be displayed on the strip. To be used when managed mode is disabled
- **Returns**: `int`

#### `ledsSegmentBehavior(id, behavior, [parameter, parameter2, parameter3, parameter4])`
Sets the behavior for a specific LED segment.
- **Parameters**:
  - `id` (int): The segment ID (0-15).
  - `behavior` (int): The behavior to set (e.g., `BEHAVIOR_PRIDE`, `BEHAVIOR_ROTATE`, etc.).
  - `parameter` (int, optional): Additional parameter for the behavior.
  - `parameter2` (int, optional): Additional parameter for the behavior.
  - `parameter3` (int, optional): Additional parameter for the behavior.
  - `parameter4` (int, optional): Additional parameter for the behavior.
- **Returns**: `nil`

#### `ledsSegmentTweenBehavior(id, behavior, [parameter, parameter2, parameter3, parameter4])`
Sets a tween behavior for a specific LED segment, transitioning smoothly to the new behavior.
- **Parameters**:
  - `id` (int): The segment ID (0-15).
  - `behavior` (int): The behavior to transition to (e.g., `BEHAVIOR_PRIDE`, `BEHAVIOR_ROTATE`, etc.).
  - `parameter` (int, optional): Additional parameter for the behavior.
  - `parameter2` (int, optional): Additional parameter for the behavior.
  - `parameter3` (int, optional): Additional parameter for the behavior.
  - `parameter4` (int, optional): Additional parameter for the behavior.
- **Returns**: `nil`

#### `ledsSegmentTweenSpeed(id, speed)`
Sets the tween speed for a specific LED segment.
- **Parameters**:
  - `id` (int): The segment ID (0-15).
  - `speed` (int): The tween speed (higher values mean slower transitions).
- **Returns**: `nil`

#### `ledsSetColor(id, r, g, b)`
Sets the color of a individual LED. Ideally not to be used while on managed mode or when behavior is set to none
- **Parameters**:
  - `id` (int): individual led
  - `r` (int):Color red (0-255).
  - `g` (int):Color green (0-255).
  - `b` (int):Color blue (0-255).
- **Returns**: `nil`

#### `ledsSegmentColor(id, r, g, b)`
Sets the color of a LED group.  Ideally not to be used while on managed mode or when behavior is set to none
- **Parameters**:
  - `id` (int): led segment
  - `r` (int):Color red (0-255).
  - `g` (int):Color green (0-255).
  - `b` (int):Color blue (0-255).
- **Returns**: `nil`


## Arduino Core

#### `tone(frequency)`
Generates tone on specified pin.
- **Parameters**:
  - `frequency` (int): Tone frequency in Hz
- **Returns**: `nil`

#### `toneDuration(frequency, duration)`
Generates tone with duration.
- **Parameters**:
  - `frequency` (int): Tone frequency in Hz
  - `duration` (int): Duration in milliseconds
- **Returns**: `nil`

#### `noTone()`
Stops tone generation.
- **Returns**: `nil`

#### `millis()`
Returns the number of milliseconds the system has been running.
- **Returns**: `int` (The number of milliseconds).

#### `delayMicroseconds(us)`
Delays execution for the specified number of microseconds.
- **Parameters**:
  - `us` (int): The number of microseconds to delay.
- **Returns**: `nil`

#### `delay(ms)`
Delays execution for the specified number of milliseconds.
- **Parameters**:
  - `ms` (int): The number of milliseconds to delay.
- **Returns**: `nil`

#### `digitalWrite(pin, value)`
Writes a digital value to a pin.
- **Parameters**:
  - `pin` (int): The pin number.
  - `value` (int): The value to write (`HIGH` or `LOW`).
- **Returns**: `nil`

#### `analogRead(pin)`
Reads an analog value from a pin.
- **Parameters**:
  - `pin` (int): The pin number.
- **Returns**: `int` (The analog value).

#### `pinMode(pin, mode)`
Sets the mode of a pin (e.g., `INPUT`, `OUTPUT`).
- **Parameters**:
  - `pin` (int): The pin number.
  - `mode` (int): The mode to set.
- **Returns**: `nil`

#### `digitalRead(pin)`
Reads a digital value from a pin.
- **Parameters**:
  - `pin` (int): The pin number.
- **Returns**: `int` (The digital value, `HIGH` or `LOW`).

### Serial Communication
SerialIo means using the pins io1 and io2 as tx and rx
#### `serialWriteString(data: string)`
Writes a string to the primary serial port.

- **Parameters**:
  - `data` (string): The string to write to the serial port.
- **Returns**: `nil`

#### `serialIoWriteString(data: string)`
Writes a string to the secondary serial port.

- **Parameters**:
  - `data` (string): The string to write to the secondary serial port.
- **Returns**: `nil`

#### `serialIoAvailableForWrite()`
Returns the number of bytes available for writing in the secondary serial port.

- **Returns**: `int` (The number of bytes available for writing).

#### `serialAvailableForWrite()`
Returns the number of bytes available for writing in the primary serial port.

- **Returns**: `int` (The number of bytes available for writing).


### I2C Communication

#### `wireAvailable()`
Returns the number of bytes available for reading from the I2C bus.

- **Returns**: `int` (The number of bytes available for reading).

#### `wireBegin(addr: uint8_t)`
Initializes the I2C bus with the specified address.

- **Parameters**:
  - `addr` (uint8_t): The I2C address of the device.
- **Returns**: `bool` (`true` if successful, otherwise `false`).

#### `wireFlush()`
Flushes the I2C buffer.

- **Returns**: `nil`

#### `wireBeginTransmission(addr: uint8_t)`
Begins a transmission to the specified I2C address.

- **Parameters**:
  - `addr` (uint8_t): The I2C address of the device.
- **Returns**: `nil`

#### `wireEndTransmission(sendStop: bool)`
Ends the I2C transmission.

- **Parameters**:
  - `sendStop` (bool): If `true`, a stop condition is sent after the transmission.
- **Returns**: `uint8_t` (The status of the transmission).

#### `wireRead()`
Reads a byte from the I2C bus.

- **Returns**: `int` (The byte read from the I2C bus).

#### `wireReadBytes(length: int)`
Reads a specified number of bytes from the I2C bus.

- **Parameters**:
  - `length` (int): The number of bytes to read.
- **Returns**: `std::vector<uint8_t>` (A vector containing the bytes read).

#### `wireRequestFrom(address: uint16_t, size: size_t, sendStop: bool)`
Requests data from a specified I2C address.

- **Parameters**:
  - `address` (uint16_t): The I2C address of the device.
  - `size` (size_t): The number of bytes to request.
  - `sendStop` (bool): If `true`, a stop condition is sent after the request.
- **Returns**: `uint8_t` (The number of bytes received).

#### `wirePeek()`
Peeks at the next byte in the I2C buffer without removing it.

- **Returns**: `int` (The next byte in the buffer).

#### `wireParseFloat()`
Parses a float from the I2C buffer.

- **Returns**: `float` (The parsed float value).

#### `wireParseInt()`
Parses an integer from the I2C buffer.

- **Returns**: `int` (The parsed integer value).

#### `wireSetTimeout(timeout: uint32_t)`
Sets the timeout for I2C operations.

- **Parameters**:
  - `timeout` (uint32_t): The timeout in milliseconds.
- **Returns**: `nil`

#### `wireGetTimeout()`
Returns the current timeout for I2C operations.

- **Returns**: `uint32_t` (The timeout in milliseconds).


### Secondary Serial (IO1/IO2)

#### `beginSerialIo(baud)`
Initializes the secondary serial port (IO1/IO2 pins).
- **Parameters**:
  - `baud` (int, optional): Baud rate (default: 115200)
- **Returns**: `nil`

#### `serialIoAvailable()`
Returns the number of bytes available to read from the secondary serial port.
- **Returns**: `int` (Number of bytes available)

#### `serialIoRead()`
Reads one byte from the secondary serial port.
- **Returns**: `int` (The byte read, or -1 if none available)

#### `serialIoReadStringUntil(terminator)`
Reads characters from the secondary serial port until the terminator is found.
- **Parameters**:
  - `terminator` (char, optional): The terminating character (default: '\n')
- **Returns**: `string` (The read string)

#### `serialIoAvailableForWrite()`
Returns the number of bytes that can be written without blocking to secondary serial.
- **Returns**: `int` (Available space in output buffer)

#### `serialIoWrite(data)`
Writes a single byte to the secondary serial port.
- **Parameters**:
  - `data` (int): The byte to write (0-255)
- **Returns**: `int` (Number of bytes written)

#### `serialIoWriteString(data)`
Writes a string to the secondary serial port.
- **Parameters**:
  - `data` (string): The string to write
- **Returns**: `int` (Number of bytes written)


### Primary Serial (USB/Console)

#### `serialAvailable()`
Returns the number of bytes available to read from the primary serial port.
- **Returns**: `int` (Number of bytes available)

#### `serialRead()`
Reads one byte from the primary serial port.
- **Returns**: `int` (The byte read, or -1 if none available)

#### `serialReadStringUntil(terminator)`
Reads characters from the primary serial port until the terminator is found.
- **Parameters**:
  - `terminator` (char, optional): The terminating character (default: '\n')
- **Returns**: `string` (The read string)

#### `serialAvailableForWrite()`
Returns the number of bytes that can be written without blocking.
- **Returns**: `int` (Available space in output buffer)

#### `serialWrite(data)`
Writes a single byte to the primary serial port.
- **Parameters**:
  - `data` (int): The byte to write (0-255)
- **Returns**: `int` (Number of bytes written)

#### `serialWriteString(data)`
Writes a string to the primary serial port.
- **Parameters**:
  - `data` (string): The string to write
- **Returns**: `int` (Number of bytes written)

### Secondary Serial (IO1/IO2)

#### `beginSerialIo(baud)`
Initializes the secondary serial port (IO1/IO2 pins).
- **Parameters**:
  - `baud` (int, optional): Baud rate (default: 115200)
- **Returns**: `nil`

#### `serialIoAvailable()`
Returns the number of bytes available to read from the secondary serial port.
- **Returns**: `int` (Number of bytes available)

#### `serialIoRead()`
Reads one byte from the secondary serial port.
- **Returns**: `int` (The byte read, or -1 if none available)

#### `serialIoReadStringUntil(terminator)`
Reads characters from the secondary serial port until the terminator is found.
- **Parameters**:
  - `terminator` (char, optional): The terminating character (default: '\n')
- **Returns**: `string` (The read string)

#### `serialIoAvailableForWrite()`
Returns the number of bytes that can be written without blocking to secondary serial.
- **Returns**: `int` (Available space in output buffer)

#### `serialIoWrite(data)`
Writes a single byte to the secondary serial port.
- **Parameters**:
  - `data` (int): The byte to write (0-255)
- **Returns**: `int` (Number of bytes written)

#### `serialIoWriteString(data)`
Writes a string to the secondary serial port.
- **Parameters**:
  - `data` (string): The string to write
- **Returns**: `int` (Number of bytes written)

## I2C Communication

#### `wireBegin(address)`
Initializes I2C communication as a master or slave.
- **Parameters**:
  - `address` (uint8_t): 7-bit slave address (0 for master mode)
- **Returns**: `bool` (true if successful)

#### `wireAvailable()`
Returns the number of bytes available for reading.
- **Returns**: `int` (Number of bytes available)

#### `wireBeginTransmission(address)`
Begins a transmission to the specified I2C device.
- **Parameters**:
  - `address` (uint8_t): 7-bit device address
- **Returns**: `nil`

#### `wireEndTransmission([sendStop])`
Ends a transmission to the I2C device.
- **Parameters**:
  - `sendStop` (bool, optional): Whether to send stop condition (default: true)
- **Returns**: `uint8_t` (Transmission status code)

#### `wireRequestFrom(address, size, [sendStop])`
Requests bytes from an I2C slave device.
- **Parameters**:
  - `address` (uint16_t): 7-bit device address
  - `size` (size_t): Number of bytes to request
  - `sendStop` (bool, optional): Whether to send stop condition (default: true)
- **Returns**: `uint8_t` (Number of bytes received)

#### `wireRead()`
Reads one byte from the I2C buffer.
- **Returns**: `int` (The byte read)

#### `wireReadBytes(length)`
Reads multiple bytes from the I2C buffer.
- **Parameters**:
  - `length` (int): Number of bytes to read
- **Returns**: `table` (Array of bytes read)

#### `wirePeek()`
Peeks at the next byte in the I2C buffer without removing it.
- **Returns**: `int` (The next byte)

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
  - `timeout` (uint32_t): Timeout in milliseconds
- **Returns**: `nil`

#### `wireGetTimeout()`
Gets the current I2C operation timeout.
- **Returns**: `uint32_t` (Timeout in milliseconds)

# Lua Constants

- [Engine Related](#engine-related)
- [Input](#input)
- [LED Behavior](#led-behavior)
- [Pins](#pins)
- [ESP32 Reset Reason](#esp32-reset-reason)

## Engine related

- `PANDA_VERSION`: A string constant representing the current version of the Protopanda firmware.
- `VCC_THRESHOLD_START`: The minimum voltage threshold required for the system to start.
- `VCC_THRESHOLD_HALT`: The minimum voltage threshold below which the system will halt to prevent damage.
- `OLED_SCREEN_WIDTH`: The width of the internal OLED screen in pixels.
- `OLED_SCREEN_HEIGHT`: The height of the internal OLED screen in pixels.
- `PANEL_WIDTH`: The width of the HUB75 panel in pixels.
- `PANEL_HEIGHT`: The height of the HUB75 panel in pixels.
- `POWER_MODE_USB_5V`: Power mode for USB 5V input.
- `POWER_MODE_USB_9V`: Power mode for USB 9V input.
- `POWER_MODE_BATTERY`: Power mode for battery input.
- `MAX_BLE_BUTTONS`:  Number of buttons per BLE device.
- `MAX_BLE_CLIENTS`: Maximum number of connected BLE devices.
- `SERVO_COUNT`: The number of servos.

## Input

- `BUTTON_RELEASED`: Indicates that a button is in the released state (not pressed).
- `BUTTON_JUST_PRESSED`: Indicates that a button has just been pressed (transition from released to pressed).
- `BUTTON_PRESSED`: Indicates that a button is currently pressed.
- `BUTTON_JUST_RELEASED`: Indicates that a button has just been released (transition from pressed to released).
- `DEVICE_X_BUTTON_LEFT`: Button mapping for left button on device (0 to `MAX_BLE_CLIENTS`).
- `DEVICE_X_BUTTON_RIGHT`: Button mapping for right button on device (0 to  `MAX_BLE_CLIENTS`).
- `DEVICE_X_BUTTON_UP`: Button mapping for up button on device (0 to `MAX_BLE_CLIENTS`).
- `DEVICE_X_BUTTON_DOWN`: Button mapping for down button on device (0 to `MAX_BLE_CLIENTS`).
- `DEVICE_X_BUTTON_CONFIRM`: Button mapping for confirm button on device (0 to `MAX_BLE_CLIENTS`).
- `DEVICE_X_BUTTON_AUX_A`: Button mapping for auxiliar function A button on device (0 to `MAX_BLE_CLIENTS`).
- `DEVICE_X_BUTTON_AUX_B`: Button mapping for auxiliar function B button on device (0 to `MAX_BLE_CLIENTS`).
- `DEVICE_X_BUTTON_BACK`: Button mapping for back button on device (0 to `MAX_BLE_CLIENTS`).
- `BUTTON_LEFT`: Same as DEVICE_0_BUTTON_LEFT
- `BUTTON_RIGHT`: Same as DEVICE_0_BUTTON_RIGHT
- `BUTTON_UP`: Same as DEVICE_0_BUTTON_UP
- `BUTTON_CONFIRM`: Same as DEVICE_0_BUTTON_CONFIRM
- `BUTTON_DOWN`: Same as DEVICE_0_BUTTON_DOWN
- `BUTTON_AUX_A`: Same as DEVICE_0_BUTTON_AUX_A
- `BUTTON_AUX_B`: Same as DEVICE_0_BUTTON_AUX_B
- `BUTTON_BACK`: Same as DEVICE_0_BUTTON_BACK



## Led Behavior

- `BEHAVIOR_PRIDE`: Displays a rainbow color pattern on the LED strip.
- `BEHAVIOR_ROTATE`: Rotates colors along the LED strip.
- `BEHAVIOR_RANDOM_COLOR`: Randomly changes the color of LEDs.
- `BEHAVIOR_FADE_CYCLE`: Cycles through colors with a fading effect.
- `BEHAVIOR_ROTATE_FADE_CYCLE`: Combines rotation and fading effects for a dynamic display.
- `BEHAVIOR_STATIC_RGB`: Sets a static RGB color for the LEDs.
- `BEHAVIOR_STATIC_HSV`: Sets a static HSV color for the LEDs.
- `BEHAVIOR_RANDOM_BLINK`: Randomly blinks LEDs on and off.
- `BEHAVIOR_ROTATE_SINE_V`: Rotates colors with a sine wave effect on the V (value) component of HSV.
- `BEHAVIOR_ROTATE_SINE_S`: Rotates colors with a sine wave effect on the S (saturation) component of HSV.
- `BEHAVIOR_ROTATE_SINE_H`: Rotates colors with a sine wave effect on the H (hue) component of HSV.
- `BEHAVIOR_FADE_IN`: Gradually fades in the LEDs from off to a specified color.
- `MAX_LED_GROUPS`: Number os avaliable led groups

## Pins
- `D1`, `D2`: The external io, io1 and io2
- `HIGH`, `LOW`: Constants representing high and low states for digital pins.
- `INPUT`, `OUTPUT`: Constants representing pin modes.
- `INPUT_PULLUP`, `INPUT_PULLDOWN`: Constants representing pull-up and pull-down modes for input pins.
- `ANALOG`: Constant representing analog pin mode.
- `OUTPUT_OPEN_DRAIN`, `OPEN_DRAIN`: Constants representing open-drain output mode.
- `PULLDOWN`: Constant representing pull-down mode.

## ESP32 reset reason
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

#### Power Modes
- `BUILT_IN_POWER_MODE`

# Led behaviors

* `BEHAVIOR_PRIDE`
  - **Description**: A rainbow effect that cycles through colors in a smooth, pride-flag-like pattern.
  - **Parameters**: None.

* `BEHAVIOR_ROTATE`
  - **Description**: Rotates a single color through the segment, creating a moving light effect.
  - **Parameters**: 
    - `1`: Red (0-255).
    - `2`: Green (0-255).
    - `3`: Blue (0-255).
    - `4`: Speed of rotation (delay in milliseconds).

* `BEHAVIOR_RANDOM_COLOR`
  - **Description**: Sets each LED in the segment to a random color.
  - **Parameters**: None.

* `BEHAVIOR_FADE_CYCLE`
  - **Description**: Fades the brightness of the LEDs in the segment up and down in a cycle.
  - **Parameters**: 
    - `1`: Hue value (0-255).
    - `2`: Controls the speed of the fade cycle.
    - `3`: Minimum brightness value.

* `BEHAVIOR_ROTATE_FADE_CYCLE`
  - **Description**: Combines rotation and fading, creating a moving light effect with fading brightness.
  - **Parameters**: 
    - `1`: Hue value (0-255).
    - `2`: Controls the speed of the fade cycle.
    - `3`: Minimum brightness value.
    - `4`: Speed of rotation (delay in milliseconds).

* `BEHAVIOR_STATIC_RGB`
  - **Description**: Sets all LEDs in the segment to a static RGB color.
  - **Parameters**: 
    - `1`: Red value (0-255).
    - `2`: Green value (0-255).
    - `3`: Blue value (0-255).

* `BEHAVIOR_STATIC_HSV`
  - **Description**: Sets all LEDs in the segment to a static HSV color.
  - **Parameters**: 
    - `1`: Hue value (0-255).
    - `2`: Saturation value (0-255).
    - `3`: Brightness value (0-255).

* `BEHAVIOR_RANDOM_BLINK`
  - **Description**: Randomly blinks LEDs in the segment with random colors and brightness.
  - **Parameters**: 
    - `1`: Base hue value (0-255).
    - `2`: Range of hue variation.
    - `3`: Maximum brightness value.
    - `4`: Delay between blinks (in milliseconds).

* `BEHAVIOR_ROTATE_SINE_V`
  - **Description**: Rotates a sine wave effect through the segment, varying brightness.
  - **Parameters**: 
    - `1`: Base hue value (0-255).
    - `2`: Base brightness value (0-255).
    - `3`: Controls the speed of the sine wave.

* `BEHAVIOR_ROTATE_SINE_S`
  - **Description**: Rotates a sine wave effect through the segment, varying saturation.
  - **Parameters**: 
    - `1`: Base hue value (0-255).
    - `2`: Base brightness value (0-255).
    - `3`: Controls the speed of the sine wave.

* `BEHAVIOR_ROTATE_SINE_H`
  - **Description**: Rotates a sine wave effect through the segment, varying hue.
  - **Parameters**: 
    - `1`: Base saturation value (0-255).
    - `2`: Base brightness value (0-255).
    - `3`: Controls the speed of the sine wave.

* `BEHAVIOR_FADE_IN`
  - **Description**: Gradually fades in the LEDs in the segment to a specified HSV color.
  - **Parameters**: 
    - `1`: Hue value (0-255).
    - `2`: Saturation value (0-255).
    - `3`: Controls the speed of the fade-in.
    - `4`: Delay between fade steps (in milliseconds).

* `BEHAVIOR_NONE`
  - **Description**: No behavior is applied. The LEDs in the segment will remain off or unchanged.
  - **Parameters**: None.


# Bluetooth Interface


## Radio and BLE functions

#### `startBLE()`
Starts the envoriment for, but dont start the radio yet. Bluetooth Low Energy (BLE).
- **Returns**: `bool` (`true` if successful, otherwise `false`).

#### `startBLERadio()`
Starts the radio.
- **Returns**: `bool` (`true` if successful, otherwise `false`).

#### `getConnectedRemoteControls()`
Returns the number of connected remote controls.
- **Returns**: `int` (The number of connected devices).

#### `isElementIdConnected(id)`
Checks if a remote control with the given ID is connected.
- **Parameters**:
  - `id` (int): The ID of the remote control.
- **Returns**: `bool` (`true` if connected, otherwise `false`).

#### `beginBleScanning()`
Starts scanning for BLE devices.
- **Returns**: `nil`

#### `setLogDiscoveredBleDevices(bool)`  
When true, each scanned device will be saved on the log filew
- **Retorna**: `nil`  

#### `setMaximumControls(count)`
Sets the maximum number of connected remote controls.
- **Parameters**:
  - `count` (int): The maximum number of devices.
- **Returns**: `nil`

#### `getCharacteristicsFromService(connectionId, uuid, refresh)`
Get all characteristics a certain service has
- **Parameters**:
  - `connectionId` (int): connection id.
  - `uuid` (string): uuid.
  - `refresh` (bool): refresh information?
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
First we create a service handler and we pass a UUID. That specific uuid is from a Human Interface Device generic. So any device that has mouse/keyboard/josystick capabilities will have thi service on them.
The moment we register it, any scanned device that is advertising and have this specific service UUID will be attempted to connect.

The next function we create a callback when the device is connected. Here's an example of the callback:
```lua
function drivers.onConnectHID(connectionId, controllerId, address, name)
    log("Connected conId="..connectionId.." controller="..controllerId.." addr=\""..address.."\" name=["..name.."]")
end
```
The connectionId can be used to get information about the device. It is unique for each connection. 
ControllerId is the ID used by the protopanda. The first device connected will have id 0, and the second one is 1. But the moment one of them disconnect, that ID will be free'd, so next connection will be able to reuse the same ID.
Address is just a string with the address and name is the name if present.

Same goes to `SetOnDisconnectCallback`, but instead of address and name, we get the reason of it beeing disconnected.
```lua
function drivers.onDisconnectHID(connectionId, controllerId, reason)
    log("Disconnected "..connectionId.." due ".. reason)
end
``` 

Once we defined those callbacks (opitional), we can attach a listener to each of the characteristics we want:
```lua
drivers.mouseListener = drivers.mouseHandler:AddCharacteristics("2a4d")
drivers.mouseListener:SetSubscribeCallback(drivers.onMouseCallback) 
drivers.mouseListener:SetRequired(true)
drivers.mouseListener:SetCallbackModeStream(false)
```
Once the charcteristics is defined, we can set it as  a required (if not present, the connection is dropped) and a callback if there is a subscribe in the service.
The callback goes as follow:
```lua
function drivers.onMouseCallback(connectionId, controllerId, data)
--handling
end
```

There we can find the connectionId, the controllerId and a array containing the data. The data will come as valeus from 0 to 255

## Handling BLE functions

### `BleServiceHandler(uuid)`
Create a handler object. Try to keep this object in to a global or stored envoriment. Because GC wont delete the cpp reference but it will do the lua reference.
- **Parameters**:
  - `uuid` (string): uuid.
- **Returns**: `BleServiceHandlerObject`

### `BleServiceHandler::ReadFromCharacteristics(clientId, characteristicsUuid)`
Reads data from a certain client and certain characteristics.
- **Parameters**:
  - `clientId` (int): connectionId.
  - `uuid` (string): can be the 16bit uuid.
- **Returns**: `int array`

### `BleServiceHandler::GetServices(clientId)`
Get avaliable services in client
- **Parameters**:
  - `clientId` (int): connectionId.
- **Returns**: `string array`

### `BleServiceHandler::AddAddressRequired(address)`
When set for the first time, it enforces the connection to accept only if the address was previously inserted.
Example would be:
```lua
Handler = BleServiceHandler("00001812-0000-1000-8000-00805f9b34fb")
Handler:AddAddressRequired("AA:BB:CC:DD:EE")
Handler:AddAddressRequired("FF:00:11:22:3")
```
That way, only devices with the specific service and those two specific addresses can connect.
If both `AddAddressRequired` and `AddNameRequired` defined. The condition is an "and". Need to match any of the address and any of the name
- **Parameters**:
  - `address` (string): address.

### `BleServiceHandler::AddNameRequired(name)`
When set for the first time, it enforces the connection to accept only if the name was previously defined
Example would be:
```lua
Handler = BleServiceHandler("00001812-0000-1000-8000-00805f9b34fb")
Handler:AddNameRequired("VR-PARK")
Handler:AddNameRequired("Beauty-r1")
```
That way, only devices with the specific service and those two specific names can connect.
If both `AddAddressRequired` and `AddNameRequired` defined. The condition is an "and". Need to match any of the address and any of the name
- **Parameters**:
  - `name` (string): address.


### `BleServiceHandler::GetCharacteristics(clientId)`
Get avaliable characteristics in current service
- **Parameters**:
  - `clientId` (int): connectionId.
- **Returns**: `string array`

### `BleServiceHandler::WriteToCharacteristics(message, clientId, characteristics[, reply])`
Write a message to the characteristics specified
- **Parameters**:
  - `message` (int array): messaage bytes.
  - `clientId` (int): connectionId.
  - `uuid` (string): can be the 16bit uuid.
  - `reply` (bool): wait for reply.
- **Returns**: `bool`

### `BleServiceHandler::SetOnDisconnectCallback(callback)`
Define a callback to when a device disconnects
- **Parameters**:
  - `callback` (lua function): callback.

### `BleServiceHandler::SetOnConnectCallback(callback)`
Define a callback to when a device connects
- **Parameters**:
  - `callback` (lua function): callback.

### `BleServiceHandler::AddCharacteristics(callback)`
Returns a handler for the specified characteristics
- **Parameters**:
  - `uuid` (string): can be the 16bit uuid.
- **Returns**: `BleCharacteristicsHandlerObject`

### `BleCharacteristicsHandler::SetSubscribeCallback(callback)`
Returns a handler for when a message oif the specified characterists arrive
- **Parameters**:
  - `uuid` (string): can be the 16bit uuid.

### `BleCharacteristicsHandler::SetCallbackModeStream(stream)`
When this is enabled, each message is handled on each loop. This way, messages can pile up, but one message per loop. When false, all messages are processed on the loop until the queue is empty.
- **Parameters**:
  - `stream` (bool): true or false

### `BleCharacteristicsHandler::SetRequired(req)`
If set as required, when the device connects and there is not the characteristics specificed present, a disconnect is issued.
- **Parameters**:
  - `req` (bool): true or false