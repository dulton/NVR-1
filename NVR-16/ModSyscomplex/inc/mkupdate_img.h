
#ifndef __MKUPDATE_IMG_H
#define __MKUPDATE_IMG_H

#define MAX_FILE_COUNT	5
#define MAX_FILE_NAME_LEN	32
#define FILE_MAGIC_VAL		0xF568AC87
#define FILE_MAGIC_VAL_1	0xF2E3A2B8

struct _file_info{
	__u8 name[MAX_FILE_NAME_LEN];
	__u32 size;
	__u32 offset;
};
typedef struct{
	__u32 endia;
	__u32	magic;
	__u32 version;
	__u32 crc;
	__u32 file_count;
	struct _file_info file_info[MAX_FILE_COUNT];
}img_head_t;

#endif
