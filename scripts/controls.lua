local _M = {
	shouldStop = false,
	VERSION_REQUIRED="1.0.3",
}

local map = require("generic").map
local input = require("input")

function _M.onSetup()
	setPanelManaged(false)
	ledsSetManaged(false)
end

function _M.onLoop(dt)
	clearPanelBuffer()
	oledClearScreen()

	local hasPressed = false

	for i=DEVICE_0_BUTTON_LEFT, DEVICE_0_BUTTON_CONFIRM do
		oledSetCursor(0, 0)
		oledDrawText("Control 1:")
		oledDrawRect((i-DEVICE_0_BUTTON_LEFT)*9, 10, 8, 8, 1)
		drawPanelChar(64, 0, string.byte('1'), color565(255,255,255), color565(0,0,0), 1)
		drawPanelRect(64+(i-DEVICE_0_BUTTON_LEFT)*9 + 8, 0, 8, 8, color565(255,255,255))
		local color = 0
		if input.readButtonStatus(i) == BUTTON_PRESSED or input.readButtonStatus(i) == BUTTON_JUST_PRESSED then
			color = 1
			hasPressed = true
			drawPanelFillRect(64+(i-DEVICE_0_BUTTON_LEFT)*9 + 8, 0, 8, 8, color565(255,255,255))
		end
		oledDrawFilledRect((i-DEVICE_0_BUTTON_LEFT)*9+1, 11, 6, 6, color)
	end
	if hasPressed then
		ledsSegmentColor(0, 100, 100, 100)
	else
		ledsSegmentColor(0, 0, 0, 0)
	end
	hasPressed = false

	for i=DEVICE_1_BUTTON_LEFT, DEVICE_1_BUTTON_CONFIRM do
		oledSetCursor(0, 18)
		oledDrawText("Control 2:")
		oledDrawRect((i-DEVICE_1_BUTTON_LEFT)*9, 26, 8, 8, 1)
		drawPanelChar(64, 10, string.byte('2'), color565(255,255,255), color565(0,0,0), 1)
		drawPanelRect(64+(i-DEVICE_1_BUTTON_LEFT)*9 + 8, 10, 8, 8, color565(255,255,255))
		local color = 0
		if input.readButtonStatus(i) == BUTTON_PRESSED or input.readButtonStatus(i) == BUTTON_JUST_PRESSED then
			color = 1
			hasPressed = true
			drawPanelFillRect(64+(i-DEVICE_1_BUTTON_LEFT)*9 + 8, 10, 8, 8, color565(255,255,255))
		end
		oledDrawFilledRect((i-DEVICE_1_BUTTON_LEFT)*9+1, 27, 6, 6, color)
	end
	if hasPressed then
		ledsSegmentColor(1, 100, 100, 100)
	else
		ledsSegmentColor(1, 0, 0, 0)
	end

	drawPanelRect(0, 0, 16, 16, color565(255,255,255))
	drawPanelChar(0, 17, string.byte('a'), color565(255,255,255), color565(0,0,0), 1)
	drawPanelChar(8, 17, string.byte('1'), color565(255,255,255), color565(0,0,0), 1)
	drawPanelLine(8, 7, 8 + 3.5*input.readAccelerometerX(0), 7, color565(255,0,0))
	drawPanelLine(8, 7, 8 , 7 + 3.5*input.readAccelerometerY(0), color565(0,255,0))
	local vz = input.readAccelerometerZ(0)
	drawPanelLine(8, 7, 8 + 3.5*vz , 7+ 3.5*vz, color565(0,0,255))
	drawPanelLine(8, 29, 7+4*input.readAccelerometerX(0), 29, color565(255,0,0))
	drawPanelLine(8, 30, 7+4*input.readAccelerometerY(0), 30, color565(0,255,0))
	drawPanelLine(8, 31, 7+4*input.readAccelerometerZ(0), 31, color565(0,0,255))


	drawPanelRect(16, 0, 16, 16, color565(255,255,255))
	drawPanelChar(16, 17, string.byte('a'), color565(255,255,255), color565(0,0,0), 1)
	drawPanelChar(16+8, 17, string.byte('2'), color565(255,255,255), color565(0,0,0), 1)
	drawPanelLine(16+8, 7, 16+8 + 3.5*input.readAccelerometerX(1), 7, color565(255,0,0))
	drawPanelLine(16+8, 7, 16+8 , 7 + 3.5*input.readAccelerometerY(1), color565(0,255,0))
	local vz = input.readAccelerometerZ(1)
	drawPanelLine(16+8, 7, 16+8 + 3.5*vz , 7+ 3.5*vz, color565(0,0,255))
	drawPanelLine(16+8, 29, 16 + 7+4*input.readAccelerometerX(1), 29, color565(255,0,0))
	drawPanelLine(16+8, 30, 16 + 7+4*input.readAccelerometerY(1), 30, color565(0,255,0))
	drawPanelLine(16+8, 31, 16 + 7+4*input.readAccelerometerZ(1), 31, color565(0,0,255))


	drawPanelRect(32, 0, 16, 16, color565(255,255,255))
	drawPanelChar(32, 17, string.byte('g'), color565(255,255,255), color565(0,0,0), 1)
	drawPanelChar(32+8, 17, string.byte('1'), color565(255,255,255), color565(0,0,0), 1)
	local gx = map(input.readGyroX(0), -12000, 12000, -7, 7)
	local gy = map(input.readGyroY(0), -12000, 12000, -7, 7)
	local gz = map(input.readGyroZ(0), -12000, 12000, -7, 7)
	drawPanelLine(32+8, 7, 32+8 + gx, 7, color565(0,255,255))
	drawPanelLine(32+8, 7, 32+8 , 7 + gy, color565(255,0,255))
	drawPanelLine(32+8, 7, 32+8 + gz , 7+ gz, color565(255,255,0))
	drawPanelLine(32+8, 29, 32 + 7+gx, 29, color565(0,255,255))
	drawPanelLine(32+8, 30, 32 + 7+gy, 30, color565(255,0,255))
	drawPanelLine(32+8, 31, 32 + 7+gz, 31, color565(255,255,0))


	drawPanelRect(48, 0, 16, 16, color565(255,255,255))
	drawPanelChar(48, 17, string.byte('g'), color565(255,255,255), color565(0,0,0), 1)
	drawPanelChar(48+8, 17, string.byte('2'), color565(255,255,255), color565(0,0,0), 1)
	local gx = map(input.readGyroX(1), -12000, 12000, -7, 7)
	local gy = map(input.readGyroY(1), -12000, 12000, -7, 7)
	local gz = map(input.readGyroZ(1), -12000, 12000, -7, 7)
	drawPanelLine(48+8, 7, 48+8 + gx, 7, color565(0,255,255))
	drawPanelLine(48+8, 7, 48+8 , 7 + gy, color565(255,0,255))
	drawPanelLine(48+8, 7, 48+8 + gz , 7+ gz, color565(255,255,0))
	drawPanelLine(48+8, 29, 48 + 7+gx, 29, color565(0,255,255))
	drawPanelLine(48+8, 30, 48 + 7+gy, 30, color565(255,0,255))
	drawPanelLine(48+8, 31, 48 + 7+gz, 31, color565(255,255,0))

	

	oledDisplay()
	flipPanelBuffer()
	ledsDisplay()
	if input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_LEFT) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_RIGHT) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_UP) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_DOWN) == BUTTON_PRESSED then 
		_M.shouldStop = true
		return true
	end
end 

function _M.onClose()

end

return _M