while true do

	white = Color.new(255,255,255,255) -- Create new color
	
	-- Draw string on the screen
	Screen.initBlend()
	Screen.clear()
	Screen.debugPrint(5, 5, "Hello World!", white)
	Screen.termBlend()
	
	-- Update screens (For double buffering)
	Screen.flip()
	
	-- Check controls for exiting
	if Controls.check(Controls.read(), SCE_CTRL_START) then
		System.exit()
	end
end