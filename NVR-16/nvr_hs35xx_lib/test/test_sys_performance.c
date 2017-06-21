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
#include <sched.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

//kernel include
#include <linux/fb.h>

#include "lib_common.h"
#include "lib_preview.h"
#include "lib_venc.h"
#include "lib_venc_osd.h"
#include "lib_vdec.h"
#include "lib_md.h"
#include "lib_audio.h"
#include "lib_misc.h"
#include "lib_snapshot.h"

//pthread_mutex_init(&mutex,NULL);
//pthread_mutex_lock(&mutex);
//pthread_mutex_unlock(&mutex);

#define  CHN_MAX		16
#define  AUDIO_CH_MAX	16

int coding_mode = 0;

#define HDD_DIR		"/mnt/hi35xx/"

#define FILE_HEAD_MAGIC	0x12345678
typedef struct
{
	unsigned int magic;
	venc_frame_type_e type;
	unsigned long long pts;
	unsigned int len;
}vfile_head_t;

typedef struct
{
	int size;
	char *buf[3];
	int buf_free_flag[3];
	int buf_free_size[3];
	int cur_buf_idx;
	FILE *fp;
	pthread_mutex_t mutex;
}wr_hdd_t;

typedef struct
{
	wr_hdd_t vrec[CHN_MAX];
	wr_hdd_t vrec_min[CHN_MAX];
	wr_hdd_t arec[CHN_MAX];
	int net_fd;
}wr_hdd_all_info_t;

wr_hdd_all_info_t wr_hdd_all_info;

static void init_wr_hdd_info(void)
{
	int chn, i;
	wr_hdd_t *pwr_hdd;
	
	memset(&wr_hdd_all_info, 0, sizeof(wr_hdd_all_info_t));
	
	wr_hdd_all_info.net_fd = -1;
	
	for(chn = 0; chn < CHN_MAX; chn++){
		pwr_hdd = &wr_hdd_all_info.vrec[chn];
		pwr_hdd->size = 128*1024;
		pthread_mutex_init(&pwr_hdd->mutex,NULL);
		for(i = 0; i < 3; i++){
			pwr_hdd->buf_free_flag[i] = 1;
			pwr_hdd->buf_free_size[i] = pwr_hdd->size;
			pwr_hdd->buf[i] = malloc(pwr_hdd->size);
			if(pwr_hdd->buf[i] == NULL){
				printf("init_wr_hdd_info: no free buf\n");
				exit(-1);
			}
		}
	}
	for(chn = 0; chn < CHN_MAX; chn++){
		pwr_hdd = &wr_hdd_all_info.vrec_min[chn];
		pwr_hdd->size = 128*1024;
		pthread_mutex_init(&pwr_hdd->mutex,NULL);
		for(i = 0; i < 3; i++){
			pwr_hdd->buf_free_flag[i] = 1;
			pwr_hdd->buf_free_size[i] = pwr_hdd->size;
			pwr_hdd->buf[i] = malloc(pwr_hdd->size);
			if(pwr_hdd->buf[i] == NULL){
				printf("init_wr_hdd_info: no free buf\n");
				exit(-1);
			}
		}
	}
	for(chn = 0; chn < AUDIO_CH_MAX; chn++){
		pwr_hdd = &wr_hdd_all_info.arec[chn];
		pwr_hdd->size = 16*1024;
		pthread_mutex_init(&pwr_hdd->mutex,NULL);
		for(i = 0; i < 3; i++){
			pwr_hdd->buf_free_flag[i] = 1;
			pwr_hdd->buf_free_size[i] = pwr_hdd->size;
			pwr_hdd->buf[i] = malloc(pwr_hdd->size);
			if(pwr_hdd->buf[i] == NULL){
				printf("init_wr_hdd_info: no free buf\n");
				exit(-1);
			}
		}
	}
}

void *wr_to_hdd_Fxn(void *arg)
{
	wr_hdd_t *pwr_hdd;
	int chn, i;
	int ret;

	while(1){
		for(chn = 0; chn < CHN_MAX; chn++){
			pwr_hdd = &wr_hdd_all_info.vrec[chn];
			if(!pwr_hdd->fp)
				continue;
			for(i = 0; i < 3; i++){
				if(pwr_hdd->buf_free_flag[i])
					continue;
				if(chn < CHN_MAX){
					ret = fwrite(pwr_hdd->buf[i], 1, pwr_hdd->size - pwr_hdd->buf_free_size[i], pwr_hdd->fp);
					if(ret != (pwr_hdd->size - pwr_hdd->buf_free_size[i])){
						printf("venc write file error: ret=%d, shoule len=%d\n", ret, pwr_hdd->size - pwr_hdd->buf_free_size[i]);
						perror("write error");
						exit(-1);
					}
				}
				pwr_hdd->buf_free_size[i] = pwr_hdd->size;
				pwr_hdd->buf_free_flag[i] = 1;
				usleep(1);
			}
		}
		usleep(100*1000);
		for(chn = 0; chn < CHN_MAX; chn++){
			pwr_hdd = &wr_hdd_all_info.vrec_min[chn];
			if(!pwr_hdd->fp && wr_hdd_all_info.net_fd < 0)
				continue;
			for(i = 0; i < 3; i++){
				if(pwr_hdd->buf_free_flag[i])
					continue;
				if(wr_hdd_all_info.net_fd > 0){//we send to net high priority
					ret = send(wr_hdd_all_info.net_fd, pwr_hdd->buf[i], pwr_hdd->size - pwr_hdd->buf_free_size[i], 0);
					if(ret != pwr_hdd->size - pwr_hdd->buf_free_size[i]){
						printf("net_send2: should send len=%d, ret=%d\n", pwr_hdd->size - pwr_hdd->buf_free_size[i], ret);
						exit(0);
					}
				}else{
					ret = fwrite(pwr_hdd->buf[i], 1, pwr_hdd->size - pwr_hdd->buf_free_size[i], pwr_hdd->fp);
					if(ret != (pwr_hdd->size - pwr_hdd->buf_free_size[i])){
						printf("venc_min write file error: ret=%d\n", ret);
						perror("write error");
						exit(-1);
					}
				}
				pwr_hdd->buf_free_size[i] = pwr_hdd->size;
				pwr_hdd->buf_free_flag[i] = 1;
			}
			usleep(1);
		}
		usleep(100*1000);
		#if 1
		for(chn = 0; chn < AUDIO_CH_MAX; chn++){
			pwr_hdd = &wr_hdd_all_info.arec[chn];
			if(!pwr_hdd->fp)
				continue;
			for(i = 0; i < 3; i++){
				if(pwr_hdd->buf_free_flag[i])
					continue;
				ret = fwrite(pwr_hdd->buf[i], 1, pwr_hdd->size - pwr_hdd->buf_free_size[i], pwr_hdd->fp);
				if(ret != (pwr_hdd->size - pwr_hdd->buf_free_size[i])){
					printf("aenc write file error: ret=%d\n", ret);
					perror("write error");
					exit(-1);
				}
				pwr_hdd->buf_free_size[i] = pwr_hdd->size;
				pwr_hdd->buf_free_flag[i] = 1;
			}
			usleep(1);
		}
		usleep(100*1000);
		#endif
	}
}

int wr_to_hdd_buf(wr_hdd_t *pwr_hdd, char *buf, int len)
{
	int ret = 0, i;
	
	//pthread_mutex_lock(&pwr_hdd->mutex);
	do{
		if(len > pwr_hdd->size){
			printf("wr_to_hdd_buf too large len=%d\n", len);
			break;
		}
		if(!pwr_hdd->buf_free_flag[pwr_hdd->cur_buf_idx] || pwr_hdd->buf_free_size[pwr_hdd->cur_buf_idx] < len){
			pwr_hdd->buf_free_flag[pwr_hdd->cur_buf_idx] = 0;
			for(i = 0; i < 3; i++){
				if(pwr_hdd->buf_free_flag[i]){
					pwr_hdd->cur_buf_idx = i;
					break;
				}
			}
			if(i == 3){
				printf("wr_to_hdd_buf: no free buffer\n");
				ret = -1;
				break;
			}
		}
		memcpy(pwr_hdd->buf[pwr_hdd->cur_buf_idx] + (pwr_hdd->size - pwr_hdd->buf_free_size[pwr_hdd->cur_buf_idx]), buf, len);
		pwr_hdd->buf_free_size[pwr_hdd->cur_buf_idx] -= len;
		ret = len;
	}while(0);
	//pthread_mutex_unlock(&pwr_hdd->mutex);
	
	return ret;
}

int net_send(int net_fd, char *buf, int len)
{
	int ret;
	
	ret = send(net_fd, buf, len, 0);
	if(ret != len){
		printf("net_send: should send len=%d, ret=%d\n", len, ret);
		exit(0);
	}
	return ret;
}

#define SERVER_IP	"192.168.1.92"
#define SERVER_PORT	20000
#define CLIENT_PORT 20007

int net_connect(void)
{
	int clifd;
	struct sockaddr_in servaddr,cliaddr;
	socklen_t socklen = sizeof(servaddr);
	
	clifd = socket(AF_INET,SOCK_STREAM,0);
	if(clifd < 0){
		printf("create socket error!\n");
		exit(1);
	}
	bzero(&cliaddr,sizeof(cliaddr));
	cliaddr.sin_family = AF_INET;
	cliaddr.sin_port = htons(CLIENT_PORT);
	cliaddr.sin_addr.s_addr = htons(INADDR_ANY); 
	
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_aton(SERVER_IP,&servaddr.sin_addr);
	servaddr.sin_port = htons(SERVER_PORT);
	
	if(bind(clifd,(struct sockaddr*)&cliaddr,sizeof(cliaddr)) < 0){
		printf("bind to port %d failure!\n",CLIENT_PORT);
		exit(1);
	}
	
	if(connect(clifd,(struct sockaddr*)&servaddr, socklen) < 0){
		printf("can't connect to %s!\n", SERVER_IP);
		exit(1);
	}
	
	return clifd;

}

void *vrec_Fxn(void *arg)
{
	return 0;
}

void *vrec_min_Fxn(void *arg)
{
	return 0;
}

void *arec_Fxn(void *arg)
{
	return 0;
}

void *snap_Fxn(void *arg)
{
	return 0;
}

static void *motion_detect_Fxn(void *arg)
{
	return 0;
}

void *adec_Fxn(void *arg)
{
	return 0;
}

void *vdec_Fxn(void *arg)
{
	return 0;
}

void test_preformance(int argc, char **argv)
{
	
}

