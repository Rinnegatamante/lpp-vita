/**
 * \defgroup Registry
 * Module that handles system registry.
 */ 

/**
 * Registry types.
 * \ingroup Registry
 */
enum RegType{
	TYPE_STRING, //!< String type.
	TYPE_NUMBER, //!< Integer type.
	TYPE_BINARY //!< Binary Type.
};

class Registry{
	
	public:
	
		/**
		 * Get a value for a given key.
		 * \ingroup Registry
		 *
		 * @par Usage example:
		 * @code
		 * flight_mode = Registry.getKey("/CONFIG/SYSTEM/", "flight_mode", TYPE_NUMBER)
		 * @endcode
		 *
		 * @param category - The category from which to take the value.
		 * @param name - The key related to the value.
		 * @param type - The type of the value.
		 *
		 * @return The requested value.
		 *
		 * @note This function is available only in unsafe mode.
		 */
		auto getKey(string category, string name, RegType type);
		
		/**
		 * Get a value for a given system key.
		 * \ingroup Registry
		 *
		 * @par Usage example:
		 * @code
		 * val = Registry.getSysKey(0, TYPE_NUMBER)
		 * @endcode
		 *
		 * @param id - The identifier of the key.
		 * @param type - The type of the value.
		 *
		 * @return The requested value.
		 *
		 * @note This function is available only in unsafe mode.
		 */
		auto getSysKey(int id, RegType type);
		
		/**
		 * Set a value for a given key.
		 * \ingroup Registry
		 *
		 * @par Usage example:
		 * @code
		 * flight_mode = Registry.setKey("/CONFIG/SYSTEM/", "flight_mode", TYPE_NUMBER, 1)
		 * @endcode
		 *
		 * @param category - The category from which to take the value.
		 * @param name - The key related to the value.
		 * @param type - The type of the value.
		 * @param value - The value to set.
		 * @param size - The size of the value <b>(optional)</b>.
		 *
		 * @note This function is available only in unsafe mode.
		 */
		void setKey(string category, string name, RegType type, auto value, int size);
}