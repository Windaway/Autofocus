//By QL.Liu
#include "autofocus.h"
#include "readBMP.h"
#include"stdio.h"
//预定义一些操作
#define max(a,b) (a>b)?a:b
#define min(a,b) (a<b)?a:b

//对焦信息存储
static unsigned int focus_rate;
static unsigned int last_focus_rate;
static unsigned int max_focus_rate;
static unsigned short focus_step;
static unsigned short focus_accomplish = 0;
static unsigned short step_tomax = 0;
static unsigned short epoch_tomax = 0;
//对焦方向
static char move_direction;
//设置ROI
static char roi_setted = 0;
static unsigned char have_max;
static unsigned int some_thing_wrong;
//对焦ROI
static struct roidefine image_roi;
//步长
const unsigned short min_focus_step = 1;
const unsigned short max_focus_step = 10;
const unsigned short default_focus_step = 10;
//图像反差允许值
const unsigned int rate_epsylon = 0;
//图像宽高
const unsigned short image_width = 1280;
const unsigned short image_height = 720;
const unsigned char region_width=1;
const unsigned char region_height=1;

//绝对值判断
unsigned int abs_(int value){
	int t = value >> 31;
	return t ^ (value + t);
};

//开方
unsigned short sqrt_(unsigned int n)
{
	unsigned short a = 0;
	while (n >= (2 * a) + 1)
	{
		n -= (2 * a++) + 1;
	}
	return a;
};

//计算到中心距离 并规则化，这里提供权重给选择区域
static unsigned int cal_distance_tocenter(struct roidefine roi){
	unsigned int imageCenterX = image_width >> 1;
	unsigned int imageCenterY = image_height >> 1;
	unsigned int regionCenterX = roi.roi_x + roi.roi_width /2;
	unsigned int regionCenterY = roi.roi_y + roi.roi_height/2;

	unsigned int dx = abs_(imageCenterX - regionCenterX);
	unsigned int dy = abs_(imageCenterY - regionCenterY);
	unsigned int ndx = dx * 100 / image_width;
	unsigned int ndy = dy * 100 / image_height;
	return sqrt_(ndx * ndx + ndy * ndy);
}

//判断提供的ROI是否有效，如果无效就重新选，主要是不要太靠边缘，不要太小。
struct roidefine focusroi(unsigned char *image, struct roidefine roi){
	if ((roi.roi_x>(image_width >> 3)) &&
		(roi.roi_y>(image_height >> 3)) &&
		((roi.roi_x + roi.roi_width)<((image_width << 3) * 7)) &&
		((roi.roi_y + roi.roi_height)<((image_height << 3) * 7)) &&
		(roi.roi_width>128) &&
		(roi.roi_height>128))
		return roi;
	else
		return get_roi_region(image);
};

//无意义
unsigned int getfocusrate(unsigned char *image, struct roidefine roi){
	struct roidefine focus_roi = focusroi(image, roi);
	return 1;
};

//得到坐标对应指针位置
unsigned char *getposition(unsigned char *image, ushort x, ushort y){
	return (image + y*image_width + x);
};

//得到区域的反差
unsigned int get_region_contrast(unsigned char *image, struct roidefine roi){
	/*
	* const unsigned char line_count=7;
	* to optimize the algorithm
	*/
	unsigned char step_y = roi.roi_height >> 3 + 1;
	unsigned char step_x = roi.roi_width >> 3 + 1;
	unsigned int sharpness = 0;
	for (unsigned char y = step_y; (y + 4) < roi.roi_height; y += step_y)
	{
		unsigned int max_contrast = 0;
//上下分块比较
		for (unsigned char x = 0; (x + 16) < roi.roi_width; x += 4)
		{
			int a = 0;
			for (unsigned char i = 0; i < 8; ++i)
				for (unsigned char j = 0; j < 4; ++j)
					a += *getposition(image, roi.roi_x + x + i, roi.roi_y + y + j);

			a = a >> 4;

			unsigned int b = 0;
			for (unsigned char i = 8; i < 16; ++i)
				for (unsigned char j = 0; j< 4; ++j)
					b += *getposition(image, roi.roi_x + x + i, roi.roi_y + y + j);

			b = b >> 4;

			unsigned int contrast = abs_(a - b);
			max_contrast = max(contrast, max_contrast);
		}
		sharpness += max_contrast;
	}
//左右分块比较
	for (unsigned int x = step_x; (x + 4) < roi.roi_width; x += step_x)
	{
		unsigned int max_contrast = 0;

		for (unsigned int y = 0; (y + 16) < roi.roi_height; y += 4)
		{
			int a = 0;
			for (unsigned int i = 0; i < 8; ++i)
				for (unsigned int j = 0; j< 4; ++j)
					a += *getposition(image, roi.roi_x + x + i, roi.roi_y + y + j);

			a = a >> 4;

			int b = 0;
			for (unsigned int i = 8; i < 16; ++i)
				for (unsigned int j = 0; j< 4; ++j)
					b += *getposition(image, roi.roi_x + x + i, roi.roi_y + y + j);

			b = b >> 4;

			unsigned int contrast = abs_(a - b);
			max_contrast = max(contrast, max_contrast);
		}
		sharpness += max_contrast;
	}
	return sharpness;
};


//获取高反差的块
struct roidefine get_roi_region(unsigned char *image){
	struct roifocus rf[200];
	struct roidefine result;
	unsigned short regions_x = image_width / 128;
	unsigned short regions_y = image_height / 128;
	unsigned int start_x = (image_width - regions_x *128)/2;
	unsigned int start_y = (image_height - regions_y *128) /2;
//分区计算反差值
	for (unsigned short y = 0; y < regions_y; ++y)
	{
		for (unsigned short x = 0; x < regions_x; ++x)
		{
			struct roidefine r;
			r.roi_x = start_x + x *128;
			r.roi_y = start_y + y * 128;
			r.roi_height = 128;
			r.roi_width = 128;
			rf[x + y*regions_x].roi_x = r.roi_x;
			rf[x + y*regions_x].roi_y = r.roi_y;
			rf[x + y*regions_x].roi_sharpness = get_region_contrast(image, r);
			rf[x + y*regions_x].roi_weighted_sharpness = 0;
		}
	}
//重新分配权重
	for (unsigned short y = 0; y < regions_y; ++y)
	{
		for (unsigned short x = 0; x < regions_x; ++x)
		{
			unsigned short x0 = x > 0 ? x - 1 : x;
			unsigned short x1 = x < (regions_x - 1) ? x + 1 : x;
			unsigned short y0 = y > 0 ? y - 1 : y;
			unsigned short y1 = y < (regions_y - 1) ? y + 1 : y;

			unsigned int extra_sharpness = 0;

			for (unsigned short iy = y0; iy < y1; ++iy)
			{
				for (unsigned short ix = x0; ix < x1; ++ix)
				{
					if (iy != 0 || ix != 0)
					{
						extra_sharpness += rf[iy*regions_x + ix].roi_sharpness /8;
					}
				}
			}

			struct roidefine r;
			r.roi_x = rf[y*regions_x + x].roi_x;
			r.roi_y = rf[y*regions_x + x].roi_y;
			r.roi_height = 128;
			r.roi_width = 128;
			unsigned int center_distance = cal_distance_tocenter(r) + 60;
			rf[x+y*regions_x].roi_weighted_sharpness = (unsigned int)((rf[y*regions_x + x].roi_sharpness + extra_sharpness) * 10000 / (center_distance*center_distance)); 
			// Remain to choose a good  method to normalize these sharpness data.
			/*
			*I`d like to normalize the distance to center with dx/image_width*100 & dy/image_height*100, then calculate the normalized distance.
			*The weighted sharpness use (distance+60)*(distance+60)/10000, but divide will use a mass of compute resource.
			*/
		}
	}
	unsigned int maxsharpness = 0;
	unsigned char maxroi = 0;
	
	unsigned short aaa = regions_x* regions_y;
	
	for (int i = 0; i<aaa; ++i){
		if (rf[i].roi_weighted_sharpness>maxsharpness){
			maxsharpness = rf[i].roi_weighted_sharpness;
			maxroi = i;
		}
	}
	result.roi_x = rf[maxroi].roi_x;
	result.roi_y = rf[maxroi].roi_y;
	result.roi_height = 128;
	result.roi_width = 128;
	return result;
};

//爬山算法原理
void focusstrategy(unsigned char *image){

	if (!roi_setted){
		image_roi = get_roi_region(image);
		roi_setted = 1;
	}
	focus_rate = get_region_contrast(image, image_roi);

	if (focus_accomplish)
		focus_accomplish = 0;

	if (some_thing_wrong){
		focus_step = default_focus_step;
		move_direction -= move_direction;
		last_focus_rate = focus_rate;
		return;
	}

	if (focus_rate>max_focus_rate){
		max_focus_rate = focus_rate;
		step_tomax = 0;
		epoch_tomax = 0;
		last_focus_rate = focus_rate;
		have_max = 1;
		return;
	}

	if (have_max){
		if (epoch_tomax == 3)
			if ((focus_rate + rate_epsylon)<max_focus_rate){
				move_direction -= move_direction;
				focus_step = step_tomax;
				focus_accomplish = 2;
				last_focus_rate = focus_rate;
				return;
			}
	}

	int delta_rate = focus_rate - last_focus_rate;

	if (delta_rate<-rate_epsylon){
		move_direction -= move_direction;
		focus_step = (focus_step - 1<min_focus_step ? min_focus_step : focus_step - 1);
		last_focus_rate = focus_rate;
	}

	if (delta_rate>rate_epsylon){
		step_tomax += focus_step;
		focus_step = (focus_step + 1>max_focus_step ? max_focus_step : focus_step + 1);
		last_focus_rate = focus_rate;
		++epoch_tomax;
		return;
	}

};//爬山算法待验证效果