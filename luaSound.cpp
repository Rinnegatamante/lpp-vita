/**
 * \defgroup Sound
 * Module that handles audio features.
 */ 

/**
 * Shutter sound IDs for Sound.playShutter.
 * \ingroup Sound
 */
enum ShutterId{
	IMAGE_CAPTURE,       //!< Image capture shutter sound
	VIDEO_CAPTURE_START, //!< Video record start shutter sound
	VIDEO_CAPTURE_END    //!< Video record end shutter sound
};
 
class Sound {
	
	public:

		/**
		 * Initialize audio subsystem.
		 * \ingroup Sound
		 *
		 * @par Usage example:
		 * @code
		 * Sound.init()
		 * @endcode
		 */
		void init(void);
		
		/**
		 * Terminate audio subsystem.
		 * \ingroup Sound
		 *
		 * @par Usage example:
		 * @code
		 * Sound.term()
		 * @endcode
		 */
		void term(void);
		
		
		/**
		 * Open an audio file.
		 * \ingroup Sound
		 *
		 * @par Usage example:
		 * @code
		 * snd = Sound.open("ux0:/data/audio.opus")
		 * @endcode
		 *
		 * @param filename - Name of the file to open
		 *
		 * @return A valid sound ID.
		 *
		 * @note Supported audio formats: .wav, .aif, .aiff, .opus, .ogg, .mid, .midi, .mp3.
		 * @note This function can be used to reproduce audio track from a .pshv video file too.
		 */
		int open(string filename);
		
		/**
		 * Play a sound.
		 * \ingroup Sound
		 *
		 * @par Usage example:
		 * @code
		 * Sound.play(snd, NO_LOOP)
		 * @endcode
		 *
		 * @param music - A valid sound ID.
		 * @param loop - If true, playback will loop <b>(optional)</b>.
		 */
		void play(int music, bool loop);
		
		/**
		 * Pause a sound.
		 * \ingroup Sound
		 *
		 * @par Usage example:
		 * @code
		 * Sound.pause(snd)
		 * @endcode
		 *
		 * @param music - A valid sound ID.
		 */
		void pause(int music);
		
		/**
		 * Resume a sound.
		 * \ingroup Sound
		 *
		 * @par Usage example:
		 * @code
		 * Sound.resume(snd)
		 * @endcode
		 *
		 * @param music - A valid sound ID.
		 */
		void resume(int music);
		
		/**
		 * Close an opened sound.
		 * \ingroup Sound
		 *
		 * @par Usage example:
		 * @code
		 * Sound.close(snd)
		 * @endcode
		 *
		 * @param music - A valid sound ID.
		 */
		void close(int music);
		
		/**
		 * Check if a given sound is in playing state.
		 * \ingroup Sound
		 *
		 * @par Usage example:
		 * @code
		 * if Sound.isPlaying(snd) then
		 * 	Sound.pause(snd)
		 * end
		 * @endcode
		 *
		 * @param music - A valid sound ID.
		 *
		 * @return true if playing, false otherwise.
		 */
		bool isPlaying(int music);
		
		/**
		 * Set internal volume for a given sound.
		 * \ingroup Sound
		 *
		 * @par Usage example:
		 * @code
		 * Sound.setVolume(snd, 100)
		 * @endcode
		 *
		 * @param music - A valid sound ID.
		 * @param volume - Volume value to set.
		 *
		 * @note <b>volume</b> must be in a range of 0 and 32767.
		 */
		void setVolume(int music, int volume);
		
		/**
		 * Get internal volume for a given sound.
		 * \ingroup Sound
		 *
		 * @par Usage example:
		 * @code
		 * vol = Sound.getVolume(snd)
		 * @endcode
		 *
		 * @param music - A valid sound ID.
		 *
		 * @return Volume value of the given sound.
		 */
		int getVolume(int music);
		
		/**
		 * Get song title of a given sound.
		 * \ingroup Sound
		 *
		 * @par Usage example:
		 * @code
		 * title = Sound.getTitle(snd)
		 * @endcode
		 *
		 * @param music - A valid sound ID.
		 *
		 * @return Song title of the given sound.
		 *
		 * @note This function works only with .ogg, .mp3, .opus and .wav audio files.
		 */
		string getTitle(int music);
		
		/**
		 * Get song author of a given sound.
		 * \ingroup Sound
		 *
		 * @par Usage example:
		 * @code
		 * author = Sound.getAuthor(snd)
		 * @endcode
		 *
		 * @param music - A valid sound ID.
		 *
		 * @return Song author of the given sound.
		 *
		 * @note This function works only with .ogg, .mp3, .opus and .wav audio files.
		 */
		string getAuthor(int music);
		
		/**
		 * Play a shutter sound.
		 * \ingroup Sound
		 *
		 * @par Usage example:
		 * @code
		 * Sound.playShutter(IMAGE_CAPTURE)
		 * @endcode
		 *
		 * @param id - A valid shutter sound ID.
		 */
		void playShutter(ShutterId id);

}