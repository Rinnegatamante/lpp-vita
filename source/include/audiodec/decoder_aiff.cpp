/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */
 
 #ifdef WANT_FASTAIFF

// Headers
#include <cstring>
#include "decoder_aiff.h"
#include "utils.h"

// Endianess swap functions
static uint32_t Endian_UInt32_Conversion(uint32_t value){
   return ((value >> 24) & 0x000000FF) | ((value >> 8) & 0x0000FF00) | ((value << 8) & 0x00FF0000) | ((value << 24) & 0xFF000000);
}
static uint16_t Endian_UInt16_Conversion(uint16_t value){
   return (uint16_t)(((value >> 8) & 0x00FF) | ((value << 8) & 0xFF00));
}

AiffDecoder::AiffDecoder() 
{
	music_type = "aiff";
}

AiffDecoder::~AiffDecoder() {
	if (file_ != NULL) {
		fclose(file_);
	}
}

bool AiffDecoder::Open(FILE* file) {
	file_=file;
	fseek(file_, 0, SEEK_END);
	chunk_size = ftell(file_);
	
	char chunk[4] = { 0 };
	uint32_t jump = 0, pos = 12;
	uint16_t bitspersample;
	
	fseek(file_, pos, SEEK_SET);
	fread(chunk, 4, 1, file_);
	while (strncmp(chunk, "SSND", 4) != 0){
		if (!strncmp(chunk, "COMM", 4)){
			fseek(file_, pos+8, SEEK_SET);
			fread(&nchannels, 1, 2, file_);
			nchannels = nchannels>>8;
			fseek(file_, pos+14, SEEK_SET);
			fread(&bitspersample, 1, 2, file_);
			bitspersample = Endian_UInt16_Conversion(bitspersample);
			switch (bitspersample) {
				case 8:
					output_format=Format::U8;
					break;
				case 16:
					output_format=Format::S16;
					break;
				case 32:
					output_format=Format::S32;
					break;
				default:
					return false;
			}
			fseek(file_, 2, SEEK_CUR);
			fread(&samplerate, 1, 4, file_);
			samplerate = Endian_UInt16_Conversion(samplerate);
		}
		pos += 4;
		fseek(file_, pos, SEEK_SET);
		fread(&jump, 1, 4, file_);
		pos += (4+Endian_UInt32_Conversion(jump));
		fseek(file_, pos, SEEK_SET);
		fread(chunk, 4, 1, file_);
	}
	
	// Get SSND chunk size
	chunk_size -= (pos+4);
	
	if (feof(file_) || ferror(file_)) {
		fclose(file_);
		return false;
	}
	
	// Start of SSND chunk
	fseek(file_, pos+4, SEEK_SET);
	audiobuf_offset = pos+4;
	cur_pos = audiobuf_offset;
	finished = false;
	return file_!=NULL;
}

bool AiffDecoder::Seek(size_t offset, Origin origin) {
	finished = false;
	if (file_ == NULL)
		return false;
	if (origin != Origin::End) {
		offset += audiobuf_offset;
	}

	bool success = fseek(file_,offset,(int)origin) == 0;
	cur_pos = ftell(file_);
	return success;
}

bool AiffDecoder::IsFinished() const {
	return finished;
}

void AiffDecoder::GetFormat(int& frequency, AudioDecoder::Format& format, int& channels) const {
	if (file_ == NULL) return;
	frequency = samplerate;
	channels = nchannels;
	format = output_format;
}

bool AiffDecoder::SetFormat(int, AudioDecoder::Format, int) {
	return false;
}

int AiffDecoder::FillBuffer(uint8_t* buffer, int length) {
	if (file_ == NULL)
		return -1;

	int real_length;
	cur_pos = cur_pos + length;

	// Handle case that another chunk is behind "data"
	if (cur_pos >= audiobuf_offset + chunk_size) {
		real_length = cur_pos - chunk_size - length - audiobuf_offset;
	} else {
		real_length = length;
	}

	if (real_length == 0) {
		finished = true;
		return 0;
	}

	int decoded = fread(buffer, 1, real_length, file_);
	for(int i=0;i<decoded;i+=2){
		uint16_t* sample = (uint16_t*)(&buffer[i]);
		sample[0] = Endian_UInt16_Conversion(sample[0]);
	}
	if (decoded < length)
		finished = true;
	
	return decoded;
}

#endif
