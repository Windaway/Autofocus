//By Cannian
//2017-4-17 by Ql.Liu
#include <stdio.h>
#include <stdlib.h>
#include "readBMP.h"
//读取BMP的库
#include "autofocus.h"
//自动对焦的库
#include <iostream>
int main(void){
	Image image;
	Image *imagepointer = &image;
	int ROI_x, ROI_y, ROI_width, ROI_height;//设定ROI
	char filename[30][10] = { "1.bmp",
		"2.bmp",
		"3.bmp",
		"4.bmp",
		"5.bmp",
		"6.bmp",
		"7.bmp",
		"8.bmp",
		"9.bmp",
		"10.bmp",
		"11.bmp",
		"12.bmp",
		"13.bmp",
		"14.bmp",
		"15.bmp",
		"16.bmp",
		"17.bmp",
		"18.bmp",
		"19.bmp",
		"20.bmp",
		"21.bmp",
		"22.bmp",
		"23.bmp",
		"24.bmp",
		"25.bmp",
		"26.bmp",
		"27.bmp",
		"28.bmp",
		"29.bmp",
		"30.bmp"
	};//设定
	struct roidefine ez;
	if (ImageLoad(filename[0], imagepointer)){
		int **greyimage = (int **)malloc(imagepointer->sizeY*sizeof(int *));
		struct roidefine ro;
		ro.roi_x = 300;
		ro.roi_y = 300;
		ro.roi_height = 200;
		ro.roi_width = 200;
		int b = get_region_contrast(imagepointer->greydata, ro);
		unsigned char *z = (imagepointer->data);
		ez = get_roi_region(imagepointer->greydata);
		printf("%d %d\n", ez.roi_x,ez.roi_y);
	}
	//载入图像并自动选择对焦区域

	int frame_loc=0;
	for (frame_loc = 0; frame_loc <30; ++frame_loc){
		if (ImageLoad(filename[frame_loc], imagepointer)){
			int **greyimage = (int **)malloc(imagepointer->sizeY*sizeof(int *));
			struct roidefine ro;
			int b = get_region_contrast(imagepointer->greydata, ez);
			unsigned char *z = (imagepointer->data);
			printf("%s %d\n", filename[frame_loc],b);
		}
	}
	//根据上面选择的对焦区域进行对焦值计算
	system("PAUSE");
	return 0;
}