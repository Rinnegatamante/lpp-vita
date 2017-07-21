-- Initializing Network
Network.init()

-- Checking if connection is available
if Network.isWifiEnabled() then

	-- Download an image
	Network.downloadFile("http://rinnegatamante.it/site/images/logo.png", "ux0:/data/data.png")

	-- Loading image in memory and deleting it from storage
	img = Graphics.loadImage("ux0:/data/data.png")
	System.deleteFile("ux0:/data/data.png")

end

-- Terminating network
Network.term()

-- Main loop
while true do
	
	-- Drawing the downloaded image
	Graphics.initBlend()
	Screen.clear()
	if img == nil then
		Graphics.debugPrint(5,220,"You must enable WiFi to use this sample.", Color.new(255,255,255))
		Graphics.debugPrint(5,240,"Press TRIANGLE to return to the sample selector", Color.new(255,255,255))
	else
		Graphics.drawImage(200,265,img)
		Graphics.debugPrint(5,220,"This image came from http://rinnegatamante.it/site/images/logo.png", Color.new(255,255,255))
		Graphics.debugPrint(5,240,"Press TRIANGLE to return to the sample selector", Color.new(255,255,255))
	end
	Graphics.termBlend()
	Screen.flip()
	
	-- Exit check
	if Controls.check(Controls.read(), SCE_CTRL_TRIANGLE) then
		if img ~= nil then
			Graphics.freeImage(img)
		end
		img = nil
		break
	end
	
end