-- Init audio device
Sound.init()

-- Loading and starting our audio file
snd = Sound.openOgg("app0:/sample.ogg")
Sound.play(snd, LOOP)

-- Main loop
while true do
	
	-- Check for input
	pad = Controls.read()
	if Controls.check(pad, SCE_CTRL_CROSS) and Sound.isPlaying(snd) then
		Sound.pause(snd)
	elseif Controls.check(pad, SCE_CTRL_SQUARE) and not Sound.isPlaying(snd) then
		Sound.resume(snd)
	elseif Controls.check(pad, SCE_CTRL_START) and Sound.isPlaying(snd) then
		Sound.close(snd)
		Sound.term()
		System.exit()
	end
	
end
