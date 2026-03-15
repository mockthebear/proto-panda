local expressions = require("expressions")
local scripts = require("scripts")
local generic = require("generic")
local menu = require("menu")
local boop = require("boop")
local configloader = require("configloader")
local drivers = require("drivers")
local input = require("input")

local models = {}
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
        local md = loadModel(model)
        print(md)
        print("ID="..tostring(md:GetId()))

        models[md:GetId()] = md


        if model.pointGroups and #model.pointGroups > 0 then 
            for idx,pg in pairs(model.pointGroups) do 
                local pgid = md:AddPointGroup(pg.pointIndices)
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
    setModelAnimation({0,1,2}, true)
end
local rotated = 0
local sin = math.sin
local cos = math.cos
local transform = {x = 0, y = 0}
local center = {x = 20.5, y= 4.5}
function onLoop(dt)
    drivers.update()
    input.update()
    if not scripts.Handle(dt) then
        return
    end
    menu.handleMenu(dt)

    rotated = rotated + 0.5

    local md = models[0]
    md:Reset()
    transform.x = 5 * cos(rotated)
    transform.y = 5 * sin(rotated)
    md:Translate(transform);
    md:Recalculate()
    md:CopyToRaster()

    local md = models[3]
    md:Reset()
    md:Rotate(center, rotated);
    md:Recalculate()
    md:CopyToRaster()


    
end

