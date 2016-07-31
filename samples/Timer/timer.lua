-- Init a new color
red = Color.new(255,0,0)

-- Init a timer
timer = Timer.new()

-- Main loop
while true do
	
	-- Check for input
	pad = Controls.read()
	if (Controls.check(pad,SCE_CTRL_START)) then
		Timer.destroy(timer)
		System.exit()
	end
	
	-- Blend something on screen
	Graphics.initBlend()
	Screen.clear()
	Graphics.debugPrint(10,30,"You lost " .. math.ceil(Timer.getTime(timer)/1000) .. " seconds of your life.",red)
	Graphics.termBlend()
	Screen.flip()
	
end