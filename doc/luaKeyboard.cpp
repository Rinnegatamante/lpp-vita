/**
 * \defgroup Keyboard
 * Module that handles system keyboard.
 */
 
/**
 * Keyboard modes to use with Keyboard.start.
 * \ingroup Keyboard
 */
enum KeyMode{
	MODE_TEXT,     //!< Text mode
	MODE_PASSWORD  //!< Password mode
};

/**
 * Keyboard types to use with Keyboard.start.
 * \ingroup Keyboard
 */
enum KeyType{
	TYPE_DEFAULT,     //!< Default type
	TYPE_LATIN,       //!< Latin encode type
	TYPE_NUMBER,      //!< Basic number type
	TYPE_NUMBER_EXT   //!< Extended number type
};
 
class Keyboard {
	
	public:
	
		/**
		 * Start a keyboard instance.
		 * \ingroup Keyboard
		 *
		 * @par Usage example:
		 * @code
		 * Keyboard.start("Insert a text", "Text")
		 * @endcode
		 *
		 * @param title - The keyboard title.
		 * @param text - The keyboard initial text.
		 * @param length - The text maximum length <b>(optional)</b>.
		 * @param type - The keyboard type <b>(optional)</b>.
		 * @param mode - The keyboard mode <b>(optional)</b>.
		 */
		void start(string title, string text, int length, KeyType type, KeyMode mode);
		
		/**
		 * Get keyboard state.
		 * \ingroup Keyboard
		 *
		 * @par Usage example:
		 * @code
		 * state = Keyboard.getState()
		 * @endcode
		 *
		 * @return Current state of the system keyboard instance.
		 */
		DlgState getState(void);
		
		/**
		 * Get keyboard user input.
		 * \ingroup Keyboard
		 *
		 * @par Usage example:
		 * @code
		 * if Keyboard.getState() == FINISHED then
		 * 		result_text = Keyboard.getInput()
		 * end
		 * @endcode
		 *
		 * @return The text entered by the user.
		 */
		string getInput(void);
		
		/**
		 * Close current system keyboard instance.
		 * \ingroup Keyboard
		 *
		 * @par Usage example:
		 * @code
		 * Keyboard.clear()
		 * @endcode
		 */
		void clear(void);
	
}