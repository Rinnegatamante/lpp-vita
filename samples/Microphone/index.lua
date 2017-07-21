local white = Color.new(255,255,255) 
local recorded = false
local recording = false
local mic_is_recording = false
local paused = false
local snd

-- Init audio device
Sound.init()

-- Main loop
while true do
	
	-- Reading input
	local pad = Controls.read()
	
	-- Check if we already recorded an audio file
	if recorded then
		
		-- Blend instructions
		Graphics.initBlend()
		Screen.clear()
		Graphics.debugPrint(5, 20, "Press CIRCLE to pause sound.", white)
		Graphics.debugPrint(5, 40, "Press SQUARE to resume sound.", white)
		Graphics.debugPrint(5, 100, "Press TRIANGLE to return to the sample selector.", white)
		Graphics.termBlend()
		
		-- Check for input
		local pad = Controls.read()
		if Controls.check(pad, SCE_CTRL_CIRCLE) and Sound.isPlaying(snd) then
			Sound.pause(snd)
		elseif Controls.check(pad, SCE_CTRL_SQUARE) and not Sound.isPlaying(snd) then
			Sound.resume(snd)
		elseif Controls.check(pad, SCE_CTRL_TRIANGLE) then
			Sound.close(snd)
			System.deleteFile("ux0:/data/lpp-vita/samples/Microphone/sample.wav")
			break
		end
		
		
	else
		
		-- Blend instructions
		Graphics.initBlend()
		Screen.clear()
		if not recording then
			Graphics.debugPrint(5, 20, "Press CROSS to start recording.", white)
		else
			Graphics.debugPrint(5, 20, "Recording phase...", white)
		end
		Graphics.debugPrint(5, 40, "Press SQUARE to pause/resume recording.", white)
		Graphics.debugPrint(5, 100, "Press TRIANGLE to return to the sample selector.", white)
		Graphics.termBlend()
		
		-- Check for input
		if Controls.check(pad, SCE_CTRL_CROSS) and not recording then
			Mic.start(5, 16000)
			recording = true
		elseif Controls.check(pad, SCE_CTRL_SQUARE) and not Sound.isPlaying(snd) then
			if mic_is_recording then
				Mic.pause()
				paused = true
			else
				Mic.resume()
				paused = false
			end
		elseif Controls.check(pad, SCE_CTRL_TRIANGLE) then
		
			-- Stopping microphone and discarding result
			if recording then
				Mic.stop("ux0:/data/discard.wav")
				System.deleteFile("ux0:/data/discard.wav")
			end
			
			break
		end
		
		if recording then
			mic_is_recording = Mic.isRecording()
			if not mic_is_recording and not paused then -- We ended our recording
				recorded = true
				Mic.stop("ux0:/data/lpp-vita/samples/Microphone/sample.wav")
				snd = Sound.openWav("ux0:/data/lpp-vita/samples/Microphone/sample.wav")
				Sound.play(snd, LOOP)
			end
		end	
		
	end

	Screen.flip()
	
end