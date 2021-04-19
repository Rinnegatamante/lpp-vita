/**
 * \defgroup Gui
 * Module that handles 2D GUI development through dear ImGui wrapper.
 */

/**
 * Themes constants to use with ::Gui.setTheme.
 * \ingroup Gui
 */
enum GuiTheme{
	DARK_THEME,     //!< Dark Theme.
	LIGHT_THEME,    //!< Light Theme.
	CLASSIC_THEME   //!< Classic Theme
};

/**
 * Window flags to use with ::Gui.initWindow.
 * \ingroup Gui
 */
enum WinFlags{
	FLAG_NONE,                 //!< No flags.
	FLAG_NO_COLLAPSE,          //!< The window can't be collapsed.
	FLAG_NO_MOVE,              //!< The window can't be moved.
	FLAG_NO_RESIZE,            //!< The window can't be resized.
	FLAG_NO_SCROLLBAR,         //!< The window has no vertical scrollbar.
	FLAG_NO_TITLEBAR,          //!< The window doesn't show a label.
	FLAG_HORIZONTAL_SCROLLBAR  //!< The window has an horizontal scrollbar.
};

/**
 * Function mode to alter frequency of execution.
 * \ingroup Gui
 */
enum ConfigMode{
	SET_ONCE,   //!< The function is executed only once.
	SET_ALWAYS  //!< The function is executed at every frame.
};

class Gui {
	
	public:
		
		/**
		 * Init Gui sub-system.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * Gui.init()
		 * @endcode
		 */
		void init(void);
		
		/**
		 * Terminate Gui sub-system.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * Gui.term()
		 * @endcode
		 */
		void term(void);
		
		/**
		 * Init Gui drawing phase.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * Gui.initBlend()
		 * @endcode
		 */
		void initBlend(void);
		
		/**
		 * Terminate Gui drawing phase.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * Gui.termBlend()
		 * @endcode
		 */
		void termBlend(void);
		
		/**
		 * Change theme for the running Gui.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * Gui.setTheme(DARK_THEME)
		 * @endcode
		 *
		 * @param theme - The theme to set.
		 */
		void setTheme(GuiTheme theme);
		
		/**
		 * Change input mode for the running Gui.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * Gui.setInputMode(true, false, true, true)
		 * @endcode
		 *
		 * @param use_touch - Enable front touch usage for cursor movement.
		 * @param use_rear - Enable rearpad touch usage for cursor movement.
		 * @param use_buttons - Enable buttons usage for cursor movement.
		 * @param indirect_touch - If enabled, cursor is moved by dragging it. If disabled, cursor jumps to the touched location.
		 */
		void setInputMode(bool use_touch, bool use_rear, bool use_buttons, bool indirect_touch);
		
		/**
		 * Init screen menubar.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * if Gui.initMainMenubar() then
		 *  Gui.termMainMenubar()
		 * end
		 * @endcode
		 *
		 * @return true if the menubar is opened, false otherwise.
		 */
		bool initMainMenubar(void);
		
		/**
		 * Terminate screen menubar.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * if Gui.initMainMenubar() then
		 *  Gui.termMainMenubar()
		 * end
		 * @endcode
		 */
		void termMainMenubar(void);
		
		/**
		 * Init a menu.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * if Gui.initMenu("My Menu") then
		 *  Gui.termMenu()
		 * end
		 * @endcode
		 *
		 * @param label - The label to show.
		 * @param enabled - The menu status to set <b>(optional)</b>.
		 *
		 * @return true if the menu is opened, false otherwise.
		 */
		bool initMenu(string label, bool enabled);
		
		/**
		 * Terminate a menu.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * if Gui.initMenu() then
		 *  Gui.termMenu()
		 * end
		 * @endcode
		 */
		void termMenu(void);
		
		/**
		 * Draw a menu item.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * if Gui.drawMenuItem("Feature", is_enabled) then
		 *  is_enabled = true
		 * end
		 * @endcode
		 * @param label - The label to show.
		 * @param selected - The item checked status <b>(optional)</b>.
		 * @param enabled - The item status to set <b>(optional)</b>.
		 *
		 * @return true if clicked, false otherwise.
		 */
		bool drawMenuItem(string label, bool selected, bool enabled);
		
		/**
		 * Draw a text.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * Gui.drawText("Hello World", Color.new(255, 255, 0))
		 * @endcode
		 *
		 * @param label - The label to show.
		 * @param color - A valid color (See ::Color) <b>(optional)</b>.
		 */
		void drawText(string label, int color);
		
		/**
		 * Calculate a text size.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * w, h = Gui.getTextSize("Hello World")
		 * end
		 * @endcode
		 *
		 * @param text - The text to calculate the size about.
		 *
		 * @return The size of the text in pixels.
		 */
		number[] getTextSize(string text);
		
		/**
		 * Draw a greyed out text.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * Gui.drawDisabledText("Hello World")
		 * @endcode
		 *
		 * @param label - The label to show.
		 */
		void drawDisabledText(string label);
		
		/**
		 * Draw a text with automatic newlines to fit the window.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * Gui.drawWrappedText("Hello World")
		 * @endcode
		 *
		 * @param label - The label to show.
		 */
		void drawWrappedText(string label);
		
		/**
		 * Draw a button.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * if Gui.drawButton("Exit") then
		 *  System.exit()
		 * end
		 * @endcode
		 *
		 * @param label - The label to show.
		 * @param width - The width of the button <b>(optional)</b>.
		 * @param height - The height of the button <b>(optional)</b>.
		 *
		 * @return true if the button has been clicked, false otherwise.
		 */
		bool drawButton(string label, number width, number height);
		
		/**
		 * Draw a small button.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * if Gui.drawSmallButton("Exit") then
		 *  System.exit()
		 * end
		 * @endcode
		 *
		 * @param label - The label to show.
		 *
		 * @return true if the button has been clicked, false otherwise.
		 */
		bool drawSmallButton(string label);
		
		/**
		 * Draw a checkbox.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * is_checked = Gui.drawCheckbox("Check", is_checked)
		 * @endcode
		 *
		 * @param label - The label to show.
		 * @param status - The initial checked status of the checkbox.
		 *
		 * @return true if the checkbox is checked, false otherwise.
		 */
		bool drawCheckbox(string label, bool status);
		
		/**
		 * Draw a radiobutton.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * is_checked = Gui.drawRadiobutton("Check", is_checked)
		 * @endcode
		 *
		 * @param label - The label to show.
		 * @param status - The initial checked status of the radiobutton.
		 *
		 * @return true if the radiobutton is checked, false otherwise.
		 */
		bool drawRadioButton(string label, bool status);
		
		/**
		 * Make next element be drawn on current line.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * Gui.resetLine()
		 * @endcode
		 */
		void resetLine(void);
		
		/**
		 * Init a window.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * Gui.initWindow("Window", FLAG_NO_MOVE + FLAG_NO_RESIZE + FLAG_NO_TITLEBAR + FLAG_NO_COLLAPSE)
		 * @endcode
		 *
		 * @param label - The label to show.
		 * @param flags - The flags to use with the window.
		 */
		void initWindow(string label, WinFlags flags);
		
		/**
		 * Terminate a window.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * Gui.termWindow()
		 * @endcode
		 */
		void termWindow(void);
		
		/**
		 * Set next window position.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * Gui.setWindowPos(100, 100, SET_ALWAYS)
		 * @endcode
		 *
		 * @param x - X coordinate of the window position in pixels.
		 * @param y - Y coordinate of the window position in pixels.
		 * @param mode - A mode to use for the function.
		 */
		void setWindowPos(number x, number y, ConfigMode mode);
		
		/**
		 * Set next window size.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * Gui.setWindowSize(100, 100, SET_ALWAYS)
		 * @endcode
		 *
		 * @param w -Width of the window in pixels.
		 * @param h - Height of the window in pixels.
		 * @param mode - A mode to use for the function.
		 */
		void setWindowSize(number w, number h, ConfigMode mode);
		
		/**
		 * Draw a separator.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * Gui.drawSeparator()
		 * @endcode
		 */
		void drawSeparator(void);
		
		/**
		 * Draw a tooltip when cursor hover on the previous item.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * Gui.drawText("Text")
		 * Gui.drawTooltip("This is a textbox")
		 * @endcode
		 *
		 * @param label - The label to show.
		 */
		void drawTooltip(string label);
		
		/**
		 * Draw a slider with variable amount of values.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * slider_val = Gui.drawSlider("A slider", 0, 100, slider_val)
		 * @endcode
		 *
		 * @param label - The label to show.
		 * @param val_min - The minimum value settable through the slider.
		 * @param val_max - The maximum value settable through the slider.
		 * @param val1 - The first value to handle with the slider.
		 * @param val2 - The second value to handle with the slider <b>(optional)</b>.
		 * @param val3 - The third value to handle with the slider <b>(optional)</b>.
		 * @param val4 - The forth value to handle with the slider <b>(optional)</b>.
		 *
		 * @return The updated values after the frame execution.
		 */
		number[] drawSlider(string label, number val_min, number val_max, number val1, number val2, number val3, number val4);
		
		/**
		 * Draw a slider with variable amount of integer values.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * slider_val = Gui.drawIntSlider("A int slider", 0, 100, slider_val)
		 * @endcode
		 *
		 * @param label - The label to show.
		 * @param val_min - The minimum value settable through the slider.
		 * @param val_max - The maximum value settable through the slider.
		 * @param val1 - The first value to handle with the slider.
		 * @param val2 - The second value to handle with the slider <b>(optional)</b>.
		 * @param val3 - The third value to handle with the slider <b>(optional)</b>.
		 * @param val4 - The forth value to handle with the slider <b>(optional)</b>.
		 *
		 * @return The updated values after the frame execution.
		 */
		int[] drawIntSlider(string label, int val_min, int val_max, int val1,int val2, int val3, int val4);
		
		/**
		 * Draw a combobox.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * elems = {"Element 1", "Element 2", "Element 3", "Element 4"}
		 * combo_idx = Gui.drawComboBox("##combo", combo_idx, elems)
		 * end
		 * @endcode
		 *
		 * @param label - The label to show.
		 * @param index - The currently selected element.
		 * @param elements - The elements to use for the combobox.
		 *
		 * @return The updated selected element.
		 */
		int drawComboBox(string label, int index, table elements);
		
		/**
		 * Draw a listbox.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * elems = {"Element 1", "Element 2", "Element 3", "Element 4"}
		 * list_idx = Gui.drawListBox("##list", list_idx, elems)
		 * end
		 * @endcode
		 *
		 * @param label - The label to show.
		 * @param index - The currently selected element.
		 * @param elements - The elements to use for the combobox.
		 *
		 * @return The updated selected element.
		 */
		int drawListBox(string label, int index, table elements);
		
		/**
		 * Draw a color picker.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * color = Color.new(255, 127, 65)
		 * color = Gui.drawColorPicker("##color_picker", color)
		 * end
		 * @endcode
		 *
		 * @param label - The label to show.
		 * @param color - The currently picked color (See ::Color)
		 * @param alpha - If true, alpha value will be pickable, not otherwise. <b>(optional)</b>.
		 *
		 * @return The updated picked color.
		 */
		int drawColorPicker(string label, int color, bool alpha);
		
		/**
		 * Draw a progressbar.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * Gui.drawProgressbar(0.25, 200, 0)
		 * end
		 * @endcode
		 *
		 * @param fraction - Progress value to show in 0.0 - 1.0 range.
		 * @param w - Width of the element in pixels <b>(optional)</b>.
		 * @param h - Height of the element in pixels <b>(optional)</b>.
		 */
		void drawProgressbar(number fraction, number w, number h);
		
		/**
		 * Draw an image in the Gui subsystem.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * icon = Graphics.loadImage("app0:/icon.png")
		 * Gui.drawImage(icon, 32, 32, 0, 0, 20, 20)
		 * @endcode
		 *
		 * @param img - A valid image ID.
		 * @param width - Width in pixels of the widget <b>(optional)</b>.
		 * @param height - Height in pixels of the widget <b>(optional)</b>.
		 * @param img_x - Source image X coordinate <b>(optional)</b>.
		 * @param img_y - Source image Y coordinate <b>(optional)</b>.
		 * @param img_w - Source image width <b>(optional)</b>.
		 * @param img_h - Source image height <b>(optional)</b>.
		 * @param color - Image tint color (See ::Color) <b>(optional)</b>.
		 */
		void drawImage(int img, number width, number height, number img_x, number img_y, number img_w, number img_h, int color);
		
		/**
		 * Set next widget position.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * Gui.setWidgetPos(300, 200)
		 * Gui.drawText("Hello World")
		 * end
		 * @endcode
		 *
		 * @param x - X coordinate in pixels.
		 * @param y - Y coordinate in pixels.
		 */
		void setWidgetPos(number x, number y);
		
		/**
		 * Set next widgets width.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * Gui.setWidgetWidth(400)
		 * color = Color.new(255, 255, 255)
		 * color = Gui.drawColorPicker("##color_picker", color)
		 * end
		 * @endcode
		 *
		 * @param w - Width of the element in pixels.
		 */
		void setWidgetWidth(number w);
		
		/**
		 * Reset next widgets width to original values.
		 * \ingroup Gui
		 *
		 * @par Usage example:
		 * @code
		 * Gui.resetWidgetWidth()
		 * color = Color.new(255, 255, 255)
		 * color = Gui.drawColorPicker("##color_picker", color)
		 * end
		 * @endcode
		 */
		void resetWidgetWidth();
}