image = Screen.loadImage("cache0:/image.jpg")
while true do
	Screen.initBlend()
	Screen.clear()
	Screen.drawImage(10, 10, image)
	Screen.drawRotateImage(200, 10, 2.3, image)
	Screen.drawScaleImage(100, 200, 1.5, 1.5, image)
	Screen.termBlend()
	if Controls.check(Controls.read(), SCE_CTRL_TRIANGLE) then
		System.exit()
	end
	Screen.flip()
end