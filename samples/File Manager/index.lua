-- Init some colors
local white = Color.new(255, 255, 255)
local yellow = Color.new(255, 255, 0)
local red = Color.new(255, 0, 0)
local oldpad = SCE_CTRL_CROSS

-- List a directory
local scripts = System.listDirectory("ux0:/")
local cur_dir = "ux0:/"

-- Init a index
local i = 1

-- Main loop
while true do
	
	-- Reset y axis for menu blending
	local y = 25
	
	-- Write title on screen
	Graphics.initBlend()
	Screen.clear()
	Graphics.debugPrint(5, 5, "File Manager Sample - Press TRIANGLE to close sample, CIRCLE to return to previous dir.", yellow)
	
	-- Write visible menu entries
	for j, file in pairs(scripts) do
		x = 5
		if j >= i and y < 960 then
			if i == j then
				color = red
				x = 10
			else
				color = white
			end
			Graphics.debugPrint(x, y, file.name, color)
			y = y + 20
		end
	end
	Graphics.termBlend()
	
	-- Check for input
	pad = Controls.read()
	if Controls.check(pad, SCE_CTRL_CROSS) and not Controls.check(oldpad, SCE_CTRL_CROSS) then
		if scripts[i].directory then
			cur_dir = cur_dir .. scripts[i].name .. "/"
			scripts = System.listDirectory(cur_dir)
			i = 1
		end
	elseif Controls.check(pad, SCE_CTRL_CIRCLE) and not Controls.check(oldpad, SCE_CTRL_CIRCLE) then
		if string.len(cur_dir) > 5 then -- Excluding ux0:/
			j=-2
			while string.sub(cur_dir,j,j) ~= "/" do
				j=j-1
			end
			cur_dir = string.sub(cur_dir,1,j)
			scripts = System.listDirectory(cur_dir)
			i = 1
		end
	elseif Controls.check(pad, SCE_CTRL_UP) and not Controls.check(oldpad, SCE_CTRL_UP) then
		i = i - 1
	elseif Controls.check(pad, SCE_CTRL_DOWN) and not Controls.check(oldpad, SCE_CTRL_DOWN) then
		i = i + 1
	elseif Controls.check(pad, SCE_CTRL_TRIANGLE) then
		break
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
