#ifndef _CUSTOM_MP4_COMMON_H_
#define _CUSTOM_MP4_COMMON_H_

#include <stdio.h>
//#include "iflytype.h"
#include "common_basetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#include <wtypes.h>
#else
#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID
{
    unsigned long Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
}GUID;
#endif
#include <pthread.h>
#endif

extern const GUID CUSTOMMP4_file_properties_object;
extern const GUID CUSTOMMP4_video_stream_properties_object;
extern const GUID CUSTOMMP4_audio_stream_properties_object;
extern const GUID CUSTOMMP4_mdat_vector_object;
extern const GUID CUSTOMMP4_mdat_segment_object;

#define CUSTOMMP4_FILE_DESCRIBE	"This is h.264 file ver 1"

#define O_R				0
#define O_W_CREAT		1
#define O_W_APPEND		2
#define O_W_AUTO		3
#pragma pack( push, 1 )

typedef struct
{
	GUID object_id;
	u32	 object_size;
}PACK_NO_PADDING base_object_t;

typedef struct
{
	base_object_t obj;
	char describe[32];
	u32  start_time;
	u32  end_time;
	u32  file_Size;
	u8   video_tracks;
	u8   audio_tracks;
	u16  reserved1;
}PACK_NO_PADDING file_properties_object_t;

typedef struct
{
	base_object_t obj;
	u32 time_scale;
	u16 width;
	u16 height;
	float frame_rate;
	u32 bit_rate;
	u32 compressor;
	u32 dpi_horizontal;
	u32 dpi_vertical;
	u16 depth;
	u16 flags;
	u32 reserved1;
}PACK_NO_PADDING video_stream_properties_object_t;

typedef struct
{
	base_object_t obj;
	u32 time_scale;
	u16 channels;
	u16 sample_bits;
	u32 sample_rate;
	u32 compressor;
	u32 sample_size;
	u32 sample_duration;
	u32 reserved1;
}PACK_NO_PADDING audio_stream_properties_object_t;

typedef struct
{
	base_object_t obj;
	u32 number_of_segments;
	u32 reserved1;
}PACK_NO_PADDING mdat_vector_object_t;

typedef struct
{
	base_object_t obj;
	u32 start_time;
	u32 end_time;
	u32 video_frames;
	u32 audio_frames;
	u32 video_allocated;
	u32 audio_allocated;
	u32 reserved1;
}PACK_NO_PADDING mdat_segment_object_t;

typedef struct
{
	u32	offset;
	u32 start_time;
	u32 end_time;
	u32 video_frames;
	u32 audio_frames;
	u32 video_allocated;
	u32 audio_allocated;
}PACK_NO_PADDING mdat_segment_info_table_t;

typedef struct
{
	u32	offset;
	u32 length;
	u64 pts;//wrchen 081226
	u32 timestamp;
	u8  key;
	u8  reserved1;
	u16 reserved2;
	u32 reserved3;
}PACK_NO_PADDING video_frame_info_table_t;

typedef struct
{
	u32	offset;
	u32 length;//wrchen 090115
	u64 pts;//wrchen 081226
	u32 timestamp;
	u32 reserved;
}PACK_NO_PADDING audio_frame_info_table_t;

#pragma pack( pop )

typedef struct
{
	FILE *stream;
	
	char file_name[32];
	u8 open_mode;
	
	u8 error_flag;
	u8 write_error_flog;//yaogang modify for bad disk
	
	long open_offset;
	long file_position;
	
	file_properties_object_t fpot;
	video_stream_properties_object_t vspot;
	audio_stream_properties_object_t aspot;
	mdat_vector_object_t mvot;
	
	//yaogang hdd
	//读文件时文件中所有段信息数组
	//写文件时当前段信息信息
	mdat_segment_info_table_t *p_mdat_sect_t;
	
	video_frame_info_table_t *p_video_table_t;
	audio_frame_info_table_t *p_audio_table_t;
	
	u32 video_table_allocated;
	u32 audio_table_allocated;
	
	u32 total_video_frames;
	u32 total_audio_frames;
	
	u32 current_media_sect_pos;
	u32 current_video_offset;
	u32 current_audio_offset;
	
	u32 current_video_sect_pos;//for video read
	u32 current_audio_sect_pos;//for audio read
	u32 current_video_frame_pos;//for video read
	u32 current_audio_frame_pos;//for audio read
	
/*#ifndef WIN32
	pthread_mutex_t lock;
#endif*/
#ifdef WIN32
	HANDLE lock;
#endif
}custommp4_t;

void uint2str(unsigned char *dst, int n);
u32 str2uint(const char *str);

custommp4_t* custommp4_open(const char *filename,u8 open_mode,u32 open_offset);
int custommp4_close(custommp4_t *file);

int custommp4_set_video(custommp4_t *file,u32 time_scale,u16 width,u16 height,float frame_rate,u32 bit_rate,u32 compressor,u16 depth);
int custommp4_set_audio(custommp4_t *file,u32 time_scale,u16 channels,u16 bits,u32 sample_rate,u32 compressor,u32 sample_size,u32 sample_duration);

BOOL custommp4_has_video(custommp4_t *file);
BOOL custommp4_has_audio(custommp4_t *file);

int custommp4_total_time(custommp4_t *file);

int custommp4_video_length(custommp4_t *file);
int custommp4_audio_length(custommp4_t *file);

u16 custommp4_video_width(custommp4_t *file);
u16 custommp4_video_height(custommp4_t *file);
u16 custommp4_video_depth(custommp4_t *file);
float custommp4_video_frame_rate(custommp4_t *file);
u32 custommp4_video_bit_rate(custommp4_t *file);
u32 custommp4_video_compressor(custommp4_t *file);
u32 custommp4_video_time_scale(custommp4_t *file);

u16 custommp4_audio_channels(custommp4_t *file);
u16 custommp4_audio_bits(custommp4_t *file);
u32 custommp4_audio_sample_rate(custommp4_t *file);
u32 custommp4_audio_sample_size(custommp4_t *file);
u32 custommp4_audio_compressor(custommp4_t *file);
u32 custommp4_audio_time_scale(custommp4_t *file);

int custommp4_read_one_media_frame(custommp4_t *file,u8 *meida_buffer,u32 maxBytes,u32 *start_time,u8 *key,u64 *pts,u8 *media_type);
//int custommp4_read_one_media_h264v_frame(custommp4_t *file,u8 *meida_buffer,u32 maxBytes,u32 *start_time,u8 *key,u8 *media_type,u16 *nal_size);
//组合sps pps I帧版本
int custommp4_read_one_media_frame2(custommp4_t *file,u8 *meida_buffer,u32 maxBytes,u32 *start_time,u8 *key,u64 *pts,u8 *media_type, u8 *pIframe_cnt);

int custommp4_video_frame_size(custommp4_t *file,int frame);
int custommp4_audio_frame_size(custommp4_t *file,int frame);

int custommp4_read_video_frame(custommp4_t *file,u8 *video_buffer,u32 maxBytes,int frame,u32 *start_time,u8 *key);
//int custommp4_read_h264_frame(custommp4_t *file,u8 *video_buffer,u32 maxBytes,int frame,u32 *start_time,u8 *key,u16 *nal_size);
int custommp4_read_audio_frame(custommp4_t *file,u8 *audio_buffer,u32 maxBytes,int frame,u32 *start_time);

int custommp4_read_one_video_frame(custommp4_t *file,u8 *video_buffer,u32 maxBytes,u32 *start_time,u32 *duration,u8 *key);
//int custommp4_read_one_h264_frame(custommp4_t *file,u8 *video_buffer,u32 maxBytes,u32 *start_time,u32 *duration,u8 *key,u16 *nal_size);
int custommp4_read_one_audio_frame(custommp4_t *file,u8 *audio_buffer,u32 maxBytes,u32 *start_time,u32 *duration);

int custommp4_seek_to_prev_key_frame(custommp4_t *file);

//csp modify 20130429
int custommp4_seek_to_next_key_frame(custommp4_t *file);

int custommp4_seek_to_prev_segment(custommp4_t *file);
int custommp4_seek_to_next_segment(custommp4_t *file);

int custommp4_seek_to_time_stamp(custommp4_t *file,u32 timestamp);
int custommp4_seek_to_sys_time(custommp4_t *file,u32 systime);

int custommp4_write_video_frame_with_buf(custommp4_t *file, u8 *video_buffer, u32 bytes, u32 timestamp, u8 isKeyFrame,u64 pts, u8 *update, char *p_w_buf, u32 *p_w_buf_len, u32 maxlen);
int custommp4_write_audio_frame_with_buf(custommp4_t *file,u8 *audio_buffer,u32 bytes,u32 timestamp,u8 *update, char *p_w_buf, u32 *p_w_buf_len, u32 maxlen);

int custommp4_write_video_frame(custommp4_t *file,u8 *video_buffer,u32 bytes,u32 timestamp,u8 isKeyFrame,u64 pts,u8 *update);
//int custommp4_write_h264_frame(custommp4_t *file,u8 *video_buffer,u32 bytes,u32 timestamp,u8 isKeyFrame,u8 *update,u16 nal_size);
int custommp4_write_audio_frame(custommp4_t *file,u8 *audio_buffer,u32 bytes,u32 timestamp,u8 *update);

BOOL custommp4_object_is(base_object_t *pobj,GUID type);

int custommp4_position(custommp4_t *file);
int custommp4_set_position(custommp4_t *file, int position);
int custommp4_end_position(custommp4_t *file);

int custommp4_read_data(custommp4_t *file,void *data,int size);
int custommp4_write_data(custommp4_t *file,void *data,int size);

int custommp4_set_file_start_time(custommp4_t *file, u32 start_time);
int custommp4_set_file_end_time(custommp4_t *file, u32 end_time);

#ifdef __cplusplus
}
#endif

#endif

