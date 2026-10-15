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

    maxControls=2,

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
_G.BUTTON_BACK = 6
_G.BUTTON_AUX_A = 7
_G.BUTTON_AUX_B = 8

local function testBit(n, pos)
    return (n >> pos) % 2 == 1
end

input.keyNames = {}
local bindCache = {} 

-- ignore case, spaces, _ and -
local function normalizeName(name)
    local key = tostring(name):lower()
    key = key:gsub("[%s_%-]", "")
    return key
end

function input.loadKeyNames()
    bindCache = {}   -- binds parsed before the names were known must be parsed again
    for driverName, module in pairs(drivers.loaded or {}) do
        if type(module) == 'table' and type(module.getKeyNames) == 'function' then
            local ok, byType = pcall(module.getKeyNames)
            if ok and type(byType) == 'table' then
                for inputType, names in pairs(byType) do
                    local merged = input.keyNames[inputType]
                    if not merged then
                        merged = {}
                        input.keyNames[inputType] = merged
                    end
                    for name, code in pairs(names) do
                        merged[normalizeName(name)] = code
                    end
                end
            else
                log("Driver '"..tostring(driverName).."' getKeyNames failed: "..tostring(byType))
            end
        end
    end
end

local function resolveKeyName(controllerType, token)
    local names = input.keyNames[controllerType]
    local key = normalizeName(token)
    if names and names[key] then
        return names[key]
    end
    local number = tonumber(token)
    if number then
        return number
    end
    return nil, "there is no key named '"..tostring(token).."' for '"..tostring(controllerType).."'"
end

-- Text used in the "[KEYBIND] Mapped ..." log line for each match mode
local MODE_LOG_TEXT = {
    equals  = "equals",
    greater = "is greater than",
    lesser  = "is lesser than",
    mask    = "has pressed button number",
}

--[[
    Keybind syntax (the key of each entry in "keybinds"). A line that can not be resolved (for example
    a key name no driver defines) is logged as "[KEYBIND] Skipped ..." and ignored, the others still load:

        <type>.<resource>.<index>     e.g. panda.buttons.1       value at a table index
        <type>.<resource>=<value>     e.g. joystick.hat=6        equals
        <type>.<resource>><number>    e.g. mouse.x>10            greater than
        <type>.<resource><number>     e.g. mouse.x<-10           lesser than
        keyboard.button=<key>         e.g. keyboard.button=a, =up, =enter, =f1, =1   key held (any combination)
        consumer.button=<key>         e.g. consumer.button=play, =next, =volumeup    media key held
        <type>.<resource>&<n>         e.g. joystick.button&4     bit position: active while the n-th
                                                                 button is pressed (1 = first button,
                                                                 2 = second, 4 = fourth...). The value
                                                                 is a bitmask, so with buttons 1 and 3
                                                                 held (value 5) both &1 and &3 are active.
]]
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
            if not drivers.inputs[element] then  
                return nil, nil, "There is no driver for the type '"..element.."' in ["..str.."] skipping it."
            end
            controllerType = element
        elseif idx == 2 then  
            local handler = drivers.inputs[controllerType][0]
            local toMatch = nil

            if not handler then  
                error("Unavalible resource '"..controllerType.."'")
            end
            
            local mode = 'none'
            if element:match(".-=.+") then 
                mode = 'equals'
                element, toMatch = element:match("(.-)=(.+)")
            elseif element:match(".->.+") then 
                mode = 'greater'
                element, toMatch = element:match("(.-)>(.+)")
                toMatch = tonumber(toMatch)
                if not toMatch then 
                    error("Unavalible resource '"..element.."' in '"..str.."'. is expected to be a number")
                end
            elseif element:match(".-<.+") then 
                mode = 'lesser'
                element, toMatch = element:match("(.-)<(.+)")
                toMatch = tonumber(toMatch)
                if not toMatch then 
                    error("Unavalible resource '"..element.."' in '"..str.."'. is expected to be a number")
                end
            elseif element:match(".-&.+") then 
                mode = 'mask'
                element, toMatch = element:match("(.-)&(.+)")
                toMatch = tonumber(toMatch)
                if not toMatch or math.type(toMatch) ~= 'integer' or toMatch < 1 or toMatch > 32 then 
                    error("Unavalible resource '"..element.."' in '"..str.."'. the number after & is expected to be a button number from 1 to 32")
                end
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
                local reference = drivers.inputs[controllerType][i][element]
                if type(reference) == 'table' then  
                    parsed.reference = true
                    parsed.resource[i] = reference
                else
                    parsed.resource[i] = element
                end
            end
            
            if toMatch then  
                -- keyboard / consumer keys: accept names and match against every key that is held
                if mode == 'equals' and element == 'button' and input.keyNames[controllerType] then
                    local code, why = resolveKeyName(controllerType, tostring(toMatch))
                    if not code then
                        return nil, controllerType, why          -- not fatal: the caller logs and skips this line
                    end
                    toMatch = code
                    parsed.held = true
                end
                -- a word compared with a numeric resource can never match (for instance when no driver defines key names)
                if mode == 'equals' and type(handler[element]) == 'number' and tonumber(toMatch) == nil then
                    return nil, controllerType, "'"..tostring(toMatch).."' is not a number, but '"..controllerType.."."..element.."' holds numbers (is the key name defined by a driver?)"
                end
                parsed.match = tonumber(toMatch) or toMatch
                parsed.mode = mode
                mappedLogString = "'"..controllerType.."' when '"..element.."' "..(MODE_LOG_TEXT[mode] or "equals").." '"..parsed.match.."'"
                break
            end
        elseif idx == 3 then
            mappedLogString = "'"..controllerType.."' directly on '"..parsed.name.."["..element.."]'"
            parsed.location = tonumber(element) or element
            break
        end
        idx = idx+1
    end

    local dataLocation = drivers.inputs[controllerType][0][parsed.name]
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

function input.setupControls(maxN)
    local pdButtonIdOffset = 1
    for i=0,maxN-1 do 
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
        _G['DEVICE_'..i..'_BUTTON_LAST']    = _G.BUTTON_AUX_B       + i * MAX_BLE_BUTTONS
    end
end

function input.Start()
    if input.mode == "BLE" then  
        beginBleScanning()
    elseif input.mode == "INFRARED" then  
        enableIRInterrupt()
    else
        error("Invalid input mode '"..input.mode.."'")
    end
end

function input.Load()
    local confs = configloader.Get()

    if not confs.input then  
        error("keybinds.json missing 'input'")
    end

    local mode = confs.input.mode or "BLE"
    mode = mode:upper()
    input.mode = mode

    if not confs.input.drivers or #confs.input.drivers == 0 then  
        confs.input.drivers = {"generic"}
    end

    local maxN = confs.input and (confs.input.maxBleDevices or 2) or 2
    _G.MAX_BLE_CLIENTS = maxN
    input.maxControls =  maxN
    input.setupControls(maxN)
    print("Gonna use "..maxN.." controllers")

    drivers.Start(maxN)

    if mode == "BLE" then
        setLogDiscoveredBleDevices(true)
        generic.displaySplashMessage("Starting:\nBLE")
        startBLE()
        setMaximumControls(maxN)
        startBLERadio(ESP_PWR_LVL_P9)
    elseif mode == "INFRARED" then
        generic.displaySplashMessage("Starting:\nIR")
        startIR()
        if not confs.infrared then  
            error("keybinds.json missing 'infrared' which is required when infrared is enabled")
        end

        if not confs.input.infrared_gpio then  
            error("keybinds.json missing 'infrared_gpio' in input segment")
        end

        if type(confs.input.infrared_gpio) ~= "number" then  
            error("keybinds.json missing 'infrared_gpio' in input segment")
        end

        setIRInterruptPin(confs.input.infrared_gpio)

        for i, mode in pairs(confs.infrared) do 
            if not mode.usercode or type(mode.bind) ~= 'table' then  
                error("keybinds.json error at element "..i..', \'usercode\' or \'bind\' is missing')
            end
            local code = tonumber(mode.usercode, 16)
            if not code then  
                error("keybinds.json error at element "..i..', \'usercode\' should be a HEX number')
            end
            local keymap = {}
            log("Loaded usercode "..mode.usercode)
            for opcodeS, func in pairs(mode.bind) do  
                local opcode = tonumber(opcodeS, 16)
                if not opcode then 
                    error("keybinds.json error at element "..i..".bind. Opcode "..opcodeS.." is not a valid HEX")
                end

                local f, err = load(func)
                if not f then  
                    error("keybinds.json error at element "..i..'.bind['..opcodeS..']: '..err)
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

    drivers.EnableDrivers(confs.input)


    drivers.WrapUp()

    input.loadKeyNames()

    input.SetKeybinds(confs.keybinds) 
end

function input.SetKeybinds(keybinds) 
    for location, target in pairs(keybinds) do  
        local binding, controllerType, logMsg = input.parseInputLocation(location)
        if not binding then
            log("[KEYBIND] Skipped '"..tostring(location).."' -> "..tostring(target)..": "..tostring(logMsg))
        else
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
end

-- Current reading (1 / 0) of one parsed bind for one controller.
-- `controller` is drivers.inputs[<type>], `clientId` the controller id.
local function evalBind(bind, controller, clientId)
    local element = bind.resource[clientId]
    local reading = 0
    if not bind.reference then  
        element = controller[clientId][element]
    end
    if bind.match then  
        if bind.mode == 'equals' then
            reading = element == bind.match and 1 or 0
            if reading == 0 and bind.held then
                -- not the first key, but it may still be held together with others (a + b)
                local held = controller[clientId].held
                if held and held[bind.match] then reading = 1 end
            end
        elseif bind.mode == 'greater' then 
            local n = tonumber(element) or 0
            if n > bind.match then  
                reading = 1
            else 
                reading = 0
            end
        elseif bind.mode == 'lesser' then 
            local n = tonumber(element) or 0
            if n < bind.match then  
                reading = 1
            else 
                reading = 0
            end
        elseif bind.mode == 'mask' then 
            -- active while the n-th button (bit n-1) of the value is set
            local n = math.tointeger(tonumber(element) or 0) or 0
            if testBit(n, bind.match - 1) then  
                reading = 1
            else 
                reading = 0
            end
        end
    elseif bind.location then 
        if type(element) ~= 'table' then  
            error(tostring(bind.resource[clientId]).." is looking for "..tostring(controller[clientId]).." at "..bind.location.." ref is "..tostring(bind.reference).." element = "..tostring(element))
        end
        reading = element[bind.location]
    end
    return reading
end

function input.updatGenericButtonStates(inputModes, clientId, pdButtonIdOffset)
    local keybind = input.keybind
    local actions = {}
    for b=1,MAX_BLE_BUTTONS do
        actions[b] = 0
    end

    for _, name in pairs(inputModes) do
        local controller = drivers.inputs[name]
        --Search for inputs on each type.. joystick, panda, mouse, keyboard etc
        if keybind[name] then
            for __, bind in pairs(keybind[name]) do
                local reading = evalBind(bind, controller, clientId)
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

local function isConnected(id)
    return type(id) == 'number' and id >= 0 and id < MAX_BLE_CLIENTS and drivers.type_by_id[id] ~= nil
end

-- parsed + cached expression, false when it can not be used (logged once)
local function getBind(expr)
    if type(expr) ~= 'string' then return false end
    local cached = bindCache[expr]
    if cached ~= nil then return cached end
    local ok, binding, controllerType, why = pcall(input.parseInputLocation, expr)
    if not ok then
        log("[INPUT] '"..expr.."': "..tostring(binding))
        binding = false
    elseif not binding then
        log("[INPUT] '"..expr.."' ignored: "..tostring(why))
        binding = false
    else
        binding.controllerType = controllerType
    end
    bindCache[expr] = binding
    return binding
end



local statusCache = {}

local function getStatusLocation(str)
    if type(str) ~= 'string' then return false end
    local cached = statusCache[str]
    if cached ~= nil then return cached end

    local parts = {}
    for part in str:gmatch("([^%.]+)") do parts[#parts + 1] = part end
    local controllerType, name, index = parts[1], parts[2], parts[3]
    local sample = controllerType and drivers.inputs[controllerType] and drivers.inputs[controllerType][0]

    local why
    if not sample then
        why = "there is no input type '"..tostring(controllerType).."'"
    elseif #parts > 3 or name == nil or sample[name] == nil then
        why = "'"..tostring(controllerType).."' has no resource '"..tostring(name).."'"
    elseif type(sample[name]) == 'table' and index == nil then
        why = "'"..name.."' is a table, use an index like '"..str..".1'"
    elseif type(sample[name]) ~= 'table' and index ~= nil then
        why = "'"..name.."' is not a table, it can not be indexed"
    end
    if why then
        log("[INPUT] readStatus('"..str.."'): "..why)
        statusCache[str] = false
        return false
    end

    local loc = { type = controllerType, name = name, index = index and (tonumber(index) or index) }
    statusCache[str] = loc
    return loc
end

local function statusValue(loc, id)
    local object = drivers.inputs[loc.type][id]
    if not object then return 0 end
    local v = object[loc.name]
    if loc.index ~= nil then
        if type(v) ~= 'table' then return 0 end
        v = v[loc.index]
    end
    if v == true then return 1 end
    if v == nil or v == false then return 0 end
    return tonumber(v) or 0
end

--   input.readStatus("mouse.x")                  -> -3
--   input.readStatus("joystick.left_analog_x")   -> 0
--   input.readStatus("mouse.buttons.1")          -> 1 / 0
function input.readStatus(location, controller)
    local loc = getStatusLocation(location)
    if not loc then return 0 end
    if controller ~= nil then
        return isConnected(controller) and statusValue(loc, controller) or 0
    end
    for id = 0, MAX_BLE_CLIENTS - 1 do
        if isConnected(id) then
            local v = statusValue(loc, id)
            if v ~= 0 then return v end
        end
    end
    return 0
end


--   input.readPressedStatus("keyboard.button", "up") 
--   input.readPressedStatus("consumer.button", "play")
--   input.readPressedStatus("mouse.x>10")
--   input.readPressedStatus("joystick.button&4")
--   input.readPressedStatus("keyboard.button", "a", 1)
function input.readPressedStatus(location, value, controller)
    local expr = location
    if value ~= nil then
        expr = tostring(location).."="..tostring(value)
    end
    local bind = getBind(expr)
    if not bind then return false end
    local data = drivers.inputs[bind.controllerType]
    if controller ~= nil then
        return isConnected(controller) and evalBind(bind, data, controller) == 1
    end
    for id = 0, MAX_BLE_CLIENTS - 1 do
        if isConnected(id) and evalBind(bind, data, id) == 1 then
            return true
        end
    end
    return false
end

function input.getKeyCode(controllerType, name)
    local code = resolveKeyName(controllerType, tostring(name))
    return code
end

function input.getHeldKeys(controllerType, controller)
    local out, seen = {}, {}
    local data = drivers.inputs[controllerType]
    if not data then return out end
    local function collect(id)
        local object = data[id]
        local held = object and object.held
        if held then
            for code, down in pairs(held) do
                if down and not seen[code] then
                    seen[code] = true
                    out[#out + 1] = code
                end
            end
        end
    end
    if controller ~= nil then
        if isConnected(controller) then collect(controller) end
    else
        for id = 0, MAX_BLE_CLIENTS - 1 do
            if isConnected(id) then collect(id) end
        end
    end
    table.sort(out)
    return out
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
    local dv = drivers.panda[id]
    if not dv then 
        return 0
    end
    return dv.last_update
end

function input.readAccelerometerX(id)
    if input.legacyreadAccelerometerX then  
        return input.legacyreadAccelerometerX(id)
    end
    local dv = drivers.panda[id]
    if not dv then 
        return 0
    end
    local v = dv.ax
    return v * ACCELEROMETER_MULTIPLIER
end

function input.readAccelerometerY(id)
    if input.legacyreadAccelerometery then  
        return input.legacyreadAccelerometery(id)
    end
    local dv = drivers.panda[id]
    if not dv then 
        return 0
    end
    local v = dv.ay
    return v * ACCELEROMETER_MULTIPLIER
end

function input.readAccelerometerZ(id)
    if input.legacyreadAccelerometerz then  
        return input.legacyreadAccelerometerz(id)
    end
    local dv = drivers.panda[id]
    if not dv then 
        return 0
    end
    local v = dv.az
    return v * ACCELEROMETER_MULTIPLIER
end

function input.readGyroX(id)
    if input.legacyreadGyroX then  
        return input.legacyreadGyroX(id)
    end
    local dv = drivers.panda[id]
    if not dv then 
        return 0
    end
    return dv.gx
end

function input.readGyroY(id)
    if input.legacyreadGyroY then  
        return input.legacyreadGyroY(id)
    end
    local dv = drivers.panda[id]
    if not dv then 
        return 0
    end
    return dv.gy
end

function input.readGyroZ(id)
    if input.legacyreadGyroZ then  
        return input.legacyreadGyroZ(id)
    end
    local dv = drivers.panda[id]
    if not dv then 
        return 0
    end
    return dv.gz
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