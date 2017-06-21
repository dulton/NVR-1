#include "mod_common.h"
#include "mod_ptz.h"

void ModPtzUpdateList(u32* pNums, u8** pData)
{
	printf("\n\n---------------------------\nProtocol number = %d\n***List***\n", *pNums);
	int i;
	for(i=0;i<*pNums;i++)
	{
		printf("%s\n", pData[i]);
	}
	printf("***End****\n---------------------------\n\n");
}

int main(int argc, char** argv)
{
	SPtzInitPara initpara;
	memset(&initpara, 0, sizeof(initpara));
	initpara.nChnNum = 4;
	initpara.pDevPath = (u8 *)"/dev/ttyAMA1";
	initpara.pfUpdateProtocol = ModPtzUpdateList;
	if(0 != ModPtzInit(&initpara))
	{
		printf("Mod Ptz init failed ! \n");
		return -1;
	}
	printf("Mod Ptz init OK ! \n");
	
	SModPtzParam param;
	memset(&param, 0, sizeof(SModPtzParam));
	param.nChn = 0;
	param.emBaudRate = EM_PTZ_BAUDRATE_2400;
	param.emCheckType = EM_PTZ_CRCCHECK_TYPE_NONE;
	param.emDataBit = EM_PTZ_DATABIT_8;
	param.emFlowCtrlType = EM_PTZ_FLOWCTRL_NONE;
	param.emStopBit = EM_PTZ_STOPBIT_1;
	param.sTourPresetPara.nPresetId[0] = 0x3e;
	
	char cmd[20];
	while(1)
	{
		scanf("%s", cmd);
		
		if(0 == strcasecmp(cmd, "quit"))
		{
			break;
		}
		else if(0 == strcasecmp(cmd, "setpara"))
		{
			int addr = 0;
			printf("input CamAddress : ");
			scanf("%d", &addr);
			param.nCamAddr = addr & 0xff;
			printf("input Protocol's name : ");
			scanf("%s", param.nProtocolName);
			if(0 != ModPtzSetParam(&param))
			{
				printf("set param error!\n");
			}
		}
		else if(0 == strcasecmp(cmd, "setspeed"))
		{
			int speed = 0;
			printf("input speed : ");
			scanf("%d", &speed);			
			if(0 != ModPtzCtrl(param.nChn, EM_PTZ_CMD_SETSPEED, (void*)&speed))
			{
				printf("set speed error!\n");
			}
		}
		else if(0 == strcasecmp(cmd, "preset"))
		{
			int key1 = 0;
			printf("0[set] 1[clear] 2[goto] : ");
			scanf("%d", &key1);
			int key2 = 0;
			printf("preset ID : ");
			scanf("%d", &key2);			
			if(0 != ModPtzCtrl(param.nChn, EM_PTZ_CMD_PRESET_SET + key1, &key2))
			{
				printf("do preset error!\n");
			}	
		}
		else if(0 == strcasecmp(cmd, "tour"))
		{
			int key1 = 0;
			//printf("0[start] 1[stop] 2[add] 3[delete] 4[auto start] 5[auto stop] : ");
			printf("0[start] 1[stop] 4[auto start] 5[auto stop] : ");
			scanf("%d", &key1);
			void* data = malloc(100);
			if(key1 < 4)
			{
				int key2 = 0;
				printf("tour ID : ");
				scanf("%d", &key2);
				memcpy(data, &key2, sizeof(int));
				/*if(key1 > 1)
				{
					unsigned long long preset = 0;
					scanf("%llu", &preset);
					SModPtzTourInfo info;
					memset(&info, 0, sizeof(SModPtzTourInfo));
					info.nTourId = key2;
					info.nPresetSet[0] = preset;
					memcpy(data, &info, sizeof(SModPtzTourInfo));
				}*/
			}
			if(0 != ModPtzCtrl(param.nChn, EM_PTZ_CMD_START_TOUR + key1, data))
			{
				printf("do preset error!\n");
			}
			free(data);
		}
		else if(0 == strcasecmp(cmd, "cmd"))
		{
			int cmd = 0;
			printf("input cmd : ");
			scanf("%d", &cmd);			
			if(0 != ModPtzCtrl(param.nChn, cmd, NULL))
			{
				printf("send cmd error!\n");
			}
		}
		else
		{
			printf("wrong order!\n");
		}
	}
	
	if(0 == ModPtzDeinit())
	{
		printf("ModPtzDeinit OK!\n");
	}
	return 0;
}
