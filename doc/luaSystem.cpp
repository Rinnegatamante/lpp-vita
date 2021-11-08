/**
 * \defgroup System
 * Module that handles system related utilities.
 */ 

/**
 * I/O attributes to use with ::System.openFile.
 * \ingroup System
 */
enum IOFlags{
	FREAD,  //!< Read attribute.
	FWRITE, //!< Write attribute.
	FRDWR,  //!< Read/Write attribute.
	FCREATE //!< Create/Write attribute.
};

/**
 * I/O seek modes to use with ::System.seekFile.
 * \ingroup System
 */
enum IOSeek{
	SET,  //!< Starts from the begin of the file.
	CUR,  //!< Starts from current position.
	END  //!< Starts from the end of the file.
};

/**
 * Power management timer features.
 * \ingroup System
 */
enum PowerTimer{
	AUTO_SUSPEND_TIMER,   //!< Automatic console suspension.
	SCREEN_OFF_TIMER,     //!< Automatic screen disabling.
	SCREEN_DIMMING_TIMER  //!< Automatic screen dimming.
};

/**
 * Image format modes to use with ::System.takeScreenshot.
 * \ingroup System
 */
enum ImgFmt{
	FORMAT_BMP,  //!< 32bpp BMP format.
	FORMAT_JPG,  //!< JPG format.
	FORMAT_PNG  //!< 8bpp paletted PNG format.
};

/**
 * Buttons schemes for ::System.setMessage.
 * \ingroup System
 */
enum BtnMode{
	BUTTON_NONE,       //!< No buttons.
	BUTTON_OK,         //!< OK button only.
	BUTTON_CANCEL,     //!< CANCEL button only.
	BUTTON_OK_CANCEL,  //!< OK and CANCEL buttons.
	BUTTON_YES_NO,     //!< YES and NO buttons.
};

/**
 * Dialog states for system messages and system keyboard.
 * \ingroup System
 */
enum DlgState{
	RUNNING,   //!< Dialog running.
	FINISHED,  //!< Dialog finished successfully.
	CANCELED   //!< Dialog canceled by user.
};

/**
 * Mount permissions for partition mounting.
 * \ingroup System
 */
enum MntPerm{
	READ_ONLY,  //!< Read only permissions.
	READ_WRITE  //!< Read/Write permissions.
};

class System{
	
	public:
	
		/**
		 * Open a file.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * handle = System.openFile("ux0:/data/file.txt", FREAD)
		 * @endcode
		 *
		 * @param filename - Filename to be opened.
		 * @param flags - An attribute flag.
		 *
		 * @return A valid file handle.
		 */
		int openFile(string filename, IOFlags flags);
		
		/**
		 * Write into an opened file.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.writeFile(handle, "Test", 4)
		 * @endcode
		 *
		 * @param handle - A file handle opened with ::System.openFile.
		 * @param text - The text to write on file.
		 * @param len - Length of the text.
		 */
		void writeFile(int handle, string text, int len);
		
		/**
		 * Read from an opened file.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * text = System.readFile(handle, 4)
		 * @endcode
		 *
		 * @param handle - A file handle opened with ::System.openFile.
		 * @param len - Length of the string to read in bytes.
		 *
		 * @return The read string.
		 */
		string readFile(int handle, int len);
		
		/**
		 * Move cursor of an opened file.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.seekFile(handle, 0, END)
		 * @endcode
		 *
		 * @param handle - A file handle opened with ::System.openFile.
		 * @param pos - Position to set.
		 * @param seek - Start point for the position.
		 */
		void seekFile(int handle, int pos, IOSeek seek);
		
		/**
		 * Calculate size of an opened file in bytes.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * size = System.sizeFile(handle)
		 * @endcode
		 *
		 * @param handle - A file handle opened with ::System.openFile.
		 *
		 * @return Filesize in bytes.
		 */
		int sizeFile(int handle);
		
		/**
		 * Extract info of a file/directory.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * info = System.statFile("ux0:/data/file.txt")
		 * creation_year = info.creation_time.year
		 * last_access_year = info.access_time.year
		 * last_modification_year = info.mod_time.year
		 * is_directory = info.directory
		 * filesize = info.size
		 * @endcode
		 *
		 * @param filename - Filename to be checked.
		 *
		 * @return A table with a proper set of info (Look at the sample above).
		 */
		table statFile(string filename);
		
		/**
		 * Extract info of an opened file.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * info = System.statOpenedFile(handle)
		 * creation_year = info.creation_time.year
		 * last_access_year = info.access_time.year
		 * last_modification_year = info.mod_time.year
		 * is_directory = info.directory
		 * filesize = info.size
		 * @endcode
		 *
		 * @param handle - A file handle opened with ::System.openFile.
		 *
		 * @return A table with a proper set of info (Look at the sample above).
		 */
		table statOpenedFile(int handle);
		
		/**
		 * Close an opened file.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.closeFile(handle)
		 * @endcode
		 *
		 * @param handle - A file handle opened with ::System.openFile.
		 */
		void closeFile(int handle);
		
		/**
		 * Check if a file exists.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * if System.doesFileExist("ux0:/data/file.txt") then
		 * 		handle = System.openFile("ux0:/data/file.txt", FREAD)
		 * end
		 * @endcode
		 *
		 * @param filename - Filename to be checked.
		 *
		 * @return true if exists, false otherwise.
		 */
		bool doesFileExist(string filename);
		
		/**
		 * Check if a folder exists.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * if System.doesDirExist("ux0:/data") then
		 * 		list = System.listDirectory("ux0:/data")
		 * end
		 * @endcode
		 *
		 * @param dirname - Folder name to be checked.
		 *
		 * @return true if exists, false otherwise.
		 */
		bool doesDirExist(string dirname);
		
		/**
		 * Rename or move a file.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.rename("ux0:/data/old.txt", "ux0:/data/new.txt")
		 * @endcode
		 *
		 * @param filename - Filename to be renamed.
		 * @param filename2 - New filename.
		 */
		void rename(string filename, string filename2);
		
		/**
		 * Copy a file.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.copyFile("ux0:/data/old.txt", "ux0:/data/new.txt")
		 * @endcode
		 *
		 * @param filename - Filename to be copied.
		 * @param filename2 - New filename.
		 */
		void copyFile(string filename, string filename2);
		
		/**
		 * Delete a file.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.deleteFile("ux0:/data/file.txt")
		 * @endcode
		 *
		 * @param filename - Filename to be removed.
		 */
		void deleteFile(string filename);
		
		/**
		 * Delete a directory.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.deleteDirectory("ux0:/data/dir")
		 * @endcode
		 *
		 * @param dirname - Path to be removed.
		 */
		void deleteDirectory(string dirname);
		
		/**
		 * Create a new directory.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.createDirectory("ux0:/data/dir")
		 * @endcode
		 *
		 * @param dirname - Path to be created.
		 */
		void createDirectory(string dirname);
		
		/**
		 * List a directory contents.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * files = System.listDirectory("ux0:/data/dir")
		 * name = files[0].name
		 * size = files[0].size
		 * is_directory = files[0].directory
		 * @endcode
		 *
		 * @param dirname - Path to be listed.
		 *
		 * @return A table with the listed files info (See the example above).
		 */
		table listDirectory(string dirname);
		
		/**
		 * Close the interpreter.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.exit()
		 * @endcode
		 */
		void exit(void);
		
		/**
		 * Get unspoofed system firmware version.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * fw = System.getFirmware()
		 * @endcode
		 *
		 * @return The unspoofed system firmware version.
		 *
		 * @note This function is available only in unsafe mode.
		 */
		string getFirmware(void);
		
		/**
		 * Get spoofed system firmware version.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * fw = System.getSpoofedFirmware()
		 * @endcode
		 *
		 * @return The spoofed system firmware version.
		 */
		string getSpoofedFirmware(void);
		
		/**
		 * Get factory system firmware version.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * fw = System.getFactoryFirmware()
		 * @endcode
		 *
		 * @return The factory system firmware version.
		 *
		 * @note This function is available only in unsafe mode.
		 */
		string getFactoryFirmware(void);
		
		/**
		 * Sleep the application for a certain time.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.wait(1000000)
		 * @endcode
		 *
		 * @param msecs - Time to wait in microseconds.
		 */
		void wait(int msecs);
		
		/**
		 * Check if the battery is under charge.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * if System.isBatteryCharging() then
		 * 		Graphics.debugPrint(0, 0, "Battery is charging", Color.new(255, 255, 255))
		 * end
		 * @endcode
		 *
		 * @return true if charging, false otherwise.
		 */
		bool isBatteryCharging(void);
		
		/**
		 * Get current battery life percentage.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * life = System.getBatteryPercentage()
		 * @endcode
		 *
		 * @return The battery life percentage.
		 */
		int getBatteryPercentage(void);
		
		/**
		 * Get current battery life.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * life = System.getBatteryLife()
		 * @endcode
		 *
		 * @return The battery life in minutes.
		 */
		int getBatteryLife(void);
		
		/**
		 * Get current battery capacity.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * cap = System.getBatteryCapacity()
		 * @endcode
		 *
		 * @return The battery capacity in mAh.
		 */
		int getBatteryCapacity(void);
		
		/**
		 * Get full battery capacity.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * cap_max = System.getBatteryFullCapacity()
		 * @endcode
		 *
		 * @return The battery full capacity in mAh.
		 */
		int getBatteryFullCapacity(void);
		
		/**
		 * Get battery temperature.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * temp = System.getBatteryTemp()
		 * @endcode
		 *
		 * @return The battery temperature in Celsius.
		 */
		int getBatteryTemp(void);
		
		/**
		 * Get battery voltage.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * voltage = System.getBatteryVolt()
		 * @endcode
		 *
		 * @return The battery voltage in mV.
		 */
		int getBatteryVolt(void);
		
		/**
		 * Get battery state of health percentage.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * soh = System.getBatteryHealth()
		 * @endcode
		 *
		 * @return The battery state of health percentage.
		 */
		int getBatteryHealth(void);
		
		/**
		 * Get battery cycles count.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * cycles = System.getBatteryCycles()
		 * @endcode
		 *
		 * @return The battery cycles count.
		 */
		int getBatteryCycles(void);
		
		/**
		 * Disable a power management timer feature.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.disableTimer(SCREEN_DIMMING_TIMER)
		 * @endcode
		 *
		 * @param timer - The timer feature to disable.
		 */
		void disableTimer(PowerTimer timer);
		
		/**
		 * Enable a power management timer feature.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.enableTimer(SCREEN_DIMMING_TIMER)
		 * @endcode
		 *
		 * @param timer - The timer feature to enable.
		 */
		void enableTimer(PowerTimer timer);
		
		/**
		 * Reset a power management timer feature.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.resetTimer(SCREEN_DIMMING_TIMER)
		 * @endcode
		 *
		 * @param timer - The timer feature to reset.
		 */
		void resetTimer(PowerTimer timer);
		
		/**
		 * Set CPU clock speed.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.setCpuSpeed(444)
		 * @endcode
		 *
		 * @param clock - The clock to set in Mhz.
		 *
		 * @note Clock must be between 0 and 444.
		 */
		void setCpuSpeed(int clock);
		
		/**
		 * Set GPU clock speed.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.setGpuSpeed(222)
		 * @endcode
		 *
		 * @param clock - The clock to set in Mhz.
		 *
		 * @note Clock must be between 0 and 222.
		 */
		void setGpuSpeed(int clock);
		
		/**
		 * Set BUS clock speed.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.setBusSpeed(222)
		 * @endcode
		 *
		 * @param clock - The clock to set in Mhz.
		 *
		 * @note Clock must be between 0 and 222.
		 */
		void setBusSpeed(int clock);
		
		/**
		 * Set GPU Crossbar clock speed.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.setGpuXbarSpeed(166)
		 * @endcode
		 *
		 * @param clock - The clock to set in Mhz.
		 *
		 * @note Clock must be between 0 and 166.
		 */
		void setBusSpeed(int clock);
		
		/**
		 * Get CPU clock speed.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * clock = System.getCpuSpeed()
		 * @endcode
		 *
		 * @return The CPU clock speed in Mhz.
		 */
		int getCpuSpeed(void);
		
		/**
		 * Get GPU clock speed.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * clock = System.getGpuSpeed()
		 * @endcode
		 *
		 * @return The GPU clock speed in Mhz.
		 */
		int getGpuSpeed(void);
		
		/**
		 * Get BUS clock speed.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * clock = System.getBusSpeed()
		 * @endcode
		 *
		 * @return The BUS clock speed in Mhz.
		 */
		int getBusSpeed(void);
		
		/**
		 * Get GPU Crossbar clock speed.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * clock = System.getGpuXbarSpeed()
		 * @endcode
		 *
		 * @return The GPU Crossbar clock speed in Mhz.
		 */
		int getGpuXbarSpeed(void);
		
		/**
		 * Launches a binary file.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.launchEboot("app0:/updater.bin")
		 * @endcode
		 *
		 * @param filename - The file to start.
		 *
		 * @note The binary file to launch must be in app0 partition.
		 */
		void launchEboot(string filename);
		
		/**
		 * Launches an installed application.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.launchApp("MLCL00001")
		 * @endcode
		 *
		 * @param titleid - The title ID of the application to start.
		 */
		void launchApp(string titleid);
		
		/**
		 * Get free space for a given device.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * Graphics.debugPrint(5, 5, System.getFreeSpace("ux0:") .. "/" .. System.getTotalSpace("ux0:"), Color.new(255, 255, 255))
		 * @endcode
		 *
		 * @param dev - The device name.
		 */
		void getFreeSpace(string dev);
		
		/**
		 * Get total space for a given device.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * Graphics.debugPrint(5, 5, System.getFreeSpace("ux0:") .. "/" .. System.getTotalSpace("ux0:"), Color.new(255, 255, 255))
		 * @endcode
		 *
		 * @param dev - The device name.
		 */
		void getTotalSpace(string dev);
		
		/**
		 * Get console time.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * h,m,s = System.getTime()
		 * @endcode
		 *
		 * @return Three values owning hours, minutes and seconds.
		 */
		int[] getTime(void);
		
		/**
		 * Get console date.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * day_num, d, m, y = System.getDate()
		 * if day_num == 1 then
		 * 		day_name = "Monday"
		 * end
		 * @endcode
		 *
		 * @return Four values owning day value, day, month, year.
		 */
		int[] getDate(void);
		
		/**
		 * Get account username.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * username = System.getUsername()
		 * @endcode
		 *
		 * @return The username set on the running console account.
		 */
		string getUsername(void);
		
		/**
		 * Get console language.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * lang = System.getLanguage()
		 * @endcode
		 *
		 * @return The language ID currently set on the console.
		 *
		 * @note For the available language IDs look at <a href="https://www.vitadevwiki.com/vita/Languages">this</a>.
		 */
		int getLanguage(void);
		
		/**
		 * Get console model.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * model = System.getModel()
		 * @endcode
		 *
		 * @return 0x10000 if PSVITA, 0x20000 if PSTV.
		 */
		int getModel(void);
		
		/**
		 * Get console unique PsId.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * psid = System.getPsId()
		 * @endcode
		 *
		 * @return The console unique PsId.
		 */
		string getPsId(void);
		
		/**
		 * Get running application title.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * title = System.getTitle()
		 * @endcode
		 *
		 * @return The running application title.
		 */
		string getTitle(void);
		
		/**
		 * Get running application title ID.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * title_id = System.getTitleID()
		 * @endcode
		 *
		 * @return The running application title ID.
		 */
		string getTitleID(void);
		
		/**
		 * Extract info from an SFO file.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * info = System.extractSfo("app0:/sce_sys/param.sfo")
		 * app_version = info.version
		 * app_title = info.title
		 * app_category = info.category
		 * app_titleid = info.titleid
		 * @endcode
		 *
		 * @param filename - The filename to extract info from.
		 *
		 * @return A table with a proper set of info (Look at the sample above).
		 */
		table extractSfo(string filename);
		
		/**
		 * Extract a ZIP file (synchronous).
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.extractZip("app0:/files.zip", "ux0:/data/app_files")
		 * @endcode
		 *
		 * @param filename - The filename to extract.
		 * @param dirname - The path where to extract files.
		 *
		 * @note If <b>dirname</b> doesn't exist, it will be automatically created.
		 */
		void extractZip(string filename, string dirname);
		
		/**
		 * Extract a specific file from a ZIP file (synchronous).
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.extractFromZip("app0:/files.zip", "image.jpg", "ux0:/data/app_files/my_image.jpg")
		 * @endcode
		 *
		 * @param filename - The filename of the ZIP archive.
		 * @param file - The file to extract.
		 * @param destname - The filename where to extract file.
		 */
		void extractFromZip(string filename, string file, string destname);
		
		/**
		 * Extract a ZIP file (asynchronous).
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.extractZipAsync("app0:/files.zip", "ux0:/data/app_files")
		 * @endcode
		 *
		 * @param filename - The filename to extract.
		 * @param dirname - The path where to extract files.
		 *
		 * @note If <b>dirname</b> doesn't exist, it will be automatically created.
		 */
		void extractZipAsync(string filename, string dirname);
		
		/**
		 * Extract a specific file from a ZIP file (asynchronous).
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.extractFromZipAsync("app0:/files.zip", "image.jpg", "ux0:/data/app_files/my_image.jpg")
		 * @endcode
		 *
		 * @param filename - The filename of the ZIP archive.
		 * @param file - The file to extract.
		 * @param destname - The filename where to extract file.
		 */
		void extractFromZipAsync(string filename, string file, string destname);
		
		/**
		 * Compress a file or a folder in a ZIP file (synchronous).
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.compressZip("app0:/sce_sys", "ux0:/data/file.zip", 9)
		 * @endcode
		 *
		 * @param path - The filename or path to compress.
		 * @param filename - The filename of the resulting zip file.
		 * @param ratio - The compression ratio to use <b>(optional)</b>.
		 *
		 * @note <b>ratio</b> must be between 0 and 9.
		 */
		void compressZip(string path, string filename, int ratio);
		
		/**
		 * Add a file or a folder in a ZIP file (synchronous).
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.addToZip("app0:/sce_sys/icon0.png", "ux0:/data/file.zip", "sce_sys", 9)
		 * @endcode
		 *
		 * @param path - The filename or path to compress.
		 * @param filename - The filename of the resulting zip file.
		 * @param parent - The parent folder inside the zip file where to place the path.
		 * @param ratio - The compression ratio to use <b>(optional)</b>.
		 *
		 * @note <b>ratio</b> must be between 0 and 9.
		 */
		void addToZip(string path, string filename, string parent, int ratio);
		
		/**
		 * Get current state of an asynchronous task.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * while System.getAsyncState() == 0 do
		 * 	running = true
		 * end
		 * running = false
		 * @endcode
		 *
		 * @return 0 if running, 1 if successfully finished, -1 otherwise.
		 */
		int getAsyncState(void);
		
		/**
		 * Get result from a finished asynchronous task.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * if System.getAsyncState() == 1 then
		 * 	res = System.getAsyncResult()
		 * end
		 * @endcode
		 *
		 * @return A string with the result for Network.requestStringAsync() result, nil otherwise.
		 */
		string getAsyncResult(void);
		
		/**
		 * Take a screenshot.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.takeScreenshot("ux0:/data/shot.jpg", FORMAT_JPG, 255)
		 * @endcode
		 *
		 * @param filename - The filename of the screenshot output.
		 * @param format - The format to use for the output file <b>(optional)</b>.
		 * @param ratio - Compression ratio for JPG compression <b>(optional)</b>.
		 *
		 * @note <b>ratio</b> must be between 0 and 255.
		 */
		void takeScreenshot(string filename, ImgFmt format, int ratio);
		
		/**
		 * Execute an URI call.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.executeUri("psgm:play?titleid=MLCL00001")
		 * @endcode
		 *
		 * @param uri - URI to exec.
		 */
		void executeUri(string uri);
		
		/**
		 * Reboot the console.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.reboot()
		 * @endcode
		 */
		int reboot(void);
		
		/**
		 * Shutdown the console.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.shutdown()
		 * @endcode
		 */
		int shutdown(void);
		
		/**
		 * Put the console in standby.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.standby()
		 * @endcode
		 */
		int standby(void);
		
		/**
		 * Get if application is running in safe mode.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * if System.isSafeMode() then
		 * 		mode = "safe"
		 * end
		 * @endcode
		 *
		 * @return true if in safe mode, false otherwise.
		 */
		bool isSafeMode(void);
		
		/**
		 * Start a system message instance.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.setMessage("Do you want to start this app?", false, BUTTON_YES_NO)
		 * @endcode
		 *
		 * @param text - The text of the system message.
		 * @param progressbar - Progressbar mode.
		 * @param buttons - Buttons scheme to use <b>(optional)</b>.
		 *
		 * @note If <b>progressbar</b> is true, <b>buttons</b> will not affect the system message.
		 */
		void setMessage(string text, bool progressbar, BtnMode buttons);
		
		/**
		 * Get current system message instance state.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * state = System.getMessageState()
		 * @endcode
		 *
		 * @return Current state of the system message instance.
		 */
		DlgState getMessageState(void);
		
		/**
		 * Set current progress for the system message instance.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.setMessageProgress(10)
		 * @endcode
		 *
		 * @param progress - Progress percentage.
		 *
		 * @note This function affects only progressbar system messages.
		 * @note <b>progress</b> must be between 0 and 100.
		 */
		void setMessageProgress(int progress);
		
		/**
		 * Set a submessage for the system message instance.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.setMessageProgMsg("10%%")
		 * @endcode
		 *
		 * @param text - Submessage text to set.
		 *
		 * @note This function affects only progressbar system messages.
		 */
		void setMessageProgMsg(string text);
		
		/**
		 * Close the current system message instance.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.closeMessage()
		 * @endcode
		 */
		void closeMessage(void);
		
		/**
		 * Unmount an already mounted partition.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.unmountPartition(0x300)
		 * @endcode
		 *
		 * @param idx - The index number of the partition.
		 *
		 * @note For the index value, look at VSH Mount ID <a href="https://wiki.henkaku.xyz/vita/SceIofilemgr#Mount_Points">here</a>.
		 * @note This function is available only in unsafe mode.
		 */
		void unmountPartition(int idx);
		
		/**
		 * Mount an unmounted partition.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.mountPartition(0x300, READ_WRITE)
		 * @endcode
		 *
		 * @param idx - The index number of the partition.
		 * @param perms - Permissions to set for the mounted partition.
		 *
		 * @note For the index value, look at VSH Mount ID <a href="https://wiki.henkaku.xyz/vita/SceIofilemgr#Mount_Points">here</a>.
		 * @note This function is available only in unsafe mode.
		 */
		void mountPartition(int idx, MntPerm perms);
		
		/**
		 * Install an extracted app.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.installApp("ux0:/vitaQuake")
		 * @endcode
		 *
		 * @param dir - The path to the extracted app.
		 *
		 * @note This function is available only in unsafe mode.
		 */
		void installApp(string dir);
		
		/**
		 * Uninstall an installed app.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.uninstallApp("GTAVCECTY")
		 * @endcode
		 *
		 * @param titleid - The titleid of the app to uninstall.
		 *
		 * @note This function is available only in unsafe mode.
		 */
		void uninstallApp(string titleid);
		
		/**
		 * Check if an app is installed.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * if System.doesAppExist("GTAVCECTY") then
		 *  System.uninstallApp("GTAVCECTY")
		 * end
		 * @endcode
		 *
		 * @param titleid - The titleid of the app to check.
		 *
		 * @return true if installed, false otherwise.
		 *
		 * @note This function is available only in unsafe mode.
		 */
		bool doesAppExist(string titleid);
		
		/**
		 * Retrieve params used at app boot.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * args = System.getBootParams()
		 * @endcode
		 *
		 * @return The params passed to the app when it got launched.
		 */
		string getBootParams(void);
		
		/**
		 * Load and start an user plugin.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * plug_id = System.loadUserPlugin("ux0:data/plugin.suprx")
		 * @endcode
		 *
		 * @param path - The path to the plugin to load.
		 *
		 * @return An identifier to the loaded module.
		 *
		 * @note This function is available only in unsafe mode.
		 */
		int loadUserPlugin(string path);
		
		/**
		 * Load and start a kernel plugin.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * plug_id = System.loadKernelPlugin("ux0:data/plugin.skprx")
		 * @endcode
		 *
		 * @param path - The path to the plugin to load.
		 *
		 * @return An identifier to the loaded module.
		 *
		 * @note This function is available only in unsafe mode.
		 */
		int loadKernelPlugin(string path);
		
		/**
		 * Stop and unload an user plugin.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.unloadUserPlugin(plug_id)
		 * @endcode
		 *
		 * @param plug_id - The module identifier for the plugin to unload.
		 *
		 * @note This function is available only in unsafe mode.
		 */
		void unloadUserPlugin(int plug_id);
		
		/**
		 * Stop and unload a kernel plugin.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.unloadKernelPlugin(plug_id)
		 * @endcode
		 *
		 * @param plug_id - The module identifier for the plugin to unload.
		 *
		 * @note This function is available only in unsafe mode.
		 */
		void unloadUserPlugin(int plug_id);
		
		/**
		 * Unmount a virtual mountpoint.
		 * \ingroup System
		 *
		 * @par Usage example:
		 * @code
		 * System.unmountMountpoint("app0:")
		 * @endcode
		 *
		 * @param mnt - The mountpoint to unmount.
		 *
		 * @note This function is available only in unsafe mode.
		 */
		void unmountMountpoint(string mnt);

}
