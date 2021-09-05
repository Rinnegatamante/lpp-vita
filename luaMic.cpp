/**
 * \defgroup Mic
 * Module that handles microphone.
 */
 
class Mic {
	
	public:
	
		/**
		 * Start a microphone recording phase.
		 * \ingroup Mic
		 *
		 * @par Usage example:
		 * @code
		 * Mic.start(5, 32000)
		 * @endcode
		 *
		 * @param time - The time to record in seconds.
		 * @param samplerate - The samplerate of the output in Hz.
		 *
		 * @note <b>samplerate</b> must be a multiple of 16000.
		 */
		int start(int time, int samplerate);
		
		/**
		 * Stop microphone recording and save result.
		 * \ingroup Mic
		 *
		 * @par Usage example:
		 * @code
		 * Mic.stop("ux0:/data/record.wav")
		 * @endcode
		 *
		 * @param filename - The filename of the resulting WAV file.
		 *
		 * @note The resulting file will always be a mono WAV file.
		 */
		void stop(string filename);
		
		/**
		 * Pause microphone recording.
		 * \ingroup Mic
		 *
		 * @par Usage example:
		 * @code
		 * Mic.pause()
		 * @endcode
		 */
		void pause(void);
		
		/**
		 * Resume microphone recording.
		 * \ingroup Mic
		 *
		 * @par Usage example:
		 * @code
		 * Mic.resume()
		 * @endcode
		 */
		void resume(void);
		
		/**
		 * Check if the microphone is recording.
		 * \ingroup Mic
		 *
		 * @par Usage example:
		 * @code
		 * if Mic.isRecording() then
		 * 		Mic.pause()
		 * end
		 * @endcode
		 *
		 * @return true if recording, false otherwise.
		 */
		bool isRecording(void);
	
}