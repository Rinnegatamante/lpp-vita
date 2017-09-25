/**
 * \defgroup Video
 * Module that handles video files playback.
 */
 
class Video {
	
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
		 * Open a PSHV video file.
		 * \ingroup Video
		 *
		 * @par Usage example:
		 * @code
		 * Video.open("app0:/video.pshv", false)
		 * @endcode
		 *
		 * @param filename - Filepath of the file to reproduce.
		 * @param loop - Looping mode for the playback.
		 */
		void open(string filename, bool loop);
		
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
		 * @return true if playing, false otherwise.
		 */
		bool isPlaying(void);
		
		/**
		 * Set internal volume for current video playback.
		 * \ingroup Video
		 *
		 * @par Usage example:
		 * @code
		 * Video.setVolume(100)
		 * @endcode
		 *
		 * @param volume - Volume value to set.
		 *
		 * @note <b>volume</b> must be in a range of 0 and 32767.
		 */
		void setVolume(int volume);
		
		/**
		 * Get internal volume for current video playback.
		 * \ingroup Video
		 *
		 * @par Usage example:
		 * @code
		 * vol = Volume.getVolume()
		 * @endcode
		 *
		 * @return Volume value of the current video playback.
		 */
		int getVolume(void);
		
}