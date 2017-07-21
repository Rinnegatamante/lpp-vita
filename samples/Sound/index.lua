local white = Color.new(255,255,255) 

-- Init audio device
Sound.init()

-- Loading and starting our audio file
local snd = Sound.openOgg("ux0:/data/lpp-vita/samples/Sound/sample.ogg")
Sound.play(snd, LOOP)

-- Main loop
while true do
	
	-- Check for input
	local pad = Controls.read()
	if Controls.check(pad, SCE_CTRL_CIRCLE) and Sound.isPlaying(snd) then
		Sound.pause(snd)
	elseif Controls.check(pad, SCE_CTRL_SQUARE) and not Sound.isPlaying(snd) then
		Sound.resume(snd)
	elseif Controls.check(pad, SCE_CTRL_TRIANGLE) then
		Sound.close(snd)
		break
	end
	
	-- Blend instructions
	Graphics.initBlend()
	Screen.clear()
	Graphics.debugPrint(5, 20, "Press CIRCLE to pause sound.", white)
	Graphics.debugPrint(5, 40, "Press SQUARE to resume sound.", white)
	Graphics.debugPrint(5, 100, "Press TRIANGLE to return to the sample selector.", white)
	Graphics.termBlend()
	Screen.flip()
	
end
