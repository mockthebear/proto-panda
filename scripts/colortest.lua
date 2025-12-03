local _M = {
	VERSION_REQUIRED="1.0.0",
	shouldStop = false,
	red = 255,
	green = 13,
	blue = 150,
	selected=1,
	colors = {"red", "green", "blue"},
	uiTimer = 0,
	rainbowShiftTimer = 0,

}
local input = require("input")

function _M.onSetup()
	setPanelManaged(false)
	delay(500)
end

function _M.drawText(x,y, str, color, bg)
	bg = bg or 0
	for i=1,#str do  
		drawPanelChar(x + (i-1)*6, y, string.byte(str:sub(i,i)), color, bg, 1)
	end
end

function _M.onLoop(dt)
	_M.uiTimer = _M.uiTimer - dt
	clearPanelBuffer()

	local selected = _M.colors[_M.selected]
	
	_M.drawText(27, 9, "G: "..string.format("%3d", _M.green), color565(0, selected ~= "green" and 255 or 0, 0), color565(0, selected == "green" and 255 or 0, 0))
	drawPanelRect(26, 8, 38, 9, color565(100,255,100))
	_M.drawText(27, 1, "R: "..string.format("%3d", _M.red),   color565(selected ~= "red" and 255 or 0, 0, 0),   color565(selected == "red" and 255 or 0, 0, 0  ))
	drawPanelRect(26, 0, 38, 9, color565(255,100,100))
	_M.drawText(27, 17, "B: "..string.format("%3d", _M.blue), color565(0, 0, selected ~= "blue" and 255 or 0),  color565(0, 0, selected == "blue" and 255 or 0 ))
	drawPanelRect(26, 16, 38, 9, color565(100,100,255))



	_M.drawText(12, 25, "0x"..string.format("%.2x%.2x%.2x", _M.red, _M.green, _M.blue), color565(_M.red, _M.green, _M.blue), 0)
	if input.readButtonStatus(BUTTON_RIGHT) == BUTTON_PRESSED and _M.uiTimer < 0 then 
		_M.uiTimer = 5
		_M[selected] = _M[selected] +1
		if _M[selected] > 255 then 
			_M[selected] = 255
		end
	end
	if input.readButtonStatus(BUTTON_LEFT) == BUTTON_PRESSED and _M.uiTimer < 0 then 
		_M.uiTimer = 5
		_M[selected] = _M[selected] -1
		if _M[selected] < 0 then 
			_M[selected] = 0
		end
	end

	if input.readButtonStatus(BUTTON_DOWN) == BUTTON_JUST_PRESSED then 
		_M.selected = _M.selected +1
		if _M.selected > 3 then 
			_M.selected = 1
		end
	end

	if input.readButtonStatus(BUTTON_UP) == BUTTON_JUST_PRESSED then 
		_M.selected = _M.selected -1
		if _M.selected < 1 then 
			_M.selected = 3
		end
	end

	drawPanelFillCircle(10, 10, 9, color565(_M.red, _M.green, _M.blue))
	drawPanelCircle(10, 10, 10, color565(255,255,255))
	
	drawPanelFillCircle(128-10, 10, 9, color565(_M.red, _M.green, _M.blue))
	drawPanelCircle(128-10, 10, 10, color565(255,255,255))

	
	flipPanelBuffer()
	if input.readButtonStatus(BUTTON_BACK) == BUTTON_JUST_PRESSED or (input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_LEFT) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_RIGHT) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_UP) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_DOWN) == BUTTON_PRESSED) then 
		_M.shouldStop = true
		return true
	end
end 

function _M.onClose()
end

return _M


