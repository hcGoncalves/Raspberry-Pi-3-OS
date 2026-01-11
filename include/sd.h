#define SD_OK 0
#define SD_TIMEOUT -1
#define SD_ERROR -2

int sdInit();
int sdReadblock(unsigned int lba, unsigned char *buffer, unsigned int num);