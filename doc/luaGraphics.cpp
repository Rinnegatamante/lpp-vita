/**
 * \defgroup Graphics
 * Module that handles 2D rendering.
 */ 

/**
 * Image filters to use with ::Graphics.setImageFilters.
 * \ingroup Graphics
 */
enum ImageFilter{
	FILTER_POINT,       //!< Point filter
	FILTER_LINEAR       //!< Linear filter
};
 
class Graphics {
	
	public:

		/**
		 * Initialize drawing phase.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * Graphics.initBlend()
		 * @endcode
		 */
		void initBlend(void);
		
		/**
		 * Terminate drawing phase.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * Graphics.termBlend()
		 * @endcode
		 */
		void termBlend(void);
		
		/**
		 * Print a text on screen using system font.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * Graphics.debugPrint(10, 10, "Hello World", Color.new(255, 255, 255))
		 * @endcode
		 *
		 * @param x - X coordinate of the text position in pixels.
		 * @param y - Y coordinate of the text position in pixels.
		 * @param text - Text to print.
		 * @param color - A valid color (See ::Color).
		 */
		void debugPrint(number x, number y, string text, int color);
		
		/**
		 * Draw a pixel.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * Graphics.drawPixel(5, 5, Color.new(15, 255, 200))
		 * @endcode
		 *
		 * @param x - X coordinate of the pixel position in pixels.
		 * @param y - Y coordinate of the pixel position in pixels.
		 * @param color - A valid color (See ::Color).
		 * @param image - Image to draw on <b>(optional)</b>.
		 *
		 * @note If <b>image</b> is not provided, pixel will be drawn on screen.
		 */
		void drawPixel(number x, number y, int color, int image);
		
		/**
		 * Get a pixel color from a loaded image.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * pixel_color = Graphics.getPixel(5, 25, img)
		 * @endcode
		 *
		 * @param x - X coordinate of the pixel.
		 * @param y - Y coordinate of the pixel.
		 * @param img - A valid image ID.
		 *
		 * @return The pixel color value (See ::Color).
		 */
		int getPixel(int x, int y, int img);
		
		/**
		 * Draw a line.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * Graphics.drawLine(10, 25, 100, 250, Color.new(25, 200, 120))
		 * @endcode
		 *
		 * @param x1 - Starting X coordinate of the line in pixels.
		 * @param x2 - Ending X coordinate of the line in pixels.
		 * @param y1 - Starting Y coordinate of the line in pixels.
		 * @param y2 - Ending Y coordinate of the line in pixels.
		 * @param color - A valid color (See ::Color).
		 */
		void drawLine(number x1, number x2, number y1, number y2, int color);
		
		/**
		 * Draw a rectangle.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * Graphics.fillRect(10, 25, 100, 250, Color.new(25, 200, 120))
		 * @endcode
		 *
		 * @param x1 - Starting X coordinate of the rectangle in pixels.
		 * @param x2 - Ending X coordinate of the rectangle in pixels.
		 * @param y1 - Starting Y coordinate of the rectangle in pixels.
		 * @param y2 - Ending Y coordinate of the rectangle in pixels.
		 * @param color - A valid color (See ::Color).
		 */
		void fillRect(number x1, number x2, number y1, number y2, int color);
		
		/**
		 * Draw an empty rectangle.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * Graphics.fillEmptyRect(10, 25, 100, 250, Color.new(25, 200, 120))
		 * @endcode
		 *
		 * @param x1 - Starting X coordinate of the rectangle in pixels.
		 * @param x2 - Ending X coordinate of the rectangle in pixels.
		 * @param y1 - Starting Y coordinate of the rectangle in pixels.
		 * @param y2 - Ending Y coordinate of the rectangle in pixels.
		 * @param color - A valid color (See ::Color).
		 */
		void fillEmptyRect(number x1, number x2, number y1, number y2, int color);
		
		/**
		 * Draw a circle.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * Graphics.fillCircle(10, 25, 1.0, Color.new(25, 200, 120))
		 * @endcode
		 *
		 * @param x - X coordinate of the circle in pixels.
		 * @param y - Y coordinate of the circle in pixels.
		 * @param rad - Radius size of the circle.
		 * @param color - A valid color (See ::Color).
		 */
		void fillCircle(number x, number y, number rad, int color);
		
		/**
		 * Create an empty image.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * img = Graphics.createImage(Color.new(25, 200, 120))
		 * @endcode
		 *
		 * @param width - Image width.
		 * @param height - Image height.
		 * @param color - A valid color (See ::Color) <b>(optional)</b>.
		 *
		 * @return A valid image ID.
		 */
		int createImage(int width, int height, int color);
		
		/**
		 * Load a .png/.jpg/.bmp image.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * img = Graphics.loadImage("app0:/image.jpg")
		 * @endcode
		 *
		 * @param filename - Name of the file to open.
		 *
		 * @return A valid image ID.
		 */
		int loadImage(string filename);
		
		/**
		 * Save a loaded image to a .png/.bmp image.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * Graphics.saveImage(img, "ux0:/data/image.png", FORMAT_PNG)
		 * @endcode
		 *
		 * @param img - A valid image ID.
		 * @param filename - The filename of the screenshot output.
		 * @param format - The format to use for the output file <b>(optional)</b>.
		 *
		 * @note <b>FORMAT_JPG</b> is not supported.
		 */
		void saveImage(int img, string filename, ImgFmt format);
		
		/**
		 * Load a .png/.jpg/.bmp image (asynchronous).
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * Graphics.loadImageAsync("app0:/image.jpg")
		 * while System.getAsyncState() == 0 do
		 * 
		 * end
		 * img = System.getAsyncResult()
		 * @endcode
		 *
		 * @param filename - Name of the file to open.
		 */
		void loadImageAsync(string filename);

		/**
		 * Load a .gif animated image.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * img = Graphics.loadAnimatedImage("app0:/image.gif")
		 * @endcode
		 *
		 * @param filename - Name of the file to open.
		 *
		 * @return A valid image ID.
		 */
		int loadAnimatedImage(string filename);
		
		/**
		 * Gets the number of frames of a loaded animated image.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * img = Graphics.loadAnimatedImage("app0:/image.gif")
		 * frames = Graphics.getImageFramesNum(img)
		 * @endcode
		 *
		 * @param img - A valid image ID.
		 *
		 * @return The number of frames for the given image.
		 */
		int getImageFramesNum(int img);
		
		/**
		 * Set current active frame for a loaded animated image.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * img = Graphics.loadAnimatedImage("app0:/image.gif")
		 * Graphics.setImageFrame(img, 8)
		 * @endcode
		 *
		 * @param img - A valid image ID.
		 * @param frame - The frame to set as active.
		 */
		void setImageFrame(int img, int frame);
		
		/**
		 * Free a loaded image.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * Graphics.freeImage(img)
		 * @endcode
		 *
		 * @param img - A valid image ID.
		 */
		void freeImage(int img);
		
		/**
		 * Set filters to use for a given image.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * Graphics.setImageFilters(img, FILTER_LINEAR, FILTER_LINEAR)
		 * @endcode
		 *
		 * @param img - A valid image ID.
		 * @param min_filter - Min filter to use.
		 * @param mag_filter - Mag filter to use.
		 */
		void setImageFilters(int img, ImageFilter min_filter, ImageFilter mag_filter);
		
		/**
		 * Get an image width.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * width = Graphics.getImageWidth(img)
		 * @endcode
		 *
		 * @param img - A valid image ID.
		 * 
		 * @return The image width in pixels.
		 */
		int getImageWidth(int img);
		
		/**
		 * Get an image height.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * height = Graphics.getImageHeight(img)
		 * @endcode
		 *
		 * @param img - A valid image ID.
		 * 
		 * @return The image height in pixels.
		 */
		int getImageHeight(int img);
		
		/**
		 * Draw an image.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * Graphics.drawImage(50, 100, img)
		 * @endcode
		 *
		 * @param x - X coordinate of the image in pixels.
		 * @param y - Y coordinate of the image in pixels.
		 * @param img - A valid image ID.
		 * @param color - Image tint color (See ::Color) <b>(optional)</b>.
		 *
		 */
		void drawImage(number x, number y, int img, int color);
		
		/**
		 * Draw a rotated image.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * Graphics.drawRotateImage(50, 100, img, 1.0)
		 * @endcode
		 *
		 * @param x - X coordinate of the image in pixels.
		 * @param y - Y coordinate of the image in pixels.
		 * @param img - A valid image ID.
		 * @param rad - Rotation radius.
		 * @param color - Image tint color (See ::Color) <b>(optional)</b>.
		 *
		 */
		void drawRotateImage(number x, number y, int img, number rad, int color);
		
		/**
		 * Draw a scaled image.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * Graphics.drawScaleImage(50, 100, img, 0.5, 0.5)
		 * @endcode
		 *
		 * @param x - X coordinate of the image in pixels.
		 * @param y - Y coordinate of the image in pixels.
		 * @param img - A valid image ID.
		 * @param x_scale - Scale value for X parameter.
		 * @param y_scale - Scale value for Y parameter.
		 * @param color - Image tint color (See ::Color) <b>(optional)</b>.
		 *
		 */
		void drawScaleImage(number x, number y, int img, number x_scale, number y_scale, int color);
		
		/**
		 * Draw a part of an image.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * Graphics.drawPartialImage(50, 100, img, 20, 20, 100, 100)
		 * @endcode
		 *
		 * @param x - X coordinate of the image in pixels.
		 * @param y - Y coordinate of the image in pixels.
		 * @param img - A valid image ID.
		 * @param x_start - Image X coordinate for the partial drawing.
		 * @param y_start - Image Y coordinate for the partial drawing.
		 * @param width - Partial drawing width.
		 * @param height - Partial drawing height.
		 * @param color - Image tint color (See ::Color) <b>(optional)</b>.
		 *
		 */
		void drawPartialImage(number x, number y, int img, int x_start, int y_start, number width, number height, int color);
		
		/**
		 * Draw an image with several effects.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * Graphics.drawImageExtended(50, 100, img, 20, 20, 100, 100, 1.0, 0.5, 0.5)
		 * @endcode
		 *
		 * @param x - X coordinate of the image in pixels.
		 * @param y - Y coordinate of the image in pixels.
		 * @param img - A valid image ID.
		 * @param x_start - Image X coordinate for the partial drawing.
		 * @param y_start - Image Y coordinate for the partial drawing.
		 * @param width - Partial drawing width.
		 * @param height - Partial drawing height.
		 * @param rad - Rotation radius.
		 * @param x_scale - Scale value for X parameter.
		 * @param y_scale - Scale value for Y parameter.
		 * @param color - Image tint color (See ::Color) <b>(optional)</b>.
		 *
		 */
		void drawImageExtended(number x, number y, int img, int x_start, int y_start, number width, number height, number rad, number x_scale, number y_scale, int color);
		
		/**
		 * Initialize a rescaler.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * Graphics.initRescaler(0, 0, 2.0, 2.0)
		 * @endcode
		 *
		 * @param x - X coordinate of the rescaler output.
		 * @param y - Y coordinate of the rescaler output.
		 * @param x_scale - Scale value for X parameter.
		 * @param y_scale - Scale value for Y parameter.
		 *
		 */
		void initRescaler(int x, int y, number x_scale, number y_scale);
		
		/**
		 * Terminate a rescaler.
		 * \ingroup Graphics
		 *
		 * @par Usage example:
		 * @code
		 * Graphics.termRescaler()
		 * @endcode
		 */
		void termRescaler(void);
		
}