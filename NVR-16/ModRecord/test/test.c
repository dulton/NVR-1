#include "mod_common.h"
#include "custommp4.h"
#include "diskmanage.h"
#include "partitionindex.h"
#include "hddcmd.h"
#include "mod_record.h"

#define CHNNUM 4

u8 g_CurRecStatus[CHNNUM] = {0};
extern int max_hdd_index;

void GetRecStatus(void* status)
{
	if(NULL == status)
	{
		printf("error: %s\n", __FUNCTION__);
	}
	else
	{
		memcpy(g_CurRecStatus, status, sizeof(g_CurRecStatus));
	}
}

void SendFrameFxn()
{	
	u8 i = 0;
	s32 ret = 0;
	SModRecRecordHeader head;
	memset(&head, 0, sizeof(head));
	custommp4_t *file1 = NULL;
	u8 media_type,key;
	u64 pts = 0;
	u32 start_time;
	s32 realsize = -1;
	unsigned char buf[512<<10] = {0};
	u8 flag = 0;
	u64 offset_pts = 0;
	u64 first_pts = 0;
	u64 last_pts = 0;
	u64 time_offset = 0;
	u8 flag2 = 1;
	
	while(1)
	{
		if(file1 == NULL)
		{
			//file1 = custommp4_open("/mnt/nfs/my0.ifv",O_R,0);
			file1 = custommp4_open("./my0.ifv",O_R,0);
			if(file1 == NULL)
			{
				printf("testcustommp4:open failed 1\n");
				usleep(30000);
				continue;
			}
		}

		realsize = custommp4_read_one_media_frame(file1,buf,sizeof(buf),&start_time,&key,&pts,&media_type);
		if(realsize <= 0)
		{
			//printf("testcustommp4:read over\n");
			custommp4_close(file1);
			file1 = NULL;

			if(flag2)
			{
				flag2 = 0;
				last_pts = pts;
			}
			offset_pts = head.nPts - pts + (last_pts - first_pts);
			time_offset = 0;
			
			usleep(30000);
			continue;
			//break;
		}
		
		if(flag == 0)
		{
			flag = 1;
			struct timeval tv;
			gettimeofday(&tv, NULL);
			time_offset = (u64)tv.tv_sec * 1000000 + tv.tv_usec - pts;
			first_pts = pts;
		}
		
		head.nFrameType = (key == 0)?0:3;
		head.nMediaType = media_type;//0;
		//printf("media_type = %d\n",media_type);
		head.nStreamType = 1;
		head.emResolution = EM_REC_ENC_D1;
		head.nBitRate = 2048;
		head.nPts = pts + time_offset + offset_pts;
		head.nTimeStamp = head.nPts / 1000;
		//printf("pts=%llu, head.nPts=%llu, start_time=%u, head.nTimeStamp=%llu\n",pts,head.nPts,start_time,head.nTimeStamp);
		head.nDate = buf;
		head.nDataLength = realsize;
		//printf("frame size : [%d KB] or [%d B]\n", realsize>>10, realsize);
		
		for(i = 0; i < CHNNUM; i++)
		{
			if(g_CurRecStatus[i] > 0)
			{
				head.nChn = i;
				ret = ModRecordWriteOneFrame(i, &head);
				printf("pts=%llu, head.nPts=%llu, start_time=%u, head.nTimeStamp=%llu\n",pts,head.nPts,start_time,head.nTimeStamp);
				if(ret != EM_REC_SUCCESS)
				{
					fprintf(stderr, "ModRecordWriteOneFrame error !  error code = %d\n", ret);
				}
			}
		}
		usleep(35 * 1000);
	}
}

int main(int argc, char** argv)
{
	fprintf(stdout, "Record test program is starting ......\n");

	char dirpath[32] = {0};
	sprintf(dirpath,"rec");
	mkdir(dirpath,1);
	disk_manager hddmanager;
	memset(&hddmanager,0,sizeof(hddmanager));
	init_disk_manager(&hddmanager);

	s32 ret = 0;	
	pthread_t id;
	if((ret = (pthread_create(&id, NULL, (void*)SendFrameFxn, NULL))) != 0)
	{
		fprintf(stderr, "create pthread error!!\n");
		return ret;
	}
	
	u8 c = 0;
	u32 chn = 0;
	u8 cmd[20] = {0};	
	while(1)
	{
		scanf("%s", cmd);
		
		if(0 == strcmp(cmd, "init"))
		{
			SModRecordRecPara recpara;
			memset(&recpara, 0, sizeof(SModRecordRecPara));
			recpara.nBitRate = 2048;
			recpara.emResolution = EM_REC_ENC_CIF;
			recpara.nFrameRate = 25;
			if(max_hdd_index == -1)
			{
				ret = ModRecordInit(CHNNUM, &recpara, GetRecStatus, NULL);
			}	
			else
			{
				ret = ModRecordInit(CHNNUM, &recpara, GetRecStatus, &hddmanager);
			}	
			if(ret != EM_REC_SUCCESS)
			{
				fprintf(stderr, "ModRecordInit error !\n");
			}
			else
			{
				//fprintf(stdout, "ModRecordInit success !\n");
			}
		}
		else if(0 == strcmp(cmd, "deinit"))
		{
			ret = ModRecordDeInit();
			if(ret != EM_REC_SUCCESS)
			{
				fprintf(stderr, "ModRecordDeInit error !\n");
			}
			else
			{
				fprintf(stdout, "ModRecordDeInit success !\n");
			}	
		}
		else if(0 == strcmp(cmd, "start"))
		{
			scanf("%d", &chn);
			ret = ModRecordStartManual(chn);
			if(ret != EM_REC_SUCCESS)
			{
				fprintf(stderr, "ModRecordStartManual error !\n");
			}
			usleep(1000);
			for(c = 0; c < CHNNUM; c++)
			{
				printf("0x%2x, ",g_CurRecStatus[c]);
			}
			printf("\n");
		}
		else if(0 == strcmp(cmd, "stop"))
		{
			scanf("%d", &chn);
			ret = ModRecordStopManual(chn);
			if(ret != EM_REC_SUCCESS)
			{
				fprintf(stderr, "ModRecordStopManual error !\n");
			}
			usleep(1000);
			for(c = 0; c < CHNNUM; c++)
			{
				printf("0x%2x, ",g_CurRecStatus[c]);
			}
			printf("\n");
		}
		else if(0 == strcmp(cmd, "startallmanual"))
		{
			for(c = 0; c < CHNNUM; c++)
			{
				ret = ModRecordStartManual(c);
				if(ret != EM_REC_SUCCESS)
				{
					fprintf(stderr, "ModRecordStartManual error !\n");
				}
			}
		}
		else if(0 == strcmp(cmd, "stopallmanual"))
		{
			for(c = 0; c < CHNNUM; c++)
			{
				ret = ModRecordStopManual(c);
				if(ret != EM_REC_SUCCESS)
				{
					fprintf(stderr, "ModRecordStopManual error !\n");
				}
			}
		}
		else if(0 == strcmp(cmd, "pause"))
		{
			scanf("%d", &chn);
			ret = ModRecordPause(chn);
			if(ret != EM_REC_SUCCESS)
			{
				fprintf(stderr, "ModRecordPause error !\n");
			}
		}
		else if(0 == strcmp(cmd, "resume"))
		{
			scanf("%d", &chn);
			ret = ModRecordResume(chn);
			if(ret != EM_REC_SUCCESS)
			{
				fprintf(stderr, "ModRecordResume error !\n");
			}
		}
		else if(0 == strcmp(cmd, "stopchn"))
		{
			scanf("%d", &chn);
			ret = ModRecordStop(chn);
			if(ret != EM_REC_SUCCESS)
			{
				fprintf(stderr, "ModRecordStop error !\n");
			}
		}
		else if(0 == strcmp(cmd, "stopall"))
		{
			ret = ModRecordStopAll();
			if(ret != EM_REC_SUCCESS)
			{
				fprintf(stderr, "ModRecordStopAll error !\n");
			}
		}
		/*else if(0 == strcmp(cmd, "setschpara"))
		{
			scanf("%d", &chn);
			SModRecSchPara para;
			memset(&para, 0, sizeof(SModRecSchPara));
			para.nChn = chn;
			para.nTimeType = EM_REC_SCH_EVERY_DAY;
			u32 i;
			scanf("%d", &i);
			para.nRecType[0][0]= i;
			para.nSchTime[0][0].nStartTime = 100;
			para.nSchTime[0][0].nStopTime = 3600 *23;
			ret = ModRecordSetSchedulePara(chn, &para);
			if(ret != EM_REC_SUCCESS)
			{
				fprintf(stderr, "ModRecordSetSchedulePara error !\n");
			}
			else
			{
				fprintf(stdout, "ModRecordSetSchedulePara success !\n");
			}
		}*/
		else if(0 == strcmp(cmd, "setpretime"))
		{
			scanf("%d", &chn);
			u32 i;
			scanf("%d", &i);
			ret = ModRecordSetPreTime(chn, i);
			if(ret != EM_REC_SUCCESS)
			{
				fprintf(stderr, "ModRecordSetPreTime error !\n");
			}
			else
			{
				fprintf(stdout, "ModRecordSetPreTime success !\n");
			}
		}
		else if(0 == strcmp(cmd, "setdelaytime"))
		{
			scanf("%d", &chn);
			u32 i;
			scanf("%d", &i);
			ret = ModRecordSetDelayTime(chn, i);
			if(ret != EM_REC_SUCCESS)
			{
				fprintf(stderr, "ModRecordSetDelayTime error !\n");
			}
			else
			{
				fprintf(stdout, "ModRecordSetDelayTime success !\n");
			}
		}
		else if(0 == strcmp(cmd, "setchntrig"))
		{
			scanf("%d", &chn);
			u32 i;
			scanf("%d", &i);
			//ret = ModRecordSetRecChnTriggered(chn, EM_RECALARM_V_MOTION, i);
			if(ret != EM_REC_SUCCESS)
			{
				fprintf(stderr, "ModRecordSetRecChnTriggered error !\n");
			}
			else
			{
				fprintf(stdout, "ModRecordSetRecChnTriggered success !\n");
			}
			//ret = ModRecordSetRecChnTriggered(chn, EM_RECALARM_V_ALARM, i);
			if(ret != EM_REC_SUCCESS)
			{
				fprintf(stderr, "ModRecordSetRecChnTriggered error !\n");
			}
			else
			{
				fprintf(stdout, "ModRecordSetRecChnTriggered success !\n");
			}
		}
		else if(0 == strcmp(cmd, "motion1"))
		{
			scanf("%d", &chn);
			ret = ModRecordSetTrigger(chn, EM_RECALARM_V_MOTION, 1);
			if(ret != EM_REC_SUCCESS)
			{
				fprintf(stderr, "ModRecordSetTrigger error !\n");
			}
			usleep(1000);
			for(c = 0; c < CHNNUM; c++)
			{
				printf("0x%2x, ",g_CurRecStatus[c]);
			}
			printf("\n");
		}
		else if(0 == strcmp(cmd, "motion0"))
		{
			scanf("%d", &chn);
			ret = ModRecordSetTrigger(chn, EM_RECALARM_V_MOTION, 0);
			if(ret != EM_REC_SUCCESS)
			{
				fprintf(stderr, "ModRecordSetTrigger error !\n");
			}
			usleep(1000);
			for(c = 0; c < CHNNUM; c++)
			{
				printf("0x%2x, ",g_CurRecStatus[c]);
			}
			printf("\n");
		}
		else if(0 == strcmp(cmd, "alarm1"))
		{
			scanf("%d", &chn);
			ret = ModRecordSetTrigger(chn, EM_RECALARM_V_ALARM, 1);
			if(ret != EM_REC_SUCCESS)
			{
				fprintf(stderr, "ModRecordSetTrigger error !\n");
			}
			usleep(1000);
			for(c = 0; c < CHNNUM; c++)
			{
				printf("0x%2x, ",g_CurRecStatus[c]);
			}
			printf("\n");
		}
		else if(0 == strcmp(cmd, "alarm0"))
		{
			scanf("%d", &chn);
			ret = ModRecordSetTrigger(chn, EM_RECALARM_V_ALARM, 0);
			if(ret != EM_REC_SUCCESS)
			{
				fprintf(stderr, "ModRecordSetTrigger error !\n");
			}
			usleep(1000);
			for(c = 0; c < CHNNUM; c++)
			{
				printf("0x%2x, ",g_CurRecStatus[c]);
			}
			printf("\n");
		}
		else if(0 == strcmp(cmd, "getstatus"))
		{
			for(c = 0; c < CHNNUM; c++)
			{
				printf("0x%2x, ",g_CurRecStatus[c]);
			}
			printf("\n");
		}
		else if(0 == strcmp(cmd, "quit"))
		{
			break;
		}
	}
	
	ModRecordDeInit();
	return ret;
}
