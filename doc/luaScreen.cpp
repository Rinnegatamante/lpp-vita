/**
 * \defgroup Screen
 * Module that handles basic screen related stuffs.
 */ 

class Screen {
	
	public:

		/**
		 * Clear the screen framebuffer.
		 * \ingroup Screen
		 *
		 * @par Usage example:
		 * @code
		 * Screen.clear()
		 * @endcode
		 *
		 * @param clr - Color of the screen after the clear <B>(optional)</B>.
		 *
		 * @note If clr is passed, that color will be used for successive calls too.
		 * @note This function must be called during rendering phase (See ::Graphics).
		 */
		void clear(int clr){}
		
		/**
		 * Flip the screen.
		 * \ingroup Screen
		 *
		 * @par Usage example:
		 * @code
		 * Screen.flip()
		 * @endcode
		 *
		 */
		void flip(void){}
		
		/**
		 * Get a pixel color from the screen framebuffer.
		 * \ingroup Screen
		 *
		 * @par Usage example:
		 * @code
		 * pixel_color = Screen.getPixel(5, 25)
		 * @endcode
		 *
		 * @param x - X coordinate of the pixel.
		 * @param y - Y coordinate of the pixel.
		 *
		 * @return The pixel color value (See ::Color).
		 */
		int getPixel(int x, int y){}
		
		/**
		 * Wait screen vertical synchronization (VSync).
		 * \ingroup Screen
		 *
		 * @par Usage example:
		 * @code
		 * Screen.waitVblankStart()
		 * @endcode
		 *
		 */
		void waitVblankStart(void){}

}