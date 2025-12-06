local versions = require("versions")

if not MAX_BLE_BUTTONS then
    _G.MAX_BLE_BUTTONS = 6
end
local drivers = {
    handlerPanda = nil,
    mouseHandler = nil,

    pandaListener = nil,
    mouseListener = nil,

    maxButtons = MAX_BLE_BUTTONS,

    panda = {},
    joystick = {},
    mouse = {},


    eventQueue = {},
    type_by_id = {},

    JOYSTICK_HAT_NOTHING                = 0,
    JOYSTICK_HAT_DIRECTION_RIGHT        = 1,
    JOYSTICK_HAT_DIRECTION_DOWN_RIGHT   = 2,
    JOYSTICK_HAT_DIRECTION_DOWN         = 3,
    JOYSTICK_HAT_DIRECTION_DOWN_LEFT    = 4,
    JOYSTICK_HAT_DIRECTION_LEFT         = 5,
    JOYSTICK_HAT_DIRECTION_TOP_RIGHT    = 6,
    JOYSTICK_HAT_DIRECTION_TOP          = 7,
    JOYSTICK_HAT_DIRECTION_TOP_LEFT     = 8,

    JOSYTICK_BUTTON_A                   = 4,
    JOSYTICK_BUTTON_B                   = 1,
    JOSYTICK_BUTTON_C                   = 3,
    JOSYTICK_BUTTON_D                   = 2,


    JOSYTICK_BUTTONS_MAP = {
        [1] = 1, --B
        [2] = 2, --D
        [8] = 3, --C
        [16] = 4, --A
    },
}



do
    for i=0,MAX_BLE_CLIENTS-1 do   
        local buttons = {}
        for b=1,MAX_BLE_BUTTONS do  
            buttons[b] = 0
        end 
        drivers.panda[i] = {
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
        drivers.mouse[i] = {
            x=0,
            y=0,
            wheel=0,
            buttons={0,0,0,0,0,0,0,0}
        }
        drivers.joystick[i] = {  
            buttons = {},   
            left_hat = drivers.JOYSTICK_HAT_NOTHING, 
            right_hat = drivers.JOYSTICK_HAT_NOTHING,  
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
    for i = 1, 5 do
        buttons[i] = data[index]
        index = index + 1
    end

    controllerInfo.last_update = millis()
end


function drivers.onSubscribeMessagePanda(connectionId, clientId, data)
    if #data ~= 20 then  
        print("Possible corrupt package in panda controller. Expected size 22 but got "..(#data))
        return
    end
    parsePandaData(drivers.panda[clientId], data)
end

function drivers.onConnectPanda(connectionId, controllerId, address, name)
    drivers.type_by_id[controllerId] = "panda"
    print("Connected protopanda hand controller "..address.." "..tostring(name)..' this controller will be the '..controllerId)
    drivers.handlerPanda:WriteToCharacteristics({0,0,0,controllerId}, connectionId, "d4d3fafb-c4c1-c2c3-b4b3-b2b1a4a3a2a1", true)
end


function drivers.EnableProtopandaController()
    if not versions.canRun("2.0.0") then  
        --Legacy controller
        setMaximumControls(2)
        acceptBLETypes("d4d31337-c4c1-c2c3-b4b3-b2b1a4a3a2a1", "afaf", "fafb")
        return false
    end
    drivers.handlerPanda = BleServiceHandler("d4d31337-c4c1-c2c3-b4b3-b2b1a4a3a2a1")
    drivers.handlerPanda:SetOnConnectCallback(drivers.onConnectPanda)
    drivers.pandaListener = drivers.handlerPanda:AddCharacteristics("d4d3afaf-c4c1-c2c3-b4b3-b2b1a4a3a2a1")
    drivers.pandaListener:SetSubscribeCallback(drivers.onSubscribeMessagePanda)
    drivers.pandaListener:SetCallbackModeStream(true)
    return true
end





local prevPacket = nil
local secondPrev = nil
function drivers.onMouseCallback(connectionId, controllerId, data)

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
                action = action ..('button['..i..']='..state..' ')
                mb[i] = state
            end
        end

        if action ~= "" then  
            print(action)
        end
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


function drivers.onConnectHID(connectionId, controllerId, address, name)
    drivers.type_by_id[controllerId] = "hid"
    print("Connected generic HID "..address.." "..tostring(name)..' this controller will be the '..controllerId)
end


function drivers.EnableGenericAndroidMouse()
    if not versions.canRun("2.0.0") then 
        --No mouse in the legacy controller
        print("You are running protopanda on a older version. Minimum version required is 2.0.0 to run mouse input") 
        return false
    end
    drivers.mouseHandler = BleServiceHandler("00001812-0000-1000-8000-00805f9b34fb")
    drivers.mouseHandler:SetOnConnectCallback(drivers.onConnectHID)
    drivers.mouseListener = drivers.mouseHandler:AddCharacteristics("2a4d")
    drivers.mouseListener:SetSubscribeCallback(drivers.onMouseCallback) 
    drivers.mouseListener:SetCallbackModeStream(false)
    return true
end

return drivers