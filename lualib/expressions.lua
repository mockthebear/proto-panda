local configloader = require("configloader")
local models = require("models")
local _M = {
	animations = {},
	by_name = {},
	by_frame = {},
	count = 0,
}

function _M.loadSingleExpression(data, filename, i)
	local id = #_M.animations+1

	if data.tracks then  
		local success, err = models.loadAnimation(data)
	    if not success then  
	    	error("Failed to load animation "..i.." at file "..filename..": "..err)
	    end

		data.id = id
		data.isModel = true
		data.modelAnimId = data.obj:GetId()
		_M.by_name[data.name] = id
		_M.by_frame[MODEL_FRAME_ID_OFFSET + data.modelAnimId] = id
		if not data.transition then 
			_M.count = _M.count+1
		end

		_M.animations[#_M.animations+1] = data
	else
		local offset = nil 
		data.id = id
		if data.frame_offset and type(data.frames) == "number" then 
			offset = data.frame_offset
		elseif data.frames and type(data.frames) == "string" then 
			offset = getFrameOffsetByName(data.frames)
		end
		if offset then 
			data.frame_offset = offset
			local animation = data.animation
			if type(data.animation) ~= 'table' then  
				if data.animation == "auto" then 
					data.animation = "loop" --Compatibility
				end
				if data.animation == "loop" or data.animation == "auto_backwards" or data.animation == "loop_backwards" or data.animation == "pingpong" then 
					local name = data.animation
					data.animation = {}
					if not data.frames then  
						error("Cannot use 'frames=loop' when there is no alias defined")
					end
					local count = getFrameCountByName(data.frames)
					if count == 0 then  
						error("Using frame group '"..data.frames.."' returned 0 frames. Are you sure this alias has loaded frames?")
					end 
					for i=1,count do  
						local idx = i  
						if name == "loop_backwards" or name == 'auto_backwards' then 
							idx = count-i+1
						end
						data.animation[i] = idx
					end
					if name == "pingpong" then  
						for i=1,count do  
							data.animation[#data.animation + 1] = count-i+1
						end
					end
					animation = data.animation
				else 
					error("Animation can only have numeric array or 'loop', got "..tostring(data.animation).." in animation id "..tostring(id))
				end
			end
			for a,c in pairs(data.animation) do 
				animation[a] = c + offset
				_M.by_frame[c + offset] = id
			end
			data.animation = animation
		end
		data.frame_offset = data.frame_offset or 0
		data.duration = tonumber(data.duration) or 250
		data.name = data.name or "expression "..(id)
			if data.onEnter then 
			local f = load(data.onEnter)
			if not f then 
				error("Error loading 'onEnter', contains syntax error")
			end
			data.onEnter = f
		end
		if data.onLeave then 
			local f = load(data.onLeave)
			if not f then 
				error("Error loading 'onLeave', contains syntax error")
			end
			data.onLeave = f
		end
			_M.by_name[data.name] = id
		if not data.transition then 
			_M.count = _M.count+1
		end
		_M.animations[#_M.animations+1] = data
	end
end

function _M.Load()
	generic.displaySplashMessage("Starting:\nModel Expressions")
	content = nil
	local conf = configloader.Get()


	local conf = configloader.Get()
	if conf.models_folder then
		local files = listFilesInFolder(conf.models_folder)
		for i,file in pairs(files) do 
			local fullPath = conf.models_folder..file 
			local modelAnim = models.getModelAnimationList(fullPath)
			if modelAnim then
				for id , anim in pairs(modelAnim) do 
					_M.loadSingleExpression(anim, fullPath, id)
				end
			end
		end
	end

	generic.displaySplashMessage("Starting:\nExpressions")
	for i ,b in pairs(conf.expressions) do 
		_M.loadSingleExpression(b, "/animation.json", i)
	end

	local res = {}
	for __, data in pairs(_M.animations) do 
		if not data.transition and not data.hidden then
			res[#res+1] = data.name
		end
	end
	_M.avaliableAnimations = res
	return true
end

function _M.GetExpression(name)
	if type(name) == 'string' then 
		name = _M.by_name[name] or name
	end
	return _M.animations[name] 
end


function _M.GetExpressionName(name)
	return _M.by_name[name]
end

function _M.GetCurrentExpressionId()
	local storage = getCurrentAnimationStorage()
	if storage == -1 then
		local frameId = getPanelCurrentFace()
		local animId = _M.by_frame[frameId]
		if not animId then 
			return -1, frameId
		end
		return animId, frameId
	else 
		return storage, getPanelCurrentFace()	
	end
end

function _M.GetCurrentFrame()
	local expId, frameId = _M.GetCurrentExpressionId()
	local exp = _M.animations[expId]
	if not exp then 
		return -1
	end
	return frameId - (exp.frame_offset or 0)
end

function _M.GetAnimationNameByFace(faceid)
	local idx = _M.by_frame[faceid]
	if not idx then 
		return "none"
	end
	local aux = _M.animations[idx]
	if not aux then 
		return "none"
	end
	return aux.name
end


function _M.Next(id)
	if not id then
		id = _M.GetCurrentExpressionId()
	end
	if (id == -1) then 
		id = 0
	end
	id = id +1 
	if id > #_M.animations then 
		id = 1
	end
	if _M.animations[id] and (_M.animations[id].transition or _M.animations[id].hidden) then 
		return _M.Next(id)
	end
	return _M.SetExpression(id)
end

function _M.Previous(id)
	if not id then
		id = _M.GetCurrentExpressionId()
	end
	if (id == -1) then 
		id = #_M.animations+1
	end
	id = id -1 
	if id <= 0 then 
		id = #_M.animations
	end
	if _M.animations[id] and (_M.animations[id].transition  or _M.animations[id].hidden) then 
		return _M.Previous(id)
	end
	return _M.SetExpression(id)
end


function _M.SetExpression(id)
	local aux = _M.GetExpression(id)
	if aux then 
		local repeats = aux.repeats or -1
		local allDrop = true
		if aux.transition then 
			if repeats == -1 then  
				repeats = 1
			end
			allDrop = false
		end
		if _M.previousExpression and _M.previousExpression.onLeave then 
			_M.previousExpression.onLeave()
		end
		setPanelManaged(false) --To avoid frame flicker
		local current_id = aux.id 
		if aux.isModel then  
			setPanelModelAnimation(aux.modelAnimId, repeats, allDrop, current_id)
		else
			setPanelAnimation(aux.animation, aux.duration, repeats, allDrop, current_id)
		end

		if aux.intro then
			_M.StackExpression(aux.intro)
		end

		if _M.previousExpression and _M.previousExpression.outro then  
			_M.StackExpression(_M.previousExpression.outro)
		end
		setPanelManaged(true)

		if aux.onEnter then 
			aux.onEnter()
		end

		_M.previousExpression = aux
		return aux
	end
	return nil
end

function _M.IsFrameFromAnimation(frameId, id)
	local aux = _M.GetExpression(id)
	if aux then 
		for i,b in pairs(aux.animation) do 
			if b == frameId then
				return true
			end
		end
	end
	return false
end

function _M.StackExpression(id)
	local aux = _M.GetExpression(id)
	if aux then 
		local repeats = aux.repeats or -1
		if aux.transition then 
			if repeats == -1 then  
				repeats = 1
			end
		end
		local current_id = aux.id 
		if aux.isModel then  
			setPanelModelAnimation(aux.modelAnimId, repeats, false, current_id)
		else
			setPanelAnimation(aux.animation, aux.duration, repeats, false, current_id)
		end
	else
		log("Unknown ID: "..tostring(id))
	end
end

function _M.GetExpressions()
	return _M.avaliableAnimations
end

function _M.GetExpressionCount()
	return _M.count
end


return _M
