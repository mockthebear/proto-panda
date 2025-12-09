local versions = require("versions")
local drivers = require("drivers")

local ACCELEROMETER_MULTIPLIER =  4 / 32768

local input = {
    started = false,
   
    panda_buttons = {},
    panda_buttons_state = {},

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
_G.BUTTON_BACK = 6

do
    local pdButtonIdOffset = 1
    for i=0,MAX_BLE_CLIENTS-1 do 
        for b=1,MAX_BLE_BUTTONS do 
            input.panda_buttons[pdButtonIdOffset] = 0
            input.panda_buttons_state[pdButtonIdOffset] = _G.BUTTON_RELEASED
            pdButtonIdOffset = pdButtonIdOffset +1
        end
        _G['DEVICE_'..i..'_BUTTON_LEFT']    = _G.BUTTON_LEFT    + i * MAX_BLE_BUTTONS
        _G['DEVICE_'..i..'_BUTTON_DOWN']    = _G.BUTTON_DOWN    + i * MAX_BLE_BUTTONS
        _G['DEVICE_'..i..'_BUTTON_RIGHT']   = _G.BUTTON_RIGHT   + i * MAX_BLE_BUTTONS
        _G['DEVICE_'..i..'_BUTTON_UP']      = _G.BUTTON_UP      + i * MAX_BLE_BUTTONS
        _G['DEVICE_'..i..'_BUTTON_CONFIRM'] = _G.BUTTON_CONFIRM + i * MAX_BLE_BUTTONS
        _G['DEVICE_'..i..'_BUTTON_BACK']    = _G.BUTTON_BACK    + i * MAX_BLE_BUTTONS

        _G['DEVICE_'..i..'_BUTTON_FIRST']   = _G.BUTTON_LEFT    + i * MAX_BLE_BUTTONS
        _G['DEVICE_'..i..'_BUTTON_LAST']    = _G.BUTTON_BACK    + i * MAX_BLE_BUTTONS
    end
end

local keybind = {
    ['joystick'] = {
        [_G.BUTTON_LEFT]     = {resource = 'right_hat', match = drivers.JOYSTICK_HAT_DIRECTION_LEFT},
        [_G.BUTTON_DOWN]     = {resource = 'right_hat', match = drivers.JOYSTICK_HAT_DIRECTION_DOWN},
        [_G.BUTTON_RIGHT]    = {resource = 'right_hat', match = drivers.JOYSTICK_HAT_DIRECTION_RIGHT},
        [_G.BUTTON_UP]       = {resource = 'right_hat', match = drivers.JOYSTICK_HAT_DIRECTION_TOP},
        [_G.BUTTON_CONFIRM]  = {resource = 'buttons' , index = drivers.JOSYTICK_BUTTON_A},
        [_G.BUTTON_BACK]     = {resource = 'buttons' , index = drivers.JOSYTICK_BUTTON_B},
    },
    ['beauty'] = {
        [_G.BUTTON_LEFT]     = {resource = 'buttons', index = drivers.BEAUTY_BUTTON_4},
        [_G.BUTTON_DOWN]     = {resource = 'buttons', index = drivers.BEAUTY_BUTTON_1},
        [_G.BUTTON_RIGHT]    = {resource = 'buttons', index = drivers.BEAUTY_BUTTON_3},
        [_G.BUTTON_UP]       = {resource = 'buttons', index = drivers.BEAUTY_BUTTON_2},
        [_G.BUTTON_CONFIRM]  = {resource = 'buttons', index = drivers.BEAUTY_BUTTON_5},
        [_G.BUTTON_BACK]     = {resource = 'buttons', index = drivers.BEAUTY_BUTTON_6},
    }
}


function input.setup()
    for mode, data in pairs(keybind) do  
        if not drivers[mode] then 
            error("Unknown keybind target name with '"..mode.."'")
        end
    end
end

function input.updatHidButtonStates(hdReading, pdButtonIdOffset, mode)
    for idx, bind in pairs(keybind[mode]) do
        local element = hdReading[bind.resource]
        local reading = 0
        if bind.match then  
            reading = element == bind.match and 1 or 0
        elseif bind.index then 
            reading = element[bind.index]
        end
        input.updateButtonByreading(pdButtonIdOffset+idx-1, reading)
    end
end

function input.updatePandaButtonStates(pdReading, pdButtonIdOffset)
    for b=1,MAX_BLE_BUTTONS do 
        input.updateButtonByreading(pdButtonIdOffset, pdReading[b])
        pdButtonIdOffset = pdButtonIdOffset +1
    end
end

function input.updateButtonByreading(buttonId, reading)
    if input.panda_buttons_state[buttonId] == _G.BUTTON_JUST_PRESSED then  
        input.panda_buttons_state[buttonId] = _G.BUTTON_PRESSED
    end
    if input.panda_buttons_state[buttonId] == _G.BUTTON_JUST_RELEASED then  
        input.panda_buttons_state[buttonId] = _G.BUTTON_RELEASED
    end

    if input.panda_buttons[buttonId] ~= reading then 
        input.panda_buttons[buttonId] = reading
        if reading == 1 then 
            input.panda_buttons_state[buttonId] = _G.BUTTON_JUST_PRESSED
        else
            input.panda_buttons_state[buttonId] = _G.BUTTON_JUST_RELEASED
        end
    end
end

function input.updateButtonStates()
    local pdButtonIdOffset = 1
    for i=0,MAX_BLE_CLIENTS-1 do 
        local mode = drivers.type_by_id[i]
        if mode then
            local obj = drivers[mode]
            if obj then  
                local reading = obj[i]
                if mode == "panda" then
                    input.updatePandaButtonStates(reading, pdButtonIdOffset, mode)
                elseif mode == "beauty" then
                    input.updatHidButtonStates(reading, pdButtonIdOffset, mode)
                elseif mode == "hid" then
                    input.updatHidButtonStates(reading, pdButtonIdOffset, mode)
                end
            end
        end

        pdButtonIdOffset = pdButtonIdOffset + MAX_BLE_BUTTONS
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
    return input.panda_buttons_state[button]
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