--[[
 This file is part of:
   ______            _____           _   ___________ 
   | ___ \          /  __ \         | | |____ |  _  \
   | |_/ /__ _ _   _| /  \/ __ _ ___| |_    / / | | |
   |    // _` | | | | |    / _` / __| __|   \ \ | | |
   | |\ \ (_| | |_| | \__/\ (_| \__ \ |_.___/ / |/ / 
   \_| \_\__,_|\__, |\____/\__,_|___/\__\____/|___/  
               __/ |                                
              |___/            by Rinnegatamante

 Lua Game Engine made to create 3D games using Ray-Casting
 algorithm.
]]--

-- Movements globals
LEFT = 0
RIGHT = 1
FORWARD = 2
BACK = 3

-- Screen Globals
local vwidth = 960 -- Pixels
local vheight = 544 -- Pixels
local viewsize = 60 -- Degrees

-- Map Globals
local wall_height = 64
local tile_size = 64
local tile_shift = 6
local map_width = 1
local map_height = 1
local map = { 
	0 
}

-- Player Globals
local pl_x = 0
local pl_y = 0
local pl_angle = 0
 
-- Local used variables (Slight speedup)
local tmp
local scale_y

-- Local funcs definitions (Slight speedup)
local floor_num = math.floor
local ceil_num = math.ceil
local drawImage = Graphics.drawImageExtended
local drawRect = Graphics.fillRect
local rad2deg = math.deg
local deg2rad = math.rad
local genColor = Color.new
local doTan = math.tan
local doSin = math.sin
local doCos = math.cos
local doMin = math.min
local PI = math.pi

 -- Colors Globals
local floor_c = genColor(255, 255, 255, 255)
local sky_c = genColor(0, 0, 0, 255)
local wall_c = genColor(0, 0, 255, 255)
local player_c = genColor(255, 255, 0, 255)
local shad_r = 0
local shad_g = 0
local shad_b = 0

-- Angles Globals (DON'T EDIT)
local ANGLE60 = vwidth
local ANGLE30 = ANGLE60 >> 1
local ANGLE15 = ANGLE30 >> 1
local ANGLE5 = ANGLE15 / 3
local ANGLE10 = ANGLE5 << 1
local ANGLE0 = 0
local ANGLE90 = ANGLE30 * 3
local ANGLE180 = ANGLE90 << 1
local ANGLE270 = ANGLE90 * 3
local ANGLE360 = ANGLE180 << 1

-- PreCalculated Trigonometric Tables (DON'T EDIT)
local sintable = {}
local sintable2 = {}
local costable = {}
local costable2 = {}
local tantable = {}
local tantable2 = {}
local fishtable = {} -- Anti-fishbowl effect values
local xsteptable = {}
local ysteptable = {}

-- Internal Globals (DON'T EDIT)
local ycenter = vheight >> 1
local dist_proj =  (vwidth >> 1) / doTan(deg2rad(viewsize >> 1))
local accuracy = 2
local shad_val = 500
local floors = false
local sky = false
local noclip = false
local shading = false
RayCast3D = {}

-- Internal Functions (DON'T EDIT)
local function arc2rad(val)
	return (val*PI)/ANGLE180
end
local function rad2arc(val)
	return (val*ANGLE180)/PI
end

local function WallRender(x,y,stride,top_wall,wh,cell_idx,offs)
	tmp = map[cell_idx]
	if tmp == nil then
		return
	end
	if tmp == 1 or tmp == 0 then
		drawRect(x+stride,x+stride+accuracy,y+top_wall,y+top_wall+wh,wall_c)
	else
		scale_y = wh / tile_size
		drawImage(x+stride,y+top_wall, offs, 0, accuracy, tile_size, 1, scale_y, tmp)
	end
end
local function WallRenderShad(x,y,stride,top_wall,wh,cell_idx,offs)
	tmp = map[cell_idx]
	scale_y = wh / tile_size
	if tmp == nil then
		return
	end
	if tmp == 1 or tmp == 0 then
		drawRect(x+stride,x+stride+accuracy,y+top_wall,y+top_wall+wh,wall_c)
	else
		drawImage(x+stride,y+top_wall, offs, 0, accuracy+1, tile_size, 1, scale_y, tmp)
	end
	drawRect(x+stride,x+stride+accuracy,y+top_wall,y+top_wall+wh,genColor(shad_r,shad_g,shad_b,doMin(255,floor_num(shad_val / scale_y))))
end
local function WallFloorRender(x,y,stride,top_wall,wh,cell_idx,offs)
	drawRect(x+stride,x+stride+accuracy,y+top_wall+wh,vheight,floor_c)
	tmp = map[cell_idx]
	if tmp == nil then
		return
	end
	if tmp == 1 or tmp == 0 then
		drawRect(x+stride,x+stride+accuracy,y+top_wall,y+top_wall+wh,wall_c)
	else
		scale_y = wh / tile_size
		drawImage(x+stride,y+top_wall, offs, 0, accuracy, tile_size, 1.0, scale_y, tmp)
	end
end
local function WallFloorRenderShad(x,y,stride,top_wall,wh,cell_idx,offs)
	drawRect(x+stride,x+stride+accuracy,y+top_wall+wh,vheight,floor_c)
	tmp = map[cell_idx]
	if tmp == nil then
		return
	end
	scale_y = wh / tile_size
	if tmp == 1 or tmp == 0 then
		drawRect(x+stride,x+stride+accuracy,y+top_wall,y+top_wall+wh,wall_c)
	else
		drawImage(x+stride,y+top_wall, offs, 0, accuracy, tile_size, 1.0, scale_y, tmp)
	end
	drawRect(x+stride,x+stride+accuracy,y+top_wall,y+top_wall+wh,genColor(shad_r,shad_g,shad_b,doMin(255,floor_num(shad_val / scale_y))))
end
local function WallSkyRender(x,y,stride,top_wall,wh,cell_idx,offs)
	tmp = map[cell_idx]
	if tmp == nil then
		return
	end
	if tmp == 1 or tmp == 0 then
		drawRect(x+stride,x+stride+accuracy,y+top_wall,y+top_wall+wh,wall_c)
	else
		scale_y = wh / tile_size
		drawImage(x+stride,y+top_wall, offs, 0, accuracy, tile_size, 1, scale_y, tmp)
	end
	drawRect(x+stride,x+stride+accuracy,y+top_wall,y,floor_c)
end
local function WallSkyRenderShad(x,y,stride,top_wall,wh,cell_idx,offs)
	tmp = map[cell_idx]
	drawRect(x+stride,x+stride+accuracy,y+top_wall,y,floor_c)
	if tmp == nil then
		return
	end
	scale_y = wh / tile_size
	if tmp == 1 or tmp == 0 then
		drawRect(x+stride,x+stride+accuracy,y+top_wall,y+top_wall+wh,wall_c)
	else
		drawImage(x+stride,y+top_wall, offs, 0, accuracy, tile_size, 1, scale_y, tmp)
	end
	drawRect(x+stride,x+stride+accuracy,y+top_wall,y+top_wall+wh,genColor(shad_r,shad_g,shad_b,doMin(255,floor_num(shad_val / scale_y))))
end
local function WallFloorSkyRender(x,y,stride,top_wall,wh,cell_idx,offs)
	tmp = map[cell_idx]
	if tmp == nil then
		return
	end
	if tmp == 1 or tmp == 0 then
		drawRect(x+stride,x+stride+accuracy,y+top_wall,y+top_wall+wh,wall_c)
	else
		scale_y = wh / tile_size
		drawImage(x+stride,y+top_wall, offs, 0, accuracy, tile_size, 1, scale_y, tmp)
	end
	drawRect(x+stride,x+stride+accuracy,y+top_wall+wh,vheight,floor_c)
	drawRect(x+stride,x+stride+accuracy,y+top_wall,y,sky_c)
end
local function WallFloorSkyRenderShad(x,y,stride,top_wall,wh,cell_idx,offs)
	tmp = map[cell_idx]
	scale_y = wh / tile_size
	drawRect(x+stride,x+stride+accuracy,y+top_wall+wh,vheight,floor_c)
	drawRect(x+stride,x+stride+accuracy,y+top_wall,y,sky_c)
	if tmp == nil then
		return
	end
	if tmp == 1 or tmp == 0 then
		drawRect(x+stride,x+stride+accuracy,y+top_wall,y+top_wall+wh,wall_c)
	else
		drawImage(x+stride,y+top_wall, offs, 0, accuracy, tile_size, 1, scale_y, tmp)
	end
	drawRect(x+stride,x+stride+accuracy,y+top_wall,y+top_wall+wh,genColor(shad_r,shad_g,shad_b,doMin(255,floor_num(shad_val / scale_y))))
end
local RenderRay = WallRender
local function ResetAngles()
	ANGLE60 = vwidth
	ANGLE30 = ANGLE60 >> 1
	ANGLE15 = ANGLE30 >> 1
	ANGLE5 = floor_num(ANGLE30 / 6)
	ANGLE10 = ANGLE5 << 1
	ANGLE0 = 0
	ANGLE90 = ANGLE30 * 3
	ANGLE180 = ANGLE90 << 1
	ANGLE270 = ANGLE90 * 3
	ANGLE360 = ANGLE180 << 1
	local i = 0
	local v
	sintable = {}
	sintable2 = {}
	costable = {}
	costable2 = {}
	tantable = {}
	tantable2 = {}
	fishtable = {}
	xsteptable = {}
	ysteptable = {}
	while i <= ANGLE360 do
		v = arc2rad(i) + 0.0001 -- avoid asymptotics values
		sintable[i] = doSin(v)
		sintable2[i] = (1.0/(sintable[i]))
		costable[i] = doCos(v)
		costable2[i] = (1.0/(costable[i]))
		tantable[i] = sintable[i] / costable[i]
		tantable2[i] = (1.0/(tantable[i]))
		if (i >= ANGLE90 and i < ANGLE270) then
			xsteptable[i] = -math.abs(tile_size / tantable[i])
		else
			xsteptable[i] = math.abs(tile_size / tantable[i])
		end
		if (i >= ANGLE0 and i < ANGLE180) then
			ysteptable[i] = math.abs(tile_size * tantable[i])
		else
			ysteptable[i] = -math.abs(tile_size * tantable[i])
		end
		i = i + 1
	end
	i = -ANGLE30
	while i <= ANGLE30 do
		v = arc2rad(i)
		fishtable[i+ANGLE30] =  1.0 / doCos(v)
		i = i + 1
	end
end
local function ResetProjections()
	dist_proj =  (vwidth >> 1) / doTan(deg2rad(viewsize >> 1))
end
local function ResetRenderer()
	if floors then
		if sky then
			if shading then
				RenderRay = WallFloorSkyRenderShad
			else
				RenderRay = WallFloorSkyRender
			end
		else
			if shading then
				RenderRay = WallFloorRenderShad
			else
				RenderRay = WallFloorRender
			end
		end
	else
		if sky then
			if shading then
				RenderRay = WallSkyRenderShad
			else
				RenderRay = WallSkyRender
			end
		else
			if shading then
				RenderRay = WallRenderShad
			else
				RenderRay = WallRender
			end
		end
	end
end
local function ResetEngine()
	ResetAngles()
	ResetProjections()
	ycenter = vheight >> 1
end

--[[setResolution: Sets renderer resolution]]--
function RayCast3D.setResolution(w, h)

	-- Change screen resolution
	vwidth = w
	vheight = h
	
	-- Reset engine with new values
	ResetEngine()
	
end

--[[setViewsize: Sets FOV]]--
function RayCast3D.setViewsize(angle)
	viewsize = angle
	ResetProjections()
end

--[[renderScene: Render viewport scene using GPU]]--
function RayCast3D.renderScene(x, y)
	local castArc = pl_angle - ANGLE30
	if castArc < 0 then
		castArc = ANGLE360 + castArc
	end
	local stride = 0
	local hgrid
	local vgrid
	local xtmp
	local xinter
	local yinter
	local ytmp
	local dist_next_hgrid
	local dist_next_vgrid
	local dist_hgrid_hit
	local dist_vgrid_hit
	local cell_idx
	local cell_idx_x
	local cell_idx_y
	while stride < vwidth do
		if (castArc > ANGLE0 and castArc < ANGLE180) then
			hgrid = ((pl_y >> tile_shift) << tile_shift) + tile_size
			dist_next_hgrid = tile_size
			xtmp = tantable2[castArc]*(hgrid-pl_y)
			xinter = xtmp + pl_x
		else
			hgrid = ((pl_y >> tile_shift) << tile_shift)
			dist_next_hgrid = -tile_size
			xtmp = tantable2[castArc]*(hgrid-pl_y)
			xinter = xtmp + pl_x
			hgrid = hgrid - 1
		end
		if (castArc == ANGLE0 or castArc == ANGLE180) then -- Prevent asymptotics values
			dist_hgrid_hit = 99999
		else
			local dist_next_xinter = xsteptable[castArc]
			while true do
				xgrid_index = floor_num(xinter) >> tile_shift
				ygrid_index = hgrid >> tile_shift
				cell_idx_x = ygrid_index*map_width+xgrid_index+1
				if (xgrid_index >= map_width or ygrid_index >= map_height or xgrid_index < 0 or ygrid_index < 0) then
					dist_hgrid_hit = 9999
					break
				elseif (map[cell_idx_x] ~= 0) then
					dist_hgrid_hit = (xinter - pl_x) * costable2[castArc]
					break
				else
					xinter = xinter + dist_next_xinter
					hgrid = hgrid + dist_next_hgrid
				end
			end
		end
		if castArc < ANGLE90 or castArc > ANGLE270 then
			vgrid = tile_size + ((pl_x >> tile_shift) << tile_shift)
			dist_next_vgrid = tile_size
			ytmp = tantable[castArc]*(vgrid - pl_x)
			yinter = ytmp + pl_y
		else
			vgrid = (pl_x >> tile_shift) << tile_shift
			dist_next_vgrid = 0 - tile_size
			ytmp = tantable[castArc]*(vgrid-pl_x)
			yinter = ytmp + pl_y
			vgrid = vgrid - 1
		end
		if (castArc == ANGLE90 or castArc == ANGLE270) then
			dist_vgrid_hit = 99999
		else
			local dist_next_yinter = ysteptable[castArc]
			dist_vgrid_hit = 0
			while dist_vgrid_hit <= dist_hgrid_hit do
				xgrid_index = vgrid >> tile_shift
				ygrid_index = floor_num(yinter) >> tile_shift
				cell_idx_y = ygrid_index*map_width+xgrid_index+1
				if (xgrid_index >= map_width or ygrid_index >= map_height or xgrid_index < 0 or ygrid_index < 0) then
					dist_vgrid_hit = 9999
					break
				elseif (map[cell_idx_y] ~= 0) then
					dist_vgrid_hit = (yinter-pl_y)*sintable2[castArc]
					break
				else
					yinter = yinter + dist_next_yinter
					vgrid = vgrid + dist_next_vgrid
				end
			end
		end
		if (dist_hgrid_hit < dist_vgrid_hit) then
			dist = dist_hgrid_hit
			xinter = floor_num(xinter)
			offs = xinter - ((xinter >> tile_shift) << tile_shift)			
			cell_idx = cell_idx_x
		else
			dist = dist_vgrid_hit
			yinter = floor_num(yinter)
			offs = yinter - ((yinter >> tile_shift) << tile_shift)
			cell_idx = cell_idx_y
		end
		dist = dist / fishtable[stride]
		wh = floor_num(wall_height * dist_proj / dist)
		bot_wall = ycenter + floor_num(wh * 0.5)
		top_wall = vheight-bot_wall
		if (bot_wall >= vheight) then
			bot_wall = vheight - 1
		end
		RenderRay(x,y,stride,top_wall,wh,cell_idx,offs)
		stride = stride + accuracy
		castArc = castArc + accuracy
		if castArc >= ANGLE360 then
			castArc = castArc - ANGLE360
		end
	end
end

--[[renderMap: Render 2D map scene using GPU]]--
function RayCast3D.renderMap(x, y, width)
	local u = 0
	local v = 0
	while (u < map_width) do
		v = 0
		while (v < map_height) do
			tmp = map[v*map_width+u+1]
			if (tmp==0) then
				color = floor_c
			else
				if tmp == 1 then
					color = wall_c
				else
					
				end
			end
			xp = x + u * width
			yp = y + v * width
			if tmp == 1 or tmp == 0 then
				drawRect(xp, xp + width, yp, yp + width, color)
			else
				w = Graphics.getImageWidth(tmp)
				s = width / w
				Graphics.drawScaleImage(xp, yp, tmp, s, s)
			end
			v = v + 1
		end
		u = u + 1
	end
	local xpp = x + (pl_x / tile_size) * width
	local ypp = y + (pl_y /tile_size) * width
	drawRect(xpp,xpp+2,ypp,ypp+2,player_c)
end

--[[enableFloors: Enable floor rendering]]--
function RayCast3D.enableFloor(val)
	floors = val
	ResetRenderer()
end

--[[enableSky: Enable sky rendering]]--
function RayCast3D.enableSky(val)
	sky = val
	ResetRenderer()
end

--[[spawnPlayer: Spawn player on the map]]--
function RayCast3D.spawnPlayer(x, y, angle)
	pl_x = x
	pl_y = y
	pl_angle = floor_num(rad2arc(deg2rad(angle)))
	ycenter = vheight >> 1
end

--[[getPlayer: Gets player status]]--
function RayCast3D.getPlayer()
	return {["x"] = pl_x, ["y"] = pl_y, ["angle"] = rad2deg(arc2rad(pl_angle))}
end

--[[movePlayer: Moves player]]--
function RayCast3D.movePlayer(dir, speed)
	xmov = ceil_num((costable[pl_angle] * speed) - .5)
	ymov = ceil_num((sintable[pl_angle] * speed) - .5)
	old_x = pl_x
	old_y = pl_y
	if dir == FORWARD then
		pl_x = pl_x + xmov
		pl_y = pl_y + ymov
	elseif dir == BACK then
		pl_x = pl_x - xmov
		pl_y = pl_y - ymov
	elseif dir == LEFT then
		pl_x = pl_x + ymov
		pl_y = pl_y - xmov
	elseif dir == RIGHT then
		pl_x = pl_x - ymov
		pl_y = pl_y + xmov
	end
	if noclip then
		return
	end
	ytmp = pl_y >> tile_shift
	xtmp = pl_x >> tile_shift
	new_cell = 1 + (xtmp) + (ytmp * map_width)
	if map[new_cell] ~= 0 then
		old2_x = pl_x
		old2_y = pl_y
		ydiff = (old_y >> tile_shift)
		ydiff2 = ydiff - ytmp
		xdiff = (old_x >> tile_shift)
		xdiff2 = xdiff - xtmp
		if  map[1 + (xdiff) + (ytmp * map_width)] ~= 0 then
			if ydiff2 > 0 then
				pl_y = (ytmp << tile_shift) + (tile_size + 1)
			elseif ydiff2 < 0 then
				pl_y = (ytmp << tile_shift) - 1
			end
		end
		xdiff = (old_x >> tile_shift)
		xdiff2 = xdiff - xtmp
		if map[1 + (xtmp) + (ydiff * map_width)] ~= 0 then
			if xdiff2 > 0 then
				pl_x = (xtmp << tile_shift) + (tile_size + 1)
			elseif xdiff2 < 0 then
				pl_x = (xtmp << tile_shift) - 1
			end
		end
		if old2_x == pl_x and old2_y == pl_y then
			pl_x = old_x
			pl_y = old_y
		end
	end
end

--[[rotateCamera: Rotates camera]]--
function RayCast3D.rotateCamera(dir, speed)
	if dir == LEFT then
		pl_angle = pl_angle - speed
		if pl_angle < ANGLE0 then
			pl_angle = floor_num(pl_angle + ANGLE360)
		end
	elseif dir == RIGHT then
		pl_angle = pl_angle + speed
		if pl_angle >= ANGLE360 then
			pl_angle = floor_num(pl_angle - ANGLE360)
		end
	elseif dir == FORWARD then
		ycenter = ycenter - (speed >> 2)
		if ycenter < 0 then
			ycenter = 0
		end
	elseif dir == BACK then
		ycenter = ycenter + (speed >> 2)
		if ycenter > vheight then
			ycenter = vheight
		end
	end
end

--[[loadMap: Loads a map in the engine]]--
function RayCast3D.loadMap(map_table, m_width, m_height, t_size, w_height)
	wall_height = w_height
	if t_size ~= tile_size then
		tile_size = t_size
		tmp = 2
		i = 1
		while tmp < tile_size do
			tmp = tmp << 1
			i = i + 1
		end
		if tmp ~= tile_size then
			error("Map tile-size must be 2^n pixels.")
		end
		tile_shift = i
		ResetAngles()
	end
	map_width = m_width
	map_height = m_height
	map = map_table
end

--[[setAccuracy: Sets renderer accuracy]]--
function RayCast3D.setAccuracy(val)
	accuracy = val
end

--[[setFloorColor: Sets floor color]]--
function RayCast3D.setFloorColor(val)
	floor_c = val
end

--[[setSkyColor: Sets sky color]]--
function RayCast3D.setSkyColor(val)
	sky_c = val
end

--[[setWallColor: Sets wall color]]--
function RayCast3D.setWallColor(val)
	wall_c = val
end

--[[setSkyColor: Sets player color]]--
function RayCast3D.setPlayerColor(val)
	player_c = val
end

--[[noClipMode: Sets noClip mode status]]--
function RayCast3D.noClipMode(val)
	noclip = val
end

--[[useShading: Sets Shading status]]--
function RayCast3D.useShading(val)
	shading = val
	ResetRenderer()
end

--[[setDepth: Sets Shading depth of field]]--
function RayCast3D.setDepth(val)
	shad_val = val
end

--[[setDepth: Sets Shading color]]--
function RayCast3D.setShadingColor(r, g, b)
	shad_r = r
	shad_g = g
	shad_b = b
end

--[[shoot: Shoot a ray and returns cell x,y values of first wall]]--
function RayCast3D.shoot(x, y, angle)
	local castArc = floor_num(rad2arc(deg2rad(angle)))
	local hgrid
	local vgrid
	local xtmp
	local xinter
	local yinter
	local ytmp
	local dist_next_hgrid
	local dist_next_vgrid
	local dist_next_xinter
	local dist_next_yinter
	local dist_hgrid_hit
	local dist_vgrid_hit
	local cell_idx
	local cell_idx_x
	local cell_idx_y
	if (castArc > ANGLE0 and castArc < ANGLE180) then
		hgrid = ((y >> tile_shift) << tile_shift) + tile_size
		dist_next_hgrid = tile_size
		xtmp = tantable2[castArc]*(hgrid-y)
		xinter = xtmp + x
	else
		hgrid = ((y >> tile_shift) << tile_shift)
		dist_next_hgrid = -tile_size
		xtmp = tantable2[castArc]*(hgrid-y)
		xinter = xtmp + x
		hgrid = hgrid - 1
	end
	if (castArc == ANGLE0 or castArc == ANGLE180) then -- Prevent asymptotics values
		dist_hgrid_hit = 99999
	else
		dist_next_xinter = xsteptable[castArc]
		while true do
			xgrid_index = floor_num(xinter) >> tile_shift
			ygrid_index = hgrid >> tile_shift
			cell_idx_x = ygrid_index*map_width+xgrid_index+1
			if (xgrid_index >= map_width or ygrid_index >= map_height or xgrid_index < 0 or ygrid_index < 0) then
				dist_hgrid_hit = 9999
				break
			elseif (map[cell_idx_x] ~= 0) then
				dist_hgrid_hit = (xinter - x) * costable2[castArc]
				break
			else
				xinter = xinter + dist_next_xinter
				hgrid = hgrid + dist_next_hgrid
			end
		end
		xx = xgrid_index
		xy = ygrid_index
	end
	if castArc < ANGLE90 or castArc > ANGLE270 then
		vgrid = tile_size + ((x >> tile_shift) << tile_shift)
		dist_next_vgrid = tile_size
		ytmp = tantable[castArc]*(vgrid - x)
		yinter = ytmp + y
	else
		vgrid = (x >> tile_shift) << tile_shift
		dist_next_vgrid = 0 - tile_size
		ytmp = tantable[castArc]*(vgrid-x)
		yinter = ytmp + y
		vgrid = vgrid - 1
	end
	if (castArc == ANGLE90 or castArc == ANGLE270) then
		dist_vgrid_hit = 99999
	else
		dist_next_yinter = ysteptable[castArc]
		dist_vgrid_hit = 0
		while dist_vgrid_hit <= dist_hgrid_hit do
			xgrid_index = vgrid >> tile_shift
			ygrid_index = floor_num(yinter) >> tile_shift
			cell_idx_y = ygrid_index*map_width+xgrid_index+1
			if (xgrid_index >= map_width or ygrid_index >= map_height or xgrid_index < 0 or ygrid_index < 0) then
				dist_vgrid_hit = 9999
				break
			elseif (map[cell_idx_y] ~= 0) then
				dist_vgrid_hit = (yinter-y)*sintable2[castArc]
				break
			else
				yinter = yinter + dist_next_yinter
				vgrid = vgrid + dist_next_vgrid
			end
		end
		yx = xgrid_index
		yy = ygrid_index
	end
	if (dist_hgrid_hit < dist_vgrid_hit) then
		x = xx
		y = xy
	else
		x = yx
		y = yy
	end
	return {["x"] = x, ["y"] = y}
end