local versions = require("versions")
local drivers = require("drivers")
local configloader = require("configloader")

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

    mode = "NONE",

    infrared={},
    keybind = {},
    forced_buttons = {},
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
_G.BUTTON_AUX_A = 6
_G.BUTTON_AUX_B = 7
_G.BUTTON_BACK = 8

do
    local pdButtonIdOffset = 1
    for i=0,MAX_BLE_CLIENTS-1 do 
        for b=1,MAX_BLE_BUTTONS do 
            input.panda_buttons[pdButtonIdOffset] = 0
            input.panda_buttons_state[pdButtonIdOffset] = _G.BUTTON_RELEASED
            input.forced_buttons[pdButtonIdOffset] = 0
            pdButtonIdOffset = pdButtonIdOffset +1
        end
        _G['DEVICE_'..i..'_BUTTON_LEFT']    = _G.BUTTON_LEFT        + i * MAX_BLE_BUTTONS
        _G['DEVICE_'..i..'_BUTTON_DOWN']    = _G.BUTTON_DOWN        + i * MAX_BLE_BUTTONS
        _G['DEVICE_'..i..'_BUTTON_RIGHT']   = _G.BUTTON_RIGHT       + i * MAX_BLE_BUTTONS
        _G['DEVICE_'..i..'_BUTTON_UP']      = _G.BUTTON_UP          + i * MAX_BLE_BUTTONS
        _G['DEVICE_'..i..'_BUTTON_CONFIRM'] = _G.BUTTON_CONFIRM     + i * MAX_BLE_BUTTONS
        _G['DEVICE_'..i..'_BUTTON_BACK']    = _G.BUTTON_BACK        + i * MAX_BLE_BUTTONS
        _G['DEVICE_'..i..'_BUTTON_AUX_A']   = _G.BUTTON_AUX_A       + i * MAX_BLE_BUTTONS
        _G['DEVICE_'..i..'_BUTTON_AUX_B']   = _G.BUTTON_AUX_B       + i * MAX_BLE_BUTTONS

        _G['DEVICE_'..i..'_BUTTON_FIRST']   = _G.BUTTON_LEFT        + i * MAX_BLE_BUTTONS
        _G['DEVICE_'..i..'_BUTTON_LAST']    = _G.BUTTON_BACK        + i * MAX_BLE_BUTTONS
    end
end

function input.parseInputLocation(str)
    local idx = 1
    local parsed = {
        resource = {},
        reference = false
    }
    local controllerType = ""
    local mappedLogString = ""
    for element in str:gmatch("([^%.]+)") do  
        if idx == 1 then  
            if not drivers[element] then  
                error("There is no driver for the type '"..element.."'")
            end
            controllerType = element
        elseif idx == 2 then  
            local handler = drivers[controllerType][0]
            local toMatch = nil

            if not handler then  
                error("Unavalible resource '"..controllerType.."'")
            end
            
            if element:match(".-=.+") then 
                element, toMatch = element:match("(.-)=(.+)")
            end
            parsed.name = element
            if not handler[element] then  
                local avaliable = ""
                for i in pairs(handler) do  
                    avaliable = avaliable .. i..', ' 
                end
                error("Unavalible resource '"..element.."' in '"..str.."'. Avaliable: "..avaliable)
            end


            --Reference of the element directly
            for i=0,MAX_BLE_CLIENTS-1 do 
                local reference = drivers[controllerType][i][element]
                if type(reference) == 'table' then  
                    parsed.reference = true
                    parsed.resource[i] = reference
                else
                    parsed.resource[i] = element
                end
            end
            
            if toMatch then  
                parsed.match = tonumber(toMatch) or toMatch
                mappedLogString = "'"..controllerType.."' when '"..element.."' equals '"..parsed.match.."'"
                break
            end
        elseif idx == 3 then
            mappedLogString = "'"..controllerType.."' directly on '"..parsed.name.."["..element.."]'"
            parsed.location = tonumber(element) or element
            break
        end
        idx = idx+1
    end

    local dataLocation = drivers[controllerType][0][parsed.name]
    if type(dataLocation) ~= 'table' and not parsed.match then   
        error("Cannot use indexing on '"..str.."'. Maybe you meant = on the last dot?")
    end
    return parsed,controllerType, mappedLogString
end

function input.runBindCommand(binds, opcode)
    local f = binds[opcode]
    if f then 
        local success, err = pcall(f)
        if not success then  
            log(string.format("Error executing opcode %02X due: %s", opcode, err))
        end
    else
        log(string.format("Unmapped opcode %02X", opcode))
    end
end

function input.Start()
    if input.mode == "BLE" then  
        beginBleScanning()
    elseif input.mode == "INFRARED" then  
        enableIRInterrupt()
    end
end

function input.Load()
    local confs = configloader.Get()

    if not confs.input then  
        error("config.json missing 'input'")
    end

    local mode = confs.input.mode or "BLE"
    mode = mode:upper()
    input.mode = mode

    if not confs.input.drivers or #confs.input.drivers == 0 then  
        confs.input.drivers = {"generic"}
    end

    if mode == "BLE" then
        setLogDiscoveredBleDevices(false)
        generic.displaySplashMessage("Starting:\nBLE")
        startBLE()
        startBLERadio(ESP_PWR_LVL_P9)
    elseif mode == "INFRARED" then
        generic.displaySplashMessage("Starting:\nIR")
        startIR()
        if not confs.infrared then  
            error("config.json missing 'infrared' which is required when infrared is enabled")
        end

        if not confs.input.infrared_gpio then  
            error("config.json missing 'infrared_gpio' in input segment")
        end

        if type(confs.input.infrared_gpio) ~= "number" then  
            error("config.json missing 'infrared_gpio' in input segment")
        end

        setIRInterruptPin(confs.input.infrared_gpio)

        for i, mode in pairs(confs.infrared) do 
            if not mode.usercode or type(mode.bind) ~= 'table' then  
                error("config.json error at element "..i..', \'usercode\' or \'bind\' is missing')
            end
            local code = tonumber(mode.usercode, 16)
            if not code then  
                error("config.json error at element "..i..', \'usercode\' should be a HEX number')
            end
            local keymap = {}
            log("Loaded usercode "..mode.usercode)
            for opcodeS, func in pairs(mode.bind) do  
                local opcode = tonumber(opcodeS, 16)
                if not opcode then 
                    error("config.json error at element "..i..".bind. Opcode "..opcodeS.." is not a valid HEX")
                end

                local f, err = load(func)
                if not f then  
                    error("config.json error at element "..i..'.bind['..opcodeS..']: '..err)
                end
                keymap[opcode] = f
            end
            input.infrared[code] = keymap
        end
        drivers.type_by_id[0] = {mode={"generic"}, type="hid"}
    elseif mode == "NONE" then
        --We just accept
    else
        error("Invalid input mode: "..tostring(mode))
    end
    
    drivers.EnableDrivers(confs.input.drivers)

    input.SetKeybinds(confs.keybinds) 
end

function input.SetKeybinds(keybinds) 
    for location, target in pairs(keybinds) do  
        local binding, controllerType, logMsg = input.parseInputLocation(location)
        local toInsert = input.keybind[controllerType]
        if not toInsert then  
            toInsert = {}
            --Lua stores references of tables :3
            --This makes our life easier
            input.keybind[controllerType] = toInsert
        end
        if not _G[target] or not tonumber(_G[target]) then  
            error("Unknown action "..target)
        end
        log("[KEYBIND] Mapped "..tostring(target).." on "..tostring(logMsg))
        binding.action = tonumber(_G[target])
        toInsert[#toInsert+1] = binding
    end
end

function input.updatGenericButtonStates(inputModes, clientId, pdButtonIdOffset)
    local keybind = input.keybind
    local actions = {}
    for b=1,MAX_BLE_BUTTONS do
        actions[b] = 0
    end

    for _, name in pairs(inputModes) do
        local controller = drivers[name]
        --Search for inputs on each type.. joystick, panda, mouse, keyboard etc
        if keybind[name] then
            for __, bind in pairs(keybind[name]) do
                local element = bind.resource[clientId]
                local reading = 0
                if not bind.reference then  
                    element = controller[clientId][element]
                end
                if bind.match then  
                    reading = element == bind.match and 1 or 0
                elseif bind.location then 
                    if type(element) ~= 'table' then  
                        error(tostring(bind.resource[clientId]).." is looking for "..tostring(controller[clientId]).." at "..bind.location.." ref is "..tostring(bind.reference).." element = "..tostring(element))
                    end
                    reading = element[bind.location]
                end
                if reading == 1 and actions[bind.action] == 0 then  
                    actions[bind.action] = 1
                end
            end
        end
    end
    if input.button_timeout then 
        if input.button_timeout < millis() then  
            for idx=1,MAX_BLE_BUTTONS do
                input.forced_buttons[idx-pdButtonIdOffset] = nil
            end
            input.button_timeout = 0
        else 
            for btn,state in pairs(input.forced_buttons) do  
                actions[btn-pdButtonIdOffset] = state
            end
        end
    end
    for idx=1,MAX_BLE_BUTTONS do
        input.updateButtonByreading(pdButtonIdOffset+idx, actions[idx])
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

function press(key, controller)
    controller = controller or 1
    input.forced_buttons[key + (controller-1) * MAX_BLE_BUTTONS] = _G.BUTTON_JUST_PRESSED
    input.button_timeout = millis() + 250
end

_G.press = press

function input.update()
    if input.mode == "INFRARED" then  

        local usercode, opcode = getLastIRCommand()
        if usercode ~= 0 then  
            local binds = input.infrared[usercode]
            if binds then  
                input.runBindCommand(binds, opcode)
            else 
                log(string.format("Unmapped IR command with usercode %02X and opcode %02X", usercode, opcode))
            end
        end
    end

    local pdButtonIdOffset = 0
    for i=0,MAX_BLE_CLIENTS-1 do 
        local controllerList = drivers.type_by_id[i]
        if controllerList ~= nil then
            input.updatGenericButtonStates(controllerList.mode, i, pdButtonIdOffset)
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