#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "common_basetypes.h"
#include "mod_config.h"

int main()
{
	int ret;
	struct timeval timeflag, starttime, endtime;
	gettimeofday(&timeflag, NULL);
	//printf("start time : %ld \n", timeflag.tv_sec*1000 + timeflag.tv_usec/1000);
	starttime = timeflag;
		
	ret = ModConfigInit();
	if(ret != 0)
	{
		printf("ModConfigInit failed!\n");
		return -1;
	}
	
	gettimeofday(&timeflag, NULL);
	//printf("end time : %ld \n", timeflag.tv_sec*1000 + timeflag.tv_usec/1000);
	endtime = timeflag;
	printf("Use time: %ld ms\n", (endtime.tv_sec-starttime.tv_sec)*1000 + (endtime.tv_usec - starttime.tv_usec)/1000);

	s8 filepath[50];
	if(0 == ModConfigGetConfigFilePath(filepath, sizeof(filepath)))
	{
		printf("Config file : %s\n", filepath);
	}
	else
	{
		printf("GetConfigFilePath failed!\n");
	}
	
	SModConfigSystemParam syspara;
	//SModConfigDvrProperty property;
	SModConfigImagePara image;
	SModConfigPtzParam ptz;
	
	char cmd1;
	char cmd2;
	while(1)
	{
		printf("\n\n\n1: get param\t2: set param\t3: get default\t4: default\t5: backup\t6: resume\nq: quit\nselect: ");
		scanf("%c", &cmd1);
		getchar();
		if('q' == cmd1)
		{
			break;
		}
		else if('4' == cmd1)
		{
			if(0 != ModConfigDefault())
			{
				printf("ModConfigDefault failed!\n");
				continue;
			}
			break;
		}
		else if('5' == cmd1)
		{
			s8 tmp[50];
			strcpy(tmp, "./config_today.ini");
			if(0 != ModConfigBackup(tmp))
			{
				printf("ModConfigBackup failed!\n");
			}
			printf("ModConfigBackup OK!\n");
			continue;
		}
		
		printf("\n1:system\t2:preview\t3:image\t\t4:mainosd\t5:strosd\n6:rectosd\t7:encmain\t8:encsub\t9:record\ta:timerrecsch\nb:ptz\t\t\nx:all\ty:property\nselect: ");
		scanf("%c", &cmd2);
		getchar();

		switch(cmd2)
		{
			case '1':
				if('1' == cmd1)
				{
					ret = ModConfigGetParam(EM_CONFIG_PARA_SYSTEM,&syspara,-1);
					if(ret != 0)
					{
						printf("get sys failed!\n");
					}
					else
					{
						printf("%s,%ld,%d,%d,%d,%d,%ld,%d,%d,%d\n",syspara.nDevName,syspara.nDevId,syspara.nVideoStandard,syspara.nAuthCheck,syspara.nOutput,syspara.nLangId,syspara.nLockTime,syspara.nDateFormat,syspara.nTimeStandard,syspara.nCyclingRecord);
					}
				}
				else if('3' == cmd1)
				{
					ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_SYSTEM,&syspara,-1);
					if(ret != 0)
					{
						printf("get default sys failed!\n");
					}
					else
					{
						printf("%s,%ld,%d,%d,%d,%d,%ld,%d,%d,%d\n",syspara.nDevName,syspara.nDevId,syspara.nVideoStandard,syspara.nAuthCheck,syspara.nOutput,syspara.nLangId,syspara.nLockTime,syspara.nDateFormat,syspara.nTimeStandard,syspara.nCyclingRecord);
					}
				}
				else if('2' == cmd1)
				{
					printf("new devname:");
					scanf("%s", syspara.nDevName);
					printf("new devId:");
					scanf("%ld", &syspara.nDevId);
					getchar();
					ret = ModConfigSetParam(EM_CONFIG_PARA_SYSTEM,&syspara,-1);
					if(ret != 0)
					{
						printf("set sys failed!\n");
					}
				}
				else if('6' == cmd1)
				{
					s8 tmp[50];
					strcpy(tmp, "./config_today.ini");
					ret = ModConfigResume(EM_CONFIG_PARA_SYSTEM, -1, tmp);
					if(ret != 0)
					{
						printf("ModConfigResume failed!\n");
					}
				}
				break;

			case '3':
				if('1' == cmd1)
				{
					ret = ModConfigGetParam(EM_CONFIG_PARA_IMAGE,&image,1);
					if(ret != 0)
					{
						printf("get image failed!\n");
					}
					else
					{
						printf("%d,%d,%d,%d\n",image.nHue,image.nSaturation,image.nContrast,image.nBrightness);
					}
				}
				else if('2' == cmd1)
				{
					printf("new hue:");
					int tmp;
					scanf("%d", &tmp);
					image.nHue = tmp;
					getchar();
					ret = ModConfigSetParam(EM_CONFIG_PARA_IMAGE,&image,1);
					if(ret != 0)
					{
						printf("set image failed!\n");
					}
				}
				break;

			case 'b':
				if('1' == cmd1)
				{
					ret = ModConfigGetParam(EM_CONFIG_PARA_PTZ,&ptz,0);
					if(ret != 0)
					{
						printf("get ptz failed!\n");
					}
					else
					{
						printf("%s\n", ptz.nProtocolName);
					}
				}
				else if('3' == cmd1)
				{
					ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_PTZ,&ptz,0);
					if(ret != 0)
					{
						printf("get default sys failed!\n");
					}
					else
					{
						printf("%s\n", ptz.nProtocolName);
					}
				}
				else if('2' == cmd1)
				{
					printf("new protocol:");
					scanf("%s", ptz.nProtocolName);
					printf("new devId:");
					long tmp = 0;
					scanf("%ld", &tmp);
					ptz.nCamAddr = tmp;
					getchar();
					ret = ModConfigSetParam(EM_CONFIG_PARA_PTZ,&ptz,0);
					if(ret != 0)
					{
						printf("set ptz failed!\n");
					}
					else
					{
						printf("set ptz OK!\n");
					}	
				}
				else if('6' == cmd1)
				{
					s8 tmp[50];
					strcpy(tmp, "./config_today.ini");
					ret = ModConfigResume(EM_CONFIG_PARA_PTZ, 0, tmp);
					if(ret != 0)
					{
						printf("ModConfigResume failed!\n");
					}
					else
					{
						printf("ModConfigResume OK!\n");
					}	
				}
				break;

			case 'x':
				if('6' == cmd1)
				{
					s8 tmp[50];
					strcpy(tmp, "./config_today.ini");
					ret = ModConfigResume(EM_CONFIG_PARA_ALL, -1, tmp);
					if(ret != 0)
					{
						printf("ModConfigResume failed!\n");
					}
				}
				break;

			default:
				break;					
		}
	}

/*	SModConfigSystemParam syspara;
	ret = ModConfigGetParam(EM_CONFIG_PARA_SYSTEM,&syspara,-1);
	if(ret != 0)
	{
		printf("sys failed!\n");
		return -1;
	}
	else
	{
		printf("%s,%ld,%d,%d,%d,%d,%ld,%d,%d,%d\n",syspara.nDevName,syspara.nDevId,syspara.nVideoStandard,syspara.nAuthCheck,syspara.nOutput,syspara.nLangId,syspara.nLockTime,syspara.nDateFormat,syspara.nTimeStandard,syspara.nCyclingRecord);
	}

	SModConfigDvrProperty property;
	ret = ModConfigGetParam(EM_CONFIG_PARA_DVR_PROPERTY, &property, -1);
	printf("ret = %d\n", ret);

	SModConfigPreviewParam preview;
	ret = ModConfigGetParam(EM_CONFIG_PARA_PREVIEW, &preview, -1);
	printf("ret = %d\n", ret);

	SModConfigImagePara image;
	for(i = 0; i < chn; i++)
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_IMAGE, &image, i);
		printf("ret = %d\n", ret);
	}
	
	SModConfigMainOsdParam mainosd;
	ret = ModConfigGetParam(EM_CONFIG_PARA_MAIN_OSD, &mainosd, -1);
	printf("ret = %d\n", ret);

	SModConfigStrOsdpara strosd;
	for(i = 0; i < chn; i++)
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_STR_OSD, &strosd, i);
		printf("ret = %d\n", ret);
	}
	
	SModConfigRectOsdpara rectosd;
	for(i = 0; i < chn; i++)
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_RECT_OSD, &rectosd, i);
		printf("ret = %d\n", ret);
	}
*/	

	ret = ModConfigDeinit();
	if(ret != 0)
	{
		printf("ModConfigDeinit failed!\n");
	}

	return 0;
}

