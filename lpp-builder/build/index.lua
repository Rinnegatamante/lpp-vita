-- Loading RayCast3D Engine
dofile("app0:/files/raycast3d.lua")

-- Initializing everything
dofile("app0:/files/primm.lua") -- Primm's Algorithm
Sound.init() -- Sound Module
ttf = Font.load("app0:/files/main.ttf") -- Font

-- Menu Stuffs
credits_voices = {
	"yDevelopers:",
	"Rinnegatamante",
	" ","yImages:",
	"Misledz",
	" ","ySprites:",
	"ETTiNGRiNDER",	
	" ","yMusics:",
	"aliaspharow",
	" ","ySounds:",
	"soundjay.com",
	" ","yVoices:",
	"oddcast.com",
	" ","yTesters:",
	"Rinnegatamante", "ihaveamac",
	" ","yEngines:",
	"Lua Player Plus 3DS", "RayCast3D Engine"
}
voices = {"Start Game", "Map Size : Small", "Start Game with Seed", "stubb", "BGM Musics : On", "Sounds Effects : On", "Credits", "Exit Game"}
sizes = {"Smallest", "Small", "Medium", "Big", "Huge", "Titanic", "Monstrous"}
accs = {"Best", "Very High", "High", "Medium", "Low", "Very Low", "Worst"}
tsizes = {15, 25, 40, 60, 80, 100, 150}
scrolls_num = {0, 0, 1, 2, 4, 6, 8}
idx_voices = 1
idx_sizes = 2
gyrosens = 1
version = "v. 1.2"

-- Map Details
wall_height = 64
tile_size = 64

-- Player Speed
pl_speed = 5
cam_speed = 50

-- Showing intro
skip_logo = false
if Controls.check(Controls.read(), SCE_CTRL_LTRIGGER) then
	skip_logo = true
end
if not skip_logo then
	lpp = Graphics.loadImage("app0:/files/lpp.png")
	rc = Graphics.loadImage("app0:/files/rc.png")
	rcs = Sound.openOgg("app0:/files/rc.ogg")
	lpps = Sound.openOgg("app0:/files/lpp.ogg")
	ltmr = Timer.new()
	a = 0
	while true do
		if a > 255 then
			a = 255
			if stp == nil then
				stp = true
				Timer.reset(ltmr)
			end
		elseif stp == nil then
			a = math.floor(Timer.getTime(ltmr) / 10)
		else
			if stp then
				if plyd == nil then
					Sound.play(lpps, NO_LOOP)
					plyd = true
				end
				if Timer.getTime(ltmr) > 2000 then
					stp = false
					Timer.reset(ltmr)
				end
			elseif a > 0 then
				a = 255 - math.floor(Timer.getTime(ltmr) / 10)
			else
				break
			end
		end
		Graphics.initBlend()
		Graphics.fillRect(0,960,0,544,Color.new(255,255,255,math.min(a, 255)))
		Graphics.drawImage(330,172,lpp,Color.new(255,255,255,math.min(a, 255)))
		Graphics.termBlend()
		Screen.flip()
	end
	Timer.reset(ltmr)
	a = 0
	stp = nil
	plyd = nil
	while true do
		if a > 255 then
			a = 255
			if stp == nil then
				stp = true
				Timer.reset(ltmr)
			end
		elseif stp == nil then
			a = math.floor(Timer.getTime(ltmr) / 10)
		else
			if stp then
				if plyd == nil then
					Sound.play(rcs, NO_LOOP)
					plyd = true
				end
				if Timer.getTime(ltmr) > 2000 then
					stp = false
					Timer.reset(ltmr)
				end
			elseif a > 0 then
				a = 255 - math.floor(Timer.getTime(ltmr) / 10)
			else
				break
			end
		end
		Graphics.initBlend()
		Graphics.fillRect(0,960,0,544,Color.new(0,0,0))
		Graphics.drawPartialImage(330, 152, 150, 10, 360, 240, rc, Color.new(255,255,255,math.min(a, 255)))
		Graphics.termBlend()
		Screen.flip()
	end
	Sound.close(rcs)
	Sound.close(lpps)
	Graphics.freeImage(lpp)
	Graphics.freeImage(rc)
	Timer.destroy(ltmr)
end

Graphics.initBlend()
Font.setPixelSizes(ttf, 40)
Font.print(ttf, 300,252, "Loading, please wait...", Color.new(255,255,255))
Font.setPixelSizes(ttf, 28)
Graphics.termBlend()
Screen.flip()
Graphics.initBlend()
Font.setPixelSizes(ttf, 40)
Font.print(ttf, 300,252, "Loading, please wait...", Color.new(255,255,255))
Font.setPixelSizes(ttf, 28)
Graphics.termBlend()
Screen.flip()
Graphics.initBlend()
Font.setPixelSizes(ttf, 40)
Font.print(ttf, 300,252, "Loading, please wait...", Color.new(255,255,255))
Font.setPixelSizes(ttf, 28)
Graphics.termBlend()
Screen.flip()

-- Loading textures
W = Graphics.loadImage("app0:/files/wall.png")
M = Graphics.loadImage("app0:/files/marked.png")
anim = Graphics.loadImage("app0:/files/player.png")
logo = Graphics.loadImage("app0:/files/logo.png")

-- Loading sounds
walk = Sound.openOgg("app0:/files/footsteps.ogg")
step = Sound.openOgg("app0:/files/step.ogg")
bgm = Sound.openOgg("app0:/files/menu.ogg")
Sound.play(bgm, LOOP)
Sound.play(walk, LOOP)
Sound.pause(walk)

-- Main Menu Map Details
dmap = {
	W,W,W,W,W,W,
	W,0,0,0,W,W,
	W,0,W,0,0,W,
	W,0,W,W,0,W,
	W,0,0,W,0,W,
	W,W,0,0,0,W,
	W,W,W,W,W,W
}
dmap_width = 6
dmap_height = 7
state = 0

-- Setting up RayCast3D Engine
RayCast3D.setResolution(960, 544)
RayCast3D.setViewsize(60)
RayCast3D.loadMap(dmap, dmap_width, dmap_height, tile_size, wall_height)
RayCast3D.spawnPlayer(96, 96, 90)

-- Set accuracy depending on console
System.setCpuSpeed(804)
if System.getCpuSpeed() == 804 then
	RayCast3D.setAccuracy(1)
	norm_acc = 1
	three_acc = 2
else
	RayCast3D.setAccuracy(2)
	norm_acc = 2
	three_acc = 5
end
voices[4] = "Video Quality : " .. accs[norm_acc]
voices[5] = "Video Quality (3D Mode) : " .. accs[three_acc]

-- Set variables
is3D = false
main_menu = false
credits = false
seed_game = false
gyromode = false

-- Setting colors
ceil_c = Color.new(83, 69, 59, 255)
floor_c = Color.new(25, 17, 15, 255)
wall_c = Color.new(25, 17, 15, 0)
white = Color.new(255,255,255)
cyan = Color.new(0,255,255)
yellow = Color.new(255,255,0)
RayCast3D.enableFloor(false)
RayCast3D.setWallColor(wall_c)

-- Music & Sounds Management
function PlayMusic(file, loop)
	Sound.play(file, loop)
	if not isMusic then
		Sound.pause(file)
	end
end
function PlaySFX(file, loop)
	if isSFX then
		Sound.play(file, loop)
	end
end
isSFX = true
isMusic = true

-- Drawing main menu
function DrawMenu()
	i = 1
	Font.setPixelSizes(ttf,40)
	Font.print(ttf, 8, 15, "LABYRINTH 3D", cyan)
	Font.setPixelSizes(ttf,28)
	while i <= #voices do
		if i == idx_voices then
			color = yellow
			x = 20
		else
			color = white
			x = 15
		end
		Font.print(ttf, x, 50 + 30 * i, voices[i], color)
		i = i + 1
	end
end

-- Drawing credits
function DrawCredits()
	tm = Timer.getTime(tmr)
	for i, voice in pairs(credits_voices) do
		if string.sub(voice,1,1) == "y" then
			v = string.sub(voice,2,-1)
			color = yellow
		else
			v = voice
			color = white
		end
		y = 540 + 30 * i - math.floor(tm / 15)
		if y >= 0 and y <= 600 then
			Font.print(ttf, 15, y, v, color)
		end
	end
	if tm >= 12000 then
		credits = false
	end
end

-- Seed Keyboard
seed_i = 1
seed_table = {0}
function DrawSeedKeyboard()
	res = ""
	res2 = ""
	res3 = ""
	Font.print(ttf, 8, 15, "SEED GAME", cyan)
	for i, n in pairs(seed_table) do
		res = res .. n
		if i <= seed_i then
			res2 = res2 .. n
			if i < seed_i then
				res3 = res3 .. n
			end
		end
	end
	Font.print(ttf, 15, 60, "Seed: " .. res, white)
	Font.print(ttf, 15, 60, "Seed: " .. res2, yellow)
	Font.print(ttf, 15, 60, "Seed: " .. res3, white)
end

-- HIDDEN ZONE (SSSSHHH!)
-- Hidden Easter Egg (Why not?)
easter_egg = false
set_egg = false
konami_code = {SCE_CTRL_UP, SCE_CTRL_UP, SCE_CTRL_DOWN, SCE_CTRL_DOWN, SCE_CTRL_LEFT, SCE_CTRL_RIGHT, SCE_CTRL_LEFT, SCE_CTRL_RIGHT}
konami_idx = 1
konami_tmr = Timer.new()
-- Cheats (No ActionReplay needed!)
cheat1 = {SCE_CTRL_CROSS, SCE_CTRL_UP, SCE_CTRL_CROSS, SCE_CTRL_DOWN, SCE_CTRL_CIRCLE, SCE_CTRL_LEFT, SCE_CTRL_CIRCLE, SCE_CTRL_RIGHT} -- x99 Scrolls
cheat2 = {SCE_CTRL_UP, SCE_CTRL_DOWN, SCE_CTRL_LEFT, SCE_CTRL_RIGHT, SCE_CTRL_UP, SCE_CTRL_DOWN, SCE_CTRL_LEFT, SCE_CTRL_RIGHT, SCE_CTRL_CIRCLE, SCE_CTRL_CROSS} -- NoClip Mode
cheat1_idx = 1
cheat2_idx = 1
noclip = false

-- Game Timer
tmr = Timer.new()

sec = Timer.new()
frame = 0
fps = 0
while true do
	
	-- Rendering scene
	Graphics.initBlend()
	Graphics.fillRect(0,960,0,544,ceil_c) -- Simulates skybox; currently faster then sky rendering
	Graphics.fillRect(0,960,273,544,floor_c) -- Simulates floor; currently faster then floor rendering
	RayCast3D.renderScene(0,0)
	if not main_menu then
		Graphics.drawImage(320,152,logo)
	end
	
	-- Background animation
	if not easter_egg and not set_egg then
		if state == 0 then
			RayCast3D.movePlayer(FORWARD, pl_speed)
			cam = RayCast3D.getPlayer()
			if cam.y >= 272 then
				state = 1
			end
		elseif state == 1 then
			RayCast3D.rotateCamera(LEFT, cam_speed)
			cam = RayCast3D.getPlayer()
			if cam.angle <= 35 then
				state = 2
			end
		elseif state == 2 then
			RayCast3D.movePlayer(FORWARD, pl_speed)
			cam = RayCast3D.getPlayer()
			if cam.y >= 338 then
				state = 3
			end
		elseif state == 3 then
			RayCast3D.rotateCamera(LEFT, cam_speed)
			RayCast3D.movePlayer(FORWARD, pl_speed)
			RayCast3D.movePlayer(RIGHT, pl_speed)
			cam = RayCast3D.getPlayer()
			if cam.angle >= 200 and cam.angle <= 320 then
				state = 4
			end
		elseif state == 4 then
			RayCast3D.movePlayer(FORWARD, pl_speed)
			cam = RayCast3D.getPlayer()
			if cam.y <= 320 then
				state = 5
			end
		elseif state == 5 then
			RayCast3D.rotateCamera(LEFT, cam_speed)
			cam = RayCast3D.getPlayer()
			if cam.angle < 270 then
				state = 6
			end
		elseif state == 6 then
			RayCast3D.movePlayer(FORWARD, pl_speed)
			cam = RayCast3D.getPlayer()
			if cam.y <= 200 then
				state = 7
			end
		elseif state == 7 then
			RayCast3D.rotateCamera(LEFT, cam_speed)
			cam = RayCast3D.getPlayer()
			if cam.angle < 230 then
				state = 8
			end
		elseif state == 8 then
			RayCast3D.movePlayer(FORWARD, pl_speed)
			cam = RayCast3D.getPlayer()
			if cam.y <= 100 then
				state = 9
			end
		elseif state == 9 then
			RayCast3D.rotateCamera(LEFT, cam_speed)
			cam = RayCast3D.getPlayer()
			if cam.angle <= 180 then
				state = 10
			end
		elseif state == 10 then
			RayCast3D.movePlayer(FORWARD, pl_speed)
			cam = RayCast3D.getPlayer()
			if cam.x <= 100 then
				state = 11
			end
		elseif state == 11 then
			RayCast3D.rotateCamera(LEFT, cam_speed)
			cam = RayCast3D.getPlayer()
			if cam.angle <= 90 then
				state = 0
			end
		end
	end
	
	-- Main Menu
	pad = Controls.read()
	if not main_menu then
		if Controls.check(pad, cheat1[cheat1_idx]) and not Controls.check(oldpad, cheat1[cheat1_idx]) then
			cheat1_idx = cheat1_idx + 1
			Timer.reset(konami_tmr)
			if cheat1_idx > #cheat1 then
				scrolls_num = {99, 99, 99, 99, 99, 99, 99}
				Timer.reset(konami_tmr)
				PlaySFX(step, NO_LOOP)
				cheat1_idx = 1
			end
		end
		if Controls.check(pad, cheat2[cheat2_idx]) and not Controls.check(oldpad, cheat2[cheat2_idx]) then
			cheat2_idx = cheat2_idx + 1
			Timer.reset(konami_tmr)
			if cheat2_idx > #cheat2 then
				PlaySFX(step, NO_LOOP)
				noclip = true
				Timer.reset(konami_tmr)
				cheat2_idx = 1
			end
		end
		if Timer.getTime(konami_tmr) > 800 then
			cheat1_idx = 1
			cheat2_idx = 1
			Timer.reset(konami_tmr)
		end
		if Timer.getTime(tmr) > 1000 then
			if Timer.getTime(tmr) > 1600 then
				Timer.reset(tmr)
			end
		else
			Font.print(ttf,430, 352,"Press Start",white)
		end
		Font.print(ttf,870, 502,version,white)
		if Controls.check(pad, SCE_CTRL_START) and not Controls.check(oldpad, SCE_CTRL_START) then
			PlaySFX(step, NO_LOOP)
			main_menu = true
		end
	else
		if credits then
			DrawCredits()
		elseif seed_game then
			DrawSeedKeyboard()
			if Controls.check(pad, SCE_CTRL_CIRCLE) and not Controls.check(oldpad, SCE_CTRL_CIRCLE) then
				PlaySFX(step, NO_LOOP)
				seed_game = false
			elseif Controls.check(pad, SCE_CTRL_TRIANGLE) and not Controls.check(oldpad, SCE_CTRL_TRIANGLE) then
				PlaySFX(step, NO_LOOP)
				if #seed_table > 1 then
					table.remove(seed_table, seed_i)
					if seed_i > #seed_table then
						seed_i = seed_i - 1
					end
				end
			elseif Controls.check(pad, SCE_CTRL_UP) and not Controls.check(oldpad, SCE_CTRL_UP) then
				PlaySFX(step, NO_LOOP)
				seed_table[seed_i] = seed_table[seed_i] + 1
				if seed_table[seed_i] > 9 then
					seed_table[seed_i] = 0
				end
			elseif Controls.check(pad, SCE_CTRL_DOWN) and not Controls.check(oldpad, SCE_CTRL_DOWN) then
				PlaySFX(step, NO_LOOP)
				seed_table[seed_i] = seed_table[seed_i] - 1
				if seed_table[seed_i] < 0 then
					seed_table[seed_i] = 9
				end
			elseif Controls.check(pad, SCE_CTRL_LEFT) and not Controls.check(oldpad, SCE_CTRL_LEFT) then
				PlaySFX(step, NO_LOOP)
				seed_i = seed_i - 1
				if seed_i < 1 then
					seed_i = 1
				end
			elseif Controls.check(pad, SCE_CTRL_RIGHT) and not Controls.check(oldpad, SCE_CTRL_RIGHT) then
				PlaySFX(step, NO_LOOP)
				seed_i = seed_i + 1
				if seed_i > #seed_table then
					table.insert(seed_table, 0)
				end
			elseif Controls.check(pad, SCE_CTRL_CROSS) and not Controls.check(oldpad, SCE_CTRL_CROSS) then
				PlaySFX(step, NO_LOOP)
				seed_game = false
				Sound.pause(bgm)
				Sound.close(bgm)
				game_seed = 0
				for i, n in pairs(seed_table) do
					game_seed = game_seed + n * (10^(#seed_table-i))
				end
				game_seed = math.floor(game_seed)
				PrimmBuild(tsizes[idx_sizes], tsizes[idx_sizes], game_seed)
				dofile("app0:/files/game.lua")
				RayCast3D.loadMap(dmap, dmap_width, dmap_height, tile_size, wall_height)
				RayCast3D.spawnPlayer(96, 96, 90)
				bgm = Sound.openOgg("app0:/files/menu.ogg",true)
				PlayMusic(bgm, LOOP)
				Timer.resume(tmr)
				state = 0
			end
		else
			if not easter_egg then
				if Controls.check(pad, konami_code[konami_idx]) and not Controls.check(oldpad, konami_code[konami_idx]) then
					konami_idx = konami_idx + 1
					Timer.reset(konami_tmr)
					if konami_idx > #konami_code then
						easter_egg = true
					end
				elseif Timer.getTime(konami_tmr) > 800 then
					konami_idx = 1
					Timer.reset(konami_tmr)
				end
				DrawMenu()
				if Controls.check(pad, SCE_CTRL_CROSS) and not Controls.check(oldpad, SCE_CTRL_CROSS) then
					PlaySFX(step, NO_LOOP)
					if idx_voices == 1 then
						Sound.pause(bgm)
						Sound.close(bgm)
						PrimmBuild(tsizes[idx_sizes], tsizes[idx_sizes], nil)
						dofile("app0:/files/game.lua")
						RayCast3D.loadMap(dmap, dmap_width, dmap_height, tile_size, wall_height)
						RayCast3D.spawnPlayer(96, 96, 90)
						bgm = Sound.openOgg("app0:/files/menu.ogg",true)
						PlayMusic(bgm, LOOP)
						Timer.resume(tmr)
						state = 0
					elseif idx_voices == 2 then
						idx_sizes = idx_sizes + 1
						if idx_sizes > #sizes then
							idx_sizes = 1
						end
						voices[2] = "Map Size : "..sizes[idx_sizes]
					elseif idx_voices == 3 then
						seed_game = true
					elseif idx_voices == 4 then
						norm_acc = norm_acc + 1
						if norm_acc > #accs then
							norm_acc = 1
						end
						voices[4] = "Video Quality : "..accs[norm_acc]
						if not is3D then
							RayCast3D.setAccuracy(norm_acc)
						end
					elseif idx_voices == 5 then
						isMusic = not isMusic
						if isMusic then
							voices[6] = "BGM Music: On"
							Sound.resume(bgm)
						else
							voices[6] = "BGM Music: Off"
							Sound.pause(bgm)
						end
					elseif idx_voices == 6 then
						isSFX = not isSFX
						if isSFX then
							voices[7] = "Sounds Effects: On"
						else
							voices[7] = "Sounds Effects: Off"
						end
					elseif idx_voices == 7 then
						Timer.reset(tmr)
						credits = true
					elseif idx_voices == 8 then
					
						-- FIXME: Add Gyroscope support
						if gyromode then
							
						end
						
						Sound.pause(bgm)
						Sound.close(bgm)
						Graphics.freeImage(W)
						Graphics.freeImage(logo)
						Graphics.freeImage(anim)
						Timer.destroy(tmr)
						Timer.destroy(konami_tmr)
						Timer.destroy(sec)
						Sound.close(step)
						Sound.close(walk)
						Sound.term()
						Font.unload(ttf)
						System.exit()
					end
				elseif Controls.check(pad, SCE_CTRL_UP) and not Controls.check(oldpad, SCE_CTRL_UP) then
					idx_voices = idx_voices - 1
					if idx_voices < 1 then
						idx_voices = #voices
					end
					PlaySFX(step, NO_LOOP)
				elseif Controls.check(pad, SCE_CTRL_DOWN) and not Controls.check(oldpad, SCE_CTRL_DOWN) then
					idx_voices = idx_voices + 1
					if idx_voices > #voices then
						idx_voices = 1
					end
					PlaySFX(step, NO_LOOP)
				elseif Controls.check(pad, SCE_CTRL_CIRCLE) and not Controls.check(oldpad, SCE_CTRL_CIRCLE) then
					PlaySFX(step, NO_LOOP)
					main_menu = false
				end
			else
				if Controls.check(pad, KEY_START) then
					easter_egg = false
					set_egg = true
					konami_idx = 1
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
				if Controls.check(pad, SCE_CTRL_LEFT) then
					RayCast3D.rotateCamera(LEFT, cam_speed)
				end
				if Controls.check(pad, SCE_CTRL_RIGHT) then
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
			end
		end
	end
	
	Graphics.termBlend()
	oldpad = pad
	Screen.flip()
end