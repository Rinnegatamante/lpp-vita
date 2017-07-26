/**
 * \defgroup Color
 * Module that handles colors and their management.
 */ 
 
public class Color {

	/**
	 * Create a new color.
	 * \ingroup Color
	 *
	 * @par Usage example:
	 * @code
	 * my_color = Color.new(255, 200, 100, 255)
	 * @endcode
	 *
	 * @param r - R channel value.
	 * @param g - G channel value.
	 * @param b - B channel value.
	 * @param a - A channel value <B>(optional)</B>.
	 *
	 * @return The combined RGBA color.
	 */
	public int new(int r, int g, int b, int a){}
	
	/**
	 * Return the R channel value of a color.
	 * \ingroup Color
	 *
	 * @par Usage example:
	 * @code
	 * r = Color.getR(my_color)
	 * @endcode
	 *
	 * @param clr - A color created with ::Color.new.
	 *
	 * @return The R channel value of the color.
	 */
	public int getR(int clr){}
	
	/**
	 * Return the G channel value of a color.
	 * \ingroup Color
	 *
	 * @par Usage example:
	 * @code
	 * g = Color.getG(my_color)
	 * @endcode
	 *
	 * @param clr - A color created with ::Color.new.
	 *
	 * @return The G channel value of the color.
	 */
	public int getG(int clr){}
	
	/**
	 * Return the B channel value of a color.
	 * \ingroup Color
	 *
	 * @par Usage example:
	 * @code
	 * b = Color.getB(my_color)
	 * @endcode
	 *
	 * @param clr - A color created with ::Color.new.
	 *
	 * @return The B channel value of the color.
	 */
	public int getB(int clr){}
	
	/**
	 * Return the A channel value of a color.
	 * \ingroup Color
	 *
	 * @par Usage example:
	 * @code
	 * a = Color.getA(my_color)
	 * @endcode
	 *
	 * @param clr - A color created with ::Color.new.
	 *
	 * @return The A channel value of the color.
	 */
	public int getA(int clr){} 

}

/**
 * \defgroup Screen
 * Module that handles basic screen related stuffs.
 */ 

public class Screen {

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
	public void clear(int clr){}
	
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
	public void flip(void){}
	
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
	public int getPixel(int x, int y){}
	
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
	public void waitVblankStart(void){}

}