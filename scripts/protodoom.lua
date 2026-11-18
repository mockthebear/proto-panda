-- PROTO DOOM
-- A tiny, original Doom-inspired raycaster made for the 64x32 Proto Panda panels.
-- This uses no Doom WAD/assets: it is deliberately lightweight and self-contained.
-- Credits: jung (@junglivre)

local _M = {
    VERSION_REQUIRED = "3.2.1",
    shouldStop = false,
    state = "title",
    width = 64,
    height = 32,
    fov = 1.10,
    maxDepth = 8,
    turnSpeed = 2.65,
    walkSpeed = 2.25,
    shootCooldown = 0,
    muzzle = 0,
    hurtFlash = 0,
    transition = 0,
    level = 1,
    score = 0,
}

local input = require("input")

-- 1 = wall, 2 = closed door, 0 = walkable.
local MAP_TEMPLATE = {
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,1,0,0,0,0,1},
    {1,0,1,0,1,0,1,0,1,1,0,1},
    {1,0,1,0,1,0,0,0,0,1,0,1},
    {1,0,1,0,1,1,1,2,0,1,0,1},
    {1,0,0,0,0,0,1,0,0,0,0,1},
    {1,1,1,1,1,0,1,0,1,1,0,1},
    {1,0,0,0,1,0,0,0,0,1,0,1},
    {1,0,1,0,1,1,1,1,0,1,0,1},
    {1,0,1,0,0,0,0,1,0,0,0,1},
    {1,0,0,0,1,1,0,0,0,1,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1},
}

local SPAWNS = {
    {10.3, 2.4}, {8.3, 4.5}, {10.2, 6.5}, {7.5, 7.4},
    {3.5, 9.5}, {9.4, 9.5}, {5.4, 5.4}, {2.5, 5.5},
}

local PICKUP_SPAWNS = {
    {4.5, 1.5, "ammo"}, {9.5, 5.5, "health"},
    {2.5, 7.5, "health"}, {8.5, 10.5, "ammo"},
}

local DIGITS = {
    ["0"] = {"111", "101", "101", "101", "111"},
    ["1"] = {"010", "110", "010", "010", "111"},
    ["2"] = {"111", "001", "111", "100", "111"},
    ["3"] = {"111", "001", "111", "001", "111"},
    ["4"] = {"101", "101", "111", "001", "001"},
    ["5"] = {"111", "100", "111", "001", "111"},
    ["6"] = {"111", "100", "111", "101", "111"},
    ["7"] = {"111", "001", "010", "010", "010"},
    ["8"] = {"111", "101", "111", "101", "111"},
    ["9"] = {"111", "101", "111", "001", "111"},
}

-- Chunky, hand-drawn title glyphs.  They are deliberately uneven and get a
-- yellow-to-red fill plus a blood-red drop shadow in drawDoomTitle().
local DOOM_FONT = {
    ["P"] = {"1110", "1001", "1110", "1000", "1000"},
    ["R"] = {"1110", "1001", "1110", "1010", "1001"},
    ["A"] = {"0110", "1001", "1111", "1001", "1001"},
    ["N"] = {"1001", "1101", "1011", "1001", "1001"},
    ["T"] = {"1111", "0110", "0110", "0110", "0110"},
    ["D"] = {"1110", "1001", "1001", "1001", "1110"},
    ["O"] = {"0110", "1001", "1001", "1001", "0110"},
    ["M"] = {"1001", "1111", "1111", "1001", "1001"},
}

local function clamp(value, low, high)
    if value < low then return low end
    if value > high then return high end
    return value
end

local function abs(value)
    return value < 0 and -value or value
end

local function angleDiff(a, b)
    local d = a - b
    while d > math.pi do d = d - math.pi * 2 end
    while d < -math.pi do d = d + math.pi * 2 end
    return d
end

local function isDown(button)
    local state = input.readButtonStatus(button)
    return state == BUTTON_PRESSED or state == BUTTON_JUST_PRESSED
end

-- The visor halves are wired left-to-right. Draw the same scene on both;
-- mirroring the right coordinates makes its image appear reversed.
local function rect(x, y, w, h, color)
    if w <= 0 or h <= 0 then return end
    drawPanelFillRect(x, y, w, h, color)
    drawPanelFillRect(64 + x, y, w, h, color)
end

local function pixel(x, y, color)
    drawPanelPixel(x, y, color)
    drawPanelPixel(64 + x, y, color)
end

local function drawDoomTitle(x, y)
    local text = "PROTODOOM"
    local cursor = x

    -- First pass: the offset shadow gives the letters a hard, metal-logo edge.
    for i = 1, #text do
        local glyph = DOOM_FONT[text:sub(i, i)]
        for row = 1, 5 do
            for col = 1, 4 do
                if glyph[row]:sub(col, col) == "1" then
                    pixel(cursor + col, y + row, _M.colors.enemy)
                end
            end
        end
        cursor = cursor + 5 + (i == 5 and 2 or 0)
    end

    cursor = x
    for i = 1, #text do
        local glyph = DOOM_FONT[text:sub(i, i)]
        for row = 1, 5 do
            local color = row == 1 and _M.colors.muzzle
                or row == 2 and _M.colors.title
                or row == 3 and _M.colors.door
                or _M.colors.wallNear
            for col = 1, 4 do
                if glyph[row]:sub(col, col) == "1" then
                    pixel(cursor + col - 1, y + row - 1, color)
                end
            end
        end
        cursor = cursor + 5 + (i == 5 and 2 or 0)
    end
end

local function number(x, y, value, color, scale)
    scale = scale or 1
    local text = tostring(math.floor(value))
    for i = 1, #text do
        local glyph = DIGITS[text:sub(i, i)]
        if glyph then
            local originX = x + (i - 1) * 4 * scale
            for row = 1, 5 do
                for col = 1, 3 do
                    if glyph[row]:sub(col, col) == "1" then
                        rect(originX + (col - 1) * scale, y + (row - 1) * scale, scale, scale, color)
                    end
                end
            end
        end
    end
end

local function label(x, y, text, color)
    for i = 1, #text do
        local px = x + (i - 1) * 6
        local c = string.byte(text:sub(i, i))
        drawPanelChar(px, y, c, color, 0, 1)
        drawPanelChar(64 + px, y, c, color, 0, 1)
    end
end

local function copyMap()
    local map = {}
    for y = 1, #MAP_TEMPLATE do
        map[y] = {}
        for x = 1, #MAP_TEMPLATE[y] do
            map[y][x] = MAP_TEMPLATE[y][x]
        end
    end
    return map
end

local function tileAt(x, y)
    local row = _M.map[math.floor(y) + 1]
    if not row then return 1 end
    return row[math.floor(x) + 1] or 1
end

local function isSolid(x, y)
    return tileAt(x, y) ~= 0
end

local function canStand(x, y)
    local r = 0.20
    return not isSolid(x - r, y - r)
       and not isSolid(x + r, y - r)
       and not isSolid(x - r, y + r)
       and not isSolid(x + r, y + r)
end

local function raycast(angle, limit)
    local step = 0.105
    local distance = 0.05
    local maxDistance = limit or _M.maxDepth
    local dx = math.cos(angle)
    local dy = math.sin(angle)

    while distance < maxDistance do
        local x = _M.player.x + dx * distance
        local y = _M.player.y + dy * distance
        local tile = tileAt(x, y)
        if tile ~= 0 then
            return distance, tile
        end
        distance = distance + step
    end
    return maxDistance, 0
end

local function spawnEnemy(index, extraHealth)
    local spot = SPAWNS[((index - 1) % #SPAWNS) + 1]
    return {
        x = spot[1], y = spot[2],
        hp = 2 + extraHealth,
        cooldown = 0.4 + (index % 3) * 0.25,
        strafe = (index % 2 == 0) and 1 or -1,
        alive = true,
        hit = 0,
        phase = index * 0.9,
    }
end

local function startLevel(level)
    _M.map = copyMap()
    _M.enemies = {}
    _M.pickups = {}
    _M.transition = 0.9
    _M.levelFlash = 0.85

    local enemyCount = _M.survival and 5 or math.min(3 + level, 7)
    for i = 1, enemyCount do
        _M.enemies[#_M.enemies + 1] = spawnEnemy(i + level, math.floor((level - 1) / 3))
    end

    for i = 1, #PICKUP_SPAWNS do
        local item = PICKUP_SPAWNS[i]
        _M.pickups[#_M.pickups + 1] = { x = item[1], y = item[2], kind = item[3], active = true }
    end
end

local function newGame(mode)
    _M.mode = mode or _M.mode or 1
    local hp, ammo = 100, 14
    _M.survival = false
    if _M.mode == 2 then
        ammo = 18
        _M.survival = true
    end
    -- This is an open square in row 2. Keep it out of the wall row below.
    _M.player = { x = 2.45, y = 1.50, angle = 0.10, hp = hp, ammo = ammo }
    _M.level = 1
    _M.score = 0
    _M.state = "play"
    _M.shootCooldown = 0
    _M.muzzle = 0
    _M.hurtFlash = 0
    _M.deathTimer = 0
    _M.spawnTimer = 0
    _M.pathField = nil
    _M.pathTimer = 0
    _M.oledTimer = 0
    startLevel(_M.level)
end

local function livingEnemies()
    local count = 0
    for i = 1, #_M.enemies do
        if _M.enemies[i].alive then count = count + 1 end
    end
    return count
end

local function useDoor()
    local px = _M.player.x + math.cos(_M.player.angle) * 0.75
    local py = _M.player.y + math.sin(_M.player.angle) * 0.75
    local tx, ty = math.floor(px) + 1, math.floor(py) + 1
    if _M.map[ty] and _M.map[ty][tx] == 2 then
        _M.map[ty][tx] = 0
        _M.transition = 0.18
    end
end

local function shoot()
    if _M.shootCooldown > 0 or _M.player.ammo <= 0 then return end
    _M.player.ammo = _M.player.ammo - 1
    _M.shootCooldown = 0.19
    _M.muzzle = 0.10

    local victim, bestDistance
    for i = 1, #_M.enemies do
        local enemy = _M.enemies[i]
        if enemy.alive then
            local dx, dy = enemy.x - _M.player.x, enemy.y - _M.player.y
            local dist = math.sqrt(dx * dx + dy * dy)
            local aim = math.atan(dy, dx)
            local tolerance = 0.065 + 0.23 / math.max(dist, 0.4)
            if abs(angleDiff(aim, _M.player.angle)) < tolerance then
                local wallDistance = raycast(aim, dist + 0.1)
                if wallDistance + 0.18 >= dist and (not bestDistance or dist < bestDistance) then
                    victim, bestDistance = enemy, dist
                end
            end
        end
    end

    if victim then
        victim.hp = victim.hp - 1
        victim.hit = 0.16
        if victim.hp <= 0 then
            victim.alive = false
            _M.score = _M.score + 100
            if math.random() < 0.42 then
                _M.pickups[#_M.pickups + 1] = { x = victim.x, y = victim.y, kind = "ammo", active = true }
            end
        end
    end
end

local function movePlayer(dt)
    local turn = 0
    if isDown(BUTTON_LEFT) then turn = turn - 1 end
    if isDown(BUTTON_RIGHT) then turn = turn + 1 end
    _M.player.angle = _M.player.angle + turn * _M.turnSpeed * dt

    local walk = 0
    if isDown(BUTTON_UP) then walk = walk + 1 end
    if isDown(BUTTON_DOWN) then walk = walk - 0.65 end
    if walk ~= 0 then
        local speed = _M.walkSpeed * walk * dt
        local nextX = _M.player.x + math.cos(_M.player.angle) * speed
        local nextY = _M.player.y + math.sin(_M.player.angle) * speed
        if canStand(nextX, _M.player.y) then _M.player.x = nextX end
        if canStand(_M.player.x, nextY) then _M.player.y = nextY end
    end

    if input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_JUST_PRESSED then shoot() end
    if input.readButtonStatus(BUTTON_AUX_A) == BUTTON_JUST_PRESSED then useDoor() end
end

local function canPathThrough(cellX, cellY)
    local row = _M.map[cellY]
    if not row then return false end
    local tile = row[cellX]
    -- Doors are pathable so a bot will reach and open them instead of
    -- treating the entire far half of the map as unreachable.
    return tile == 0 or tile == 2
end

local function buildPathField()
    local field, queueX, queueY = {}, {}, {}
    for y = 1, #_M.map do field[y] = {} end

    local startX = math.floor(_M.player.x) + 1
    local startY = math.floor(_M.player.y) + 1
    field[startY][startX] = 0
    queueX[1], queueY[1] = startX, startY

    local head = 1
    local directions = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} }
    while head <= #queueX do
        local x, y = queueX[head], queueY[head]
        local cost = field[y][x] + 1
        head = head + 1
        for i = 1, #directions do
            local nx, ny = x + directions[i][1], y + directions[i][2]
            if canPathThrough(nx, ny) and field[ny][nx] == nil then
                field[ny][nx] = cost
                queueX[#queueX + 1], queueY[#queueY + 1] = nx, ny
            end
        end
    end
    return field
end

local function getPathField(dt)
    _M.pathTimer = (_M.pathTimer or 0) - dt
    if _M.pathTimer <= 0 or not _M.pathField then
        _M.pathField = buildPathField()
        _M.pathTimer = 0.12
    end
    return _M.pathField
end

local function updateEnemies(dt, pathField)
    for i = 1, #_M.enemies do
        local enemy = _M.enemies[i]
        if enemy.alive then
            enemy.cooldown = enemy.cooldown - dt
            enemy.hit = math.max(0, enemy.hit - dt)
            enemy.phase = enemy.phase + dt * 5

            local dx, dy = _M.player.x - enemy.x, _M.player.y - enemy.y
            local dist = math.sqrt(dx * dx + dy * dy)
            if dist > 0.01 then
                if dist > 1.05 then
                    local cellX = math.floor(enemy.x) + 1
                    local cellY = math.floor(enemy.y) + 1
                    local bestX, bestY = cellX, cellY
                    local bestCost = pathField[cellY] and pathField[cellY][cellX] or 9999
                    local directions = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} }
                    for d = 1, #directions do
                        local nx, ny = cellX + directions[d][1], cellY + directions[d][2]
                        local cost = pathField[ny] and pathField[ny][nx]
                        if cost and cost < bestCost then
                            bestX, bestY, bestCost = nx, ny, cost
                        end
                    end

                    -- Move to the next shortest-path cell. On a door cell,
                    -- unlock it when close enough and continue through.
                    local targetX, targetY = bestX - 0.5, bestY - 0.5
                    if _M.map[bestY] and _M.map[bestY][bestX] == 2 then
                        local doorDx, doorDy = targetX - enemy.x, targetY - enemy.y
                        if doorDx * doorDx + doorDy * doorDy < 0.55 then
                            _M.map[bestY][bestX] = 0
                            _M.transition = 0.15
                        end
                    end

                    local moveX, moveY = targetX - enemy.x, targetY - enemy.y
                    local moveLength = math.sqrt(moveX * moveX + moveY * moveY)
                    if moveLength > 0.04 then
                        local nx, ny = moveX / moveLength, moveY / moveLength
                        local pushX, pushY = 0, 0
                        for j = 1, #_M.enemies do
                            local other = _M.enemies[j]
                            if other ~= enemy and other.alive then
                                local awayX, awayY = enemy.x - other.x, enemy.y - other.y
                                local awayLength = math.sqrt(awayX * awayX + awayY * awayY)
                                if awayLength > 0.01 and awayLength < 0.62 then
                                    pushX = pushX + awayX / awayLength
                                    pushY = pushY + awayY / awayLength
                                end
                            end
                        end
                    local speed = (0.35 + _M.level * 0.025) * dt
                        local ex = enemy.x + (nx + pushX * 0.42) * speed
                        local ey = enemy.y + (ny + pushY * 0.42) * speed
                    if canStand(ex, enemy.y) then enemy.x = ex else enemy.strafe = -enemy.strafe end
                    if canStand(enemy.x, ey) then enemy.y = ey else enemy.strafe = -enemy.strafe end
                    end
                elseif dist < 1.35 and enemy.cooldown <= 0 then
                    _M.player.hp = _M.player.hp - (5 + _M.level)
                    _M.hurtFlash = 0.18
                    enemy.cooldown = 0.75
                end
            end
        end
    end
end

local function updatePickups()
    for i = 1, #_M.pickups do
        local item = _M.pickups[i]
        if item.active then
            local dx, dy = item.x - _M.player.x, item.y - _M.player.y
            if dx * dx + dy * dy < 0.45 then
                item.active = false
                if item.kind == "health" then
                    _M.player.hp = math.min(100, _M.player.hp + 28)
                else
                    _M.player.ammo = math.min(30, _M.player.ammo + 6)
                end
            end
        end
    end
end

local function update(dtMs)
    local dt = clamp((dtMs or 16) / 1000, 0, 0.050)
    _M.shootCooldown = math.max(0, _M.shootCooldown - dt)
    _M.muzzle = math.max(0, _M.muzzle - dt)
    _M.hurtFlash = math.max(0, _M.hurtFlash - dt)
    _M.transition = math.max(0, _M.transition - dt)
    _M.levelFlash = math.max(0, (_M.levelFlash or 0) - dt)

    movePlayer(dt)
    updateEnemies(dt, getPathField(dt))
    updatePickups()

    if _M.player.hp <= 0 then
        _M.state = "dead"
        _M.deathTimer = 1.0
    elseif _M.survival then
        _M.spawnTimer = _M.spawnTimer - dt
        local wanted = math.min(4 + math.floor(_M.score / 500), 7)
        if livingEnemies() < wanted and _M.spawnTimer <= 0 then
            _M.enemies[#_M.enemies + 1] = spawnEnemy(#_M.enemies + _M.score + 1, math.floor(_M.score / 1000))
            _M.spawnTimer = 0.85
        end
    elseif livingEnemies() == 0 then
        _M.level = _M.level + 1
        startLevel(_M.level)
    end
end

local function shadeWall(distance, tile)
    if tile == 2 then return _M.colors.door end
    if distance < 1.6 then return _M.colors.wallNear end
    if distance < 3.4 then return _M.colors.wallMid end
    return _M.colors.wallFar
end

local function renderWorld()
    clearPanelBuffer()
    rect(0, 0, 64, 16, _M.colors.sky)
    rect(0, 16, 64, 16, _M.colors.floor)
    _M.rays = {}

    for col = 0, 31 do
        local screenX = col * 2
        local rayAngle = _M.player.angle - _M.fov * 0.5 + ((col + 0.5) / 32) * _M.fov
        local distance, tile = raycast(rayAngle)
        distance = distance * math.cos(rayAngle - _M.player.angle)
        _M.rays[col + 1] = distance

        local wallHeight = clamp(math.floor(27 / math.max(distance, 0.2)), 1, 31)
        local top = math.floor((32 - wallHeight) * 0.5)
        rect(screenX, top, 2, wallHeight, shadeWall(distance, tile))
        if distance < 2.4 then
            pixel(screenX, top, _M.colors.edge)
            pixel(screenX + 1, top, _M.colors.edge)
        end
    end
end

local function renderObject(x, y, color, kind)
    local dx, dy = x - _M.player.x, y - _M.player.y
    local distance = math.sqrt(dx * dx + dy * dy)
    local relative = angleDiff(math.atan(dy, dx), _M.player.angle)
    if abs(relative) > _M.fov * 0.60 or distance < 0.12 then return end

    local screen = math.floor((relative / _M.fov + 0.5) * 64)
    local rayIndex = clamp(math.floor(screen / 2) + 1, 1, 32)
    if distance > _M.rays[rayIndex] + 0.16 then return end

    local size = clamp(math.floor((kind == "enemy" and 15 or 8) / distance), 2, kind == "enemy" and 13 or 7)
    local left = clamp(screen - math.floor(size / 2), 0, 63)
    local yPos = kind == "enemy" and (16 - math.floor(size * 0.35)) or (23 - math.floor(size * 0.20))
    rect(left, yPos, size, size, color)

    if kind == "enemy" and size >= 5 then
        rect(left + math.floor(size * 0.22), yPos + math.floor(size * 0.28), 1, 1, _M.colors.eye)
        rect(left + math.floor(size * 0.68), yPos + math.floor(size * 0.28), 1, 1, _M.colors.eye)
    end
end

local function renderSprites()
    local drawList = {}
    for i = 1, #_M.enemies do
        local enemy = _M.enemies[i]
        if enemy.alive then
            local dx, dy = enemy.x - _M.player.x, enemy.y - _M.player.y
            drawList[#drawList + 1] = { d = dx * dx + dy * dy, e = enemy }
        end
    end
    table.sort(drawList, function(a, b) return a.d > b.d end)
    for i = 1, #drawList do
        local enemy = drawList[i].e
        local color = enemy.hit > 0 and _M.colors.enemyHit or _M.colors.enemy
        renderObject(enemy.x, enemy.y, color, "enemy")
    end
    for i = 1, #_M.pickups do
        local item = _M.pickups[i]
        if item.active then
            renderObject(item.x, item.y, item.kind == "health" and _M.colors.health or _M.colors.ammo, "pickup")
        end
    end
end

local function renderHud()
    rect(1, 1, 18, 2, _M.colors.hudDark)
    rect(1, 1, math.floor(18 * _M.player.hp / 100), 2, _M.colors.health)
    rect(45, 1, 18, 2, _M.colors.hudDark)
    rect(45, 1, math.floor(18 * _M.player.ammo / 30), 2, _M.colors.ammo)
    if not _M.survival then
        number(2, 4, _M.level, _M.colors.health)
    end
    number(46, 4, _M.score % 10000, _M.colors.ammo)

    if _M.levelFlash > 0 and not _M.survival then
        number(29, 9, _M.level, _M.colors.title, 2)
    end

    -- Small crosshair and a deliberately chunky weapon silhouette.
    pixel(31, 15, _M.colors.crosshair)
    pixel(32, 15, _M.colors.crosshair)
    pixel(31, 16, _M.colors.crosshair)
    pixel(32, 16, _M.colors.crosshair)
    rect(27, 29, 10, 3, _M.muzzle > 0 and _M.colors.muzzle or _M.colors.weapon)
    rect(30, 27, 4, 3, _M.colors.weapon)

    if _M.hurtFlash > 0 then
        rect(0, 0, 64, 1, _M.colors.enemyHit)
        rect(0, 31, 64, 1, _M.colors.enemyHit)
    end
end

local function renderMenu()
    clearPanelBuffer()
    rect(0, 0, 64, 32, _M.colors.titleBg)
    drawDoomTitle(7, 1)
    rect(3, 8, 58, 1, _M.colors.wallNear)

    local cards = { 12, 36 }
    for i = 1, 2 do
        local selected = i == _M.menuChoice
        local border = selected and _M.colors.title or _M.colors.wallFar
        local inside = selected and _M.colors.wallMid or _M.colors.hudDark
        rect(cards[i], 10, 16, 17, border)
        rect(cards[i] + 2, 12, 12, 13, inside)
    end

    -- Green play arrow = campaign. Red skull = survival: endless respawns.
    rect(17, 14, 2, 8, _M.colors.health)
    rect(19, 16, 2, 4, _M.colors.health)
    rect(21, 17, 2, 2, _M.colors.health)
    rect(42, 14, 6, 7, _M.colors.enemy)
    rect(43, 16, 1, 2, _M.colors.deadBg)
    rect(46, 16, 1, 2, _M.colors.deadBg)
    rect(43, 22, 1, 2, _M.colors.enemy)
    rect(46, 22, 1, 2, _M.colors.enemy)

    for i = 1, 2 do
        rect(29 + (i - 1) * 5, 28, 3, 2, i == _M.menuChoice and _M.colors.title or _M.colors.wallFar)
    end
end

local function renderDead()
    clearPanelBuffer()
    rect(0, 0, 64, 32, _M.colors.deadBg)
    rect(23, 7, 18, 16, _M.colors.enemyHit)
    rect(26, 11, 3, 3, _M.colors.deadBg)
    rect(35, 11, 3, 3, _M.colors.deadBg)
    rect(28, 18, 8, 2, _M.colors.deadBg)
    rect(15, 27, 34, 2, _M.colors.enemy)
end

local function showMenuInfo()
    oledClearScreen()
    oledSetCursor(0, 0)
    oledDrawText("PROTO DOOM\n\nSELECTED:\n" .. (_M.menuChoice == 1 and "CAMPAIGN" or "ENDLESS") .. "\n\n< > change\nCONFIRM start")
    oledDisplay()
end

local function drawOledBar(y, name, value, maximum)
    local width = math.floor(92 * clamp(value / maximum, 0, 1))
    oledSetCursor(0, y)
    oledDrawText(name)
    oledDrawRect(33, y, 94, 8, 1)
    oledDrawFilledRect(34, y + 1, width, 6, 1)
end

local function showGameInfo()
    oledClearScreen()
    oledSetCursor(0, 0)
    oledDrawText(_M.survival and "ENDLESS" or "CAMPAIGN")
    oledSetCursor(0, 12)
    oledDrawText("SCORE: " .. _M.score)
    oledSetCursor(0, 23)
    oledDrawText(_M.survival and "SURVIVE" or "LEVEL: " .. _M.level)
    drawOledBar(36, "HP", _M.player.hp, 100)
    drawOledBar(49, "AMMO", _M.player.ammo, 30)
    oledDisplay()
end

local function showDeadInfo()
    oledClearScreen()
    oledSetCursor(0, 0)
    oledDrawText("YOU DIED\n\nSCORE: " .. _M.score .. "\n\nRESTARTING...")
    oledDisplay()
end

local function returnToMenu()
    _M.state = "menu"
    _M.menuChoice = 1
    _M.oledTimer = 0
    showMenuInfo()
end

function _M.onSetup()
    setPanelManaged(false)
    _M.colors = {
        sky = color565(10, 15, 34), floor = color565(22, 16, 15),
        wallNear = color565(182, 55, 36), wallMid = color565(110, 36, 30), wallFar = color565(54, 25, 29),
        door = color565(180, 118, 30), edge = color565(255, 145, 72),
        enemy = color565(182, 28, 42), enemyHit = color565(255, 225, 170), eye = color565(255, 235, 80),
        health = color565(40, 220, 85), ammo = color565(70, 165, 255),
        hudDark = color565(20, 23, 30), crosshair = color565(230, 230, 220), weapon = color565(125, 125, 130), muzzle = color565(255, 250, 170),
        titleBg = color565(9, 7, 16), title = color565(255, 180, 35), text = color565(235, 235, 230), deadBg = color565(45, 5, 8),
    }
    math.randomseed(math.floor((getFreeHeap() or 1) + (getFreePsram() or 1)))
    returnToMenu()
end

function _M.onLoop(dt)
    if _M.state == "menu" then
        if input.readButtonStatus(BUTTON_LEFT) == BUTTON_JUST_PRESSED then
            _M.menuChoice = _M.menuChoice == 1 and 2 or _M.menuChoice - 1
            showMenuInfo()
        elseif input.readButtonStatus(BUTTON_RIGHT) == BUTTON_JUST_PRESSED then
            _M.menuChoice = _M.menuChoice == 2 and 1 or _M.menuChoice + 1
            showMenuInfo()
        elseif input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_JUST_PRESSED then
            newGame(_M.menuChoice)
            showGameInfo()
        end
        renderMenu()
    elseif _M.state == "dead" then
        renderDead()
        if _M.oledTimer <= 0 then
            showDeadInfo()
            _M.oledTimer = 0.9
        end
        _M.deathTimer = _M.deathTimer - clamp((dt or 16) / 1000, 0, 0.05)
        _M.oledTimer = _M.oledTimer - clamp((dt or 16) / 1000, 0, 0.05)
        if _M.deathTimer <= 0 or input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_JUST_PRESSED then newGame() end
    else
        update(dt)
        renderWorld()
        renderSprites()
        renderHud()
        _M.oledTimer = _M.oledTimer - clamp((dt or 16) / 1000, 0, 0.05)
        if _M.oledTimer <= 0 then
            showGameInfo()
            _M.oledTimer = 0.20
        end
    end

    flipPanelBuffer()

    if input.readButtonStatus(BUTTON_BACK) == BUTTON_JUST_PRESSED then
        if _M.state == "menu" then
            _M.shouldStop = true
            return true
        end
        returnToMenu()
    end
end

function _M.onClose()
    collectgarbage()
end

return _M
