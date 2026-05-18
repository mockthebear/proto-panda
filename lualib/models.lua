local json = require("json")

local _M = {
	models = {},
	resourceByName = {},
	animationsByName = {},
	animations = {},
}


local keyframeTypeMap = {
    ["translate"] = function(tr, obj) 
    	obj.x = obj.x or 0
    	obj.y = obj.y or 0
    	if type(obj.x) ~= 'number' then 
    		return nil, "x is not a number"
    	end
    	if type(obj.y) ~= 'number' then 
    		return nil, "x is not a number"
    	end
    	return tr:AddKeyFrame(KeyFrame(KEYFRAME_TRANSLATE, obj.at, {x=obj.x, y=obj.y}, {x=obj.cx or 0,y=obj.cy or 0}, 0, obj.i == true)) 
    end,


    ["rotate"] = function(tr, obj) 
    	obj.a = obj.a or 0
    	obj.cx = obj.cx or 0
    	obj.cy = obj.cy or 0
    	if type(obj.a) ~= 'number' then 
    		return nil, "a is not a number"
    	end
    	if type(obj.cx) ~= 'number' then 
    		return nil, "cx is not a number"
    	end
    	if type(obj.cy) ~= 'number' then 
    		return nil, "cy is not a number"
    	end
    	return tr:AddKeyFrame(KeyFrame(KEYFRAME_ROTATE, obj.at, {x=obj.a, y=0}, {x=obj.cx,y=obj.cy}, 0, obj.i == true, obj.umc == true)) 
    end,

    ["scale"] = function(tr, obj) 
    	obj.sx = obj.sx or 0
    	obj.sy = obj.sy or 0
    	obj.cx = obj.cx or 0
    	obj.cy = obj.cy or 0
    	if type(obj.sx) ~= 'number' then 
    		return nil, "sx is not a number"
    	end
    	if type(obj.sy) ~= 'number' then 
    		return nil, "sx is not a number"
    	end
    	if type(obj.cx) ~= 'number' then 
    		return nil, "cx is not a number"
    	end
    	if type(obj.cy) ~= 'number' then 
    		return nil, "cy is not a number"
    	end
    	return tr:AddKeyFrame(KeyFrame(KEYFRAME_SCALE, obj.at, {x=obj.sx, y=obj.sy}, {x=obj.cx,y=obj.cy}, 0, obj.i == true, obj.umc == true)) 
    end,
    ["reset"] = function(tr, obj) 
    	return tr:AddKeyFrame(KeyFrame(KEYFRAME_RESET, obj.at, {x=0, y=0})) 
	end,
	["color"] = function(tr, obj) 
		if type(obj.color) ~= 'number' then  
			return nil, 'color is not a number'
		end	
    	return tr:AddKeyFrame(KeyFrame(KEYFRAME_COLOR, obj.at, {x=0, y=0}, {x=0,y=0}, obj.color, obj.i == true, false)) 
	end,
	["visibility"] = function(tr, obj) 
		if type(obj.v) ~= 'boolean' then  
			return nil, 'v is not a boolean ('..type(obj.v)..')'
		end	
    	return tr:AddKeyFrame(KeyFrame(KEYFRAME_VISIBILITY, obj.at, {x=0, y=0}, {x=0,y=0}, 0, true, obj.v)) 
	end,
	["sine"] = function(tr, obj) 
		if type(obj.amplitude_x) ~= 'number' then  
			return nil, 'amplitude_x is not a number ('..type(obj.amplitude_x)..')'
		end	
		if type(obj.amplitude_y) ~= 'number' then  
			return nil, 'amplitude_y is not a number ('..type(obj.amplitude_y)..')'
		end	
		if type(obj.rotations_x) ~= 'number' then  
			return nil, 'rotations_x is not a number ('..type(obj.rotations_x)..')'
		end	
		if type(obj.rotations_y) ~= 'number' then  
			return nil, 'rotations_y is not a number ('..type(obj.rotations_y)..')'
		end	
		local mode = 0
		if obj.x_sin == true then  
			mode = 1
		end
		if obj.y_sin == true then  
			mode = mode+2
		end
    	return tr:AddKeyFrame(KeyFrame(KEYFRAME_SINE, obj.at, {x=obj.amplitude_x, y=obj.amplitude_y}, {x=obj.rotations_x,y=obj.rotations_y}, mode, false, false)) 
	end,
	["shader"] = function(tr, obj) 
		if type(obj.shader) ~= 'string' then  
			return nil, 'shader is not a string ('..type(obj.string)..')'
		end	
		if not obj.shader:match("SHADER_") then  
			return nil, "Invalid shader"
		end

		local shdr = _G[obj.shader]
		if type(shdr) ~= 'number' then  
			return nil, "Invalid shader"
		end

    	return tr:AddKeyFrame(KeyFrame(KEYFRAME_SHADER, obj.at, {x=0, y=0}, {x=0,y=0}, shdr, obj.i == true, false)) 
	end
}


function _M.loadModelsFromJson(filename)
    
    local fp, err = io.open(filename, "r")
    if not fp then 
        return nil,"Failed to load "..filename..": "..tostring(err)
    end

    local content = fp:read("*a")
    fp:close()
    json.filename = filename
    local content = json.decode(content)

    if content.models then
	    for i,model in pairs(content.models) do  
	        local md = loadModel(model, model.name)
	        if not md then  
	            return nil, "Error loading model: "..model.name
	        end

	       	_M.models[md:GetId()] = md
	        _M.resourceByName[model.name] = {model=md, type="model"}


	        if model.pointGroups and #model.pointGroups > 0 then 
	            for idx,pg in pairs(model.pointGroups) do 
	                local pgid = md:AddPointGroup(pg.indices)
	                _M.resourceByName[model.name.."."..idx]  = {id=pgid, model=md, type="pointgroup"}
	            end
	        end
	    end
	end
	return true
end

function _M.getModelAnimationList(filename)
    
    local fp, err = io.open(filename, "r")
    if not fp then 
        error("Failed to load "..filename..": "..tostring(err))
    end

    local content = fp:read("*a")
    fp:close()
    json.filename = filename
    local content = json.decode(content)

	return content.animations
end

function _M.loadAnimation(anim)
	if not anim.name or type(anim.name) ~= 'string' then  
		return nil, "Animation must contain a string name (".. type(anim.name)..")"
	end
	if not anim.duration or type(anim.duration) ~= 'number' then  
		return nil, "Animation must contain a number duration in ms  (".. type(anim.duration)..")"
	end
	if not anim.tracks or type(anim.tracks) ~= 'table' then  
		return nil, "Animation must contain a table with tracks  (".. type(anim.tracks)..")"
	end
	if #anim.tracks == 0 then 
		return nil, "Animation has zero tracks"
	end
	local kfAnim = newKeyframeAnimation(anim.duration)
	for idx, track in pairs(anim.tracks) do 
		if not track.modelname or type(track.modelname) ~= 'string' then  
			return nil, "At track "..idx..", no valid 'modelname'"
		end
		if not track.keyframes or type(track.keyframes) ~= 'table' then  
			return nil, "At track "..idx..", no valid 'keyframes'"
		end
		if not _M.resourceByName[track.modelname] then  
			return nil, "At track "..idx..", there is no modelname: '"..track.modelname.."'"
		end
		if #track.keyframes > 0 then
			local tr = KeyframeTrack()
			if not tr:SetResource(track.modelname) then  
				return nil, "At track "..idx..", invalid model resource: '"..track.modelname.."'"
			end
			local ok, err = _M.parseTrack(tr, track.keyframes)
			if not ok then  
				return nil, "At track "..idx..", "..err
			end
			kfAnim:AddTrack(tr)
	    end
	end
	anim.obj = kfAnim
	_M.animationsByName[anim.name] = anim
	_M.animations[#_M.animations+1] = anim
	return kfAnim, nil
end

function _M.parseTrack(track, frames)
	for idx,frame in pairs(frames) do  
		if not frame.t or type(frame.t) ~= 'string' then  
			return nil, "At keyframe "..idx..", type t is not defined"
		end
		local adder = keyframeTypeMap[frame.t]
		if not adder then  
			return nil, "At keyframe "..idx..", type t "..frame.t.." is invalid"
		end

		if not frame.at or type(frame.at) ~= 'number' or frame.at < 0 then  
			return nil, "At keyframe "..idx..", value for at is invalid"
		end

		local ok, err = adder(track, frame)
		if not ok then  
			return nil, "At keyframe "..idx..", "..err
		end
	end
	return true
end

function _M.Load()
	generic.displaySplashMessage("Starting:\nModels")
	local conf = configloader.Get()
	if conf.models_folder then
		local files = listFilesInFolder(conf.models_folder)
		for i,file in pairs(files) do  
			local fullPath = conf.models_folder..file
			local ok, err = _M.loadModelsFromJson(fullPath)
			if not ok then 
				error(err)
			end
		end
	else
		log("Skipping loading models for 'models_folder' not beeing defined")	
	end
end

return _M