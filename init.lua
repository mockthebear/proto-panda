local expressions = require("expressions")
local scripts = require("scripts")
local generic = require("generic")
local menu = require("menu")
local boop = require("boop")
local configloader = require("configloader")
local drivers = require("drivers")
local input = require("input")


function loadModelsFromJson(filename)
    local json = require("json")
    local fp, err = io.open(filename, "r")
    if not fp then 
        error("Failed to load "..filename..": "..tostring(err))
    end
    local content = fp:read("*a")
    fp:close()
    json.filename = filename
    local content = json.decode(content)
    for i,model in pairs(content.models) do  
        print("Loading "..model.name)
        local id = loadModel(model)
        print("ID="..id)
        if model.pointGroups and #model.pointGroups > 0 then 
            for idx,pg in pairs(model.pointGroups) do 
                local pgid = addModelPointList(pg.pointIndices, id)
                print("Added point group id "..pgid)
            end
        end
    end
end

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
    generic.displaySplashMessage("Starting:\nExpressions")
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

    loadModelsFromJson("/models/face.json")
    

end

function onPreflight()
    ledsSetManaged(true)
    setPanelManaged(true)
    expressions.Next()
    input.Start()
    setPoweringMode(BUILT_IN_POWER_MODE)
    ledsGentlySeBrightness(tonumber(dictGet("led_brightness") ) or 64)
    gentlySetPanelBrightness(tonumber(dictGet("panel_brightness")) or 64)
    setModelAnimation({0,1,2,3}, true)
end

function onLoop(dt)
    drivers.update()
    input.update()
    if not scripts.Handle(dt) then
        return
    end
    menu.handleMenu(dt)
end

