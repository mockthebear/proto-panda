local versions = require("versions")
local json = require("json")
local configloader = require("configloader")
local drivers = require("drivers")
local input = require("input")
local ui = require("ui")
local generic = require("generic")
local models = require("models")
local fft = require("fft")
local leds = require("leds")
local scripts = require("scripts")
local boop = require("boop")
local menu = require("menu")
local expressions = require("expressions")
local overlays = require("overlays")
local servos = require("servos")

function onSetup()

    dictLoad()

    local seed = tonumber(dictGet("random_seed")) or millis()
    seed = seed + millis()
    math.randomseed(seed)
    print("Random seed is "..seed)
    dictSet("random_seed", tostring(seed))
    if dictGet("created") ~= "1" then
        menu.setDictDefaultValues()
    end
    dictSave()
    

    configloader.Load()
    input.Load()
    models.Load()
    expressions.Load() 
    scripts.Load() 
    boop.Load()
    generic.displaySplashMessage("Starting:\nServos")
    servos.setup()
    generic.displaySplashMessage("Starting:\nFFT")
    fft.load()
    generic.displaySplashMessage("Starting:\nLeds")
    leds.begin()
    generic.displaySplashMessage("Starting:\nMenu") 
    menu.setup()
    generic.displaySplashMessage("Starting:\nOverlays") 
    overlays.setup()

end

function onPreflight()
    ledsSetManaged(true)
    setPanelManaged(true)

    if configloader.Get().main_animation ~= nil then
        expressions.SetExpression(configloader.Get().main_animation)
    else
        expressions.Next()
    end

    if configloader.Get().startup_animation ~= nil or configloader.Get().starting_animation then
        expressions.StackExpression("startup")
    end

    input.Start() 
    setPoweringMode(BUILT_IN_POWER_MODE)
    ledsGentlySeBrightness(tonumber(dictGet("led_brightness") ) or 64)
    gentlySetPanelBrightness(tonumber(dictGet("panel_brightness")) or 64)


end

function onLoop(dt)
    overlays.update(dt)
    drivers.update()
    input.update()
    expressions.update()
    servos.update(dt)
    if not scripts.Handle(dt) then
        return
    end
    menu.handleMenu(dt)
end
