/**
 * \defgroup Database
 * Module that handles SQLite databases management.
 */


class Database {
	
	public:
	
		/**
		 * Open an existing database.
		 * \ingroup Database
		 *
		 * @par Usage example:
		 * @code
		 * db = Database.open("ur0:shell/db/app.db")
		 * @endcode
		 *
		 * @param filename - Filename of the database to open.
		 *
		 * @return A valid database handle.
		 *
		 * @note If the database doesn't exist, it's created using this function.
		 */
		int open(string filename);
		
		/**
		 * Close an opened database
		 * \ingroup Database
		 *
		 * @par Usage example:
		 * @code
		 * Database.close(db)
		 * @endcode
		 */
		int close(void);
		
		/**
		 * Execute a query.
		 * \ingroup Database
		 *
		 * @par Usage example:
		 * @code
		 * results = Database.execQuery(db, "SELECT columnName FROM tableName")
		 * Graphics.debugPrint(0, 0, results[0].columnName, Color.new(255, 255, 255))
		 * @endcode
		 *
		 * @param db - A valid database handle.
		 * @param query - An SQL query to execute.
		 *
		 * @return A table with the results of the executed query.
		 */
		table execQuery(int db, string query);
	
}