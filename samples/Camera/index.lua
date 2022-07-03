local white = Color.new(255,255,255)

-- Initializing outer camera at 640x480@30 
Camera.init(OUTER_CAM, VGA_RES, 30)

-- Main loop
while true do
	
	-- Getting current camera output
	scene = Camera.getOutput()
	
	-- Blend camera scene
	Graphics.initBlend()
	Screen.clear()
	Graphics.drawImage(160, 32, scene)
	Graphics.debugPrint(5, 5, "Press TRIANGLE to return to the sample selector.",white)
	Graphics.termBlend()
	
	-- Check for input
	if Controls.check(Controls.read(), SCE_CTRL_TRIANGLE) then
		Camera.term()
		break
	end
	
	-- Flip screen
	Screen.flip()
	
end