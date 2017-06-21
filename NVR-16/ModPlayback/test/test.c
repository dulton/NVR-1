#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//#include "preview.h"
#include "lib_common.h"
#include "common_basetypes.h"
#include "utils.h"
#include "mod_playback.h"
#include "custommp4.h"
#include "diskmanage.h"
#include "partitionindex.h"
#include "hddcmd.h"

void PlayProgress(int prog, BOOL stop)
{
	printf("prog = %d, stop = %d\n", prog, stop);
}

#if 1
int main()
{
	int init_param = 0;
	init_param |= TL_128MDDR;
	init_param |= TL_PAL;
	//init_param |= TL_VGA;

	tl_hslib_init_c(TL_BOARD_TYPE_R9504_4D1, init_param);
	//tl_hslib_init(TL_BOARD_TYPE_R9504_4D1);
	//tl_preview(PREVIEW_1D1,0);
	//usleep(5*1000*1000);
	//ftStartMultiPreview(PREVIEW_4CIF,0);
	//PBConfig pbconfig;
	//strcpy(pbconfig.fileName,"/mnt/test.ifv");
	//ModPlayBackByFile(&pbconfig);

	char dirpath[32] = {0};
	sprintf(dirpath,"rec");
	mkdir(dirpath,1);
	disk_manager hddmanager;
	memset(&hddmanager,0,sizeof(hddmanager));
	init_disk_manager(&hddmanager);
	
	PbMgrHandle hPbMgr = ModPlayBackInit(4, &hddmanager);

	SPBSearchPara SSearchParam;
	memset(&SSearchParam,0,sizeof(SSearchParam));
	
	struct tm tm_time;
	tm_time.tm_year = 1970 - 1900;
	tm_time.tm_mon = 1 - 1;
	tm_time.tm_mday = 1;
	tm_time.tm_hour = 0;
	tm_time.tm_min = 0;
	tm_time.tm_sec = 0;
	SSearchParam.nStartTime = mktime(&tm_time);

	tm_time.tm_year = 2011 - 1900;
	tm_time.tm_mon = 8 - 1;
	tm_time.tm_mday = 20;
	tm_time.tm_hour = 23;
	tm_time.tm_min = 59;
	tm_time.tm_sec = 59;
	SSearchParam.nEndTime = mktime(&tm_time);

	SSearchParam.nMaskChn = 0x0000000f; //1101
	SSearchParam.nMaskType = 0xff;

	printf("normal playback 5 seconds \n");
	ModPlayBackByTime(hPbMgr,&SSearchParam);
	sleep(5);


#if 0





	printf("speed up, playback 3 seconds\n");
	ModPlayBackControl(hPbMgr,EM_CTL_SPEED_UP,0);
	sleep(3);


	printf("speed up, playback 3 seconds\n");
	ModPlayBackControl(hPbMgr,EM_CTL_SPEED_UP,0);
	sleep(3);

	printf("pause 3 seconds\n");
	ModPlayBackControl(hPbMgr,EM_CTL_PAUSE,0);
	sleep(3);

	printf("speed up, playback 2 seconds\n");
	ModPlayBackControl(hPbMgr,EM_CTL_SPEED_UP,0);
	sleep(2);

	printf("set normal speed\n");
	ModPlayBackControl(hPbMgr,EM_CTL_SET_SPEED,PLAYRATE_1);

	printf("seek to '1302530710' time,playback 5 seconds\n");
	ModPlayBackControl(hPbMgr,EM_CTL_SEEK,1302530710);
	sleep(5);


	printf("speed down, playback 3 seconds\n");
	ModPlayBackControl(hPbMgr,EM_CTL_SPEED_DOWN,0);
	sleep(3);

	printf("speed down, playback 3 seconds\n");
	ModPlayBackControl(hPbMgr,EM_CTL_SPEED_DOWN,0);
	sleep(3);

	printf("speed down, playback 3 seconds\n");
	ModPlayBackControl(hPbMgr,EM_CTL_SPEED_DOWN,0);
	sleep(3);

	printf("set normal speed,playback 5 seconds\n");
	ModPlayBackControl(hPbMgr,EM_CTL_SET_SPEED,PLAYRATE_1);
	sleep(5);



	printf("pause 3 seconds\n");
	ModPlayBackControl(hPbMgr,EM_CTL_PAUSE,0);
	sleep(3);
	
	printf("resume playback,keep 5 seconds\n");
	ModPlayBackControl(hPbMgr,EM_CTL_RESUME,0);
	sleep(5);
	
	printf("step playback 100 times\n");
	u32 i = 0;
	for(i=0; i<100; i++)
	{
		ModPlayBackControl(hPbMgr,EM_CTL_STEP,0);
	}

	printf("seek to next sect,playback 3 seconds\n");
	ModPlayBackControl(hPbMgr,EM_CTL_NXT_SECT,0);
	sleep(3);

	printf("seek to pre sect,playback 3 seconds\n");
	ModPlayBackControl(hPbMgr,EM_CTL_PRE_SECT,0);
	sleep(3);

	printf("seek to '1302530765' time,playback 5 seconds\n");
	ModPlayBackControl(hPbMgr,EM_CTL_SEEK,1302530765);
	sleep(5);

	printf("backward playback 15 seconds\n");
	ModPlayBackControl(hPbMgr,EM_CTL_BACKWARD,0);
	sleep(15);

	printf("forward playback 10 seconds\n");
	ModPlayBackControl(hPbMgr,EM_CTL_FORWARD,0);
	sleep(10);

	printf("stop playback\n");
	ModPlayBackControl(hPbMgr,EM_CTL_STOP,0);
	sleep(5);
	
	printf("normal playback\n");
	ModPlayBackByTime(hPbMgr,&SSearchParam);
	sleep(5);
	
	printf("All test end!\n");

	#endif

	while(1)
	{
		usleep(100*1000);
	}

	return 0;
}
#else
int main()
{
	int init_param = 0;
	init_param |= TL_128MDDR;
	init_param |= TL_PAL;
	tl_hslib_init_c(TL_BOARD_TYPE_R9504_4D1, init_param);
	
	char dirpath[32] = {0};
	sprintf(dirpath,"rec");
	mkdir(dirpath,1);
	disk_manager hddmanager;
	memset(&hddmanager,0,sizeof(hddmanager));
	init_disk_manager(&hddmanager);
	
	PbMgrHandle hPbMgr = ModPlayBackInit(4, &hddmanager);
	
	SPBRecfileInfo RecInfo;
	RecInfo.nChn = 0;
	RecInfo.nType = 8;
	RecInfo.nStartTime = 1306867214;
	RecInfo.nEndTime = 1306867223;
	RecInfo.nFormat = 3;      //3:cif; 4:4cif
	RecInfo.nStreamFlag = 0;  //0:¨º¨®?¦Ì¨¢¡Â;1:¨°??¦Ì¨¢¡Â
	RecInfo.nSize = 928087;
	RecInfo.nOffset = 0;
	RecInfo.nDiskNo = 0;
	RecInfo.nPtnNo = 0;
	RecInfo.nFileNo = 0;
	
	ModPlayBackByFile(hPbMgr,&RecInfo);
	if(0 != ModPlayBackProgress(PlayProgress))
	{
		printf("regist function failed!\n");
	}
	printf("OK\n");
	while(1)
	{
		sleep(100);
	}
}
#endif

