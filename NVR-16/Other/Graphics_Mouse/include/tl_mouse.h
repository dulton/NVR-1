#ifndef _MOUSE_H_
#define _MOUSE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/input.h>

struct mousedev_motion {
    int dx, dy, dz; 		//坐标的偏移量，dz表示滚轮的值
    unsigned long buttons;	//指示鼠标的状态
							//bit0: 左键的状态，0-->无动作或者释放；1-->按下
							//bit1: 右键的状态，0-->无动作或者释放；1-->按下
							//bit2: 中间键状态，0-->无动作或者释放；1-->按下
							//判断按下的动作：与上次鼠标状态的值异或一下，如果不同，且当前值为1表示按下该键了
							//判断释放的动作：与上次鼠标状态的值异或一下，如果不同，且当前值为0表示释放该键了
							//拖动的判断：左键按下，移动，左键释放的过程
    unsigned short abs_x;	//x的绝对坐标, [0, 1024]
    unsigned short abs_y;	//y的绝对坐标, [0, 768]
};

struct mousedev_status {
	unsigned char status;
	unsigned short x,y,z;
};

#define	MOUSE_LEFT_DOWN		0x1
#define	MOUSE_RIGHT_DOWN	0x2
#define	MOUSE_MID_DOWN		0x3
#define	MOUSE_LEFT_UP		0x4
#define	MOUSE_RIGHT_UP		0x5
#define	MOUSE_MID_UP		0x6
#define	MOUSE_MOVE			0x7
#define	MOUSE_WHEEL_DOWN	0x8
#define	MOUSE_WHEEL_UP		0x9
//cyl added
#define MOUSE_LEFT_DRAG 0x10
//#define MOUSE_MID_MOVE		0x10

#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
#define DEV_MOUSE "/dev/event0"
struct mouse_input_event {
	struct timeval time;
	unsigned short type;
	unsigned short code;
	int value;
};
#else
#define DEV_MOUSE "/dev/mice"//"/dev/input/mice"//"/dev/mouse0"
#endif

#endif

