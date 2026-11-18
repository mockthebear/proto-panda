-- DVD logo screensaver using the real DVD wordmark as a PNG mask.
-- Set SPEED anywhere from 0.0 (still) to 1.0 (fast). 0.1 is the default.
local SPEED = 0.1
-- Set to false to disable the bounce sounds.
local SOUND_ENABLED = true

local _M = {
    VERSION_REQUIRED = "3.2.1",
    shouldStop = false,
    x = 21,
    y = 11,
    directionX = 1,
    directionY = 1,
    colorIndex = 1,
    rainbow = false,
    rainbowPhase = 0,
    logo = nil,
    soundSequence = nil,
    soundIndex = 1,
    nextSoundAt = 0,
    wallHits = 0,
    cornerHits = 0,
}

local input = require("input")

-- Each physical face panel is 64x32. Draw the same animation on both.
local PANEL_WIDTH = 64
local PANEL_HEIGHT = 32
local LOGO_WIDTH = 22
local LOGO_HEIGHT = 10
local VERTICAL_SPEED = 0.72
local CORNER_TOLERANCE = 2

-- The buzzer is monophonic, so notes are scheduled rather than stacked.
-- Each item is {frequency Hz, duration ms, silent gap ms}.
local EDGE_BEEP = {
    {540, 38, 0},
}

local CORNER_FANFARE = {
    {659, 58, 26}, -- E5
    {784, 58, 26}, -- G5
    {1047, 115, 0}, -- C6
}

local COLORS = {
    {255,  50,  50},
    {255, 145,  35},
    {250, 230,  45},
    { 50, 235, 105},
    { 45, 180, 255},
    {125,  85, 255},
    {235,  70, 220},
}

local function clamp(value, minimum, maximum)
    return math.max(minimum, math.min(maximum, value))
end

local function hsvToRgb(hue)
    hue = hue % 360
    local sector = math.floor(hue / 60)
    local fraction = hue / 60 - sector
    local q = math.floor(255 * (1 - fraction))
    local t = math.floor(255 * fraction)

    if sector == 0 then return 255, t, 0 end
    if sector == 1 then return q, 255, 0 end
    if sector == 2 then return 0, 255, t end
    if sector == 3 then return 0, q, 255 end
    if sector == 4 then return t, 0, 255 end
    return 255, 0, q
end

local function pixelColor(localX, localY)
    if not _M.rainbow then
        local color = COLORS[_M.colorIndex]
        return color565(color[1], color[2], color[3])
    end

    local hue = _M.rainbowPhase + localX * 7 + localY * 4
    local red, green, blue = hsvToRgb(hue)
    return color565(red, green, blue)
end

local function drawLogo(panelOffset)
    local pixel = 1
    for y = 0, LOGO_HEIGHT - 1 do
        for x = 0, LOGO_WIDTH - 1 do
            -- dvd.png is a white-on-black mask: black pixels are transparent.
            if _M.logo[pixel] ~= 0 then
                drawPanelPixel(
                    math.floor(panelOffset + _M.x + x),
                    math.floor(_M.y + y),
                    pixelColor(x, y)
                )
            end
            pixel = pixel + 1
        end
    end
end

local function chooseNextColor()
    local previous = _M.colorIndex
    repeat
        _M.colorIndex = math.random(1, #COLORS)
    until #COLORS == 1 or _M.colorIndex ~= previous
end

local function queueSound(sequence)
    if not SOUND_ENABLED then
        return
    end

    _M.soundSequence = sequence
    _M.soundIndex = 1
    _M.nextSoundAt = 0
end

local function updateSound()
    local sequence = _M.soundSequence
    if not sequence or millis() < _M.nextSoundAt then
        return
    end

    local note = sequence[_M.soundIndex]
    if not note then
        _M.soundSequence = nil
        return
    end

    toneDuration(note[1], note[2])
    _M.soundIndex = _M.soundIndex + 1
    _M.nextSoundAt = millis() + note[2] + note[3]
end

local function updateOledStatus()
    oledClearScreen()
    oledSetCursor(0, 0)
    oledDrawText("DVD")
    oledSetCursor(0, 16)
    oledDrawText("WALLS: " .. _M.wallHits)
    oledSetCursor(0, 32)
    oledDrawText("CORNERS: " .. _M.cornerHits)
    oledSetCursor(0, 52)
    oledDrawText("BACK: exit")
    oledDisplay()
end

local function updatePosition(dt)
    local normalizedSpeed = clamp(SPEED, 0, 1)
    local pixelsPerSecond = normalizedSpeed * 120
    local seconds = math.min(dt, 50) / 1000
    local hitHorizontal = false
    local hitVertical = false

    _M.x = _M.x + _M.directionX * pixelsPerSecond * seconds
    _M.y = _M.y + _M.directionY * pixelsPerSecond * VERTICAL_SPEED * seconds

    if _M.x <= 0 then
        _M.x = 0
        _M.directionX = 1
        hitHorizontal = true
    elseif _M.x + LOGO_WIDTH >= PANEL_WIDTH then
        _M.x = PANEL_WIDTH - LOGO_WIDTH
        _M.directionX = -1
        hitHorizontal = true
    end

    if _M.y <= 0 then
        _M.y = 0
        _M.directionY = 1
        hitVertical = true
    elseif _M.y + LOGO_HEIGHT >= PANEL_HEIGHT then
        _M.y = PANEL_HEIGHT - LOGO_HEIGHT
        _M.directionY = -1
        hitVertical = true
    end

    if hitHorizontal and not hitVertical then
        if _M.y <= CORNER_TOLERANCE then
            _M.y = 0
            _M.directionY = 1
            hitVertical = true
        elseif _M.y + LOGO_HEIGHT >= PANEL_HEIGHT - CORNER_TOLERANCE then
            _M.y = PANEL_HEIGHT - LOGO_HEIGHT
            _M.directionY = -1
            hitVertical = true
        end
    elseif hitVertical and not hitHorizontal then
        if _M.x <= CORNER_TOLERANCE then
            _M.x = 0
            _M.directionX = 1
            hitHorizontal = true
        elseif _M.x + LOGO_WIDTH >= PANEL_WIDTH - CORNER_TOLERANCE then
            _M.x = PANEL_WIDTH - LOGO_WIDTH
            _M.directionX = -1
            hitHorizontal = true
        end
    end

    if hitHorizontal and hitVertical then
        _M.rainbow = true
        _M.cornerHits = _M.cornerHits + 1
        queueSound(CORNER_FANFARE)
    elseif hitHorizontal or hitVertical then
        _M.rainbow = false
        _M.wallHits = _M.wallHits + 1
        chooseNextColor()
        queueSound(EDGE_BEEP)
    end

    if hitHorizontal or hitVertical then
        updateOledStatus()
    end
end

function _M.onSetup()
    setPanelManaged(false)
    math.randomseed(millis())
    _M.logo = decodePng("/scripts/dvd.png")
    _M.wallHits = 0
    _M.cornerHits = 0
    noTone()
    updateOledStatus()
end

function _M.onLoop(dt)
    updatePosition(dt)
    updateSound()
    _M.rainbowPhase = (_M.rainbowPhase + dt * 0.12) % 360

    clearPanelBuffer()
    drawLogo(0)
    drawLogo(PANEL_WIDTH)
    flipPanelBuffer()

    if input.readButtonStatus(BUTTON_BACK) == BUTTON_JUST_PRESSED then
        _M.shouldStop = true
        return true
    end
end

function _M.onClose()
    noTone()
    _M.logo = nil
    collectgarbage()
end

return _M
