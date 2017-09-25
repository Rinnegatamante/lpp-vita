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


#if defined(HAVE_TREMOR) || defined(HAVE_OGGVORBIS)

// Headers
#include <cassert>
#ifdef HAVE_TREMOR
#include <tremor/ivorbiscodec.h>
#include <tremor/ivorbisfile.h>
#else
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#endif
#include "audio_decoder.h"
#include "decoder_oggvorbis_pshv.h"

OggVorbisForPSHVDecoder::OggVorbisForPSHVDecoder() {
	music_type = "ogg";
}

OggVorbisForPSHVDecoder::~OggVorbisForPSHVDecoder() {
	if (ovf) {
		ov_clear(ovf);
		delete ovf;
	}
}

size_t pshv_rc(void *ptr, size_t size, size_t nmemb, void *datasource){
	return fread((FILE*)ptr,size,nmemb,(FILE*)datasource);
}

int pshv_sk(void *datasource, ogg_int64_t offset, int whence){
	if (whence == SEEK_END){
		fseek((FILE*)datasource,0x08,SEEK_SET);
		uint32_t audiosize;
		fread(&audiosize,4,1,(FILE*)datasource);
		return fseek((FILE*)datasource,0x0C+audiosize+offset,SEEK_SET);
	}else return fseek((FILE*)datasource,offset,whence);
}

int pshv_cl(void *datasource){
	return fclose((FILE*)datasource);
}

long pshv_tl(void *datasource){
	return ftell((FILE*)datasource);
}


bool OggVorbisForPSHVDecoder::Open(FILE* file) {
	finished = false;

	if (ovf) {
		ov_clear(ovf);
		delete ovf;
	}
	ovf = new OggVorbis_File;
	
	ov_callbacks pshv_callbacks;
	pshv_callbacks.read_func = pshv_rc;
	pshv_callbacks.seek_func = pshv_sk;
	pshv_callbacks.tell_func = pshv_tl;
	pshv_callbacks.close_func = pshv_cl;
	
	int res = ov_open_callbacks(file, ovf, NULL, 0, pshv_callbacks);
	if (res < 0) {
		error_message = "OggVorbis: Error reading file";
		delete ovf;
		fclose(file);
		return false;
	}

	vorbis_info *vi = ov_info(ovf, -1);
	if (!vi) {
		error_message = "OggVorbis: Error getting file information";
		ov_clear(ovf);
		delete ovf;
		return false;
	}

	// (long)ov_pcm_total(ovf, -1)) -> decoded length in samples, maybe useful for ticks later?
	frequency = vi->rate;
	channels = vi->channels;

	return true;
}

bool OggVorbisForPSHVDecoder::Seek(size_t offset, Origin origin) {
	if (offset == 0 && origin == Origin::Begin) {
		if (ovf) {
			ov_raw_seek(ovf, 0);
		}
		finished = false;
		return true;
	}

	return false;
}

bool OggVorbisForPSHVDecoder::IsFinished() const {
	if (!ovf)
		return false;

	return finished;
}

void OggVorbisForPSHVDecoder::GetFormat(int& freq, AudioDecoder::Format& format, int& chans) const {
	freq = frequency;
	format = Format::S16;
	chans = channels;
}

bool OggVorbisForPSHVDecoder::SetFormat(int freq, AudioDecoder::Format format, int chans) {
	if (freq != frequency || chans != channels || format != Format::S16)
		return false;

	return true;
}

bool OggVorbisForPSHVDecoder::SetPitch(int pitch) {
	if (pitch != 100) {
		return false;
	}

	return true;
}

int OggVorbisForPSHVDecoder::FillBuffer(uint8_t* buffer, int length) {
	if (!ovf)
		return -1;

	static int section;
	int read = 0;
	int to_read = length;

	do {
#ifdef HAVE_TREMOR
		read = ov_read(ovf, reinterpret_cast<char*>(buffer + length - to_read), to_read, &section);
#else
		read = ov_read(ovf, reinterpret_cast<char*>(buffer + length - to_read), to_read, 0/*LE*/, 2/*16bit*/, 1/*signed*/, &section);
#endif
		// stop decoding when error or end of file
		if (read <= 0)
			break;

		to_read -= read;
	} while(to_read > 0);

	// end of file
	if (read == 0)
		finished = true;

	// error
	if (read < 0)
		return -1;

	return length - to_read;
}

#endif
