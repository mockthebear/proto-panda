local _M = {
    VERSION_REQUIRED="1.0.0",
    shouldStop = false,
    accumulatedPos = 170,
    accumulatedSpeed = 0,
    apples={},
    score = 0,
    colorEffectMult = 0,
}
local input = require("input")

function _M.onSetup()
    setPanelManaged(false)
    _M.apples={ {x=0, y = math.random(0, 31)}, {x=16, y = math.random(0, 31)}, {x=24, y = math.random(0, 31)}, {x=30, y = math.random(0, 31)} }
    _M.score = 0
end

local generic = require("generic")


function _M.drawScore()
    local str = tostring(_M.score)
    for i=1,#str do
        drawPanelChar(0 + (i-1)*6, 0, string.byte(str:sub(i,i)), color565(255,255,255), 0, 1)
        drawPanelChar(128 - (i)*6, 0, string.byte(str:sub(i,i)), color565(255,255,255), 0, 1)
    end
end


function _M.onLoop(dt)
    clearPanelBuffer()

    dt = dt / 1000

    local pos = readLidar()

    if pos < 120 then 
        pos = 120
    end

    if pos > 320 then 
        pos = 320
    end

    local speed = 0 
    local pixelAdd = 0
    if pos > _M.accumulatedPos then 
        speed = 1950
        pixelAdd = 6
    elseif pos < _M.accumulatedPos then  
        speed = -1950
        pixelAdd = -1
    end

    _M.accumulatedSpeed = _M.accumulatedSpeed + speed * dt;

    if (_M.accumulatedSpeed > 195) then 
        _M.accumulatedSpeed = 195
    end
    if (_M.accumulatedSpeed < -195) then 
        _M.accumulatedSpeed = -195
    end
    

    _M.accumulatedPos = _M.accumulatedPos + _M.accumulatedSpeed*dt

    if _M.accumulatedPos < 120 then 
        _M.accumulatedPos = 120
    end

    if _M.accumulatedPos > 320 then 
        _M.accumulatedPos = 320
    end

    local Ypos = generic.map(_M.accumulatedPos, 120, 320, 26, 0)

    for i,appl in pairs(_M.apples) do  
        appl.x = appl.x + 25 * dt
        drawPanelRect(appl.x, appl.y, 2, 2,  color565(255,0,0))
        drawPanelRect(128-appl.x, appl.y, 2, 2,  color565(255,0,0))
        oledDrawRect(appl.x+1, appl.y, 2, 2, 1)
        if appl.x >= 63 and math.abs(Ypos+1-appl.y) <= 3 then 
            _M.apples[i] = {x=0, y = math.random(0, 31)}
            _M.score = _M.score +3
            _M.colorEffectMult = 1
            toneDuration(440, 100)
        end
        if appl.x >= 64 then 
            _M.apples[i] = {x=0, y = math.random(0, 31)}
            _M.score = _M.score -1 
            if _M.score < 0 then 
                _M.score = 0
            end
        end
    end

    if _M.colorEffectMult > 0 then 
        _M.colorEffectMult = _M.colorEffectMult - 1 * dt 
        if _M.colorEffectMult < 0 then 
            _M.colorEffectMult = 0
        end
    end

    local mult = math.floor(_M.colorEffectMult * 255)

    drawPanelLine(63,Ypos,63,Ypos+5, color565(255 - mult ,255,255 -mult ))
    drawPanelLine(64,Ypos,64,Ypos+5, color565(255 - mult ,255,255 -mult ))

    drawPanelPixel(63, Ypos+pixelAdd, color565(pixelAdd < 0 and 255 or 0, pixelAdd > 0 and 255 or 0,0))
    drawPanelPixel(64, Ypos+pixelAdd, color565(pixelAdd < 0 and 255 or 0, pixelAdd > 0 and 255 or 0,0))

    _M.drawScore()

    oledDrawRect(0, 0, 65, 33, 1)
    oledDrawLine(63,Ypos,63,Ypos+5, 1)
    oledSetCursor(1, 1)
    oledDrawText(tostring(_M.score))
   

    flipPanelBuffer()
    if input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_LEFT) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_RIGHT) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_UP) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_DOWN) == BUTTON_PRESSED then 
        _M.shouldStop = true
        return true
    end
end 

function _M.onClose()
    _M.wah = nil
    collectgarbage()
end

return _M