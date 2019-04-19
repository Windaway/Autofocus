#ifndef AUTO_FOCUS_H_INC_
#define AUTO_FOCUS_H_INC_

typedef unsigned int uint;
typedef unsigned short ushort;

//定义对焦区域和反差值
struct roifocus{
	unsigned int roi_x;
	unsigned int roi_y;
	unsigned int roi_sharpness;
	unsigned int roi_weighted_sharpness;
};
//定义对焦区域位置
struct roidefine{
	unsigned int roi_x;
	unsigned int roi_y;
	unsigned int roi_width;
	unsigned int roi_height;
};
//按位加速求绝对值
unsigned int abs_(int value);
//按位加速求开方
unsigned short sqrt_(unsigned int n);
//计算到中心距离
static unsigned int cal_distance_tocenter(struct roidefine roi);
//得到像素点位置
unsigned char *get_position(unsigned char *image, ushort x, ushort y);

unsigned int get_focus_rate(unsigned char *image, struct roidefine roi);
//得到区域反差值
unsigned int get_region_contrast(unsigned char *image, struct roidefine roi);
//得到高反差区域
struct roidefine get_roi_region(unsigned char *image);
//验证提供的ROI区域不合适就自动取的
struct roidefine focusroi(unsigned char *image, struct roidefine roi);
//爬山策略
void focusstrategy(unsigned char *image);

#endif