#ifndef _AUDIO_DECODER_AIFF_H
#define _AUDIO_DECODER_AIFF_H

// Headers
#include "audio_decoder.h"
#include <string>	
#include <memory>

/**
 * Standalone basic audio decoder for AIFF
 */
class AiffDecoder : public AudioDecoder {
public:
	AiffDecoder();

	~AiffDecoder();

	bool Open(FILE* file) override;

	bool Seek(size_t offset, Origin origin) override;

	bool IsFinished() const override;

	void GetFormat(int& frequency, AudioDecoder::Format& format, int& channels) const override;

	bool SetFormat(int frequency, AudioDecoder::Format format, int channels) override;

private:
	int FillBuffer(uint8_t* buffer, int length) override;
	Format output_format;
	FILE * file_;
	bool finished;
	uint32_t samplerate;
	uint16_t nchannels;
	uint32_t audiobuf_offset;
	uint32_t chunk_size;
	uint32_t cur_pos;
};

#endif
