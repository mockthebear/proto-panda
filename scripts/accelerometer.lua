local _M = {
	shouldStop = false
}
local input = require("input")

local RED = color565(255, 0, 0)
local GREEN = color565(0, 255, 0)
local BLUE = color565(0, 0, 255)

function _M.onSetup()
	setPanelManaged(false)
	delay(500)
end

function _M.onLoop(dt)
	clearPanelBuffer()
	drawPanelLine(32,1, 32 + readAccelerometerX() * 16, 1 , RED)

	drawPanelLine(32,16, 32 + readAccelerometerY() * 16, 16 , GREEN)

	drawPanelLine(32,31, 32 + readAccelerometerZ() * 16, 31 , BLUE)

	flipPanelBuffer()
	if input.readButtonStatus(BUTTON_BACK) == BUTTON_JUST_PRESSED or readButtonStatus(BUTTON_CONFIRM) == BUTTON_JUST_PRESSED then 
		_M.shouldStop = true
		return true
	end
end 

function _M.onClose()
end

return _M