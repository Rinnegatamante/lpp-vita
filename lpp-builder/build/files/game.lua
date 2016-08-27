local paused = false
local skip_end = false
local p_voices = {"Resume Game","Return Main Menu"}
local scrolls = scrolls_num[idx_sizes]
local p_idx = 1
bgm = Sound.openOgg("app0:/files/game.ogg")
spell = Sound.openOgg("app0:/files/spell.ogg")
Sound.play(spell, NO_LOOP)
Sound.pause(spell)
PlayMusic(bgm, LOOP)
Timer.reset(tmr)
Timer.resume(tmr)
RayCast3D.noClipMode(noclip)
local a_st = 0
local tcells = {}

function DrawPause()
	i = 1
	Font.setPixelSizes(ttf,40)
	Font.print(ttf, 8, 15, "PAUSED GAME", cyan)
	Font.setPixelSizes(ttf,28)
	Font.print(ttf, 8, 500, "Seed: " .. seed, yellow)
	while i <= #p_voices do
		if i == p_idx then
			color = yellow
			x = 20
		else
			color = white
			x = 15
		end
		Font.print(ttf, x, 50 + 30 * i, p_voices[i], color)
		i = i + 1
	end
end

Timer.reset(sec)
while true do
	
	-- Rendering scene
	Graphics.initBlend()
	Graphics.fillRect(0,960,0,544,ceil_c) -- Simulates skybox; currently faster then sky rendering
	Graphics.fillRect(0,960,272,544,floor_c) -- Simulates floor; currently faster then floor rendering
	RayCast3D.renderScene(0,0)
		
		-- Player Animation
	Graphics.drawPartialImage(284,440,0,104 * a_st,376,104,anim)
	
	-- Drawing Framerate
	frame = frame + 1
	if Timer.getTime(sec) >= 1000 then
		fps = frame
		frame = 0
		Timer.reset(sec)
	end
	Font.print(ttf, 870, 500, math.floor(fps), yellow)
	
	-- Main controls triggering
	pad = Controls.read()	
	if paused then
		DrawPause()
		if Controls.check(pad, SCE_CTRL_UP) and not Controls.check(oldpad, SCE_CTRL_UP) then
			PlaySFX(step, NO_LOOP)
			p_idx = p_idx - 1
			if p_idx < 1 then
				p_idx = #p_voices
			end
		elseif Controls.check(pad, SCE_CTRL_DOWN) and not Controls.check(oldpad, SCE_CTRL_DOWN) then
			PlaySFX(step, NO_LOOP)
			p_idx = p_idx + 1
			if p_idx > #p_voices then
				p_idx = 1
			end
		elseif Controls.check(pad, SCE_CTRL_CROSS) and not Controls.check(oldpad, SCE_CTRL_CROSS) then
			PlaySFX(step, NO_LOOP)
			if p_idx == 1 then
				paused = false
				Timer.resume(tmr)
				Sound.resume(bgm)
			elseif p_idx == 2 then
				skip_end = true
				break
			end
		end
	else
	
		-- FIXME: Add Gyroscope support
		if gyromode then
			
		end
		
		cmove = false
		cx, cy = Controls.readLeftAnalog()
		cx2 = Controls.readRightAnalog()
		if cx2 > 180 then
			RayCast3D.rotateCamera(RIGHT, cam_speed)
		elseif cx2 < 64 then
			RayCast3D.rotateCamera(LEFT, cam_speed)
		end
		if cx > 180 then
			RayCast3D.movePlayer(RIGHT, pl_speed)
			if isSFX then
				Sound.resume(walk)
			end
		elseif cx < 64 then
			RayCast3D.movePlayer(LEFT, pl_speed)
			if isSFX then
				Sound.resume(walk)
			end
		end
		if cy < 64 then
			RayCast3D.movePlayer(FORWARD, pl_speed)
			cmove = true
			if isSFX then
				Sound.resume(walk)
			end
		elseif cy > 180 then
			RayCast3D.movePlayer(BACK, pl_speed)
			cmove = true
			if isSFX then
				Sound.resume(walk)
			end
		end
		if Controls.check(pad,SCE_CTRL_LEFT) then
			RayCast3D.rotateCamera(LEFT, cam_speed)
		end
		if Controls.check(pad,SCE_CTRL_RIGHT) then
			RayCast3D.rotateCamera(RIGHT, cam_speed)
		end
		if Controls.check(pad,SCE_CTRL_UP) then
			RayCast3D.movePlayer(FORWARD, pl_speed)
			if isSFX then
				Sound.resume(walk)
			end
		elseif Controls.check(pad,SCE_CTRL_DOWN) then
			RayCast3D.movePlayer(BACK, pl_speed)
			if isSFX then
				Sound.resume(walk)
			end
		elseif not cmove then
			Sound.pause(walk)
		end
		
		-- Scroll Usage
		if Controls.check(pad, SCE_CTRL_CROSS) and not Controls.check(oldpad, SCE_CTRL_CROSS) and scrolls > 0 then
			pl = RayCast3D.getPlayer()
			cell = RayCast3D.shoot(pl.x, pl.y, pl.angle)
			c_idx = 1+cell.x+cell.y*wmap
			if map[c_idx] == W then
				scrolls = scrolls - 1
				table.insert(tcells, {["idx"] = c_idx,["st"] = 1})
				a_st = 1
				PlaySFX(spell, NO_LOOP)
			end
		end
		
		-- Player Animation
		if a_st > 0 then
			a_st = a_st + 1
			if a_st > 5 then
				a_st = 0
			end
		end
		
	end
	
	-- Tricky way to check if player won
	if Sound.isPlaying(walk) or not isSFX then
		player = RayCast3D.getPlayer()
		dx1 = (player.x + 5) >> 6
		dy1 = (player.y + 5) >> 6
		dx2 = (player.x - 5) >> 6
		dy2 = (player.y - 5) >> 6
		cx = player.x >> 6
		cy = player.y >> 6
		if dx1 - cx ~= 0 then
			cell = dx1 + cy * wmap + 1
			if map[cell] == 1 then
				Sound.pause(walk)
				break
			end
		end
		if dx2 - cx ~= 0 then
			cell = dx2 + cy * wmap + 1
			if map[cell] == 1 then
				Sound.pause(walk)
				break
			end
		end
		if dy1 - cy ~= 0 then
			cell = cx + dy1 * wmap + 1
			if map[cell] == 1 then
				Sound.pause(walk)
				break
			end
		end
		if dy2 - cy ~= 0 then
			cell = cx + dy2 * wmap + 1
			if map[cell] == 1 then
				Sound.pause(walk)
				break
			end
		end	
	end
	
	-- Drawing Seed
	tm = Timer.getTime(tmr)
	if tm < 5000 and not paused then
		Font.print(ttf, 5, 20, "Seed: " .. seed, Color.new(255,255,0))
	end
	
	-- Drawing Scrolls
	Font.print(ttf, 700, 20, "Scrolls: " .. scrolls, yellow)
	
	-- FIXME: Add Blocks Fade Animation
	i = 1
	while i <= #tcells do	
		cell = tcells[i]
		map[cell.idx] = M
		table.remove(tcells, i)
	end
	
	-- Pause Menu Triggering
	if Controls.check(pad,SCE_CTRL_START) and not Controls.check(oldpad, SCE_CTRL_START) then
		paused = not paused
		if paused then
			Sound.pause(walk)
			Sound.pause(bgm)
			Timer.pause(tmr)
		else
			if isMusic then
				Sound.resume(bgm)
			end
			Timer.resume(tmr)
		end
	end
	
	Graphics.termBlend()
	Screen.flip()
	oldpad = pad
	
end

-- Calculating time
Timer.pause(tmr)
raw = math.floor(Timer.getTime(tmr) / 1000)
h = math.floor(raw / 3600)
m = math.floor((raw - h * 3600) / 60)
s = raw % 60
if s < 10 then
	s = "0" .. s
end
if m < 10 then
	m = "0" .. m
end
if h < 10 then
	h = "0" .. h
end
res = h..":"..m..":"..s
 
while not skip_end do
	
	-- Rendering scene
	Graphics.initBlend()
	Graphics.fillRect(0,960,0,544,ceil_c) -- Simulates skybox; currently faster then sky rendering
	RayCast3D.renderScene(0,0)
		
	-- Player Animation
	Graphics.drawPartialImage(284,440,0,0,376,104,anim)
	
	-- Rendering Text
	Font.print(ttf,150, 150, "Your Time: "..res, white)
	Font.print(ttf,156, 180, "Seed: "..seed, white)
	Font.setPixelSizes(ttf,44)
	Font.print(ttf,100, 90, "YOU FOUND THE EXIT!", yellow)
	Font.setPixelSizes(ttf,24)
	
	Graphics.termBlend()
	
	pad = Controls.read()
	
	-- Return Main Menu
	if Controls.check(pad, SCE_CTRL_START) or Controls.check(pad, SCE_CTRL_CROSS) then
		oldpad = pad
		break
	end
	
	Screen.flip()	
	oldpad = pad
	
end

Sound.pause(bgm)
Sound.close(bgm)
Sound.close(spell)