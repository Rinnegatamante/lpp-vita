red = Color.new(255,0,0)
timer = Timer.new()
while true do
	pad = Controls.read()
	Screen.initBlend()
	Screen.clear()
	if (Controls.check(pad,PSP2_CTRL_START)) then
		Timer.destroy(timer)
		System.exit()
	end
	Screen.debugPrint(10,30,"You lost " .. math.ceil(Timer.getTime(timer)/1000) .. " seconds of your life.",red)
	Screen.termBlend()
	Screen.flip()
end