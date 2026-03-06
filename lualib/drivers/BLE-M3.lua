local blem3 = {
    
    type="hid",

    match = {
        byname='ble-m3'
    },

    mode = {
        'generic'
    },

}

function blem3.processPackets(connectionId, controllerId, data)
    if data[1] == 7 and data[2] == 184 and data[3] == 2 then  
        if prevPacket[2] == 240 and prevPacket[3] == 1 then  
            --LEFT
            local beauty = drivers.generic[controllerId]
            beauty.timeout = millis()+250
            beauty.buttons[4] = 1
        end
    elseif data[1] == 7 and data[2] == 240 and data[3] == 1  then
        if prevPacket[1] == 7 and prevPacket[2] == 184 and prevPacket[3] == 2 then  
            local beauty = drivers.generic[controllerId]
            beauty.timeout = millis()+250
            beauty.buttons[3] = 1
        end       
    elseif data[1] == 7 and data[2] == 244 and data[3] == 3 and data[4] == 116 then
        if prevPacket[4] == 160 then  
            --down
            local beauty = drivers.generic[controllerId]
            beauty.timeout = millis()+250
            beauty.buttons[1] = 1
        end 
        
    elseif data[1] == 7 and data[4] == 72 and data[5] == 5 then
        if prevPacket[5] == 4 then  
            --up
            local beauty = drivers.generic[controllerId]
            beauty.timeout = millis()+250
            beauty.buttons[2] = 1
        end
 
    elseif data[1] == 4 and data[2] == 244 and data[3] == 1 and data[4] == 204 then
        --Confirm   
        local beauty = drivers.generic[controllerId]
        beauty.timeout = millis()+250
        beauty.buttons[5] = 1   

    elseif data[1] == 7 and data[2] == 0 and data[3] == 0 then
        --back
        local beauty = drivers.generic[controllerId]
        beauty.timeout = millis()+250
        beauty.buttons[6] = 1
    end

    secondPrev = prevPacket
    prevPacket = data
end

function blem3.onUpdate(drivers, i)
    if drivers.generic[i].timeout and drivers.generic[i].timeout < millis() then  
        drivers.generic[i].timeout = nil
        local buttons = drivers.generic[i].buttons
        for a,c in pairs(buttons) do
            buttons[a] = 0
        end
    end
end

--[[
\/
98, 244, 3, 36, 11, 
7, 244, 3, 251, 9, 
7, 244, 3, 204, 8, 
7, 244, 3, 160, 7, 
, 
7, 244, 3, 72, 5, 
7, 244, 3, 28, 4, 
7, 244, 3, 240, 2, 
0, 208, 249, 80, 12







/\
7, 244, 3, 240, 2, 
7, 244, 3, 28, 4, 
7, 244, 3, 72, 5, 
, 
7, 244, 3, 160, 7, 
7, 244, 3, 204, 8, 
7, 244, 3, 251, 9, 
 
0, 208, 249, 80, 12, 




>>


7, 128, 3, 32, 9, 
7, 184, 2, 32, 9, 
7, 240, 1, 32, 9, 
7, 40, 1, 32, 9, 
7, 96, 0, 32, 9, 
0, 96, 0, 32, 9, 



<<
7, 240, 1, 32, 9, 
7, 184, 2, 32, 9, 
7, 128, 3, 32, 9, 


7, 216, 5, 32, 9,  -
7, 160, 6, 32, 9,  -
7, 104, 7, 32, 9,  -
7, 48, 8, 32, 9,  -
0, 48, 8, 32, 9,  -


center
, 
4, 244, 1, 204, 1, 




photo
6, 0, 0, 0, 0, 
7, 0, 0, 0, 0, 
Keyboard press:         2       0
Keyboard press:         0       0
6, 0, 0, 0, 0,  

]]

return blem3