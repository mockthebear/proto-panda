local _M = {

	sendStack = {
		start_byte = 0;
		version = 0;
		length = 0;
		commandValue = 0;
		feedbackValue = 0;
		paramMSB = 0;
		paramLSB = 0;
		checksumMSB = 0;
		checksumLSB = 0;
		end_byte = 0;
	},

	recStack = {
		start_byte = 0;
		version = 0;
		length = 0;
		commandValue = 0;
		feedbackValue = 0;
		paramMSB = 0;
		paramLSB = 0;
		checksumMSB = 0;
		checksumLSB = 0;
		end_byte = 0;
	}
}


local __dfplayer_STACK_SIZE      = 10;   
local __dfplayer_SB              = 0x7E;
local __dfplayer_VER             = 0xFF; 
local __dfplayer_LEN             = 0x6;  
local __dfplayer_FEEDBACK        = 1;  
local __dfplayer_NO_FEEDBACK     = 0; 
local __dfplayer_EB              = 0xEF;

local __dfplayer_NEXT            = 0x01;
local __dfplayer_PREV            = 0x02;
local __dfplayer_PLAY            = 0x03;
local __dfplayer_INC_VOL         = 0x04;
local __dfplayer_DEC_VOL         = 0x05;
local __dfplayer_VOLUME          = 0x06;
local __dfplayer_EQ              = 0x07;
local __dfplayer_PLAYBACK_MODE   = 0x08;
local __dfplayer_PLAYBACK_SRC    = 0x09;
local __dfplayer_STANDBY         = 0x0A;
local __dfplayer_NORMAL          = 0x0B;
local __dfplayer_RESET           = 0x0C;
local __dfplayer_PLAYBACK        = 0x0D;
local __dfplayer_PAUSE           = 0x0E;
local __dfplayer_SPEC_FOLDER     = 0x0F;
local __dfplayer_VOL_ADJ         = 0x10;
local __dfplayer_REPEAT_PLAY     = 0x11;
local __dfplayer_USE_MP3_FOLDER  = 0x12;
local __dfplayer_INSERT_ADVERT   = 0x13;
local __dfplayer_SPEC_TRACK_3000 = 0x14;
local __dfplayer_STOP_ADVERT     = 0x15;
local __dfplayer_STOP            = 0x16;
local __dfplayer_REPEAT_FOLDER   = 0x17;
local __dfplayer_RANDOM_ALL      = 0x18;
local __dfplayer_REPEAT_CURRENT  = 0x19;
local __dfplayer_SET_DAC         = 0x1A;


local __dfplayer_SEND_INIT        = 0x3F;
local __dfplayer_RETRANSMIT       = 0x40;
local __dfplayer_REPLY            = 0x41;
local __dfplayer_GET_STATUS_      = 0x42;
local __dfplayer_GET_VOL          = 0x43;
local __dfplayer_GET_EQ           = 0x44;
local __dfplayer_GET_MODE         = 0x45;
local __dfplayer_GET_VERSION      = 0x46;
local __dfplayer_GET_TF_FILES     = 0x47;
local __dfplayer_GET_U_FILES      = 0x48;
local __dfplayer_GET_FLASH_FILES  = 0x49;
local __dfplayer_KEEP_ON          = 0x4A;
local __dfplayer_GET_TF_TRACK     = 0x4B;
local __dfplayer_GET_U_TRACK      = 0x4C;
local __dfplayer_GET_FLASH_TRACK  = 0x4D;
local __dfplayer_GET_FOLDER_FILES = 0x4E;
local __dfplayer_GET_FOLDERS      = 0x4F;

local __dfplayer_EQ_NORMAL       = 0;
local __dfplayer_EQ_POP          = 1;
local __dfplayer_EQ_ROCK         = 2;
local __dfplayer_EQ_JAZZ         = 3;
local __dfplayer_EQ_CLASSIC      = 4;
local __dfplayer_EQ_BASE         = 5;

local __dfplayer_REPEAT          = 0;
local __dfplayer_FOLDER_REPEAT   = 1;
local __dfplayer_SINGLE_REPEAT   = 2;
local __dfplayer_RANDOM          = 3;

local __dfplayer_U               = 1;
local __dfplayer_TF              = 2;
local __dfplayer_AUX             = 3;
local __dfplayer_SLEEP           = 4;
local __dfplayer_FLASH           = 5;

local __dfplayer_VOL_ADJUST      = 0x10;

local __dfplayer_STOP_REPEAT     = 0;
local __dfplayer_START_REPEAT    = 1;

function _M.begin()
	beginSerialIo(9600)
	_M.threshold = 100;
	_M.sendStack.start_byte = __dfplayer_SB;
	_M.sendStack.version    = __dfplayer_VER;
	_M.sendStack.length     = __dfplayer_LEN;
	_M.sendStack.end_byte   = __dfplayer_EB;

	_M.recStack.start_byte  = __dfplayer_SB;
	_M.recStack.version     = __dfplayer_VER;
	_M.recStack.length      = __dfplayer_LEN;
	_M.recStack.end_byte    = __dfplayer_EB;
end


function _M.findChecksum(stack)
    local sum = stack.version + stack.length + stack.commandValue + stack.feedbackValue + stack.paramMSB + stack.paramLSB
    local checksum = (~sum) + 1
    stack.checksumMSB = (checksum >> 8) & 0xFF
    stack.checksumLSB = checksum & 0xFF
end

function _M.loop(trackNum)
  _M.sendStack.commandValue  = __dfplayer_PLAYBACK_MODE
  _M.sendStack.feedbackValue = __dfplayer_NO_FEEDBACK
  _M.sendStack.paramMSB = (trackNum >> 8) & 0xFF
  _M.sendStack.paramLSB = trackNum & 0xFF
  
  _M.findChecksum(_M.sendStack)
  _M.sendData()
end


function _M.flush()
	while (serialIoAvaliable() > 0) do 
		serialIoRead()
	end
end

function _M.currentVolume()
	return _M.query(__dfplayer_GET_VOL);
end

function _M.reset()
	_M.sendStack.commandValue  = __dfplayer_RESET;
	_M.sendStack.feedbackValue = __dfplayer_NO_FEEDBACK;
	_M.sendStack.paramMSB = 0;
	_M.sendStack.paramLSB = 1;

	_M.findChecksum(_M.sendStack);
	_M.sendData();
end

function _M.playbackSource(source)
	if ((source > 0) and (source <= 5)) then
		_M.sendStack.commandValue  = __dfplayer_PLAYBACK_SRC;
		_M.sendStack.feedbackValue = __dfplayer_NO_FEEDBACK;
		_M.sendStack.paramMSB = 0;
		_M.sendStack.paramLSB = source;

		_M.findChecksum(_M.sendStack);
		_M.sendData();
	end
end

function _M.EQSelect(setting)

	if (setting <= 5) then
		_M.sendStack.commandValue  = __dfplayer_EQ;
		_M.sendStack.feedbackValue = __dfplayer_NO_FEEDBACK;
		_M.sendStack.paramMSB = 0;
		_M.sendStack.paramLSB = setting;

		_M.findChecksum(_M.sendStack);
		_M.sendData();
	end
end




function _M.stop()
	_M.sendStack.commandValue = __dfplayer_STOP;
	_M.sendStack.feedbackValue = __dfplayer_NO_FEEDBACK;
	_M.sendStack.paramMSB = 0;
	_M.sendStack.paramLSB = 0;

	_M.findChecksum(_M.sendStack);
	_M.sendData();
end

function _M.play(trackNum)
	_M.sendStack.commandValue  = __dfplayer_PLAY;
	_M.sendStack.feedbackValue = __dfplayer_NO_FEEDBACK;
	_M.sendStack.paramMSB = (trackNum >> 8) & 0xFF;
	_M.sendStack.paramLSB = trackNum & 0xFF;

	_M.findChecksum(_M.sendStack);
	_M.sendData();
end

function _M.playFromMP3Folder(trackNum)
	_M.sendStack.commandValue  = __dfplayer_USE_MP3_FOLDER;
	_M.sendStack.feedbackValue = __dfplayer_NO_FEEDBACK;
	_M.sendStack.paramMSB = (trackNum >> 8) & 0xFF;
	_M.sendStack.paramLSB = trackNum & 0xFF;

	_M.findChecksum(_M.sendStack);
	_M.sendData();
end

function _M.standbyMode()
	_M.sendStack.commandValue  = __dfplayer_STANDBY;
	_M.sendStack.feedbackValue = __dfplayer_NO_FEEDBACK;
	_M.sendStack.paramMSB = 0;
	_M.sendStack.paramLSB = 1;

	_M.findChecksum(_M.sendStack);
	_M.sendData();
end

function _M.normalMode()
	_M.sendStack.commandValue  = __dfplayer_NORMAL;
	_M.sendStack.feedbackValue = __dfplayer_NO_FEEDBACK;
	_M.sendStack.paramMSB = 0;
	_M.sendStack.paramLSB = 1;

	_M.findChecksum(_M.sendStack);
	_M.sendData();
end

function _M.resume()
	_M.sendStack.commandValue  = __dfplayer_PLAYBACK;
	_M.sendStack.feedbackValue = __dfplayer_NO_FEEDBACK;
	_M.sendStack.paramMSB = 0;
	_M.sendStack.paramLSB = 1;

	_M.findChecksum(_M.sendStack);
	_M.sendData();
end

function _M.pause()
	_M.sendStack.commandValue  = __dfplayer_PAUSE;
	_M.sendStack.feedbackValue = __dfplayer_NO_FEEDBACK;
	_M.sendStack.paramMSB = 0;
	_M.sendStack.paramLSB = 1;

	_M.findChecksum(_M.sendStack);
	_M.sendData();
end

function _M.volumeAdjustSet(gain)
	if (gain <= 31) then
		_M.sendStack.commandValue  = __dfplayer_VOL_ADJ;
		_M.sendStack.feedbackValue = __dfplayer_NO_FEEDBACK;
		_M.sendStack.paramMSB = 0;
		_M.sendStack.paramLSB = __dfplayer_VOL_ADJUST + gain;

		_M.findChecksum(_M.sendStack);
		_M.sendData();
	end
end

function _M.randomAll()
	_M.sendStack.commandValue  = __dfplayer_RANDOM_ALL;
	_M.sendStack.feedbackValue = __dfplayer_NO_FEEDBACK;
	_M.sendStack.paramMSB = 0;
	_M.sendStack.paramLSB = 0;

	_M.findChecksum(_M.sendStack);
	_M.sendData();
end

function _M.isPlaying()
	local result = _M.query(__dfplayer_GET_STATUS_);
	if (result ~= -1) then
		return (result & 1);
	end
	return 0;
end

function _M.currentEQ()
	return _M.query(__dfplayer_GET_EQ);
end

function _M.currentMode()
	return _M.query(__dfplayer_GET_MODE);
end

function _M.currentVersion()
	return _M.query(__dfplayer_GET_VERSION);
end

function _M.numSdTracks()
	return _M.query(__dfplayer_GET_U_FILES);
end

function _M.currentTrack()
	return _M.query(__dfplayer_GET_U_TRACK);
end


function _M.sleep()
	_M.playbackSource(__dfplayer_SLEEP);
end

function _M.wakeUp()
	_M.playbackSource(__dfplayer_TF);
end


function _M.volume(volume)
	if (volume <= 30) then
		_M.sendStack.commandValue  = __dfplayer_VOLUME;
		_M.sendStack.feedbackValue = __dfplayer_NO_FEEDBACK;
		_M.sendStack.paramMSB = 0;
		_M.sendStack.paramLSB = volume;

		_M.findChecksum(_M.sendStack);
		_M.sendData();
	end
end


function _M.query(cmd, msb, lsb)
	msb = msb or 0
	lsb = lsb or 0
	_M.flush();

	_M.sendStack.commandValue  = cmd;
	_M.sendStack.feedbackValue = __dfplayer_NO_FEEDBACK;
	_M.sendStack.paramMSB = msb;
	_M.sendStack.paramLSB = lsb;

	_M.findChecksum(_M.sendStack);
	_M.sendData();
	

	vTaskDelay(100)

	if (_M.parseFeedback()) then
		if (_M.recStack.commandValue ~= 0x40) then
			return (_M.recStack.paramMSB << 8) | _M.recStack.paramLSB;
		end
	end
	return -1;
end

function _M.sendData()
	serialIoWrite(_M.sendStack.start_byte);
	serialIoWrite(_M.sendStack.version);
	serialIoWrite(_M.sendStack.length);
	serialIoWrite(_M.sendStack.commandValue);
	serialIoWrite(_M.sendStack.feedbackValue);
	serialIoWrite(_M.sendStack.paramMSB);
	serialIoWrite(_M.sendStack.paramLSB);
	serialIoWrite(_M.sendStack.checksumMSB);
	serialIoWrite(_M.sendStack.checksumLSB);
	serialIoWrite(_M.sendStack.end_byte);
end

function _M.parseFeedback()
    local state = "find_start_byte"
    local timeout = 100
    local start_time = millis()
    
    while true do
        if serialIoAvaliable() > 0 then
            local recChar = serialIoRead()
            
            --serialWriteString("Rec: ")
            --serialWriteString(string.format("%02X", recChar))
            --serialWriteString("State: ")

            if state == "find_start_byte" then
                --serialWriteString("find_start_byte\n")
                
                if recChar == __dfplayer_SB then
                    _M.recStack.start_byte = recChar
                    state = "find_ver_byte"
                end
                
            elseif state == "find_ver_byte" then
                --serialWriteString("find_ver_byte\n")
                
                if recChar ~= __dfplayer_VER then
                    -- Debug: --serialWriteString("ver error\n")
                    state = "find_start_byte"
                    return false
                end
                
                _M.recStack.version = recChar
                state = "find_len_byte"
                
            elseif state == "find_len_byte" then
                --serialWriteString("find_len_byte\n")
                
                if recChar ~= __dfplayer_LEN then
                    -- Debug: --serialWriteString("len error\n")
                    state = "find_start_byte"
                    return false
                end
                
                _M.recStack.length = recChar
                state = "find_command_byte"
                
            elseif state == "find_command_byte" then
                --serialWriteString("find_command_byte\n")
                
                _M.recStack.commandValue = recChar
                state = "find_feedback_byte"
                
            elseif state == "find_feedback_byte" then
                --serialWriteString("find_feedback_byte\n")
                
                _M.recStack.feedbackValue = recChar
                state = "find_param_MSB"
                
            elseif state == "find_param_MSB" then
                --serialWriteString("find_param_MSB\n")
                
                _M.recStack.paramMSB = recChar
                state = "find_param_LSB"
                
            elseif state == "find_param_LSB" then
                --serialWriteString("find_param_LSB\n")
                
                _M.recStack.paramLSB = recChar
                state = "find_checksum_MSB"
                
            elseif state == "find_checksum_MSB" then
                --serialWriteString("find_checksum_MSB\n")
                
                _M.recStack.checksumMSB = recChar
                state = "find_checksum_LSB"
                
            elseif state == "find_checksum_LSB" then
                --serialWriteString("find_checksum_LSB\n")
                
                _M.recStack.checksumLSB = recChar
                
                local recChecksum = (_M.recStack.checksumMSB << 8) | _M.recStack.checksumLSB
                
                -- Save current checksum values before recalculating
                local tempMSB = _M.recStack.checksumMSB
                local tempLSB = _M.recStack.checksumLSB
                
                _M.findChecksum(_M.recStack)
                local calcChecksum = (_M.recStack.checksumMSB << 8) | _M.recStack.checksumLSB
                
                -- Restore the received checksum values
                _M.recStack.checksumMSB = tempMSB
                _M.recStack.checksumLSB = tempLSB
                
                if recChecksum ~= calcChecksum then
                    -- Debug output
                    --serialWriteString("checksum error\n")
                    --serialWriteString("recChecksum: 0x" .. string.format("%04X", recChecksum) .. "\n")
                    --serialWriteString("calcChecksum: 0x" .. string.format("%04X", calcChecksum) .. "\n\n")
                    
                    state = "find_start_byte"
                    return false
                else
                    state = "find_end_byte"
                end
                
            elseif state == "find_end_byte" then
                --serialWriteString("find_end_byte\n")
                
                if recChar ~= __dfplayer_EB then
                     --serialWriteString("eb error\n")
                    state = "find_start_byte"
                    return false
                end
                
                _M.recStack.end_byte = recChar
                state = "find_start_byte"
                return true
            end
        end
        
        -- Check for timeout
        if (millis() - start_time) >= timeout then
            --serialWriteString("timeout error\n")
            state = "find_start_byte"
            return false
        end
        
        -- Small delay to prevent busy waiting
        vTaskDelay(1)
    end
end

return _M