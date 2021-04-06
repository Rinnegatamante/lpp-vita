/**
 * \defgroup Video
 * Module that handles video files playback.
 */
 
/**
 * Play modes for video playback.
 * \ingroup Video
 */
enum PlayMode{
	NORMAL_MODE,           //!< Normal speed with audio
	FAST_FORWARD_2X_MODE,  //!< Fast Forward at 2x speed without audio
	FAST_FORWARD_4X_MODE,  //!< Fast Forward at 4x speed without audio
	FAST_FORWARD_8X_MODE,  //!< Fast Forward at 8x speed without audio
	FAST_FORWARD_16X_MODE, //!< Fast Forward at 16x speed without audio
	FAST_FORWARD_32X_MODE, //!< Fast Forward at 32x speed without audio
};
 
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
		 * Open an MP4 video file.
		 * \ingroup Video
		 *
		 * @par Usage example:
		 * @code
		 * Video.open("app0:/video.mp4", false)
		 * @endcode
		 *
		 * @param filename - Filepath of the file to reproduce.
		 * @param loop - If true, playback will loop <b>(optional)</b>.
		 */
		void open(string filename, bool loop);
		
		/**
		 * Close the current video playback.
		 * \ingroup Video
		 *
		 * @par Usage example:
		 * @code
		 * Video.close()
		 * @endcode
		 */
		void close(void);
		
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
		
		/**
		 * Get current time for current video playback.
		 * \ingroup Video
		 *
		 * @par Usage example:
		 * @code
		 * t = Volume.getTime()
		 * @endcode
		 *
		 * @return Current time position in the video playback in milliseconds.
		 */
		number getTime(void);
		
		/**
		 * Jump to a specific position for current video playback.
		 * \ingroup Video
		 *
		 * @par Usage example:
		 * @code
		 * Video.jumpToTime(Video.getTime() + 5000)
		 * @endcode
		 *
		 * @param time - The position in milliseconds where to jump in the video playback.
		 */
		void jumpToTime(number time);
		
		/**
		 * Set play mode for current video playback.
		 * \ingroup Video
		 *
		 * @par Usage example:
		 * @code
		 * Video.setPlayMode(FAST_FORWARD_16X_MODE)
		 * @endcode
		 *
		 * @param mode - The mode to set.
		 */
		void setPlayMode(PlayMode mode);
		
		/**
		 * Open a subtitles file for current video playback.
		 * \ingroup Video
		 *
		 * @par Usage example:
		 * @code
		 * Video.openSubs("ux0:data/subtitles.vtt")
		 * @endcode
		 *
		 * @param fname - The file to use as subtitles source.
		 *
		 * @note Supported subtitles formats: .srt, .vtt.
		 */
		void openSubs(string fname);
		
		/**
		 * Close currently opened subtitles file.
		 * \ingroup Video
		 *
		 * @par Usage example:
		 * @code
		 * Video.closeSubs()
		 * @endcode
		 */
		void closeSubs();
		
		/**
		 * Get current available subtitles for the video playback.
		 * \ingroup Video
		 *
		 * @par Usage example:
		 * @code
		 * Graphics.debugPrint(20, 20, Video.getSubs(), Color.new(255, 255, 255))
		 * @endcode
		 *
		 * @return Subtitles matching current video playtime from the opened subtitles file.
		 */
		string getSubs(void);
}