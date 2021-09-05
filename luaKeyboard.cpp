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

/**
 * Optional keyboard features usable.
 * \ingroup Keyboard
 */
enum KeyOption{
	OPT_MULTILINE,     //!< Multiline keyboard
	OPT_NO_AUTOCAP,    //!< Disabled auto-cap on first letter
	OPT_NO_ASSISTANCE  //!< Disabled word assistant
};
 
class Keyboard {
	
	public:
	
		/**
		 * Start a keyboard instance.
		 * \ingroup Keyboard
		 *
		 * @par Usage example:
		 * @code
		 * Keyboard.start("Insert a text", "Text", 512, TYPE_DEFAULT, MODE_TEXT, OPT_MULTILINE + OPT_NO_AUTOCAP)
		 * @endcode
		 *
		 * @param title - The keyboard title.
		 * @param text - The keyboard initial text.
		 * @param length - The text maximum length <b>(optional)</b>.
		 * @param type - The keyboard type <b>(optional)</b>.
		 * @param mode - The keyboard mode <b>(optional)</b>.
		 * @param opt - Optional keyboard features to use <b>(optional)</b>.
		 */
		void start(string title, string text, int length, KeyType type, KeyMode mode, KeyOption opt);
		
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