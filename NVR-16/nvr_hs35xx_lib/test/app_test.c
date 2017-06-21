#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>

#include "lib_common.h"
#include "lib_preview.h"
#include "lib_audio.h"
#include "lib_venc.h"
#include "lib_vdec.h"
#include "lib_video.h"
#include "lib_venc_osd.h"
#include "lib_video_cover.h"
#include "lib_misc.h"
#include "lib_snapshot.h"

#define HDD_DIR	"./"		//µ±Ç°Ä¿Â¼

#define FILE_HEAD_MAGIC	0x12345678
typedef struct
{
	unsigned int magic;
	venc_frame_type_e type;
	unsigned long long pts;
	unsigned int len;
}vfile_head_t;

#define TL_PAL  0x02
#define TL_NTSC 0x04
#define TL_VGA	0x20
#define TL_HDMI	0x80

unsigned int asc_to_u32(char *s, int *err)
{
	unsigned int val;
	char *p;
		
	*err = -1;	
	if(s == NULL)
		return 0;
	
	if((*s && *s == '0') && ( *(s+1) && *(s+1) == 'x')){
		p = s + 2;
		if('\0' == *p)
			return 0;
		for(; *p; p++){
			if(!((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'f')))
				return 0;
		}
		val = 0;
		for(p = s + 2; *p; p++){
			val = val * 16 + ((*p <= '9')? (*p - '0'):(*p - 'a' + 10));
		}
	}else{
		p = s ;
		if('\0' == *p)
			return 0;
		for(; *p; p++){
			if(!(*p >= '0' && *p <= '9'))
				return 0;
		}
		val = 0;
		for(p = s; *p; p++){
			val = val * 10 + (*p - '0');
		}
	}
		
	*err = 0;
	return val;
}

int check_args(int argc, char **argv, unsigned int *out, int len)
{
	int i;
	int err;
	
	if(argc < len){
		printf("too few args!\n");
		return -1;
	}
	for(i = 0; i < len; i++){
		out[i] = asc_to_u32(argv[i], &err);
		if(err){
			printf("wrong arg %d:(%s)\n", i, argv[i]);
			return -1;
		}
	}
	return 0;
}

void test_preview (int argc,  char **argv)
{
	
}

void test_venc(void)
{
	
}

void test_venc2(void)
{
	
}

void test_audio_loop(int argc, char *argv[])
{
	
}

void test_audio_rec(void)
{
	
}

void test_vdec(void)
{
	
}

void test_alarm(void)
{
	
}

void test_connect_status(void)
{
	
}

void test_video_cover(int argc, char **argv)
{
	
}

void test_venc_and_dec(int argc, char **argv)
{
	
}

static void usage(void)
{
	printf("<Usage>:The program Used for testing the audio and video coding and decoding!\n"); 
	printf("\app_test 1\t preview  \n");
	printf("\app_test 2\t video encoded,save file.\n");
	printf("\app_test 3\t video decoding,need H264 File.\n");
	printf("\app_test 4\t audio record. \n");
	printf("\app_test 5\t audio player. \n");
	printf("\app_test 6\t none  \n");
	printf("\app_test 7\t none  \n");
	printf("\app_test 8\t video connect status.\n");
	printf("\app_test 9\t none  \n");
	printf("\app_test a\t none  \n");
	printf("\app_test b\t set image parameter,contain: hue Saturation contrast brightness.\n");
	printf("\app_test c\t get image parameter  \n");
	printf("\app_test d\t set VGA or HDMI resolution.\n");
	printf("\app_test e\t read or write TW2867\n");
	printf("\app_test f\t test_video_cover  \n");
	printf("\app_test g\t none  \n");
	printf("\app_test h\t none  \n");
	printf("\app_test j\t none  \n");
	printf("\app_test k\t none  \n");
	printf("\app_test l\t none  \n");
	printf("\n");
}

#define DBG_MEM_RW_SIZE	6

static int fd_tw286x = -1;

#include "tl-tw286x.h"

int rz_tw286x_rd(int chip_id, unsigned int adr, unsigned char *out_val)
{
	unsigned int buf[DBG_MEM_RW_SIZE];
	
	buf[1] = chip_id;
	buf[2] = adr;
	
	//return 0;//csp modify
	
	if(ioctl(fd_tw286x, TL_2865_RD, buf) < 0){
		printf("common.c@tw286x_rd: TL_2865_RD error\n");
		return -1;
	}
	*out_val = buf[0];
	
	printf("chip id:%d, adr:0x%x, data:0x%x\n\n", chip_id, adr, *out_val);
	
	return 0;
}

int rz_tw286x_wr(int chip_id, unsigned int adr, unsigned char data)
{
	unsigned int buf[DBG_MEM_RW_SIZE];
	
	buf[1] = chip_id;
	buf[2] = adr;
	buf[3] = data;
	
	//return 0;//csp modify
    
	if(ioctl(fd_tw286x, TL_2865_WR, buf) < 0){
		printf("common.c@tw286x_wr: TL_2865_RD error\n");
		return -1;
	}
    
	printf("chip id:%d, adr:0x%x, data:0x%x\n\n", chip_id, adr, data);
    
	return 0;
}

int main(int argc, char **argv)
{
	unsigned int buf[DBG_MEM_RW_SIZE] = {0};
	
#if 0
	fd_tw286x = open("/dev/tw_286x", O_RDWR);
	if(fd_tw286x < 0)
	{
		perror("open /dev/tw_286x");
		printf("<ERROR>rz_hslib_init fail!!!!!!!!!!!!!please Check whether loaded the tw_286x.ko driver!\n");
		return -1;
	}
	
	if(ioctl(fd_tw286x, TL_AUTO_CHIP_TYPE, buf) < 0)
	{
		printf("<ERROR>rz_hslib_init fail!!!!!!!!!!!!! TL_AUTO_CHIP_TYPE: system will not work!\n");
		return -1;
	}
	printf("decoder chip type=%d\n",buf[0]);
	
	unsigned char out_val = 0xff;
	
	rz_tw286x_wr(0, 0x40, 0x00);
	rz_tw286x_rd(0, 0xE0, &out_val);
	rz_tw286x_wr(0, 0xE0, 0);
	
	FILE *fp = fopen("tw2968reg.txt","w");
	
	rz_tw286x_wr(0, 0x40, 0x00);
	
	int i;
	for(i=0;i<256;i++)
	{
		out_val = 0xa5;
		rz_tw286x_rd(0, i, &out_val);
		fprintf(fp,"page: 0, reg: %02x, value: %02x\n",i,out_val);
	}
	
	rz_tw286x_wr(0, 0x40, 0x01);
	
	for(i=0;i<256;i++)
	{
		out_val = 0xa5;
		rz_tw286x_rd(0, i, &out_val);
		fprintf(fp,"page: 1, reg: %02x, value: %02x\n",i,out_val);
	}
	
	fclose(fp);
	
	close(fd_tw286x);
	return 0;
#endif
	
#if 1
	tl_hslib_init_c(TL_BOARD_TYPE_NR3116, 0);
	//tl_hslib_init_c(TL_BOARD_TYPE_NR3132, 0);
	//tl_hslib_init_c(TL_BOARD_TYPE_NR3124, 0);
	
	tl_vga_resolution(VGA_RESOL_N_1280_60);
	
	ipc_node* head = NULL;
	int ret = IPC_Search(&head, PRO_TYPE_ONVIF, 0);
	if(ret < 0)
	{
		return -1;
	}
	
	tl_preview(PREVIEW_CLOSE, 0);
	tl_preview(PREVIEW_1D1, 0);
	
	sleep(5);
	
	head->ipcam.enable = 1;
	tl_set_ipcamera(0, &head->ipcam);
	
	IPC_Free(head);
	head = NULL;
	
	while(1)
	{
		sleep(1);
	}
	
	return ret;
#endif
	
	if(argc < 2)
	{
		usage();
		return 0;
	}
	if(argc > 2)
	{
		check_args((argc - 2), (argv + 2), &buf[2], (argc - 2));
	}
	
	return 0;
}

