/*
*  readBMP.h
*
*  Created by Nina Amenta on Sun May 23 2004.
*  Free to good home!
*
*/


/* Image type - contains height, width, and data */
struct Image {
	unsigned long sizeX;
	unsigned long sizeY;
	unsigned char *data;
	unsigned char *greydata;
};
typedef struct Image Image;

/* Function that reads in the image; first param is filename, second is image struct */
/* As side effect, sets w and h */
int ImageLoad(char* filename, Image* image);
