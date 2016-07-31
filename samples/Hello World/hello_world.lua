-- Create a new color
white = Color.new(255,255,255,255) 

-- Main loop
while true do
	
	-- Draw a string on the screen
	Graphics.initBlend()
	Screen.clear()
	Graphics.debugPrint(5, 5, "Hello World!", white)
	Graphics.termBlend()
	
	-- Update screen (For double buffering)
	Screen.flip()
	
	-- Check controls for exiting
	if Controls.check(Controls.read(), SCE_CTRL_START) then
		System.exit()
	end
	
end