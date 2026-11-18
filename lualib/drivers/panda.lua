local panda = {
	type="core",

	mode = {
        'panda'
    },
}

local TIME_CHARACTERISTIC_UUID = "d4d3c10c-c4c1-c2c3-b4b3-b2b1a4a3a2a1"
local MIN_VALID_EPOCH = 1577836800


function panda.getDriverModules(maxClients)
	local pandaData = {
        panda = {}
    }
	for i=0,maxClients-1 do  
		local buttons = {}
        for b=1,MAX_BLE_BUTTONS do  
            buttons[b] = 0
        end 
		pandaData.panda[i] = {
            az = 0,
            ax = 0, 
            ay = 0,
            gz = 0,
            gx = 0,
            gy = 0,
            temp = 0,
            buttons = buttons,
            controllerId = 0,
            last_update=0,
        }
    end 
    panda.modules = pandaData
  	return pandaData
end

local function readInt16(data, index)
    local low = data[index]
    local high = data[index + 1]
    index = index + 2
    local value = high * 256 + low
    if value >= 0x8000 then
        value = value - 0x10000
    end
    return value, index
end

local function parsePandaData(controllerInfo, data)
    local index = 1
    controllerInfo.az,index = readInt16(data, index)
    controllerInfo.ax,index = readInt16(data, index)
    controllerInfo.ay,index = readInt16(data, index)
    controllerInfo.gz,index = readInt16(data, index)
    controllerInfo.gx,index = readInt16(data, index)
    controllerInfo.gy,index = readInt16(data, index)
    controllerInfo.temp,index = readInt16(data, index)

    controllerInfo.controllerId = data[index]
    index = index +1

    local buttons = controllerInfo.buttons
    for i = 1, 8 do
        buttons[i] = data[index] or 0
        index = index + 1
    end

    controllerInfo.last_update = millis()
end

local function readUInt32LE(data, index)
    return data[index]
        + data[index + 1] * 0x100
        + data[index + 2] * 0x10000
        + data[index + 3] * 0x1000000
end

local function readInt16LE(data, index)
    local value = data[index] + data[index + 1] * 0x100
    if value >= 0x8000 then
        value = value - 0x10000
    end
    return value
end

function panda.syncClock(connectionId)
    local ok, data = panda.handler:ReadFromCharacteristics(connectionId, TIME_CHARACTERISTIC_UUID)
    if not ok or not data or #data < 4 then
        return
    end

    local epoch = readUInt32LE(data, 1)
    if epoch < MIN_VALID_EPOCH then
        log("Ignoring invalid controller clock value")
        return
    end

    local utcOffsetMinutes = 0
    if #data >= 6 then
        utcOffsetMinutes = readInt16LE(data, 5)
    end

    if setClockFromBle(epoch, utcOffsetMinutes) then
        log("Clock synchronized from controller")
    else
        log("Ignoring controller clock with invalid timezone offset")
    end
end



function panda.onSubscribeMessagePanda(connectionId, clientId, data)
    if #data < 20 then  
        log("Possible corrupt package in panda controller. Expected size at least 20 but got "..(#data))
        return
    end

    parsePandaData(drivers.inputs.panda[clientId], data)
end

function panda.onDisconnectPanda(connectionId, controllerId, reason)
    log("Disconnected "..connectionId.." due ".. reason)
    drivers.DisconnectDevice(controllerId, 'panda')
end

function panda.onConnectPanda(connectionId, controllerId, address, name)
    drivers.ConnectDevice(controllerId, address, "panda")
    log("Connected protopanda hand controller "..address.." "..tostring(name)..' this controller will be the '..controllerId)
    panda.handler:WriteToCharacteristics({0,0,0,controllerId}, connectionId, "d4d3fafb-c4c1-c2c3-b4b3-b2b1a4a3a2a1", true)
    panda.syncClock(connectionId)
end


function panda.onEnable()
    if not hasBLEStarted() then
        return false
    end
    panda.handler = BleServiceHandler("d4d31337-c4c1-c2c3-b4b3-b2b1a4a3a2a1")
    panda.handler:SetOnConnectCallback(panda.onConnectPanda)
    panda.handler:SetOnDisconnectCallback(panda.onDisconnectPanda)
    panda.pandaListener = panda.handler:AddCharacteristics("d4d3afaf-c4c1-c2c3-b4b3-b2b1a4a3a2a1")
    panda.pandaListener:SetSubscribeCallback(panda.onSubscribeMessagePanda)
    panda.pandaListener:SetCallbackModeStream(true)
    return true
end

return panda
