local expressions = require("expressions")
local scripts = require("scripts")
local generic = require("generic")
local menu = require("menu")
local boop = require("boop")
local configloader = require("configloader")




local handler
local handlerPanda
local listener
local listenerPanda

function onCallback(clientid, data)
    print(data)

    local str = ""
    for i,b in pairs(data) do  
        str = str ..'['..i..']' ..type(b) .."("..tostring(b).."), " 
    end
    print(clientid.." BT DATA: "..str)
end

local prevPacket = nil
local secondPrev = nil
function onBluetoothMouseCallback(clientid, data)
    
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
    prevPacket = data
end


function onConnectPanda(clientid, a,b)
    handlerPanda:WriteToCharacteristics({0,0,0,1}, clientid, "d4d3fafb-c4c1-c2c3-b4b3-b2b1a4a3a2a1", true)
end

function onSetup()

    dictLoad()
    

    setLogDiscoveredBleDevices(false)
    generic.displaySplashMessage("Starting:\nBLE")
    startBLE()
    
    handler = BleServiceHandler("00001812-0000-1000-8000-00805f9b34fb")
    listener = handler:AddCharacteristics("2a4d")
    listener:SetSubscribeCallback(onBluetoothMouseCallback) 

    handlerPanda = BleServiceHandler("d4d31337-c4c1-c2c3-b4b3-b2b1a4a3a2a1")
    handlerPanda:SetOnConnectCallback(onConnectPanda)
    listenerPanda = handlerPanda:AddCharacteristics("d4d3afaf-c4c1-c2c3-b4b3-b2b1a4a3a2a1")
    listenerPanda:SetSubscribeCallback(onCallback)
    
    startBLERadio()


    local seed = tonumber(dictGet("random_seed")) or millis()
    seed = seed + millis()
    math.randomseed(seed)
    print("Random seed is "..seed)
    dictSet("random_seed", tostring(seed))
    if dictGet("created") ~= "1" then
        menu.setDictDefaultValues()
    end
    dictSave()
    generic.displaySplashMessage("Starting:\nExpressions")

    configloader.Load("/config.json")

    expressions.Load() 
    scripts.Load() 
    boop.Load()

    generic.displaySplashMessage("Starting:\nLeds")
    ledsBeginDual(25, 25, 0) 
    ledsDisplay()
    ledsSegmentRange(0, 0, 24)
    ledsSegmentRange(1, 25, 49)
    ledsSegmentBehavior(0, BEHAVIOR_PRIDE) 
    ledsSegmentBehavior(1, BEHAVIOR_PRIDE)
    generic.displaySplashMessage("Starting:\nMenu") 
    menu.setup()

end

function onPreflight()
    ledsSetManaged(true)
    setPanelManaged(true)
    expressions.Next()
    generic.setAutoPowerMode(tonumber(dictGet("panel_brightness")) or 64)
    beginBleScanning()
    ledsGentlySeBrightness(tonumber(dictGet("led_brightness") ) or 64)
    gentlySetPanelBrightness(tonumber(dictGet("panel_brightness")) or 64)
    
end

function onLoop(dt)
    if not scripts.Handle(dt) then
        return
    end
    menu.handleMenu(dt)
end

