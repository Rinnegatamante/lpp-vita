/**
 * \defgroup Camera
 * Module that handles photo cameras hardware.
 */

/**
 * Camera types.
 * \ingroup Camera
 */
enum CameraType{
	INNER_CAM, //!< Front camera
	OUTER_CAM //!< Retro camera
};

/**
 * Camera resolutions.
 * \ingroup Camera
 */
enum CameraRes{
	VGA_RES,     //!< VGA (640x480) resolution
	QVGA_RES,    //!< QVGA (320x240) resolution
	QQVGA_RES,   //!< QQVGA (160x120) resolution
	CIF_RES,     //!< CIF (352x288) resolution
	QCIF_RES,    //!< QCIF (176x144) resolution
	PSP_RES,     //!< PSP (480x272) resolution
	NGP_RES      //!< NGP (640x360) resolution
};

/**
 * Camera antiflicker modes.
 * \ingroup Camera
 */
enum CameraAntiFlick{
	ANTIFLICKER_AUTO,   //!< Automatic Antiflicker
	ANTIFLICKER_50HZ,   //!< 50Hz Antiflicker
	ANTIFLICKER_60HZ    //!< 60Hz Antiflicker
};

/**
 * Camera ISO speed mode.
 * \ingroup Camera
 */
enum CameraISO{
	ISO_AUTO,  //!< Automatic ISO speed mode
	ISO_100,   //!< 100/21 ISO speed mode
	ISO_200,   //!< 100/24 ISO speed mode
	ISO_400    //!< 100/27 ISO speed mode
};

/**
 * Camera White Balance mode.
 * \ingroup Camera
 */
enum CameraWB{
	WB_AUTO,       //!< Automatic White Balance mode
	WB_DAYLIGHT,   //!< Daylight White Balance mode
	WB_CWF,        //!< Cool White Fluorescent mode
	WB_SLSA        //!< Standard Light Source A mode
};

/**
 * Camera Backlight mode.
 * \ingroup Camera
 */
enum CameraBacklight{
	BACKLIGHT_OFF,  //!< Backlight disabled
	BACKLIGHT_ON    //!< Backlight enabled
};

/**
 * Camera Nightmode mode.
 * \ingroup Camera
 */
enum CameraNightmode{
	NIGHTMODE_OFF,   //!< Nightmode disabled
	NIGHTMODE_LOW,   //!< Nightmode for 10 lux or below
	NIGHTMODE_MED,   //!< Nightmode for 100 lux or below
	NIGHTMODE_HIGH   //!< Nightmode for over 100 lux
};

/**
 * Camera Reverse mode.
 * \ingroup Camera
 */
enum CameraReverse{
	REVERSE_OFF,    //!< Reverse mode disabled
	REVERSE_MIRROR, //!< Mirror reverse mode
	REVERSE_FLIP,   //!< Flip reverse mode
	REVERSE_BOTH    //!< Flip and Mirror reverse mode
};

/**
 * Camera effects.
 * \ingroup Camera
 */
enum CameraEffect{
	EFFECT_NONE,        //!< No effect
	EFFECT_NEGATIVE,    //!< Negative effect
	EFFECT_BLACKWHITE,  //!< Black and White effect
	EFFECT_SEPIA,       //!< Sepia effect
	EFFECT_BLUE,        //!< Blue Neon effect
	EFFECT_RED,         //!< Red Neon effect
	EFFECT_GREEN        //!< Green Neon effect
};

class Camera {
	
	public:
	
		/**
		 * Init camera hardware.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * Camera.init(INNER_CAM, VGA_RES, 30)
		 * @endcode
		 *
		 * @param type - Camera to init.
		 * @param resolution - Output resolution.
		 * @param framerate - Output framerate.
		 *
		 * @note <b>framerate</b> must be between 3 and 120.
		 * @note <b>framerate</b> can be set to 120 only with \link CameraRes::QVGA_RES QVGA_RES\endlink,\link CameraRes::QQVGA_RES QQVGA_RES\endlink or \link CameraRes::CIF_RES CIF_RES\endlink as <b>resolution</b>.
		 */
		void init(CameraType type, CameraRes resolution, int framerate);
		
		/**
		 * Terminate camera hardware.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * Camera.term()
		 * @endcode
		 */
		void term(void);
		
		/**
		 * Return current camera output as image.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * img = Camera.getOutput()
		 * @endcode
		 *
		 * @note The returned image became invalid when Camera.term is called.
		 */
		int getOutput(void);
		
		/**
		 * Set camera brightness.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * Camera.setBrightness(200)
		 * @endcode
		 *
		 * @param value - Value to set.
		 *
		 * @note <b>value</b> must be between 0 and 255.
		 */
		void setBrightness(int value);
		
		/**
		 * Get current camera brightness.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * brightness = Camera.getBrightness()
		 * @endcode
		 *
		 * @return The current camera brightness.
		 */
		int getBrightness(void);
		
		/**
		 * Set camera saturation.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * Camera.setSaturation(2.0)
		 * @endcode
		 *
		 * @param value - Value to set.
		 *
		 * @note <b>value</b> must be between 0.0 and 4.0.
		 */
		void setSaturation(number value);
		
		/**
		 * Get current camera saturation.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * saturation = Camera.getSaturation()
		 * @endcode
		 *
		 * @return The current camera saturation.
		 */
		int getSaturation(void);
		
		/**
		 * Set camera sharpness percentage.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * Camera.setSharpness(100)
		 * @endcode
		 *
		 * @param value - Value to set.
		 *
		 * @note <b>value</b> must be between 100 and 400.
		 */
		void setSharpness(int value);
		
		/**
		 * Get current camera sharpness.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * sharpness = Camera.getSharpness()
		 * @endcode
		 *
		 * @return The current camera sharpness.
		 */
		int getSharpness(void);
		
		/**
		 * Set camera contrast.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * Camera.setContrast(200)
		 * @endcode
		 *
		 * @param value - Value to set.
		 *
		 * @note <b>value</b> must be between 0 and 255.
		 */
		void setContrast(int value);
		
		/**
		 * Get current camera contrast.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * contrast = Camera.getBrightness()
		 * @endcode
		 *
		 * @return The current camera contrast.
		 */
		int getContrast(void);
		
		/**
		 * Set camera reverse mode.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * Camera.setReverse(REVERSE_FLIP)
		 * @endcode
		 *
		 * @param mode - Mode to set.
		 */
		void setReverse(CameraReverse mode);
		
		/**
		 * Get current camera reverse mode.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * mode = Camera.getReverse()
		 * @endcode
		 *
		 * @return The current camera reverse mode.
		 */
		CameraReverse getReverse(void);
		
		/**
		 * Set camera effect.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * Camera.setEffect(EFFECT_SEPIA)
		 * @endcode
		 *
		 * @param effect - Effect to set.
		 *
		 * @note <b>value</b> must be between 0 and 255.
		 */
		void setEffect(CameraEffect effect);
		
		/**
		 * Get current camera effect.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * effect = Camera.getEffect()
		 * @endcode
		 *
		 * @return The current camera effect.
		 */
		CameraEffect getEffect(void);
		
		/**
		 * Set camera zoom.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * Camera.setZoom(200)
		 * @endcode
		 *
		 * @param value - Value to set.
		 *
		 * @note <b>value</b> must be between 0 and 255.
		 */
		void setZoom(int value);
		
		/**
		 * Get current camera zoom.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * zoom = Camera.getZoom()
		 * @endcode
		 *
		 * @return The current camera zoom.
		 */
		int getZoom(void);
		
		/**
		 * Set camera antiflicker mode.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * Camera.setAntiFlicker(ANTIFLICKER_50HZ)
		 * @endcode
		 *
		 * @param mode - Mode to set.
		 */
		void setAntiFlicker(CameraAntiFlick mode);
		
		/**
		 * Get current camera antiflicker mode.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * mode = Camera.getAntiflicker()
		 * @endcode
		 *
		 * @return The current camera antiflicker mode.
		 */
		CameraAntiFlick getAntiFlicker(void);
		
		/**
		 * Set camera ISO speed mode.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * Camera.setISO(ISO_AUTO)
		 * @endcode
		 *
		 * @param mode - Mode to set.
		 */
		void setISO(CameraISO mode);
		
		/**
		 * Get current camera ISO speed mode.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * mode = Camera.getISO()
		 * @endcode
		 *
		 * @return The current camera ISO speed mode.
		 */
		CameraISO getISO(void);
		
		/**
		 * Set camera gain.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * Camera.setGain(5)
		 * @endcode
		 *
		 * @param value - Value to set.
		 *
		 * @note <b>value</b> must be between 0 and 16.
		 */
		void setGain(int value);
		
		/**
		 * Get current camera gain.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * gain = Camera.getGain()
		 * @endcode
		 *
		 * @return The current camera gain.
		 */
		int getGain(void);
		
		/**
		 * Set camera white balance mode.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * Camera.setWhiteBalance(WB_SLSA)
		 * @endcode
		 *
		 * @param mode - Mode to set.
		 */
		void setWhiteBalance(CameraWB mode);
		
		/**
		 * Get current camera white balance mode.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * mode = Camera.getWhiteBalance()
		 * @endcode
		 *
		 * @return The current camera white balance mode.
		 */
		CameraWB getWhiteBalance(void);
		
		/**
		 * Set camera backlight mode.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * Camera.setBacklight(BACKLIGHT_ON)
		 * @endcode
		 *
		 * @param mode - Mode to set.
		 */
		void setBacklight(CameraBacklight mode);
		
		/**
		 * Get current camera backlight mode.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * mode = Camera.getBacklight()
		 * @endcode
		 *
		 * @return The current camera backlight mode.
		 */
		CameraBacklight getBacklight(void);
		
		/**
		 * Set camera nightmode mode.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * Camera.setNightmode(NIGHTMODE_LOW)
		 * @endcode
		 *
		 * @param mode - Mode to set.
		 */
		void setNightmode(CameraNightmode mode);
		
		/**
		 * Get current camera nightmode mode.
		 * \ingroup Camera
		 *
		 * @par Usage example:
		 * @code
		 * mode = Camera.getNightmode()
		 * @endcode
		 *
		 * @return The current camera nightmode mode.
		 */
		CameraNightmode getNightmode(void);
	
}