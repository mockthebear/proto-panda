local _M = {}
local input = require("input")



function _M.generateUi(title, onEnter, onQuit)
	local uiData = {
		elements = {},
		title = title,
		textScrollingPos = 1,
		maxTextShowSize = 24,
	    textScrollingTimer = 1500,
	    selected = 1,
		onQuit = onQuit,
		onEnter = onEnter,
	}

	if not uiData.onEnter then 
		uiData.onEnter = function()
			uiData.textScrollingPos = 1
		    uiData.textScrollingTimer = 1500
		    uiData.selected = 1
		    uiData.rebuildTitles()
		end
	end


	uiData.rebuildTitles = function()
		for i,b in pairs(uiData.elements) do  
			b.name = b.nameFunction()
		end
	end
	
	uiData.addElement = function(name, action)
		uiData.elements[#uiData.elements+1] = {
			nameFunction = name,
			action = action
		}
	end

	uiData.handle = function(dt)
		local elements = uiData.elements

	    if input.readButtonStatus(BUTTON_DOWN) == BUTTON_JUST_PRESSED then 
	        uiData.selected = uiData.selected+1
	        toneDuration(540, 100)
	        if (uiData.selected > #elements) then 
	            uiData.selected = 1
	        end
	        uiData.textScrollingPos = 1
	        uiData.textScrollingTimer = 1500
	    end
	    if input.readButtonStatus(BUTTON_UP) == BUTTON_JUST_PRESSED then 
	        uiData.selected = uiData.selected-1
	        toneDuration(340, 100)
	        if (uiData.selected < 1) then 
	            uiData.selected = #elements
	        end
	        uiData.textScrollingPos = 1
	        uiData.textScrollingTimer = 1500
	    end
	    
	    if input.readButtonStatus(BUTTON_LEFT) == BUTTON_JUST_PRESSED then 
	        uiData.onQuit()
	        toneDuration(440, 100)
	        return
	    end 

	    if input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_JUST_PRESSED or input.readButtonStatus(BUTTON_RIGHT) == BUTTON_JUST_PRESSED then
	        toneDuration(440, 100) 
	        uiData.elements[uiData.selected].action(uiData.selected, uiData)
	        uiData.rebuildTitles()
	        return false
	    end

	    uiData.textScrollingTimer = uiData.textScrollingTimer - dt
	    if uiData.textScrollingTimer <= 0 then
	        uiData.textScrollingTimer = 200
	        uiData.textScrollingPos = uiData.textScrollingPos+1
	        if uiData.textScrollingPos > (#elements[uiData.selected].name-(uiData.maxTextShowSize-1)) then  
	            uiData.textScrollingTimer = 2500
	        end
	        if uiData.textScrollingPos > (#elements[uiData.selected].name-uiData.maxTextShowSize) then  
	            uiData.textScrollingTimer = 1500
	            uiData.textScrollingPos = 1
	        end
	    end
	    return true
	end

	uiData.draw = function()
		local elements = uiData.elements
        oledSetCursor(0, 0)
        oledDrawText(uiData.title)
        oledDrawLine(0,10, 128, 10, 1)

        local maxPerScreen = 4
        local drawPos = 0
        for i=1,#elements do
            if uiData.selected >= maxPerScreen then  
                drawPos = uiData.selected-maxPerScreen
            end
            if (i-drawPos) <= maxPerScreen and ((i-1)-drawPos) >= 0  then
                oledSetCursor(0, 11 + 11*((i-1)-drawPos))
                if uiData.selected == i then 
                    oledSetTextColor(BLACK, WHITE)
                    oledDrawText(">")
                else
                    oledSetTextColor(WHITE, BLACK)
                end
                local name = elements[i].name
                if #name > uiData.maxTextShowSize then
                    oledDrawText(name:sub(uiData.textScrollingPos, math.min(uiData.textScrollingPos+uiData.maxTextShowSize, #name) ))
                else
                    oledDrawText(name)
                end
            end
        end
        if maxPerScreen < #elements then
            oledSetCursor(OLED_SCREEN_WIDTH/2 - 16, OLED_SCREEN_HEIGHT - 8)
            oledSetTextColor(BLACK, WHITE)
            oledDrawText("\\/  \\/ \\/")
        end
        oledDisplay()
	end

	return uiData
end

return _M