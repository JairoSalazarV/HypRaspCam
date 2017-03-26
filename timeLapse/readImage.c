#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"
#include "stb_image.h"


uint8_t* subimage( int x1, int y1, int croppedRows, int croppedCols, uint8_t* img, int imgRows, int imgCols );

int main() {

	//
	// Read image
	//
	int width, height, bpp;
	uint8_t* rgb_image = stbi_load("../tmpTimeLapse/1.png", &width, &height, &bpp, 3);    	

	//
    // Example: crop image and save
	//
	int cropR = 600;
	int cropC = 1500;
	uint8_t* croppedImg = subimage( 200, 700, cropR, cropC, rgb_image, height, width );
	stbi_write_png("duckCroped.png", cropC, cropR, 3, croppedImg, cropC*3);

	//
    // It finishes
	//
	printf("It finished successfully...\n\n");
	stbi_image_free(croppedImg);
	stbi_image_free(rgb_image);
    return 0;
    
}


uint8_t* subimage( int x1, int y1, int croppedRows, int croppedCols, uint8_t* img, int imgRows, int imgCols )
{ 
	
	uint8_t* croppedImg = (uint8_t*)malloc(croppedRows*croppedCols*3);
	
	if( 
		croppedRows < 1 || croppedCols < 1 || 
		x1 > imgRows 	|| x1 < 0 || 
		y1 > imgCols 	|| y1 < 0
	)
	{
		printf("Crop coordinates are out of bounds \n");
		return croppedImg;
	}

	int beforeX1	= (x1>0)?x1:0;
	int afterX2 	= ((x1+croppedCols)<imgCols)?imgCols-(x1+croppedCols):0;
	int beforeY1	= (y1>0)?y1-1:0;
	int step		= beforeX1 + afterX2;
	
	//printf( "img(%d x %d) | cropped(%d x %d) beforeX1: %d, afterX2: %d, step: %d\n", imgRows, imgCols, croppedRows, croppedCols,  beforeX1, afterX2, step );
	
	int r, c;
	uint8_t *ptr, *cropPtr;
	
	cropPtr = &croppedImg[0];
	ptr		= &img[0];
	ptr	   += ((beforeY1 * imgCols) + beforeX1)*3;

	for( r=0; r<croppedRows; r++ )
	{
		memcpy( cropPtr, ptr, croppedCols*3 );
		ptr 	+= imgCols*3;
		cropPtr	+= croppedCols*3;
	}
	
	return croppedImg;

}







