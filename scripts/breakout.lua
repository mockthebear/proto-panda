local _M = {
    VERSION_REQUIRED="1.0.0",
    shouldStop = false,
    X_OFFSET=24,
    GAME_WIDTH = 31,
    GAME_HEIGHT = 31,
    RACKET_WIDTH = 6,
    RACKET_HEIGHT = 1,
    RACKET_SPEED = 30,
    BALL_SIZE = 2,
    BALL_SPEED = 40,
    gameStarted = false,
    BRICK_WIDTH = 5,
    BRICK_HEIGHT = 2,
    BRICK_ROWS = 4,
    BRICK_COLS = 5,
}
local input = require("input")

_M.brickColors = {
    color565(255, 0, 0), 
    color565(0, 255, 0),
    color565(0, 0, 255), 
    color565(255, 255, 0),
    color565(255, 0, 255), 
    color565(0, 255, 255)
}

function _M.initGame()
    _M.racketX = math.floor((_M.GAME_WIDTH - _M.RACKET_WIDTH) / 2)
    _M.racketY = _M.GAME_HEIGHT - 2
    _M.ballX = math.floor(_M.GAME_WIDTH / 2)
    _M.ballY = math.floor(_M.GAME_HEIGHT / 2)
    _M.ballDX = 0
    _M.ballDY = 0
    _M.score = 0
    _M.gameStarted = false

    _M.bricks = {}
    for row = 1, _M.BRICK_ROWS do
        _M.bricks[row] = {}
        for col = 1, _M.BRICK_COLS do
            _M.bricks[row][col] = {
                x = (col-1) * (_M.BRICK_WIDTH + 1) + 1,
                y = row * (_M.BRICK_HEIGHT + 1) + 1,
                width = _M.BRICK_WIDTH,
                height = _M.BRICK_HEIGHT,
                active = true,
                color = _M.brickColors[(row + col) % #_M.brickColors + 1]
            }
        end
    end
end

function _M.checkCollision(ballX, ballY, rectX, rectY, rectW, rectH)
    return ballX < rectX + rectW and
           ballX + _M.BALL_SIZE > rectX and
           ballY < rectY + rectH and
           ballY + _M.BALL_SIZE > rectY
end

function _M.handleBrickCollisions()
    for row = 1, _M.BRICK_ROWS do
        for col = 1, _M.BRICK_COLS do
            local brick = _M.bricks[row][col]
            if brick.active and _M.checkCollision(_M.ballX, _M.ballY, brick.x, brick.y, brick.width, brick.height) then
                local ballCenterX = _M.ballX + _M.BALL_SIZE/2
                local ballCenterY = _M.ballY + _M.BALL_SIZE/2
                local brickCenterX = brick.x + brick.width/2
                local brickCenterY = brick.y + brick.height/2
           
                local overlapX = (brick.width/2 + _M.BALL_SIZE/2) - math.abs(ballCenterX - brickCenterX)
                local overlapY = (brick.height/2 + _M.BALL_SIZE/2) - math.abs(ballCenterY - brickCenterY)
                
                if overlapX < overlapY then
                    _M.ballDX = -_M.ballDX
                else
                    _M.ballDY = -_M.ballDY
                end
                
                brick.active = false

                local speed = math.sqrt(_M.ballDX^2 + _M.ballDY^2)
                _M.ballDX = (_M.ballDX / speed) * _M.BALL_SPEED
                _M.ballDY = (_M.ballDY / speed) * _M.BALL_SPEED
                return true
            end
        end
    end
    return false
end

function _M.updateBall(dt)
	dt = dt/1000

	if input.readButtonStatus(BUTTON_RIGHT) == BUTTON_PRESSED then
        _M.racketX = math.min(_M.racketX + _M.RACKET_SPEED * dt, _M.GAME_WIDTH - _M.RACKET_WIDTH)
    end

    if input.readButtonStatus(BUTTON_LEFT) == BUTTON_PRESSED then
        _M.racketX = math.max(_M.racketX - _M.RACKET_SPEED* dt, 0)
    end

    if not _M.gameStarted and input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_JUST_PRESSED then
        _M.gameStarted = true
        local angle = math.random() * math.pi / 2 - math.pi / 4
        _M.ballDX = _M.BALL_SPEED * math.cos(angle)
        _M.ballDY = -_M.BALL_SPEED * math.sin(angle)
    end

    if not _M.gameStarted then

        _M.ballX = _M.racketX + math.floor(_M.RACKET_WIDTH / 2)
        _M.ballY = _M.racketY - _M.BALL_SIZE
        return
    end
  	

 

    _M.ballX = _M.ballX + _M.ballDX * dt
    _M.ballY = _M.ballY + _M.ballDY * dt

    if _M.ballX <= 0 then
        _M.ballX = 0
        _M.ballDX = -_M.ballDX
    elseif _M.ballX + _M.BALL_SIZE >= _M.GAME_WIDTH then
        _M.ballX = _M.GAME_WIDTH - _M.BALL_SIZE
        _M.ballDX = -_M.ballDX
    end

    if _M.ballY <= 0 then
        _M.ballY = 0
        _M.ballDY = -_M.ballDY
    end

    _M.handleBrickCollisions()

    if _M.ballY + _M.BALL_SIZE >= _M.racketY and
       _M.ballY < _M.racketY + _M.RACKET_HEIGHT and
       _M.ballX + _M.BALL_SIZE > _M.racketX and
       _M.ballX < _M.racketX + _M.RACKET_WIDTH then
        
        -- Calculate where the ball hit the racket (from -1 to 1)
        local hitPos = ((_M.ballX + _M.BALL_SIZE/2) - (_M.racketX + _M.RACKET_WIDTH/2)) / (_M.RACKET_WIDTH/2)
        
        -- Adjust angle based on hit position
        local angle = math.pi / 3 * hitPos -- 60 degrees max angle
        _M.ballDX = _M.BALL_SPEED * math.sin(angle)
        _M.ballDY = -_M.BALL_SPEED * math.cos(angle) -- Always bounce up
                
        -- Ensure ball doesn't get stuck in racket
        _M.ballY = _M.racketY - _M.BALL_SIZE
    end



    -- Bottom collision (lose life)
    if _M.ballY + _M.BALL_SIZE >= _M.GAME_HEIGHT then
        _M.initGame()
    end
end

function _M.drawText(x,y, str, color, bg)
	bg = bg or 0
	for i=1,#str do  
		drawPanelChar(x + (i-1)*6, y, string.byte(str:sub(i,i)), color, bg, 1)
	end
end


-- Draw the game
function _M.drawGame()

    drawPanelRect(_M.X_OFFSET-1, 0, _M.GAME_WIDTH+2, _M.GAME_HEIGHT, color565(100, 100, 100))
    drawPanelRect(128-(_M.X_OFFSET-1+_M.GAME_WIDTH+2), 0, _M.GAME_WIDTH+2, _M.GAME_HEIGHT, color565(100, 100, 100))
    

    drawPanelRect(_M.X_OFFSET+_M.racketX, _M.racketY-1, _M.RACKET_WIDTH, _M.RACKET_HEIGHT, color565(255, 255, 255))
    drawPanelRect(128-(_M.X_OFFSET+_M.racketX+_M.RACKET_WIDTH), _M.racketY-1, _M.RACKET_WIDTH, _M.RACKET_HEIGHT, color565(255, 255, 255))

    for row = 1, _M.BRICK_ROWS do
        for col = 1, _M.BRICK_COLS do
            local brick = _M.bricks[row][col]
            if brick.active then
                drawPanelRect(_M.X_OFFSET+brick.x, brick.y, brick.width, brick.height, brick.color)
                drawPanelRect(128-(_M.X_OFFSET+brick.x+brick.width), brick.y, brick.width, brick.height, brick.color)
            end
        end
    end
    
    drawPanelRect(_M.X_OFFSET+_M.ballX, _M.ballY, _M.BALL_SIZE, _M.BALL_SIZE, color565(255, 0, 0))
    drawPanelRect(128-(_M.X_OFFSET+_M.ballX+_M.BALL_SIZE), _M.ballY, _M.BALL_SIZE, _M.BALL_SIZE, color565(255, 0, 0))

    if not _M.gameStarted then
        _M.drawText(_M.X_OFFSET+4, 15, "Start", color565(255, 255, 255))
        _M.drawText(55+(_M.X_OFFSET+4), 15, "Start", color565(255, 255, 255))
    end
end

function _M.onSetup()
    setPanelManaged(false)
    _M.initGame()
end

function _M.onLoop(dt)
    clearPanelBuffer()
    
    _M.updateBall(dt)
    _M.drawGame()
    
    flipPanelBuffer()
    
    if input.readButtonStatus(BUTTON_BACK) == BUTTON_JUST_PRESSED or (input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_LEFT) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_RIGHT) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_UP) == BUTTON_PRESSED and input.readButtonStatus(BUTTON_DOWN) == BUTTON_PRESSED) then 
    
        _M.shouldStop = true
        return true
    end
end

function _M.onClose()
    collectgarbage()
end

return _M