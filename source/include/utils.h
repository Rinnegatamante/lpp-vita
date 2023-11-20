// Misc utils
#define ALIGN(x, a)	(((x) + ((a) - 1)) & ~((a) - 1))

// ASCII / UTF16 compatibility
void utf2ascii(char* dst, uint16_t* src);
void ascii2utf(uint16_t* dst, char* src);

// GPU utils
void *gpu_alloc(SceKernelMemBlockType type, unsigned int size, unsigned int alignment, unsigned int attribs, SceUID *uid);
void gpu_free(SceUID uid);