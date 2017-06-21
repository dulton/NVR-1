#ifndef _PARTITION_INDEX_H_
#define _PARTITION_INDEX_H_

#include <stdio.h>
//#include "iflytype.h"
#include "common_basetypes.h"

#ifndef WIN32
#include <pthread.h>
#endif

#define _IFLY_DEBUG_DISK_PRT_
//#define _IFLY_DEBUG_DISK_PRT2_

#ifdef _IFLY_DEBUG_DISK_PRT_
	#define dbgprint(format,args...) fprintf(stderr, format, ##args)
#else
	#define dbgprint(format,args...)
#endif

#ifdef _IFLY_DEBUG_DISK_PRT2_
	#define dbgprint2(format,args...) fprintf(stderr, format, ##args)
#else
	#define dbgprint2(format,args...)
#endif

//#define SIZE_OF_FILE_VECTOR	((s64)134217728)//yaogang hdd
//#define SIZE_OF_RESERVED		((s32)393216)
//#define SIZE_OF_RESERVED		((s32)(512*1024))
#define SIZE_OF_RESERVED		((s32)(800*1024))

#define RECTYPE_MASK_TIMER	0x1
#define RECTYPE_MASK_MD		0x2
#define RECTYPE_MASK_ALARM	0x4
#define RECTYPE_MASK_HAND	0x8
#define RECTYPE_MASK_ALL	0x10

#pragma pack( push, 1 )

typedef struct
{
	u16 file_no;
	u16 sect_nums;//文件容器内部最后使用的段
	u32 start_time;
	u32 end_time;
}PACK_NO_PADDING  chn_last_use_file;

typedef struct
{
	u32 op_nums;					//
	u32 reserved1;					//
	u32 version;					//
	u32 reserved2;					//
	u32 total_file_nums;			//
	u32 recorded_file_nums;			//
	u32 full_file_nums;				//
	u32 full_file_offset;			//
	chn_last_use_file chn_info[33];	//
	u8  reserved3[80];				//
	u32 verify;						//
}PACK_NO_PADDING  partition_index_header;

typedef struct
{
	u32 file_no;
	u8  chn_no;
	u8  busy;
	u16 sect_nums;//文件容器内部最后使用的段
	u32 start_time;
	u32 end_time;
}PACK_NO_PADDING file_use_info;

typedef struct
{
	u8  type;
	u8  image_format;
	u8  stream_flag;
	u8  video_compressor;// 1:xvid; 2:jpeg; 3:h.264
	u8  audio_compressor;// 1:pcmu; 2:pcma; 3:adpcm
	u8  reserved1[11];
	u32 start_time;
	u32 end_time;
	u32 start_position;
	u32 end_position;
}PACK_NO_PADDING segment_use_info;

typedef struct
{
	u32  channel_no;//u32  ChaMask | 1<<chn
	u8   play_no;
	u8   type;
	u32  start_time;
	u32  end_time;
	u32  card_no;
	u8   mask;
}PACK_NO_PADDING search_param_t;

typedef struct
{
	u8   channel_no;//u32  channel_no;//csp modify
	u8   type;
	u32  start_time;
	u32  end_time;
	u8   image_format;//3:cif;4:4cif
	u8   stream_flag;//0:视频流;1:音频流
	u32	 size;
	u32  offset;
	u8   disk_no;
	u8   ptn_no;
	u16  file_no;
}PACK_NO_PADDING recfileinfo_t;

typedef struct
{
	u8	channel_no;
	u8	type;
	u8	snap_format;  //格式,0-jpeg,1-bmp,默认jpeg 
	u8	reserver;
	u16 	width;
	u16	height;
	u32	size;
	u32	offset;
	u8	disk_no;
	u8	ptn_no;
	u16	file_no;
	u32	start_time;
	u32	start_timeus;
} PACK_NO_PADDING recsnapinfo_t;

//yaogang modify 20141225
/*********************************************************
图片保存方案
文件容器还是256M，图片就保存在文件容器中
所有的图片的索引信息都记录在索引文件中
索引文件结构:
index_file_header
file_use_info * 128 		//分配n 个文件容器，每分区暂定128 个文件容器
pic_use_info * 32768	//图片信息，每文件容器最大图片数量，32768*8k(分辨率352*288)
*********************************************************/
//48
typedef struct
{
	u16	file_nums;			//文件容器个数PIC_FILE_NUMS	
	u16	file_cur_no;			//当前正在使用的文件容器，用完回卷0xffff	2
	u32	partition_pic_nums;	//分区保存的图片数量							2
	time_t		start_sec;	//分区使用开始时间							1
	suseconds_t	start_usec;
	time_t		end_sec;		//分区使用结束时间
	suseconds_t	end_usec;
	char reserved[24];
	u32 verify; 				//													2
}PACK_NO_PADDING  partition_pic_index_header;
//32
typedef struct
{
	u16	file_no;
	u8	status;				//使用情况: 未使用、未写满、已写满
	u8	pic_type_mask;		//抓图类型掩码，标识当前文件容器中是否保存过相应类型的图片0
	u32	chn_mask;			//通道掩码，标识当前文件容器中是否保存过相应通道的图片0
	u32	next_pic_pos;			//写下一张图片时的起始位置0xffffffff
	u32	file_pic_nums;		//文件中保存的图片数量
	time_t		start_sec;	//文件容器内第一张图片时间					1
	suseconds_t	start_usec;
	time_t		end_sec;		//文件容器内最后一张图片时间
	suseconds_t	end_usec;
}PACK_NO_PADDING file_pic_use_info;

/*抓图类型	bit_mask	value
定时			0		
移动侦测	1
报警			2 //海康只有外部报警和IPC外部报警才抓图
动测或报警			(1<<1) |(1<<2)			用于备份
动测且报警			(1<<1) |(1<<2) |(1<<7)	同上
手动			3
/////////////////智能侦测	4
手动截图	4
////////////////////////////////回放截图	6
全部类型			0xff
预录			5		因为报警后要上传发生报警前n秒的图片
日常			6		(每天两个时间点上传深广)
全部类型	0xff
*/

//24
typedef struct
{
	u8	chn;		//通道	
	u8	pic_type;	//抓图类型
	u16	file_no;	//所在文件容器
	u16 	width;
	u16	height;
	u32	file_offset;	//图片存储位置在文件容器内部偏移
	u32	pic_no;	//图片序号
	u32	pic_size;		//图片大小
	time_t		tv_sec;	//图片时间
	suseconds_t	tv_usec;
}PACK_NO_PADDING pic_use_info;

//用于保存图片文件容器数量
#define PIC_FILE_NUMS		128
#define PRE_RECSNAP_FILES	2

//图片大小以8K计，文件容器中最多可写入的图片数量
#define PIC_NUMS_PER_FILE	32768


#pragma pack( pop )

typedef struct
{
	partition_index_header header;	//分区索引头
	partition_pic_index_header pic_header;// 分区图片索引头
	FILE *index1;					//索引文件
	FILE *index2;					//备份索引文件
	FILE *pic_idx;					//图片索引文件
	FILE *pic_idx_bk;				//图片备份索引文件
//	u8   *mapdst1;					//索引文件内存映射指针
//	u8   *mapdst2;					//备份索引文件内存映射指针
//	u32  length1;						//索引文件长度
//	u32  length2;						//备份索引文件长度
	u8  *cachedst;
	u32 cachelen;
	u8  valid;						//分区是否有效
#ifndef WIN32
	pthread_mutex_t lock;			//分区使用锁
#endif
	char mountpath[64];				//分区挂载路径
} partition_index;


int fileflush(FILE *fp);
int filecp(char *src,char *dst);
long seek_to_segment(partition_index *index,partition_index_header *pHeader,int file_no,int sect_no);
int init_partition_index(partition_index *index,char *path);
int destroy_partition_index(partition_index *index);
s64 get_partition_total_space(partition_index *index);
s64 get_partition_free_space(partition_index *index);
int get_chn_next_segment(partition_index *index,int chn,int *file_no,int *sect_offset);
int get_first_full_file_end_time(partition_index *index,u32 *end_time);
int get_chn_next_segment_force(partition_index *index,int chn,int *file_no,int *sect_offset);
int update_chn_cur_segment(partition_index *index,int chn,segment_use_info *p_s_u_info,u8 finished);
int search_rec_file(partition_index *index,search_param_t *search,recfileinfo_t *fileinfo_buf,int max_nums,u8 disk_no,u8 ptn_no);

//yaogang modify 20141225
int get_index_fileinfo(partition_index* index, u16 file_no, file_pic_use_info *finfo, u8 need_lock);
int refresh_index_fileinfo(partition_index* index, file_pic_use_info *finfo, u8 need_lock);
int refresh_index_header(partition_index* index, partition_pic_index_header *pheader, u8 need_lock);
int refresh_index_picinfo(partition_index* index, pic_use_info* pinfo, u8 need_lock);

int init_partition_pic_index(partition_index* index);
int write_snapshot_to_partition(partition_index* index, u8 chn, u8 pic_type, time_t tv_sec, suseconds_t tv_usec, void *pdata, u32 data_size, u32 width, u32 height);
int search_rec_snap(partition_index *index,search_param_t *search,recsnapinfo_t *snapinfo_buf,int max_nums,u8 disk_no,u8 ptn_no);
int read_snap_file(partition_index *index, u8 *pbuf, u16 nFileNo, u32 nOffset, u32 nSize);


#endif

