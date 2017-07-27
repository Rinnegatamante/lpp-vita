/**
 * \defgroup Color
 * Module that handles colors and their management.
 */ 
 
class Color {
	
	public:
	
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
		int new(int r, int g, int b, int a);
		
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
		int getR(int clr);
		
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
		int getG(int clr);
		
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
		int getB(int clr);
		
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
		int getA(int clr); 

}
