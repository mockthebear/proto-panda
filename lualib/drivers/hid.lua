local hid = {
    hidLayouts = {},
    reportIdByHandle={},
    type="core",
    mode = {'joystick', 'mouse', 'keyboard', 'consumer'},
}

local DEBUG_INPUT = true
local DEBUG_LOG = print

local function buildKeyTables(entries)
    local nameOf, codeOf = {}, {}
    for _, e in ipairs(entries) do
        nameOf[e[1]] = e[2]
        for i = 3, #e do codeOf[e[i]] = e[1] end
    end
    return nameOf, codeOf
end

local MODIFIER_NAMES = {
    [0]="LCtrl", [1]="LShift", [2]="LAlt", [3]="LGui",
    [4]="RCtrl", [5]="RShift", [6]="RAlt", [7]="RGui",
}

local keyboardEntries = {
    {40, "Enter", "enter", "return"}, {41, "Esc", "esc", "escape"}, {42, "Backspace", "backspace"},
    {43, "Tab", "tab"}, {44, "Space", "space"},
    {45, "-"}, {46, "="}, {47, "["}, {48, "]"}, {49, "\\"}, {50, "# (non-US)"},
    {51, ";"}, {52, "'"}, {53, "`"}, {54, ","}, {55, "."}, {56, "/"},
    {57, "CapsLock", "capslock"},
    {70, "PrintScreen"}, {71, "ScrollLock"}, {72, "Pause"},
    {73, "Insert", "insert"}, {74, "Home", "home"}, {75, "PageUp", "pageup"},
    {76, "Delete", "delete"}, {77, "End", "end"}, {78, "PageDown", "pagedown"},
    {79, "Right", "right"}, {80, "Left", "left"}, {81, "Down", "down"}, {82, "Up", "up"},
    {83, "NumLock"},
    {84, "Keypad /"}, {85, "Keypad *"}, {86, "Keypad -"}, {87, "Keypad +"}, {88, "Keypad Enter"},
    {98, "Keypad 0"}, {99, "Keypad ."}, {100, "\\ (non-US)"}, {101, "Menu"},
}
for i = 0, 25 do keyboardEntries[#keyboardEntries + 1] = {4 + i, string.char(65 + i), string.char(97 + i)} end 
for i = 1, 9 do keyboardEntries[#keyboardEntries + 1] = {29 + i, tostring(i), tostring(i)} end                
keyboardEntries[#keyboardEntries + 1] = {39, "0", "0"}
for i = 1, 12 do keyboardEntries[#keyboardEntries + 1] = {57 + i, "F" .. i, "f" .. i} end                  
for i = 1, 9 do keyboardEntries[#keyboardEntries + 1] = {88 + i, "Keypad " .. i} end                        

local consumerEntries = {
    {0x30, "Power", "power"},
    {0x40, "Menu", "menu"}, {0x41, "Menu Pick", "menupick"}, {0x42, "Menu Up", "menuup"},
    {0x43, "Menu Down", "menudown"}, {0x44, "Menu Left", "menuleft"}, {0x45, "Menu Right", "menuright"},
    {0x46, "Menu Escape", "menuescape"},
    {0x6F, "Brightness Up", "brightnessup"}, {0x70, "Brightness Down", "brightnessdown"},
    {0xB0, "Play"}, {0xB1, "Pause", "pause"},
    {0xB3, "Fast Forward", "ff", "fastforward"}, {0xB4, "Rewind", "rewind", "rew"},
    {0xB5, "Next Track", "next"}, {0xB6, "Previous Track", "prev", "previous"},
    {0xB7, "Stop", "stop"}, {0xB8, "Eject", "eject"},
    {0xCD, "PlayPause", "play", "playpause"}, {0xE2, "Mute", "mute"},
    {0xE9, "Volume Up", "volumeup", "volup"}, {0xEA, "Volume Down", "volumedown", "voldown"},
    {0x18A, "Mail", "mail"}, {0x192, "Calculator", "calculator"},
    {0x221, "AC Search", "search"}, {0x223, "AC Home", "home"},
    {0x224, "AC Back", "back"}, {0x225, "AC Forward", "forward"},
}

local KEY_NAMES, KEYBOARD_KEY_NAMES = buildKeyTables(keyboardEntries)
local CONSUMER_NAMES, CONSUMER_KEY_NAMES = buildKeyTables(consumerEntries)

function hid.getDriverModules(maxClients)
    local hidData = {
        mouse = {},
        keyboard = {},
        consumer = {},
        joystick = {},
    }
    for i=0,maxClients-1 do  
        hidData.mouse[i] = {
            x=0,
            y=0,
            wheel=0,
            buttons={0,0,0,0,0,0,0,0}
        }
        hidData.keyboard[i] = {
            button = 0,
            held = {},
            keys = {}, 
            modifiers = 0,
        }
        hidData.consumer[i] = {
            button = 0,
            held = {},
        }
         hidData.joystick[i] = {  

            buttons = 0,
            button = 0, 

            hat = -1, 
            hat_up = 0, 
            hat_right = 0, 
            hat_down = 0, 
            hat_left = 0,

            left_analog_x = 0,
            left_analog_y = 0,
            right_analog_x = 0,
            right_analog_y = 0,

            stick_x = 0,
            stick_y = 0,
            gyro = {x=0,y=0,z=0},
            accelerometer = {x=0,y=0,z=0},
        }
    end 
    hid.modules = hidData
    return hidData
end

function hid.getKeyNames()
    return {
        keyboard = KEYBOARD_KEY_NAMES,
        consumer = CONSUMER_KEY_NAMES,
    }
end



local function toSigned(v, size)
    local bits = size * 8
    if bits > 0 and v >= (1 << (bits - 1)) then return v - (1 << bits) end
    return v
end

local function kindOf(page, usage)
    if page == 0x01 then
        if usage == 0x02 then return "mouse" end
        if usage == 0x06 or usage == 0x07 then return "keyboard" end
        if usage == 0x04 or usage == 0x05 then return "joystick" end  
    elseif page == 0x0C then
        return "consumer"
    end
    return "other"
end

function hid.parseReportMap(map)
    local layout = { reports = {}, order = {}, byLength = {} }
    local g = { page = 0, lmin = 0, lmaxRaw = 0, lmaxSize = 1, size = 0, count = 0, id = 0 }
    local stack = {}
    local usages, umin, umax = {}, nil, nil
    local depth, app = 0, nil
    local pos, n = 1, #map

    while pos <= n do
        local prefix = map[pos]
        pos = pos + 1
        if prefix == 0xFE then
            pos = pos + 2 + (map[pos] or 0)
        else
            local size = prefix & 3
            if size == 3 then size = 4 end
            local typ, tag = (prefix >> 2) & 3, prefix >> 4
            if pos + size - 1 > n then break end
            local raw = 0
            for i = 0, size - 1 do raw = raw | (map[pos + i] << (8 * i)) end
            pos = pos + size

            if typ == 1 then
                if     tag == 0x0 then g.page = raw
                elseif tag == 0x1 then g.lmin = toSigned(raw, size)
                elseif tag == 0x2 then g.lmaxRaw, g.lmaxSize = raw, size
                elseif tag == 0x7 then g.size = raw
                elseif tag == 0x8 then g.id = raw
                elseif tag == 0x9 then g.count = raw
                elseif tag == 0xA then
                    local copy = {}
                    for k, v in pairs(g) do copy[k] = v end
                    stack[#stack + 1] = copy
                elseif tag == 0xB then
                    g = table.remove(stack) or g
                end
            elseif typ == 2 then
                if     tag == 0x0 then usages[#usages + 1] = raw & 0xFFFF
                elseif tag == 0x1 then umin = raw & 0xFFFF
                elseif tag == 0x2 then umax = raw & 0xFFFF end
            else
                if tag == 0xA then
                    depth = depth + 1
                    if depth == 1 then app = { page = g.page, usage = usages[1] or umin or 0 } end
                elseif tag == 0xC then
                    if depth > 0 then depth = depth - 1 end
                elseif tag == 0x8 then
                    local rep = layout.reports[g.id]
                    if not rep then
                        rep = { id = g.id, bits = 0, fields = {},
                                kind = kindOf(app and app.page or 0, app and app.usage or 0) }
                        layout.reports[g.id] = rep
                        layout.order[#layout.order + 1] = rep
                    end
                    local isConst = (raw & 1) ~= 0
                    local isVar   = (raw & 2) ~= 0
                    local lmax = (g.lmin < 0) and toSigned(g.lmaxRaw, g.lmaxSize) or g.lmaxRaw
                    if not isConst then
                        if isVar then
                            for i = 1, g.count do
                                local u = 0
                                if #usages > 0 then
                                    u = usages[i] or usages[#usages]
                                elseif umin then
                                    u = umin + i - 1
                                    if umax and u > umax then u = umax end
                                end
                                rep.fields[#rep.fields + 1] = {
                                    page = g.page, usage = u, size = g.size,
                                    offset = rep.bits + (i - 1) * g.size,
                                    lmin = g.lmin, lmax = lmax }
                            end
                        else
                            rep.fields[#rep.fields + 1] = {
                                page = g.page, array = true, count = g.count, size = g.size,
                                offset = rep.bits, lmin = g.lmin, lmax = lmax,
                                base = (umin or usages[1] or 0) - g.lmin }
                        end
                    end
                    rep.bits = rep.bits + g.count * g.size
                end
                usages, umin, umax = {}, nil, nil
            end
        end
    end

    for _, rep in ipairs(layout.order) do
        if #rep.fields > 0 then
            rep.bytes = (rep.bits + 7) // 8
            local list = layout.byLength[rep.bytes]
            if not list then list = {}; layout.byLength[rep.bytes] = list end
            list[#list + 1] = rep
        end
    end
    return layout
end

function hid.describeReportMap(layout)
    local parts = {}
    for _, rep in ipairs(layout.order) do
        if rep.bytes then
            parts[#parts + 1] = string.format("id%d=%s/%dB", rep.id, rep.kind, rep.bytes)
        end
    end
    return table.concat(parts, " ")
end


local function getBits(data, offset, size)
    local v = 0
    for i = 0, size - 1 do
        local bit = offset + i
        local byte = data[(bit >> 3) + 1] or 0
        v = v | (((byte >> (bit & 7)) & 1) << i)
    end
    return v
end

local function fieldValue(data, f)
    local v = getBits(data, f.offset, f.size)
    if f.lmin < 0 and v >= (1 << (f.size - 1)) then v = v - (1 << f.size) end
    return v
end

local function arrayUsage(f, v)
    if v < f.lmin or v > f.lmax then return nil end
    local u = v + f.base
    if u == 0 then return nil end
    return u
end


local function handleMouse(rep, controllerId, data)
    local mouse = drivers.inputs.mouse[controllerId]
    if not mouse then return end
    local dx, dy, wheel, hwheel = 0, 0, 0, 0
    local hasX, hasY, hasWheel, hasPan = false, false, false, false
    local mb = mouse.buttons
    local action, empty = "", true

    for _, f in ipairs(rep.fields) do
        if f.page == 0x09 then
            local state = (fieldValue(data, f) ~= 0) and 1 or 0
            if mb[f.usage] ~= state then
                action = action .. ('mouse.buttons.' .. f.usage .. ' -> ' .. state .. ' ')
                empty = false
                mb[f.usage] = state
            end
        elseif f.page == 0x01 then
            if     f.usage == 0x30 then dx = fieldValue(data, f); hasX = true
            elseif f.usage == 0x31 then dy = fieldValue(data, f); hasY = true
            elseif f.usage == 0x38 then wheel = fieldValue(data, f); hasWheel = true end
        elseif f.page == 0x0C and f.usage == 0x238 then
            hwheel = fieldValue(data, f); hasPan = true
        end
    end

    if hasX then mouse.x = dx end
    if hasY then mouse.y = dy end
    if hasWheel then mouse.wheel = wheel end
    if hasPan then mouse.hwheel = hwheel end

    if DEBUG_INPUT then
        if dx ~= 0 then action = action .. ('mouse.x=' .. dx .. ' '); empty = false end
        if dy ~= 0 then action = action .. ('mouse.y=' .. dy .. ' '); empty = false end
        if wheel ~= 0 then action = action .. ('mouse.wheel=' .. wheel .. ' '); empty = false end
        if hwheel ~= 0 then action = action .. ('mouse.hwheel=' .. hwheel .. ' '); empty = false end
        if action ~= "" then DEBUG_LOG(action) end
        if empty then DEBUG_LOG("Mouse all zeros.") end
    end
end


local function describeKeys(mods, keys)
    local out = {}
    for bit = 0, 7 do
        if mods & (1 << bit) ~= 0 then out[#out + 1] = MODIFIER_NAMES[bit] end
    end
    for _, usage in ipairs(keys) do
        out[#out + 1] = KEY_NAMES[usage] or string.format("0x%02X", usage)
    end
    if #out == 0 then return "released" end
    return table.concat(out, "+")
end

local function handleKeyboard(rep, controllerId, data)
    local kb = drivers.inputs.keyboard[controllerId]
    if not kb then return end
    local mods = 0
    local keys = kb.keys or {}
    for i = #keys, 1, -1 do keys[i] = nil end

    for _, f in ipairs(rep.fields) do
        if f.page == 0x07 then
            if f.array then
                for i = 0, f.count - 1 do
                    local u = arrayUsage(f, getBits(data, f.offset + i * f.size, f.size))
                    if u and u > 3 then keys[#keys + 1] = u end
                end
            elseif fieldValue(data, f) ~= 0 then
                if f.usage >= 0xE0 and f.usage <= 0xE7 then
                    mods = mods | (1 << (f.usage - 0xE0))
                else
                    keys[#keys + 1] = f.usage
                end
            end
        end
    end

    local held = kb.held
    if not held then held = {}; kb.held = held end
    for k in pairs(held) do held[k] = nil end
    for _, u in ipairs(keys) do held[u] = true end

    kb.modifiers = mods
    kb.keys = keys
    kb.button = keys[1] or 0
    if DEBUG_INPUT then
        DEBUG_LOG("keyboard.modifiers=" .. mods .. " keys=" .. table.concat(keys, ",") .. "  -> " .. describeKeys(mods, keys))
    end
end

local function handleConsumer(rep, controllerId, data)
    local cons = drivers.inputs.consumer[controllerId]
    if not cons then return end
    local held = cons.held
    for k in pairs(held) do held[k] = nil end
    local first = 0

    for _, f in ipairs(rep.fields) do
        if f.page == 0x0C then
            if f.array then
                for i = 0, f.count - 1 do
                    local u = arrayUsage(f, getBits(data, f.offset + i * f.size, f.size))
                    if u then
                        held[u] = true
                        if first == 0 then first = u end
                    end
                end
            elseif fieldValue(data, f) ~= 0 then
                held[f.usage] = true
                if first == 0 then first = f.usage end
            end
        end
    end

    cons.button = first
    if DEBUG_INPUT then
        DEBUG_LOG(string.format("consumer usage=0x%04X  -> %s", first,
            first == 0 and "released" or (CONSUMER_NAMES[first] or "?")))
    end
end

local scratchAxes, scratchSim, scratchPressed = {}, {}, {}

local function handleJoystick(rep, controllerId, data)
    local joy = drivers.inputs.joystick[controllerId]
    if not joy then return end
    local axes, sim, pressed = scratchAxes, scratchSim, scratchPressed
    for k in pairs(axes) do axes[k] = nil end
    for k in pairs(sim) do sim[k] = nil end
    for i = #pressed, 1, -1 do pressed[i] = nil end
    local hat, hatCount = nil, 8
    local mask = 0

    for _, f in ipairs(rep.fields) do
        if f.page == 0x09 then
            if fieldValue(data, f) ~= 0 then
                pressed[#pressed + 1] = f.usage
                if f.usage >= 1 and f.usage <= 63 then
                    mask = mask | (1 << (f.usage - 1))
                end
            end
        elseif f.page == 0x01 then
            if f.usage == 0x39 then
                local v = fieldValue(data, f)
                hat = (v >= f.lmin and v <= f.lmax) and (v - f.lmin) or nil
                hatCount = f.lmax - f.lmin + 1
            elseif f.usage >= 0x30 and f.usage <= 0x37 then
                axes[f.usage] = fieldValue(data, f) - ((f.lmin + f.lmax + 1) // 2)
            end
        elseif f.page == 0x02 then
            sim[f.usage] = fieldValue(data, f)
        end
    end

    joy.buttons = mask
    joy.button = mask

    local rx, ry
    if axes[0x32] and axes[0x35] then rx, ry = axes[0x32], axes[0x35]
    else rx, ry = axes[0x33], axes[0x34] end

    if axes[0x30] then joy.left_analog_x = axes[0x30] end
    if axes[0x31] then joy.left_analog_y = axes[0x31] end
    if rx then joy.right_analog_x = rx end
    if ry then joy.right_analog_y = ry end
    local eight = (hatCount == 8)
    joy.hat = hat or -1 
    joy.hat_up    = (eight and (hat == 7 or hat == 0 or hat == 1)) and 1 or 0
    joy.hat_right = (eight and (hat == 1 or hat == 2 or hat == 3)) and 1 or 0
    joy.hat_down  = (eight and (hat == 3 or hat == 4 or hat == 5)) and 1 or 0
    joy.hat_left  = (eight and (hat == 5 or hat == 6 or hat == 7)) and 1 or 0

    joy.hat_directions = hatCount
    if sim[0xC5] then joy.brake = sim[0xC5] end
    if sim[0xC4] then joy.accelerator = sim[0xC4] end

    if DEBUG_INPUT then
        local sig = string.format("joystick L(%s,%s) R(%s,%s) hat=%s brake=%s accel=%s buttons=%d [%s]",
            tostring(joy.left_analog_x), tostring(joy.left_analog_y),
            tostring(joy.right_analog_x), tostring(joy.right_analog_y),
            tostring(joy.hat), tostring(joy.brake), tostring(joy.accelerator),
            mask, table.concat(pressed, ","))
        if sig ~= joy.lastLog then
            joy.lastLog = sig
            DEBUG_LOG(sig)
        end
    end
end

local HANDLERS = {
    mouse = handleMouse, keyboard = handleKeyboard,
    consumer = handleConsumer, joystick = handleJoystick,
}

function hid.dispatchHidReport(controllerId, data, layout, reportId)
    local rep = reportId and layout.reports[reportId]
    if rep and rep.bytes ~= #data then rep = nil end

    if not rep then
        local list = layout.byLength[#data]
        if not list then return false end
        rep = list[1]
        if #list > 1 and not layout.warned then
            layout.warned = true
            DEBUG_LOG("HID: several reports have " .. #data .. " bytes and no report ID is available, using the first one (" .. rep.kind .. ")")
        end
    end

    local handler = HANDLERS[rep.kind]
    if not handler then return false end
    handler(rep, controllerId, data)
    return true
end



function hid.reportIdOf(connectionId, controllerId, handle)
    local cache = hid.reportIdByHandle[controllerId]
    if not cache then
        cache = {}
        hid.reportIdByHandle[controllerId] = cache
    end
    local id = cache[handle]
    if id == nil then
        id = -1
        local ok, value = hid.handler:ReadDescriptor(connectionId, handle, "2908")
        if ok and value and #value >= 1 then id = value[1] end
        cache[handle] = id
        DEBUG_LOG(string.format("HID characteristic handle 0x%04X is report ID %d", handle, id))
    end
    return id
end

function hid.onDisconnectHID(connectionId, controllerId, reason)
    DEBUG_LOG("Disconnected "..connectionId.." due ".. reason)
    hid.hidLayouts[controllerId] = nil
    hid.reportIdByHandle[controllerId] = nil
    drivers.DisconnectDevice(controllerId, 'hid')
end

function hid.onConnectHID(connectionId, controllerId, address, name)
    local matchedDriver, matchName = drivers.FindDriver(connectionId, controllerId, address, name)
    drivers.ConnectDevice(controllerId, address, matchName or 'hid')

    DEBUG_LOG("Connected conId="..connectionId.." controller="..controllerId.." addr=\""..address.."\" name=["..name.."] type="..(matchName or 'hid'))

    if matchName then  
        return
    end

    local ok, map = hid.handler:ReadFromCharacteristics(connectionId, "2a4b")
    if ok and map and #map > 0 then
        local hex = {}
        for i = 1, #map do hex[i] = string.format("%02X", map[i]) end
        DEBUG_LOG("HID report map hex: " .. table.concat(hex, " "))

        local good, layout = pcall(hid.parseReportMap, map)
        if good then
            hid.hidLayouts[controllerId] = layout
            DEBUG_LOG("HID report map: " .. hid.describeReportMap(layout))
        else
            DEBUG_LOG("HID report map parse failed: " .. tostring(layout))
        end
    else
        DEBUG_LOG("HID report map not available, using packet length heuristics")
    end
end

function hid.onHidCallback(connectionId, controllerId, data, handle)
    local controlling = drivers.type_by_id[controllerId]
    if controlling and controlling.processPackets then
        controlling.processPackets(connectionId, controllerId, data, handle)
        return
    end

    local layout = hid.hidLayouts[controllerId]
    if layout then
        -- The report ID is only needed when several reports have the same size
        local reportId = nil
        local sameSize = layout.byLength[#data]
        if handle and sameSize and #sameSize > 1 then
            reportId = hid.reportIdOf(connectionId, controllerId, handle)
        end
        if hid.dispatchHidReport(controllerId, data, layout, reportId) then
            return
        end
    end
    hid.legacyHidCallback(connectionId, controllerId, data)
end

local lastPacket = {}
local function diffPacket(data)
    local out = {}
    for i = 1, #data do
        if data[i] ~= lastPacket[i] then
            out[#out + 1] = string.format("[%d]%s>%d", i, tostring(lastPacket[i] or "-"), data[i])
        end
        lastPacket[i] = data[i]
    end
    if #out > 0 then print("changed: " .. table.concat(out, " ")) end
end

function hid.legacyHidCallback(connectionId, controllerId, data)
    diffPacket(data)

    local len = #data
    local action = ""
    if len == 2 then
        --Mouse press
        if DEBUG_INPUT then
            DEBUG_LOG("keyboard.button="..  data[1] .. ' -> '.. data[2])
        end
        local keyboard = drivers.inputs.keyboard[controllerId]
        keyboard.button = data[1]
    elseif len == 4 or len == 3 then
        local empty = true
        local mouse = drivers.inputs.mouse[controllerId]

        local buttons = data[1]
        local deltaX = data[2]
        local deltaY = data[3]
        local deltaWheel = data[4] or 0
        if deltaX >= 128 then deltaX = deltaX - 256 end
        if deltaY >= 128 then deltaY = deltaY - 256 end
        if deltaWheel >= 128 then deltaWheel = deltaWheel - 256 end
        mouse.x = deltaX
        mouse.y = deltaY
        mouse.wheel = deltaWheel

        if deltaX ~= 0 then
            action = action ..('mouse.x='..deltaX..' ')
            empty = false
        end
        if deltaY ~= 0 then
            action = action ..('mouse.y='..deltaY..' ')
            empty = false
        end
        if deltaWheel ~= 0 then
            action = action ..('mouse.wheel='..deltaWheel..' ')
            empty = false
        end
        local mb = mouse.buttons
        for i=0,7 do
            local state = buttons & (1 << i) == 0 and 0 or 1
            if mb[i+1] ~= state then
                action = action ..('mouse.buttons.'..(i+1)..' -> '..state..' ')
                empty = false
                mb[i+1] = state
            end
        end

        if DEBUG_INPUT then
            if action ~= "" then
                DEBUG_LOG(action)
            end
            if empty then
                DEBUG_LOG("Mouse all zeros.")
            end
        end

    elseif len == 5 then
        local str = ""
        for i,b in pairs(data) do
            str = str .. b..', '
        end
        print(str)
    elseif len >= 6  then
        local joystickObject = drivers.inputs.joystick[controllerId]
        local buttonStates = ""
        local buttons = data[5]
        if buttons == 0 then
            joystickObject.button = 0
            joystickObject.buttons = 0
            buttonStates = " joystick.button=0"
        else
            joystickObject.button = buttons
            joystickObject.buttons = buttons
            buttonStates = " joystick.buttons="..buttons
        end

        joystickObject.left_hat = data[6] or 0
        joystickObject.right_hat = data[7] or 0

        joystickObject.left_analog_x = data[1] - 127
        joystickObject.left_analog_y = data[2] - 127

        joystickObject.right_analog_x = data[3] - 127
        joystickObject.right_analog_y = data[4] - 128

        if DEBUG_INPUT then
            DEBUG_LOG("Joystick moved: "..
                "  joystickObject.left_hat="..joystickObject.left_hat ..
                ", joystickObject.right_hat="..joystickObject.right_hat ..
                ", joystickObject.left_analog_x="..joystickObject.left_analog_x..
                ", joystickObject.left_analog_y="..joystickObject.left_analog_y..
                ", joystickObject.right_analog_y="..joystickObject.right_analog_y..' '..buttonStates)
        end

    else

        local str = ""
        for i,b in pairs(data) do
            str = str ..b..', '
        end
        DEBUG_LOG("Packet size is unknown: "..(#data)..", data is: "..str)
    end
end


function hid.onEnable(self)
    if not hasBLEStarted() then
        return false
    end
    hid.handler = BleServiceHandler("00001812-0000-1000-8000-00805f9b34fb")
    hid.handler:SetOnConnectCallback(hid.onConnectHID)
    hid.handler:SetOnDisconnectCallback(hid.onDisconnectHID)
    hid.handler:SetEncryptionRequired(true)
    hid.mouseListener = hid.handler:AddCharacteristics("2a4d")
    hid.mouseListener:SetSubscribeCallback(hid.onHidCallback) 
    hid.mouseListener:SetCallbackModeStream(false)
    return true
end


return hid