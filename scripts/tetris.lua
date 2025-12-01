local _M = {
    VERSION_REQUIRED="1.0.0",
    shouldStop = false,
    TETRIS_Y = 20,
    TETRIS_X = 10,
    inputDelayAmount = 50,
}
local input = require("input")


_M.t_line = {
    {
        {0,1,0,0},
        {0,1,0,0},
        {0,1,0,0},
        {0,1,0,0}
    },

    {
        {1,1,1,1},
        {0,0,0,0},
        {0,0,0,0},
        {0,0,0,0},
    },
    
    {
        {0,1,0,0},
        {0,1,0,0},
        {0,1,0,0},
        {0,1,0,0},
    },
    
    {
        {1,1,1,1},
        {0,0,0,0},
        {0,0,0,0},
        {0,0,0,0},
    },

};

_M.t_tpiece = {
    {
        {0,1,0,0},
        {1,1,1,0},
        {0,0,0,0},
        {0,0,0,0}
    },

    {
        {0,1,0,0},
        {0,1,1,0},
        {0,1,0,0},
        {0,0,0,0},
    },
    
    {
        {0,0,0,0},
        {1,1,1,0},
        {0,1,0,0},
        {0,0,0,0},
    },
    
    {
        {0,1,0,0},
        {1,1,0,0},
        {0,1,0,0},
        {0,0,0,0},
    },

};

_M.t_square = {
    {
        {1,1,0,0},
        {1,1,0,0},
        {0,0,0,0},
        {0,0,0,0}
    },

    {
        {1,1,0,0},
        {1,1,0,0},
        {0,0,0,0},
        {0,0,0,0},
    },
    
    {
        {1,1,0,0},
        {1,1,0,0},
        {0,0,0,0},
        {0,0,0,0},
    },
    
    {
        {1,1,0,0},
        {1,1,0,0},
        {0,0,0,0},
        {0,0,0,0},
    },

};

_M.t_lright = {
    {
        {0,1,0,0},
        {0,1,0,0},
        {0,1,1,0},
        {0,0,0,0}
    },

    {
        {0,0,0,0},
        {1,1,1,0},
        {1,0,0,0},
        {0,0,0,0},
    },
    
    {
        {1,1,0,0},
        {0,1,0,0},
        {0,1,0,0},
        {0,0,0,0},
    },
    
    {
        {0,0,1,0},
        {1,1,1,0},
        {0,0,0,0},
        {0,0,0,0},
    },

};


_M.t_lleft = {
    {
        {0,1,0,0},
        {0,1,0,0},
        {1,1,0,0},
        {0,0,0,0}
    },

    {
        {1,0,0,0},
        {1,1,1,0},
        {0,0,0,0},
        {0,0,0,0},
    },
    
    {
        {0,1,1,0},
        {0,1,0,0},
        {0,1,0,0},
        {0,0,0,0},
    },
    
    {
        {0,0,0,0},
        {1,1,1,0},
        {0,0,1,0},
        {0,0,0,0},
    },

};

_M.t_lwierd = {
    {
        {0,1,0,0},
        {1,1,0,0},
        {1,0,0,0},
        {0,0,0,0}
    },

    {
        {1,1,0,0},
        {0,1,1,0},
        {0,0,0,0},
        {0,0,0,0},
    },
    
    {
        {0,1,0,0},
        {1,1,0,0},
        {1,0,0,0},
        {0,0,0,0}
    },
    
    {
        {1,1,0,0},
        {0,1,1,0},
        {0,0,0,0},
        {0,0,0,0},
    },
    

};

_M.t_rwierd = {
    {
        {1,0,0,0},
        {1,1,0,0},
        {0,1,0,0},
        {0,0,0,0}
    },

    {
        {0,1,1,0},
        {1,1,0,0},
        {0,0,0,0},
        {0,0,0,0},
    },
    
    {
        {1,0,0,0},
        {1,1,0,0},
        {0,1,0,0},
        {0,0,0,0}
    },
    
   {
        {0,1,1,0},
        {1,1,0,0},
        {0,0,0,0},
        {0,0,0,0},
    },
}



_M.pieces = {
    _M.t_tpiece,
    _M.t_square,
    _M.t_lright,
    _M.t_lleft,
    _M.t_rwierd,
    _M.t_lwierd,
    _M.t_line,
}

function shiftArray(arr)
    local result = {}
    for i, v in ipairs(arr) do
        if type(v) == "table" then
            result[i - 1] = shiftArray(v)
        else
            result[i - 1] = v
        end
    end
    return result
end

for idx, arr in pairs(_M.pieces) do  
    _M.pieces[idx] = shiftArray(arr)
end

function _M.getPartTetris(i, rotation)

    return _M.pieces[i][rotation]
end

function _M.ClearPart(pad)
    local part = _M.getPartTetris(_M.kind, _M.rotation);
    for ay=0,3 do
        for ax=0,3 do
            if part[ay][ax] ~= 0 then
                if (_M.y+ay < (_M.TETRIS_Y) and _M.y+ay >= 0 and _M.x+ax < (_M.TETRIS_X) and _M.x+ax >= 0) then
                    pad[_M.y+ay][_M.x+ax] = 0;
                end
            end
        end
    end
end


function _M.SetPart(pad)
    local part = _M.getPartTetris(_M.kind, _M.rotation);
    for ay=0,3 do
        for ax=0,3 do
            if part[ay][ax] ~= 0 then
                if (_M.y+ay < (_M.TETRIS_Y) and _M.y+ay >= 0 and _M.x+ax < (_M.TETRIS_X) and _M.x+ax >= 0) then
                    pad[_M.y+ay][_M.x+ax] = _M.kind;
                end
            end
        end
    end
end

function _M.getSideCollision(pad)
    local part = _M.getPartTetris(_M.kind, _M.rotation);
    for ay=0,3 do
        for ax=0,3 do
            if part[ay][ax] ~= 0 then
                if (_M.x+ax > (_M.TETRIS_X)) then
                    return 2
                end
                if (_M.x+ax < 0) then
                    return 1
                end
            end
        end
    end
    return 0
end

function _M.isColliding(pad)
    local part = _M.getPartTetris(_M.kind, _M.rotation);
    for ay=0,3 do
        for ax=0,3 do
            if part[ay][ax] ~= 0 then
                if _M.y+ay >= (_M.TETRIS_Y) or _M.y+ay < 0 then
                    return true
                end
                if _M.x+ax >= (_M.TETRIS_X) or _M.x+ax < 0 then
                    return true
                end
                if pad[_M.y+ay][_M.x+ax] ~= 0 then
                    return true
                end
            end
        end
    end
    return false
end


function _M.Rotate(pad)
    _M.ClearPart(pad)
    local prev = _M.rotation
    _M.rotation = (_M.rotation+1)%4

    local side = _M.getSideCollision(pad)
    if (side ~= 0) then
        local xPos = _M.x
        if (side == 1) then
            _M.x = _M.x +1
            if (_M.isColliding(pad)) then
                _M.x = _M.x +1
                if (_M.isColliding(pad)) then
                    _M.x =  xPos
                end
            end
        else
            if (_M.isColliding(pad)) then
                if (_M.isColliding(pad)) then
                    _M.x = xPos
                end
            end
        end
    end

    if (_M.isColliding(pad)) then
        _M.rotation = prev
    end
    _M.SetPart(pad)
    _M.ValidateAliveness()
end

function _M.Drop(pad)
    while (_M.Move(pad, 0, 1)) do end
    _M.ValidateAliveness()
end

function _M.Move(pad, ax, ay)
    if not _M.alive then
        _M.ValidateAliveness()
        return false
    end

    _M.ClearPart(pad)
  
    _M.x = _M.x + ax

    if (_M.isColliding(pad)) then
        _M.x = _M.x - ax;
        ax = 0
    end

    _M.y = _M.y + ay

    if (ay ~= 0) then
        if (_M.isColliding(pad)) then
            _M.alive = false
            _M.y = _M.y - ay
            _M.SetPart(pad)
            _M.ValidateAliveness()
            return false
        end
    end
    _M.SetPart(pad)
    _M.ValidateAliveness()
    return true
end 

function _M.Clear() 


    _M.nextPart = math.random(1,7)
    _M.setNextPart(_M.nextPart)
    _M.nextPart = math.random(1,7)

    _M.nextPart = math.random(1,7)
    for y = 0, _M.TETRIS_Y  do
        _M.pad[y] = {}
        for x = 0, _M.TETRIS_X  do
            _M.pad[y][x] = 0
        end
    end

    for ay=0,3 do
        for ax=0,3 do
            _M.pad[ay][ax] = 0;
        end
    end


    _M.partTimer = 500;
end

function _M.DropColumn(ypos)
    if (ypos <= 0) then
        return;
    end
    for ya = ypos, 0, -1 do
        for xa = 0, _M.TETRIS_X  do
            local val
            if (ya < 0) then
                val = 0;
            else
                val = _M.pad[ya][xa];
            end
            _M.pad[ya+1][xa] = val;
        end
    end
end

function _M.CheckForLines()
    for ya = _M.TETRIS_Y, 0, -1 do
        local lineComplete = true
        for xa = 0, _M.TETRIS_X - 1 do
            if _M.pad[ya][xa] == 0 then
                lineComplete = false
                break
            end
        end

        if lineComplete then
            for xb = 0, _M.TETRIS_X - 1 do
                _M.pad[ya][xb] = 0
            end
            _M.DropColumn(ya - 1)
            return _M.CheckForLines()
        end
    end
end

function _M.setNextPart(partId)
    _M.x = 5
    _M.y = 0 
    _M.kind = partId
    _M.rotation = 0
    _M.alive = true
end
function _M.ValidateAliveness()
    if not _M.alive then
        _M.setNextPart(_M.nextPart)
        _M.nextPart = math.random(1,7)
        _M.CheckForLines()
        if _M.pad[0][5] ~= 0 then
            _M.Clear()
            _M.SetPart(_M.pad)
            _M.partTimer = 0
        end
    end
end

function _M.getPartColor(pd)
    local colorTable = {
        [1] = color565(255, 0, 0),
        [2] = color565(0, 255, 0),
        [3] = color565(0, 0, 255),
        [4] = color565(255, 100, 0),
        [5] = color565(0, 255, 255),
        [6] = color565(255, 255, 0),
        [7] = color565(255, 0, 255)
    }
    return colorTable[pd] or 0
end

function _M.Draw(ox, oy)
    local pd
    drawPanelRect(ox - 1,  (oy - 1) , _M.TETRIS_X + 2, _M.TETRIS_Y + 2, color565(100, 100, 100))
    drawPanelRect(127 - (ox+_M.TETRIS_X +1) , (oy - 1), _M.TETRIS_X +2, _M.TETRIS_Y + 2, color565(100, 100, 100))

    --drawPanelLine(ox - 1, 6, ox + _M.TETRIS_X, 6, color565(100, 100, 100))
    --drawPanelLine(127- (ox - 1)-_M.TETRIS_X, 6,  (ox), 6, color565(100, 100, 100))

    for y = 0, _M.TETRIS_Y-1 do
        for x = 0, _M.TETRIS_X-1  do
            pd = _M.pad[y][x]
            drawPanelPixel(ox + x, (oy + y), _M.getPartColor(pd))
            drawPanelPixel(127- (ox + x+1), (oy + y), _M.getPartColor(pd))

        end
    end

    drawPanelRect(ox + _M.TETRIS_X + 1, (oy - 1) , 5, 6, color565(100, 100, 100))
    drawPanelRect(127- (ox + _M.TETRIS_X + 1+5),  (oy - 1) , 5, 6, color565(100, 100, 100))

    local part = _M.getPartTetris(_M.nextPart, 0)
    for ay = 0, 3 do
        for ax = 0, 3 do
            if part[ay][ax] ~= 0 then
                drawPanelPixel(ox + _M.TETRIS_X + 2 + ax, (oy + ay), _M.getPartColor(_M.nextPart))
                drawPanelPixel(127 - (ox + _M.TETRIS_X + 2 + ax+1),  (oy + ay), _M.getPartColor(_M.nextPart))
            end
        end
    end
end

function _M.ManageInput()
    if input.readButtonStatus(BUTTON_UP) == BUTTON_JUST_PRESSED then
        _M.Drop(_M.pad)
    end

    if input.readButtonStatus(BUTTON_RIGHT) == BUTTON_PRESSED and (input.readButtonStatus(BUTTON_RIGHT) == BUTTON_JUST_PRESSED or _M.inputDelay <= 0) then
        _M.Move(_M.pad, 1, 0)
        if input.readButtonStatus(BUTTON_RIGHT) == BUTTON_PRESSED then
            _M.inputDelay = _M.inputDelayAmount * 2
        else
            _M.inputDelay = _M.inputDelayAmount
        end
    end

    if input.readButtonStatus(BUTTON_LEFT) == BUTTON_PRESSED and (input.readButtonStatus(BUTTON_LEFT) == BUTTON_JUST_PRESSED or _M.inputDelay <= 0) then
        _M.Move(_M.pad, -1, 0)
        if input.readButtonStatus(BUTTON_LEFT) == BUTTON_PRESSED then
            _M.inputDelay = _M.inputDelayAmount * 2
        else
            _M.inputDelay = _M.inputDelayAmount
        end
    end

    if input.readButtonStatus(BUTTON_DOWN) == BUTTON_PRESSED and (input.readButtonStatus(BUTTON_DOWN) == BUTTON_JUST_PRESSED or _M.inputDelay <= 0) then
        _M.Move(_M.pad, 0, 1)
        _M.inputDelay = _M.inputDelayAmount
    end

    if input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_JUST_PRESSED then
        _M.Rotate(_M.pad)
    end
end

function _M.onSetup()
    setPanelManaged(false)
    _M.inputDelay = 0
    _M.pad = {}
    _M.Clear()
end



function _M.onLoop(dt)
    clearPanelBuffer()

    _M.ManageInput()

    _M.inputDelay = _M.inputDelay - dt
    _M.partTimer = _M.partTimer - dt

     if _M.partTimer <= 0 then
        _M.Move(_M.pad, 0, 1)
        _M.partTimer = 1000
    end
    _M.Draw(16, 3)

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