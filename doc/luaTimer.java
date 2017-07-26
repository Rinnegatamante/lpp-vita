/**
 * \defgroup Timer
 * Module that handles system timers.
 */
 
public class Timer {
	
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
	public int new(void){}
	
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
	public int getTime(int timer){}
	
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
	public void setTime(int timer, int msecs){}
	
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
	public void destroy(int timer){}
	
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
	public void pause(int timer){}
	
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
	public void resume(int timer){}
	
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
	public void reset(int timer){}
	
	/**
	 * Check if a system timer is in playing state.
	 * \ingroup Timer
	 *
	 * @par Usage example:
	 * @code
	 * if Timer.isPlaying(tmr) then
	 * 	Timer.pause(tmr)
	 * end
	 * @endcode
	 *
	 * @param timer - The timer ID to check.
	 *
	 * @return true if playing, false otherwise.
	 */
	public bool isPlaying(int timer){}
	
}