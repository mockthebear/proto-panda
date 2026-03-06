local expressions = require("expressions")
local generic = require("generic")
local configloader = require("configloader")
local input = require("input")

local DrawText = generic.DrawText
local DrawSprite = generic.DrawSprite
local Map = generic.map
local GetCurrentFrame = expressions.GetCurrentFrame
local StackExpression = expressions.StackExpression
local IsFrameFromAnimation = expressions.IsFrameFromAnimation

local _M = {
    boopIsOn = false,
    config = {
    },   
    avgRead = 0,
    mode = "lidar",
    
    calibration_stage = 1,
    measurementsMax = 50,
    avg = 0,
    measurements=0,
    minimumLidar=0,
    triggerPosition=0,
    maxLidarProcessing = 400,


    boopTimerDuration = 500,
    boopTimer = 0,


    isBooped = false,
}


function _M.Load()
    _M.gear_sprite = decodePng("/lualib/gear.png")
    _M.config = configloader.Get().boop



    if not _M.config["trigger_mode"] then
        _M.config["trigger_mode"] = "lidar"
    end


    _M.mode = _M.config["trigger_mode"]
    if _M.mode == "gpio" then  
        if type(_M.config["gpio"]) ~= 'number' then  
            error("Boop mode gpio is enabled. The field 'gpio' is not present or is not a number")
        end
        if type(_M.config["gpio_state"]) ~= 'number' then  
            error("Boop mode gpio is enabled. The field 'gpio' is not present or is not a number")
        end
        pinMode(_M.config["gpio"], INPUT)
        _M.gpio = _M.config["gpio"]
        _M.gpio_state = _M.config["gpio_state"]
    end
    print("Mode is ".._M.mode)

    if _M.config["transitionIn"] then
        if type(_M.config["transitionIn"]) ~= "string" then 
            error("Field 'triggerStop' should be an string")
        end
        local exp = expressions.GetExpression(_M.config["transitionIn"])
        if not exp then 
            error("On boop 'transitionIn' is defined the ".._M.config["transitionIn"].." but thats not a valid expression")
        end
        if not exp.transition then 
            error("On boop 'transitionIn' the animation should have transition=true")
        end
    end

    if _M.config["transitionOut"] then
        if type(_M.config["transitionOut"]) ~= "string" then 
            error("Field 'triggerStop' should be an string")
        end
        local exp = expressions.GetExpression(_M.config["transitionOut"])
        if not exp then 
            error("On boop 'transitionOut' is defined the ".._M.config["transitionOut"].." but thats not a valid expression")
        end
        if not exp.transition then 
            error("On boop 'transitionOut' the animation should have transition=true")
        end
    end

    if _M.config["boopAnimationName"] then
        if type(_M.config["boopAnimationName"]) ~= "string" then 
            error("Field 'boopAnimation' should be an string")
        end
        local exp = expressions.GetExpression(_M.config["boopAnimationName"])
        if not exp then 
            error("On boop 'boopAnimationName' is defined the ".._M.config["boopAnimationName"].." but thats not a valid expression")
        end
        if exp.transition then 
            error("On boop 'boopAnimationName' the animation should have transition=false")
        end
    end
    if _M.config["transictionOnlyOnAnimation"] then
        if type(_M.config["transictionOnlyOnAnimation"]) ~= "string" then 
            error("Field 'transictionOnlyOnAnimation' should be an string")
        end
        local exp = expressions.GetExpression(_M.config["transictionOnlyOnAnimation"])
        if not exp then 
            error("On boop 'transictionOnlyOnAnimation' is defined the ".._M.config["transictionOnlyOnAnimation"].." but thats not a valid expression")
        end
        if exp.transition then 
            error("On boop 'transictionOnlyOnAnimation' the animation should have transition=false")
        end
    end

    if _M.config["transictionInOnlyOnSpecificFrame"] then
        if type(_M.config["transictionInOnlyOnSpecificFrame"]) ~= "number" then 
            error("Field 'transictionInOnlyOnSpecificFrame' should be an number")
        end
    end 
    
    if _M.config["transictionOutOnlyOnSpecificFrame"] then
        if type(_M.config["transictionOutOnlyOnSpecificFrame"]) ~= "number" then 
            error("Field 'transictionOutOnlyOnSpecificFrame' should be an number")
        end
    end

    if dictGet("boop_configured") ~= "1" then  
        print("Boop is not configured yet.")
        _M.configured = false
        return
    end
    _M.configured = true

    _M.minimumLidar = tonumber(dictGet("boop_min") ) or 0
    _M.triggerPosition = tonumber(dictGet("boop_trigg"))  or 0
    _M.boopTimerDuration = tonumber(dictGet("boop_duration")) or 500

    print("Boop configuration loaded min=".._M.minimumLidar.." trigger=".._M.triggerPosition.." timer=".._M.boopTimerDuration)

    if _M.triggerPosition == 0 then 
        print("Boop is not configured yet!")
        _M.configured = false
        return
    end
end


function _M.onEnter()
    _M.old_managed = isPanelManaged()
    setPanelManaged(false)
    delay(500)
    toneDuration(440, 500)
    _M.calibration_stage = 1
    _M.triggerPosition=0
    _M.quit = false
    if _M.mode ~= "lidar" then  
        if _M.calibration_stage < 6 then  
            _M.calibration_stage = 6
            if _M.triggerPosition == 0 then 
                _M.triggerPosition = 100
            end
            if _M.minimumLidar == 0 then 
                _M.minimumLidar = 50
            end
        end
    else
        if not hasLidar() then  
            log("Attempted to calibrate boop, but lidar is not present")
            generic.displayWarning("No lidar", "No lidar present", 2000)
            return false
        end
    end
    return true
end

function _M.EnterDisplayConfig()
    if not hasLidar() then  
        return false
    end
    _M.old_managed = isPanelManaged()
    setPanelManaged(false)
    delay(500)
    toneDuration(440, 500)
    _M.calibration_stage = 7
    _M.quit = false
    return true
end


function _M.Calibrate(dt)
    
    if input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_JUST_PRESSED then 
        if _M.calibration_stage == 1 then
            toneDuration(440, 500)
            _M.calibration_stage = 2
            _M.measurements = 0
            _M.avg = 0
        elseif _M.calibration_stage == 3 then
            toneDuration(440, 500)
            _M.calibration_stage = 4
            _M.measurements = 0
            _M.avg = 0
        elseif _M.calibration_stage == 5 then
            toneDuration(440, 500)
            _M.calibration_stage = 6
        elseif _M.calibration_stage == 6 then
            toneDuration(440, 500)
            _M.calibration_stage = 7
            _M.boopTimer = 0
            _M.boopTimerDuration = 500
            _M.calibrationReading = 0
            _M.calibrationOk = 0
        elseif _M.calibration_stage == 7 then
            _M.calibration_stage = 8
            toneDuration(440, 500)
        elseif _M.calibration_stage == 8 then
            _M.calibration_stage = 9
            toneDuration(440, 500)
            _M.quit = true
            dictSet("has_boop", "1")
            dictSet("boop_configured", "1")
            dictSet("boop_min", tostring(_M.minimumLidar))
            dictSet("boop_trigg", tostring(_M.triggerPosition))
            dictSet("boop_duration", tostring(_M.boopTimerDuration))
            dictSave()
            if _M.old_managed then
               setPanelManaged(true)
            end
        end
    end
    if input.readButtonStatus(BUTTON_LEFT) == BUTTON_JUST_PRESSED then 
        if _M.calibration_stage == 5 then
            toneDuration(640, 500)
            _M.calibration_stage = 4
        end
    end
    if input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_PRESSED  then 
        local reading = readLidar() 
        if reading < 8190 then
            _M.avg =  _M.avg + reading
            _M.measurements = _M.measurements +1
        end
        if _M.measurements >= _M.measurementsMax then  
            if _M.calibration_stage == 2 then
                _M.calibration_stage = 3
                _M.minimumLidar = math.floor(_M.avg /_M.measurements)
            elseif _M.calibration_stage == 4 then
                _M.calibration_stage = 5
                _M.triggerPosition = math.floor(_M.avg /_M.measurements)
                if _M.triggerPosition > _M.maxLidarProcessing then  
                    _M.triggerPosition = _M.maxLidarProcessing
                end
            end
        end
    else
        _M.avg = 0
        _M.measurements = 0
    end

    if _M.calibration_stage == 7 then 
        if input.readButtonStatus(BUTTON_UP) == BUTTON_JUST_PRESSED then 
            if _M.boopTimerDuration == 1 then 
                _M.boopTimerDuration = 0
            end
            _M.boopTimerDuration = _M.boopTimerDuration + 50
            toneDuration(340, 50)
            _M.boopTimer = 0
        end
        if input.readButtonStatus(BUTTON_DOWN) == BUTTON_JUST_PRESSED then 
            _M.boopTimerDuration = _M.boopTimerDuration - 50
            toneDuration(640, 50)
            _M.boopTimer = 0
            if _M.boopTimerDuration <= 0 then  
                _M.boopTimerDuration = 1
            end
        end
        _M.calibrationReading, _M.calibrationOk = _M.readLidar()
        _M.isBoopedCheck(_M.calibrationReading, _M.calibrationOk, dt)
    end
    if input.readButtonStatus(BUTTON_BACK) == BUTTON_JUST_PRESSED then 
        _M.quit = true
        if _M.old_managed then
           setPanelManaged(true)
        end
    end
end


function _M.CalibrateDraw()
    oledClearScreen()
    if _M.calibration_stage == 1 then 
        clearPanelBuffer()
        DrawSprite(32-8/2,16-8/2, 8, 8, _M.gear_sprite)
        DrawSprite(64+32-8/2,16-8/2, 8, 8, _M.gear_sprite)
        flipPanelBuffer()
        oledSetCursor(0, 0)
       
        oledDrawText("Calibarion will\nhappen in 4 steps\nPress confirm to\nstart")
        oledSetFontSize(1)
        oledSetCursor(32, 64-8)
        oledDrawText("[Confirm] Next")
        
    elseif _M.calibration_stage == 2 then 
        clearPanelBuffer()
        DrawSprite(32-8/2,16-8/2, 8, 8, _M.gear_sprite)
        DrawSprite(64+32-8/2,16-8/2, 8, 8, _M.gear_sprite)
        flipPanelBuffer()
        oledSetCursor(0, 0)       
        oledDrawText("Place your hand ~2cm\nfrom touching the\nfront of visor and\nhold confirm.\n\n".._M.measurements..'/'.._M.measurementsMax)
        oledSetFontSize(1)

        oledSetCursor(16, 64-8)
        oledDrawText("[Hold Confirm] Next")
      
    elseif _M.calibration_stage == 3 then 
        clearPanelBuffer()
        DrawSprite(32-8/2,16-8/2, 8, 8, _M.gear_sprite)
        DrawSprite(64+32-8/2,16-8/2, 8, 8, _M.gear_sprite)
        flipPanelBuffer()
        oledSetCursor(0, 0)       
        oledDrawText("Min: ".._M.minimumLidar..'\nPress confirm for \nnext step')
        oledSetFontSize(1)
        oledSetCursor(32, 64-8)
        oledDrawText("[Confirm] Next")
  
    elseif _M.calibration_stage == 4 then 
        clearPanelBuffer()
        local reading, ok = _M.readLidar()
        _M.drawLidarBar(reading, ok)
        oledSetCursor(0, 12) 
        oledDrawText("Move your hand on the\ndistance you want to\nstart the booping\nR: "..math.floor(reading).."mm\n".._M.measurements..'/'.._M.measurementsMax)
        oledSetFontSize(1)
        oledSetCursor(16, 64-8)
        oledDrawText("[Hold Confirm] Next")
        flipPanelBuffer()
  
    elseif _M.calibration_stage == 5 then 
        clearPanelBuffer()
        
        local reading, ok = _M.readLidar()
        oledSetCursor(20, 24) 
        if _M.drawLidarBar(reading, ok) then  
           
            oledSetFontSize(2)
            oledDrawText("BOOP")
            DrawText(17, 23, "BOOP", color565(255,255,255), color565(0,0,0))
            DrawText(64+17, 23, "BOOP", color565(255,255,255), color565(0,0,0))
        else
            oledSetFontSize(2)
            oledDrawText("----")
            DrawText(17, 23, "----", color565(255,255,255), color565(0,0,0))
            DrawText(64+17, 23, "----", color565(255,255,255), color565(0,0,0))
        end
        oledSetFontSize(1)
        oledSetCursor(0, 38) 
        oledDrawText("This is ok?")
        oledSetCursor(10, 64-8)
        oledDrawText("[<<<] Back [Confirm] Next")
        flipPanelBuffer()
    elseif _M.calibration_stage == 6 then 
        oledSetCursor(0, 0) 
        oledDrawText("To avoid unwanted tr-igger. Lets configure\na timer in next step\n[UP] increase the timer\n[DOWN] decreases")
        oledSetFontSize(1)
        oledSetCursor(32, 64-8)
        oledDrawText("[Confirm] Next")

        DrawSprite(32-8/2,16-8/2, 8, 8, _M.gear_sprite)
        DrawSprite(64+32-8/2,16-8/2, 8, 8, _M.gear_sprite)
        flipPanelBuffer()
    elseif _M.calibration_stage == 7 then 
        clearPanelBuffer()
        
        oledSetCursor(20, 24) 
        _M.drawLidarBar(_M.calibrationReading, _M.calibrationOk)

        drawPanelRect(0, 16, 16, 16, color565(255,255,255))
        drawPanelRect(64+16, 16, 16, 16, color565(255,255,255))
        oledDrawRect(0, 22, 16, 16, 1)

        local ratio = (_M.boopTimer/_M.boopTimerDuration)
        if (ratio < 0) then  
            ratio = 0
        end
        if (ratio > 1) then  
            ratio = 1
        end

        if ratio*14 > 0 then
            oledDrawFilledRect(1,23, ratio * 14, 14, 1)
            drawPanelFillRect(1,  17, ratio*14, 14, color565((1-ratio)*255,ratio * 255,0))
            drawPanelFillRect(65+16, 17, ratio*14, 14, color565((1-ratio)*255,ratio * 255,0))
        end

        if _M.isBooped then  
            oledSetFontSize(2)
            oledDrawText("BOOP!")
            DrawText(17, 23, "BOOP!", color565(255,255,255), color565(0,0,0))
            DrawText(64+17+16, 23, "BOOP!", color565(255,255,255), color565(0,0,0))
        else
            oledSetFontSize(2)
            oledDrawText("----")
            DrawText(17, 23, "-----", color565(255,255,255), color565(0,0,0))
            DrawText(64+17+16, 23, "-----", color565(255,255,255), color565(0,0,0))
        end
        oledSetFontSize(1)
        oledSetCursor(0, 48) 
        oledDrawText("Timer: ".._M.boopTimerDuration.." ms /\\+ \\/-")

        oledSetCursor(32, 64-8)
        oledDrawText("[Confirm] Next")

        flipPanelBuffer()
    elseif _M.calibration_stage == 8 then
        clearPanelBuffer()
        DrawSprite(32-8/2,16-8/2, 8, 8, _M.gear_sprite)
        DrawSprite(64+32-8/2,16-8/2, 8, 8, _M.gear_sprite)
        flipPanelBuffer()
        oledSetCursor(0, 0)
        oledSetFontSize(2)       
        oledDrawText("Completed.")
        oledSetFontSize(1)
        oledSetCursor(32, 64-8)
        oledDrawText("[Confirm] Exit")
                
    end
    oledDisplay()
end

function _M.drawLidarBar(reading, ok)
    local rd = Map(reading, 0, _M.maxLidarProcessing, 1, 62)
    local rd2 = Map(reading, 0, _M.maxLidarProcessing, 1, 126)
    drawPanelRect(0, 0, 63, 5, color565(255,255,255))
    drawPanelRect(64, 0, 63, 5, color565(255,255,255))
    oledDrawRect(0,0, 127, 10, 1)
    oledDrawFilledRect(1,1, rd2, 8, 1)
    local booped = false
    if ok then
        if _M.triggerPosition > 0 then
            if reading > _M.minimumLidar and reading < _M.triggerPosition then
                drawPanelFillRect(65,  1, rd, 3, color565(0,255,0))
                drawPanelFillRect(1,   1, rd, 3, color565(0,255,0))
                booped = true
            else
                drawPanelFillRect(65,  1, rd, 3, color565(200,255,0))
                drawPanelFillRect(1,   1, rd, 3, color565(200,255,0))
            end
        else 
            drawPanelFillRect(65,  1, rd, 3, color565(255,255,255))
            drawPanelFillRect(1,   1, rd, 3, color565(255,255,255))
        end
    else
        drawPanelFillRect(65,  1, 62, 3, color565(255,0,0))
        drawPanelFillRect(1,   1, 62, 3, color565(255,0,0))
    end
    if _M.triggerPosition > 0 then 
        rd = Map(_M.triggerPosition, 0, _M.maxLidarProcessing, 1, 62)
        rd2 = Map(_M.triggerPosition, 0, _M.maxLidarProcessing, 1, 126)
        oledDrawLine(rd2, 0, rd2, 10, 0)
        oledDrawLine(rd2, 10, rd2, 20, 1)
        drawPanelLine(rd, 0, rd, 8, color565(0,0,255))
        drawPanelLine(64+rd, 0, 64+rd, 8, color565(0,0,255))

        rd = Map(_M.minimumLidar, 0, _M.maxLidarProcessing, 1, 62)
        rd2 = Map(_M.minimumLidar, 0, _M.maxLidarProcessing, 1, 126)
        oledDrawLine(rd2, 0, rd2, 10, 0)
        oledDrawLine(rd2, 10, rd2, 20, 1)
        drawPanelLine(rd, 0, rd, 8, color565(255,0,0))
        drawPanelLine(64+rd, 0, 64+rd, 8, color565(255,0,0))
    end
    return booped
end

function _M.readLidar()
    local readingRaw = readLidar()
    local reading = readingRaw
    local ok = true
    if reading >= 8190 then
        out_of_range_color = color565(255,0,0)
        ok = false
    elseif reading > _M.maxLidarProcessing then 
        reading = _M.maxLidarProcessing
    end
    if reading <= 0 then 
        reading = 0
    end
    if ok then
        _M.avgRead = _M.avgRead * 0.8 + reading * 0.2
    end
    return _M.avgRead, ok, readingRaw
end

function _M.isBoopedCheck(dt)
    if _M.mode == "lidar" then 
        if hasLidar() then

            local reading, ok = _M.readLidar()

            if ok then 
                if reading > _M.minimumLidar and reading < _M.triggerPosition then
                    _M.boopTimer = _M.boopTimer+dt  
                else 
                    _M.boopTimer = 0
                end
                if _M.boopTimer >= _M.boopTimerDuration then 
                    _M.boopTimer = _M.boopTimerDuration
                    _M.isBooped = true
                    return true
                end
            else 
                if _M.isBooped then 
                    return true
                end
                _M.boopTimer = 0
            end
            _M.isBooped = false
        end
        return false
    elseif _M.mode == "gpio" then 
        if not _M.isBooped then
            if digitalRead(_M.gpio) == _M.gpio_state then  
                _M.boopTimer = _M.boopTimer+dt  
            else
                _M.boopTimer = 0
            end
            if _M.boopTimer >= _M.boopTimerDuration then 
                _M.boopTimer = 0
                _M.isBooped = true
                return true
            end
        else 
            if digitalRead(_M.gpio) ~= _M.gpio_state then  
                _M.boopTimer = _M.boopTimer+dt  
            else
                _M.boopTimer = 0
            end
            if _M.boopTimer >= _M.boopTimerDuration then 
                _M.boopTimer = 0
                _M.isBooped = false
                return false
            end
            return true
        end
    end
    return false
end

function _M.reset()
    _M.boopIsOn = false
end

function _M.manageBoop(dt)

    local config = _M.config
    if not _M.configured and not config.enabled  then
        return
    end
    local isBooped = _M.isBoopedCheck(dt)

    if isBooped then 
        if not _M.boopIsOn then
            local isOnCorrectFrame = true
            if config.transictionOnlyOnAnimation then  
                if not IsFrameFromAnimation(getPanelCurrentFace(), config.transictionOnlyOnAnimation) then  
                    isOnCorrectFrame = false
                end
            end
            if config.transictionInOnlyOnSpecificFrame then 
                if config.transictionInOnlyOnSpecificFrame ~= GetCurrentFrame() then  
                    isOnCorrectFrame = false
                end
            end
            if isOnCorrectFrame then
                _M.boopIsOn = true
                if config.transitionOut then
                    StackExpression(config.transitionOut)
                end
                StackExpression(config.boopAnimationName)
                if config.transitionIn then
                    StackExpression(config.transitionIn)
                end
            end
        end
    else
        if IsFrameFromAnimation(getPanelCurrentFace(), config.boopAnimationName)  then  
            if config.transictionOutOnlyOnSpecificFrame then 
                if config.transictionOutOnlyOnSpecificFrame ~= GetCurrentFrame() then  
                    return
                end
            end
            local menu = require("menu")
            if menu.menuExpression == config.boopAnimationName then
                --Avoid dropping the animation if it was a user selected one
                return
            end
            _M.boopIsOn = false   
            popPanelAnimation()       
        end
    end
    
end

return _M