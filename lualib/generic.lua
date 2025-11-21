local _M = {

}

function _M.map(x, in_min, in_max, out_min, out_max) 
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
end

function _M.setAutoPowerMode(brightness)
	setPoweringMode(BUILT_IN_POWER_MODE)
	if BUILT_IN_POWER_MODE == POWER_MODE_USB_9V or BUILT_IN_POWER_MODE == POWER_MODE_BATTERY then 
		waitForPower(brightness)
	else
		panelPowerOn()
	end
end

function _M.displaySplashMessage(msg)
	oledClearScreen()
	oledSetCursor(0,0)
	oledSetFontSize(2)
	oledDrawText(msg)
	oledDisplay()
	oledSetFontSize(1)
end

function _M.displayWarning(headMessage, message, duration)
	duration = duration or 2000
	log(headMessage..": "..message)
	local stop = millis()+duration
	local scroll = _M.scrollingText(message, 20, 250)
	while scroll.cycles <= 0 or stop > millis() do
		oledClearScreen()
		oledSetCursor(0,0)
		oledSetFontSize(1)
		oledDrawText(headMessage)
		oledDrawLine(0, 15, OLED_SCREEN_WIDTH, 15, WHITE)
		oledSetFontSize(2)
		oledSetCursor(0,16)
		scroll:update()
		oledDrawText(scroll:text())
		oledSetFontSize(1)
		oledDisplay()
	end
end

function _M.scrollingText(message, maxSize, stepDuration)
	return {
		message = message,
		maxTextShowSize=maxSize,
		textScrollingPos = 1,
    	textScrollingTimer = 1500,
    	stepDuration=stepDuration,
    	lastDt = millis(),
    	cycles=0,

    	update = function(self, dt)
    		if not dt then 
    			dt = millis()-self.lastDt
    			self.lastDt = millis()
    		end
    		self.textScrollingTimer = self.textScrollingTimer - dt
		    if self.textScrollingTimer <= 0 then
		        self.textScrollingTimer = self.stepDuration
		        self.textScrollingPos = self.textScrollingPos+1
		        if self.textScrollingPos > (#message-(self.maxTextShowSize-1)) then  
		            self.textScrollingTimer = 2500
		        end
		        if self.textScrollingPos > (#message-self.maxTextShowSize) then  
		            self.textScrollingTimer = 1500
		            self.textScrollingPos = 1
		            self.cycles = self.cycles+1
		        end
		    end
    	end,

    	text = function(self)
    		if #self.message > self.maxTextShowSize then
               return self.message:sub(self.textScrollingPos, math.min(self.textScrollingPos+self.maxTextShowSize, #self.message))
            end
            return self.message
    	end,
	}
end

function _M.DrawSprite(xpos,ypos, w, h, data)
	local pix = 1
	for y=1, h do 
        for x=1, w do 
        	local pixdata = data[pix] 
            if pixdata ~= 0 and pixdata ~= nil then 
                drawPanelPixel(xpos+x, ypos+y, pixdata)
            end
            pix = pix+1
        end
    end
end

function _M.DrawText(x,y, str, color, bg)
	bg = bg or 0
	for i=1,#str do  
		drawPanelChar(x + (i-1)*6, y, string.byte(str:sub(i,i)), color, bg, 1)
	end
end

return _M