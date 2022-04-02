-- Create a cube
model = {
 
	-- First face (PZ)
	-- First triangle
	Render.createVertex(-0.5, -0.5, 0.5, 0.0, 0.0, 0.0, 0.0, 1.0),
	Render.createVertex(0.5, -0.5, 0.5, 1.0, 0.0, 0.0, 0.0, 1.0),
	Render.createVertex(0.5, 0.5, 0.5, 1.0, 1.0, 0.0, 0.0, 1.0),
	-- Second triangle
	Render.createVertex(0.5, 0.5, 0.5, 1.0, 1.0, 0.0, 0.0, 1.0),
	Render.createVertex(-0.5, 0.5, 0.5, 0.0, 1.0, 0.0, 0.0, 1.0),
	Render.createVertex(-0.5, -0.5, 0.5, 0.0, 0.0, 0.0, 0.0, 1.0),
 
	-- Second face (MZ)
	-- First triangle
	Render.createVertex(-0.5, -0.5, -0.5, 0.0, 0.0, 0.0, 0.0, -1.0),
	Render.createVertex(-0.5, 0.5, -0.5, 1.0, 0.0, 0.0, 0.0, -1.0),
	Render.createVertex(0.5, 0.5, -0.5, 1.0, 1.0, 0.0, 0.0, -1.0),
	-- Second triangle
	Render.createVertex(0.5, 0.5, -0.5, 1.0, 1.0, 0.0, 0.0, -1.0),
	Render.createVertex(0.5, -0.5, -0.5, 0.0, 1.0, 0.0, 0.0, -1.0),
	Render.createVertex(-0.5, -0.5, -0.5, 0.0, 0.0, 0.0, 0.0, -1.0),
 
	-- Third face (PX)
	-- First triangle
	Render.createVertex(0.5, -0.5, -0.5, 0.0, 0.0, 1.0, 0.0, 0.0),
	Render.createVertex(0.5, 0.5, -0.5, 1.0, 0.0, 1.0, 0.0, 0.0),
	Render.createVertex(0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.0, 0.0),
	-- Second triangle
	Render.createVertex(0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.0, 0.0),
	Render.createVertex(0.5, -0.5, 0.5, 0.0, 1.0, 1.0, 0.0, 0.0),
	Render.createVertex(0.5, -0.5, -0.5, 0.0, 0.0, 1.0, 0.0, 0.0),
 
	-- Fourth face (MX)
	-- First triangle
	Render.createVertex(-0.5, -0.5, -0.5, 0.0, 0.0, -1.0, 0.0, 0.0),
	Render.createVertex(-0.5, -0.5, 0.5, 1.0, 0.0, -1.0, 0.0, 0.0),
	Render.createVertex(-0.5, 0.5, 0.5, 1.0, 1.0, -1.0, 0.0, 0.0),
	-- Second triangle
	Render.createVertex(-0.5, 0.5, 0.5, 1.0, 1.0, -1.0, 0.0, 0.0),
	Render.createVertex(-0.5, 0.5, -0.5, 0.0, 1.0, -1.0, 0.0, 0.0),
	Render.createVertex(-0.5, -0.5, -0.5, 0.0, 0.0, -1.0, 0.0, 0.0),
 
	-- Fifth face (PY)
	-- First triangle
	Render.createVertex(-0.5, 0.5, -0.5, 0.0, 0.0, 0.0, 1.0, 0.0),
	Render.createVertex(-0.5, 0.5, 0.5, 1.0, 0.0, 0.0, 1.0, 0.0),
	Render.createVertex(0.5, 0.5, 0.5, 1.0, 1.0, 0.0, 1.0, 0.0),
	-- Second triangle
	Render.createVertex(0.5, 0.5, 0.5, 1.0, 1.0, 0.0, 1.0, 0.0),
	Render.createVertex(0.5, 0.5, -0.5, 0.0, 1.0, 0.0, 1.0, 0.0),
	Render.createVertex(-0.5, 0.5, -0.5, 0.0, 0.0, 0.0, 1.0, 0.0),
 
	-- Sixth face (MY)
	-- First triangle
	Render.createVertex(-0.5, -0.5, -0.5, 0.0, 0.0, 0.0, -1.0, 0.0),
	Render.createVertex(0.5, -0.5, -0.5, 1.0, 0.0, 0.0, -1.0, 0.0),
	Render.createVertex(0.5, -0.5, 0.5, 1.0, 1.0, 0.0, -1.0, 0.0),
	-- Second triangle
	Render.createVertex(0.5, -0.5, 0.5, 1.0, 1.0, 0.0, -1.0, 0.0),
	Render.createVertex(-0.5, -0.5, 0.5, 0.0, 1.0, 0.0, -1.0, 0.0),
	Render.createVertex(-0.5, -0.5, -0.5, 0.0, 0.0, 0.0, -1.0, 0.0)
   
}

-- Load image
texture = Graphics.loadImage("ux0:/data/lpp-vita/samples/3D Cube/texture.png");
-- Load the cube
mod1 = Render.loadModel(model, texture)

-- Set default angle, position and translation values
z = -6.0
x = 0.0
y = 0.0
angleX = 1.0
angleY = 1.0
angleZ = 1.0;
translX = 0.0017
translY = 0.0034

-- Main loop
while true do
 
	-- Rotate the model
	angleX = angleX + 0.2
	angleY = angleY + 0.2
	angleZ = angleZ + 0.2
	
	-- Move the model
	x = x + translX
	y = y + translY
	
	-- Blend the model with info on screen
	Graphics.initBlend()
	Screen.clear()
	Render.drawModel(mod1, math.sin(x), math.cos(y), z, angleX, angleY, angleZ);
	Graphics.debugPrint(3, 3, "Lua Player Plus Vita - Render Module Sample - Cube Test", Color.new(255,255,255))
	Graphics.debugPrint(3, 23, "Press TRIANGLE to return to the main menu.", Color.new(255,255,255))
	Graphics.termBlend()
	Screen.flip()
	
	-- Exit sample
	if Controls.check(Controls.read(), SCE_CTRL_TRIANGLE) then
		Render.unloadModel(mod1)
		for i, vertex in pairs(model) do
			Render.destroyVertex(vertex)
		end
		break
	end
   
end
