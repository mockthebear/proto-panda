local drivers = {
    handlerPanda = nil,
    mouseHandler = nil,

    pandaListener = nil,
    mouseListener = nil,
}

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
local function parsePandaData(data)
    local index = 1
    local z,x,y,az,ax,zy,temp;

--[1]number(9), [2]number(2), [3]number(40), [4]number(246), [5]number(171), [6]number(31), [7]number(14), [8]number(0), [9]number(238), [10]number(255), [11]number(247), [12]number(255), [13]number(97), [14]number(1), [15]number(0), [16]number(0), [17]number(0), [18]number(0), [19]number(0), [20]number(0),

    z,index = readInt16(data, index)
    x,index = readInt16(data, index)
    y,index = readInt16(data, index)
    az,index = readInt16(data, index)
    ax,index = readInt16(data, index)
    ay,index = readInt16(data, index)
    temp,index = readInt16(data, index)
    
    local buttons = {}
    for i = 1, 7 do
        buttons[i] = data[index]
        index = index + 1
    end
    
    -- Read controller ID
    local controllerId = data[index]
    
    -- Print the parsed data
   --[[ print("Parsed Sensor Data:")
    print("  Orientation - Z: " .. z .. ", X: " .. x .. ", Y: " .. y)
    print("  Acceleration - AZ: " .. az .. ", AX: " .. ax .. ", AY: " .. ay)
    print("  Temperature: " .. temp)]]
    
    local buttonStr = ""
    for i, btn in ipairs(buttons) do
        buttonStr = buttonStr .. "Button[" .. i .. "]: " .. btn .. " "
    end
    print(#data, buttonStr)

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


function drivers.onSubscribeMessagePanda(clientid, data)
    local pandaData = parsePandaData(data)
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
function drivers.onMouseCallback(clientid, data)
    
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

    local str = ""
    for i,b in pairs(data) do  
        str = str ..'['..i..']' ..type(b) .."("..tostring(b).."), " 
    end
    print(clientid.." BT DATA: "..str)
    secondPrev = prevPacket
    prevPacket = data
end




function drivers.EnableGenericAndroidMouse()
    drivers.mouseHandler = BleServiceHandler("00001812-0000-1000-8000-00805f9b34fb")
    drivers.mouseListener = drivers.mouseHandler:AddCharacteristics("2a4d")
    drivers.mouseListener:SetSubscribeCallback(drivers.onMouseCallback) 
end

return drivers