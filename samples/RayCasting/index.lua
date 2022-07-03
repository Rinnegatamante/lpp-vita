-------------------------------------------------
-- Ray-Casting Algorithm Tech-Demo for lpp-vita --
-------------------------------------------------

-- Load RayCast3D Engine
dofile("ux0:/data/lpp-vita/samples/RayCasting/raycast3d.lua")

-- Map Details
wall_height = 64
tile_size = 64
map_width = 15
map_height = 6
acc = 3
W = Graphics.loadImage("ux0:/data/lpp-vita/samples/RayCasting/wall.png")
I = Graphics.loadImage("ux0:/data/lpp-vita/samples/RayCasting/wall2.png")
map = {
	W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,
	W,0,W,0,0,0,W,0,0,W,I,I,I,I,I,
	W,0,W,W,W,0,0,0,0,W,I,0,0,0,I,
	W,0,0,0,0,0,0,0,0,0,0,0,0,0,I,
	W,0,0,0,W,0,0,0,0,W,I,0,0,0,I,
	W,W,W,W,W,W,W,W,W,W,I,I,I,I,I
}

-- Player Speed
pl_speed = 5
cam_speed = 50

-- Setting up RayCast3D Engine
RayCast3D.setResolution(960, 544)
RayCast3D.setViewsize(60)
RayCast3D.loadMap(map, map_width, map_height, tile_size, wall_height)
RayCast3D.spawnPlayer(80, 80, 90)

-- Set accuracy depending on console
System.setCpuSpeed(444)
RayCast3D.setAccuracy(acc)

-- Set used variables
local instructions = true
local oldpad = SCE_CTRL_CROSS

-- Using an image as skybox
RayCast3D.enableFloor(false)
RayCast3D.setFloorColor(Color.new(29,14,7))
skybox = Graphics.loadImage("ux0:/data/lpp-vita/samples/RayCasting/skybox.png")

-- Enabling shading and using it
RayCast3D.useShading(true)
RayCast3D.setDepth(300)

local pad = 0

while true do
	
	-- Rendering scene
	Graphics.initBlend()
	Screen.clear()
	Graphics.drawImage(0,0,skybox)
	RayCast3D.renderScene(0,0)
	if Controls.check(pad, SCE_CTRL_CIRCLE) then
		RayCast3D.renderMap(0, 100, 64)
	end
	if instructions then
		Graphics.fillRect(10, 950, 200, 380, Color.new(0,0,0))
		Graphics.fillRect(11, 949, 201, 379, Color.new(50,50,50))
		Graphics.debugPrint(15, 205, "Welcome to RayCast3D Engine sample demo!", Color.new(255, 255, 0))
		Graphics.debugPrint(15, 230, "DPAD = Move Player", Color.new(255, 255, 255))
		Graphics.debugPrint(15, 250, "LTRIGGER = Strafe left", Color.new(255, 255, 255))
		Graphics.debugPrint(15, 270, "RTRIGGER = Strafe right", Color.new(255, 255, 255))
		Graphics.debugPrint(15, 290, "SELECT = Change accuracy", Color.new(255, 255, 255))
		Graphics.debugPrint(15, 310, "CROSS = Open/Close this box", Color.new(255, 255, 255))
		Graphics.debugPrint(15, 330, "CIRCLE = Show 2D minimap", Color.new(255, 255, 255))
		Graphics.debugPrint(15, 350, "TRIANGLE = Exit", Color.new(255, 255, 255))
	end
	Graphics.termBlend()
	
	-- Player and camera movements
	pad = Controls.read()
	if Controls.check(pad, SCE_CTRL_LEFT) then
		RayCast3D.rotateCamera(LEFT, cam_speed)
	end
	if Controls.check(pad, SCE_CTRL_RIGHT) then
		RayCast3D.rotateCamera(RIGHT, cam_speed)
	end
	if Controls.check(pad, SCE_CTRL_UP) then
		RayCast3D.movePlayer(FORWARD, pl_speed)
	end
	if Controls.check(pad, SCE_CTRL_TRIANGLE) then
		Graphics.freeImage(W)
		Graphics.freeImage(I)
		Graphics.freeImage(skybox)
		break
	end
	if Controls.check(pad, SCE_CTRL_CROSS) and not Controls.check(oldpad, SCE_CTRL_CROSS) then
		instructions = not instructions
	end
	if Controls.check(pad, SCE_CTRL_DOWN) then
		RayCast3D.movePlayer(BACK, pl_speed)
	end
	if Controls.check(pad, SCE_CTRL_LTRIGGER) then
		RayCast3D.movePlayer(LEFT, pl_speed)
	end
	if Controls.check(pad, SCE_CTRL_RTRIGGER) then
		RayCast3D.movePlayer(RIGHT, pl_speed)
	end
	if Controls.check(pad, SCE_CTRL_SELECT) and not Controls.check(oldpad, SCE_CTRL_SELECT) then
	
		-- Change accuracy 
		acc=acc+1
		if acc > 8 then
			acc = 1
		end
		RayCast3D.setAccuracy(acc)
		
	end
	
	oldpad = pad
	
	Screen.flip()

end