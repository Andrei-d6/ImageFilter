
typedef struct {
	int width, height;
	int maxval;
	int imageType;
} Pachet;

typedef struct {
	unsigned char pixel;
}BW;

typedef struct{
	unsigned char r, g, b;
} RGB;

int readImage(char *file, int *width, int *height, int *maxval, BW **bwImage, RGB **rgbImage);
void writeImage(char *file, int width, int height, int maxval, int imageType, BW *bwImage, RGB *rgbImage);

void filterBwImage(int width, int height, int maxval, BW * bwImage, BW * bwFilteredImage, char *filter);
void filterRgbImage(int width, int height, int maxval, RGB * rgbImage, RGB * rgbFilteredImage, char *filter);
