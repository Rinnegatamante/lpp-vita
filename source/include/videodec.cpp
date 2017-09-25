#include <stdio.h>
#include <stdlib.h>
#include <vitasdk.h>
#include "videodec.h"

#define	CHECKBUF_SIZE 3

static uint8_t initCodeMagic[CHECKBUF_SIZE] = {0x00, 0x00, 0x01};

static int NalSearchNextNal(uint8_t* pucBuf, uint32_t inputSize, uint8_t NalUnitType){
	int j = 0;
	int ret = -1;
	int nalLength;
	uint8_t checkBuf[CHECKBUF_SIZE];
	int checkBufCapacity;

	nalLength = 0;
	checkBufCapacity = CHECKBUF_SIZE;

	if(inputSize <= CHECKBUF_SIZE) return ret;

	while (j < inputSize){
		
		uint8_t getCh;

		while(checkBufCapacity > 0){
			getCh = pucBuf[j++];
			checkBuf[CHECKBUF_SIZE - checkBufCapacity] = getCh;
			checkBufCapacity--;
			nalLength++;
		}
		
		if (memcmp(&checkBuf[0], &initCodeMagic[0], 3) == 0){

			getCh = pucBuf[j++];
			nalLength++;

			if ((getCh & 0x01f) == NalUnitType){
				if (nalLength > 5){
					if (pucBuf[j-5] == 0x00) nalLength -= 5;
					else nalLength -= 4;
					ret = nalLength;
					break;
				}
			}

		}
		
		checkBufCapacity += 1;
		for (int i = 0; i < (CHECKBUF_SIZE-1); i++){
			checkBuf[i] = checkBuf[i+1];
		}

	}
	
	return ret;

}

int ctrlRewind(controller* cd){
	cd->readSize = 0;
	cd->bufOffs = 0;
	cd->bufIdx = 0;
	return sceIoLseek(cd->fd, cd->videoOffs, SEEK_SET);
}

static int ctrlOpen(controller* cd, const char* filename, float* framerate){
	cd->fd = sceIoOpen(filename, SCE_O_RDONLY, 0777);
	sceIoLseek(cd->fd, 4, SEEK_SET);
	sceIoRead(cd->fd, framerate, 4);
	uint32_t audio_size = 0;
	sceIoRead(cd->fd, &audio_size, 4);
	cd->videoOffs = 0x0C + audio_size;
	cd->fileSize = sceIoLseek(cd->fd, 0, SEEK_END) - cd->videoOffs;
	cd->readSize = 0;
	return sceIoLseek(cd->fd, cd->videoOffs, SEEK_SET);
}

int ctrlInit(controller* cd, const char* filename, uint32_t bufSize, float* framerate){
	memset(cd, 0, sizeof(controller));
	cd->bufSize = bufSize;
	cd->tempBuf[0] = (uint8_t*)malloc(cd->bufSize);
	cd->tempBuf[1] = (uint8_t*)malloc(cd->bufSize);
	cd->status = true;
	return ctrlOpen(cd, filename, framerate);
}

int ctrlTerm(controller* cd){
	cd->status = false;
	sceIoClose(cd->fd);
	free(cd->tempBuf[0]);
	free(cd->tempBuf[1]);
	return 0;
}

int ctrlReadFrame(controller* cd, uint8_t* buf, uint32_t* bufSize){
	uint8_t nextBufIdx = (cd->bufIdx + 1) % 2;
	if (cd->readSize >= cd->fileSize && cd->bufOffs == 0) return 0;
	while (cd->bufOffs < cd->bufSize){
		int rBytes = sceIoRead(cd->fd, cd->tempBuf[cd->bufIdx] + cd->bufOffs, cd->bufSize - cd->bufOffs);
		if (rBytes < 0) return 0;
		else if (rBytes == 0) break;
		cd->bufOffs += rBytes;
		cd->readSize += rBytes;
	}
	int auLen = NalSearchNextNal(cd->tempBuf[cd->bufIdx], cd->bufOffs, 0x09);
	if (auLen <= 0){
		if (cd->readSize >= cd->fileSize) auLen = cd->bufOffs;
	}
	memcpy(buf, cd->tempBuf[cd->bufIdx], auLen);
	*bufSize = auLen;
	memcpy(cd->tempBuf[nextBufIdx], cd->tempBuf[cd->bufIdx] + auLen, cd->bufOffs - auLen);
	cd->bufOffs -= auLen;
	cd->bufIdx = nextBufIdx;
	return 1;
}