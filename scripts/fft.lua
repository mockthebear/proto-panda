local _M = {
	VERSION_REQUIRED="1.0.8",
	shouldStop = false,
	uiTimer = 0,
	fails = 0
}
local input = require("input")

function _M.onSetup()
	setPanelManaged(false)
	delay(500)
	beginSerialIo(921600)
    setTimeoutSerialIo(1)
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


function _M.onLoop(dt)
	_M.uiTimer = _M.uiTimer - dt
	

	if _M.uiTimer < 0 then  
        _M.uiTimer = 25
        local res = serialIoReadStringUntil()
        if #res > 0 and #res == (64*2) then 
        	_M.fails = 0
            oledClearScreen()
            clearPanelBuffer()
            local pos = 0
            for i=1,#res/2 do   
                local sb = res:sub(1 + (i-1) * 2, i * 2)
                local n = tonumber(sb, 16)/4
                if n then  
                    oledDrawLine(pos*2, 64-n*4, pos*2, 63, 1)
                    oledDrawLine(pos*2+1, 64-n*4, pos*2+1, 63, 1)
                    drawPanelLine(pos, 32-n*2, pos, 31, positionToRainbowColor(pos, 64))
                    drawPanelLine(128-pos, 32-n*2, 128-pos, 61, positionToRainbowColor(pos, 64))
                    pos = pos +1
                end
            end
            oledDisplay()
            flipPanelBuffer()
        else 
        	_M.fails = _M.fails +1
        end
        
    end
	
	if _M.fails > 200 then  
		_M.shouldStop = true
		return true
	end
	
	if input.readButtonStatus(BUTTON_BACK) == BUTTON_JUST_PRESSED or (input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_LEFT) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_RIGHT) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_UP) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_DOWN) == BUTTON_PRESSED) then 
    	_M.shouldStop = true
		return true
	end
end 

function _M.onClose()
end

return _M


