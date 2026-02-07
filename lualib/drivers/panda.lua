local panda = {
	type="core",

	mode = {
        'panda'
    },
}


function panda.getDriverModules()
	local pandaData = {}
	for i=0,MAX_BLE_CLIENTS-1 do  
		local buttons = {}
        for b=1,MAX_BLE_BUTTONS do  
            buttons[b] = 0
        end 
		pandaData[i] = {
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



function panda.onSubscribeMessagePanda(connectionId, clientId, data)
    if #data < 20 then  
        log("Possible corrupt package in panda controller. Expected size at least 20 but got "..(#data))
        return
    end
    parsePandaData(drivers.panda[clientId], data)
end

function panda.onDisconnectPanda(connectionId, controllerId, reason)
    log("Disconnected "..connectionId.." due ".. reason)
    drivers.type_by_id[controllerId] = nil
end

function panda.onConnectPanda(connectionId, controllerId, address, name)
    drivers.type_by_id[controllerId] = panda
    log("Connected protopanda hand controller "..address.." "..tostring(name)..' this controller will be the '..controllerId)
    panda.handlerPanda:WriteToCharacteristics({0,0,0,controllerId}, connectionId, "d4d3fafb-c4c1-c2c3-b4b3-b2b1a4a3a2a1", true)
end


function panda.onEnable()
    if not versions.canRun("2.0.0") then  
        --Legacy controller
        setMaximumControls(2)
        acceptBLETypes("d4d31337-c4c1-c2c3-b4b3-b2b1a4a3a2a1", "afaf", "fafb")
        return false
    end
    panda.handlerPanda = BleServiceHandler("d4d31337-c4c1-c2c3-b4b3-b2b1a4a3a2a1")
    panda.handlerPanda:SetOnConnectCallback(panda.onConnectPanda)
    panda.handlerPanda:SetOnDisconnectCallback(panda.onDisconnectPanda)
    panda.pandaListener = panda.handlerPanda:AddCharacteristics("d4d3afaf-c4c1-c2c3-b4b3-b2b1a4a3a2a1")
    panda.pandaListener:SetSubscribeCallback(panda.onSubscribeMessagePanda)
    panda.pandaListener:SetCallbackModeStream(true)
    return true
end

return panda