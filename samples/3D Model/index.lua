-- Setting PSVITA at maximum speed
System.setCpuSpeed(444)
System.setBusSpeed(222)
System.setGpuSpeed(222)
System.setGpuXbarSpeed(166)

-- Load image
texture = Graphics.loadImage("ux0:/data/lpp-vita/samples/3D Model/Monkey.bmp")
-- Load the model
mod1 = Render.loadObject("ux0:/data/lpp-vita/samples/3D Model/Monkey.obj", texture)

-- Set default angle, position and translation values
z = -20.0
x = 0.0
y = 0.0
angleX = 1.0
angleY = 1.0
angleZ = 1.0
translX = 0.0017
translY = 0.0034

-- Main loop
while true do
 
	-- Rotate the model
	angleX = angleX + 0.2
	angleY = angleY + 0.2
	angleZ = angleY + 0.2
	
	-- Move the model
	x = x + translX
	y = y + translY
	
	-- Blend the model with info on screen
	Graphics.initBlend()
	Screen.clear()
	Render.drawModel(mod1, math.sin(x), math.cos(y), z, angleX, angleY, angleZ)
	Graphics.debugPrint(3, 3, "Lua Player Plus Vita - Render Module Sample - 3D Model Test", Color.new(255,255,255))
	Graphics.debugPrint(3, 23, "Press TRIANGLE to return to the main menu.", Color.new(255,255,255))
	Graphics.termBlend()
	Screen.flip()
	
	-- Exit sample
	if Controls.check(Controls.read(), SCE_CTRL_TRIANGLE) then
		Render.unloadModel(mod1)
		break
	end
   
end
