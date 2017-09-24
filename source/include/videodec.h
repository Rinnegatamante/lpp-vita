#include <vitasdk.h>

typedef struct controller{
	bool status;
	SceUID   fd;
	uint8_t* tempBuf[2];
	uint32_t fileSize;
	uint32_t readSize;
	uint32_t bufOffs;
	uint8_t  bufIdx;
	uint32_t bufSize;
} controller;

int ctrlInit(controller* cd, const char* filename, uint32_t bufSize);
int ctrlTerm(controller* cd);
int ctrlReadFrame(controller* cd, uint8_t* buf, uint32_t* bufSize);
int ctrlRewind(controller* cd);