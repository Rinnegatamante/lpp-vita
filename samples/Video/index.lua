local white = Color.new(255,255,255) 

-- Init video device
Video.init()

-- Loading and starting our video file
Video.open("ux0:/data/lpp-vita/samples/Video/video.mp4")
Video.openSubs("ux0:/data/lpp-vita/samples/Video/video.vtt")

local modes = {
	NORMAL_MODE,
	FAST_FORWARD_2X_MODE,
	FAST_FORWARD_4X_MODE,
	FAST_FORWARD_8X_MODE,
	FAST_FORWARD_16X_MODE,
	FAST_FORWARD_32X_MODE
}

local mode_idx = 1
local oldpad = 0

-- Main loop
while true do
	
	-- Blend instructions
	Graphics.initBlend()
	Screen.clear()
	frame = Video.getOutput()
	if frame ~= 0 then
		w = Graphics.getImageWidth(frame)
		h = Graphics.getImageHeight(frame)
		Graphics.setImageFilters(frame, FILTER_LINEAR, FILTER_LINEAR)
		Graphics.drawScaleImage(0, 0, frame, 960 / w, 544 / h)
	end
	Graphics.debugPrint(20, 20, "Time (ms): " .. Video.getTime(), Color.new(255, 255, 255))
	Graphics.debugPrint(20, 40, "Speed: x" .. modes[mode_idx] / 100, Color.new(255, 255, 255))
	Graphics.debugPrint(20, 500, Video.getSubs(), Color.new(255, 255, 255))
	Graphics.termBlend()
	Screen.waitVblankStart()
	Screen.flip()
	
	-- Check for input
	local pad = Controls.read()
	if Controls.check(pad, SCE_CTRL_CIRCLE) and Video.isPlaying() then
		Video.pause()
	elseif Controls.check(pad, SCE_CTRL_SQUARE) and not Video.isPlaying() then
		Video.resume()
	elseif Controls.check(pad, SCE_CTRL_TRIANGLE) then
		Video.close()
		break
	elseif Controls.check(pad, SCE_CTRL_LTRIGGER) and not Controls.check(oldpad, SCE_CTRL_LTRIGGER) then
		mode_idx = mode_idx - 1
		if mode_idx < 1 then
			mode_idx = 1
		end
		Video.setPlayMode(modes[mode_idx])
	elseif Controls.check(pad, SCE_CTRL_RTRIGGER) and not Controls.check(oldpad, SCE_CTRL_RTRIGGER) then
		mode_idx = mode_idx + 1
		if mode_idx > #modes then
			mode_idx = #modes
		end
		Video.setPlayMode(modes[mode_idx])
	elseif Controls.check(pad, SCE_CTRL_LEFT) and not Controls.check(oldpad, SCE_CTRL_LEFT) then
		local cur_time = Video.getTime()
		jump_time = cur_time - 10000
		if jump_time < 0 then
			jump_time = 0
		end
		Video.jumpToTime(jump_time)
	elseif Controls.check(pad, SCE_CTRL_RIGHT) and not Controls.check(oldpad, SCE_CTRL_RIGHT) then
		Video.jumpToTime(Video.getTime() + 10000)
	end
	
	oldpad = pad
end