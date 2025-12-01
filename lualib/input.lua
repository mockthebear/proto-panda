--local versions = require("versions")
--local drivers = require("drivers")
MAX_BLE_CLIENTS = 4
MAX_BLE_BUTTONS = 5
local ACCELEROMETER_MULTIPLIER =  4 / 32768

local input = {
    started = false,
   
    panda_buttons = {},

    legacyReadButtonStatus = readButtonStatus,
    legacyreadAccelerometerX = readAccelerometerX,
    legacyreadAccelerometerY = readAccelerometerY,
    legacyreadAccelerometerZ = readAccelerometerZ,
    legacyreadGyroX = readGyroX,
    legacyreadGyroY = readGyroY,
    legacyreadGyroZ = readGyroZ,
    legacygetBleDeviceLastUpdate = getBleDeviceLastUpdate,
}

_G.BUTTON_RELEASED = 0
_G.BUTTON_JUST_PRESSED = 1
_G.BUTTON_PRESSED = 2
_G.BUTTON_JUST_RELEASED = 3


_G.BUTTON_LEFT = 1
_G.BUTTON_DOWN = 2
_G.BUTTON_RIGHT = 3
_G.BUTTON_UP = 4
_G.BUTTON_CONFIRM = 5

do
    local pdButtonId = 1
    for i=1,MAX_BLE_CLIENTS do 
        for b=1,MAX_BLE_BUTTONS do 
            input.panda_buttons[pdButtonId] = _G.BUTTON_RELEASED
            pdButtonId = pdButtonId +1
        end
    end
end


function input.updateButtonStates()
    local pdButtonId = 1
    for i=1,MAX_BLE_CLIENTS do 
        local pd = drivers.panda[id].buttons
        for b=1,MAX_BLE_BUTTONS do 
            if input.panda_buttons[pdButtonId] == _G.BUTTON_JUST_PRESSED then  
                input.panda_buttons[pdButtonId] = _G.BUTTON_PRESSED
            end
            if input.panda_buttons[pdButtonId] == _G.BUTTON_JUST_RELEASED then  
                input.panda_buttons[pdButtonId] = _G.BUTTON_RELEASED
            end
            if input.panda_buttons[pdButtonId] ~= pd[b] then 
                if pd[b] == 1 then 
                    input.panda_buttons[pdButtonId] = _G.BUTTON_JUST_PRESSED
                else
                    input.panda_buttons[pdButtonId] = _G.BUTTON_JUST_RELEASED
                end
            end
            pdButtonId = pdButtonId +1
        end
    end
end



function input.getBleDeviceLastUpdate(id)
    if input.legacygetBleDeviceLastUpdate then  
        return input.legacygetBleDeviceLastUpdate(id)
    end
    return drivers.panda[id].last_update
end

function input.readAccelerometerX(id)
    if input.legacyreadAccelerometerX then  
        return input.legacyreadAccelerometerX(id)
    end
    local v = drivers.panda[id].ax
    return v * ACCELEROMETER_MULTIPLIER
end

function input.readAccelerometerY(id)
    if input.legacyreadAccelerometery then  
        return input.legacyreadAccelerometery(id)
    end
    local v = drivers.panda[id].ay
    return v * ACCELEROMETER_MULTIPLIER
end

function input.readAccelerometerZ(id)
    if input.legacyreadAccelerometerz then  
        return input.legacyreadAccelerometerz(id)
    end
    local v = drivers.panda[id].az
    return v * ACCELEROMETER_MULTIPLIER
end

function input.readGyroX(id)
    if input.legacyreadGyroX then  
        return input.legacyreadGyroX(id)
    end
    return drivers.panda[id].gx
end

function input.readGyroY(id)
    if input.legacyreadGyroY then  
        return input.legacyreadGyroY(id)
    end
    return drivers.panda[id].gy
end

function input.readGyroZ(id)
    if input.legacyreadGyroZ then  
        return input.legacyreadGyroZ(id)
    end
    return drivers.panda[id].gz
end

function input.readButtonStatus(button)
    if input.legacyReadButtonStatus then  
        return input.legacyReadButtonStatus()
    end
    return panda_buttons[button]
end

local function checkRegisterGlobalCompat(gfname, fn)
    if _G[gfname] then 
        if _G[gfname] ~= fn then  
            print("Rewritten function "..gfname..' in legacy mode')
        end
    end
end

checkRegisterGlobalCompat('readButtonStatus', input.readButtonStatus)
checkRegisterGlobalCompat('readAccelerometerX', input.readAccelerometerX)
checkRegisterGlobalCompat('readAccelerometerY', input.readAccelerometerY)
checkRegisterGlobalCompat('readAccelerometerZ', input.readAccelerometerZ)
checkRegisterGlobalCompat('readGyroX', input.readGyroX)
checkRegisterGlobalCompat('readGyroY', input.readGyroY)
checkRegisterGlobalCompat('readGyroZ', input.readGyroZ)
checkRegisterGlobalCompat('getBleDeviceLastUpdate', input.getBleDeviceLastUpdate)
checkRegisterGlobalCompat('getBleDeviceUpdateDt', input.getBleDeviceLastUpdate)



return input