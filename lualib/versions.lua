local _M = {}

function _M.parseVersion(version)
	local release, major, minor = version:match("(%d+)%.(%d+)%.(%d+)")
	if not release then 
		release, major = version:match("(%d+)%.(%d+)")
		minor = 0
	end
	release = tonumber(release) or 0
	major = tonumber(major) or 0
	minor = tonumber(minor) or 0
	if release == 0 then 
		error("Invalid panda version: "..tostring(version))
	end
	return release, major, minor
end


function _M.canRun(version)
	local pandaRelease, pandaMajor, pandaMinor = _M.parseVersion(PANDA_VERSION)
	if not pandaRelease then
		error("Invalid panda version")
	end
	
	if not version then 
		return true, "No version defined"
	end

	local scriptRelease, scriptMajor, scriptMinor = _M.parseVersion(version)
	if not scriptRelease then 
		return false, "Failed to parse script version"
	end


	if pandaRelease > scriptRelease then 
		return true
	elseif scriptRelease > pandaRelease then 
		return false, "Require firmware release to be "..scriptRelease..", but got "..pandaRelease..". Required version "..version
	end


	if pandaMajor > scriptMajor then  
		return true  
	elseif scriptMajor > pandaMajor then 
		return false, "Require firmware major to be "..scriptMajor..", but got "..pandaMajor..". Required version "..version
	end

	if scriptMinor > pandaMinor then 
		return false, "Require firmware minor to be "..scriptMinor..", but got "..pandaMinor..". Required version "..version
	end

	return true
end

return _M