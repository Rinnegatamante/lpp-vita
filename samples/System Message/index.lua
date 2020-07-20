-- Initializing a system message
System.setMessage("This sample will show a progressbar message.\nIt will take 20 seconds to finish.\nDo you want to start this sample?", false, BUTTON_YES_NO)
ret = nil
messageIdx = 1
prog_timer = nil
progress = 10

-- Main loop
while true do
	
	-- Initializing drawing phase
	Graphics.initBlend()
	Screen.clear()
	
	-- Checking for message status
	if messageIdx < 3 then
		status = System.getMessageState()
		
		-- Updating progressbar status
		if prog_timer ~= nil then
			if Timer.getTime(prog_timer) > 2000 then
				if progress > 100 then
					System.closeMessage()
				else
					System.setMessageProgress(progress)
					System.setMessageProgMsg(progress .. "%")
					progress = progress + 10
				end
				Timer.reset(prog_timer)
			end
		end
		
		if status ~= RUNNING then
			
			-- Check if user didn't canceled the message
			if status == CANCELED then
				messageIdx = 3
				ret = "You canceled the sample"
			else
				messageIdx = messageIdx + 1
				
				if messageIdx == 3 then
					
					-- Sample finished, showing result
					ret = "Sample finished"
					Timer.destroy(prog_timer)
					
				else
					
					-- Starting progressbar message
					System.setMessage("Progressbar sample in progress.\nPlease wait...", true)
					System.setMessageProgMsg("0%")
					prog_timer = Timer.new()
					
				end
				
			end
		
		end
	end
	
	-- Dtawing state on screen
	if messageIdx == 3 then
		Graphics.debugPrint(5, 5, ret, Color.new(255,255,255))
		Graphics.debugPrint(5, 25, "Press TRIANGLE to return to the sample selector.", Color.new(255,255,255))
		if Controls.check(Controls.read(), SCE_CTRL_TRIANGLE) then
			Graphics.termBlend()
			break
		end
	end
	Graphics.termBlend()
	Screen.flip()
	Screen.waitVblankStart()
	
end