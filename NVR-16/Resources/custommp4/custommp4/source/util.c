#include "custommp4.h"
#include "fs_yg.h"//yaogang hdd
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#include <stdlib.h>
#endif
#include <errno.h>

#include <time.h>

//#define SIZE_OF_FILE_VECTOR		((s64)134217728)//yaogang hdd

//extern int tl_power_atx_check();

//debug
static u8 read_data_flag = 0;// 1 // 1:mdattable 2:video 3:audio
int SetReadDataFlag(u8 byFlag)
{
	read_data_flag = byFlag;
	return 0;
}

int PrintReadDataFlag()
{
	printf("read data reason:%d\n",read_data_flag);
	return 0;
}

int fileflush(FILE *fp)
{
#ifndef WIN32
	//pw 2010/10/14
	if(NULL == fp)
	{
		return -1;
	}
	fflush(fp);
	fsync(fileno(fp));
#endif
	return 1;
}

int filecp(char *src,char *dst)
{
	//printf("*****filecp start*****\n");
	
	int nRet;
	FILE* fp1,*fp2;
	unsigned char byData[1024];
	
	fp1 = fopen(src,"rb");
	if(fp1 == NULL)
	{
		return 0;
	}
	
	fp2 = fopen(dst,"wb");
	if(fp2 == NULL)
	{
		printf("fopen %s failed,errno=%d,str=%s\n",dst,errno,strerror(errno));
		fclose(fp1);
		return 0;
	}
	
	//size_t fread(void *ptr,size_t size,size_t nmemb,FILE *stream);
	//size_t fwrite(const void *ptr,size_t size,size_t nmemb,FILE *stream);
	
	nRet = fread(byData,1,sizeof(byData),fp1);
	while(nRet > 0)
	{
		fwrite(byData,nRet,1,fp2);
		nRet = fread(byData,1,sizeof(byData),fp1);
	}
	fileflush(fp2);
	
	fclose(fp1);
	fclose(fp2);
	
	//printf("*****filecp success*****\n");
	
	return 1;
}

void uint2str(unsigned char *dst, int n)
{
	dst[0] = (n)&0xff;
	dst[1] = (n>>8)&0xff;
	dst[2] = (n>>16)&0xff;
	dst[3] = (n>>24)&0xff;
}

u32 str2uint(const char *str)
{
	return ( str[0] | (str[1]<<8) | (str[2]<<16) | (str[3]<<24) );	
}

int custommp4_position(custommp4_t *file)
{
	return file->file_position;
}

int custommp4_set_position(custommp4_t *file, int position)
{
	if(position > (int)SIZE_OF_FILE_VECTOR || position < 0)
	{
		file->error_flag = 1;
		printf("warning:set pos error:cur_pos=%ld,set_pos=%d\n",file->file_position,position);
		return -1;//csp modify//MR HDD
	}
	file->file_position = position;
	return 1;
}

int custommp4_end_position(custommp4_t *file)
{
	int ret;
	
	//pw 2010/10/14
	if(NULL == file)
	{
		return -1;
	}
	
	if(file->open_mode == O_R)
	{
		ret = file->open_offset+file->fpot.file_Size;
	}
	else
	{
		//ret = custommp4_position(file)+file->p_mdat_sect_t->video_frames*sizeof(video_frame_info_table_t)+file->p_mdat_sect_t->audio_frames*sizeof(audio_frame_info_table_t);
		ret = custommp4_position(file);
	}
	
	return ret;
}

int custommp4_read_data(custommp4_t *file, void *data, int size)
{
	int result;
	long last_pos,cur_pos;
	
	if((u32)(file->file_position) > (u32)SIZE_OF_FILE_VECTOR)
	{
		printf("warning:pos error 1:file_pos=%ld,size=%d\n",file->file_position,size);
		PrintReadDataFlag();
		return -1;
	}
	if((u32)((u32)(file->file_position) + (u32)(size)) > (u32)(SIZE_OF_FILE_VECTOR))
	{
		printf("warning:pos error 2:file_pos=%ld,size=%d\n",file->file_position,size);
		PrintReadDataFlag();
		return -1;
	}
	
	//unsigned char cur_atx_flag = tl_power_atx_check();
	//if(cur_atx_flag == 0 || file->error_flag)
	if(file->error_flag)
	{
		file->error_flag = 1;
		return -1;
	}
	
	last_pos = ftell(file->stream);
	//if(ftell(file->stream) != file->file_position)//zlb 20090113
	if(last_pos != file->file_position)
	{
		result = fseek(file->stream, file->file_position, SEEK_SET);
		if(result)
		{
			printf("warning:fseek error 1,result=%d,errno=%d,errstr=%s\n",result,errno,strerror(errno));
			result = ftell(file->stream);
			printf("warning:file_pos=%ld,last_pos=%ld,real_pos=%d,size=%d\n",file->file_position,last_pos,result,size);
			PrintReadDataFlag();
			return -1;
		}
		result = ftell(file->stream);
		if(result != file->file_position)
		{
			printf("warning:fseek error 2,file_pos=%ld,last_pos=%ld,real_pos=%d\n",file->file_position,last_pos,result);
			PrintReadDataFlag();
			return -1;
		}
	}
	
	last_pos = file->file_position;
	//last_pos = ftell(file->stream);//zlb 20090113
	result = fread(data, size, 1, file->stream);
	cur_pos = ftell(file->stream);
	if(result <= 0 || cur_pos - last_pos != size)
	{
		printf("warning:fread error,size=%d,result=%d,file_pos=%ld,last_pos=%ld,cur_pos=%ld\n",size,result,file->file_position,last_pos,cur_pos);
		PrintReadDataFlag();
		//return -1;//wrchen 081223
	}
	
	file->file_position += size;
	
	SetReadDataFlag(0);
	
	//return result;
	return size;//返回实际长度
}

int custommp4_write_data(custommp4_t *file,void *data,int size)
{
	int result;
	
	//if(size <= 0)
	if(size < 0)
	{
		file->error_flag = 1;
		printf("custommp4_write_data error,size=%d\n",size);
		//return size;
		return -1;
	}
	if(size == 0)
	{
		//printf("custommp4_write_data warning,size=%d\n",size);
		return 1;
	}
	
	if(file->file_position > (int)SIZE_OF_FILE_VECTOR)
	{
		file->error_flag = 1;
		printf("custommp4_write_data:file_pos=%ld\n",file->file_position);
		//exit(1);
		return -1;
	}
	if(file->file_position + size > (int)SIZE_OF_FILE_VECTOR || size > (int)SIZE_OF_FILE_VECTOR)
	{
		file->error_flag = 1;
		printf("custommp4_write_data:file_pos=%ld,size=%d\n",file->file_position,size);
		//exit(1);
		return -1;
	}
	
	//unsigned char cur_atx_flag = tl_power_atx_check();
	//if(cur_atx_flag == 0 || file->error_flag)
	if(file->error_flag)
	{
		file->error_flag = 1;
		return -1;
	}

	//yaogang modify for bad disk
	if (file->write_error_flog)
	{
		return -1;
	}
	
	if(ftell(file->stream) != file->file_position)
	{
		result = fseek(file->stream, file->file_position, SEEK_SET);
		if(result)
		{
			time_t cur;
			
			printf("custommp4_write_data:fseek error=%d,errno=%d,errstr=%s, fd: %d\n", 
				result, errno, strerror(errno), fileno(file->stream));
			
			//system("netstat");//yaogang modify for bad disk	
			
			cur = time(NULL);
			//printf("system current time  : %s\n",ctime(&cur));
			
			result = ftell(file->stream);
			printf("custommp4_write_data:right_pos=%ld,real_pos=%d\n", file->file_position, result);
			
			file->error_flag = 1;
			
			//exit(1);
			return -1;
		}
	}
	
	result = fwrite(data, size, 1, file->stream);
	//printf("fwrite file name:%s, size=%d\n", file->file_name, size);//yaogang hdd
	//file->file_position += size; //yaogang modify 20170419
	/*fileflush(file->stream);*/
	if(result != 1)
	{
		file->error_flag = 1;
		file->write_error_flog = 1;//yaogang modify for bad disk
		printf("custommp4_write_data:fwrite error=%d,,errno=%d,errstr=%s, fd: %d, file_name=%s,size=%d, file_position: %d\n",
			result, errno, strerror(errno), fileno(file->stream), file->file_name, size, file->file_position);

		//system("netstat");//yaogang modify for bad disk
		
		return -1;
	}

	file->file_position += size; //yaogang modify 20170419
	
	return result;
}

/*int custommp4_object_read_header(custommp4_t *file, base_object_t *pobj)
{
	int result = custommp4_read_data(file, pobj, sizeof(base_object_t));
	return !result;
}*/

BOOL custommp4_object_is(base_object_t *pobj,GUID type)
{
	if(!memcmp(&pobj->object_id,&type,sizeof(GUID)))
	{
		return TRUE;
	}
	return FALSE;
}

