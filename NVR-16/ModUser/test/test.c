#include "mod_common.h"
#include "mod_user.h"

int main()
{
	printf("Mod User starting......\n");
	
	SModUserPara* modpara = calloc(sizeof(SModUserPara), 1);
	strcpy(modpara->UserPara[0].UserName, "admin");
	strcpy(modpara->UserPara[0].Password, "3");
	
	UserCenter center = ModUserInit(modpara);
	if(NULL == center)
	{
		return -1;
	}
	printf("Mod User Init OK\n");
	
	SModUserLoginPara* loginpara = calloc(sizeof(SModUserLoginPara), 1);
	SModUserUserPara* userpara = calloc(sizeof(SModUserUserPara), 1);
	
	u32 nUserId[200] = {0};
	char* cmd = calloc(20, 1);
	
	while(1)
	{
		scanf("%s", cmd);
		if(0 == strcasecmp(cmd, "login"))
		{
			printf("input name:");
			scanf("%s", loginpara->UserName);
			printf("input password:");
			scanf("%s", loginpara->Password);
			if(0 != ModUserLogin(center, loginpara, &nUserId[0]))
			{
				printf("Login failed!\n");
			}
			else
			{
				printf("UserId: %d\n", nUserId[0]);
			}
		}
		else if(0 == strcasecmp(cmd, "logout"))
		{
			if(0 != ModUserLogout(center, nUserId[0]))
			{
				printf("Logout failed!\n");
			}
		}
		else if(0 == strcasecmp(cmd, "adduser"))
		{
			memset(userpara, 0, sizeof(SModUserUserPara));
			userpara->emLoginTwince = EM_MODUSER_LOGINTWINCE_NO;
			printf("input name:");
			scanf("%s", userpara->UserName);
			printf("input password:");
			scanf("%s", userpara->Password);
			if(0 != ModUserAddUser(center, userpara))
			{
				printf("Add user failed!\n");
			}
		}
		else if(0 == strcasecmp(cmd, "deluser"))
		{
			char name[20];
			printf("input name:");
			scanf("%s", name);
			if(0 != ModUserDeleteUser(center, name))
			{
				printf("Del user failed!\n");
			}
		}
		else if(0 == strcasecmp(cmd, "modifyuser"))
		{
			
		}
		else if(0 == strcasecmp(cmd, "getuserinfo"))
		{
			
		}
		else if(0 == strcasecmp(cmd, "getuserlist"))
		{
			char** list = NULL;
			list = calloc(sizeof(char*), MOD_USER_MAX_USER_NUM);
			int i = 0;
			for(i = 0; i < MOD_USER_MAX_USER_NUM; i++)
			{
				list[i] = calloc(USER_GROUP_NAME_MAX_LENGTH, 1);
			}
			int k = USER_GROUP_NAME_MAX_LENGTH;
			if(0 != ModUserGetListUserRegisted(center, list, &k, &i))
			{
				printf("get user list failed!\n");
			}
			else
			{
				printf("user num = %d\n", i);
				while(i)
				{
					printf("%s\n", list[--i]);
				}
			}
			for(i = 0; i < MOD_USER_MAX_USER_NUM; i++)
			{
				free(list[i]);
			}
			free(list);
		}
		else if(0 == strcasecmp(cmd, "quit"))
		{
			break;
		}
		else
		{
			printf("cmd is not valid!\n");
		}
	}
	
	if(0 != ModUserDeinit(center))
	{
		printf("mod deinit failed!\n");
	}
	
	free(modpara);
	free(loginpara);
	free(userpara);
	free(cmd);
	return 0;
}
