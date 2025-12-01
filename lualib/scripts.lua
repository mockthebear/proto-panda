local _M = {
    previous_state = {},
    running_script = false
}


local configloader = require("configloader")
local generic = require("generic")
local versions = require("versions")

function _M.Load(filepath)
	local conf = configloader.Get()
    local auxScripts = {}
    for a,c in pairs(conf.scripts) do  
    	local success, data = pcall(dofile, c.file)
    	if not success then 
    		generic.displayWarning("On script: "..c.file, data)
    	else
    		local run, message = versions.canRun(data.VERSION_REQUIRED)
    		if message ~= nil then 
    			generic.displayWarning("On script: "..c.file, message)
    		end
    		if run then 
    			auxScripts[#auxScripts+1] = c
    		end 
    		data = nil
    	end
    end
    _M.scripts = auxScripts
    collectgarbage()
end

function _M.StoreState() 
	_M.previous_state[#_M.previous_state+1] = {
		panel_managed = isPanelManaged(),
		led_managed = ledsIsManaged(),
		led_brightness = ledsGetBrightness(),
		panel_brightness = getPanelBrightness(),
	}
end

function _M.GetScripts() 
	return _M.scripts
end

function _M.PopState() 
	local res = _M.previous_state[#_M.previous_state]
	if not res then 
		return
	end
	setPanelManaged(res.panel_managed)
	ledsSetManaged(res.led_managed)

	if ledsGetBrightness() ~= res.led_brightness then
		ledsGentlySeBrightness(res.led_brightness, 1, ledsGetBrightness())
	end

	if getPanelBrightness() ~= res.panel_brightness then
		gentlySetPanelBrightness(res.led_brightness, 1, res.panel_brightness)
	end


	_M.previous_state[#_M.previous_state] = nil
end

function _M.Handle(dt) 
	if not _M.running_script then 
		return true
	end

	local success, res = pcall(_M.running_script.onLoop, dt)
	if not success or _M.running_script.shouldStop then 
		if not success then
			generic.displayWarning("Runtime error", res)
			print("Error running script: "..tostring(res))
		end
		_M.running_script.onClose()
		_M.running_script = nil
		_M.PopState() 
	end

	return false	
end

function _M.StartScript(id) 
	local res = _M.scripts[id]
	if not res then  
		return false
	end
	_M.StoreState() 
	local data = dofile(res.file)
	_M.running_script = data
	oledClearScreen()
	oledDisplay()
	_M.running_script.onSetup(res)
end


return _M