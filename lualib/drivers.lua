local versions = require("versions")

local DEBUG_INPUT = false

if not MAX_BLE_BUTTONS then
    _G.MAX_BLE_BUTTONS = 8
end
if not MAX_BLE_CLIENTS then
    _G.MAX_BLE_CLIENTS = 2
end

local drivers = {
    mouseHandler = nil,
    mouseListener = nil,

    maxButtons = MAX_BLE_BUTTONS,
    maxClients = 2,

    panda = {},
    joystick = {},
    mouse = {},
    keyboard = {},
    generic = {},
    paired_data = {},

    validEntries = {
        ['generic'] = true,
    },


    eventQueue = {},
    type_by_id = {},

    loaded = {},
    core = {},

    last_connection = 0,


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

function drivers.Start(maxClients)
    drivers.maxClients = maxClients
    for i=0,maxClients-1 do   
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

        drivers.keyboard[i] = {
            button = 0
        }

        drivers.joystick[i] = {  
            buttons = {},   
            left_hat = 0, 
            right_hat = 0,  
            left_analog_x = 0,
            left_analog_y = 0,
            right_analog_x = 0,
            right_analog_y = 0,

            stick_x = 0,
            stick_y = 0,
            gyro = {x=0,y=0,z=0},
            accelerometer = {x=0,y=0,z=0},
        }

        for __,b in pairs(drivers.JOSYTICK_BUTTONS_MAP) do  
            drivers.joystick[i].buttons[b] =0
        end
    end
end

function drivers.EnableDrivers(input)
    local driversToLoad = input.drivers
    if input.enableHidControllers then
        local hid = {}

        hid.mode=drivers.device_attribute_map["hid"]
        function hid.onEnable(self)
            if not hasBLEStarted() then
                return false
            end
            hid.handler = BleServiceHandler("00001812-0000-1000-8000-00805f9b34fb")
            hid.handler:SetOnConnectCallback(drivers.onConnectHID)
            hid.handler:SetOnDisconnectCallback(drivers.onDisconnectHID)
            hid.handler:SetEncryptionRequired(true)
            hid.mouseListener = hid.handler:AddCharacteristics("2a4d")
            hid.mouseListener:SetSubscribeCallback(drivers.onHidCallback) 
            hid.mouseListener:SetCallbackModeStream(false)
            return true
        end
        
        drivers.loaded["hid"] = hid
    end

    for i,driverName in pairs(driversToLoad) do  
        local success, content = pcall(dofile,"/lualib/drivers/"..driverName..'.lua')
        if success and content then
            if drivers.loaded[driverName] then  
                error("Duplicated driver declared: "..driverName)
            end  
            drivers.loaded[driverName] = content
            --todo change to derivate
            if content.type == "hid" then
                if input.enableHidControllers then
                    if content.mode then  
                        for i, mode in pairs(content.mode) do 
                            if not drivers.validEntries[mode] then  
                                error("Invalid mode '"..mode.."' in driver "..driverName)
                            end
                        end
                        content.handler = drivers.loaded["hid"].handler
                        drivers.device_attribute_map[driverName] = content.mode
                    else 
                        error("No mode set for "..driverName)
                    end

                    content.attribute_map = drivers.device_attribute_map[driverName]
                    print("Loaded hid driver "..driverName)
                else 
                    drivers.loaded[driverName] = nil
                    log("Skipping "..driverName.." because it inherit HID and hid is not enabled")
                end
            elseif content.type == "core" then

                local modules = content.getDriverModules(drivers.maxClients)
                for inputName , object in pairs(modules) do
                    drivers.validEntries[inputName] = true
                    drivers[inputName] = object
                end
                drivers.core[driverName] = content
                print("Loaded core driver "..driverName)
            else 
                error("Undefined driver type "..tostring(content.type))
            end
        else 
            log("Failed to load driver '"..driverName.."' due "..tostring(content))
        end
    end
end


function drivers.beginPairing()
    log("Pairing started")
    setScanModeByAddress(false)
    requestClearBleResults()
    drivers.pairing_mode=true
end


function drivers.stopPairing()
    log("Pairing stopped")
    setScanModeByAddress(true)
    drivers.pairing_mode=false
end

function drivers.WrapUp()
    for i,b in pairs(drivers.loaded) do  
        if b.onEnable and not b:onEnable() then  
            log("Failed to enable core driver "..tostring(i))
        end
        if not b.handler and b.type == 'core' then  
            error("Driver "..i.." dont have a valid handler")
        end
    end
    
    for i,b in pairs(drivers.loaded) do  
        if not b.handler then  
            if b.type and drivers.loaded[b.type] then  
                b.handler = drivers.loaded[b.type].handler
            else 
                error("Driver "..i.." dont have a valid handler inherited")
            end
        end
    end

    if hasBLEStarted() then
        local confs = configloader.Get()
        if confs.input.pairController then
            log("Connect by paired only")
            setScanModeByAddress(true)
            drivers.registerPaired = true
            local pairedRaw = dictGet("paired_data") or {}
            if pairedRaw ~= "" then  
                local pairedData = json.decode(pairedRaw)
                drivers.paired_data = pairedData
                local count = 0
                for driverName, devices in pairs(pairedData) do  
                    local drv = drivers.loaded[driverName]
                    if drv then  
                        for addr, __ in pairs(devices) do
                            count = count +1
                            drv.handler:AddPairedDeviceAddress(addr)
                        end
                    end
                end
                log("Total of "..count.." devices to be connected!")
                if count == 0 then  
                    drivers.beginPairing()
                end
            else 
                log("No paired devices registered")
                drivers.beginPairing()
            end
        else  
            drivers.pairing_mode=false
            --Were not pairing, just connecting on ANY device avaliable
            setScanModeByAddress(false)
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


function drivers.DisconnectDevice(controllerId, driverName)
    drivers.type_by_id[controllerId] = nil

    drivers.last_connection = millis()
    drivers.last_action = "Disconnected"
    drivers.last_name = driverName
    if drivers.registerPaired then
        beginBleScanning()
    end
end

function drivers.ConnectDevice(controllerId, address, driverName)
    --Check if is a valid driver
    local driverHandle = drivers.loaded[driverName]

    if not driverHandle then  
        error("No driver named '"..driverName.."'")
        return nil
    end

    drivers.last_connection = millis()
    drivers.last_action = "Connected"
    drivers.last_name = driverName

    drivers.type_by_id[controllerId] = driverHandle
    --If is pairing mode, then we should check if this is a new device, if so, we save it!
    if drivers.registerPaired then  
        if not drivers.paired_data[driverName] then  
            drivers.paired_data[driverName] = {}
        end
        local obj = drivers.paired_data[driverName]
        if not obj[address] then  
            obj[address] = true
            driverHandle.handler:AddPairedDeviceAddress(address)
            dictSet("paired_data", json.encode(drivers.paired_data))
            dictSave()
        end
        drivers.stopPairing()
        stopBleScanning()
    end
    return true
end 


function drivers.onDisconnectHID(connectionId, controllerId, reason)
    log("Disconnected "..connectionId.." due ".. reason)
    drivers.DisconnectDevice(controllerId, 'hid')
end

function drivers.onConnectHID(connectionId, controllerId, address, name)
    local matchedDriver, matchName = drivers.FindDriver(connectionId, controllerId, address, name)
    drivers.ConnectDevice(controllerId, address, matchName or 'hid')

    log("Connected conId="..connectionId.." controller="..controllerId.." addr=\""..address.."\" name=["..name.."] type="..(matchName or 'hid'))
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
        if DEBUG_INPUT then
            log("keyboard.button="..  data[1] .. ' -> '.. data[2])
        end
        local keyboard = drivers.keyboard[controllerId]
        keyboard.button = data[1]
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
        local joystickObject = drivers.joystick[controllerId]
        local buttonStates = ""
        local buttons = data[5]
        if buttons == 0 then  
            for a,c in pairs(drivers.JOSYTICK_BUTTONS_MAP) do
                joystickObject.buttons[c] = 0
            end
            buttonStates = " joystick.buttons.<all> 0"
        else
            local idx = drivers.JOSYTICK_BUTTONS_MAP[buttons]
            if idx then
                joystickObject.buttons[idx] = 1
                buttonStates = " joystick.buttons."..idx..' 1'
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


function drivers.update()
    for i,b in pairs(drivers.type_by_id) do  
        if b.onUpdate then  
            b.onUpdate(drivers, i)
        end
    end
end

return drivers