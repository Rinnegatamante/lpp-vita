-- Load an image
image = Graphics.loadImage("ux0:/image.jpg")

-- Main loop
while true do
	
	-- Blend some images with different funcs (normal, rotated, scaled)
	Graphics.initBlend()
	Screen.clear()
	Graphics.drawImage(10, 10, image)
	Graphics.drawRotateImage(200, 10, 2.3, image)
	Graphics.drawScaleImage(100, 200, 1.5, 1.5, image)
	Graphics.termBlend()
	
	-- Check for input
	if Controls.check(Controls.read(), SCE_CTRL_TRIANGLE) then
		System.exit()
	end
	
	-- Flip screen
	Screen.flip()
	
end