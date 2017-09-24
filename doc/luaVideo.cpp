/**
 * \defgroup Video
 * Module that handles video files playback.
 */
 
class Timer {
	
	public:
		
		/**
		 * Init video sub-system.
		 * \ingroup Video
		 *
		 * @par Usage example:
		 * @code
		 * Video.init()
		 * @endcode
		 */
		void init(void);
		
		/**
		 * Terminate video sub-system.
		 * \ingroup Video
		 *
		 * @par Usage example:
		 * @code
		 * Video.term()
		 * @endcode
		 */
		void term(void);
		
		/**
		 * Open an H.264 elementary stream file (.h264).
		 * \ingroup Video
		 *
		 * @par Usage example:
		 * @code
		 * Video.openAvc("app0:/video.h264", 29.970, false)
		 * @endcode
		 *
		 * @param filename - Filepath of the file to reproduce.
		 * @param framerate - Framerate to use for the playback.
		 * @param loop - Looping mode for the playback.
		 *
		 * @return The time of the given timer in milliseconds.
		 */
		void openAvc(string filename, number framerate, bool loop);
		
		/**
		 * Get the current video frame as image.
		 * \ingroup Video
		 *
		 * @par Usage example:
		 * @code
		 * frame = Video.getOutput()
		 * if frame ~= 0 then
		 * 	Graphics.drawImage(0, 0, frame)
		 * end
		 * @endcode
		 *
		 * @return The current video frame, 0 if no frame is available.
		 *
		 * @note Returned image became invalid when ::Video.term is called.
		 */
		int getOutput(void);
		
		/**
		 * Pause current video playback.
		 * \ingroup Video
		 *
		 * @par Usage example:
		 * @code
		 * Video.pause()
		 * @endcode
		 */
		void pause(void);
		
		/**
		 * Resume current video playback.
		 * \ingroup Video
		 *
		 * @par Usage example:
		 * @code
		 * Video.resume()
		 * @endcode
		 */
		void resume(void);
		
		/**
		 * Get current playback state.
		 * \ingroup Video
		 *
		 * @par Usage example:
		 * @code
		 * if Video.isPlaying() then
		 * 	Video.pause()
		 * end
		 * @endcode
		 *
		 * @return - true if playing, false otherwise.
		 */
		bool isPlaying(void);
		
}