#define BUFFER_WIDTH 1920
#define BUFFER_HEIGHT 1080

void framebufferInit();

void drawPixel(int x, int y, unsigned int rgb);

void drawLine(int x0, int y0, int x1, int y1, unsigned int rgb);
void drawRect(int x, int y, int width, int height, unsigned int rgb, int fill);
void drawCircle(int x, int y, int r, unsigned int rgb, int fill);

void drawChar(unsigned char ch, int x, int y, unsigned int rgb);
void drawString(int x, int y, char *s, unsigned int rgb);

void fillFramebuffer(unsigned int rgb);