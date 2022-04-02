-- Initializing a Keyboard
Keyboard.start("Sample Title", "Sample Text")
ret = "Waiting for user input..."

-- Main loop
while true do
	
	-- Initializing drawing phase
	Graphics.initBlend()
	Screen.clear()
	
	-- Checking for keyboard status
	status = Keyboard.getState()
	if status ~= RUNNING then
		
		-- Check if user didn't canceled the keyboard
		if status ~= CANCELED then
			ret = "You typed: " .. Keyboard.getInput()
		else
			ret = "You canceled the keyboard"
		end
		
		-- Terminating keyboard
		Keyboard.clear()
		
	end
	
	-- Dtawing state on screen
	Graphics.debugPrint(5, 5, ret, Color.new(255,255,255))
	if ret ~= "Waiting for user input..." then
		Graphics.debugPrint(5, 25, "Press TRIANGLE to return to the sample selector.", Color.new(255,255,255))
		if Controls.check(Controls.read(), SCE_CTRL_TRIANGLE) then
			Graphics.termBlend()
			break
		end
	end
	Graphics.termBlend()
	Screen.flip()
	
end
