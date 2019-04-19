/*
*  readBMP.c
*
*  Created by Nina Amenta on Sun May 23 2004.
*  Free to good home!
*
*/

#include <stdio.h>      // Header file for standard file i/o.
#include <stdlib.h>     // Header file for malloc/free.
#include "readBMP.h"

/* Simple BMP reading code, should be adaptable to many
systems. Originally from Windows, ported to Linux, now works on my Mac
OS system.

NOTE!! only reads 24-bit RGB, single plane, uncompressed, unencoded
BMP, not all BMPs. BMPs saved by xv should be fine. */

//
// This code was created by Jeff Molofee '99 
//  (www.demonews.com/hosted/nehe)
// Ported to Linux/GLUT by Richard Campbell '99
// Code and comments for adaptation to big endian/little endian systems 
// Nina Amenta '04
//

/* Reads a long 32 bit integer; comment out one or the other shifting line below,
whichever makes your system work right. */
unsigned int endianReadInt(FILE* file) {
	unsigned char  b[4];
	unsigned int i;

	if (fread(b, 1, 4, file) < 4)
		return 0;
	/* LITTLE VS BIG ENDIAN LINES - comment out one or the other */
	i = (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0]; // big endian
	//i = (b[0]<<24) | (b[1]<<16) | (b[2]<<8) | b[3]; // little endian
	return i;
}

/* Reads a 16 bit integer; comment out one or the other shifting line below,
whichever makes your system work right. */
unsigned short int endianReadShort(FILE* file) {
	unsigned char  b[2];
	unsigned short s;

	if (fread(b, 1, 2, file) < 2)
		return 0;
	/* LITTLE VS BIG ENDIAN LINES - comment out one or the other */
	s = (b[1] << 8) | b[0]; // big endian
	//s = (b[0]<<8) | b[1]; // little endian
	return s;
}


// quick and dirty bitmap loader...for 24 bit bitmaps with 1 plane only.  
// See http://www.dcs.ed.ac.uk/~mxr/gfx/2d/BMP.txt for more info.
int ImageLoad(char *filename, Image *image) {
	FILE *file;
	unsigned long size;                 // size of the image in bytes.
	unsigned long greysize;                 // size of the image in bytes.
	unsigned long i;                    // standard counter.
	unsigned short int planes;          // number of planes in image (must be 1) 
	unsigned short int bpp;             // number of bits per pixel (must be 24)
	unsigned char temp;                          // temporary color storage for bgr-rgb conversion.

	// make sure the file is there.
	if ((file = fopen(filename, "rb")) == NULL)
	{
		printf("File Not Found : %s\n", filename);
		return 0;
	}

	// seek through the bmp header, up to the width/height:
	fseek(file, 18, SEEK_CUR);

	// read the width
	if (!(image->sizeX = endianReadInt(file))) {
		printf("Error reading width from %s.\n", filename);
		return 0;
	}
	//printf("Width of %s: %lu\n", filename, image->sizeX);

	// read the height 
	if (!(image->sizeY = endianReadInt(file))) {
		printf("Error reading height from %s.\n", filename);
		return 0;
	}
	//printf("Height of %s: %lu\n", filename, image->sizeY);

	// calculate the size (assuming 24 bits or 3 bytes per pixel).
	size = image->sizeX * image->sizeY * 3;
	greysize = image->sizeX * image->sizeY;
	// read the planes
	if (!(planes = endianReadShort(file))) {
		printf("Error reading planes from %s.\n", filename);
		return 0;
	}
	if (planes != 1) {
		printf("Planes from %s is not 1: %u\n", filename, planes);
		return 0;
	}

	// read the bits per pixel
	if (!(bpp = endianReadShort(file))) {
		printf("Error reading bpp from %s.\n", filename);
		return 0;
	}
	if (bpp != 24) {
		printf("Bpp from %s is not 24: %u\n", filename, bpp);
		return 0;
	}

	// seek past the rest of the bitmap header.
	fseek(file, 24, SEEK_CUR);

	// read the data. 
	image->data = (unsigned char *)malloc(size);
	if (image->data == NULL) {
		printf("Error allocating memory for color-corrected image data");
		return 0;
	}

	if ((i = fread(image->data, size, 1, file)) != 1) {
		printf("Error reading image data from %s.\n", filename);
		return 0;
	}

	for (i = 0; i<size; i += 3) { // reverse all of the colors. (bgr -> rgb)
		//temp = image->data[i];
		//image->data[i] = image->data[i + 2];
		//image->data[i + 2] = temp;
		temp = image->data[i];
		image->data[i] = image->data[i + 2];
		image->data[i + 2] = temp;
	}

	image->greydata = (unsigned char *)malloc(greysize);
	if (image->greydata == NULL) {
		printf("Error allocating memory for greyimage data");
		return 0;
	}


	for (i = 0; i<greysize; i += 1) { 
		int temp = ((unsigned char(image->data[3 * i]) * 299 + unsigned char(image->data[3 * i + 1]) * 587 + unsigned char(image->data[3 * i + 2]) * 114 + 500)) / 1000;
		image->greydata[i] = unsigned char(temp);
	}


	// we're done.
	return 1;
}

