local _M = {
     default_speed=180/500,
     servos = {}
}
_M.STATE_ON_POSITION = 0 
_M.STATE_MOVING = 1
_M.STATE_IDLE = 2

function _M.clampServo(servoId, min, max)
     _M.servos[servoId].hasClamping = true
     _M.servos[servoId].clampMax = max
     _M.servos[servoId].clampMin = min
end

function _M.setup()
     if not hasServo() then  
          return
     end
     _M.started = true
     for i=0,servoCount() do  
          _M.servos[i] = {
               postion = 90,
               targetAngle=90,
               speed = _M.default_speed,
               stepsChange=0,
               moveThreshold = 2,
               timer = 0,
               state = _M.STATE_IDLE,
               hasClamping=false,
               clampMax = 180,
               clampMin = 0,
               enabled=true,
               canPause=true,
               pauseTimer=0,
          }
     end
end

function _M.getServoState(servoId)
     return _M.servos[servoId].state
end

function _M.isServoMovementCompleted(servoId)
     return _M.servos[servoId].state == _M.STATE_ON_POSITION or _M.servos[servoId].state == _M.STATE_IDLE
end

function _M.setServoSpeed(servoId, speed)
     _M.servos[servoId].speed = speed/1000
end

function _M.setServoPosition(servoId, angleTarget)
     if _M.servos[servoId].invert then  
          angleTarget = 180-angleTarget
     end
     if _M.servos[servoId].postion ~= angleTarget then
          _M.servos[servoId].state = _M.STATE_MOVING
     end
     _M.servos[servoId].targetAngle = angleTarget
end

function _M.invert(servoId)
     _M.servos[servoId].invert = not _M.servos[servoId].invert
end

function _M.update(dt)
     if not _M.started then  
          return
     end
     for i=0,servoCount() do  
          local sdata = _M.servos[i]
          if sdata.postion ~= sdata.targetAngle then  
               local diff = math.abs(sdata.postion-sdata.targetAngle)
               local shouldMove = false
               if (sdata.state == _M.STATE_ON_POSITION or sdata.state == _M.STATE_IDLE) and diff > sdata.moveThreshold then  
                    sdata.state = _M.STATE_MOVING
                    shouldMove = true
               elseif sdata.state == _M.STATE_MOVING then 
                    shouldMove = true
               end
               if shouldMove then
                    if not sdata.enabled then 
                         sdata.enabled = true
                         servoResume(i) 
                         sdata.state = _M.STATE_MOVING 
                    end
                    sdata.pauseTimer = 500
                    if diff > 1 then  
                         sdata.state = _M.STATE_MOVING
                         if sdata.postion < sdata.targetAngle then 
                              sdata.postion = sdata.postion + sdata.speed * dt
                         else 
                              sdata.postion = sdata.postion - sdata.speed * dt
                         end
                    else 
                         sdata.postion = sdata.targetAngle

                         sdata.state = _M.STATE_ON_POSITION
                    end
                    local sendPos = sdata.postion
                    if sdata.hasClamping then 
                         sendPos = map(sendPos, 0, 180, sdata.clampMin, sdata.clampMax) 
                    end
                    servoMove(i, sendPos)
               end
          elseif sdata.enabled then
               sdata.pauseTimer = sdata.pauseTimer - dt
               if sdata.canPause and sdata.pauseTimer < 0 then  
                    sdata.enabled = false
                    sdata.state = _M.STATE_IDLE
                    servoPause(i)  
               end
          end
     end
end

return _M