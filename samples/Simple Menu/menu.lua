-- Init some colors
white = Color.new(255, 255, 255)
yellow = Color.new(255, 255, 0)
red = Color.new(255, 0, 0)

-- List a directory
scripts = System.listDirectory("ux0:/lpp")

-- Init a index
i = 1

-- Main loop
while true do
	
	-- Reset y axis for menu blending
	y = 25
	
	-- Write title on screen
	Graphics.initBlend()
	Screen.clear()
	Graphics.debugPrint(5, 5, "Lua Player Plus - Select script to start", yellow)
	
	-- Write visible menu entries
	for j, file in pairs(scripts) do
		x = 5
		if i == j then
			color = red
			x = 10
		else
			color = white
		end
		Graphics.debugPrint(x, y, file.name, color)
		y = y + 20
	end
	Graphics.termBlend()
	
	-- Check for input
	pad = Controls.read()
	if Controls.check(pad, SCE_CTRL_CROSS) then
		Graphics.initBlend()
		Screen.clear()
		Graphics.termBlend()
		Screen.flip()
		Graphics.initBlend()
		Screen.clear()
		Graphics.termBlend()
		System.wait(800000)
		dofile("ux0:/lpp/" .. scripts[i].name)
	elseif Controls.check(pad, SCE_CTRL_UP) and not Controls.check(oldpad, SCE_CTRL_UP) then
		i = i - 1
	elseif Controls.check(pad, SCE_CTRL_DOWN) and not Controls.check(oldpad, SCE_CTRL_DOWN) then
		i = i + 1
	end
	
	-- Check for out of bounds in menu
	if i > #scripts then
		i = 1
	elseif i < 1 then
		i = #scripts
	end
	
	-- Update oldpad and flip screen
	oldpad = pad
	Screen.flip()
	
end
