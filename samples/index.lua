while true do
	white = Color.new(255,255,255,255)
	Screen.debugPrint(5, 5, "Hello World!", white)
	Screen.flip()
	Screen.waitVblankStart()
end