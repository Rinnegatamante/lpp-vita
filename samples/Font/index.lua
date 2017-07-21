-- Loading a TTF font
local fnt = Font.load("ux0:/data/lpp-vita/samples/Font/main.ttf")

-- Main loop
while true do

	-- Starting GPU rendering
	Graphics.initBlend()
	
	-- Clearing screen
	Screen.clear()
	
	-- Drawing something on screen
	Font.print(fnt, 5, 45, "Hello World", Color.new(255, 255, 255))
	Font.print(fnt, 5, 100, "Press TRIANGLE to return to the sample selector.", Color.new(255, 255, 255))
	
	-- Terminating GPU rendering
	Graphics.termBlend()
	
	-- Updating screen
	Screen.flip()
	
	-- Check for input
	if Controls.check(Controls.read(), SCE_CTRL_TRIANGLE) then
		Font.unload(fnt)
		break
	end
	
end