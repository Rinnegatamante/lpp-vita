/**
 * \defgroup Font
 * Module that handles font writing stuffs.
 */
 
class Font {
	
	public:
	
		/**
		 * Load a .ttf/.pgf/.pvf font file.
		 * \ingroup Font
		 *
		 * @par Usage example:
		 * @code
		 * fnt = Font.load("app0:/font.ttf")
		 * @endcode
		 *
		 * @param filename - The name of the font file
		 *
		 * @return A valid font ID.
		 */
		int load(string filename);
		
		/**
		 * Unload a loaded font.
		 * \ingroup Font
		 *
		 * @par Usage example:
		 * @code
		 * Font.unload(fnt)
		 * @endcode
		 *
		 * @param font - A valid font loaded with Font.load.
		 */
		void unload(int font);
		
		/**
		 * Set font size for drawing.
		 * \ingroup Font
		 *
		 * @par Usage example:
		 * @code
		 * Font.setPixelSizes(fnt, 20)
		 * @endcode
		 *
		 * @param font - A valid font loaded with Font.load.
		 * @param size - Size to set for Font.print calls in pixels.
		 */
		void setPixelSizes(int font, int size);
		
		/**
		 * Calculate width for a given text and a given font.
		 * \ingroup Font
		 *
		 * @par Usage example:
		 * @code
		 * width = Font.getTextWidth(fnt, "Hello World")
		 * @endcode
		 *
		 * @param font - A valid font loaded with Font.load.
		 * @param text - Text to calculate width of.
		 *
		 * @return The text width in pixels.
		 */
		int getTextWidth(int font, string text);
		
		/**
		 * Calculate height for a given text and a given font.
		 * \ingroup Font
		 *
		 * @par Usage example:
		 * @code
		 * width = Font.getTextHeight(fnt, "Hello World")
		 * @endcode
		 *
		 * @param font - A valid font loaded with Font.load.
		 * @param text - Text to calculate width of.
		 *
		 * @return The text height in pixels.
		 */
		int getTextHeight(int font, string text);
		
		/**
		 * Print a text on screen using a font.
		 * \ingroup Font
		 *
		 * @par Usage example:
		 * @code
		 * Font.print(fnt, 5, 10, "Hello World", Color.new(255, 255, 255))
		 * @endcode
		 *
		 * @param font - A valid font loaded with Font.load.
		 * @param x - X starting coordinate for the print.
		 * @param y - Y starting coordinate for the print.
		 * @param text - Text to print.
		 * @param color - Color of the text (See ::Color).
		 */
		void print(int font, number x, number y, string text, int color);
			
}