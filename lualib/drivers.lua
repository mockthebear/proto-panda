local drivers = {
    handlerPanda = nil,
    mouseHandler = nil,

    pandaListener = nil,
    mouseListener = nil,

    maxButtons = MAX_BLE_BUTTONS,

    panda = {},
    joystick = {},
    mouse = {},
}
do
    for i=1,MAX_BLE_CLIENTS do   
        local buttons = {}
        for b=1,MAX_BLE_BUTTONS do  
            buttons[b] = 0
        end 
        drivers.panda[i] = {
            z = 0,
            x = 0, 
            y = 0,
            az = 0,
            ax = 0,
            ay = 0,
            temp = 0,
            buttons = buttons,
            controllerId = 0
        }
        drivers.mouse[i] = {
            x=0,
            y=0,
            wheel=0,
            buttons={0,0,0,0,0,0,0,0}
        }
        drivers.joystick[i] = {
            left_axis_x=0,
            left_axis_y=0,

            right_axis_x=0,
            right_axis_y=0,

            wheel=0,
            left=0,
            right=0,
            middle=0,
        }
    end
end

local function readInt16(data)
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
    clientInfo.z,index = readInt16(data, index)
    clientInfo.x,index = readInt16(data, index)
    clientInfo.y,index = readInt16(data, index)
    clientInfo.az,index = readInt16(data, index)
    clientInfo.ax,index = readInt16(data, index)
    clientInfo.ay,index = readInt16(data, index)
    clientInfo.temp,index = readInt16(data, index)

    clientInfo.controllerId = data[index]
    index = index +1

    local buttons = clientInfo.buttons
    for i = 1, 5 do
        buttons[i] = data[index]
        index = index + 1
    end

    return {
        z = z,
        x = x, 
        y = y,
        az = az,
        ax = ax,
        ay = ay,
        temp = temp,
        buttons = buttons,
        controllerId = controllerId
    }
end


function drivers.onSubscribeMessagePanda(controllerId, clientid, data)
    parsePandaData(data, drivers.panda[controllerId])
end

function drivers.onConnectPanda(controllerId, clientid, address, name)
    drivers.handlerPanda:WriteToCharacteristics({0,0,0,controllerId}, clientid, "d4d3fafb-c4c1-c2c3-b4b3-b2b1a4a3a2a1", true)
end


function drivers.EnableProtopandaController()
    drivers.handlerPanda = BleServiceHandler("d4d31337-c4c1-c2c3-b4b3-b2b1a4a3a2a1")
    drivers.handlerPanda:SetOnConnectCallback(drivers.onConnectPanda)
    drivers.pandaListener = drivers.handlerPanda:AddCharacteristics("d4d3afaf-c4c1-c2c3-b4b3-b2b1a4a3a2a1")
    drivers.pandaListener:SetSubscribeCallback(drivers.onSubscribeMessagePanda)
end





local prevPacket = nil
local secondPrev = nil
function drivers.onMouseCallback(controllerId, clientid, data)

    local len = #data 
    local action = ""
    if len == 2 then  
        --Mouse press
        print("Mouse press only: ", data[1], data[2])
    elseif len == 4 or len == 3 then 
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
        end
        if deltaY ~= 0 then  
            action = action ..('Move_y='..deltaX..' ')
        end
        if deltaWheel ~= 0 then  
            action = action ..('Scroll='..deltaWheel..' ')
        end
        local mb = mouse.buttons
        for i=1,8 do  
            local state = buttons & (1 << i) == 0 and 0 or 1
            if mb[i] ~= state then  
                action = action ..('button['..i..']='..state)
                mb[i] = state
            end
        end

        print(action)
    elseif len == 6 or len == 8 then  
        local leftX = data[1] - 127
        local leftY = data[2] - 127
        local rightX = data[3] - 127
        local rightY = data[4] - 128

        local buttons = data[5]
        local leftHat = data[6] or 0
        local rightHat = data[7] or 0
        --local extra3 = data[8] or 0
        local buttonStates = {
            Button1 = (buttons & 0x01) ~= 0,  -- Bit 0: Value 1
            Button2 = (buttons & 0x02) ~= 0,  -- Bit 1: Value 2
            Button3 = (buttons & 0x04) ~= 0,  -- Bit 2: Value 4
            Button4 = (buttons & 0x08) ~= 0,  -- Bit 3: Value 8
            Button5 = (buttons & 0x10) ~= 0,  -- Bit 4: Value 16
            Button6 = (buttons & 0x20) ~= 0,  -- Bit 5: Value 32
            Button7 = (buttons & 0x40) ~= 0,  -- Bit 6: Value 64
            Button8 = (buttons & 0x80) ~= 0,  -- Bit 7: Value 128
        }
        local leftHatStates = {
            LeftHat1 = (leftHat & 0x01) ~= 0,  -- Bit 0: Value 1
            LeftHat12 = (leftHat & 0x02) ~= 0,  -- Bit 1: Value 2
            LeftHat13 = (leftHat & 0x04) ~= 0,  -- Bit 2: Value 4
            LeftHat14 = (leftHat & 0x08) ~= 0,  -- Bit 3: Value 8
            LeftHat15 = (leftHat & 0x10) ~= 0,  -- Bit 4: Value 16
            LeftHat16 = (leftHat & 0x20) ~= 0,  -- Bit 5: Value 32
            LeftHat17 = (leftHat & 0x40) ~= 0,  -- Bit 6: Value 64
            LeftHat18 = (leftHat & 0x80) ~= 0,  -- Bit 7: Value 128
        }
        local rightHatStates = {
            RightHat1 = (rightHat & 0x01) ~= 0,  -- Bit 0: Value 1
            RightHat2 = (rightHat & 0x02) ~= 0,  -- Bit 1: Value 2
            RightHat3 = (rightHat & 0x04) ~= 0,  -- Bit 2: Value 4
            RightHat4 = (rightHat & 0x08) ~= 0,  -- Bit 3: Value 8
            RightHat5 = (rightHat & 0x10) ~= 0,  -- Bit 4: Value 16
            RightHat6 = (rightHat & 0x20) ~= 0,  -- Bit 5: Value 32
            RightHat7 = (rightHat & 0x40) ~= 0,  -- Bit 6: Value 64
            RightHat8 = (rightHat & 0x80) ~= 0,  -- Bit 7: Value 128
        }

        for i,b in pairs(buttonStates) do  
            if b then  
                action = action ..('Pressed '..i..' ')
            end
        end
        for i,b in pairs(leftHatStates) do  
            if b then  
                action = action ..('Pressed '..i..' ')
            end
        end
        for i,b in pairs(rightHatStates) do  
            if b then  
                action = action ..('Pressed '..i..' ')
            end
        end
        if leftX ~= 0 then  
            action = action ..('leftX='..leftX..' ')
        end
        if leftY ~= 0 then  
            action = action ..('leftY='..leftY..' ')
        end
        if rightX ~= 0 then  
            action = action ..('rightX='..rightX..' ')
        end
        if rightY ~= 0 then  
            action = action ..('rightY='..rightY..' ')
        end
    end
    print(action)

    --[[
    if data[1] == 156 and data[2] == 64 and data[3] == 236 then  
        print(clientid, "DOWN")
    elseif data[1] == 2 and data[2] == 48 and data[3] == 0 then  
        print(clientid, "UP")
    elseif data[1] == 255 and data[2] == 23 and data[3] == 128 then  
        print(clientid, "RIGHT")
    elseif data[1] == 56 and data[2] == 128 and data[3] == 17 then  
        print(clientid, "LEFT")
    elseif data[1] == 1 and data[2] == 0 and data[3] == 0 then
        if prevPacket and prevPacket[1] == 0 and prevPacket[2] == 0 and prevPacket[3] == 0 then
            if secondPrev and secondPrev[1] == 0 and secondPrev[2] == 0 and secondPrev[3] == 0 then
                print(clientid, "MIDDLE") 
            end
        end
    elseif data[1] == 234 and data[2] == 0 and data[3] == nil then
        print(clientid, "BACK") 
    end


    secondPrev = prevPacket
    prevPacket = data]]

end




function drivers.EnableGenericAndroidMouse()
    drivers.mouseHandler = BleServiceHandler("00001812-0000-1000-8000-00805f9b34fb")
    drivers.mouseListener = drivers.mouseHandler:AddCharacteristics("2a4d")
    drivers.mouseListener:SetSubscribeCallback(drivers.onMouseCallback) 
end

return drivers