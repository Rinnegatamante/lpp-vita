-- Init debug FTP
ip, port = Network.initFTP()

-- Main loop
while true do

	-- Blend something on screen
	Graphics.initBlend()
	Screen.clear()
	Graphics.debugPrint(5, 5, "PSVITA listening on " .. ip .. " - Port " .. port, Color.new(255,255,255))
	Graphics.debugPrint(5, 25, "Press Triangle to exit", Color.new(255,255,255))
	
	-- Check for input
	if Controls.check(Controls.read(), SCE_CTRL_TRIANGLE) then
		Network.termFTP()
		System.wait(800000)
		System.exit()
	end
	
	-- Term blending and flip screen
	Graphics.termBlend()
	Screen.flip()
	
end