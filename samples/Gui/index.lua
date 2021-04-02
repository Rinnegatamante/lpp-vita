Gui.init()
Gui.setInputMode(true, false, true, true)

is_checked = false
theme = DARK_THEME

radio_set = 1

slider_val = 52.4
slider_int_val = 38

-- Main loop
while true do
	
	Graphics.initBlend()
	Screen.clear()
	Gui.initBlend()
	
	-- Main menubar
	if Gui.initMainMenubar() then
		if Gui.initMenu("Settings") then
			if Gui.initMenu("Theme") then
				if Gui.drawMenuItem("Dark", theme == DARK_THEME) then
					theme = DARK_THEME
					Gui.setTheme(theme)
				end
				if Gui.drawMenuItem("Light", theme == LIGHT_THEME) then
					theme = LIGHT_THEME
					Gui.setTheme(theme)
				end
				if Gui.drawMenuItem("Classic", theme == CLASSIC_THEME) then
					theme = CLASSIC_THEME
					Gui.setTheme(theme)
				end
				Gui.termMenu()
			end
			if Gui.drawMenuItem("Exit Sample") then
				System.exit()
			end
			Gui.termMenu()
		end
		Gui.termMainMenubar()
	end
	
	-- Main window
	Gui.setWindowPos(0, 22, SET_ALWAYS)
	Gui.setWindowSize(960, 544-22, SET_ALWAYS)
	Gui.initWindow("#main_window#", FLAG_NO_MOVE + FLAG_NO_RESIZE + FLAG_NO_TITLEBAR + FLAG_NO_COLLAPSE)
	Gui.drawText("Hello World ImGui")
	Gui.drawText("Inside lpp-vita through Gui module!", Color.new(255,0,0))
	Gui.drawSeparator()
	is_checked = Gui.drawCheckbox("Checkbox", is_checked)
	Gui.drawSeparator()
	if Gui.drawRadioButton("Radio 1", radio_set == 1) then
		radio_set = 1
	end
	if Gui.drawRadioButton("Radio 2", radio_set == 2) then
		radio_set = 2
	end
	if Gui.drawRadioButton("Radio 3", radio_set == 3) then
		radio_set = 3
	end
	Gui.drawSeparator()
	slider_val = Gui.drawSlider("A slider", 0, 100, slider_val)
	slider_int_val = Gui.drawIntSlider("A integer slider", 0, 100, slider_int_val)
	Gui.drawSeparator()
	if Gui.drawButton("Exit the sample") then
		System.exit()
	end
	Gui.resetLine()
	if Gui.drawButton("Reboot device") then
		System.reboot()
	end
	Gui.termWindow()
	
	Gui.termBlend()
	Graphics.termBlend()
	
	-- Flip screen
	Screen.flip()
	
end