/**
 * \defgroup Sound
 * Module that handles audio features.
 */ 

/**
 * Loop mode for Sound.play.
 * \ingroup Sound
 */
enum LoopMode{
	NO_LOOP,     //!< Non-loop mode
	LOOP         //!< Loop mode
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
		 * Open a .wav audio file.
		 * \ingroup Sound
		 *
		 * @par Usage example:
		 * @code
		 * snd = Sound.openWav("ux0:/data/audio.wav")
		 * @endcode
		 *
		 * @param filename - Name of the file to open
		 *
		 * @return A valid sound ID.
		 */
		int openWav(string filename);
		
		/**
		 * Open a .aif/.aiff audio file.
		 * \ingroup Sound
		 *
		 * @par Usage example:
		 * @code
		 * snd = Sound.openAiff("ux0:/data/audio.aiff")
		 * @endcode
		 *
		 * @param filename - Name of the file to open
		 *
		 * @return A valid sound ID.
		 */
		int openAiff(string filename);
		
		/**
		 * Open a .mp3 audio file.
		 * \ingroup Sound
		 *
		 * @par Usage example:
		 * @code
		 * snd = Sound.openMp3("ux0:/data/audio.mp3")
		 * @endcode
		 *
		 * @param filename - Name of the file to open
		 *
		 * @return A valid sound ID.
		 */
		int openMp3(string filename);
		
		/**
		 * Open a .mid/.midi audio file.
		 * \ingroup Sound
		 *
		 * @par Usage example:
		 * @code
		 * snd = Sound.openMidi("ux0:/data/audio.mid")
		 * @endcode
		 *
		 * @param filename - Name of the file to open
		 *
		 * @return A valid sound ID.
		 */
		int openMidi(string filename);
		
		/**
		 * Open a .ogg audio file.
		 * \ingroup Sound
		 *
		 * @par Usage example:
		 * @code
		 * snd = Sound.openOgg("ux0:/data/audio.ogg")
		 * @endcode
		 *
		 * @param filename - Name of the file to open
		 *
		 * @return A valid sound ID.
		 */
		int openOgg(string filename);
		
		/**
		 * Open a .opus audio file.
		 * \ingroup Sound
		 *
		 * @par Usage example:
		 * @code
		 * snd = Sound.openOpus("ux0:/data/audio.opus")
		 * @endcode
		 *
		 * @param filename - Name of the file to open
		 *
		 * @return A valid sound ID.
		 */
		int openOpus(string filename);
		
		/**
		 * Plays a sound.
		 * \ingroup Sound
		 *
		 * @par Usage example:
		 * @code
		 * Sound.play(snd, NO_LOOP)
		 * @endcode
		 *
		 * @param music - A valid sound ID.
		 * @param loop - Loop mode.
		 */
		void play(int music, LoopMode loop);
		
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

}