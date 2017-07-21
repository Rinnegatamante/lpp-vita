local white = Color.new(255,255,255) 

-- Load an image
local image = Graphics.loadImage("ux0:/data/lpp-vita/samples/Image/image.jpg")

-- Main loop
while true do
	
	-- Blend some images with different funcs (normal, rotated, scaled)
	Graphics.initBlend()
	Screen.clear()
	Graphics.debugPrint(5, 500, "Press TRIANGLE to return to the sample selector.", white)
	Graphics.drawImage(10, 10, image)
	Graphics.drawRotateImage(200, 50, image, 2.3)
	Graphics.drawScaleImage(100, 200, image, 1.5, 1.5)
	Graphics.termBlend()
	
	-- Check for input
	if Controls.check(Controls.read(), SCE_CTRL_TRIANGLE) then
		Graphics.freeImage(image)
		break
	end
	
	-- Flip screen
	Screen.flip()
	
end