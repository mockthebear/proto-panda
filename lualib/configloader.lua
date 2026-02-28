local _M = {}

local json = require("json")
local files = {"/config.json"}

function _M.LoadAndMerge(filename)
	local fp, err = io.open(filename, "r")
	if not fp then 
		error("Failed to load "..filename..": "..tostring(err))
	end
	local content = fp:read("*a")
	fp:close()
	json.filename = filename
	local content = json.decode(content)
	for i,b in pairs(content) do  
		if _M.config[i] then
			log("[WARNING] rewriting config "..i..", done by "..filename)
		end
		_M.config[i] = b
	end
end

function _M.Load()
	_M.config = {}

	_M.LoadAndMerge("/config.json")

	if not _M.config.expressions or #_M.config.expressions == 0 then  
		error("Config is missing 'expressions'")
	end

	_M.LoadAndMerge("/keybinds.json")
	_M.LoadAndMerge("/misc.json")
	_M.LoadAndMerge("/wifi.json")

	if not _M.config.scripts or #_M.config.scripts == 0 then  
		error("Config is missing 'scripts'")
	end

	return
end

function _M.Get()
	return _M.config
end

return _M