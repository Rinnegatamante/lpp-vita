ip, port = Network.initFTP()
while true do
	Screen.initBlend()
	Screen.clear()
	Screen.debugPrint(5, 5, "PSVITA listening on " .. ip .. " - Port " .. port, Color.new(255,255,255))
	Screen.debugPrint(5, 25, "Press Triangle to exit", Color.new(255,255,255))
	if Controls.check(Controls.read(), SCE_CTRL_TRIANGLE) then
		Network.termFTP()
		System.wait(800000)
		System.exit()
	end
	Screen.termBlend()
	Screen.flip()
end