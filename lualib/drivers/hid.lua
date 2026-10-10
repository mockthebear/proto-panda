local hid = {
    hidLayouts = {},
    type="core",
    mode = {'joystick', 'mouse', 'keyboard'},
}

local DEBUG_INPUT = true
function hid.getDriverModules(maxClients)
    local hidData = {
        mouse = {},
        keyboard = {},
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
            button = 0
        }
         hidData.joystick[i] = {  
            -- Bitmask of the pressed buttons: HID button N is bit N-1,
            -- so buttons 1 and 3 together read as 5. Use it with e.g. "joystick.buttons&4".
            buttons = 0,
            button = 0,     -- same value, also written by the legacy (no report map) path

            hat = -1,       -- 0=N 1=NE 2=E 3=SE 4=S 5=SW 6=W 7=NW, -1 when released
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



local function toSigned(v, size)
    local bits = size * 8
    if bits > 0 and v >= (1 << (bits - 1)) then return v - (1 << bits) end
    return v
end

local function kindOf(page, usage)
    if page == 0x01 then
        if usage == 0x02 then return "mouse" end
        if usage == 0x06 or usage == 0x07 then return "keyboard" end
        if usage == 0x04 or usage == 0x05 then return "joystick" end  -- joystick / gamepad
    elseif page == 0x0C then
        return "consumer"
    end
    return "other"
end

-- map: table of bytes (1-indexed), as returned by ReadFromCharacteristics(..., "2a4b")
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
        if prefix == 0xFE then                       -- long item, skip
            pos = pos + 2 + (map[pos] or 0)
        else
            local size = prefix & 3
            if size == 3 then size = 4 end
            local typ, tag = (prefix >> 2) & 3, prefix >> 4
            if pos + size - 1 > n then break end
            local raw = 0
            for i = 0, size - 1 do raw = raw | (map[pos + i] << (8 * i)) end
            pos = pos + size

            if typ == 1 then                          -- global items
                if     tag == 0x0 then g.page = raw
                elseif tag == 0x1 then g.lmin = toSigned(raw, size)
                elseif tag == 0x2 then g.lmaxRaw, g.lmaxSize = raw, size
                elseif tag == 0x7 then g.size = raw
                elseif tag == 0x8 then g.id = raw
                elseif tag == 0x9 then g.count = raw
                elseif tag == 0xA then                -- push
                    local copy = {}
                    for k, v in pairs(g) do copy[k] = v end
                    stack[#stack + 1] = copy
                elseif tag == 0xB then                -- pop
                    g = table.remove(stack) or g
                end
            elseif typ == 2 then                      -- local items
                if     tag == 0x0 then usages[#usages + 1] = raw & 0xFFFF
                elseif tag == 0x1 then umin = raw & 0xFFFF
                elseif tag == 0x2 then umax = raw & 0xFFFF end
            else                                      -- main items
                if tag == 0xA then                    -- collection
                    depth = depth + 1
                    if depth == 1 then app = { page = g.page, usage = usages[1] or umin or 0 } end
                elseif tag == 0xC then                -- end collection
                    if depth > 0 then depth = depth - 1 end
                elseif tag == 0x8 then                -- input
                    local rep = layout.reports[g.id]
                    if not rep then
                        rep = { id = g.id, bits = 0, fields = {},
                                kind = kindOf(app and app.page or 0, app and app.usage or 0) }
                        layout.reports[g.id] = rep
                        layout.order[#layout.order + 1] = rep
                    end
                    local isConst = (raw & 1) ~= 0
                    local isVar   = (raw & 2) ~= 0
                    -- logical max is only signed when logical min is negative
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
                        else                          -- array (e.g. keyboard keys, consumer usage)
                            rep.fields[#rep.fields + 1] = {
                                page = g.page, array = true, count = g.count, size = g.size,
                                offset = rep.bits, lmin = g.lmin, lmax = lmax,
                                base = (umin or usages[1] or 0) - g.lmin }
                        end
                    end
                    rep.bits = rep.bits + g.count * g.size
                end
                usages, umin, umax = {}, nil, nil     -- local items reset after every main item
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

-- usage code held by an array slot, or nil when the slot is empty / out of range
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
    local mb = mouse.buttons
    local action, empty = "", true

    for _, f in ipairs(rep.fields) do
        if f.page == 0x09 then                                    -- buttons
            local state = (fieldValue(data, f) ~= 0) and 1 or 0
            if mb[f.usage] ~= state then
                action = action .. ('mouse.buttons.' .. f.usage .. ' -> ' .. state .. ' ')
                empty = false
                mb[f.usage] = state
            end
        elseif f.page == 0x01 then
            if     f.usage == 0x30 then dx = fieldValue(data, f)
            elseif f.usage == 0x31 then dy = fieldValue(data, f)
            elseif f.usage == 0x38 then wheel = fieldValue(data, f) end
        elseif f.page == 0x0C and f.usage == 0x238 then           -- AC Pan (horizontal wheel)
            hwheel = fieldValue(data, f)
        end
    end

    mouse.x, mouse.y, mouse.wheel, mouse.hwheel = dx, dy, wheel, hwheel

    if DEBUG_INPUT then
        if dx ~= 0 then action = action .. ('mouse.x=' .. dx .. ' '); empty = false end
        if dy ~= 0 then action = action .. ('mouse.y=' .. dy .. ' '); empty = false end
        if wheel ~= 0 then action = action .. ('mouse.wheel=' .. wheel .. ' '); empty = false end
        if action ~= "" then log(action) end
        if empty then log("Mouse all zeros.") end
    end
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
                    if u and u > 3 then keys[#keys + 1] = u end   -- 1..3 are rollover/error codes
                end
            elseif fieldValue(data, f) ~= 0 then
                if f.usage >= 0xE0 and f.usage <= 0xE7 then
                    mods = mods | (1 << (f.usage - 0xE0))         -- ctrl/shift/alt/gui, left then right
                else
                    keys[#keys + 1] = f.usage                     -- bitmap style keyboards
                end
            end
        end
    end

    kb.modifiers = mods
    kb.keys = keys
    kb.button = keys[1] or 0
    if DEBUG_INPUT then
        log("keyboard.modifiers=" .. mods .. " keys=" .. table.concat(keys, ","))
    end
end

local function handleConsumer(rep, controllerId, data)
    local kb = drivers.inputs.keyboard[controllerId]
    if not kb then return end
    local usage = 0
    for _, f in ipairs(rep.fields) do
        if f.page == 0x0C then
            if f.array then
                for i = 0, f.count - 1 do
                    local u = arrayUsage(f, getBits(data, f.offset + i * f.size, f.size))
                    if u then usage = u; break end
                end
            elseif fieldValue(data, f) ~= 0 then
                usage = f.usage
            end
        end
    end
    kb.consumer = usage              -- full 16 bit usage (0xE9 volume up, 0xCD play/pause, ...)
    kb.button = usage & 0xFF         -- what your old 2-byte branch stored (data[1])
    if DEBUG_INPUT then
        log(string.format("consumer usage=0x%04X", usage))
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
        if f.page == 0x09 then                                    -- buttons 1..N
            if fieldValue(data, f) ~= 0 then
                pressed[#pressed + 1] = f.usage
                if f.usage >= 1 and f.usage <= 63 then
                    mask = mask | (1 << (f.usage - 1))            -- button N -> bit N-1
                end
            end
        elseif f.page == 0x01 then
            if f.usage == 0x39 then                               -- hat switch
                local v = fieldValue(data, f)
                hat = (v >= f.lmin and v <= f.lmax) and (v - f.lmin) or nil   -- nil = released
                hatCount = f.lmax - f.lmin + 1
            elseif f.usage >= 0x30 and f.usage <= 0x37 then       -- X Y Z Rx Ry Rz Slider Dial
                axes[f.usage] = fieldValue(data, f) - ((f.lmin + f.lmax + 1) // 2)   -- centre = 0
            end
        elseif f.page == 0x02 then                                -- simulation: accelerator, brake...
            sim[f.usage] = fieldValue(data, f)
        end
    end

    joy.buttons = mask
    joy.button = mask

    -- Right stick is Z/Rz on most pads (Xbox style, Q36); otherwise Rx/Ry.
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
    if sim[0xC5] then joy.brake = sim[0xC5] end              -- usually the left trigger
    if sim[0xC4] then joy.accelerator = sim[0xC4] end        -- usually the right trigger

    if DEBUG_INPUT then
        local sig = string.format("joystick L(%s,%s) R(%s,%s) hat=%s brake=%s accel=%s buttons=%d [%s]",
            tostring(joy.left_analog_x), tostring(joy.left_analog_y),
            tostring(joy.right_analog_x), tostring(joy.right_analog_y),
            tostring(joy.hat), tostring(joy.brake), tostring(joy.accelerator),
            mask, table.concat(pressed, ","))
        if sig ~= joy.lastLog then
            joy.lastLog = sig
            log(sig)
        end
    end
end

local HANDLERS = {
    mouse = handleMouse, keyboard = handleKeyboard,
    consumer = handleConsumer, joystick = handleJoystick,
}

-- true when the packet was decoded through the report map
function hid.dispatchHidReport(controllerId, data, layout)
    local list = layout.byLength[#data]
    if not list then return false end
    local rep = list[1]
    local handler = HANDLERS[rep.kind]
    if not handler then return false end
    if #list > 1 and not layout.warned then
        layout.warned = true
        log("HID: several reports have " .. #data .. " bytes, using the first one (" .. rep.kind .. ")")
    end
    handler(rep, controllerId, data)
    return true
end

function hid.onDisconnectHID(connectionId, controllerId, reason)
    log("Disconnected "..connectionId.." due ".. reason)
    hid.hidLayouts[controllerId] = nil
    drivers.DisconnectDevice(controllerId, 'hid')
end

function hid.onConnectHID(connectionId, controllerId, address, name)
    local matchedDriver, matchName = drivers.FindDriver(connectionId, controllerId, address, name)
    drivers.ConnectDevice(controllerId, address, matchName or 'hid')

    log("Connected conId="..connectionId.." controller="..controllerId.." addr=\""..address.."\" name=["..name.."] type="..(matchName or 'hid'))

    if matchName then  
        return
    end

    local ok, map = hid.handler:ReadFromCharacteristics(connectionId, "2a4b")
    if ok and map and #map > 0 then
        local hex = {}
        for i = 1, #map do hex[i] = string.format("%02X", map[i]) end
        log("HID report map hex: " .. table.concat(hex, " "))

        local good, layout = pcall(hid.parseReportMap, map)
        if good then
            hid.hidLayouts[controllerId] = layout
            log("HID report map: " .. hid.describeReportMap(layout))
        else
            log("HID report map parse failed: " .. tostring(layout))
        end
    else
        log("HID report map not available, using packet length heuristics")
    end
end

function hid.onHidCallback(connectionId, controllerId, data)
    local controlling = drivers.type_by_id[controllerId]
    if controlling and controlling.processPackets then
        controlling.processPackets(connectionId, controllerId, data)
        return
    end

    local layout = hid.hidLayouts[controllerId]
    if layout and hid.dispatchHidReport(controllerId, data, layout) then
        return
    end
    hid.legacyHidCallback(connectionId, controllerId, data)
end


-- Debug helper: prints only the byte positions that changed since the previous packet,
-- e.g. "changed: [2]64>65 [5]19>0"  ([index]old>new, 1-based)
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
            log("keyboard.button="..  data[1] .. ' -> '.. data[2])
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
                log(action)
            end
            if empty then
                log("Mouse all zeros.")
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
            log("Joystick moved: "..
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
        log("Packet size is unknown: "..(#data)..", data is: "..str)
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