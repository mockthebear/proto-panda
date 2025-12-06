local _M = {
	VERSION_REQUIRED="1.0.0",
	shouldStop = false,
	snake = {}
}
local input = require("input")

local RED = color565(255, 0, 0)
local GREEN = color565(0, 255, 0)
local BLUE = color565(0, 0, 255)
local WHITE = color565(255, 255, 255)

local UP = 0
local RIGHT = 1
local DOWN = 2  
local LEFT = 3

local speed = 18

function _M.onSetup()
	setPanelManaged(false)
	delay(500)
	_M.restart()
end

function _M.restart()
	_M.snake = {
		x = 32,
		y = 16,
		size = 5,
		segments = {},
		direction = RIGHT,
		foodx = math.random(0, 63),
		foody = math.random(0, 31),
	}
end

local function moveVector(dir) 
	if dir == UP then 
		return 0, -speed
	elseif dir == RIGHT then 
		return speed, 0
	elseif dir == DOWN then 
		return 0, speed
	elseif dir == LEFT then 
		return -speed, 0
	else 
		error("Invalid direction")
	end
end

local function hslToRgb(h, s, l)
    h = h / 360
    local r, g, b

    if s == 0 then
        r, g, b = l, l, l -- achromatic
    else
        local function hueToRgb(p, q, t)
            if t < 0 then t = t + 1 end
            if t > 1 then t = t - 1 end
            if t < 1/6 then return p + (q - p) * 6 * t end
            if t < 1/2 then return q end
            if t < 2/3 then return p + (q - p) * (2/3 - t) * 6 end
            return p
        end

        local q = l < 0.5 and l * (1 + s) or l + s - l * s
        local p = 2 * l - q
        r = hueToRgb(p, q, h + 1/3)
        g = hueToRgb(p, q, h)
        b = hueToRgb(p, q, h - 1/3)
    end

    return math.floor(r * 255), math.floor(g * 255), math.floor(b * 255)
end


local function positionToRainbowColor(position, total_length)
    position = math.max(0, math.min(position, total_length))


    local hue = (position / total_length) * 360

    local saturation = 1.0
    local lightness = 0.5

    local r, g, b = hslToRgb(hue, saturation, lightness)

    return color565(r, g, b)
end

function _M.onLoop(dt)
	clearPanelBuffer()

	drawPanelRect(0,0, 64, 32, color565(100, 100, 100))
	drawPanelRect(64,0, 64, 32, color565(100, 100, 100))


	dt = dt/1000;

	local snake = _M.snake

	if input.readButtonStatus(BUTTON_LEFT) == BUTTON_JUST_PRESSED then 
		if snake.direction ~= RIGHT then
			snake.direction = LEFT
		end
	end
	if input.readButtonStatus(BUTTON_RIGHT) == BUTTON_JUST_PRESSED then 
		if snake.direction ~= LEFT then
			snake.direction = RIGHT
		end
	end
	if input.readButtonStatus(BUTTON_UP) == BUTTON_JUST_PRESSED then 
		if snake.direction ~= DOWN then
			snake.direction = UP
		end
	end
	if input.readButtonStatus(BUTTON_DOWN) == BUTTON_JUST_PRESSED then 
		if snake.direction ~= UP then
			snake.direction = DOWN
		end
	end

	local x,y = moveVector(snake.direction) 
	local prevX = math.floor(snake.x)
	local prevY = math.floor(snake.y)


	snake.x = snake.x + (x * dt)
	snake.y = snake.y + (y * dt)

	if (snake.x < 1) then
		snake.x = 62
	elseif (snake.x >= 63) then
		snake.x = 1
	end

	if (snake.y < 1) then
		snake.y = 30
	elseif (snake.y >= 31) then
		snake.y = 1
	end

	local floorX = math.floor(snake.x)
	local floorY = math.floor(snake.y)

	drawPanelPixel(floorX, floorY, WHITE)
	drawPanelPixel(64+floorX, floorY, WHITE)
	drawPanelPixel(snake.foodx, snake.foody, WHITE)
	drawPanelPixel(64+snake.foodx, snake.foody, WHITE)

	if snake.foodx == floorX and snake.foody == floorY then 
		snake.foodx = math.random(2, 60)
		snake.foody = math.random(2, 30)
		snake.size = snake.size + 4
		toneDuration(440, 200)
	end

	local hasMoved = floorX ~= prevX or floorY ~= prevY 
	if hasMoved then
		if snake.size > #snake.segments then 
			snake.segments[#snake.segments+1] = {prevX, prevY}
		end
	end
	
	local seg = snake.segments
	for i=#snake.segments, 1, -1  do
		if hasMoved then
			if i == 1 then  
				seg[i][1] = prevX
				seg[i][2] = prevY
			else
				seg[i][1] = seg[i-1][1]
				seg[i][2] = seg[i-1][2]
				if floorX == seg[i][1] and floorY == seg[i][2] then 
					_M.restart()
					return
				end
			end
		end
		local color = positionToRainbowColor(i, #snake.segments)
		drawPanelPixel(seg[i][1], seg[i][2], color)
		drawPanelPixel(64+seg[i][1], seg[i][2], color)
	end

	flipPanelBuffer()
	if input.readButtonStatus(BUTTON_BACK) == BUTTON_JUST_PRESSED or (input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_LEFT) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_RIGHT) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_UP) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_DOWN) == BUTTON_PRESSED) then 
		_M.shouldStop = true
		return true
	end
end 

function _M.onClose()
end

return _M