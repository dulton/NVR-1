#include "partitionindex.h"
#include "fs_yg.h"//yaogang hdd

#include <string.h>
#include <time.h>

#ifndef WIN32
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>

#endif

#include <assert.h>
#include <errno.h>

//extern int tl_power_atx_check();

#ifndef MAX_CHN_NUM 
#define MAX_CHN_NUM 64
#endif

#define DISK_NEW_MMAP_WAY//csp modify 20100311

int sync_partition_index(void *start, size_t length, int flags)
{
	void *real_start = (void *)((int)start - ((int)start & 0x0fff));
	int real_length = length + ((int)start & 0x0fff);
	int ret = msync(real_start,real_length,flags);
	/*printf("******ret=%d,start=0x%08x,length=%d,real_start=0x%08x,real_length=%d\n",ret,(int)start,length,(int)real_start,real_length);
	if(ret<0)
	{
		printf("!!!!!!msync failed,errcode=%d,errstr=%s\n",errno,strerror(errno));
		exit(-1);
	}
	else
	{
		printf("!!!!!!msync success,errcode=%d,errstr=%s\n",errno,strerror(errno));
	}*/
	return ret;
}

int lock_partition_index(partition_index *index)
{
#ifndef WIN32
	return pthread_mutex_lock(&index->lock);
#endif
	return 1;
}

int unlock_partition_index(partition_index *index)
{
#ifndef WIN32
	return pthread_mutex_unlock(&index->lock);
#endif
	return 1;
}

BOOL is_partition_pic_index_valid(FILE *fp)
{
	partition_pic_index_header header;
	u8  *p = (u8 *)&header;
	u32 verify = 0;
	u32 file_len = 0;
	int i = 0;
	
	memset(&header,0,sizeof(header));
	
	int nRet = fread(&header,sizeof(header),1,fp);
	if(nRet<=0)
	{
		printf("read file error\n");
		return FALSE;
	}
	printf("%s: \n", __func__);
	printf("\t file_nums: %d\n", header.file_nums);
	printf("\t file_cur_no: %d\n", header.file_cur_no);
	printf("\t partition_pic_nums: %d\n", header.partition_pic_nums);
	printf("\t start_sec: %d\n", header.start_sec);
	printf("\t start_usec: %d\n", header.start_usec);
	printf("\t end_sec: %d\n", header.end_sec);
	printf("\t end_usec: %d\n", header.end_usec);
	
	//csp add 20090331
	if(header.reserved[0] != 0)
	{
		printf("is_partition_pic_index_valid:error 1\n");
		return FALSE;
	}
	
	if(header.file_nums > PIC_FILE_NUMS-PRE_RECSNAP_FILES || header.file_cur_no > PIC_FILE_NUMS-PRE_RECSNAP_FILES)
	{
		printf("count file error, file_nums: %d, file_cur_no: %d\n", header.file_nums, header.file_cur_no);
		return FALSE;
	}
	fseek(fp,0,SEEK_END);
	file_len = ftell(fp);
	
	if(file_len != sizeof(partition_pic_index_header)+header.file_nums*(sizeof(file_pic_use_info)+PIC_NUMS_PER_FILE*sizeof(pic_use_info)))//yaogang hdd
	{
		printf("file_pic_index len error\n");
		return FALSE;
	}
	
	verify = 0;
	for(i=0;i<sizeof(partition_pic_index_header)-4;i++)
	{
		verify += p[i];
	}
	printf("%s verify: 0x%x, header.verify: 0x%x\n", __func__, verify, header.verify);
	if(verify != header.verify)
	{
		printf("%s: file verify failed\n", __FUNCTION__);
		return FALSE;
	}
	return TRUE;
}


BOOL is_partition_index_valid(FILE *fp)
{
	partition_index_header header;
	u8  *p = (u8 *)&header;
	u32 verify = 0;
	u32 file_len = 0;
	int i = 0;
	
	memset(&header,0,sizeof(header));//csp add 20090331
	
	int nRet = fread(&header,sizeof(header),1,fp);
	if(nRet<=0)
	{
		printf("read file error\n");
		return FALSE;
	}
	
	//csp add 20090331
	if(header.reserved1 != 0)
	{
		printf("is_partition_index_valid:error flag\n");
		return FALSE;
	}
	
	if(header.full_file_nums>header.total_file_nums || header.recorded_file_nums>header.total_file_nums || header.full_file_offset>=header.total_file_nums)
	{
		printf("count file error\n");
		return FALSE;
	}
	fseek(fp,0,SEEK_END);
	file_len = ftell(fp);
	//if(file_len != sizeof(partition_index_header)+header.total_file_nums*(sizeof(file_use_info)+8192))
	if(file_len != sizeof(partition_index_header)+header.total_file_nums*(sizeof(file_use_info)+NFILE_PER_SECT*32))//yaogang hdd
	{
		printf("file len error\n");
		return FALSE;
	}
	for(i=0;i<sizeof(partition_index_header)-4;i++)
	{
		verify += p[i];
	}
	if(verify != header.verify)
	{
		printf("verify error 1\n");
		return FALSE;
	}
	return TRUE;
}

int update_partition_index(partition_index *index)
{
	if(index->valid)
	{
		return 1;
	}
	
	return 0;
}

int write_partition_index_header(partition_index *index,partition_index_header *pHeader)
{
	u8  *p = (u8 *)pHeader;
	u32 verify = 0;
	int i = 0;
	
	pHeader->op_nums++;
	for(i = 0; i < sizeof(partition_index_header) - 4; i++)
	{
		verify += p[i];
	}
	pHeader->verify = verify;
	
	#ifdef DISK_NEW_MMAP_WAY
	memcpy(index->cachedst,pHeader,sizeof(partition_index_header));
	
	fseek(index->index1,0,SEEK_SET);
	fwrite(pHeader,sizeof(partition_index_header),1,index->index1);
	fflush(index->index1);
	fsync(fileno(index->index1));
	
	fseek(index->index2,0,SEEK_SET);
	fwrite(pHeader,sizeof(partition_index_header),1,index->index2);
	fflush(index->index2);
	fsync(fileno(index->index2));
	#else
	memcpy(index->mapdst1,pHeader,sizeof(partition_index_header));
	sync_partition_index(index->mapdst1, sizeof(partition_index_header), MS_SYNC);
	
	memcpy(index->mapdst2,pHeader,sizeof(partition_index_header));
	sync_partition_index(index->mapdst2, sizeof(partition_index_header), MS_SYNC);
	#endif
	
	return 1;
}

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int pic_indexfilecp(char *src,char *dst)
{
	//printf("*****indexfilecp start*****\n");
	
	int nRet, read_size = 0;
	FILE *fp1 = NULL;
	FILE *fp2 = NULL;
	

#define THIS_DATA_SIZE (1 << 20)
	unsigned char *byData = NULL;
	byData = malloc(THIS_DATA_SIZE);
	if(NULL == byData)
	{
		return 0;
	}
	read_size = THIS_DATA_SIZE;

	
	fp1 = fopen(src,"rb");
	if(fp1 == NULL)
	{
		#ifdef THIS_DATA_SIZE
		free(byData);
		#endif
		return 0;
	}
	
	fp2 = fopen(dst,"rb+");
	if(fp2 == NULL)
	{
		//fp2 = fopen(dst,"wb+");//"wb"
		//if(fp2 == NULL)
		{
			printf("fopen %s failed,errno=%d,str=%s\n",dst,errno,strerror(errno));
			fclose(fp1);
			#ifdef THIS_DATA_SIZE
			free(byData);
			#endif
			return 0;
		}
	}

	fseek(fp1,0,SEEK_END);
	int filelen = ftell(fp1);
	fseek(fp1,0,SEEK_SET);
	//csp add
	memset(byData,0,read_size);
	
	int remain = filelen;
	int real = 0;
	while(remain > 0)
	{
		real = read_size > remain ? remain : read_size;
		//printf("real=%d\n", real);
		nRet = fread(byData,real,1,fp1);
		if(nRet <= 0)
		{
			break;
		}
		fwrite(byData,real,1,fp2);
		remain -= real;
	}
	//csp add
	
	fileflush(fp2);
	
	fclose(fp1);
	fclose(fp2);
	
	//printf("*****indexfilecp success*****\n");
	
	#ifdef THIS_DATA_SIZE
	free(byData);
	#endif
	
	return 1;
}

//csp add 20090331
int indexfilecp(char *src,char *dst)
{
	//printf("*****indexfilecp start*****\n");
	
	int nRet, read_size = 0;
	FILE *fp1 = NULL;
	FILE *fp2 = NULL;
	
#if 0
	unsigned char byData[1024];
	read_size = sizeof(byData);
#else
	#define THIS_DATA_SIZE (1 << 20)
	unsigned char *byData = NULL;
	byData = malloc(THIS_DATA_SIZE);
	if(NULL == byData)
	{
		return 0;
	}
	read_size = THIS_DATA_SIZE;
#endif
	
	fp1 = fopen(src,"rb");
	if(fp1 == NULL)
	{
		#ifdef THIS_DATA_SIZE
		free(byData);
		#endif
		return 0;
	}
	
	fp2 = fopen(dst,"rb+");
	if(fp2 == NULL)
	{
		fp2 = fopen(dst,"wb+");//"wb"
		if(fp2 == NULL)
		{
			printf("fopen %s failed,errno=%d,str=%s\n",dst,errno,strerror(errno));
			fclose(fp1);
			#ifdef THIS_DATA_SIZE
			free(byData);
			#endif
			return 0;
		}
	}
	//else
	//{
		//fseek(fp2,0,SEEK_SET);
	//}
	
	//size_t fread(void *ptr,size_t size,size_t nmemb,FILE *stream);
	//size_t fwrite(const void *ptr,size_t size,size_t nmemb,FILE *stream);
	
	//csp add
	fseek(fp1,0,SEEK_END);
	int filelen = ftell(fp1);
	fseek(fp1,0,SEEK_SET);
	//csp add
	
#if 1
	memset(byData,0,read_size);
	
	nRet = fread(byData,1,512,fp1);
	if(nRet != 512)
	{
		printf("indexfilecp error 1,nRet=%d\n",nRet);
		fclose(fp1);
		fclose(fp2);
		#ifdef THIS_DATA_SIZE
		free(byData);
		#endif
		return 0;
	}
	
	//csp add
	unsigned char byData2[512];
	memset(byData2,0,sizeof(byData2));
	fread(byData2,1,512,fp2);
	if(memcmp(byData,byData2,512) == 0)
	{
		partition_index_header header;
		memcpy(&header,byData,sizeof(header));
		
		filelen = sizeof(header) + header.total_file_nums * (sizeof(file_use_info));
		
		//printf("indexfilecp same\n");
		
		//fclose(fp1);
		//fclose(fp2);
		//#ifdef THIS_DATA_SIZE
		//free(byData);
		//#endif
		//return 0;
		
		fseek(fp2,0,SEEK_SET);
	}
	else
	{
		printf("indexfilecp diff\n");
		
		fseek(fp2,0,SEEK_SET);
	}
	//csp add
	
	unsigned char backup_data[4] = {0,0,0,0};
	memcpy(backup_data,byData+4,4);
	memset(byData+4,0xff,4);
	
	fwrite(byData,nRet,1,fp2);
	
	#if 0
	nRet = fread(byData,1,read_size,fp1);
	while(nRet > 0)
	{
		fwrite(byData,nRet,1,fp2);
		nRet = fread(byData,1,read_size,fp1);
	}
	#else
	//csp add
	int remain = filelen - 512;
	while(remain > 0)
	{
		int real = read_size > remain ? remain : read_size;
		//printf("real=%d\n", real);
		nRet = fread(byData,real,1,fp1);
		if(nRet <= 0)
		{
			break;
		}
		fwrite(byData,real,1,fp2);
		remain -= real;
	}
	//csp add
	#endif
	
	fileflush(fp2);
	
	fseek(fp2,4,SEEK_SET);
	fwrite(backup_data,4,1,fp2);
	fileflush(fp2);
#else
	nRet = fread(byData,1,sizeof(byData),fp1);
	while(nRet > 0)
	{
		fwrite(byData,nRet,1,fp2);
		nRet = fread(byData,1,sizeof(byData),fp1);
	}
	fileflush(fp2);
#endif
	
	fclose(fp1);
	fclose(fp2);
	
	//printf("*****indexfilecp success*****\n");
	
	#ifdef THIS_DATA_SIZE
	free(byData);
	#endif
	
	return 1;
}

int init_partition_index(partition_index *index,char *path)
{
	BOOL bValid = FALSE;
	BOOL bBackupValid = FALSE;
	char filename[64];
	char backup_filename[64];
	
	//yaogang hdd
	//sprintf(filename, "%s/index00.bin", path);
	//sprintf(backup_filename, "%s/index01.bin", path);
	sprintf(filename, "%s/index10.bin", path);
	sprintf(backup_filename, "%s/index11.bin", path);
	
	memset(index, 0, sizeof(partition_index));
	
	strcpy(index->mountpath, path);
	
	dbgprint("init_partition_index:before fopen_1 file:%s\n",filename);
	index->index1 = fopen(filename, "rb+");
	if(index->index1 == NULL)
	{
		perror("fopen file index1 error 1!");
		return 0;
	}
	else
	{
		if(is_partition_index_valid(index->index1))
		{
			dbgprint("index file:%s is valid\n",filename);
			bValid = TRUE;
		}
		else
		{
			dbgprint("index file:%s is not valid\n",filename);
		}
		fclose(index->index1);
		index->index1 = NULL;
		dbgprint("fclose 1\n");
	}
	
	if(bValid)
	{
		dbgprint("valid:before file copy,src:%s,dst:%s\n",filename,backup_filename);
		indexfilecp(filename, backup_filename);
		dbgprint("valid:after file copy,src:%s,dst:%s\n",filename,backup_filename);
		
		//goto VERIFYEND;
	}
	else
	{
		dbgprint("***init_partition_index:before fopen_2 file:%s,index2=0x%08x\n",backup_filename,(int)index->index2);
		index->index2 = fopen(backup_filename,"rb+");
		dbgprint("***init_partition_index:after fopen_2 file:%s,index2=0x%08x\n",backup_filename,(int)index->index2);
		
		if(index->index2 == NULL)
		{
			perror("fopen file index2 error 1!");
			return 0;
		}
		else
		{
			if(is_partition_index_valid(index->index2))
			{
				dbgprint("backup index file:%s is valid\n",backup_filename);
				bBackupValid = TRUE;
			}
			else
			{
				dbgprint("backup index file:%s is not valid\n",backup_filename);
			}
			fclose(index->index2);
			index->index2 = NULL;
			dbgprint("fclose 2\n");

			if(bBackupValid)
			{
				dbgprint("backup valid:before file copy,src:%s,dst:%s\n",filename,backup_filename);
				indexfilecp(backup_filename, filename);
				dbgprint("backup valid:after file copy,src:%s,dst:%s\n",filename,backup_filename);
			}
			else
			{
				printf("no valid index file\n");
				return 0;
			}
		}
	}
	
	//yaogang modify 20141225
	BOOL bValid_pic = FALSE;
	BOOL bBackupValid_pic = FALSE;
	char filename_pic[64];
	char backup_filename_pic[64];
	
	//yaogang hdd
	//sprintf(filename, "%s/index00.bin", path);
	//sprintf(backup_filename, "%s/index01.bin", path);
	sprintf(filename_pic, "%s/pindx10.bin", path);
	sprintf(backup_filename_pic, "%s/pindx11.bin", path);
	
	index->pic_idx = fopen(filename_pic, "rb+");
	if(index->pic_idx == NULL)
	{
		perror("fopen pic_idx error 1!");
		return 0;
	}
	else
	{
		//printf("%s fopen file: %s success\n", __FUNCTION__, filename_pic);
		if(is_partition_pic_index_valid(index->pic_idx))
		{
			dbgprint("pic_index file:%s is valid\n",filename_pic);
			bValid_pic = TRUE;
			//printf("%s fopen file: %s verify success\n", __FUNCTION__, filename_pic);
		}
		else
		{
			printf("%s fopen file: %s verify failed\n", __FUNCTION__, filename_pic);
		}
		fclose(index->pic_idx);
		index->pic_idx = NULL;
		dbgprint("fclose pic_idx\n");
	}
	printf("%s bValid_pic: %d\n", __FUNCTION__, bValid_pic);
	if(bValid_pic)
	{
		dbgprint("valid:before pic file copy,src:%s,dst:%s\n",filename_pic,backup_filename_pic);
		pic_indexfilecp(filename_pic, backup_filename_pic);
		dbgprint("valid:after pic file copy,src:%s,dst:%s\n",filename_pic,backup_filename_pic);
	}
	else
	{
		dbgprint("***init_partition_index:before fopen_2 pic file:%s,index2=0x%08x\n",backup_filename_pic, (int)index->pic_idx_bk);
		index->pic_idx_bk = fopen(backup_filename_pic,"rb+");
		dbgprint("***init_partition_index:after fopen_2 pic file:%s,index2=0x%08x\n",backup_filename_pic, (int)index->pic_idx_bk);
		
		if(index->pic_idx_bk == NULL)
		{
			perror("fopen pic_idx_bk 1!");
			return 0;
		}
		else
		{
			//printf("%s fopen file: %s success\n", __FUNCTION__, backup_filename_pic);
			if(is_partition_pic_index_valid(index->pic_idx_bk))
			{
				dbgprint("backup pic index file:%s is valid\n",backup_filename_pic);
				bBackupValid_pic = TRUE;
			}
			else
			{
				dbgprint("backup pic index file:%s is not valid\n",backup_filename_pic);
			}
			fclose(index->pic_idx_bk);
			index->pic_idx_bk = NULL;
			dbgprint("fclose pic_idx_bk\n");

			if(bBackupValid_pic)
			{
				dbgprint("backup valid:before pic file copy, src:%s, dst:%s\n",filename_pic, backup_filename_pic);
				pic_indexfilecp(backup_filename_pic, filename_pic);
				dbgprint("backup valid:after pic file copy, src:%s, dst:%s\n",filename_pic, backup_filename_pic);
			}
			else
			{
				printf("no valid pic index file\n");
				return 0;
			}
		}
	}
	
	
	
VERIFYEND://csp add 20090331	
	printf("%s VERIFYEND 1\n", __FUNCTION__);
	
	index->index1 = fopen(filename, "rb+");
	if(index->index1 == NULL)
	{
		perror("fopen file index1 error 2!");
		return 0;
	}
	index->index2 = fopen(backup_filename, "rb+");
	if(index->index2 == NULL)
	{
		perror("fopen file index2 error 2!");
		fclose(index->index1);
		index->index1 = NULL;
		return 0;
	}
	
	//yaogang modify 20141225
	index->pic_idx = fopen(filename_pic, "rb+");
	if(index->pic_idx == NULL)
	{
		perror("fopen pic_idx err 2");
		//printf("open file: %s failed\n", filename_pic);
		fclose(index->index1);
		index->index1 = NULL;
		fclose(index->index2);
		index->index2 = NULL;
		return 0;
	}
	index->pic_idx_bk = fopen(backup_filename_pic, "rb+");
	if(index->pic_idx_bk == NULL)
	{
		perror("fopen pic_idx_bk err 2");
		fclose(index->index1);
		index->index1 = NULL;
		fclose(index->index2);
		index->index2 = NULL;
		fclose(index->pic_idx);
		index->pic_idx = NULL;
		return 0;
	}
	
	fread(&index->header,sizeof(index->header),1,index->index1);
	fseek(index->index1,0,SEEK_SET);

	//yaogang modify 20141225
	fread(&index->pic_header,sizeof(index->pic_header),1,index->pic_idx);
	fseek(index->pic_idx,0,SEEK_SET);
	
	int readlen = sizeof(partition_index_header) + index->header.total_file_nums * sizeof(file_use_info);
	//readlen = (readlen+4096-1)/4096*4096;
	
	index->cachedst = (u8 *)malloc(readlen);
	if(index->cachedst == NULL)
	{
		perror("###malloc cache###");
		fclose(index->index1);
		index->index1 = NULL;
		fclose(index->index2);
		index->index2 = NULL;
		return 0;
	}
	index->cachelen = readlen;
	
	fread(index->cachedst,index->cachelen,1,index->index1);
	fseek(index->index1,0,SEEK_SET);
	
	
#ifndef WIN32
	pthread_mutex_init(&index->lock,NULL);//csp modify 20100416
#endif
	
	index->valid = 1;
	
	//update_partition_index(index);
	printf("%s VERIFYEND 2\n", __FUNCTION__);
	return 1;
}

int destroy_partition_index(partition_index *index)
{
	if(index->valid)
	{
		index->valid = 0;
#ifndef WIN32
		//lock_partition_index(index);
		pthread_mutex_destroy(&index->lock);
#endif
		if(index->cachedst)
		{
			free(index->cachedst);
			index->cachedst = NULL;
			index->cachelen = 0;
		}
		if(index->index1)
		{
			//printf("fclose 1 with fsync\n");
			fclose(index->index1);
			index->index1 = NULL;
		}
		if(index->index2)
		{
			//printf("fclose 2 with fsync\n");
			fclose(index->index2);
			index->index2 = NULL;
		}
		if(index->pic_idx)
		{
			//printf("fclose pic_idx with fsync\n");
			fclose(index->pic_idx);
			index->pic_idx = NULL;
		}
		if(index->pic_idx_bk)
		{
			//printf("fclose pic_idx_bk with fsync\n");
			fclose(index->pic_idx_bk);
			index->pic_idx_bk = NULL;
		}
	}
	
	return 1;
}

s64 get_partition_total_space(partition_index *index)
{
	int file_nums;
	
	if(index && index->valid)
	{
		dbgprint("get_partition_total_space:index addr:0x%08x\n",(int)&index);
		dbgprint("total_file_nums=%d\n",index->header.total_file_nums);
		dbgprint("total_pic_file_nums=%d\n",index->pic_header.file_nums);

		//printf("get_partition_total_space:index addr:0x%08x\n",(int)&index);
		//printf("total_file_nums=%d\n",index->header.total_file_nums);
		//printf("total_pic_file_nums=%d\n",index->pic_header.file_nums);
				

		file_nums = index->header.total_file_nums + index->pic_header.file_nums;
		return file_nums * SIZE_OF_FILE_VECTOR;
	}
	return 0;
}

s64 get_partition_free_space(partition_index *index)
{
	int file_nums;
	
	if(index && index->valid)
	{
		dbgprint("free_file_nums=%d\n", index->header.total_file_nums - index->header.recorded_file_nums);
		dbgprint("free_pic_file_nums=%d\n", index->pic_header.file_nums - index->pic_header.file_cur_no);

		//printf("free_file_nums=%d\n", index->header.total_file_nums - index->header.recorded_file_nums);
		//printf("free_pic_file_nums=%d\n", index->pic_header.file_nums - index->pic_header.file_cur_no);

		file_nums = index->header.total_file_nums - index->header.recorded_file_nums;
		file_nums += index->pic_header.file_nums - index->pic_header.file_cur_no;
		return file_nums * SIZE_OF_FILE_VECTOR;
	}
	return 0;
}

long seek_to_segment(partition_index *index,partition_index_header *pHeader,int file_no,int sect_no)
{
	if(index->valid)
	{
		//long position = sizeof(partition_index_header) + pHeader->total_file_nums * sizeof(file_use_info) + 8192 * file_no + 32 * sect_no;
		long position = sizeof(partition_index_header) + pHeader->total_file_nums * sizeof(file_use_info) + (NFILE_PER_SECT*32) * file_no + 32 * sect_no;//yaogang hdd
		return position;
	}
	return 0;
}

int get_chn_next_segment(partition_index *index,int chn,int *file_no,int *sect_offset)
{
	if(index->valid)
	{
		lock_partition_index(index);
		
		if(index->header.full_file_nums < index->header.total_file_nums)
		{
			chn_last_use_file *p_chn_info = &index->header.chn_info[chn+1];
			segment_use_info s_u_info;
			file_use_info f_u_info;
			long position;
			if(p_chn_info->file_no < index->header.total_file_nums)//320G-->594//yaogang hdd
			{
				//csp modify 20130106
				position = seek_to_segment(index, &index->header, p_chn_info->file_no, p_chn_info->sect_nums);
				//memcpy(&s_u_info, index->mapdst1 + position, sizeof(s_u_info));
				fseek(index->index1, position, SEEK_SET);
				fread(&s_u_info, sizeof(s_u_info), 1, index->index1);
				//fseek(index->index1,0,SEEK_SET);
				
				//csp modify 20130106
				//if(p_chn_info->sect_nums < 255 && p_chn_info->end_time <= (u32)time(NULL))//缺少开始时间的判断
				//if(p_chn_info->sect_nums < 255 && p_chn_info->end_time <= (u32)time(NULL) && s_u_info.end_position <= (SIZE_OF_FILE_VECTOR-2*SIZE_OF_RESERVED))
				//yaogang hdd
				if(p_chn_info->sect_nums < (NFILE_PER_SECT-1) && p_chn_info->end_time <= (u32)time(NULL) && s_u_info.end_position <= (SIZE_OF_FILE_VECTOR-2*SIZE_OF_RESERVED))
				{
					//csp modify 20130106
					//position = seek_to_segment(index, &index->header, p_chn_info->file_no, p_chn_info->sect_nums);
					//memcpy(&s_u_info, index->mapdst1 + position, sizeof(s_u_info));
					
					*file_no = p_chn_info->file_no;
					*sect_offset = (s_u_info.end_position / 512 + 1) * 512;//上一个文件的结束位置512对齐后就是新文件开始位置//yaogang hdd
					p_chn_info->sect_nums++;//文件数递增//yaogang hdd
					
					//printf("#####chn%d:file_no=%d,sect_offset=%d,sect_nums=%d\n",chn,*file_no,*sect_offset,p_chn_info->sect_nums+1);
					
					//写新文件在文件容器的索引//yaogang hdd
					memset(&s_u_info,0,sizeof(s_u_info));
					s_u_info.type = 0;
					s_u_info.start_time = time(NULL);
					s_u_info.end_time = s_u_info.start_time;
					s_u_info.start_position = *sect_offset;
					s_u_info.end_position = s_u_info.start_position;
					position = seek_to_segment(index, &index->header, p_chn_info->file_no, p_chn_info->sect_nums);
					
					#ifdef DISK_NEW_MMAP_WAY
					fseek(index->index1,position,SEEK_SET);
					fwrite(&s_u_info,sizeof(s_u_info),1,index->index1);
					fflush(index->index1);
					fsync(fileno(index->index1));
					
					fseek(index->index2,position,SEEK_SET);
					fwrite(&s_u_info,sizeof(s_u_info),1,index->index2);
					fflush(index->index2);
					fsync(fileno(index->index2));
					#else
					memcpy(index->mapdst1 + position, &s_u_info, sizeof(s_u_info));
					sync_partition_index(index->mapdst1 + position, sizeof(s_u_info), MS_SYNC);
					
					memcpy(index->mapdst2+position, &s_u_info, sizeof(s_u_info));
					sync_partition_index(index->mapdst2 + position, sizeof(s_u_info), MS_SYNC);
					#endif
					
					unlock_partition_index(index);
					
					return 1;
				}
				else//切换下一个文件容器//yaogang hdd
				{
					f_u_info.start_time = p_chn_info->start_time;
					f_u_info.end_time = p_chn_info->end_time;
					f_u_info.file_no = p_chn_info->file_no;
					f_u_info.busy = 0;
					f_u_info.chn_no = chn+1;
					f_u_info.sect_nums = p_chn_info->sect_nums;
					
					position = sizeof(index->header)+sizeof(file_use_info)*((index->header.full_file_offset+index->header.full_file_nums)%index->header.total_file_nums);
					
					#ifdef DISK_NEW_MMAP_WAY
					memcpy(index->cachedst + position, &f_u_info, sizeof(f_u_info));
					
					fseek(index->index1,position,SEEK_SET);
					fwrite(&f_u_info,sizeof(f_u_info),1,index->index1);
					fflush(index->index1);
					fsync(fileno(index->index1));
					
					fseek(index->index2,position,SEEK_SET);
					fwrite(&f_u_info,sizeof(f_u_info),1,index->index2);
					fflush(index->index2);
					fsync(fileno(index->index2));
					#else
					memcpy(index->mapdst1 + position, &f_u_info, sizeof(f_u_info));
					sync_partition_index(index->mapdst1 + position, sizeof(f_u_info), MS_SYNC);
					
					memcpy(index->mapdst2+position,&f_u_info,sizeof(f_u_info));
					sync_partition_index(index->mapdst2+position,sizeof(f_u_info),MS_SYNC);
					#endif
					
					index->header.full_file_nums++;
					p_chn_info->file_no = 0xffff;
					p_chn_info->start_time = 0;
					p_chn_info->end_time = 0;
					p_chn_info->sect_nums = 0;
					
					write_partition_index_header(index,&index->header);
				}
			}
			if(index->header.recorded_file_nums < index->header.total_file_nums)
			{
				*file_no = index->header.recorded_file_nums;//此处可能有问题
				*sect_offset = 0;
				
				p_chn_info->start_time = time(NULL);
				p_chn_info->end_time = p_chn_info->start_time;
				p_chn_info->sect_nums = 0;
				p_chn_info->file_no = *file_no;
				
				index->header.recorded_file_nums++;
				
				//printf("*****chn%d:file_no=%d,sect_offset=%d,sect_nums=%d\n",chn,*file_no,*sect_offset,p_chn_info->sect_nums+1);
				
				memset(&s_u_info,0,sizeof(s_u_info));
				s_u_info.type = 0;
				s_u_info.start_time = p_chn_info->start_time;
				s_u_info.end_time = s_u_info.start_time;
				s_u_info.start_position = *sect_offset;
				s_u_info.end_position = s_u_info.start_position;
				
				position = seek_to_segment(index,&index->header,p_chn_info->file_no,p_chn_info->sect_nums);
				
				#ifdef DISK_NEW_MMAP_WAY
				fseek(index->index1,position,SEEK_SET);
				fwrite(&s_u_info,sizeof(s_u_info),1,index->index1);
				fflush(index->index1);
				fsync(fileno(index->index1));
				
				fseek(index->index2,position,SEEK_SET);
				fwrite(&s_u_info,sizeof(s_u_info),1,index->index2);
				fflush(index->index2);
				fsync(fileno(index->index2));
				#else
				memcpy(index->mapdst1+position,&s_u_info,sizeof(s_u_info));
				sync_partition_index(index->mapdst1+position,sizeof(s_u_info),MS_SYNC);
				
				memcpy(index->mapdst2+position,&s_u_info,sizeof(s_u_info));
				sync_partition_index(index->mapdst2+position,sizeof(s_u_info),MS_SYNC);
				#endif
				
				unlock_partition_index(index);
				
				return 1;
			}
		}
		
		unlock_partition_index(index);
		
		return 0;
	}
	
	return 0;
}

int get_first_full_file_end_time(partition_index *index,u32 *end_time)
{
	if(index->valid)
	{
		file_use_info f_u_info;
		long position;
		
		lock_partition_index(index);
		
		position = sizeof(index->header)+index->header.full_file_offset*sizeof(file_use_info);
		
		//memcpy(&f_u_info, index->mapdst1 + position, sizeof(f_u_info));
		memcpy(&f_u_info, index->cachedst + position, sizeof(f_u_info));
		
		unlock_partition_index(index);
		
		*end_time = f_u_info.end_time;
		
		return 1;
	}
	
	return 0;
}

int get_chn_next_segment_force(partition_index *index,int chn,int *file_no,int *sect_offset)
{
	if(index->valid)
	{
		file_use_info f_u_info;
		segment_use_info s_u_info;
		long position;
		
		lock_partition_index(index);
		
		position = sizeof(index->header)+index->header.full_file_offset*sizeof(file_use_info);
		
		//memcpy(&f_u_info, index->mapdst1+position, sizeof(f_u_info));
		memcpy(&f_u_info, index->cachedst+position, sizeof(f_u_info));
		
		index->header.full_file_offset = (index->header.full_file_offset+1)%index->header.total_file_nums;
		index->header.full_file_nums--;
		//index->header.recorded_file_nums--;
		//write_partition_index_header(index,&index->header);
		
		index->header.chn_info[chn+1].file_no = f_u_info.file_no;
		index->header.chn_info[chn+1].sect_nums = 0;
		index->header.chn_info[chn+1].start_time = time(NULL);
		index->header.chn_info[chn+1].end_time = index->header.chn_info[chn+1].start_time;
		write_partition_index_header(index,&index->header);
		*file_no = f_u_info.file_no;
		*sect_offset = 0;
		
		memset(&s_u_info,0,sizeof(s_u_info));
		s_u_info.type = 0;
		s_u_info.start_time = index->header.chn_info[chn+1].start_time;
		s_u_info.end_time = s_u_info.start_time;
		s_u_info.start_position = 0;
		s_u_info.end_position = s_u_info.start_position;	
		
		position = seek_to_segment(index,&index->header,f_u_info.file_no,0);
		
		#ifdef DISK_NEW_MMAP_WAY
		fseek(index->index1,position,SEEK_SET);
		fwrite(&s_u_info,sizeof(s_u_info),1,index->index1);
		fflush(index->index1);
		fsync(fileno(index->index1));
		
		fseek(index->index2,position,SEEK_SET);
		fwrite(&s_u_info,sizeof(s_u_info),1,index->index2);
		fflush(index->index2);
		fsync(fileno(index->index2));
		#else
		memcpy(index->mapdst1+position,&s_u_info,sizeof(s_u_info));
		sync_partition_index(index->mapdst1+position,sizeof(s_u_info),MS_SYNC);
		
		memcpy(index->mapdst2+position,&s_u_info,sizeof(s_u_info));
		sync_partition_index(index->mapdst2+position,sizeof(s_u_info),MS_SYNC);
		#endif
		
		unlock_partition_index(index);
		
		return 1;
	}
	
	return 0;
}

int update_chn_cur_segment(partition_index *index,int chn,segment_use_info *p_s_u_info,u8 finished)
{
	//printf("chn%d update_chn_cur_segment:hehe1\n",chn);
	/*
	unsigned char cur_atx_flag = tl_power_atx_check();
	if(cur_atx_flag == 0)
	{
		printf("chn%d update_chn_cur_segment:hehe1.1\n",chn);
		return 0;
	}
	*/
	//printf("chn%d update_chn_cur_segment:hehe1.2\n",chn);
	
	//lock_partition_index(index);
	
	if(index->valid)
	{
		chn_last_use_file *p_chn_info;
		long position;
		
		dbgprint("chn%d update_chn_cur_segment:hehe2\n",chn);
		
		lock_partition_index(index);
		
		dbgprint("chn%d update_chn_cur_segment:hehe2.1\n",chn);
		
		p_chn_info = &index->header.chn_info[chn + 1];
		p_chn_info->end_time = p_s_u_info->end_time;
		
		dbgprint("chn%d update_chn_cur_segment:hehe2.2\n",chn);
		
		if(p_s_u_info->type == 0)
		{
			printf("##################warning:update chn%d record type is 0\n",chn);
		}
		
		dbgprint("chn%d update_chn_cur_segment:file_no=%d,sect_nums=%d,hehe2.3\n",chn,p_chn_info->file_no,p_chn_info->sect_nums);
		
		position = seek_to_segment(index,&index->header,p_chn_info->file_no,p_chn_info->sect_nums);
		
		dbgprint("chn%d update_chn_cur_segment:position=%ld,length=%d,(%d,%d),hehe2.4\n",chn,position,index->length1,p_s_u_info->start_time,p_s_u_info->end_time);
		
		dbgprint("chn%d update_chn_cur_segment:hehe2.5\n",chn);
		
		#ifdef DISK_NEW_MMAP_WAY
		segment_use_info tmp_seg_info = *p_s_u_info;
		
		fseek(index->index1,position,SEEK_SET);
		fwrite(&tmp_seg_info,sizeof(tmp_seg_info),1,index->index1);
		fflush(index->index1);
		fsync(fileno(index->index1));
		
		fseek(index->index2,position,SEEK_SET);
		fwrite(&tmp_seg_info,sizeof(tmp_seg_info),1,index->index2);
		fflush(index->index2);
		fsync(fileno(index->index2));
		#else
		memcpy(index->mapdst1+position,p_s_u_info,sizeof(segment_use_info));
		sync_partition_index(index->mapdst1+position,sizeof(segment_use_info),MS_SYNC);
		
		memcpy(index->mapdst2+position,p_s_u_info,sizeof(segment_use_info));
		sync_partition_index(index->mapdst2+position,sizeof(segment_use_info),MS_SYNC);
		#endif
		
		dbgprint("chn%d update_chn_cur_segment:hehe3\n",chn);
		
		if(finished)//文件是否结束
		{
			//yaogang hdd
			//if(p_chn_info->sect_nums >= 255 || p_s_u_info->end_position > (SIZE_OF_FILE_VECTOR-2*SIZE_OF_RESERVED) || p_chn_info->end_time > (u32)time(NULL))
			if(p_chn_info->sect_nums >= (NFILE_PER_SECT-1) || p_s_u_info->end_position > (SIZE_OF_FILE_VECTOR-2*SIZE_OF_RESERVED) || p_chn_info->end_time > (u32)time(NULL))
			{
				dbgprint("hehehehe\n");
				
				file_use_info f_u_info;
				f_u_info.start_time = p_chn_info->start_time;
				f_u_info.end_time = p_chn_info->end_time;
				f_u_info.file_no = p_chn_info->file_no;
				f_u_info.busy = 0;
				f_u_info.chn_no = chn+1;
				f_u_info.sect_nums = p_chn_info->sect_nums;
				position = sizeof(index->header)+sizeof(file_use_info)*((index->header.full_file_offset+index->header.full_file_nums)%index->header.total_file_nums);
				
				#ifdef DISK_NEW_MMAP_WAY
				memcpy(index->cachedst + position, &f_u_info, sizeof(f_u_info));
				
				fseek(index->index1,position,SEEK_SET);
				fwrite(&f_u_info,sizeof(f_u_info),1,index->index1);
				fflush(index->index1);
				fsync(fileno(index->index1));
				
				fseek(index->index2,position,SEEK_SET);
				fwrite(&f_u_info,sizeof(f_u_info),1,index->index2);
				fflush(index->index2);
				fsync(fileno(index->index2));
				#else
				memcpy(index->mapdst1 + position, &f_u_info, sizeof(f_u_info));
				sync_partition_index(index->mapdst1 + position, sizeof(f_u_info), MS_SYNC);
				
				memcpy(index->mapdst2 + position, &f_u_info, sizeof(f_u_info));
				sync_partition_index(index->mapdst2 + position, sizeof(f_u_info), MS_SYNC);
				#endif
				
				index->header.full_file_nums++;
				p_chn_info->file_no = 0xffff;
				p_chn_info->start_time = 0;
				p_chn_info->end_time = 0;
				p_chn_info->sect_nums = 0;
			}
		}
		
		write_partition_index_header(index, &index->header);
		
		unlock_partition_index(index);
		
		//fileflush(index->index1);
		//fileflush(index->index2);
		
		dbgprint("chn%d update_chn_cur_segment:hehe4\n",chn);
		
		return 1;
	}
	
	//unlock_partition_index(index);
	
	dbgprint("chn%d update_chn_cur_segment:hehe5\n",chn);
	
	return 0;
}

int is_type_matching(u8 type,u8 mask)
{
	type = type & 0x0f;
	if(!type) return 0;
	
	mask = mask & 0x1f;
	if(!mask) return 0;
	
	if(mask & RECTYPE_MASK_ALL)
	{
		return 1;
	}
	else
	{
		//if(mask == (type & mask))
		if(type & mask)
		{
			return 1;
		}
	}
	
	return 0;
}

int search_rec_file(partition_index *index,search_param_t *search,recfileinfo_t *fileinfo_buf,int max_nums,u8 disk_no,u8 ptn_no)
{
	if(index->valid)
	{
		file_use_info f_u_info;
		segment_use_info s_u_info;
		chn_last_use_file *p_chn_info;
		long position;
		u16  i,j;
		int  ret = 0;
		u16 real_ch = 0;
		
		lock_partition_index(index);
		
		dbgprint("search_rec_file:disk_no=%d,ptn_no=%d,full=%d\n",disk_no,ptn_no,index->header.full_file_nums);
		for(i=0;i<index->header.full_file_nums;i++)
		{
			position = sizeof(index->header)+((index->header.full_file_offset+i)%index->header.total_file_nums)*sizeof(file_use_info);
			//memcpy(&f_u_info,index->mapdst1+position,sizeof(f_u_info));
			memcpy(&f_u_info,index->cachedst+position,sizeof(f_u_info));
			
			dbgprint("hehe1,chn=%d,i=%d,position=%ld,ret=%d,disk_no=%d,ptn_no=%d,index=0x%x\n",search->channel_no,i,position,ret,disk_no,ptn_no,(int)index);
			/*test begin
			if(f_u_info.chn_no == 1)
			{
				dbgprint("hehe1.1,file_no=%d,chn=%d,sect_nums=%d,start_time=%d,end_time=%d\n",f_u_info.file_no,f_u_info.chn_no,f_u_info.sect_nums+1,f_u_info.start_time,f_u_info.end_time);
				if(f_u_info.end_time <= f_u_info.start_time)
				{
					dbgprint("!!!!!!!!!!!!!!!!end<=start(%d,%d,%d,%d,%d)!!!!!!!!!!!!!!!!\n",f_u_info.chn_no,f_u_info.start_time,f_u_info.end_time,f_u_info.file_no,f_u_info.sect_nums);
				}
			}
			test end*/
#if 1
			//if(f_u_info.chn_no != search->channel_no+1 || f_u_info.start_time >= search->end_time || f_u_info.end_time <= search->start_time)
			//cyl modified
			
			if((!(1<<(f_u_info.chn_no - 1)&search->channel_no)) || f_u_info.start_time >= search->end_time || f_u_info.end_time <= search->start_time/* || search->end_time < search->start_time*/)
			{
				continue;
			}
#endif
			/*test begin
			if(f_u_info.chn_no == 1) 
				dbgprint("hehe1.2,file_no=%d,chn=%d,sect_nums=%d,start_time=%d,end_time=%d\n",f_u_info.file_no,f_u_info.chn_no,f_u_info.sect_nums+1,f_u_info.start_time,f_u_info.end_time);
			int cc = 0;
			
			dbgprint("hehe2,chn=%d,i=%d,position=%ld,ret=%d,disk_no=%d,ptn_no=%d,index=0x%x\n",search->channel_no,i,position,ret,disk_no,ptn_no,(int)index);
			test end*/
			
#if 1//csp debug
			for(j=0;j<f_u_info.sect_nums+1;j++)
			{
				if(f_u_info.file_no == 0xffff)//csp  20110301
				{
					printf("warning:error^^^^^^\n");
					break;
				}
				
				position = seek_to_segment(index,&index->header,f_u_info.file_no,j);
				//memcpy(&s_u_info,index->mapdst1+position,sizeof(s_u_info));
				fseek(index->index1, position, SEEK_SET);
				fread(&s_u_info, sizeof(s_u_info), 1, index->index1);
				//fseek(index->index1,0,SEEK_SET);
				
				//printf("position=%d,(%d,%d,%d)\n",position,index->header.total_file_nums,f_u_info.file_no,j);
				
				#ifdef DELETE_FILE
				//printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~s_u_info.state:%d\n", s_u_info.state);
				if(s_u_info.state == 10)
				{
					continue;
				}
				#endif
				
#if 1//csp debug
				if(s_u_info.start_time >= search->end_time)
				{
					/*test begin
					if(f_u_info.chn_no == 1 && f_u_info.file_no == 0)
					{
						dbgprint("break1\n");
					}
					test end*/
					break;
				}

				
				if(s_u_info.end_time <= search->start_time)
				{
					/*test begin
					if(f_u_info.chn_no == 1 && f_u_info.file_no == 0)
					{
						dbgprint("break2,search->start_time=%d,s_u_info.start_time=%d,s_u_info.end_time=%d,pos:(%d,%d)\n",search->start_time,s_u_info.start_time,s_u_info.end_time,s_u_info.start_position,s_u_info.end_position);
					}
					test end*/
					continue;
				}
				/*test begin
				else
				{
					dbgprint("no break,search->start_time=%d,s_u_info.start_time=%d,s_u_info.end_time=%d,pos:(%d,%d)\n",search->start_time,s_u_info.start_time,s_u_info.end_time,s_u_info.start_position,s_u_info.end_position);
				}test end*/
				
				//if(s_u_info.end_time <= s_u_info.start_time/* || s_u_info.end_position <= s_u_info.start_position*/)
				if(s_u_info.end_time < s_u_info.start_time/* || s_u_info.end_position <= s_u_info.start_position*/)
				{
					/*test beginif(f_u_info.chn_no == 1 && f_u_info.file_no == 0)
					{
						dbgprint("break3\n");
					}test end*/
					continue;
				}
				
				if(s_u_info.end_position <= s_u_info.start_position)
				{
					/*test beginif(f_u_info.chn_no == 1 && f_u_info.file_no == 0)
					{
						dbgprint("break4\n");
					}test end*/
					continue;
				}
#endif
				
	#if 1//csp debug
				if(is_type_matching(s_u_info.type,search->type))
				{
					if(ret+1>max_nums)
					{
						unlock_partition_index(index);
						//return -1;
						return ret;
					}
					fileinfo_buf[ret].disk_no = disk_no;
					fileinfo_buf[ret].ptn_no = ptn_no;
					//fileinfo_buf[ret].channel_no = search->channel_no;//cyl
					fileinfo_buf[ret].channel_no = f_u_info.chn_no;
					fileinfo_buf[ret].start_time = s_u_info.start_time;
					fileinfo_buf[ret].end_time = s_u_info.end_time;
					fileinfo_buf[ret].file_no = f_u_info.file_no;
					fileinfo_buf[ret].offset = s_u_info.start_position;
					fileinfo_buf[ret].size = s_u_info.end_position - s_u_info.start_position;
					fileinfo_buf[ret].type = s_u_info.type;
					fileinfo_buf[ret].image_format = s_u_info.image_format;
					fileinfo_buf[ret].stream_flag = s_u_info.stream_flag;
					++ret;
					
					//++cc;
				}
	#endif
			}
#endif
			
			//if(f_u_info.chn_no == 1) dbgprint("add %d files\n",cc);
			
			//dbgprint("hehe3,chn=%d,i=%d,position=%ld,ret=%d,disk_no=%d,ptn_no=%d,index=0x%x\n",search->channel_no,i,position,ret,disk_no,ptn_no,(int)index);
		}
		
	#if 1//csp debug
		real_ch=1;
		for(real_ch=1;real_ch<=64;real_ch++)
		{
			if(!(search->channel_no&(1<<(real_ch-1))))
			{
				//printf("real_ch :%d \r\n",real_ch);
				continue;
			}
			//printf("real_ch :%d, mexchn :%d\r\n",real_ch,(search->channel_no&(1<<(real_ch-1))));
			
			p_chn_info = &index->header.chn_info[real_ch];
			
			if(p_chn_info->file_no < index->header.total_file_nums)
			{
				//printf("no full file=%d,sect_nums=%d\n",p_chn_info->file_no,p_chn_info->sect_nums+1);
				//printf("chn_info:(%d,%d),search:(%d,%d)\n",p_chn_info->start_time,p_chn_info->end_time,search->start_time,search->end_time);
				if(!(p_chn_info->start_time >= search->end_time || p_chn_info->end_time <= search->start_time))
				{
					//printf("haha1\n");
					
					for(j=0;j<p_chn_info->sect_nums+1;j++)
					{
						position = seek_to_segment(index,&index->header,p_chn_info->file_no,j);
						//memcpy(&s_u_info,index->mapdst1+position,sizeof(s_u_info));
						fseek(index->index1, position, SEEK_SET);
						fread(&s_u_info, sizeof(s_u_info), 1, index->index1);
						//fseek(index->index1,0,SEEK_SET);
						
						//printf("haha2,position=%ld,(%d,%d)\n",position,s_u_info.start_time,s_u_info.end_time);
						#ifdef DELETE_FILE
						if(s_u_info.state == 10)
						{
							continue;
						}
						#endif
						
						//csp modify 20140822
						//if(search->end_time < search->start_time)
						//{
						//	break;
						//	//continue;
						//}
						
						if(s_u_info.start_time >= search->end_time)
						{
							break;
							//continue;
						}
						
						//printf("haha3\n");
						
						if(s_u_info.end_time <= search->start_time)
						{
							continue;
						}
						
						//printf("haha4\n");
						
						//if(s_u_info.end_time <= s_u_info.start_time || s_u_info.end_position <= s_u_info.start_position)
						if(s_u_info.end_time < s_u_info.start_time || s_u_info.end_position <= s_u_info.start_position)
						{
							continue;
						}
						//printf("haha5\n");
						
						if(is_type_matching(s_u_info.type,search->type))
						{
							//printf("haha6\n");
							
							if(ret+1>max_nums)
							{
								unlock_partition_index(index);
								//return -1;
								return ret;
							}
							fileinfo_buf[ret].disk_no = disk_no;
							fileinfo_buf[ret].ptn_no = ptn_no;
							//fileinfo_buf[ret].channel_no = search->channel_no;
							fileinfo_buf[ret].channel_no = real_ch;
							fileinfo_buf[ret].start_time = s_u_info.start_time;
							fileinfo_buf[ret].end_time = s_u_info.end_time;
							fileinfo_buf[ret].file_no = p_chn_info->file_no;
							fileinfo_buf[ret].offset = s_u_info.start_position;
							fileinfo_buf[ret].size = s_u_info.end_position - s_u_info.start_position;
							fileinfo_buf[ret].type = s_u_info.type;
							fileinfo_buf[ret].image_format = s_u_info.image_format;
							fileinfo_buf[ret].stream_flag = s_u_info.stream_flag;
							++ret;
						}
					}
					
					//printf("haha7\n");
				}
			}
		}
	#endif
		
		unlock_partition_index(index);
		
		//printf("********************search_rec_file:disk_no=%d,ptn_no=%d,total=%d\n",disk_no,ptn_no,ret);
		return ret;
	}
	
	return 0;
}

//yaogang modify 20141225
int init_partition_pic_index(partition_index* index)
{
	int i, j;
	FILE *pf = NULL;
	file_pic_use_info finfo;
	u8 *p = (u8*)&index->pic_header;

	if(index->valid)
	{
		lock_partition_index(index);
		memset(&index->pic_header, 0, sizeof(index->pic_header));
		index->pic_header.file_cur_no = 	0;
		index->pic_header.file_nums = 	PIC_FILE_NUMS -PRE_RECSNAP_FILES;//预录使用2个
		index->pic_header.start_sec = 	0;
		index->pic_header.start_usec = 	0;
		index->pic_header.end_sec = 		0;
		index->pic_header.end_usec = 	0;

		index->pic_header.verify = 0;
		for (i=0; i<sizeof(index->pic_header) -4; i++)
		{
			index->pic_header.verify += p[i];
		}

		for (j = 0; j<2; j++)
		{
			
			pf = j ? index->pic_idx_bk :  index->pic_idx;
			
			if ( fseek(pf, 0, SEEK_SET) )
			{
				//perror("init_partition_pic_index fseek pic_idx failed");
				printf("%s fseek pic_idx %d failed, %s\n", __func__, j, strerror(errno));
				unlock_partition_index(index);
				return 1;
			}
			if (fwrite(p, sizeof(index->pic_header), 1, pf) != 1)
			{
				//perror("init_partition_pic_index fwrite 1 failed");
				printf("%s fwrite pic_header %d failed, %s\n", __func__, j, strerror(errno));
				unlock_partition_index(index);
				return 1;
			}

			memset(&finfo, 0, sizeof(finfo));
			finfo.next_pic_pos= 0xffffffff;
			
			for (i=0; i<PIC_FILE_NUMS-PRE_RECSNAP_FILES; i++)
			{
				finfo.file_no = i;
				if (fwrite(&finfo, sizeof(file_pic_use_info), 1, pf) != 1)
				{
					//perror("init_partition_pic_index fwrite 2 failed");
					printf("%s fwrite file_pic_use_info %d,%d failed, %s\n", __func__, j, i, strerror(errno));
					unlock_partition_index(index);
					return 1;
				}
			}
			
			fileflush(pf);
			
		}
		unlock_partition_index(index);
	}
	return 0;
}

//得到索引文件中指定文件容器的信息所在位置
u32 get_index_file_pos(u16 file_no)
{
	return sizeof(partition_pic_index_header) + sizeof(file_pic_use_info) * file_no;
}

u32 get_index_pic_pos(u16 file_no, u32 pic_no)
{
	return sizeof(partition_pic_index_header) \
			+ sizeof(file_pic_use_info) * (PIC_FILE_NUMS - PRE_RECSNAP_FILES) \
			+ sizeof(pic_use_info) * PIC_NUMS_PER_FILE * file_no \
			+ sizeof(pic_use_info) * pic_no;
}

u32 write_pic_to_file(partition_index* index, u16 file_no, u32 next_pic_pos, void *pdata, u32 data_size)
{
	FILE *pf = NULL;
	char filename[64];
	int ret;

	sprintf(filename, "%s/pic%05d.ifv", index->mountpath, file_no);
	//printf("%s filename: %s\n", __FUNCTION__, filename);

	pf = fopen(filename, "r+");
	if(pf == NULL)
	{
		printf("%s fopen %s failed\n", __FUNCTION__, filename);
		
		return 1;
	}

	ret = fseek(pf, next_pic_pos, SEEK_SET);
	if (ret)
	{
		printf("%s: %s fseek to %d failed, %s\n", __FUNCTION__, filename, next_pic_pos, strerror(errno));
		fclose(pf);
		return 1;
	}

	ret = fwrite(pdata, data_size, 1, pf);
	if (ret != 1)
	{
		printf("%s: %s fwrite failed, %s\n", __FUNCTION__, filename,  strerror(errno));
		fclose(pf);
		return 1;
	}

	ret = fclose(pf);
	if (ret)
	{
		printf("%s: %s fclose failed, %s\n", __FUNCTION__, filename,  strerror(errno));
		perror(NULL);
		return 1;
	}

	return 0;
}

int refresh_index_header_unlock(partition_index* index, partition_pic_index_header *pheader)
{
	int ret;

	if (NULL == index || NULL == pheader)
	{
		printf("%s: param invalid\n", __FUNCTION__);
		return 1;
	}
/*
	printf("%s: \n", __func__);
	printf("\t file_nums: %d\n", pheader->file_nums);
	printf("\t file_cur_no: %d\n", pheader->file_cur_no);
	printf("\t partition_pic_nums: %d\n", pheader->partition_pic_nums);
	printf("\t start_sec: %d\n", pheader->start_sec);
	printf("\t start_usec: %d\n", pheader->start_usec);
	printf("\t end_sec: %d\n", pheader->end_sec);
	printf("\t end_usec: %d\n", pheader->end_usec);
*/	
	if(index->valid)
	{
		//lock_partition_index(index);
		ret = fseek(index->pic_idx, 0, SEEK_SET);
		if (ret)
		{
			printf("%s: index fseek to %d failed\n", __FUNCTION__, 0);
			perror(NULL);
			//unlock_partition_index(index);
			return 1;
		}

		ret = fwrite(pheader, sizeof(partition_pic_index_header), 1, index->pic_idx);
		if (ret != 1)
		{
			printf("%s: index fwrite failed\n", __FUNCTION__);
			perror(NULL);
			//unlock_partition_index(index);
			return 1;
		}
		
		fileflush(index->pic_idx);

		ret = fseek(index->pic_idx_bk, 0, SEEK_SET);
		if (ret)
		{
			printf("%s: index_bk fseek to %d failed\n", __FUNCTION__, 0);
			perror(NULL);
			//unlock_partition_index(index);
			return 1;
		}

		ret = fwrite(pheader, sizeof(partition_pic_index_header), 1, index->pic_idx_bk);
		if (ret != 1)
		{
			printf("%s: index_bk fwrite failed\n", __FUNCTION__);
			perror(NULL);
			//unlock_partition_index(index);
			return 1;
		}
		
		fileflush(index->pic_idx_bk);
		
		//unlock_partition_index(index);
	}
	//fsync(fileno(index->pic_idx));

	return 0;
}

int get_index_fileinfo(partition_index* index, u16 file_no, file_pic_use_info *finfo)
{
	int ret;
	u32 pos;

	if (NULL == index || NULL == finfo)
	{
		printf("%s: param invalid\n", __FUNCTION__);
		return 1;
	}
	
	pos = get_index_file_pos(file_no);

	if(index->valid)
	{
		lock_partition_index(index);
		ret = fseek(index->pic_idx, pos, SEEK_SET);
		if (ret)
		{
			printf("%s: index fseek to %d failed\n", __FUNCTION__, pos);
			perror(NULL);
			unlock_partition_index(index);
			return 1;
		}
		
		ret = fread(finfo, sizeof(file_pic_use_info), 1, index->pic_idx);
		if (ret != 1)
		{
			printf("%s: index fread failed\n", __FUNCTION__);
			perror(NULL);
			unlock_partition_index(index);
			return 1;
		}
		unlock_partition_index(index);
	}
	return 0;
}

int get_index_picinfo(partition_index* index, u16 file_no, u32 pic_no, pic_use_info *pinfo)
{
	int ret;
	u32 pos;

	if (NULL == index || NULL == pinfo)
	{
		printf("%s: param invalid\n", __FUNCTION__);
		return 1;
	}
	
	pos = get_index_pic_pos(file_no, pic_no);

	if(index->valid)
	{
		lock_partition_index(index);
		ret = fseek(index->pic_idx, pos, SEEK_SET);
		if (ret)
		{
			printf("%s: index fseek to %d failed\n", __FUNCTION__, pos);
			perror(NULL);
			unlock_partition_index(index);
			return 1;
		}
		
		ret = fread(pinfo, sizeof(pic_use_info), 1, index->pic_idx);
		if (ret != 1)
		{
			printf("%s: index fread failed\n", __FUNCTION__);
			perror(NULL);
			unlock_partition_index(index);
			return 1;
		}
		unlock_partition_index(index);
	}
	return 0;
}

int refresh_index_fileinfo(partition_index* index, file_pic_use_info *finfo)
{
	int ret;
	u32 pos;

	if (NULL == index || NULL == finfo)
	{
		printf("%s: param invalid\n", __FUNCTION__);
		return 1;
	}

	pos = get_index_file_pos(finfo->file_no);
	if(index->valid)
	{
		lock_partition_index(index);
		ret = fseek(index->pic_idx, pos, SEEK_SET);
		if (ret)
		{
			printf("%s: index fseek to %d failed, %s\n", __FUNCTION__, pos, strerror(errno));
			unlock_partition_index(index);
			return 1;
		}

		ret = fwrite(finfo, sizeof(file_pic_use_info), 1, index->pic_idx);
		if (ret != 1)
		{
			printf("%s: index fwrite failed, %s\n", __FUNCTION__, strerror(errno));
			unlock_partition_index(index);
			return 1;
		}
		fileflush(index->pic_idx);

		ret = fseek(index->pic_idx_bk, pos, SEEK_SET);
		if (ret)
		{
			printf("%s: pic_idx_bk fseek to %d failed, %s\n", __FUNCTION__, pos, strerror(errno));
			unlock_partition_index(index);
			return 1;
		}

		ret = fwrite(finfo, sizeof(file_pic_use_info), 1, index->pic_idx_bk);
		if (ret != 1)
		{
			printf("%s: pic_idx_bk fwrite failed, %s\n", __FUNCTION__, strerror(errno));
			
			unlock_partition_index(index);
			return 1;
		}
		fileflush(index->pic_idx_bk);
		
		unlock_partition_index(index);
	}
	//fsync(fileno(index->pic_idx));

	return 0;
}

int refresh_index_picinfo(partition_index* index, pic_use_info* pinfo)
{
	int ret;
	u32 pos;

	if (NULL == index || NULL == pinfo)
	{
		printf("%s: param invalid\n", __FUNCTION__);
		return 1;
	}

	pos = get_index_pic_pos(pinfo->file_no, pinfo->pic_no);

	if(index->valid)
	{
		lock_partition_index(index);
		ret = fseek(index->pic_idx, pos, SEEK_SET);
		if (ret)
		{
			printf("%s: index fseek to %d failed, %s\n", __FUNCTION__, pos, strerror(errno));
			return 1;
		}

		ret = fwrite(pinfo, sizeof(pic_use_info), 1, index->pic_idx);
		if (ret != 1)
		{
			printf("%s: index fwrite failed, %s\n", __FUNCTION__, strerror(errno));
			return 1;
		}
		fileflush(index->pic_idx);

		ret = fseek(index->pic_idx_bk, pos, SEEK_SET);
		if (ret)
		{
			printf("%s: pic_idx_bk fseek to %d failed, %s\n", __FUNCTION__, pos, strerror(errno));
			return 1;
		}

		ret = fwrite(pinfo, sizeof(pic_use_info), 1, index->pic_idx_bk);
		if (ret != 1)
		{
			printf("%s: pic_idx_bk fwrite failed, %s\n", __FUNCTION__, strerror(errno));
			return 1;
		}
		fileflush(index->pic_idx_bk);
		
		unlock_partition_index(index);
	}
	//fsync(fileno(index->pic_idx));

	return 0;
}
int read_snap_file(partition_index *index, u8 *pbuf, u16 nFileNo, u32 nOffset, u32 nSize)
{
	FILE *pf = NULL;
	char filename[64];
	int ret;

	//check_file_param(index, buf, nFileNo, nOffset, nSize);
	
	sprintf(filename,"%s/pic%05d.ifv", index->mountpath, nFileNo);
	printf("%s filename: %s, offset: %d, size: %d\n", __func__, filename, nOffset, nSize);

	pf = fopen(filename, "rb");
	if(pf == NULL)
	{
		printf("%s fopen %s failed\n", __FUNCTION__, filename);
		perror(NULL);
		return 1;
	}

	ret = fseek(pf, nOffset, SEEK_SET);
	if (ret)
	{
		printf("%s: %s fseek to %d failed\n", __FUNCTION__, filename, nOffset);
		perror(NULL);
		return 1;
	}

	ret = fread(pbuf, nSize, 1, pf);
	if (ret != 1)
	{
		printf("%s: %s fwrite failed\n", __FUNCTION__, filename);
		perror(NULL);
		return 1;
	}

	ret = fclose(pf);
	if (ret)
	{
		printf("%s: %s fclose failed\n", __FUNCTION__, filename);
		perror(NULL);
		return 1;
	}

	return 0;
}

int write_snapshot_to_partition(partition_index* index, u8 chn, u8 pic_type, time_t tv_sec, suseconds_t tv_usec, void *pdata, u32 data_size, u32 width, u32 height)
{
	int ret, i;
	partition_pic_index_header * phead = &index->pic_header;
	partition_pic_index_header header;
	u32 verify;
	
	if (index->valid != 1)
	{
		printf("%s: partition invalid, mountpoint: %s\n", __FUNCTION__, index->mountpath);
		return 1;//分区无效
	}
	struct tm tm;
	gmtime_r(&tv_sec, &tm);

	//printf("%s chn: %d, time: %02d:%02d:%02d\n", __func__, chn, tm.tm_hour, tm.tm_min, tm.tm_sec);
	
	//printf("%s: chn: %d, pic_type: 0x%x\n", __FUNCTION__, chn, pic_type);

	if (phead->file_cur_no > phead->file_nums)
	{
		printf("%s: partition:%s pic file_cur_no: %d, out of range\n", __FUNCTION__, index->mountpath, phead->file_cur_no);
		return 1;//分区文件容器用完，感觉不太可能运行到这
	}

	file_pic_use_info finfo;
	ret = get_index_fileinfo(index, phead->file_cur_no, &finfo);
	if (ret)
	{
		printf("%s: get_index_fileinfo failed\n", __FUNCTION__);
		return 1;
	}
	
	u16 file_cur_no = phead->file_cur_no;
	u32 next_pic_pos = finfo.next_pic_pos;

	if (file_cur_no > phead->file_nums -1)
	{
		printf("%s file_cur_no: %d, chn: %d, time: %02d:%02d:%02d\n", \
			__func__, file_cur_no, chn, tm.tm_hour, tm.tm_min, tm.tm_sec);
		return 2;//切换分区
	}

	//printf("%s: fileno: %d, status: %d, file_pic_nums: %d, next_pic_pos: %d, pic_size: %d\n", 
	//	__FUNCTION__, file_cur_no, finfo.status, finfo.file_pic_nums, next_pic_pos, data_size);
	//printf("%s: next_pic_pos: %d, pic_size: %d\n", __FUNCTION__, next_pic_pos, data_size);

	switch (finfo.status)
	{
		case 0: //未使用
		{
			//printf("%s a new file\n", __FUNCTION__);
			finfo.status = 1;
			next_pic_pos = 0;//初始化时写为0xffff ffff
		}		//继续向下运行
		case 1: //正在使用
		{
			if (next_pic_pos + data_size > SIZE_OF_FILE_VECTOR)//文件容器空间不足
			{
				printf("%s file%d is full\n", __FUNCTION__, file_cur_no);
				if (file_cur_no + 1 == phead->file_nums)//该分区文件容器已经写完
				{
					printf("%s partition: %s full\n", __FUNCTION__, index->mountpath);
					
					//更新索引头
					lock_partition_index(index);
					
					phead->file_cur_no = phead->file_nums;//标示该分区文件容器已经用完
					
					u8 *p = (u8 *)phead;
					verify = 0;
					for (i=0; i<sizeof(partition_pic_index_header)-4; i++)
					{
						verify += p[i];
					}
					phead->verify = verify;
					ret = refresh_index_header_unlock(index, phead);
					if (ret)
					{
						printf("%s: refresh_index_header failed 1\n", __FUNCTION__);
						//exit(0);
						return 1;
					}
					
					unlock_partition_index(index);
					
					return 2;//切换分区
				}
				//切换文件容器
				//更新当前文件容器信息
				finfo.status = 2;//已写满
				ret = refresh_index_fileinfo(index, &finfo);
				if (ret)
				{
					printf("%s: refresh_index_file failed\n", __FUNCTION__);
					//exit(0);
					return 1;
				}
				
				file_cur_no++;
				next_pic_pos = 0;
				
				//更新索引头
				lock_partition_index(index);
				memcpy(&header, phead, sizeof(header));
				
				header.file_cur_no = file_cur_no;
				
				u8 *p = (u8 *)&header;
				verify = 0;
				for (i=0; i<sizeof(header)-4; i++)
				{
					verify += p[i];
				}
				header.verify = verify;
				ret = refresh_index_header_unlock(index, &header);
				if (ret)
				{
					printf("%s: refresh_index_header failed\n", __FUNCTION__);
					//exit(0);
					return 1;
				}
				
				memcpy(phead, &header, sizeof(header));
				unlock_partition_index(index);
				
				ret = get_index_fileinfo(index, file_cur_no, &finfo);
				if (ret)
				{
					printf("%s: get_index_fileinfo failed\n", __FUNCTION__);
					return 1;
				}
			}
			
			ret = write_pic_to_file(index, file_cur_no, next_pic_pos, pdata, data_size);
			if (ret)
			{
				printf("%s: write_pic_to_file failed, file_no: %d, pos: %d, size: %d\n", __FUNCTION__, file_cur_no, next_pic_pos, data_size);
				//exit(0);
				return 1;
			}
			else
			{
				//更新图片索引
				pic_use_info pic;
				
				pic.chn = chn;
				pic.pic_type = pic_type;
				pic.pic_no = finfo.file_pic_nums;
				pic.pic_size = data_size;
				pic.width = width;
				pic.height = height;
				pic.file_no = file_cur_no;
				pic.file_offset = next_pic_pos;
				pic.tv_sec = tv_sec;
				pic.tv_usec = tv_usec;
				ret = refresh_index_picinfo(index, &pic);
				if (ret)
				{
					printf("%s: refresh_index_file failed\n", __FUNCTION__);
					//exit(0);
					return 1;
				}
				
				//更新文件索引
				finfo.status = 1;
				finfo.pic_type_mask |= pic_type;
				finfo.chn_mask |= 1<<chn;
				finfo.next_pic_pos = next_pic_pos+data_size;
				finfo.file_pic_nums++;
				finfo.end_sec = tv_sec;
				finfo.end_usec = tv_usec;
				if (finfo.start_sec == 0)
				{
					finfo.start_sec = tv_sec;
					finfo.start_usec = tv_usec;
				}
				ret = refresh_index_fileinfo(index, &finfo);
				if (ret)
				{
					printf("%s: refresh_index_file failed\n", __FUNCTION__);
					//exit(0);
					return 1;
				}
				
				//更新索引头
				lock_partition_index(index);
				memcpy(&header, phead, sizeof(header));
				
				header.file_cur_no = file_cur_no;
				header.partition_pic_nums++;
				header.end_sec = tv_sec;
				header.end_usec = tv_usec;
				if (header.start_sec == 0)
				{
					header.start_sec = tv_sec;
					header.start_usec = tv_usec;
				}
				u8 *p = (u8 *)&header;
				int i;
				
				verify = 0;
				for (i=0; i<sizeof(header)-4; i++)
				{
					verify += p[i];
				}
				header.verify = verify;
				//printf("%s verify: 0x%x\n", __func__, verify);
				
				ret = refresh_index_header_unlock(index, &header);
				if (ret)
				{
					printf("%s: refresh_index_header failed\n", __FUNCTION__);
					//exit(0);
					return 1;
				}
				memcpy(phead, &header, sizeof(header));
				unlock_partition_index(index);
			}
		}break;
		case 2: //已写满
		{
			printf("%s finfo.status: %d, impossible\n", __FUNCTION__, finfo.status);
		}break;
		default:
			printf("%s finfo.status: %d, invalid\n", __FUNCTION__, finfo.status);
	}

	return 0;
}

int search_rec_snap(partition_index *index,search_param_t *search,recsnapinfo_t *snapinfo_buf,int max_nums,u8 disk_no,u8 ptn_no)
{
	file_pic_use_info finfo;
	pic_use_info pinfo;
	u16 loopfiles;
	u32 looppics;
	u32 i, j, cnt;
	int ret;

	//printf("%s: search typemask: 0x%x, chnmask: 0x%x\n", __FUNCTION__, search->type, search->channel_no);
	//printf("%s: starttime: %d, endtime: %d\n", __FUNCTION__, search->start_time, search->end_time);

	cnt = 0;
	if(index->valid)
	{
		//当前保存有图片的文件容器数目
		loopfiles = index->pic_header.file_cur_no+1;
												
		//printf("search_rec_snap:disk_no=%d,ptn_no=%d, record files=%d\n", disk_no, ptn_no, loopfiles);
		for(i=0; i<loopfiles; i++)
		{
			ret = get_index_fileinfo(index, i, &finfo);
			if (ret)
			{
				printf("%s: get_index_fileinfo failed\n", __FUNCTION__);
				return -1;
			}
			//时间比较
			//printf("%s: file%d starttime: %d, endtime: %d\n", __FUNCTION__, i, finfo.start_sec, finfo.end_sec);
			if ((finfo.start_sec > search->end_time) || (finfo.end_sec < search->start_time))
			{
				//printf("%s: file%d time not in search range\n", __FUNCTION__, i);
				continue;
			}
			//printf("%s:yg 1\n", __FUNCTION__);
			//事件类型比较
			//printf("%s: file%d type 0x%x\n", __FUNCTION__, i, finfo.pic_type_mask);
			if ((search->type & finfo.pic_type_mask) == 0)
			{
				//printf("%s: file%d type mask not match\n", __FUNCTION__, i);
				continue;
			}
			//printf("%s:yg 2\n", __FUNCTION__);
			//通道比较
			//printf("%s: file%d chn mask: 0x%x\n", __FUNCTION__, i, finfo.chn_mask);
			if ((search->channel_no & finfo.chn_mask) == 0)
			{
				//printf("%s: file%d chn mask not match\n", __FUNCTION__, i);
				continue;
			}
			//printf("%s:yg 3\n", __FUNCTION__);
			
			looppics = finfo.file_pic_nums;
			for (j=0; j<looppics; j++)
			{
				ret = get_index_picinfo(index, i, j, &pinfo);
				if (ret)
				{
					printf("%s: get_index_fileinfo failed\n", __FUNCTION__);
					return -1;
				}
				//printf("%s:yg 4\n", __FUNCTION__);
				//时间比较
				//printf("%s: pic%d time %d\n", __FUNCTION__, j, pinfo.tv_sec);
				if ((pinfo.tv_sec > search->end_time) || (pinfo.tv_sec < search->start_time))
				{
					//printf("%s: pic%d time not in search range\n", __FUNCTION__, j);
					continue;
				}
				//printf("%s:yg 5\n", __FUNCTION__);
				//事件类型比较
				//printf("%s: pic%d type 0x%x\n", __FUNCTION__, j, pinfo.pic_type);
				if ((search->type & pinfo.pic_type) == 0)
				{
					//printf("%s: pic type  not match\n", __FUNCTION__);
					continue;
				}
				//printf("%s:yg 6\n", __FUNCTION__);
				//通道比较
				//printf("%s: pic%d chn %d\n", __FUNCTION__, j, pinfo.chn);
				if ((search->channel_no & (1<<pinfo.chn)) == 0)
				{
					//printf("%s: pic chn mask not match\n", __FUNCTION__);
					continue;
				}
				//printf("%s:yg 7\n", __FUNCTION__);
				//find it
				if(cnt+1 > max_nums)
				{
					printf("%s: cnt+1 > max_nums\n", __FUNCTION__);
					return cnt;
				}
				//printf("%s:yg 8\n", __FUNCTION__);
				snapinfo_buf[cnt].disk_no = disk_no;
				snapinfo_buf[cnt].ptn_no = ptn_no;
				//fileinfo_buf[ret].channel_no = search->channel_no;
				snapinfo_buf[cnt].channel_no = pinfo.chn;
				snapinfo_buf[cnt].start_time = pinfo.tv_sec;
				snapinfo_buf[cnt].start_timeus = pinfo.tv_usec;
				snapinfo_buf[cnt].file_no = pinfo.file_no;
				snapinfo_buf[cnt].offset = pinfo.file_offset;
				snapinfo_buf[cnt].size = pinfo.pic_size;
				snapinfo_buf[cnt].type = pinfo.pic_type;
				snapinfo_buf[cnt].snap_format = 0;
				snapinfo_buf[cnt].width = pinfo.width;
				snapinfo_buf[cnt].height = pinfo.height;
				cnt++;
			}
			
		}
	}

	return cnt;
}


