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

    inputs = {
        generic = {},
    },
    
    paired_data = {},

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

    last_connection = 0
}

drivers.device_attribute_map = {}

function drivers.Start(maxClients)
    drivers.maxClients = maxClients
    for i=0,maxClients-1 do   
        drivers.inputs.generic[i] = {
            timeout=0,
            buttons={0,0,0,0,0,0,0,0}
        }
    end
end

function drivers.EnableDrivers(input)
    local driversToLoad = input.drivers

    for i,driverName in pairs(driversToLoad) do  
        local success, content = pcall(dofile,"/lualib/drivers/"..driverName..'.lua')
        if success and content then
            if driverName ~= 'hid' or input.enableHidControllers then
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
                        if type(inputName) ~= 'string' then  
                            error("Input method "..tostring(inputName).." is not valid for driver "..driverName)
                        end
                        print("Added input method: "..inputName)
                        drivers.validEntries[inputName] = true
                        drivers.inputs[inputName] = object
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

function drivers.update()
    for i,b in pairs(drivers.type_by_id) do  
        if b.onUpdate then  
            b.onUpdate(drivers, i)
        end
    end
end

return drivers