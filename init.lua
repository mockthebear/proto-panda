local expressions = require("expressions")
local scripts = require("scripts")
local generic = require("generic")
local menu = require("menu")

local brightness

function onSetup()

    dictLoad()
    setMaximumControls(2)
    acceptBLETypes("d4d31337-c4c1-c2c3-b4b3-b2b1a4a3a2a1", "afaf", "fafb")
    startBLE()
    
    brightness = tonumber(dictGet("panel_brightness")) or 64
    print("Panel is: "..dictGet("panel_brightness").." led is "..dictGet("led_brightness"))
    setPanelMaxBrighteness( brightness )

    local seed = tonumber(dictGet("random_seed")) or millis()
    seed = seed + millis()
    math.randomseed(seed)
    print("Random seed is "..seed)
    dictSet("random_seed", tostring(seed))
    dictSave()

    expressions.Load("/expressions.json") 
    scripts.Load("/scripts.json") 

    ledsBeginDual(25, 25, tonumber(dictGet("led_brightness") ) or 64) 
    displayLeds()
    ledsSegmentRange(0, 0, 24)
    ledsSegmentRange(1, 25, 49)
    ledsSegmentBehavior(0, BEHAVIOR_PRIDE) 
    ledsSegmentBehavior(1, BEHAVIOR_PRIDE) 
    
    menu.setup()
end

function onPreflight()
    generic.setAutoPowerMode()
    beginBleScanning()
    gentlySetPanelBrighteness(brightness)
    setPanelManaged(true)
    expressions.Next()
end

function onLoop(dt)
    if not scripts.Handle(dt) then
        return
    end
    menu.handleMenu(dt)
end

