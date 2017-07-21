-- Init a new color
local red = Color.new(255,0,0)

-- Init a timer
local timer = Timer.new()

-- Main loop
while true do
	
	-- Check for input
	local pad = Controls.read()
	if (Controls.check(pad,SCE_CTRL_TRIANGLE)) then
		Timer.destroy(timer)
		break
	end
	
	-- Blend something on screen
	Graphics.initBlend()
	Screen.clear()
	Graphics.debugPrint(10,30,math.ceil(Timer.getTime(timer)/1000) .. " seconds passed.",red)
	Graphics.debugPrint(10,50,"Press TRIANGLE to return sample selector",red)
	Graphics.termBlend()
	Screen.flip()
	
end