local expressions = require("expressions")
local scripts = require("scripts")
local generic = require("generic")
local menu = require("menu")
local boop = require("boop")
local configloader = require("configloader")
local drivers = require("drivers")
local input = require("input")

local models = {}
local modelAnimation = {}
local modelByName = {}
local pointGroupByName = {}
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
    modelAnimation = content['animations']
    for i,model in pairs(content.models) do  
        print("Loading "..model.name)
        local md = loadModel(model, model.name)
        print(md)
        if not md then  
            error("Error loading model: "..model.name)
        end

        print("ID="..tostring(md:GetId()))

        models[md:GetId()] = md

        modelByName[model.name] = {model=md}


        if model.pointGroups and #model.pointGroups > 0 then 
            for idx,pg in pairs(model.pointGroups) do 
                local pgid = md:AddPointGroup(pg.indices)
                print("Added point group id "..pgid)
                pointGroupByName[model.name.."."..idx]  = {id=pgid, model=md}
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


    loadModelsFromJson("/models/meme.json")
    --preloadData()
    

end

function onPreflight()
    ledsSetManaged(true)
    setPanelManaged(true)
    expressions.Next()
    input.Start() 
    setPoweringMode(BUILT_IN_POWER_MODE)
    ledsGentlySeBrightness(tonumber(dictGet("led_brightness") ) or 64)
    gentlySetPanelBrightness(tonumber(dictGet("panel_brightness")) or 64)
    --[[setModelAnimation({0,1,2}, true)

    local md = models[0]
    md:Reset()

    md:TranslatePoints(0, {x=0, y=-10})

    md:Recalculate()
    md:CopyToRaster()
]]

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

    --[[rotated = rotated + 0.5

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
    md:CopyToRaster()]]



    --[[runModelAnim(dt)
    for idx, anim in pairs(modelAnimation) do
        for trackid, track in pairs(anim.tracks) do
            if track.r.model then  
                track.r.model:Recalculate()
                track.r.model:CopyToRaster()
            end
        end 
    end]]


    
end


local tools = {
    "translate",
    "rotate",
    "scale",
    "reset",
}

local math_rad = math.rad
local interpolations = {
    translate = function(r, t, data)
        if deltat == 0 then  
            if r.id then  
                r.model:Translate({x=data.x, y=data.y})
            else 
                r.model:TranslatePoints(r.id, {x=data.x, y=data.y})
            end
            return
        end
        local delta = t/data.deltat
        local px = delta * data.x
        local py = delta * data.y
        if r.id then  
            r.model:TranslatePoints(r.id, {x=px, y=py})
        else
            r.model:Translate({x=px, y=py})
        end

    end,

    rotate = function(r, t, data)
        if data.a == 0 then  
            return
        end
        local delta = t/data.deltat
        local ang = delta * data.a
        local center = {x=data.cx, y=data.cy}
        if data.umc then  
            r.model:Recalculate()
            center = r.model:GetCenter()
        end
        r.model:Rotate(center, math_rad(ang))
    end,

    scale = function(r, t, data)
        local center = {x=data.cx, y=data.cy}
        if data.umc then  
            r.model:Recalculate()
            center = r.model:GetCenter()
        end
        local delta = t / data.deltat
        -- Calculate target scale factor
        local targetScaleX = data.sx
        local targetScaleY = data.sy
        -- Interpolate between 1.0 (no scale) and target scale
        local scaleX = 1.0 + (targetScaleX - 1.0) * delta
        local scaleY = 1.0 + (targetScaleY - 1.0) * delta
        r.model:Scale(center, {x=scaleX, y=scaleY})
    end,
    reset = function(r, t, data, instant)
        if instant then
            r.model:Reset()
        end
    end,
}

function resetTrack(track)
    for i,b in pairs(tools) do  
        for a,c in pairs(track.keyframes) do 
            if c.t  == b then
                track.currentFrame[b] = a

                --print("First for "..b..' is '..a)
                break
            end
        end
    end
    if track.r.model then  
        track.r.model:Reset()
    end
end

function preloadData()
    for idx, anim in pairs(modelAnimation) do

        for trackid, track in pairs(anim.tracks) do
            if not track.currentFrame then  
                --construct data
                track.currentFrame = {}
                
                

                local toolDiff = {}

                local rid = track.r+1
                local name = anim.resources[rid]

                if not name then  
                    error("Unknown resource id "..track.r)
                end

                if modelByName[name] then 
                    track.r = modelByName[name]
                elseif pointGroupByName[name] then  
                    track.r = pointGroupByName[name]
                else 
                    error("Unknown resource"..name)
                end

                for i,b in pairs(tools) do  
                    toolDiff[b] = 0
                    for a, keyframe in pairs(track.keyframes) do 
                        if keyframe.t  == b then
                            local aux = toolDiff[b]
                            toolDiff[b] = keyframe.at
                            keyframe.deltat = keyframe.at - aux
                        end
                    end
                end

                resetTrack(track)
            end
        end
    end
end



function runModelAnim(dt)

    local anim = modelAnimation[1]
    if not anim.prevDt then  
        anim.prevDt = 0
    end

    local prev = anim.prevDt


    for idx, track in pairs(anim.tracks) do
        --for each timeline

        local keyframes = track.keyframes
        for operationName , frameId in pairs(track.currentFrame) do 
            local remaining = dt
            local sumLocal = anim.prevDt

            local nextKf = keyframes[frameId]

            if nextKf then
                local maxIter = 20

                while remaining > 0 do 

                    if sumLocal+remaining > nextKf.at then  
                        local currentStepDt = nextKf.at-sumLocal

                        remaining = remaining - currentStepDt

                        

                        interpolations[operationName](track.r, currentStepDt, nextKf, true)
                        sumLocal = sumLocal + currentStepDt
                        --print("Finished moving["..frameId.."] ["..sumLocal.."/"..nextKf.at.."]Partial transform, stepping "..currentStepDt.." ms. Only "..remaining.." remain")
                        
                        local found = false
                        for i=frameId+1, #keyframes do  
                            nextKf = keyframes[i]
                            if nextKf.t == operationName then  
                                frameId = i
                                track.currentFrame[operationName] = i
                                found = true
                                break
                            end
                        end
                        if not found then 
                            nextKf = nil
                            frameId = #keyframes+1
                            track.currentFrame[operationName] = frameId
                            break
                        end
                    else
                        local fn = interpolations[operationName]
                        if not fn then  
                            error("Unimplemented transformation: "..operationName)
                        end
                        fn(track.r, remaining, nextKf, false)
                        sumLocal = sumLocal + remaining 
                        tnow = sumLocal
                        remaining = 0
                    end

                    maxIter = maxIter -1
                    if maxIter <= 0 then 
                        break
                    end
                end
            end
        end
    end
  
    anim.prevDt = anim.prevDt + dt
    if anim.prevDt >= anim.duration then  
        local res = anim.prevDt-anim.duration
        anim.prevDt = 0
        for idx, track in pairs(anim.tracks) do
            resetTrack(track)
        end
        runModelAnim(res)
    end

end


