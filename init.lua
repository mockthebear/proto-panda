local expressions = require("expressions")
local scripts = require("scripts")
local generic = require("generic")
local menu = require("menu")
local boop = require("boop")
local configloader = require("configloader")
local drivers = require("drivers")
local input = require("input")

function onSetup()

    dictLoad()

    setLogDiscoveredBleDevices(false)
    generic.displaySplashMessage("Starting:\nBLE")
    startBLE()

    drivers.EnableProtopandaController()
    drivers.EnableGenericAndroidMouse()

    
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
    input.Load()
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
    drivers.update()
    input.updateButtonStates()
    if not scripts.Handle(dt) then
        return
    end
    menu.handleMenu(dt)
end

