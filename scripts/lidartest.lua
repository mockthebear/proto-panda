local _M = {
	VERSION_REQUIRED="1.0.0",
	shouldStop = false,
	uiTimer = 0,
	rainbowShift = 0,
	rainbowShiftTimer = 0,

}
local input = require("input")

function _M.onSetup()
	setPanelManaged(false)
	delay(500)
	_M.points = {}
	_M.cursor = 0
	_M.avg = 0
	for i=0,127 do 
		_M.points[i] = 0
	end
end

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

function _M.drawTextRainbow(x,y, str, shift)
	for i=1,#str do  
		local colorIndex = (i+shift) % (#str+1)
		drawPanelChar(x + (i-1)*6, y, string.byte(str:sub(i,i)), positionToRainbowColor(colorIndex, #str), 0, 1)
	end
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

	_M.rainbowShiftTimer = _M.rainbowShiftTimer - dt 
	if _M.rainbowShiftTimer < 0 then 
		_M.rainbowShiftTimer = 90
		_M.rainbowShift = _M.rainbowShift + 1
	end

	local readingRaw = readLidar()
	local reading = readingRaw
	local out_of_range_color = color565(200,200,200)
	if reading >= 8190 then
		out_of_range_color = color565(255,0,0)
		reading = 600
	elseif reading > 600 then 
		out_of_range_color = color565(255,255,0)
		reading = 600
	end
	if reading <= 0 then 
		reading = 0
	end

	_M.avg = _M.avg * 0.8 + reading * 0.2

	if hasLidar() then
		_M.drawTextRainbow(64, 0, "Lidar: on", _M.rainbowShift)
		_M.drawTextRainbow(64, 15, string.format("R: %.4dmm", readingRaw), _M.rainbowShift)
		_M.drawTextRainbow(64, 25, string.format("A: %.4dmm", math.floor(_M.avg)), color565(200,200,200))

		drawPanelRect(64, 8, 60, 6, color565(255,255,255))
		if reading > 0 then
			drawPanelFillRect(65,  9, (reading/600)*59, 4, out_of_range_color)
		end
	else
		_M.drawText(64, 0, "Lidar: off", color565(255,255,255))
		_M.drawText(64, 15, "Reading:", color565(255,255,255))
		_M.drawText(64, 25, string.format("%.4dmm", reading), color565(200,200,200))
	end
	oledClearScreen()
	oledSetCursor(0,0)
	oledDrawText( string.format("R: %.4dmm\nA: %.4dmm", reading, math.floor(_M.avg)))

	_M.points[_M.cursor] = reading
	_M.cursor = _M.cursor +1
	if _M.cursor > 127 then 
		_M.cursor = 0
	end
	
	for i=0,127 do 
		local iOffst = (_M.cursor+i)%127
		local read = _M.points[iOffst]
		if read > 600 then  
			read = 600
		end
		oledDrawPixel(i, 63-(read/600)*63, 1)
		if i%2 == 1 then 
			local rd = _M.points[iOffst+1]
			if rd then  
				read = (read + rd)/2
			end
			drawPanelPixel(i/2, 31-(read/600)*31, color565(200,200,200))
		end
	end
	oledDisplay()
	flipPanelBuffer()
	if input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_LEFT) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_RIGHT) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_UP) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_DOWN) == BUTTON_PRESSED then 
		_M.shouldStop = true
		return true
	end
end 

function _M.onClose()
end

return _M


