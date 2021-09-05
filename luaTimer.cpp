/**
 * \defgroup Timer
 * Module that handles system timers.
 */
 
class Timer {
	
	public:
		
		/**
		 * Create a new system timer.
		 * \ingroup Timer
		 *
		 * @par Usage example:
		 * @code
		 * tmr = Timer.new()
		 * @endcode
		 *
		 * @return The system timer ID.
		 *
		 * @note The created timer will be already in playing state.
		 */
		int new(void);
		
		/**
		 * Get a system timer time.
		 * \ingroup Timer
		 *
		 * @par Usage example:
		 * @code
		 * time = Timer.getTime(tmr)
		 * @endcode
		 *
		 * @param timer - The timer ID to get the time of.
		 *
		 * @return The time of the given timer in milliseconds.
		 */
		int getTime(int timer);
		
		/**
		 * Set a system timer time.
		 * \ingroup Timer
		 *
		 * @par Usage example:
		 * @code
		 * Timer.setTime(tmr, 1000)
		 * @endcode
		 *
		 * @param timer - The timer ID to set the time of.
		 * @param msecs - The time to set in milliseconds.
		 */
		void setTime(int timer, int msecs);
		
		/**
		 * Destroy a system timer.
		 * \ingroup Timer
		 *
		 * @par Usage example:
		 * @code
		 * Timer.destroy(tmr)
		 * @endcode
		 *
		 * @param timer - The timer ID to destroy.
		 */
		void destroy(int timer);
		
		/**
		 * Pause a system timer.
		 * \ingroup Timer
		 *
		 * @par Usage example:
		 * @code
		 * Timer.pause(tmr)
		 * @endcode
		 *
		 * @param timer - The timer ID to pause.
		 */
		void pause(int timer);
		
		/**
		 * Resume a system timer.
		 * \ingroup Timer
		 *
		 * @par Usage example:
		 * @code
		 * Timer.resume(tmr)
		 * @endcode
		 *
		 * @param timer - The timer ID to resume.
		 */
		void resume(int timer);
		
		/**
		 * Reset a system timer time.
		 * \ingroup Timer
		 *
		 * @par Usage example:
		 * @code
		 * Timer.reset(tmr)
		 * @endcode
		 *
		 * @param timer - The timer ID to reset.
		 */
		void reset(int timer);
		
		/**
		 * Check if a system timer is in playing state.
		 * \ingroup Timer
		 *
		 * @par Usage example:
		 * @code
		 * if Timer.isPlaying(tmr) then
		 * 		Timer.pause(tmr)
		 * end
		 * @endcode
		 *
		 * @param timer - The timer ID to check.
		 *
		 * @return true if playing, false otherwise.
		 */
		bool isPlaying(int timer);
		
}