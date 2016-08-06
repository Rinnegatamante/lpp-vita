-- Loading a TTF font
fnt = Font.load("app0:/main.ttf")

-- Main loop
while true do

	-- Starting GPU rendering
	Graphics.initBlend()
	
	-- Clearing screen
	Screen.clear()
	
	-- Drawing something on screen
	Font.print(fnt, 5, 45, "Hello World", Color.new(255, 255, 255))
	
	-- Terminating GPU rendering
	Graphics.termBlend()
	
	-- Updating screen
	Screen.flip()
	
end