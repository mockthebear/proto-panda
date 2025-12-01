local _M = {
	VERSION_REQUIRED="1.0.0",
	shouldStop = false,
	step=0,
	stepTimer=0,

	cycle=0,
}
local input = require("input")

local function hslToRgb(h, s, l)
    h = h / 360
    local r, g, b

    if s == 0 then
        r, g, b = l, l, l -- achromatic
    else
        local function hueToRgb(p, q, t)
            if t < 0 then t = t + 1 end
            if t > 1 then t = t - 1 end
            if t < 1/6 then return p + (q - p) * 6 * t end
            if t < 1/2 then return q end
            if t < 2/3 then return p + (q - p) * (2/3 - t) * 6 end
            return p
        end

        local q = l < 0.5 and l * (1 + s) or l + s - l * s
        local p = 2 * l - q
        r = hueToRgb(p, q, h + 1/3)
        g = hueToRgb(p, q, h)
        b = hueToRgb(p, q, h - 1/3)
    end

    return math.floor(r * 255), math.floor(g * 255), math.floor(b * 255)
end

local function positionToRainbowColor(position, total_length)
    position = math.max(0, math.min(position, total_length))


    local hue = (position / total_length) * 360

    local saturation = 1.0
    local lightness = 0.5

    local r, g, b = hslToRgb(hue, saturation, lightness)

    return color565(r, g, b)
end

function _M.onSetup()
	setPanelManaged(false)
	_M.oldBrightness = getPanelBrightness()
	_M.moveRange = 10
	delay(500)
end

function _M.drawTextCentered(x,y, str, color, bg)
	bg = bg or 0
	y = y -3
	x = x - (#str*6)/2
	for i=1,#str do  
		local selectedColor = color
		if color == nil then 
			selectedColor = positionToRainbowColor(i, #str)
		end
		drawPanelChar(x + (i-1)*6, y, string.byte(str:sub(i,i)), selectedColor, bg, 1)
		drawPanelChar(64+x + (i-1)*6, y, string.byte(str:sub(i,i)), selectedColor, bg, 1)
	end
end

function _M.onLoop(dt)
	clearPanelBuffer()

	local x = math.cos(math.rad(_M.cycle))*_M.moveRange
	local y = math.sin(math.rad(_M.cycle))*_M.moveRange
	if _M.step == 0 then
		oledClearScreen()
		drawPanelFillRect(0,0, 128, 64, color565(0,0,0))
		oledSetCursor(0,0)
		oledSetFontSize(1)
		oledDrawText("Press any button to go to the next step.")
		oledDisplay()
		_M.drawTextCentered(32 + x ,16 + y, "Hi", color565(100,100,100), 0)
	elseif _M.step == 1 then
		drawPanelFillRect(0,0, 128, 64, color565(255,0,0))
		_M.drawTextCentered(32 + x ,16 + y, "Red", 0, color565(255,0,0))
	elseif _M.step == 2 then
		drawPanelFillRect(0,0, 128, 64, color565(0,255,0))
		_M.drawTextCentered(32 + x ,16 + y, "Green", 0, color565(0,255,0))
	elseif _M.step == 3 then
		drawPanelFillRect(0,0, 128, 64, color565(0,0,255))
		_M.drawTextCentered(32 + x ,16 + y, "Blue", 0, color565(0,0,255))
	elseif _M.step == 4 then
		drawPanelFillRect(0,0, 128, 64, color565(127,127,127))
		_M.drawTextCentered(32 + x ,16 + y, "Gray", 0, color565(127,127,127))
	elseif _M.step == 5 then
		oledClearScreen()
		oledSetCursor(0,0)
		oledSetFontSize(2)
		oledDrawText("Horizontal")
		oledDisplay()
		for yy=0,32 do
			if yy%2 == 1 then
				drawPanelLine(0,yy,128,yy, color565(255,255,255))
			end
		end
	elseif _M.step == 6 then
		oledClearScreen()
		oledSetCursor(0,0)
		oledSetFontSize(2)
		oledDrawText("Vertical")
		oledDisplay()
		for xx=0,128 do
			if xx%2 == 1 then
				drawPanelLine(xx,0,xx,63, color565(255,255,255))
			end
		end
	elseif _M.step == 7 then
		oledClearScreen()
		oledSetCursor(0,0)
		oledSetFontSize(2)
		oledDrawText("Vertical")
		oledDisplay()
		for xx=0,128 do
			drawPanelLine(xx,0,xx,63, color565(xx % 4 == 0 and 255 or 0,xx % 4 == 1 and 255 or 0,xx % 4 == 2 and 255 or 0))
		end
	elseif _M.step == 8 then
		_M.drawTextCentered(32 ,8 , "Power test", nil, 0)
		_M.drawTextCentered(32  ,16, "is", nil, 0)
		_M.drawTextCentered(32  ,24 , "NEXT",nil, 0)
	elseif _M.step == 9 then
		_M.drawTextCentered(32 ,8 , "Depending", nil, 0)
		_M.drawTextCentered(32  ,16, "on your", nil, 0)
		_M.drawTextCentered(32  ,24 , "supply",nil, 0)
	elseif _M.step == 10 then
		_M.drawTextCentered(32 ,8 , "power may", nil, 0)
		_M.drawTextCentered(32  ,16, "be cut off", nil, 0)
		_M.drawTextCentered(32  ,24 , "and reset",nil, 0)
	elseif _M.step >= 11 and _M.step < 28 then
		local brightness = math.min(255, math.max(8, (_M.step-11) * 16))
		_M.moveRange = 5
		setPanelBrightness(brightness)
		drawPanelFillRect(0,0, 128, 64, color565(255,255,255))
		_M.drawTextCentered(32 ,5 , "Brightness", nil, color565(255,255,255))
		_M.drawTextCentered(32 + x ,20 + y, tostring(brightness), color565(0,0,0), color565(255,255,255))
	elseif _M.step == 28 then
		drawPanelFillRect(0,0, 128, 64, color565(255,255,255))
	else
		setPanelBrightness(_M.oldBrightness)
		_M.shouldStop = true
	end

	


	_M.stepTimer = _M.stepTimer -dt

	_M.cycle = _M.cycle + 360 * (dt/1000)

	if input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_JUST_PRESSED or input.readButtonStatus(BUTTON_LEFT) == BUTTON_JUST_PRESSED or input.readButtonStatus(BUTTON_RIGHT) == BUTTON_JUST_PRESSED or input.readButtonStatus(BUTTON_UP) == BUTTON_JUST_PRESSED or input.readButtonStatus(BUTTON_DOWN) == BUTTON_JUST_PRESSED then 
		if _M.stepTimer < 0 then
			_M.step = _M.step +1
			_M.stepTimer = 150
		end
	end

	flipPanelBuffer()
	if input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_LEFT) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_RIGHT) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_UP) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_DOWN) == BUTTON_PRESSED then 
		_M.shouldStop = true
		return true
	end
end 

function _M.onClose()
end

return _M


