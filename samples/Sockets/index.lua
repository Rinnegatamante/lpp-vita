-- Initializing Network
Network.init()

-- Checking if connection is available
if Network.isWifiEnabled() then

	-- Opening a new socket and connecting to the host
	skt = Socket.connect("rinnegatamante.it", 80)

	-- Our payload to request a file
	payload = "GET /site/images/logo.png HTTP/1.1\r\nHost: rinnegatamante.it\r\n\r\n"

	-- Sending request
	Socket.send(skt, payload)

	-- Since sockets are non blocking, we wait till at least a byte is received
	raw_data = ""
	while raw_data == "" do
		raw_data = raw_data .. Socket.receive(skt, 8192)
	end

	-- Keep downloading till the whole response is received
	dwnld_data = raw_data
	retry = 0
	while dwnld_data ~= "" or retry < 1000 do
		dwnld_data = Socket.receive(skt, 8192)
		raw_data = raw_data .. dwnld_data
		if dwnld_data == "" then
			retry = retry + 1
		else
			retry = 0
		end
	end

	-- Extracting Content-Length value
	offs1, offs2 = string.find(raw_data, "Length: ")
	offs3 = string.find(raw_data, "\r", offs2)
	content_length = tonumber(string.sub(raw_data, offs2, offs3))

	-- Saving downloaded image
	stub, content_offset = string.find(raw_data, "\r\n\r\n")
	handle = System.openFile("ux0:/data/data.png", FCREATE)
	content = string.sub(raw_data, content_offset+1)
	System.writeFile(handle, string.sub(raw_data, content_offset+1), string.len(content))
	System.closeFile(handle)

	-- Closing socket
	Socket.close(skt)

	-- Loading image in memory and deleting it from storage
	img = Graphics.loadImage("ux0:/data/data.png")
	System.deleteFile("ux0:/data/data.png")

end

-- Terminating network
Network.term()

-- Main loop
while true do
	
	-- Drawing the downloaded image
	Graphics.initBlend()
	Screen.clear()
	if img == nil then
		Graphics.debugPrint(5,220,"You must enable WiFi to use this sample.", Color.new(255,255,255))
		Graphics.debugPrint(5,240,"Press TRIANGLE to return to the sample selector", Color.new(255,255,255))
	else
		Graphics.drawImage(200,265,img)
		Graphics.debugPrint(5,220,"This image came from http://rinnegatamante.it/site/images/logo.png", Color.new(255,255,255))
		Graphics.debugPrint(5,240,"Press TRIANGLE to return to the sample selector", Color.new(255,255,255))
	end
	Graphics.termBlend()
	Screen.flip()
	
	-- Exit check
	if Controls.check(Controls.read(), SCE_CTRL_TRIANGLE) then
		if img ~= nil then
			Graphics.freeImage(img)
		end
		img = nil
		break
	end
	
end