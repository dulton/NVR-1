#include "common.h"
#include "custommp4.h"
#include "isoformat.h"

unsigned char buf[65536*4*2];

void testcustommp4()
{
/*
	custommp4_t *file1;
	custommp4_t *file2;
	u8 media_type,key;
	u64 pts;//wrchen 081226
	u32 start_time;
	
	int i = 0;
	int j = 0;
	
	int width;
	int height;
	
	int realsize = 1;
	
	while(1)
	{
		if(i % 1 == 0)
		{
			j = i / 1;
			printf("\n%dst:\n",j);
			system("free");
			printf("\n");
		}
		i++;
		
		file1 = custommp4_open("mysrc.ifv",O_R,0);
		if(file1 == NULL)
		{
			printf("testcustommp4:open failed 1\n");
			exit(-1);
		}
		file2 = custommp4_open("mydst.ifv",O_W_CREAT,0);
		if(file2 == NULL)
		{
			printf("testcustommp4:open failed 2\n");
			exit(-1);
		}
		width  = custommp4_video_width(file1);
		height = custommp4_video_height(file1);
		custommp4_set_video(file2,1000,(u16)width,(u16)height,25,512*1024,str2uint("XVID"),0x18);
		
		while(1)
		{
			realsize = custommp4_read_one_media_frame(file1,buf,sizeof(buf),&start_time,&key,&pts,&media_type);
			if(realsize <= 0)
			{
				printf("testcustommp4:read over\n");
				break;
			}
			custommp4_write_video_frame(file2,buf,realsize,start_time,key,NULL);
		}
		custommp4_close(file1);
		custommp4_close(file2);
	}
*/
}

/*int custommp4_to_iflymp4_1(int argc, char **argv)
{
	int i;
	
	custommp4_t *file1;
	iflymp4_t *file2;
	
	int width;
	int height;
	
	int vlen;
	int alen;
	
	int vsize;
	int vtimestamp = 0;
	
	u32 start_time;
	u8  key;
	
	if(argc<4)
	{
		printf("usage:test infilename offset outfilename\n");
		exit(-1);
	}
	
	file1 = custommp4_open(argv[1],O_R,atoi(argv[2]));
	file2 = iflymp4_open(argv[3],0,1,0);
	if(file1 == NULL || file2 == NULL)
	{
		printf("open file failed\n");
		exit(-1);
	}
	
	width  = custommp4_video_width(file1);
	height = custommp4_video_height(file1);
	
	vlen = custommp4_video_length(file1);
	alen = custommp4_audio_length(file1);
	
	printf("video frames:%d\n",vlen);
	
	if(custommp4_has_video(file1))
	{
		iflymp4_set_video(file2, 1, width, height, 25, 1000, "XVID");
	}
	if(custommp4_has_audio(file1))
	{
		iflymp4_set_audio(file2,1,8000,8,240,1000,30,"PCMU");
	}
	
	for(i=0;i<vlen;i++)
	{
		int j;
		vsize = custommp4_read_video_frame(file1,buf,sizeof(buf),i,&start_time,&key);
		for(j=0;j<1;j++)
		{
			iflymp4_write_video_frame(file2,buf,vsize,1,key,40,0);
			vtimestamp += 40;
		}
	}
	
	custommp4_close(file1);
	iflymp4_close(file2);
	
	return 0;
}

int custommp4_to_iflymp4_2(int argc, char **argv)
{
	int i;
	
	custommp4_t *file1 = custommp4_open("y00005.ifv",O_R,0);
	iflymp4_t *file2 = iflymp4_open("layout.ifv",0,1,0);
	
	int width  = custommp4_video_width(file1);
	int height = custommp4_video_height(file1);
	
	int vlen = custommp4_video_length(file1);
	int alen = custommp4_audio_length(file1);
	
	int vsize;
	int vtimestamp = 0;
	
	u32 start_time;
	u8  key;
	
	printf("video frames:%d\n",vlen);
	
	if(custommp4_has_video(file1))
	{
		iflymp4_set_video(file2, 1, width, height, 25, 1000, "XVID");
	}
	if(custommp4_has_audio(file1))
	{
		iflymp4_set_audio(file2,1,8000,8,240,1000,30,"PCMU");
	}
	
	for(i=0;i<vlen;i++)
	{
		int j;
		u8  a[4];
		vsize = custommp4_read_video_frame(file1,buf,sizeof(buf),i,&start_time,&key);
		printf("frame%d:key=%d,start_time=%d,vsize=%d\n",i+1,key,start_time,vsize);
		a[0] = buf[0];
		a[1] = buf[1];
		a[2] = buf[2];
		a[3] = buf[3];
		printf("(%02x%02x%02x%02x)\n",a[0],a[1],a[2],a[3]);
		for(j=0;j<1;j++)
		{
			iflymp4_write_video_frame(file2,buf,vsize,1,key,40,0);
			vtimestamp += 40;
		}
		Sleep(1000);
	}
	
	custommp4_close(file1);
	iflymp4_close(file2);
	
	return 0;
}
*/
int custommp4_to_iflymp43(int argc, char **argv)
{
	//FILE *fp = fopen("stream","wb");
	int i;
	int vlen;
	int vsize;
	//iflymp4_t *file1 = iflymp4_open("rec_2.ifv",0,1,0);
	custommp4_t *file2 = custommp4_open("123.ifv",O_R,0);
	u32 start_time;
	u8 key;
	if(file2 == NULL) return -1;
	vlen = custommp4_video_length(file2);
	printf("video frames:%d\n\n",vlen);
	//iflymp4_set_video(file1,1,352,288,25,1000,"XVID");
	//iflymp4_set_audio(file1,1,8000,8,240,1000,30,"PCMU");
	//for(i=5025;i<5251;i++)
	//for(i=5100;i<5176;i++)
//	for(i=0;i<vlen;i++)
	while(1)
	{
//		vsize = custommp4_read_video_frame(file2,buf,sizeof(buf),i,&start_time,&key);
		//printf("#####video frame%d:size=%d\n",i+1,vsize);
		//iflymp4_write_video_frame(file1,buf,vsize,1,1,40,0);
		
		u8 nMediaType = 0;
		u8 nKey = 0;
		u64 nPts = 0;
		
		u32 nStampTime = 0;
		s32 nRealSize = 0;
		
		static u32 nLastStampTime = 0;
		
		nRealSize = custommp4_read_one_media_frame(file2, buf, sizeof(buf), &nStampTime, &key, &nPts, &nMediaType);
		if(nRealSize <= 0)
		{
			break;
		}
		if(!nMediaType)
		{
			//printf("%s key:%d stamp:%u pts:%u\n",nMediaType?"Audio":"Video",key,nStampTime,(u32)nPts);
			
			if(nLastStampTime == 0)
			{
				nLastStampTime = nStampTime;
			}
			else
			{
				printf("%s key:%d stamp:%u span:%u\n",nMediaType?"Audio":"Video",key,nStampTime,nStampTime-nLastStampTime);
			}
			
			nLastStampTime = nStampTime;
		}

		/*if(i!=5110)
		{
			fwrite(buf,vsize,1,fp);
		}
		else
		{
			//memset(buf,0xff,vsize);
			buf[0] = 0x00;
			buf[1] = 0x00;
			buf[2] = 0x01;
			buf[3] = 0xb6;
			fwrite(buf,vsize,1,fp);
		}*/
		
		/*//if(i==5110 || i == 5111)
		{
			char filename[64];
			FILE *tmpfp;
			sprintf(filename,"frames\\frame%d",i);
			tmpfp = fopen(filename,"wb");
			fwrite(buf,vsize,1,tmpfp);
			fclose(tmpfp);
			
			printf("frame%d size=%d\n",i-5025,vsize);
			
		}*/
	}
	//iflymp4_close(file1);
	custommp4_close(file2);
	//fclose(fp);
	
	return 0;
}

/*int iflymp4_to_custommp4(int argc, char **argv)
{
	int i;
	
	iflymp4_t *file1 = iflymp4_open("rec.ifv",1,0,0);
	custommp4_t *file2 = custommp4_open("my.ifv",O_W_CREAT,0);
	
	int width  = iflymp4_video_width(file1,1);
	int height = iflymp4_video_height(file1,1);
	
	int vlen = iflymp4_video_length(file1,1);
	int alen = iflymp4_audio_length(file1,1);
	
	int vsize;
	int vtimestamp = 0;
	
	u8  update;
	
	printf("video frames:%d\n",vlen);
	
	if(iflymp4_has_video(file1)) custommp4_set_video(file2,1000,(u16)width,(u16)height,25,512*1024,str2uint("XVID"),0x18);
	if(iflymp4_has_audio(file1)) custommp4_set_audio(file2,1000,1,8,8000,str2uint("PCMU"),240,30);
	
	for(i=0;i<vlen;i++)
	{
		int j;
		vsize = iflymp4_read_frame(file1,buf,1);
		for(j=0;j<1;j++)
		{
			custommp4_write_video_frame(file2,buf,vsize,vtimestamp,1,&update);
			vtimestamp += 40;
		}
	}
	
	iflymp4_close(file1);
	custommp4_close(file2);
	
	return 0;
}
*/
int ReadVideoStream(int argc, char **argv)
{
	int i;
	
	int vlen;
	int vsize;
	
	u32 start_time;
	u8 key;
	
	custommp4_t *file2 = NULL;
	FILE *fp = NULL;
	
	if(argc < 3)
	{
		perror("param");
		exit(-1);
	}
	
	file2 = custommp4_open(argv[1],O_R,atoi(argv[3]));
	fp = fopen(argv[2],"wb");
	
	if(file2 == NULL || fp == NULL)
	{
		perror("file");
		exit(-1);
	}
	
	vlen = custommp4_video_length(file2);
	printf("video frames:%d\n\n",vlen);
	
	for(i=0;i<vlen;i++)
	{
		vsize = custommp4_read_video_frame(file2,buf,sizeof(buf),i,&start_time,&key);
		fwrite(buf,vsize,1,fp);
	}
	
	custommp4_close(file2);
	fclose(fp);
	
	printf("conv over\n");
	
	return 0;
}

int ReadAudioStream(int argc, char **argv)
{
	int i;
	
	int alen;
	int asize;
	
	u32 start_time;
	
	custommp4_t *file2 = NULL;
	FILE *fp = NULL;
	
	if(argc < 3)
	{
		perror("param");
		exit(-1);
	}
	
	file2 = custommp4_open(argv[1],O_R,0);
	fp = fopen(argv[2],"wb");
	
	if(file2 == NULL || fp == NULL)
	{
		perror("file");
		exit(-1);
	}
	
	alen = custommp4_audio_length(file2);
	printf("audio frames:%d\n\n",alen);
	
	for(i=0;i<alen;i++)
	{
		asize = custommp4_read_audio_frame(file2,buf,sizeof(buf),i,&start_time);
		fwrite(buf,asize,1,fp);
	}
	
	custommp4_close(file2);
	fclose(fp);
	
	printf("conv over\n");
	
	return 0;
}

int main(int argc, char **argv)
{
	//return ReadAudioStream(argc,argv);
	//return ReadVideoStream(argc,argv);
	custommp4_to_iflymp43(argc,argv);
}
