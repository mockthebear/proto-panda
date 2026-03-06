local beautyr1 = {
    
    type="hid",

    match = {
        byname='beauty-r1'
    },

    mode = {
        'generic'
    },

}

local prevPacket = nil
local secondPrev = nil
function beautyr1.processPackets(connectionId, controllerId, data)
    if data[1] == 156 and data[2] == 64 and data[3] == 236 then  
        local beauty = drivers.generic[controllerId]
        beauty.timeout = millis()+250
        beauty.buttons[1] = 1
    elseif data[1] == 2 and data[2] == 48 and data[3] == 0 then  
        local beauty = drivers.generic[controllerId]
        beauty.timeout = millis()+250
        beauty.buttons[2] = 1
    elseif data[1] == 255 and data[2] == 23 and data[3] == 128 then  
        local beauty = drivers.generic[controllerId]
        beauty.timeout = millis()+250
        beauty.buttons[3] = 1
    elseif data[1] == 56 and data[2] == 128 and data[3] == 17 then  
        local beauty = drivers.generic[controllerId]
        beauty.timeout = millis()+250
        beauty.buttons[4] = 1
    elseif data[1] == 1 and data[2] == 0 and data[3] == 0 then
        if prevPacket and prevPacket[1] == 0 and prevPacket[2] == 0 and prevPacket[3] == 0 then
            if secondPrev and secondPrev[1] == 0 and secondPrev[2] == 0 and secondPrev[3] == 0 then
                local beauty = drivers.generic[controllerId]
                beauty.timeout = millis()+250
                beauty.buttons[5] = 1
            end
        end
    elseif (data[1] == 234 or data[1] == 233) and data[2] == 0 and data[3] == nil then
        local beauty = drivers.generic[controllerId]
        beauty.timeout = millis()+250
        beauty.buttons[6] = 1
    end

    secondPrev = prevPacket
    prevPacket = data
end

function beautyr1.onUpdate(drivers, i)
    if drivers.generic[i].timeout and drivers.generic[i].timeout < millis() then  
        drivers.generic[i].timeout = nil
        local buttons = drivers.generic[i].buttons
        for a,c in pairs(buttons) do
            buttons[a] = 0
        end
    end
end

return beautyr1