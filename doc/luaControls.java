/**
 * \defgroup Controls
 * Module that handles physical user input systems.
 */
 
/**
 * Device types. Possible results of ::Controls.getDeviceInfo.
 * \ingroup Controls
 */
public enum DevType{
	UNPAIRED_DEV,     //!< Unpaired device
	VITA_DEV,         //!< PSVITA controller device
	VIRTUAL_DEV,      //!< Virtual controller device
	DS3_DEV,          //!< Dualshock 3 device
	DS4_DEV           //!< Dualshock 4 device
}

/**
 * Available controls value.
 * \ingroup Controls
 *
 * @note <b>SCE_CTRL_VOLUP</b>, <b>SCE_CTRL_VOLDOWN</b> and <b>SCE_CTRL_POWER</b> can be intercepted only with unsafe mode.
 */
public enum Ctrl{
	SCE_CTRL_UP,        //!< Digital Up button
	SCE_CTRL_DOWN,      //!< Digital Down button
	SCE_CTRL_LEFT,      //!< Digital Left button
	SCE_CTRL_RIGHT,     //!< Digital Right button
	SCE_CTRL_CROSS,     //!< Cross button
	SCE_CTRL_CIRCLE,    //!< Circle button
	SCE_CTRL_SQUARE,    //!< Square button
	SCE_CTRL_TRIANGLE,  //!< Triangle button
	SCE_CTRL_LTRIGGER,  //!< L Trigger button
	SCE_CTRL_RTRIGGER,  //!< R Trigger button
	SCE_CTRL_START,     //!< Start button
	SCE_CTRL_SELECT,    //!< Select button
	SCE_CTRL_POWER,     //!< Power button
	SCE_CTRL_VOLUP,     //!< Volume Up button
	SCE_CTRL_VOLDOWN,   //!< Volume Down button
	SCE_CTRL_PSBUTTON   //!< PS button
}
 
public class Controls {
	
	/**
	 * Read pressed buttons.
	 * \ingroup Controls
	 *
	 * @par Usage example:
	 * @code
	 * pad = Controls.read()
	 * @endcode
	 *
	 * @return Bitmask of the pressed buttons.
	 */
	public int read(void){}
	
	/**
	 * Read left analog state.
	 * \ingroup Controls
	 *
	 * @par Usage example:
	 * @code
	 * x, y = Controls.readLeftAnalog()
	 * @endcode
	 *
	 * @return X and Y values of left analog.
	 */
	public int[] readLeftAnalog(void){}
	
	/**
	 * Read right analog state.
	 * \ingroup Controls
	 *
	 * @par Usage example:
	 * @code
	 * x, y = Controls.readRightAnalog()
	 * @endcode
	 *
	 * @return X and Y values of right analog.
	 */
	public int[] readRightAnalog(void){}
	
	/**
	 * Read touchscreen state.
	 * \ingroup Controls
	 *
	 * @par Usage example:
	 * @code
	 * x1, y1, x2, y2 = Controls.readTouch()
	 * @endcode
	 *
	 * @return Several sets of X and Y coordinates for every touch report.
	 * 
	 * @note A maximum of 6 different touch reports can be detected.
	 */
	public int[] readTouch(void){}
	
	/**
	 * Read rearpad state.
	 * \ingroup Controls
	 *
	 * @par Usage example:
	 * @code
	 * x1, y1, x2, y2 = Controls.readRetroTouch()
	 * @endcode
	 *
	 * @return Several sets of X and Y coordinates for every touch report.
	 * 
	 * @note A maximum of 4 different touch reports can be detected.
	 */
	public int[] readRetroTouch(void){}
	
	/**
	 * Check if a button is pressed.
	 * \ingroup Controls
	 *
	 * @par Usage example:
	 * @code
	 * pad = Controls.read()
	 * if Controls.check(pad, SCE_CTRL_CROSS) then
	 * 	pressed = true
	 * end
	 * @endcode
	 *
	 * @param bitmask - A controls bitmask returned by ::Controls.read.
	 * @param value - A control value to check.
	 *
	 * @return true if pressed, false otherwise.
	 */
	public bool check(int bitmask, Ctrl value){}
	
	/**
	 * Changes vibration intensity for Dualshock devices.
	 * \ingroup Controls
	 *
	 * @par Usage example:
	 * @code
	 * Controls.rumble(0, 100, 100)
	 * @endcode
	 *
	 * @param port - Device port to use.
	 * @param small - Intensity for small sensor.
	 * @param large - Intensity for large sensor.
	 */
	public void rumble(int port, int small, int large){}
	
	/**
	 * Set lightbar color for Dualshock 4 devices.
	 * \ingroup Controls
	 *
	 * @par Usage example:
	 * @code
	 * Controls.setLightbar(0, Color.new(255, 255, 255))
	 * @endcode
	 *
	 * @param port - Device port to use.
	 * @param color - An RGBA color value (See ::Color).
	 */
	public void setLightbar(int port, int color){}
	
	/**
	 * Locks default PS button functionality.
	 * \ingroup Controls
	 *
	 * @par Usage example:
	 * @code
	 * Controls.lockHomeButton()
	 * @endcode
	 */
	public void lockHomeButton(void){}
	
	/**
	 * Unlocks default PS button functionality.
	 * \ingroup Controls
	 *
	 * @par Usage example:
	 * @code
	 * Controls.lockHomeButton()
	 * @endcode
	 */
	public void unlockHomeButton(void){}
	
	/**
	 * Get info about controllers.
	 * \ingroup Controls
	 *
	 * @par Usage example:
	 * @code
	 * ctrl = Controls.getDeviceInfo()
	 * ctrl_type = ctrl[1].type
	 * @endcode
	 *
	 * @return A table showing info about the 5 available controllers (See the example above).
	 */
	public table getDeviceInfo(void){}
	
	/**
	 * Check if an headset is plugged in.
	 * \ingroup Controls
	 *
	 * @par Usage example:
	 * @code
	 * if Controls.headsetStatus() then
	 * 	headset = true
	 * end
	 * @endcode
	 *
	 * @return true if plugged in, false otherwise.
	 *
	 * @note This function is available only in unsafe mode.
	 */
	public bool headsetStatus(void){}
	
	/**
	 * Return set controller enter button.
	 * \ingroup Controls
	 *
	 * @par Usage example:
	 * @code
	 * pad = Controls.read()
	 * SCE_CTRL_CONFIRM = Controls.getEnterButton()
	 * if Controls.check(pad, SCE_CTRL_CONFIRM) then
	 * 	accepted = yes
	 * end
	 * @endcode
	 *
	 * @return \link Ctrl::SCE_CTRL_CROSS SCE_CTRL_CROSS\endlink or \link Ctrl::SCE_CTRL_CIRCLE SCE_CTRL_CIRCLE\endlink.
	 */
	public Ctrl getEnterButton(void){}
	
	/**
	 * Enable gyroscope sensor.
	 * \ingroup Controls
	 *
	 * @par Usage example:
	 * @code
	 * Controls.enableGyro()
	 * @endcode
	 */
	public void enableGyro(void){}
	
	/**
	 * Enable accelerometer sensor.
	 * \ingroup Controls
	 *
	 * @par Usage example:
	 * @code
	 * Controls.enableAccel()
	 * @endcode
	 */
	public void enableAccel(void){}
	
	/**
	 * Disable gyroscope sensor.
	 * \ingroup Controls
	 *
	 * @par Usage example:
	 * @code
	 * Controls.disableGyro()
	 * @endcode
	 */
	public void disableGyro(void){}
	
	/**
	 * Disable accelerometer sensor.
	 * \ingroup Controls
	 *
	 * @par Usage example:
	 * @code
	 * Controls.disableAccel()
	 * @endcode
	 */
	public void disableAccel(void){}
	
	/**
	 * Read gyroscope state.
	 * \ingroup Controls
	 *
	 * @par Usage example:
	 * @code
	 * x, y, z = Controls.readGyro()
	 * @endcode
	 *
	 * @return X, Y and Z value of gyroscope state.
	 */
	public number[] readGyro(void){}
	
	/**
	 * Read accelerometer state.
	 * \ingroup Controls
	 *
	 * @par Usage example:
	 * @code
	 * x, y, z = Controls.readAccel()
	 * @endcode
	 *
	 * @return X, Y and Z value of accelerometer state.
	 */
	public number[] readAccel(void){}
	
}