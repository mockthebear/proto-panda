local _M = {
	VERSION_REQUIRED="1.0.7",
	shouldStop = false,
	colorShiftTime = 1000,
	selectedColormode = 1,
}
local input = require("input")

local colormodes = {
	COLOR_MODE_RGB,
    COLOR_MODE_RBG,
    COLOR_MODE_GRB,
    COLOR_MODE_GBR,
    COLOR_MODE_BRG,
    COLOR_MODE_BGR,
}
function _M.onSetup()
	setPanelColorMode(COLOR_MODE_RGB)
end


function _M.onLoop(dt)
	_M.colorShiftTime = _M.colorShiftTime - dt
	if _M.colorShiftTime < 0 then 
		setPanelColorMode(colormodes[_M.selectedColormode])
		_M.selectedColormode = _M.selectedColormode +1 
		if _M.selectedColormode > #colormodes then  
			 _M.selectedColormode = 1
		end
		_M.colorShiftTime = 500
	end
	

	if input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_LEFT) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_RIGHT) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_UP) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_DOWN) == BUTTON_PRESSED then 
		_M.shouldStop = true
		return true
	end
end 

function _M.onClose()
	setPanelColorMode(COLOR_MODE_RGB)
end

return _M


