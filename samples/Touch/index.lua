-- Init a new color
local red = Color.new(255,0,0)

-- Main loop
while true do
	
	-- Check for input
	local x1,y1 = Controls.readTouch()
	local x2,y2 = Controls.readRetroTouch()
	local pad = Controls.read()
	if Controls.check(pad, SCE_CTRL_TRIANGLE) then
		break
	end
	
	-- Drawing info on screen
	Graphics.initBlend()
	Screen.clear()
	if x1 ~= nil then
		Graphics.debugPrint(5,20,"Front: x = " .. x1 .. " , y = " .. y1, red)
	else
		Graphics.debugPrint(5,20,"Front: not pressed", red)
	end
	if x2 ~= nil then
		Graphics.debugPrint(5,40,"Rear: x = " .. x2 .. " , y = " .. y2, red)
	else
		Graphics.debugPrint(5,40,"Rear: not pressed", red)
	end
	Graphics.debugPrint(5,60, "Press TRIANGLE to return to sample selector.", red)
	Graphics.termBlend()
	Screen.flip()
	
end