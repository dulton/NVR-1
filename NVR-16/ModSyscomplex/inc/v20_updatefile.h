#ifndef _V20_UPDATEFILE_H_
#define _V20_UPDATEFILE_H_

#if 0//csp modify
typedef unsigned long U32;
typedef unsigned char U8;

#define V20_FILE_MAGIC	0xAB34F100
#define MAX_FILE_NAME_LEN	32

#define FS_TYPE_RAMDISK	1
#define FS_TYPE_CRAMFS	2
#define FS_TYPE_JFS	3

typedef enum{//下面的值不能随意修改
	FT_BYFNAME = 0,
	FT_BOOTLOAD = 1,
	FT_KERNEL = 2,
	FT_FS_ROOT = 3,
	FT_FS_NORMAL = 4,
	FT_OTHER = 5,
	FT_CONTAINER = 6 //放各种类型文件
}file_type_e;

typedef struct{
	U32 magic;
	U32 crc;
	U32 file_cnt;
	U32 flash_size;
	U32 rsv;
	U8 user_info[64];
}file_head_t;

typedef struct{
	U32 file_oft;
	U32 flash_oft;
	U32 mtd_size;
	U32 len;
	file_type_e ft;
	U32 ft_arg;
	U8 file_name[MAX_FILE_NAME_LEN+4];
}file_info_t;
#endif

#endif

