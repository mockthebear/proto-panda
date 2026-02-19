local versions = require("versions")

if not MAX_BLE_BUTTONS then
    _G.MAX_BLE_BUTTONS = 8
end

local drivers = {
    mouseHandler = nil,
    mouseListener = nil,

    maxButtons = MAX_BLE_BUTTONS,

    panda = {},
    joystick = {},
    mouse = {},
    keyboard = {},
    generic = {},

    validEntries = {
        ['joystick'] = true,
        ['mouse'] = true,
        ['keyboard'] = true,
        ['generic'] = true,

    },


    eventQueue = {},
    type_by_id = {},

    loaded = {},
    core = {},


    JOSYTICK_BUTTONS_MAP = {
        [1] = 1, --B
        [2] = 2, --D
        [8] = 3, --C
        [16] = 4, --A
    },
}

drivers.device_attribute_map = {
    ['hid'] = {'joystick', 'mouse', 'keyboard'},
}

do
    for i=0,MAX_BLE_CLIENTS-1 do   
        drivers.generic[i] = {
            timeout=0,
            buttons={0,0,0,0,0,0,0,0}
        }
        
        drivers.mouse[i] = {
            x=0,
            y=0,
            wheel=0,
            buttons={0,0,0,0,0,0,0,0}
        }
        drivers.joystick[i] = {  
            buttons = {},   
            left_hat = 0, 
            right_hat = 0,  
            left_analog_x = 0,
            left_analog_y = 0,
            right_analog_x = 0,
            right_analog_y = 0,
        }
        for __,b in pairs(drivers.JOSYTICK_BUTTONS_MAP) do  
            drivers.joystick[i].buttons[b] =0
        end
    end
end


function drivers.EnableDrivers(driversToLoad)
    for i,driverName in pairs(driversToLoad) do  
        local success, content = pcall(dofile,"/lualib/drivers/"..driverName..'.lua')
        if success and content then  
            if content.type == "hid" then
                if content.mode then  
                    for i, mode in pairs(content.mode) do 
                        if not drivers.validEntries[mode] then  
                            error("Invalid mode '"..mode.."' in driver "..driverName)
                        end
                    end
                    drivers.device_attribute_map[driverName] = content.mode
                else 
                    error("No mode set for "..driverName)
                end
                if drivers.loaded[driverName] then  
                    error("Duplicated driver declared: "..driverName)
                end
                content.attribute_map = drivers.device_attribute_map[driverName]
                drivers.loaded[driverName] = content
                print("Loaded hid driver "..driverName)
            elseif content.type == "core" then
                drivers.validEntries[driverName] = true
                drivers[driverName] = content.getDriverModules()
                drivers.core[driverName] = content
                print("Loaded core driver "..driverName)
            else 
                error("Undefined driver type "..tostring(content.type))
            end
        else 
            log("Failed to load driver '"..driverName.."' due "..tostring(content))
        end
    end

    drivers.EnableGenericAndroidMouse()
    for i,b in pairs(drivers.core) do  
        if not b.onEnable() then  
            log("Failed to enable core driver "..tostring(i))
        end
    end
end

function drivers.FindDriver(connectionId, controllerId, address, name)
    for driverName , loadedData in pairs(drivers.loaded) do  
        print("Seach in "..driverName)
        if loadedData.match then  
            local match = loadedData.match
            if match.byname then  
                if name and name:lower() == match.byname:lower() then  
                    return loadedData, driverName
                end
            elseif match.byaddress then  
                if address and address:lower() == match.byaddress then  
                    return loadedData, driverName
                end
            end
        end
    end
    return nil, nil
end

function drivers.onDisconnectHID(connectionId, controllerId, reason)
    log("Disconnected "..connectionId.." due ".. reason)
    drivers.type_by_id[controllerId] = nil
end

function drivers.onConnectHID(connectionId, controllerId, address, name)
    drivers.type_by_id[controllerId] = {mode=drivers.device_attribute_map["hid"]}
    local matchedDriver, matchName = drivers.FindDriver(connectionId, controllerId, address, name)
    matchName = matchName or "hid"
    if matchedDriver then  
        drivers.type_by_id[controllerId] = matchedDriver
    end

    log("Connected conId="..connectionId.." controller="..controllerId.." addr=\""..address.."\" name=["..name.."] type="..matchName)
end

function drivers.onHidCallback(connectionId, controllerId, data)
    local controlling = drivers.type_by_id[controllerId]
    if controlling.processPackets then  
        controlling.processPackets(connectionId, controllerId, data)
        return
    end

    local len = #data 
    local action = ""
    if len == 2 then  
        --Mouse press
        print("Keyboard press: ", data[1], data[2])
    elseif len == 4 or len == 3 then 
        local empty = true
        local mouse = drivers.mouse[controllerId]

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
            action = action ..('Move_x='..deltaX..' ')
            empty = false
        end
        if deltaY ~= 0 then  
            action = action ..('Move_y='..deltaY..' ')
            empty = false
        end
        if deltaWheel ~= 0 then  
            action = action ..('Scroll='..deltaWheel..' ')
            empty = false
        end
        local mb = mouse.buttons
        for i=1,8 do  
            local state = buttons & (1 << i) == 0 and 0 or 1
            if mb[i] ~= state then  
                action = action ..('button['..i..']='..state..' ')
                empty = false
                mb[i] = state
            end
        end

        if action ~= "" then  
            print(action)
        end
        if empty then  
            print("Mouse all zeros.")
        end
        
    elseif len == 5 then  
        local str = ""
        for i,b in pairs(data) do  
            str = str .. b..', '
        end
        print(str)
    elseif len == 6 or len == 8 then  
        local joystickObject = drivers.joystick[controllerId]

        local buttons = data[5]
        if buttons == 0 then  
            for a,c in pairs(drivers.JOSYTICK_BUTTONS_MAP) do
                joystickObject.buttons[c] = 0
            end
        else
            local idx = drivers.JOSYTICK_BUTTONS_MAP[buttons]
            if idx then
                joystickObject.buttons[idx] = 1
            else 
                print('Unhandled button press with id '..buttons..' TYPE '..type(buttons))
            end
        end
        joystickObject.left_hat = data[6] or 0
        joystickObject.right_hat = data[7] or 0
       
        joystickObject.left_analog_x = data[1] - 127
        joystickObject.left_analog_y = data[2] - 127

        joystickObject.right_analog_x = data[3] - 127
        joystickObject.right_analog_y = data[4] - 128
    else 
        print("packet size is unknown: "..(#data))
    end
end

function drivers.EnableGenericAndroidMouse()
    if not versions.canRun("2.0.0") then 
        --No mouse in the legacy controller
        log("You are running protopanda on a older version. Minimum version required is 2.0.0 to run mouse input") 
        return false
    end
    if not hasBLEStarted() then 
        return false
    end
    drivers.mouseHandler = BleServiceHandler("00001812-0000-1000-8000-00805f9b34fb")
    drivers.mouseHandler:SetOnConnectCallback(drivers.onConnectHID)
    drivers.mouseHandler:SetOnDisconnectCallback(drivers.onDisconnectHID)
    drivers.mouseListener = drivers.mouseHandler:AddCharacteristics("2a4d")
    drivers.mouseListener:SetSubscribeCallback(drivers.onHidCallback) 
    drivers.mouseListener:SetCallbackModeStream(false)
    return true
end

function drivers.update()
    for i,b in pairs(drivers.type_by_id) do  
        if b.onUpdate then  
            b.onUpdate(drivers, i)
        end
    end
end

return drivers