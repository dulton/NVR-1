#include "common.h"
#include "custommp4.h"
#include "fs_yg.h"//yaogang hdd
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#ifndef WIN32
#include <unistd.h>
#include <stdlib.h>
#endif

//#define SIZE_OF_FILE_VECTOR		((s64)134217728)//yaogang hdd

extern int SetReadDataFlag(u8 byFlag);
extern int fileflush(FILE *fp);

//extern int tl_power_atx_check();

const GUID CUSTOMMP4_file_properties_object			= {0xABD3D211L,0xA9BA,0x11cf,{0x8E,0xE6,0x00,0xC0,0x0C,0x20,0x53,0x65}};
const GUID CUSTOMMP4_video_stream_properties_object	= {0x86D15241L,0x311D,0x11D0,{0xA3,0xA4,0x00,0xA0,0xC9,0x03,0x48,0xF6}};
const GUID CUSTOMMP4_audio_stream_properties_object	= {0x4B1ACBE3L,0x100B,0x11D0,{0xA3,0x9B,0x00,0xA0,0xC9,0x03,0x48,0xF6}};
const GUID CUSTOMMP4_mdat_vector_object				= {0x4CFEDB20L,0x75F6,0x11CF,{0x9C,0x0F,0x00,0xA0,0xC9,0x03,0x49,0xCB}};
const GUID CUSTOMMP4_mdat_segment_object			= {0x1806D474L,0xCADF,0x4509,{0xA4,0xBA,0x9A,0xAB,0xCB,0x96,0xAA,0xE8}};

int lock_custommp4(custommp4_t *file)
{
/*#ifndef WIN32
	return pthread_mutex_lock(&file->lock);
#endif*/
	
/* 2007-11-18 */
#ifdef WIN32
	WaitForSingleObject(file->lock,INFINITE);
#endif
/* 2007-11-18 */
	
	return 1;
}

int unlock_custommp4(custommp4_t *file)
{
/*#ifndef WIN32
	return pthread_mutex_unlock(&file->lock);
#endif*/
	
/* 2007-11-18 */
#ifdef WIN32
	ReleaseMutex(file->lock);
#endif
/* 2007-11-18 */
	
	return 1;
}

int custommp4_init(custommp4_t *file)
{
	memset(file, 0, sizeof(custommp4_t));
	
/*#ifndef WIN32
	pthread_mutex_init(&file->lock,NULL);
#endif*/
	
/* 2007-11-18 */
#ifdef WIN32
	file->lock = CreateMutex(NULL, FALSE, NULL);
#endif
/* 2007-11-18 */
	
	//file->error_flag = 0;
	
	return 1;
}

int custommp4_destroy(custommp4_t *file)
{
/*#ifndef WIN32
	pthread_mutex_destroy(&file->lock);
#endif*/
	
/* 2007-11-18 */
#ifdef WIN32
	CloseHandle(file->lock);
#endif
/* 2007-11-18 */
	
	int result = 1;
	
	if(file->p_mdat_sect_t) free(file->p_mdat_sect_t);
	if(file->p_video_table_t) free(file->p_video_table_t);
	if(file->p_audio_table_t) free(file->p_audio_table_t);
	
	if(file->stream/* && file->error_flag == 0*/)
	{
		int result = fclose(file->stream);
		if(result)
		{
			printf("custommp4_destroy:fclose error=%d,errno=%d,errstr=%s\n",result,errno,strerror(errno));
			#ifndef WIN32
			//sleep(1);
			//exit(1);
			#endif
			result = -1;
		}
	}
	free(file);
	
	//return 1;
	return result;
}

int custommp4_file_properties_object_init(file_properties_object_t *pfpot)
{
	pfpot->obj.object_id = CUSTOMMP4_file_properties_object;
	pfpot->obj.object_size = sizeof(file_properties_object_t);
	pfpot->video_tracks = 0;
	pfpot->audio_tracks = 0;
	pfpot->start_time = (u32)(time(NULL));
	pfpot->end_time = pfpot->start_time;
	pfpot->file_Size = sizeof(file_properties_object_t)+sizeof(video_stream_properties_object_t)+sizeof(audio_stream_properties_object_t)+sizeof(mdat_vector_object_t);
	//printf("custommp4_file_properties_object_init:file_Size=%d\n",pfpot->file_Size);
	pfpot->reserved1 = 0;
	strncpy(pfpot->describe,CUSTOMMP4_FILE_DESCRIBE,sizeof(pfpot->describe));
	return 1;
}

int custommp4_video_stream_properties_object_init(video_stream_properties_object_t *pvspot)
{
	pvspot->obj.object_id = CUSTOMMP4_video_stream_properties_object;
	pvspot->obj.object_size = sizeof(video_stream_properties_object_t);
	pvspot->time_scale = 1000;
	pvspot->width = 352;
	pvspot->height = 288;
	pvspot->frame_rate = 25;
	pvspot->bit_rate = 512*1024;
	pvspot->compressor = str2uint("XVID");
	pvspot->depth = 0x18;
	pvspot->dpi_horizontal = 72;
	pvspot->dpi_vertical = 72;
	pvspot->flags = 0;
	pvspot->reserved1 = 0;
	return 1;
}

int custommp4_audio_stream_properties_object_init(audio_stream_properties_object_t *paspot)
{
	paspot->obj.object_id = CUSTOMMP4_audio_stream_properties_object;
	paspot->obj.object_size = sizeof(audio_stream_properties_object_t);
	paspot->time_scale = 1000;
	paspot->channels = 1;
	paspot->sample_bits = 16;//8;//csp modify
	paspot->sample_rate = 8000;
	paspot->compressor = str2uint("PCMU");
	paspot->sample_duration = 30;
	//paspot->sample_size = 240;
	#ifdef USE_AUDIO_PCMU
	paspot->sample_size = 642;
	#else
	paspot->sample_size = 168;//wrchen 090116
	#endif
	
	paspot->reserved1 = 0;
	return 1;
}

int custommp4_mdat_vector_object_init(mdat_vector_object_t *pmvot)
{
	pmvot->obj.object_id = CUSTOMMP4_mdat_vector_object;
	pmvot->obj.object_size = sizeof(mdat_vector_object_t);
	pmvot->number_of_segments = 0;
	pmvot->reserved1 = 0;
	return 1;
}

int custommp4_mdat_segment_info_table_init(custommp4_t *file, mdat_segment_info_table_t *p_mdat_sect_t)
{
	p_mdat_sect_t->video_frames = 0;
	p_mdat_sect_t->audio_frames = 0;
	p_mdat_sect_t->start_time = time(NULL);
	p_mdat_sect_t->end_time = p_mdat_sect_t->start_time;
	p_mdat_sect_t->offset = custommp4_position(file);
	if(file->fpot.video_tracks) p_mdat_sect_t->video_allocated = 3000;
	else p_mdat_sect_t->video_allocated = 0;
	if(file->fpot.audio_tracks) p_mdat_sect_t->audio_allocated = 4000;
	else p_mdat_sect_t->audio_allocated = 0;
	return 1;
}

/*读取文件自定义属性*/
int ReadFilePropertiesObject(custommp4_t *file)
{
	return custommp4_read_data(file,&file->fpot,sizeof(file_properties_object_t));
}




/*读取文件视频流属性*/
int ReadVideoStreamPropertiesObject(custommp4_t *file)
{
	return custommp4_read_data(file,&file->vspot,sizeof(video_stream_properties_object_t));
}


/*读取文件音频流属性*/
int ReadAudioStreamPropertiesObject(custommp4_t *file)
{
	return custommp4_read_data(file,&file->aspot,sizeof(audio_stream_properties_object_t));
}


int ReadmdatVectorObject(custommp4_t *file)
{
	return custommp4_read_data(file,&file->mvot,sizeof(mdat_vector_object_t));
}




int ReadmdatSegmentObject(custommp4_t *file,int index)
{
	mdat_segment_object_t msot;
	int ret;
	file->p_mdat_sect_t[index].offset = custommp4_position(file);
	memset(&msot,0,sizeof(msot));
	ret = custommp4_read_data(file, &msot, sizeof(mdat_segment_object_t));
	if(ret<=0)
	{
		printf("ReadmdatSegmentObject%d (offset:%d) failed 1\n",index+1,file->p_mdat_sect_t[index].offset);
		return 0;
	}
	if(!custommp4_object_is(&msot.obj,CUSTOMMP4_mdat_segment_object))
	{
		printf("ReadmdatSegmentObject%d (offset:%d) failed 2\n",index+1,file->p_mdat_sect_t[index].offset);
		return 0;
	}
	if(msot.obj.object_size<sizeof(mdat_segment_object_t))
	{
		printf("ReadmdatSegmentObject%d (offset:%d) failed 3\n",index+1,file->p_mdat_sect_t[index].offset);
		return 0;
	}
	//printf("ReadmdatSegmentObject%d (offset:%d,size:%d) success\n",index+1,file->p_mdat_sect_t[index].offset,msot.obj.object_size);
	/*printf("ReadmdatSegmentObject%d start_time=%d,end_time=%d,video_frames=%d,audio_frames=%d,video_allocated=%d,audio_allocated=%d,sizeof(msot)=%d\n",
		index+1,
		msot.start_time,
		msot.end_time,
		msot.video_frames,
		msot.audio_frames,
		msot.video_allocated,
		msot.audio_allocated,
		sizeof(msot));*/
	file->p_mdat_sect_t[index].start_time = msot.start_time;
	file->p_mdat_sect_t[index].end_time = msot.end_time;
	file->p_mdat_sect_t[index].video_frames = msot.video_frames;
	file->p_mdat_sect_t[index].audio_frames = msot.audio_frames;
	file->p_mdat_sect_t[index].video_allocated = msot.video_allocated;
	file->p_mdat_sect_t[index].audio_allocated = msot.audio_allocated;
	ret = custommp4_set_position(file,file->p_mdat_sect_t[index].offset+msot.obj.object_size);
	if(ret <= 0)
	{
		printf("ReadmdatSegmentObject%d (offset:%d) failed 4\n",index+1,file->p_mdat_sect_t[index].offset);
		return 0;
	}
	file->total_video_frames += msot.video_frames;
	file->total_audio_frames += msot.audio_frames;
	return ret;
}

int custommp4_get_segment_of_video_frame(u32 *p_sect_offset,u32 *p_sect,custommp4_t *file,int frame)
{
	u32 i;
	u32 frame_count = 0;
	if((u32)frame>=file->total_video_frames)
	{
		return 0;
	}
	for(i=0;i<file->mvot.number_of_segments;i++)
	{
		frame_count += file->p_mdat_sect_t[i].video_frames;
		if(frame_count > (u32)frame)
		{
			break;
		}
	}
	*p_sect = i;
	*p_sect_offset = frame-(frame_count-file->p_mdat_sect_t[i].video_frames);
	return 1;
}

int custommp4_get_segment_of_audio_frame(u32 *p_sect_offset,u32 *p_sect,custommp4_t *file,int frame)
{
	u32 i;
	u32 frame_count = 0;
	if((u32)frame>=file->total_audio_frames)
	{
		return 0;
	}
	for(i=0;i<file->mvot.number_of_segments;i++)
	{
		frame_count += file->p_mdat_sect_t[i].audio_frames;
		if(frame_count > (u32)frame)
		{
			break;
		}
	}
	*p_sect = i;
	*p_sect_offset = frame-(frame_count-file->p_mdat_sect_t[i].audio_frames);
	return 1;
}

int ReadmdatIndexTable(custommp4_t *file,int index)
{
	int ret;
	u32 i;
	SetReadDataFlag(1);
	//printf("video_table_allocated: %d\n", file->video_table_allocated);
	//printf("video_frames: %d\n", file->p_mdat_sect_t[index].video_frames);
	if(file->video_table_allocated < file->p_mdat_sect_t[index].video_frames)
	{
		file->video_table_allocated = file->p_mdat_sect_t[index].video_frames;
		file->p_video_table_t = realloc(file->p_video_table_t,file->video_table_allocated*sizeof(video_frame_info_table_t));
		if(file->video_table_allocated && file->p_video_table_t == NULL)
		{
			printf("read video index table failed:realloc\n");
			return 0;
		}
	}
	if(file->audio_table_allocated < file->p_mdat_sect_t[index].audio_frames)
	{
		file->audio_table_allocated = file->p_mdat_sect_t[index].audio_frames;
		file->p_audio_table_t = realloc(file->p_audio_table_t,file->audio_table_allocated*sizeof(audio_frame_info_table_t));
		if(file->audio_table_allocated && file->p_audio_table_t == NULL)
		{
			printf("read audio index table failed:realloc\n");
			return 0;
		}
	}
	if(file->p_mdat_sect_t[index].video_frames)
	{
		custommp4_set_position(file,file->p_mdat_sect_t[index].offset+sizeof(mdat_segment_object_t));
		ret = custommp4_read_data(file, file->p_video_table_t, file->p_mdat_sect_t[index].video_frames*sizeof(video_frame_info_table_t));
		if(ret<=0)
		{
			printf("read video index table failed:read\n");
			return 0;
		}
		//csp modify:相对偏移

		for(i=0;i<file->p_mdat_sect_t[index].video_frames;i++)
		{

			file->p_video_table_t[i].offset += file->open_offset;
		}
	}
	if(file->p_mdat_sect_t[index].audio_frames)
	{
		custommp4_set_position(file,file->p_mdat_sect_t[index].offset+sizeof(mdat_segment_object_t)+file->p_mdat_sect_t[index].video_allocated*sizeof(video_frame_info_table_t));
		ret = custommp4_read_data(file, file->p_audio_table_t, file->p_mdat_sect_t[index].audio_frames*sizeof(audio_frame_info_table_t));
		if(ret<=0)
		{
			printf("read audio index table failed:read\n");
			return 0;
		}
		//csp modify:相对偏移
		for(i=0;i<file->p_mdat_sect_t[index].audio_frames;i++)
		{
			file->p_audio_table_t[i].offset += file->open_offset;
		}
	}
	file->current_video_sect_pos = index;
	file->current_audio_sect_pos = index;

	file->current_media_sect_pos = index;
	file->current_video_offset = 0;
	file->current_audio_offset = 0;

	return 1;
 }

int ReadmdatVideoIndexTable(custommp4_t *file,int index)
{
	int ret;
	u32 i;
	if(file->video_table_allocated < file->p_mdat_sect_t[index].video_frames)
	{
		file->video_table_allocated = file->p_mdat_sect_t[index].video_frames;
		file->p_video_table_t = realloc(file->p_video_table_t,file->video_table_allocated*sizeof(video_frame_info_table_t));
		if(file->video_table_allocated && file->p_video_table_t == NULL)
		{
			printf("read video index table failed 1\n");
			return 0;
		}
	}
	if(file->p_mdat_sect_t[index].video_frames)
	{
		custommp4_set_position(file,file->p_mdat_sect_t[index].offset+sizeof(mdat_segment_object_t));
		ret = custommp4_read_data(file,file->p_video_table_t,file->p_mdat_sect_t[index].video_frames*sizeof(video_frame_info_table_t));
		if(ret<=0)
		{
			printf("read video index table failed 2\n");
			return 0;
		}
		//csp modify:相对偏移
		for(i=0;i<file->p_mdat_sect_t[index].video_frames;i++)
		{
			file->p_video_table_t[i].offset += file->open_offset;
		}
	}
	file->current_video_sect_pos = index;
	return 1;
}

int ReadmdatAudioIndexTable(custommp4_t *file,int index)
{
	int ret;
	u32 i;
	if(file->audio_table_allocated < file->p_mdat_sect_t[index].audio_frames)
	{
		file->audio_table_allocated = file->p_mdat_sect_t[index].audio_frames;
		file->p_audio_table_t = realloc(file->p_audio_table_t,file->audio_table_allocated*sizeof(audio_frame_info_table_t));
		if(file->audio_table_allocated && file->p_audio_table_t == NULL)
		{
			printf("read audio index table failed 1\n");
			return 0;
		}
	}
	if(file->p_mdat_sect_t[index].audio_frames)
	{
		custommp4_set_position(file,file->p_mdat_sect_t[index].offset+sizeof(mdat_segment_object_t)+file->p_mdat_sect_t[index].video_allocated*sizeof(video_frame_info_table_t));
		ret = custommp4_read_data(file,file->p_audio_table_t,file->p_mdat_sect_t[index].audio_frames*sizeof(audio_frame_info_table_t));
		if(ret<=0)
		{
			printf("read audio index table failed 2\n");
			return 0;
		}
		//csp modify:相对偏移
		for(i=0;i<file->p_mdat_sect_t[index].audio_frames;i++)
		{
			file->p_audio_table_t[i].offset += file->open_offset;
		}
	}
	file->current_audio_sect_pos = index;
	return 1;
}

custommp4_t* custommp4_open(const char *filename,u8 open_mode,u32 open_offset)
{
	custommp4_t *file;
	char flags[10];
	//u32 sect_offset,sect;
	
	if(open_mode == O_R)
	{
		sprintf(flags,"rb");
	}
	else if(open_mode == O_W_CREAT)
	{
		open_offset = 0;
		sprintf(flags,"wb");
	}
	else if(open_mode == O_W_APPEND)
	{
		sprintf(flags,"rb+");
	}
	else if(open_mode == O_W_AUTO)
	{
		FILE *fp = fopen(filename,"rb+");
		if(fp != NULL)
		{
			fclose(fp);
			sprintf(flags,"rb+");
		}
		else
		{
			open_offset = 0;
			sprintf(flags,"wb");
		}
	}
	else
	{
		printf("open mode error\n");
		return NULL;
	}
	
	file = (custommp4_t *)malloc(sizeof(custommp4_t));
	if(file == NULL) return NULL;
	
	custommp4_init(file);
	
	file->stream = fopen(filename,flags);
	if(file->stream == NULL)
	{
		//printf("open file failed\n");
		perror("open file failed, err: ");
		custommp4_destroy(file);
		return NULL;
	}
	strcpy(file->file_name, filename);
	file->open_mode = open_mode;
	file->open_offset = open_offset;
	file->file_position = open_offset;
	fseek(file->stream, open_offset, SEEK_SET);
	if(open_mode == O_R)
	{
		int ret;
		u32 i;
		ret = ReadFilePropertiesObject(file);
		if(ret<=0)
		{
			printf("ReadFilePropertiesObject failed 1\n");
			custommp4_destroy(file);
			return NULL;
		}
		if(!custommp4_object_is(&file->fpot.obj,CUSTOMMP4_file_properties_object))
		{
			printf("ReadFilePropertiesObject failed 2\n");
			custommp4_destroy(file);
			return NULL;
		}
		if(file->fpot.obj.object_size != sizeof(file->fpot))
		{
			printf("ReadFilePropertiesObject failed 3\n");
			custommp4_destroy(file);
			return NULL;
		}
		//printf("start time=%d,end time=%d,video tracks=%d,audio tracks=%d,size=%d\n",file->fpot.start_time,file->fpot.end_time,file->fpot.video_tracks,file->fpot.audio_tracks,file->fpot.file_Size);
		/*printf("size:(%d,%d,%d,%d),total size:%d\n",
			sizeof(file->fpot),sizeof(file->vspot),sizeof(file->aspot),sizeof(file->mvot),
			sizeof(file->fpot)+sizeof(file->vspot)+sizeof(file->aspot)+sizeof(file->mvot));*/
		
		ret = ReadVideoStreamPropertiesObject(file);
		if(ret<=0 || !custommp4_object_is(&file->vspot.obj,CUSTOMMP4_video_stream_properties_object) || file->vspot.obj.object_size != sizeof(file->vspot))
		{
			printf("ReadVideoStreamPropertiesObject failed\n");
			custommp4_destroy(file);
			return NULL;
		}
		ret = ReadAudioStreamPropertiesObject(file);
		if(ret<=0 || !custommp4_object_is(&file->aspot.obj,CUSTOMMP4_audio_stream_properties_object) || file->aspot.obj.object_size != sizeof(file->aspot))
		{
			printf("ReadAudioStreamPropertiesObject failed\n");
			custommp4_destroy(file);
			return NULL;
		}
		ret = ReadmdatVectorObject(file);
		if(ret<=0 || !custommp4_object_is(&file->mvot.obj,CUSTOMMP4_mdat_vector_object))
		{
			printf("ReadAudioStreamPropertiesObject failed\n");
			custommp4_destroy(file);
			return NULL;
		}
		//printf("file has %d segment%s\n",file->mvot.number_of_segments,(file->mvot.number_of_segments>1)?"s":"");
		/*if(file->mvot.number_of_segments == 0)
		{
			file->mvot.number_of_segments = 1;
			printf("change:file has %d segment%s\n",file->mvot.number_of_segments,(file->mvot.number_of_segments>1)?"s":"");
		}*/
		if(file->mvot.number_of_segments)
		{
			file->p_mdat_sect_t = malloc(file->mvot.number_of_segments*sizeof(mdat_segment_info_table_t));
			if(file->p_mdat_sect_t == NULL)
			{
				printf("alloc memory for sect table failed\n");
				custommp4_destroy(file);
				return NULL;
			}
			for(i=0;i<file->mvot.number_of_segments;i++)
			{
				if(ReadmdatSegmentObject(file,i)<=0)
				{
					#ifndef WIN32
					custommp4_destroy(file);
					return NULL;
					#else
					if(i == 0)
					{
						custommp4_destroy(file);
						return NULL;
					}
					else
					{
						break;
					}
					#endif
				}
			}
			#ifdef WIN32
			if(i != file->mvot.number_of_segments)
			{
				file->mvot.number_of_segments = i;
			}
			#endif
			if(file->total_video_frames)
			{
				file->video_table_allocated = file->total_video_frames<3000?file->total_video_frames:3000;
				file->p_video_table_t = malloc(file->video_table_allocated*sizeof(video_frame_info_table_t));
				if(file->video_table_allocated && file->p_video_table_t == NULL)
				{
					printf("alloc memory for video table failed\n");
					custommp4_destroy(file);
					return NULL;
				}
				
				/*if(custommp4_get_segment_of_video_frame(&sect_offset,&sect,file,0))
				{
					if(ReadmdatVideoIndexTable(file,sect)<=0)
					{
						custommp4_destroy(file);
						return NULL;
					}
					file->current_video_frame_pos = 0;
				}*/
			}
			if(file->total_audio_frames)
			{
				file->audio_table_allocated = file->total_audio_frames<4000?file->total_audio_frames:4000;
				file->p_audio_table_t = malloc(file->audio_table_allocated*sizeof(audio_frame_info_table_t));
				if(file->audio_table_allocated && file->p_audio_table_t == NULL)
				{
					printf("alloc memory for audio table failed\n");
					custommp4_destroy(file);
					return NULL;
				}
				
				/*if(custommp4_get_segment_of_audio_frame(&sect_offset,&sect,file,0))
				{
					if(ReadmdatAudioIndexTable(file,sect)<=0)
					{
						custommp4_destroy(file);
						return NULL;
					}
					file->current_audio_frame_pos = 0;
				}*/
			}
			if(ReadmdatIndexTable(file,0)<=0)
			{
				custommp4_destroy(file);
				return NULL;
			}
			
			//printf("custommp4_open:hehe,file_Size=%d,start_time=%d,end_time=%d,videos=%d,audios=%d\n",file->fpot.file_Size,file->fpot.start_time,file->fpot.end_time,file->total_video_frames,file->total_audio_frames);
			
			//printf("custommp4_open:file_Size=%d\n",file->fpot.file_Size);
			
			if(file->fpot.end_time == file->fpot.start_time)
			{
				if(file->mvot.number_of_segments)
				{
					file->fpot.end_time = file->fpot.start_time+(file->p_mdat_sect_t[file->mvot.number_of_segments-1].end_time-file->p_mdat_sect_t[0].start_time)/1000;
				}
				/*else
				{
					file->fpot.end_time = file->fpot.start_time;
				}*/
			}
			//printf("custommp4_open:file end time=%d\n",file->fpot.end_time);
		}
	}
	else
	{
		mdat_segment_object_t msot;
		
		custommp4_file_properties_object_init(&file->fpot);
		custommp4_video_stream_properties_object_init(&file->vspot);
		custommp4_audio_stream_properties_object_init(&file->aspot);
		custommp4_mdat_vector_object_init(&file->mvot);

		//yaogang modify for bad disk
		#if 0
		custommp4_write_data(file,&file->fpot,sizeof(file->fpot));
		custommp4_write_data(file,&file->vspot,sizeof(file->vspot));
		custommp4_write_data(file,&file->aspot,sizeof(file->aspot));
		custommp4_write_data(file,&file->mvot,sizeof(file->mvot));
		#else
		if (custommp4_write_data(file,&file->fpot,sizeof(file->fpot)) != 1)
		{
			custommp4_destroy(file);
			file = NULL;
			return NULL;
		}
		
		if (custommp4_write_data(file,&file->vspot,sizeof(file->vspot)) != 1)
		{
			custommp4_destroy(file);
			file = NULL;
			return NULL;
		}
		
		if (custommp4_write_data(file,&file->aspot,sizeof(file->aspot)) != 1)
		{
			custommp4_destroy(file);
			file = NULL;
			return NULL;
		}
		
		if (custommp4_write_data(file,&file->mvot,sizeof(file->mvot)) != 1)
		{
			custommp4_destroy(file);
			file = NULL;
			return NULL;
		}
		
		#endif
		
		file->p_mdat_sect_t = (mdat_segment_info_table_t *)malloc(sizeof(mdat_segment_info_table_t));
		custommp4_mdat_segment_info_table_init(file, file->p_mdat_sect_t);
		msot.obj.object_id = CUSTOMMP4_mdat_segment_object;
		msot.obj.object_size = sizeof(msot);
		msot.video_frames = 0;
		msot.audio_frames = 0;
		msot.reserved1 = 0;
		msot.start_time = file->p_mdat_sect_t->start_time;
		msot.end_time = file->p_mdat_sect_t->end_time;
		msot.video_allocated = file->p_mdat_sect_t->video_allocated;
		msot.audio_allocated = file->p_mdat_sect_t->audio_allocated;

		//yaogang modify for bad disk
		if (custommp4_write_data(file, &msot, sizeof(msot)) != 1)
		{
			custommp4_destroy(file);
			file = NULL;
			return NULL;
		}
		
		file->video_table_allocated = 3000;
		file->audio_table_allocated = 4000;
		
		file->p_video_table_t = (video_frame_info_table_t *)malloc(file->video_table_allocated * sizeof(video_frame_info_table_t));
		memset(file->p_video_table_t,0,file->video_table_allocated * sizeof(video_frame_info_table_t));
		file->p_audio_table_t = (audio_frame_info_table_t *)malloc(file->audio_table_allocated * sizeof(audio_frame_info_table_t));
		memset(file->p_audio_table_t,0,file->audio_table_allocated * sizeof(audio_frame_info_table_t));
	}
	
	return file;
}

int custommp4_close(custommp4_t *file)
{
	//unsigned char cur_atx_flag = tl_power_atx_check();
	//yaogang modify for bad disk
	//if(file && file->open_mode != O_R && file->error_flag == 0 )//&& cur_atx_flag)
	if(file && file->open_mode != O_R && !file->error_flag && !file->write_error_flog)
	{
		if(file->p_mdat_sect_t->video_frames || file->p_mdat_sect_t->audio_frames)
		{
			u32 last_pos;
			mdat_segment_object_t msot;
			
			last_pos = custommp4_position(file);
			
			custommp4_set_position(file,file->p_mdat_sect_t->offset+sizeof(mdat_segment_object_t));
			custommp4_write_data(file, file->p_video_table_t, sizeof(video_frame_info_table_t) * file->p_mdat_sect_t->video_frames);
			
			custommp4_set_position(file,file->p_mdat_sect_t->offset+sizeof(mdat_segment_object_t)+file->p_mdat_sect_t->video_allocated*sizeof(video_frame_info_table_t));
			custommp4_write_data(file, file->p_audio_table_t, sizeof(audio_frame_info_table_t) * file->p_mdat_sect_t->audio_frames);
			
			custommp4_set_position(file,file->p_mdat_sect_t->offset);
			msot.obj.object_id = CUSTOMMP4_mdat_segment_object;
			msot.obj.object_size = last_pos-file->p_mdat_sect_t->offset;
			msot.video_frames = file->p_mdat_sect_t->video_frames;
			msot.audio_frames = file->p_mdat_sect_t->audio_frames;
			msot.reserved1 = 0;
			msot.start_time = file->p_mdat_sect_t->start_time;
			msot.end_time = file->p_mdat_sect_t->end_time;
			msot.video_allocated = file->p_mdat_sect_t->video_allocated;
			msot.audio_allocated = file->p_mdat_sect_t->audio_allocated;
			custommp4_write_data(file, &msot, sizeof(msot));
			
			custommp4_set_position(file,file->open_offset+sizeof(file->fpot)+sizeof(file->vspot)+sizeof(file->aspot));
			file->mvot.number_of_segments++;
			custommp4_write_data(file,&file->mvot,sizeof(file->mvot));
			
			custommp4_set_position(file,file->open_offset);
			//file->fpot.end_time = time(NULL);
			file->fpot.file_Size = last_pos - file->open_offset;
			//printf("custommp4_close:file_Size=%d\n",file->fpot.file_Size);
			custommp4_write_data(file,&file->fpot,sizeof(file->fpot));
			
			fileflush(file->stream);//2007-09-25 csp add
		}
	}
	
	int result = custommp4_destroy(file);
	return result;//return 1;
}

int custommp4_set_video(custommp4_t *file,u32 time_scale,u16 width,u16 height,float frame_rate,u32 bit_rate,u32 compressor,u16 depth)
{
	u32 last_pos = custommp4_position(file);
	file->vspot.time_scale = time_scale;
	file->vspot.width = width;
	file->vspot.height = height;
	file->vspot.frame_rate = frame_rate;
	file->vspot.bit_rate = bit_rate;
	file->vspot.compressor = compressor;
	file->vspot.depth = depth;
	file->fpot.video_tracks = 1;
	custommp4_set_position(file,file->open_offset);
	
	//yaogang modify for bad disk
	//custommp4_write_data(file,&file->fpot,sizeof(file->fpot));
	if (custommp4_write_data(file,&file->fpot,sizeof(file->fpot)) != 1)
	{
		file->write_error_flog = 1;
		return -1;
	}
	
	//custommp4_write_data(file,&file->vspot,sizeof(file->vspot));
	if (custommp4_write_data(file,&file->vspot,sizeof(file->vspot)) != 1)
	{
		file->write_error_flog = 1;
		return -1;
	}
	
	custommp4_set_position(file,last_pos);
	if(file->p_mdat_sect_t->video_frames == 0 && file->p_mdat_sect_t->audio_frames == 0) file->p_mdat_sect_t->video_allocated = 3000;
	return 1;
}

int custommp4_set_audio(custommp4_t *file,u32 time_scale,u16 channels,u16 bits,u32 sample_rate,u32 compressor,u32 sample_size,u32 sample_duration)
{
	u32 last_pos = custommp4_position(file);
	file->aspot.time_scale = time_scale;
	file->aspot.channels = channels;
	file->aspot.sample_bits = bits;
	file->aspot.sample_rate = sample_rate;
	file->aspot.compressor = compressor;
	file->aspot.sample_size = sample_size;
	file->aspot.sample_duration = sample_duration;
	file->fpot.audio_tracks = 1;

	custommp4_set_position(file,file->open_offset);
	
	//yaogang modify for bad disk
	//custommp4_write_data(file,&file->fpot,sizeof(file->fpot));
	if (custommp4_write_data(file,&file->fpot,sizeof(file->fpot)) != 1)
	{
		file->write_error_flog = 1;
		return -1;
	}
	
	custommp4_set_position(file,file->open_offset+sizeof(file->fpot)+sizeof(file->vspot));

	//custommp4_write_data(file,&file->aspot,sizeof(file->aspot));
	if (custommp4_write_data(file,&file->aspot,sizeof(file->aspot)) != 1)
	{
		file->write_error_flog = 1;
		return -1;
	}

	custommp4_set_position(file,last_pos);
	if(file->p_mdat_sect_t->video_frames == 0 && file->p_mdat_sect_t->audio_frames == 0) file->p_mdat_sect_t->audio_allocated = 4000;
	return 1;
}

BOOL custommp4_has_video(custommp4_t *file)
{
	return (file->total_video_frames!=0);
	//return file->fpot.video_tracks;
}

BOOL custommp4_has_audio(custommp4_t *file)
{
	return (file->total_audio_frames!=0);
	//return file->fpot.audio_tracks;
}

int custommp4_total_time(custommp4_t *file)
{
	int total_time = 0;
	if(file->mvot.number_of_segments)
	{
		total_time = file->p_mdat_sect_t[file->mvot.number_of_segments-1].end_time-file->p_mdat_sect_t[0].start_time;
	}
	return total_time;
}

int custommp4_video_length(custommp4_t *file)
{
	return file->total_video_frames;
}

int custommp4_audio_length(custommp4_t *file)
{
	return file->total_audio_frames;
}

u16 custommp4_video_width(custommp4_t *file)
{
	return file->vspot.width;
}

u16 custommp4_video_height(custommp4_t *file)
{
	return file->vspot.height;
}

u16 custommp4_video_depth(custommp4_t *file)
{
	return file->vspot.depth;
}

float custommp4_video_frame_rate(custommp4_t *file)
{
	return file->vspot.frame_rate;
}

u32 custommp4_video_bit_rate(custommp4_t *file)
{
	return file->vspot.bit_rate;
}

u32 custommp4_video_compressor(custommp4_t *file)
{
	return file->vspot.compressor;
}

u32 custommp4_video_time_scale(custommp4_t *file)
{
	return file->vspot.time_scale;
}

u16 custommp4_audio_channels(custommp4_t *file)
{
	return file->aspot.channels;
}

u16 custommp4_audio_bits(custommp4_t *file)
{
	return file->aspot.sample_bits;
}

u32 custommp4_audio_sample_rate(custommp4_t *file)
{
	return file->aspot.sample_rate;
}

u32 custommp4_audio_sample_size(custommp4_t *file)
{
	if(file->total_audio_frames)
	{
		return file->aspot.sample_size;
	}
	return 0;
}

u32 custommp4_audio_compressor(custommp4_t *file)
{
	return file->aspot.compressor;
}

u32 custommp4_audio_time_scale(custommp4_t *file)
{
	return file->aspot.time_scale;
}

int custommp4_read_one_media_frame(custommp4_t *file,u8 *meida_buffer,u32 maxBytes,u32 *start_time,u8 *key,u64 *pts,u8 *media_type)
{
	int len = 0;
	//printf("chen 0 0\n");
	if(file->current_media_sect_pos >= file->mvot.number_of_segments)
	{
		return 0;
	}
	//printf("chen 0 1\n");
	if(file->current_video_offset >= file->p_mdat_sect_t[file->current_media_sect_pos].video_frames && file->current_audio_offset >= file->p_mdat_sect_t[file->current_media_sect_pos].audio_frames)
	{
		file->current_media_sect_pos++;
		if(file->current_media_sect_pos >= file->mvot.number_of_segments)
		{
			return 0;
		}
		//printf("next sect 1\n");
		if(ReadmdatIndexTable(file,file->current_media_sect_pos) <= 0)
		{
			printf("custommp4_read_one_media_frame:ReadmdatIndexTable error\n");
			return -1;
		}
		//printf("next sect 2\n");
	}
	//printf("chen 0 2\n");
	if(file->current_video_offset >= file->p_mdat_sect_t[file->current_media_sect_pos].video_frames && file->current_audio_offset >= file->p_mdat_sect_t[file->current_media_sect_pos].audio_frames)
	{
		return 0;
	}
	//printf("chen 0 3\n");
	if(file->current_video_offset >= file->p_mdat_sect_t[file->current_media_sect_pos].video_frames)
	{
		//audio
		if(file->p_audio_table_t[file->current_audio_offset].offset >= (u32)SIZE_OF_FILE_VECTOR)//yaogang hdd
		{
			//printf("warning:offset error 1,sample offset:%d,sample length:%d,buffer length:%d\n",file->p_audio_table_t[file->current_audio_offset].offset,file->aspot.sample_size,maxBytes);
			printf("warning:offset error 1,sample offset:%d,sample length:%d,buffer length:%d\n",file->p_audio_table_t[file->current_audio_offset].offset,file->p_audio_table_t[file->current_audio_offset].length,maxBytes);//wrchen 090115
			return -1;
		}
		//if(file->aspot.sample_size > maxBytes)
		if(file->p_audio_table_t[file->current_audio_offset].length > maxBytes)//wrchen 090115
		{
			//printf("warning:size error 1,sample offset:%d,sample length:%d,buffer length:%d\n",file->p_audio_table_t[file->current_audio_offset].offset,file->aspot.sample_size,maxBytes);
			printf("warning:size error 1,sample offset:%d,sample length:%d,buffer length:%d\n",file->p_audio_table_t[file->current_audio_offset].offset,file->p_audio_table_t[file->current_audio_offset].length,maxBytes);//wrchen 090115
			return -1;
		}
		SetReadDataFlag(3);
		custommp4_set_position(file,file->p_audio_table_t[file->current_audio_offset].offset);
		//len = custommp4_read_data(file, meida_buffer, file->aspot.sample_size);
		len = custommp4_read_data(file, meida_buffer, file->p_audio_table_t[file->current_audio_offset].length);//wrchen
		*media_type = 1;
		*key = 0;
		*start_time = file->p_audio_table_t[file->current_audio_offset].timestamp;
		//len = file->aspot.sample_size;
		file->current_audio_offset++;
	}
	else if(file->current_audio_offset >= file->p_mdat_sect_t[file->current_media_sect_pos].audio_frames)
	{
		//video
		if(file->p_video_table_t[file->current_video_offset].offset >= (u32)SIZE_OF_FILE_VECTOR)//yaogang hdd
		{
			printf("warning:offset error 1,frame offset:%d,frame length:%d,buffer length:%d\n",file->p_video_table_t[file->current_video_offset].offset,file->p_video_table_t[file->current_video_offset].length,maxBytes);
			return -1;
		}
		if(file->p_video_table_t[file->current_video_offset].length > maxBytes)
		{
			printf("warning:size error 1,frame offset:%d,frame length:%d,buffer length:%d\n",file->p_video_table_t[file->current_video_offset].offset,file->p_video_table_t[file->current_video_offset].length,maxBytes);
			return -1;
		}
		SetReadDataFlag(2);
		memset(meida_buffer,0xff,4);
		custommp4_set_position(file,file->p_video_table_t[file->current_video_offset].offset);
		//printf("size=%d,file->current_video_offset=%d\n",file->p_video_table_t[file->current_video_offset].length,file->current_video_offset);
		memset(meida_buffer,0,4);//add by csp
		len = custommp4_read_data(file, meida_buffer, file->p_video_table_t[file->current_video_offset].length);
		*media_type = 0;
		*key = file->p_video_table_t[file->current_video_offset].key;
		*pts = file->p_video_table_t[file->current_video_offset].pts;//wrchen 081226
		*start_time = file->p_video_table_t[file->current_video_offset].timestamp;
		//len = file->p_video_table_t[file->current_video_offset].length;
		file->current_video_offset++;
	}
	else
	{
		//printf("chen 0\n");
		if(file->total_audio_frames&&file->p_video_table_t[file->current_video_offset].timestamp > file->p_audio_table_t[file->current_audio_offset].timestamp)
		{
		//printf("chen 1\n");
			//audio
			if(file->p_audio_table_t[file->current_audio_offset].offset >= (u32)SIZE_OF_FILE_VECTOR)//yaogang hdd
			{
		//printf("chen 1 1\n");
				//printf("warning:offset error 2,sample offset:%d,sample length:%d,buffer length:%d\n",file->p_audio_table_t[file->current_audio_offset].offset,file->aspot.sample_size,maxBytes);
				printf("warning:offset error 2,sample offset:%d,sample length:%d,buffer length:%d\n",file->p_audio_table_t[file->current_audio_offset].offset,file->p_audio_table_t[file->current_audio_offset].length,maxBytes);//wrchen 090115
				return -1;
			}
			if(file->p_audio_table_t[file->current_audio_offset].length > maxBytes)
			{
		//printf("chen 1 2\n");
				//printf("warning:size error 2,sample offset:%d,sample length:%d,buffer length:%d\n",file->p_audio_table_t[file->current_audio_offset].offset,file->aspot.sample_size,maxBytes);
				printf("warning:size error 2,sample offset:%d,sample length:%d,buffer length:%d\n",file->p_audio_table_t[file->current_audio_offset].offset,file->p_audio_table_t[file->current_audio_offset].length,maxBytes);//wrchen 090115
				return -1;
			}
			SetReadDataFlag(3);
			custommp4_set_position(file,file->p_audio_table_t[file->current_audio_offset].offset);
			//len = custommp4_read_data(file, meida_buffer, file->aspot.sample_size);
			if(0==file->p_audio_table_t[file->current_audio_offset].length)
			{
				printf("write length=%u\n",file->p_audio_table_t[file->current_audio_offset].length);
			}
			len = custommp4_read_data(file, meida_buffer, file->p_audio_table_t[file->current_audio_offset].length);//wrchen 090115
			if(len!=file->p_audio_table_t[file->current_audio_offset].length)
			{
				printf("ret len=%d,write length=%u\n",len,file->p_audio_table_t[file->current_audio_offset].length);
			}
			*media_type = 1;
			*key = 0;
			*start_time = file->p_audio_table_t[file->current_audio_offset].timestamp;
			//len = file->aspot.sample_size;
			file->current_audio_offset++;
		}
		else
		{
		//printf("chen 2\n");
			//video
			if(file->p_video_table_t[file->current_video_offset].offset >= (u32)SIZE_OF_FILE_VECTOR)//yaogang hdd
			{
		//printf("chen 2 1\n");
				printf("warning:offset error 2,frame offset:%d,frame length:%d,buffer length:%d\n",file->p_video_table_t[file->current_video_offset].offset,file->p_video_table_t[file->current_video_offset].length,maxBytes);
				return -1;
			}
			if(file->p_video_table_t[file->current_video_offset].length > maxBytes)
			{
		//printf("chen 2 2\n");
				printf("warning:size error 2,frame offset:%d,frame length:%d,buffer length:%d\n",file->p_video_table_t[file->current_video_offset].offset,file->p_video_table_t[file->current_video_offset].length,maxBytes);
				return -1;
			}
			SetReadDataFlag(2);
			memset(meida_buffer,0xff,4);
			custommp4_set_position(file,file->p_video_table_t[file->current_video_offset].offset);
			memset(meida_buffer,0,4);//add by csp
			len = custommp4_read_data(file, meida_buffer, file->p_video_table_t[file->current_video_offset].length);
			*media_type = 0;
			*key = file->p_video_table_t[file->current_video_offset].key;
			*pts= file->p_video_table_t[file->current_video_offset].pts;//wrchen 081226
			*start_time = file->p_video_table_t[file->current_video_offset].timestamp;
			//len = file->p_video_table_t[file->current_video_offset].length;
			file->current_video_offset++;
		}
	}
	return len;
}

//组合sps pps I帧版本
int custommp4_read_one_media_frame2(custommp4_t *file,u8 *meida_buffer,u32 maxBytes,u32 *pstart_time,u8 *pkey,u64 *pts,u8 *pmedia_type, u8 *pIframe_cnt)
{
	unsigned int reallen;
	int len;
	unsigned int start_time;
	u8 key;
	u8 media_type;
	u8 flag = 0;//*(meida_buffer+reallen+4) & 0x1f) == 0x07 开始*(meida_buffer+reallen+4) & 0x1f) == 0x05 结束组合I帧
	u8 cnt = 0;
	u8 frame_type;
	
	*pIframe_cnt = 1;//默认有1帧
	if (!file || !meida_buffer || !pstart_time || !pkey || !pmedia_type)
	{
		return 0;
	}

	*pIframe_cnt = 1;//默认有1帧
	
	reallen = len = 0;
	do
	{
		len = custommp4_read_one_media_frame(file, meida_buffer+reallen, maxBytes-reallen, &start_time, &key, pts, &media_type);
		//printf("1 len: %d, start_time: %u, key: %u, media_type: %u\n", len, start_time, key, media_type);
		cnt++;
		
		if (len <= 0)
		{
			reallen = len;
			break;
		}			

		if (flag)//组合开始后到结束之间读到音频帧丢弃
		{
			if (0 != media_type) //audio
			{
				//printf("audio continue\n");
				continue;
			}
		}
		else //否则返回音频帧
		{
			if (0 != media_type) //audio
			{
				*pstart_time = start_time;
				*pkey = key;
				//*ppts = pts;
				*pmedia_type = media_type;
				reallen = len;
				//printf("return audio\n");
				break;
			}
		}

		frame_type = *(meida_buffer+reallen+4) & 0x1f;
		//printf("key frame: 0x%x\n", frame_type);
		
		if (frame_type == 0x7)//SPS
		{
			*pstart_time = start_time;
			*pkey = 1;
			//*ppts = pts;
			*pmedia_type = media_type;
			if (len < 50)//sps帧本身很小，如果IPC传过来的I帧已经是组合了的，那么此时就不用组合了
			{
				//printf("combination_Iframe start\n");
				flag = 1;
			}
			else
			{
				//printf("is combination_Iframe\n");
				*pIframe_cnt = 1;//读取的帧已经是组合了的
			}
			
			cnt = 0;
		}
		else
		{
			if (flag)//组合开始
			{
				if ((*(meida_buffer+reallen+4) & 0x1f) == 0x5)//I帧数据
				{
					flag = 0;
					*pIframe_cnt = cnt+1;
					//printf("combination_Iframe end Iframe_cnt = %d\n", *pIframe_cnt);
				}
				else if (frame_type != 0x8/*PPS*/ && frame_type != 0x6/*IDR*/)
				{
					//出错
					//printf("combination Iframe failed\n");
					return -1;
				}
				
			}
			else //P帧
			{
				*pstart_time = start_time;
				*pkey = key;
				//*ppts = pts;
				*pmedia_type = media_type;
				*pIframe_cnt = 1;
				reallen = len;
				break;
			}
		}
		
		reallen += len;
	}while (flag);

	return reallen;
}


#if 0
int custommp4_read_one_media_h264v_frame(custommp4_t *file,u8 *meida_buffer,u32 maxBytes,u32 *start_time,u8 *key,u8 *media_type,u16 *nal_size)
{
	int len = 0;
	if(file->current_media_sect_pos >= file->mvot.number_of_segments)
	{
		return 0;
	}
	if(file->current_video_offset >= file->p_mdat_sect_t[file->current_media_sect_pos].video_frames && file->current_audio_offset >= file->p_mdat_sect_t[file->current_media_sect_pos].audio_frames)
	{
		file->current_media_sect_pos++;
		if(file->current_media_sect_pos >= file->mvot.number_of_segments)
		{
			return 0;
		}
		//printf("next sect 1\n");
		if(ReadmdatIndexTable(file,file->current_media_sect_pos) <= 0)
		{
			printf("custommp4_read_one_media_frame:ReadmdatIndexTable error\n");
			return -1;
		}
		//printf("next sect 2\n");
	}
	if(file->current_video_offset >= file->p_mdat_sect_t[file->current_media_sect_pos].video_frames && file->current_audio_offset >= file->p_mdat_sect_t[file->current_media_sect_pos].audio_frames)
	{
		return 0;
	}
	if(file->current_video_offset >= file->p_mdat_sect_t[file->current_media_sect_pos].video_frames)
	{
		//audio
		if(file->p_audio_table_t[file->current_audio_offset].offset >= (u32)SIZE_OF_FILE_VECTOR)//yaogang hdd
		{
			printf("warning:offset error 1,sample offset:%d,sample length:%d,buffer length:%d\n",file->p_audio_table_t[file->current_audio_offset].offset,file->aspot.sample_size,maxBytes);
			return -1;
		}
		if(file->aspot.sample_size > maxBytes)
		{
			printf("warning:size error 1,sample offset:%d,sample length:%d,buffer length:%d\n",file->p_audio_table_t[file->current_audio_offset].offset,file->aspot.sample_size,maxBytes);
			return -1;
		}
		SetReadDataFlag(3);
		custommp4_set_position(file,file->p_audio_table_t[file->current_audio_offset].offset);
		len = custommp4_read_data(file, meida_buffer, file->aspot.sample_size);
		*media_type = 1;
		*key = 0;
		*start_time = file->p_audio_table_t[file->current_audio_offset].timestamp;
		//len = file->aspot.sample_size;
		file->current_audio_offset++;
	}
	else if(file->current_audio_offset >= file->p_mdat_sect_t[file->current_media_sect_pos].audio_frames)
	{
		//video
		if(file->p_video_table_t[file->current_video_offset].offset >= (u32)SIZE_OF_FILE_VECTOR)//yaogang hdd
		{
			printf("warning:offset error 1,frame offset:%d,frame length:%d,buffer length:%d\n",file->p_video_table_t[file->current_video_offset].offset,file->p_video_table_t[file->current_video_offset].length,maxBytes);
			return -1;
		}
		if(file->p_video_table_t[file->current_video_offset].length > maxBytes)
		{
			printf("warning:size error 1,frame offset:%d,frame length:%d,buffer length:%d\n",file->p_video_table_t[file->current_video_offset].offset,file->p_video_table_t[file->current_video_offset].length,maxBytes);
			return -1;
		}
		SetReadDataFlag(2);
		memset(meida_buffer,0xff,4);
		custommp4_set_position(file,file->p_video_table_t[file->current_video_offset].offset);
		len = custommp4_read_data(file, meida_buffer, file->p_video_table_t[file->current_video_offset].length);
		*media_type = 0;
		*key = file->p_video_table_t[file->current_video_offset].key;
		*start_time = file->p_video_table_t[file->current_video_offset].timestamp;
		*nal_size = file->p_video_table_t[file->current_video_offset].reserved2;
		//len = file->p_video_table_t[file->current_video_offset].length;
		file->current_video_offset++;
		/*if(len > 0)
		{
			if(*key == 0)
			{
				if(!(meida_buffer[0] == 0x00 && 
					meida_buffer[1] == 0x00 && 
					meida_buffer[2] == 0x01 && 
					meida_buffer[3] == 0xb6))
				{
					printf("P frame error 1\n");
					return -1;
				}
			}
			else if(*key == 1)
			{
				if(!(meida_buffer[0] == 0x00 && 
					meida_buffer[1] == 0x00 && 
					meida_buffer[2] == 0x01 && 
					meida_buffer[3] == 0xb0))
				{
					printf("I frame error 1\n");
					return -1;
				}
			}
			else
			{
				printf("video frame type error 1\n");
				return -1;
			}
		}*/
	}
	else
	{
		if(file->p_video_table_t[file->current_video_offset].timestamp > file->p_audio_table_t[file->current_audio_offset].timestamp)
		{
			//audio
			if(file->p_audio_table_t[file->current_audio_offset].offset >= (u32)SIZE_OF_FILE_VECTOR)//yaogang hdd
			{
				printf("warning:offset error 2,sample offset:%d,sample length:%d,buffer length:%d\n",file->p_audio_table_t[file->current_audio_offset].offset,file->aspot.sample_size,maxBytes);
				return -1;
			}
			if(file->aspot.sample_size > maxBytes)
			{
				printf("warning:size error 2,sample offset:%d,sample length:%d,buffer length:%d\n",file->p_audio_table_t[file->current_audio_offset].offset,file->aspot.sample_size,maxBytes);
				return -1;
			}
			SetReadDataFlag(3);
			custommp4_set_position(file,file->p_audio_table_t[file->current_audio_offset].offset);
			len = custommp4_read_data(file, meida_buffer, file->aspot.sample_size);
			*media_type = 1;
			*key = 0;
			*start_time = file->p_audio_table_t[file->current_audio_offset].timestamp;
			//len = file->aspot.sample_size;
			file->current_audio_offset++;
		}
		else
		{
			//video
			if(file->p_video_table_t[file->current_video_offset].offset >= (u32)SIZE_OF_FILE_VECTOR)//yaogang hdd
			{
				printf("warning:offset error 2,frame offset:%d,frame length:%d,buffer length:%d\n",file->p_video_table_t[file->current_video_offset].offset,file->p_video_table_t[file->current_video_offset].length,maxBytes);
				return -1;
			}
			if(file->p_video_table_t[file->current_video_offset].length > maxBytes)
			{
				printf("warning:size error 2,frame offset:%d,frame length:%d,buffer length:%d\n",file->p_video_table_t[file->current_video_offset].offset,file->p_video_table_t[file->current_video_offset].length,maxBytes);
				return -1;
			}
			SetReadDataFlag(2);
			memset(meida_buffer,0xff,4);
			custommp4_set_position(file,file->p_video_table_t[file->current_video_offset].offset);
			len = custommp4_read_data(file, meida_buffer, file->p_video_table_t[file->current_video_offset].length);
			*media_type = 0;
			*key = file->p_video_table_t[file->current_video_offset].key;
			*start_time = file->p_video_table_t[file->current_video_offset].timestamp;
			*nal_size = file->p_video_table_t[file->current_video_offset].reserved2;
			//len = file->p_video_table_t[file->current_video_offset].length;
			file->current_video_offset++;
			/*if(len > 0)
			{
				if(*key == 0)
				{
					if(!(meida_buffer[0] == 0x00 && 
						meida_buffer[1] == 0x00 && 
						meida_buffer[2] == 0x01 && 
						meida_buffer[3] == 0xb6))
					{
						printf("P frame error 2\n");
						return -1;
					}
				}
				else if(*key == 1)
				{
					if(!(meida_buffer[0] == 0x00 && 
						meida_buffer[1] == 0x00 && 
						meida_buffer[2] == 0x01 && 
						meida_buffer[3] == 0xb0))
					{
						printf("I frame error 2\n");
						return -1;
					}
				}
				else
				{
					printf("video frame type error 2\n");
					return -1;
				}	
			}*/
		}
	}
	return len;
}
#endif

int custommp4_video_frame_size(custommp4_t *file,int frame)
{
	u32 sect_offset,sect;
	int len = 0;
	if(custommp4_get_segment_of_video_frame(&sect_offset,&sect,file,frame))
	{
		lock_custommp4(file);

		if(file->current_video_sect_pos != sect)
		{
			ReadmdatVideoIndexTable(file,sect);
		}
		file->current_video_frame_pos = frame;

		len = file->p_video_table_t[sect_offset].length;

		unlock_custommp4(file);
	}
	return len;
}

int custommp4_audio_frame_size(custommp4_t *file,int frame)
{
	u32 sect_offset,sect;
	int len = 0;
	if(custommp4_get_segment_of_audio_frame(&sect_offset,&sect,file,frame))
	{
		lock_custommp4(file);

		if(file->current_audio_sect_pos != sect)
		{
			ReadmdatAudioIndexTable(file,sect);
		}
		file->current_audio_frame_pos = frame;

		len = file->aspot.sample_size;

		unlock_custommp4(file);
	}
	return len;
}

int custommp4_read_video_frame(custommp4_t *file,u8 *video_buffer,u32 maxBytes,int frame,u32 *start_time,u8 *key)
{
	u32 sect_offset,sect;
	int len = 0;
	if(custommp4_get_segment_of_video_frame(&sect_offset,&sect,file,frame))
	{
		lock_custommp4(file);

		//printf("frame=%d,sect=%d,offset=%d,videos=%d,audios=%d\n",frame,sect,sect_offset,file->p_mdat_sect_t[sect].video_frames,file->p_mdat_sect_t[sect].audio_frames);
		
		if(file->current_video_sect_pos != sect)
		{
			ReadmdatVideoIndexTable(file,sect);
		}
		file->current_video_frame_pos = frame;
		custommp4_set_position(file, file->p_video_table_t[sect_offset].offset);
		custommp4_read_data(file, video_buffer, file->p_video_table_t[sect_offset].length);
		
		//printf("offset=%d,len=%d,key=%d,sizeof(video_frame_info_table_t)=%d\n",file->p_video_table_t[sect_offset].offset,file->p_video_table_t[sect_offset].length,file->p_video_table_t[sect_offset].key,sizeof(video_frame_info_table_t));
		
		*start_time = file->p_video_table_t[sect_offset].timestamp;
		*key = file->p_video_table_t[sect_offset].key;
		len = file->p_video_table_t[sect_offset].length;

		unlock_custommp4(file);
	}
	return len;
}

/*int custommp4_read_h264_frame(custommp4_t *file,u8 *video_buffer,u32 maxBytes,int frame,u32 *start_time,u8 *key,u16 *nal_size)
{
	u32 sect_offset,sect;
	int len = 0;
	if(custommp4_get_segment_of_video_frame(&sect_offset,&sect,file,frame))
	{
		lock_custommp4(file);
		
		//printf("frame=%d,sect=%d,offset=%d,videos=%d,audios=%d\n",frame,sect,sect_offset,file->p_mdat_sect_t[sect].video_frames,file->p_mdat_sect_t[sect].audio_frames);
		
		if(file->current_video_sect_pos != sect)
		{
			ReadmdatVideoIndexTable(file,sect);
		}
		file->current_video_frame_pos = frame;
		custommp4_set_position(file,file->p_video_table_t[sect_offset].offset);
		custommp4_read_data(file, video_buffer, file->p_video_table_t[sect_offset].length);
		
		//printf("offset=%d,len=%d,key=%d,sizeof(video_frame_info_table_t)=%d\n",file->p_video_table_t[sect_offset].offset,file->p_video_table_t[sect_offset].length,file->p_video_table_t[sect_offset].key,sizeof(video_frame_info_table_t));
		
		*start_time = file->p_video_table_t[sect_offset].timestamp;
		*key = file->p_video_table_t[sect_offset].key;
		len = file->p_video_table_t[sect_offset].length;
		
		*nal_size = file->p_video_table_t[sect_offset].reserved2;		
		
		unlock_custommp4(file);
	}
	return len;
}*/

int custommp4_read_audio_frame(custommp4_t *file,u8 *audio_buffer,u32 maxBytes,int frame,u32 *start_time)
{
	u32 sect_offset,sect;
	int len = 0;
	if(custommp4_get_segment_of_audio_frame(&sect_offset,&sect,file,frame))
	{
		lock_custommp4(file);
		
		if(file->current_audio_sect_pos != sect)
		{
			ReadmdatAudioIndexTable(file,sect);
		}
		file->current_audio_frame_pos = frame;
		custommp4_set_position(file,file->p_audio_table_t[sect_offset].offset);
		//printf("read audio:frame%d timestamp=%d,offset=%d\n",frame,file->p_audio_table_t[sect_offset].timestamp,file->p_audio_table_t[sect_offset].offset);
		custommp4_read_data(file, audio_buffer, file->aspot.sample_size);
		
		*start_time = file->p_audio_table_t[sect_offset].timestamp;
		len = file->aspot.sample_size;

		unlock_custommp4(file);
	}
	return len;
}

int custommp4_read_one_video_frame(custommp4_t *file,u8 *video_buffer,u32 maxBytes,u32 *start_time,u32 *duration,u8 *key)
{
	u32 sect_offset,sect;
	int frame;
	int len = 0;

	lock_custommp4(file);

	frame = file->current_video_frame_pos;
	if((u32)frame >= file->total_video_frames)
	{
		unlock_custommp4(file);
		return 0;
	}
	if(custommp4_get_segment_of_video_frame(&sect_offset,&sect,file,frame))
	{
		//printf("frame=%d,sect=%d,offset=%d,videos=%d,audios=%d\n",frame,sect,sect_offset,file->p_mdat_sect_t[sect].video_frames,file->p_mdat_sect_t[sect].audio_frames);
		
		if(file->current_video_sect_pos != sect)
		{
			ReadmdatVideoIndexTable(file,sect);
		}
		custommp4_set_position(file, file->p_video_table_t[sect_offset].offset);
		custommp4_read_data(file, video_buffer, file->p_video_table_t[sect_offset].length);
		
		//printf("offset=%d,len=%d,key=%d,sizeof(video_frame_info_table_t)=%d\n",file->p_video_table_t[sect_offset].offset,file->p_video_table_t[sect_offset].length,file->p_video_table_t[sect_offset].key,sizeof(video_frame_info_table_t));
		
		*start_time = file->p_video_table_t[sect_offset].timestamp;
		*key = file->p_video_table_t[sect_offset].key;
		len = file->p_video_table_t[sect_offset].length;

		if(sect_offset < file->p_mdat_sect_t[sect].video_frames-1)
		{
			*duration = file->p_video_table_t[sect_offset+1].timestamp - *start_time;
			++frame;
		}
		else
		{
			if((u32)frame == file->total_video_frames-1)
			{
				*duration = 40;
				//frame = 0;
			}
			else
			{
				++frame;
				custommp4_get_segment_of_video_frame(&sect_offset,&sect,file,frame);
				ReadmdatVideoIndexTable(file,sect);
				*duration = file->p_video_table_t[0].timestamp - *start_time;
			}
		}
		file->current_video_frame_pos = frame;
	}

	unlock_custommp4(file);

	return len;
}

/*int custommp4_read_one_h264_frame(custommp4_t *file,u8 *video_buffer,u32 maxBytes,u32 *start_time,u32 *duration,u8 *key,u16 *nal_size)
{
	u32 sect_offset,sect;
	int frame;
	int len = 0;
	
	lock_custommp4(file);
	
	frame = file->current_video_frame_pos;
	if((u32)frame >= file->total_video_frames)
	{
		unlock_custommp4(file);
		return 0;
	}
	if(custommp4_get_segment_of_video_frame(&sect_offset,&sect,file,frame))
	{
		//printf("frame=%d,sect=%d,offset=%d,videos=%d,audios=%d\n",frame,sect,sect_offset,file->p_mdat_sect_t[sect].video_frames,file->p_mdat_sect_t[sect].audio_frames);
		
		if(file->current_video_sect_pos != sect)
		{
			ReadmdatVideoIndexTable(file,sect);
		}
		custommp4_set_position(file,file->p_video_table_t[sect_offset].offset);
		custommp4_read_data(file, video_buffer, file->p_video_table_t[sect_offset].length);
		
		//printf("offset=%d,len=%d,key=%d,sizeof(video_frame_info_table_t)=%d\n",file->p_video_table_t[sect_offset].offset,file->p_video_table_t[sect_offset].length,file->p_video_table_t[sect_offset].key,sizeof(video_frame_info_table_t));
		
		*start_time = file->p_video_table_t[sect_offset].timestamp;
		*key = file->p_video_table_t[sect_offset].key;
		len = file->p_video_table_t[sect_offset].length;
		
		*nal_size = file->p_video_table_t[sect_offset].reserved2;
		
		if(sect_offset < file->p_mdat_sect_t[sect].video_frames-1)
		{
			*duration = file->p_video_table_t[sect_offset+1].timestamp - *start_time;
			++frame;
		}
		else
		{
			if((u32)frame == file->total_video_frames-1)
			{
				*duration = 40;
				//frame = 0;
			}
			else
			{
				++frame;
				custommp4_get_segment_of_video_frame(&sect_offset,&sect,file,frame);
				ReadmdatVideoIndexTable(file,sect);
				*duration = file->p_video_table_t[0].timestamp - *start_time;
			}
		}
		file->current_video_frame_pos = frame;
	}
	
	unlock_custommp4(file);
	
	return len;
}*/

int custommp4_read_one_audio_frame(custommp4_t *file,u8 *audio_buffer,u32 maxBytes,u32 *start_time,u32 *duration)
{
	u32 sect_offset,sect;
	int frame;
	int len = 0;

	lock_custommp4(file);

	frame = file->current_audio_frame_pos;
	if((u32)frame >= file->total_audio_frames)
	{
		unlock_custommp4(file);
		return 0;
	}
	if(custommp4_get_segment_of_audio_frame(&sect_offset,&sect,file,frame))
	{
		if(file->current_audio_sect_pos != sect)
		{
			ReadmdatAudioIndexTable(file,sect);
		}
		file->current_audio_frame_pos = frame;
		custommp4_set_position(file,file->p_audio_table_t[sect_offset].offset);
		custommp4_read_data(file, audio_buffer, file->aspot.sample_size);
		
		*start_time = file->p_audio_table_t[sect_offset].timestamp;
		len = file->aspot.sample_size;

		if(sect_offset < file->p_mdat_sect_t[sect].audio_frames-1)
		{
			*duration = file->p_audio_table_t[sect_offset+1].timestamp - *start_time;
			++frame;
		}
		else
		{
			if((u32)frame == file->total_audio_frames-1)
			{
				*duration = 30;
				//frame = 0;
			}
			else
			{
				++frame;
				custommp4_get_segment_of_audio_frame(&sect_offset,&sect,file,frame);
				ReadmdatAudioIndexTable(file,sect);
				*duration = file->p_audio_table_t[0].timestamp - *start_time;
			}
		}
		file->current_audio_frame_pos = frame;
	}

	unlock_custommp4(file);

	return len;
}

int custommp4_seek_to_prev_key_frame(custommp4_t *file)
{
	int i,j;
	int cur_video_pos;
	u32 start_time;
	if(file->mvot.number_of_segments == 0)
	{
		//printf("failed 1\n");
		return 0;
	}
	if(file->current_media_sect_pos > file->mvot.number_of_segments)
	{
		file->current_media_sect_pos = file->mvot.number_of_segments;
	}
	if(file->current_media_sect_pos < file->mvot.number_of_segments)
	{
		//xdc modify 0715
		//cur_video_pos = file->current_video_offset-2;
		cur_video_pos = file->current_video_offset-5;
		
		//printf("cur_video_pos=%d\n",cur_video_pos);
		
		if(cur_video_pos >= (int)file->p_mdat_sect_t[file->current_media_sect_pos].video_frames)
		{
			cur_video_pos = file->p_mdat_sect_t[file->current_media_sect_pos].video_frames-1;
		}
		for(i=cur_video_pos;i>=0;i--)
		{
			if(file->p_video_table_t[i].key > 0)//if(file->p_video_table_t[i].key == 1)//zlb20091010
			{
				file->current_video_offset = i;
				start_time = file->p_video_table_t[i].timestamp;
				//printf("seek_video_pos=%d\n",i);
				break;
			}
		}
		if(i>=0)
		{
			for(j=0;j<(int)file->p_mdat_sect_t[file->current_media_sect_pos].audio_frames;j++)
			{
				if(file->p_audio_table_t[j].timestamp > start_time)
				{
					break;
				}
			}
			file->current_audio_offset = j;
			return 1;
		}
	}
	//没找到，切换上一个段
	while((int)file->current_media_sect_pos > 0)
	{
		ReadmdatIndexTable(file,file->current_media_sect_pos-1);//上一个段
		//printf("last segment: %d\n", file->current_media_sect_pos);
		cur_video_pos = file->p_mdat_sect_t[file->current_media_sect_pos].video_frames-1;
		//yg modify 20140807
		while(1)
		{
			for(i=cur_video_pos;i>=0;i--)
			{
				if(file->p_video_table_t[i].key > 0)//if(file->p_video_table_t[i].key == 1)//zlb20091010
				{
					file->current_video_offset = i;
					start_time = file->p_video_table_t[i].timestamp;
					//printf("yg i: %d\n", i);
					break;
				}
			}
			//防止关键帧的三个帧分散在相邻的两个段内
			if(file->current_video_offset+4 < file->p_mdat_sect_t[file->current_media_sect_pos].video_frames)
			{
				break;
			}
			else
			{
				cur_video_pos = file->current_video_offset-1;
			}
		}	
		//yg modify 20140807 end
		if(i>=0)
		{
			for(j=0;j<(int)file->p_mdat_sect_t[file->current_media_sect_pos].audio_frames;j++)
			{
				if(file->p_audio_table_t[j].timestamp > start_time)
				{
					break;
				}
			}
			file->current_audio_offset = j;
			return 1;
		}
	}
	//printf("failed 2\n");
	return 0;
}

//csp modify 20130429
int custommp4_seek_to_next_key_frame(custommp4_t *file)
{
	int i,j;
	int cur_video_pos;
	u32 start_time;
	if(file->mvot.number_of_segments == 0)
	{
		//printf("failed 1\n");
		return 0;
	}
	if(file->current_media_sect_pos >= file->mvot.number_of_segments)
	{
		file->current_media_sect_pos = file->mvot.number_of_segments;
		return 0;
	}
	cur_video_pos = file->current_video_offset;
	for(i=cur_video_pos; i<file->p_mdat_sect_t[file->current_media_sect_pos].video_frames; i++)
	{
		if(file->p_video_table_t[i].key > 0)
		{
			file->current_video_offset = i;
			start_time = file->p_video_table_t[i].timestamp;
			//printf("seek_video_pos=%d\n",i);
			break;
		}
	}
	if(i<file->p_mdat_sect_t[file->current_media_sect_pos].video_frames)
	{
		for(j=0;j<(int)file->p_mdat_sect_t[file->current_media_sect_pos].audio_frames;j++)
		{
			if(file->p_audio_table_t[j].timestamp > start_time)
			{
				break;
			}
		}
		file->current_audio_offset = j;
		return 1;
	}
	
	while((int)(file->current_media_sect_pos+1) < file->mvot.number_of_segments)
	{
		ReadmdatIndexTable(file,file->current_media_sect_pos+1);
		//printf("next segment\n");
		cur_video_pos = 0;
		for(i=cur_video_pos; i<file->p_mdat_sect_t[file->current_media_sect_pos].video_frames; i++)
		{
			if(file->p_video_table_t[i].key > 0)
			{
				file->current_video_offset = i;
				start_time = file->p_video_table_t[i].timestamp;
				break;
			}
		}
		if(i<file->p_mdat_sect_t[file->current_media_sect_pos].video_frames)
		{
			for(j=0;j<(int)file->p_mdat_sect_t[file->current_media_sect_pos].audio_frames;j++)
			{
				if(file->p_audio_table_t[j].timestamp > start_time)
				{
					break;
				}
			}
			file->current_audio_offset = j;
			return 1;
		}
	}
	//printf("failed 2\n");
	//file->current_media_sect_pos = file->mvot.number_of_segments;
	return 0;
}

int custommp4_seek_to_prev_segment(custommp4_t *file)
{
	u32 i,j;
	u32 start_time;
	if(file->mvot.number_of_segments == 0)
	{
		return 0;
	}
	if(file->current_media_sect_pos > file->mvot.number_of_segments)
	{
		file->current_media_sect_pos = file->mvot.number_of_segments;
	}
	while((int)file->current_media_sect_pos > 0)
	{	
		ReadmdatIndexTable(file,file->current_media_sect_pos-1);
		for(i=0;i<file->p_mdat_sect_t[file->current_media_sect_pos].video_frames;i++)
		{
			if(file->p_video_table_t[i].key == 1)
			{
				file->current_video_offset = i;
				start_time = file->p_video_table_t[i].timestamp;
				break;
			}
		}
		if(i<file->p_mdat_sect_t[file->current_media_sect_pos].video_frames)
		{
			for(j=0;j<file->p_mdat_sect_t[file->current_media_sect_pos].audio_frames;j++)
			{
				if(file->p_audio_table_t[j].timestamp > start_time)
				{
					break;
				}
			}
			file->current_audio_offset = j;
			return 1;
		}
	}
	return 0;
}

int custommp4_seek_to_next_segment(custommp4_t *file)
{
	u32 i,j;
	u32 start_time;
	while(file->current_media_sect_pos+1 < file->mvot.number_of_segments)
	{
		ReadmdatIndexTable(file,file->current_media_sect_pos+1);
		for(i=0;i<file->p_mdat_sect_t[file->current_media_sect_pos].video_frames;i++)
		{
			if(file->p_video_table_t[i].key == 1)
			{
				file->current_video_offset = i;
				start_time = file->p_video_table_t[i].timestamp;
				break;
			}
		}
		if(i<file->p_mdat_sect_t[file->current_media_sect_pos].video_frames)
		{
			for(j=0;j<file->p_mdat_sect_t[file->current_media_sect_pos].audio_frames;j++)
			{
				if(file->p_audio_table_t[j].timestamp > start_time)
				{
					break;
				}
			}
			file->current_audio_offset = j;
			return 1;
		}
	}
	return 0;
}

int custommp4_seek_to_time_stamp(custommp4_t *file, u32 timestamp)
{
	u32 i,j,k;
	u32 start_time;
	
	DBG("seek_to_time_stamp 1\n");
	
	lock_custommp4(file);//2007-11-18
	
	for(i=0;i<file->mvot.number_of_segments;i++)
	{
		if(timestamp > file->p_mdat_sect_t[i].end_time) continue;
		
		/* 2007-11-18 */
		#ifdef WIN32
		file->current_video_sect_pos = (u32)(-1);
		file->current_audio_sect_pos = (u32)(-1);
		#endif
		/* 2007-11-18 */
		
		#ifndef WIN32//2007-11-18
		if(i != file->current_media_sect_pos)
		#endif//2007-11-18
		{
			if(ReadmdatIndexTable(file,i) <= 0)
			{
				unlock_custommp4(file);//2007-11-18
				
				printf("custommp4_seek_to_time_stamp:ReadmdatIndexTable error\n");
				//return -1;//被覆盖的文件尽早结束放像
				
				return -1;//2007-11-18
			}
		}
		
		for(j=0;j<file->p_mdat_sect_t[file->current_media_sect_pos].video_frames;j++)
		{
			//printf("[j:%d, timestamp:%d, pts:%llu, len:%d]\n",j, file->p_video_table_t[j].timestamp, file->p_video_table_t[j].pts, file->p_video_table_t[j].length);
			if(file->p_video_table_t[j].key > 0 && file->p_video_table_t[j].timestamp/*+1500*/ >= timestamp)//2007-11-18
			//if(file->p_video_table_t[j].timestamp/*+1500*/ >= timestamp)//2007-11-18
			{
				file->current_video_offset = j;
				start_time = file->p_video_table_t[j].timestamp;
				break;
			}
		}
		//printf("[j = %d, video_frames = %d], [j'timestamp:%d, timestamp:%d], pts:%llu\n", j, file->p_mdat_sect_t[file->current_media_sect_pos].video_frames, file->p_video_table_t[j].timestamp, timestamp,file->p_video_table_t[j].pts);
		if(j<file->p_mdat_sect_t[file->current_media_sect_pos].video_frames)
		{
			for(k=0;k<file->p_mdat_sect_t[file->current_media_sect_pos].audio_frames;k++)
			{
				if(file->p_audio_table_t[k].timestamp > start_time)
				{
					break;
				}
			}
			file->current_audio_offset = k;
			
			DBG("seek_to_time_stamp 2\n");
			
			/* 2007-11-18 */
			#ifdef WIN32
			{
				u32 v_frames = 0;
				u32 a_frames = 0;
				unsigned int m;
				for(m=0;m<i;m++)
				{
					v_frames += file->p_mdat_sect_t[m].video_frames;
					a_frames += file->p_mdat_sect_t[m].audio_frames;
				}
				v_frames += file->current_video_offset;
				a_frames += file->current_audio_offset;
				file->current_video_frame_pos = v_frames;
				file->current_audio_frame_pos = a_frames;
			}
			#endif
			/* 2007-11-18 */
			
			unlock_custommp4(file);//2007-11-18
			
			return 1;
		}
	}
	
	unlock_custommp4(file);//2007-11-18
	
	DBG("seek_to_time_stamp 3\n");
	
	return 0;
}

int custommp4_set_file_start_time(custommp4_t *file, u32 start_time)
{
	if(!file)
		return -1;

	file->fpot.start_time = start_time;
	
	return 0;
}

int custommp4_set_file_end_time(custommp4_t *file, u32 end_time)
{
	if(!file)
		return -1;

	file->fpot.end_time = end_time;
	
	return 0;
}

int custommp4_seek_to_sys_time(custommp4_t *file,u32 systime)
{
	u32 timestamp;
	if(systime > file->fpot.end_time)
	{
		return 0;
	}
	if(systime < file->fpot.start_time) timestamp = 0;
	else timestamp = (systime - file->fpot.start_time)*1000;
	//printf("file->fpot[%d, %d], seek to [%d]\n", file->fpot.start_time, file->fpot.end_time, systime);
	return custommp4_seek_to_time_stamp(file,timestamp);
}

int custommp4_write_video_frame_with_buf(custommp4_t *file, u8 *video_buffer, u32 bytes, u32 timestamp, u8 isKeyFrame, u64 pts,u8 *update, char *p_w_buf, u32 *p_w_buf_len, u32 maxlen)
{
	u32 last_pos = 0;
	int ret = 1;
	
	if(update) *update = 0;
	
	//unsigned char cur_atx_flag = tl_power_atx_check();
	/*
	if(cur_atx_flag == 0)//电池供电
	{
		file->error_flag = 1;
		return 0;
	}
	*/
	if(file->error_flag)
	{
		printf("%s error_flag 1\n", __func__);
		return 0;
	}

	//yaogang modify for bad disk
	if (file->write_error_flog)
	{
		printf("%s write_error_flog\n", __func__);
		return 0;
	}
	
	if(file->p_mdat_sect_t->video_allocated == 0) return -1;
	
	/*if(file->total_video_frames == 0 && file->total_audio_frames == 0)
	{
		file->fpot.start_time = time(NULL);
	}*/
	
	file->total_video_frames++;
	if(file->p_mdat_sect_t->video_frames == 0 && file->p_mdat_sect_t->audio_frames == 0)
	{
		file->p_mdat_sect_t->start_time = timestamp;
		ret = custommp4_set_position(file,file->p_mdat_sect_t->offset+sizeof(mdat_segment_object_t)+file->p_mdat_sect_t->video_allocated*sizeof(video_frame_info_table_t)+file->p_mdat_sect_t->audio_allocated*sizeof(audio_frame_info_table_t));
		if(ret != 1)
		{
			printf("yg  custommp4_write_video_frame_with_buf 1\n");
			return 0;
		}
	}
	file->p_mdat_sect_t->end_time = timestamp;
	if(p_w_buf_len)
	{
		u32 tmp_offset = *p_w_buf_len + custommp4_position(file)-file->open_offset;
		file->p_video_table_t[file->p_mdat_sect_t->video_frames].offset = tmp_offset;
	}
	else
	{
		file->p_video_table_t[file->p_mdat_sect_t->video_frames].offset = custommp4_position(file)-file->open_offset;
	}
	
	file->p_video_table_t[file->p_mdat_sect_t->video_frames].length = bytes;
	file->p_video_table_t[file->p_mdat_sect_t->video_frames].timestamp = timestamp;
	file->p_video_table_t[file->p_mdat_sect_t->video_frames].key = isKeyFrame;
	file->p_video_table_t[file->p_mdat_sect_t->video_frames].pts= pts;
	
	if(p_w_buf && p_w_buf_len)
	{
		if(bytes + *p_w_buf_len > maxlen)
		{
			ret = custommp4_write_data(file, p_w_buf, *p_w_buf_len);// 先将p_w_buf中的数据写入文件
			if(ret != 1)
			{
				printf("yg  custommp4_write_video_frame_with_buf 2\n");
				return 0;
			}
			memcpy(p_w_buf, video_buffer, bytes);// 再将当前帧写入p_w_buf
			*p_w_buf_len = bytes;
		}
		else
		{
			memcpy(p_w_buf + *p_w_buf_len, video_buffer, bytes);
			*p_w_buf_len += bytes;
		}
		
		if((file->p_mdat_sect_t->video_frames + 1) >= file->video_table_allocated)
		{
			ret = custommp4_write_data(file, p_w_buf, *p_w_buf_len);
			if(ret != 1)
			{	
				printf("yg  custommp4_write_video_frame_with_buf 3\n");
				return 0;
			}
			*p_w_buf_len = 0;
		}
	}
	else
	{
		ret = custommp4_write_data(file, video_buffer, bytes);
		if(ret != 1)
		{
			printf("yg  custommp4_write_video_frame_with_buf 4\n");
			return 0;
		}
	}
	
	last_pos = custommp4_position(file);
	
	file->p_mdat_sect_t->video_frames++;
	if(file->p_mdat_sect_t->video_frames >= file->video_table_allocated)
	{
		mdat_segment_object_t msot;
		
		ret = custommp4_set_position(file,file->p_mdat_sect_t->offset+sizeof(mdat_segment_object_t));
		if(ret != 1)
		{
			printf("yg  custommp4_write_video_frame_with_buf 5\n");
			return 0;
		}
		ret = custommp4_write_data(file, file->p_video_table_t, sizeof(video_frame_info_table_t) * file->p_mdat_sect_t->video_frames);
		if(ret != 1)
		{
			printf("yg  custommp4_write_video_frame_with_buf 6\n");
			return 0;
		}
		
		ret = custommp4_set_position(file,file->p_mdat_sect_t->offset+sizeof(mdat_segment_object_t)+file->p_mdat_sect_t->video_allocated*sizeof(video_frame_info_table_t));
		if(ret != 1)
		{
			printf("yg  custommp4_write_video_frame_with_buf 7\n");
			return 0;
		}
		ret = custommp4_write_data(file, file->p_audio_table_t, sizeof(audio_frame_info_table_t) * file->p_mdat_sect_t->audio_frames);
		if(ret != 1)
		{
			printf("yg  custommp4_write_video_frame_with_buf 8\n");
			return 0;
		}
		
		//printf("one segment finish 1,video frames:%d,audio frames:%d\n",file->p_mdat_sect_t->video_frames,file->p_mdat_sect_t->audio_frames);
		
		ret = custommp4_set_position(file,file->p_mdat_sect_t->offset);
		if(ret != 1)
		{
			printf("yg  custommp4_write_video_frame_with_buf 9\n");
			return 0;
		}
		
		msot.obj.object_id = CUSTOMMP4_mdat_segment_object;
		msot.obj.object_size = last_pos-file->p_mdat_sect_t->offset;//整个段的大小，下个段的起始位置由此得到
		msot.video_frames = file->p_mdat_sect_t->video_frames;
		msot.audio_frames = file->p_mdat_sect_t->audio_frames;
		msot.reserved1 = 0;
		msot.start_time = file->p_mdat_sect_t->start_time;
		msot.end_time = file->p_mdat_sect_t->end_time;
		msot.video_allocated = file->p_mdat_sect_t->video_allocated;
		msot.audio_allocated = file->p_mdat_sect_t->audio_allocated;
		
		ret = custommp4_write_data(file,&msot,sizeof(msot));
		if(ret != 1)
		{
			printf("yg  custommp4_write_video_frame_with_buf 10\n");
			return 0;
		}
		
		ret = custommp4_set_position(file,file->open_offset+sizeof(file->fpot)+sizeof(file->vspot)+sizeof(file->aspot));
		if(ret != 1)
		{
			printf("yg  custommp4_write_video_frame_with_buf 11\n");
			return 0;
		}
		
		file->mvot.number_of_segments++;
		
		ret = custommp4_write_data(file, &file->mvot, sizeof(file->mvot));
		if(ret != 1)
		{
			printf("yg  custommp4_write_video_frame_with_buf 12\n");
			return 0;
		}
		
		ret = custommp4_set_position(file,last_pos);
		if(ret != 1)
		{
			printf("yg  custommp4_write_video_frame_with_buf 13\n");
			return 0;
		}
		
		custommp4_mdat_segment_info_table_init(file, file->p_mdat_sect_t);
		
		if(update) *update = 1;
	}
	else
	{
		ret = custommp4_set_position(file,last_pos);
		if(ret != 1)
		{
			printf("yg  custommp4_write_video_frame_with_buf 14\n");
			return 0;
		}
	}
	
	if(ret != 1)
	{
		bytes = 0;
	}
	
	return bytes;
}

int custommp4_write_video_frame(custommp4_t *file,u8 *video_buffer,u32 bytes,u32 timestamp,u8 isKeyFrame,u64 pts,u8 *update)
{
	u32 last_pos;
	
	if(update) *update = 0;
	
	if(file->p_mdat_sect_t->video_allocated == 0) return -1;
	
	file->total_video_frames++;
	file->p_mdat_sect_t->video_frames++;
	if(file->p_mdat_sect_t->video_frames == 1 && file->p_mdat_sect_t->audio_frames == 0)
	{
		file->p_mdat_sect_t->start_time = timestamp;
		custommp4_set_position(file,file->p_mdat_sect_t->offset+sizeof(mdat_segment_object_t)+file->p_mdat_sect_t->video_allocated*sizeof(video_frame_info_table_t)+file->p_mdat_sect_t->audio_allocated*sizeof(audio_frame_info_table_t));
	}
	file->p_mdat_sect_t->end_time = timestamp;
	file->p_video_table_t[0].offset = custommp4_position(file)-file->open_offset;//csp modify:相对偏移
	file->p_video_table_t[0].length = bytes;
	file->p_video_table_t[0].timestamp = timestamp;
	file->p_video_table_t[0].key = isKeyFrame;
	file->p_video_table_t[0].pts= pts;//wrchen 081226
	custommp4_write_data(file, video_buffer, bytes);
	
	last_pos = custommp4_position(file);
	
	custommp4_set_position(file,file->p_mdat_sect_t->offset+sizeof(mdat_segment_object_t)+(file->p_mdat_sect_t->video_frames-1)*sizeof(video_frame_info_table_t));
	custommp4_write_data(file,file->p_video_table_t,sizeof(video_frame_info_table_t));
	//printf("file frame length=%d,bytes=%d\n",tmp_vframe.length,bytes);
	
	if(file->p_mdat_sect_t->video_frames >= file->video_table_allocated)
	{
		mdat_segment_object_t msot;
		
		//printf("one segment finish 1,video frames:%d,audio frames:%d\n",file->p_mdat_sect_t->video_frames,file->p_mdat_sect_t->audio_frames);
		
		custommp4_set_position(file,file->p_mdat_sect_t->offset);
		msot.obj.object_id = CUSTOMMP4_mdat_segment_object;
		msot.obj.object_size = last_pos-file->p_mdat_sect_t->offset;
		msot.video_frames = file->p_mdat_sect_t->video_frames;
		msot.audio_frames = file->p_mdat_sect_t->audio_frames;
		msot.reserved1 = 0;
		msot.start_time = file->p_mdat_sect_t->start_time;
		msot.end_time = file->p_mdat_sect_t->end_time;
		msot.video_allocated = file->p_mdat_sect_t->video_allocated;
		msot.audio_allocated = file->p_mdat_sect_t->audio_allocated;
		custommp4_write_data(file,&msot,sizeof(msot));
		
		custommp4_set_position(file,file->open_offset+sizeof(file->fpot)+sizeof(file->vspot)+sizeof(file->aspot));
		file->mvot.number_of_segments++;
		custommp4_write_data(file,&file->mvot,sizeof(file->mvot));
		
		custommp4_set_position(file,last_pos);
		
		custommp4_mdat_segment_info_table_init(file, file->p_mdat_sect_t);
		
		if(update) *update = 1;
	}
	else
	{
		custommp4_set_position(file,last_pos);
	}
	
	return bytes;
}

/*int custommp4_write_h264_frame(custommp4_t *file,u8 *video_buffer,u32 bytes,u32 timestamp,u8 isKeyFrame,u8 *update,u16 nal_size)
{
	u32 last_pos;
	
	if(update) *update = 0;
	
	if(file->p_mdat_sect_t->video_allocated == 0) return -1;
	
	file->total_video_frames++;
	file->p_mdat_sect_t->video_frames++;
	if(file->p_mdat_sect_t->video_frames == 1 && file->p_mdat_sect_t->audio_frames == 0)
	{
		file->p_mdat_sect_t->start_time = timestamp;
		custommp4_set_position(file,file->p_mdat_sect_t->offset+sizeof(mdat_segment_object_t)+file->p_mdat_sect_t->video_allocated*sizeof(video_frame_info_table_t)+file->p_mdat_sect_t->audio_allocated*sizeof(audio_frame_info_table_t));
	}
	file->p_mdat_sect_t->end_time = timestamp;
	file->p_video_table_t[0].offset = custommp4_position(file)-file->open_offset;//csp modify:相对偏移
	file->p_video_table_t[0].length = bytes;
	file->p_video_table_t[0].timestamp = timestamp;
	file->p_video_table_t[0].key = isKeyFrame;
	file->p_video_table_t[0].reserved1 = 0;
	file->p_video_table_t[0].reserved2 = nal_size;
	custommp4_write_data(file, video_buffer, bytes);
	
	last_pos = custommp4_position(file);
	
	custommp4_set_position(file,file->p_mdat_sect_t->offset+sizeof(mdat_segment_object_t)+(file->p_mdat_sect_t->video_frames-1)*sizeof(video_frame_info_table_t));
	custommp4_write_data(file,file->p_video_table_t,sizeof(video_frame_info_table_t));
	
	if(file->p_mdat_sect_t->video_frames >= file->video_table_allocated)
	{
		mdat_segment_object_t msot;
		
		//printf("one segment finish 1,video frames:%d,audio frames:%d\n",file->p_mdat_sect_t->video_frames,file->p_mdat_sect_t->audio_frames);
		
		custommp4_set_position(file,file->p_mdat_sect_t->offset);
		msot.obj.object_id = CUSTOMMP4_mdat_segment_object;
		msot.obj.object_size = last_pos-file->p_mdat_sect_t->offset;
		msot.video_frames = file->p_mdat_sect_t->video_frames;
		msot.audio_frames = file->p_mdat_sect_t->audio_frames;
		msot.reserved1 = 0;
		msot.start_time = file->p_mdat_sect_t->start_time;
		msot.end_time = file->p_mdat_sect_t->end_time;
		msot.video_allocated = file->p_mdat_sect_t->video_allocated;
		msot.audio_allocated = file->p_mdat_sect_t->audio_allocated;
		custommp4_write_data(file, &msot, sizeof(msot));
		
		custommp4_set_position(file,file->open_offset+sizeof(file->fpot)+sizeof(file->vspot)+sizeof(file->aspot));
		file->mvot.number_of_segments++;
		custommp4_write_data(file,&file->mvot,sizeof(file->mvot));
		
		custommp4_set_position(file, last_pos);
		
		custommp4_mdat_segment_info_table_init(file,file->p_mdat_sect_t);
		
		if(update) *update = 1;
	}
	else
	{
		custommp4_set_position(file,last_pos);
	}
	
	return bytes;
}*/

int custommp4_write_audio_frame_with_buf(custommp4_t *file,u8 *audio_buffer,u32 bytes,u32 timestamp,u8 *update, char *p_w_buf, u32 *p_w_buf_len, u32 maxlen)
{
	u32 last_pos = 0;
	int ret = 1;
	
	if(update) *update = 0;

	/*
	unsigned char cur_atx_flag = tl_power_atx_check();
	if(cur_atx_flag == 0)//电池供电
	{
		file->error_flag = 1;
		return 0;
	}
	*/
	if(file->error_flag)
	{
		return 0;
	}

	//yaogang modify for bad disk
	if (file->write_error_flog)
	{
		printf("%s write_error_flog\n", __func__);
		return 0;
	}
	
	if(file->p_mdat_sect_t->audio_allocated == 0) return -1;
	
	file->total_audio_frames++;
	if(file->p_mdat_sect_t->audio_frames == 0 && file->p_mdat_sect_t->video_frames == 0)
	{
		file->p_mdat_sect_t->start_time = timestamp;
		ret = custommp4_set_position(file,file->p_mdat_sect_t->offset+sizeof(mdat_segment_object_t)+file->p_mdat_sect_t->video_allocated*sizeof(video_frame_info_table_t)+file->p_mdat_sect_t->audio_allocated*sizeof(audio_frame_info_table_t));
		if(ret != 1)
		{
			return 0;
		}
	}
	file->p_mdat_sect_t->end_time = timestamp;
	if(p_w_buf_len)
	{
		file->p_audio_table_t[file->p_mdat_sect_t->audio_frames].offset = *p_w_buf_len + custommp4_position(file)-file->open_offset;
	}
	else
	{
		file->p_audio_table_t[file->p_mdat_sect_t->audio_frames].offset = custommp4_position(file)-file->open_offset;
	}
	file->p_audio_table_t[file->p_mdat_sect_t->audio_frames].timestamp = timestamp;
	file->p_audio_table_t[file->p_mdat_sect_t->audio_frames].length= bytes;
	
	if(p_w_buf && p_w_buf_len)
	{
		if(bytes + *p_w_buf_len > maxlen)
		{
			ret = custommp4_write_data(file, p_w_buf, *p_w_buf_len);
			if(ret != 1)
			{
				return 0;
			}
			memcpy(p_w_buf, audio_buffer, bytes);
			*p_w_buf_len = bytes;
		}
		else
		{
			memcpy(p_w_buf + *p_w_buf_len, audio_buffer, bytes);
			*p_w_buf_len += bytes;
		}
		
		if((file->p_mdat_sect_t->audio_frames + 1) >= file->audio_table_allocated)
		{
			ret = custommp4_write_data(file, p_w_buf, *p_w_buf_len);
			if(ret != 1)
			{
				return 0;
			}
			*p_w_buf_len = 0;
		}
	}
	else
	{
		ret = custommp4_write_data(file, audio_buffer, file->aspot.sample_size);
		if(ret != 1)
		{
			return 0;
		}
	}
	
	//printf("write audio:frame%d timestamp=%d,offset=%d\n",file->total_audio_frames-1,timestamp,file->p_audio_table_t[0].offset);
	
	last_pos = custommp4_position(file);
	file->p_mdat_sect_t->audio_frames++;
	
	if(file->p_mdat_sect_t->audio_frames >= file->audio_table_allocated)
	{
		mdat_segment_object_t msot;
		//mdat_vector_object_t tmp_mvot;
		
		ret = custommp4_set_position(file,file->p_mdat_sect_t->offset+sizeof(mdat_segment_object_t));
		if(ret != 1)
		{
			return 0;
		}
		ret = custommp4_write_data(file, file->p_video_table_t, sizeof(video_frame_info_table_t) * file->p_mdat_sect_t->video_frames);
		if(ret != 1)
		{
			return 0;
		}
		
		ret = custommp4_set_position(file,file->p_mdat_sect_t->offset+sizeof(mdat_segment_object_t)+file->p_mdat_sect_t->video_allocated*sizeof(video_frame_info_table_t));
		if(ret != 1)
		{
			return 0;
		}
		ret = custommp4_write_data(file, file->p_audio_table_t, sizeof(audio_frame_info_table_t) * file->p_mdat_sect_t->audio_frames);
		if(ret != 1)
		{
			return 0;
		}
		
		//printf("one segment finish 2,video frames:%d,audio frames:%d\n",file->p_mdat_sect_t->video_frames,file->p_mdat_sect_t->audio_frames);
		
		ret = custommp4_set_position(file,file->p_mdat_sect_t->offset);
		if(ret != 1)
		{
			return 0;
		}
		
		msot.obj.object_id = CUSTOMMP4_mdat_segment_object;
		msot.obj.object_size = last_pos-file->p_mdat_sect_t->offset;
		msot.video_frames = file->p_mdat_sect_t->video_frames;
		msot.audio_frames = file->p_mdat_sect_t->audio_frames;
		msot.reserved1 = 0;
		msot.start_time = file->p_mdat_sect_t->start_time;
		msot.end_time = file->p_mdat_sect_t->end_time;
		msot.video_allocated = file->p_mdat_sect_t->video_allocated;
		msot.audio_allocated = file->p_mdat_sect_t->audio_allocated;
		ret = custommp4_write_data(file,&msot,sizeof(msot));
		if(ret != 1)
		{
			return 0;
		}
		
		ret = custommp4_set_position(file,file->open_offset+sizeof(file->fpot)+sizeof(file->vspot)+sizeof(file->aspot));
		if(ret != 1)
		{
			return 0;
		}
		
		file->mvot.number_of_segments++;
		
		ret = custommp4_write_data(file, &file->mvot, sizeof(file->mvot));
		if(ret != 1)
		{
			return 0;
		}
		
		ret = custommp4_set_position(file,last_pos);
		if(ret != 1)
		{
			return 0;
		}
		
		custommp4_mdat_segment_info_table_init(file,file->p_mdat_sect_t);
		
		if(update) *update = 1;
	}
	else
	{
		ret = custommp4_set_position(file,last_pos);
		if(ret != 1)
		{
			return 0;
		}
	}
	
	if(ret != 1)
	{
		return 0;
	}
	
	//return file->aspot.sample_size;
	return bytes;
}


int custommp4_write_audio_frame(custommp4_t *file,u8 *audio_buffer,u32 bytes,u32 timestamp,u8 *update)
{
	u32 last_pos;
	
	if(update) *update = 0;
	
	if(file->p_mdat_sect_t->audio_allocated == 0) 
	{
		printf("file->p_mdat_sect_t->audio_allocated=%d\n",file->p_mdat_sect_t->audio_allocated);
		return -1;
	}
	//printf("file->total_audio_frames=%d\n",file->total_audio_frames);
	file->total_audio_frames++;
	file->p_mdat_sect_t->audio_frames++;
	if(file->p_mdat_sect_t->audio_frames == 1 && file->p_mdat_sect_t->video_frames == 0)
	{
		file->p_mdat_sect_t->start_time = timestamp;
		custommp4_set_position(file,file->p_mdat_sect_t->offset+sizeof(mdat_segment_object_t)+file->p_mdat_sect_t->video_allocated*sizeof(video_frame_info_table_t)+file->p_mdat_sect_t->audio_allocated*sizeof(audio_frame_info_table_t));
	}
	file->p_mdat_sect_t->end_time = timestamp;
	file->p_audio_table_t[0].offset = custommp4_position(file)-file->open_offset;//csp modify:相对偏移
	file->p_audio_table_t[0].length= bytes;
	file->p_audio_table_t[0].timestamp = timestamp;
	custommp4_write_data(file, audio_buffer, bytes);

	//printf("write audio:frame%d timestamp=%d,offset=%d\n",file->total_audio_frames-1,timestamp,file->p_audio_table_t[0].offset);

	last_pos = custommp4_position(file);

	custommp4_set_position(file,file->p_mdat_sect_t->offset+sizeof(mdat_segment_object_t)+file->p_mdat_sect_t->video_allocated*sizeof(video_frame_info_table_t)+(file->p_mdat_sect_t->audio_frames-1)*sizeof(audio_frame_info_table_t));
	custommp4_write_data(file,file->p_audio_table_t,sizeof(audio_frame_info_table_t));
	if((file->p_audio_table_t[0].length!= bytes)||(file->p_audio_table_t[0].length== 0))
	{
		printf("file audio length=%u,bytes=%u,%u frame\n",file->p_audio_table_t[0].length,bytes,file->p_mdat_sect_t->audio_frames);
	}

	if(file->p_mdat_sect_t->audio_frames >= file->audio_table_allocated)
	{
		mdat_segment_object_t msot;
		
		//printf("one segment finish 2,video frames:%d,audio frames:%d\n",file->p_mdat_sect_t->video_frames,file->p_mdat_sect_t->audio_frames);
		
		custommp4_set_position(file,file->p_mdat_sect_t->offset);
		msot.obj.object_id = CUSTOMMP4_mdat_segment_object;
		msot.obj.object_size = last_pos-file->p_mdat_sect_t->offset;
		msot.video_frames = file->p_mdat_sect_t->video_frames;
		msot.audio_frames = file->p_mdat_sect_t->audio_frames;
		msot.reserved1 = 0;
		msot.start_time = file->p_mdat_sect_t->start_time;
		msot.end_time = file->p_mdat_sect_t->end_time;
		msot.video_allocated = file->p_mdat_sect_t->video_allocated;
		msot.audio_allocated = file->p_mdat_sect_t->audio_allocated;
		custommp4_write_data(file,&msot,sizeof(msot));
		
		custommp4_set_position(file,file->open_offset+sizeof(file->fpot)+sizeof(file->vspot)+sizeof(file->aspot));
		file->mvot.number_of_segments++;
		custommp4_write_data(file,&file->mvot,sizeof(file->mvot));
		
		custommp4_set_position(file,last_pos);
		
		custommp4_mdat_segment_info_table_init(file,file->p_mdat_sect_t);
		
		if(update) *update = 1;
	}
	else
	{
		custommp4_set_position(file,last_pos);
	}
	//printf("write one frame audio,len=%u,total frame:%u\n",bytes,file->total_audio_frames);
	return bytes;
}
