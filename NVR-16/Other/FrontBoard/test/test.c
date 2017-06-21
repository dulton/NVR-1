#include "mod_common.h"
#include "Frontboard.h"
#include "mod_config.h"

int main()
{
	s32 ret = 0;
	
	ret = ModConfigInit();
	if(ret != 0)
	{
		printf("ModConfigInit error!\n");
		return -1;
	}
	
	ret = FrontboardCreate();
	if(ret != 0)
	{
		printf("FrontboardCreate error!\n");
		return -1;
	}
	
	char cmd[10];
	while(1)
	{
		scanf("%s", cmd);
		if(!strcasecmp(cmd, "read"))
		{
			s8 tmp[4];
			ret = FrontboardRead(tmp, 1);
			if(ret != 0)
			{
				printf("front board read error!\n");
				continue;
			}
			printf("read key:[0x%02x]\n", *tmp);
		}
		else if(!strcasecmp(cmd, "write"))
		{
			s8 tmp[4];
			int tmp1, tmp2;
			scanf("%x", &tmp1);
			scanf("%x", &tmp2);
			tmp[0] = (u8)tmp1;
			tmp[1] = (u8)tmp2;
			ret = FrontboardWrite(tmp, 2);
			if(ret != 0)
			{
				printf("front board write error!\n");
				continue;
			}
			printf("write key:[0x%02x%02x]\n", *tmp, *(tmp + 1));
		}
		else if(!strcasecmp(cmd, "quit"))
		{
			break;
		}
	}
	
	FrontboardDestory();
	ModConfigDeinit();
	
	return 0;
}

