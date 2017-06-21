#include "biz_user.h"
#include "biz_config.h"
#include "biz_types.h"

s32 UserInit(u32 nEnable, SBizUserPara* psPara)
{
	if(nEnable)
	{
		//int ret;//csp modify
		
		if(NULL == psPara)
		{
			return -1;
		}
		
		SModUserPara sConfig;
		
		sConfig.nFlagGroupSetted = psPara->nFlagGroupSetted;
		sConfig.nFlagUserSetted[0] = psPara->nFlagUserSetted[0];
		sConfig.nFlagUserSetted[1] = psPara->nFlagUserSetted[1];

		int i;
		for(i = 0; i < MOD_BIZ_MAX_USER_NUM; i++)
		{
			if((sConfig.nFlagUserSetted[i/64] >> (i%64)) & 1)
			{
				strcpy(sConfig.UserPara[i].UserName, psPara->sUserPara[i].UserName);
				strcpy(sConfig.UserPara[i].Password, psPara->sUserPara[i].Password);
				strcpy(sConfig.UserPara[i].GroupBelong, psPara->sUserPara[i].GroupBelong);
				strcpy(sConfig.UserPara[i].UserDescript, psPara->sUserPara[i].UserDescript);
				sConfig.UserPara[i].emLoginTwince = psPara->sUserPara[i].emLoginTwince;
				sConfig.UserPara[i].emIsBindPcMac = psPara->sUserPara[i].emIsBindPcMac;
				sConfig.UserPara[i].PcMacAddress = psPara->sUserPara[i].PcMacAddress;
				int j;
				for(j = 0; j < sizeof(sConfig.UserPara[i].UserAuthor.nAuthor); j++)
				{
					sConfig.UserPara[i].UserAuthor.nAuthor[j] = psPara->sUserPara[i].UserAuthor.nAuthor[j];
				}
				for(j = 0; j < 4; j++)
				{
					sConfig.UserPara[i].UserAuthor.nLiveView[j] = psPara->sUserPara[i].UserAuthor.nLiveView[j];
				}
				for(j = 0; j < 4; j++)
				{
					sConfig.UserPara[i].UserAuthor.nRemoteView[j] = psPara->sUserPara[i].UserAuthor.nRemoteView[j];
				}
				for(j = 0; j < 4; j++)
				{
					sConfig.UserPara[i].UserAuthor.nRecord[j] = psPara->sUserPara[i].UserAuthor.nRecord[j];
				}
				for(j = 0; j < 4; j++)
				{
					sConfig.UserPara[i].UserAuthor.nPlayback[j] = psPara->sUserPara[i].UserAuthor.nPlayback[j];
				}
				for(j = 0; j < 4; j++)
				{
					sConfig.UserPara[i].UserAuthor.nBackUp[j] = psPara->sUserPara[i].UserAuthor.nBackUp[j];
				}
				for(j = 0; j < 4; j++)
				{
					sConfig.UserPara[i].UserAuthor.nPtzCtrl[j] = psPara->sUserPara[i].UserAuthor.nPtzCtrl[j];
				}				
			}
		}
		
		for(i = 0; i < MOD_BIZ_MAX_GROUP_NUM; i++)
		{
			if((sConfig.nFlagGroupSetted >> i) & 1)
			{
				strcpy(sConfig.GroupPara[i].GroupName, psPara->sGroupPara[i].GroupName);
				strcpy(sConfig.GroupPara[i].GroupDescript, psPara->sGroupPara[i].GroupDescript);
				sConfig.GroupPara[i].nDoWithUser = psPara->sGroupPara[i].nDoWithUser;
				int j;
				for(j = 0; j < sizeof(sConfig.GroupPara[i].GroupAuthor.nAuthor); j++)
				{
					sConfig.GroupPara[i].GroupAuthor.nAuthor[j] = psPara->sGroupPara[i].GroupAuthor.nAuthor[j];
				}
				for(j = 0; j < 4; j++)
				{
					sConfig.GroupPara[i].GroupAuthor.nLiveView[j] = psPara->sGroupPara[i].GroupAuthor.nLiveView[j];
				}
				for(j = 0; j < 4; j++)
				{
					sConfig.GroupPara[i].GroupAuthor.nRemoteView[j] = psPara->sGroupPara[i].GroupAuthor.nRemoteView[j];
				}
				for(j = 0; j < 4; j++)
				{
					sConfig.GroupPara[i].GroupAuthor.nRecord[j] = psPara->sGroupPara[i].GroupAuthor.nRecord[j];
				}
				for(j = 0; j < 4; j++)
				{
					sConfig.GroupPara[i].GroupAuthor.nPlayback[j] = psPara->sGroupPara[i].GroupAuthor.nPlayback[j];
				}
				for(j = 0; j < 4; j++)
				{
					sConfig.GroupPara[i].GroupAuthor.nBackUp[j] = psPara->sGroupPara[i].GroupAuthor.nBackUp[j];
				}
				for(j = 0; j < 4; j++)
				{
					sConfig.GroupPara[i].GroupAuthor.nPtzCtrl[j] = psPara->sGroupPara[i].GroupAuthor.nPtzCtrl[j];
				}				
			}
		}
		
		SBizManager* psBizManager = &g_sBizManager;
		
		if(NULL == (psBizManager->hUser = ModUserInit(&sConfig)))
		{
			return -1;
		}
		//printf("psBizManager->hUser = %lld, NULL = %d\n", psBizManager->hUser, NULL);
	}
	
	return 0;
}


s32 UserLogin(SBizUserLoginPara* para, u32* nUserId)
{
	if(para == NULL)
	{
		return -1;
	}

	SBizManager* psBizManager = &g_sBizManager;
	SModUserLoginPara sPara;
	strcpy(sPara.UserName, para->UserName);
	strcpy(sPara.Password, para->Password);
	sPara.emFrom = para->emFrom;
	sPara.nPcIp = para->nPcIp;
	sPara.nPcMac = para->nPcMac;
	//printf("psBizManager->hUser = %lld, NULL = %d\n", psBizManager->hUser, NULL);
	return ModUserLogin(psBizManager->hUser,&sPara,nUserId);
}

s32 UserLogout(u32 nUserId)	
{
	SBizManager* psBizManager = &g_sBizManager;
	return ModUserLogout(psBizManager->hUser,nUserId);
}

EMBIZUSERCHECKAUTHOR UserCheckAuthor(EMBIZUSEROPERATION emOperation, u32* para, u32 nUserId)
{
	SBizManager* psBizManager = &g_sBizManager;
	return ModUserCheckAuthor(psBizManager->hUser,emOperation,para,nUserId);
}

s32 UserAddGroup(SBizUserGroupPara* para)
{
	if(para == NULL)
	{
		return -1;
	}
	
	SModUserGroupPara GroupPara;
	
	strcpy(GroupPara.GroupName, para->GroupName);
	strcpy(GroupPara.GroupDescript, para->GroupDescript);
	GroupPara.nDoWithUser = para->nDoWithUser;
	int j;
	for(j = 0; j < sizeof(GroupPara.GroupAuthor.nAuthor); j++)
	{
		GroupPara.GroupAuthor.nAuthor[j] = para->GroupAuthor.nAuthor[j];
	}
	for(j = 0; j < 4; j++)
	{
		GroupPara.GroupAuthor.nLiveView[j] = para->GroupAuthor.nLiveView[j];
	}
	for(j = 0; j < 4; j++)
	{
		GroupPara.GroupAuthor.nRemoteView[j] = para->GroupAuthor.nRemoteView[j];
	}
	for(j = 0; j < 4; j++)
	{
		GroupPara.GroupAuthor.nRecord[j] = para->GroupAuthor.nRecord[j];
	}
	for(j = 0; j < 4; j++)
	{
		GroupPara.GroupAuthor.nPlayback[j] = para->GroupAuthor.nPlayback[j];
	}
	for(j = 0; j < 4; j++)
	{
		GroupPara.GroupAuthor.nBackUp[j] = para->GroupAuthor.nBackUp[j];
	}
	for(j = 0; j < 4; j++)
	{
		GroupPara.GroupAuthor.nPtzCtrl[j] = para->GroupAuthor.nPtzCtrl[j];
	}

	SBizManager* psBizManager = &g_sBizManager;
	return ModUserAddGroup(psBizManager->hUser,&GroupPara);
}

s32 UserModifyGroup(SBizUserGroupPara* para)
{
	if(para == NULL)
	{
		return -1;
	}
	
	SModUserGroupPara GroupPara;
	
	strcpy(GroupPara.GroupName, para->GroupName);
	strcpy(GroupPara.GroupDescript, para->GroupDescript);
	GroupPara.nDoWithUser = para->nDoWithUser;
	int j;
	for(j = 0; j < sizeof(GroupPara.GroupAuthor.nAuthor); j++)
	{
		GroupPara.GroupAuthor.nAuthor[j] = para->GroupAuthor.nAuthor[j];
	}
	for(j = 0; j < 4; j++)
	{
		GroupPara.GroupAuthor.nLiveView[j] = para->GroupAuthor.nLiveView[j];
	}
	for(j = 0; j < 4; j++)
	{
		GroupPara.GroupAuthor.nRemoteView[j] = para->GroupAuthor.nRemoteView[j];
	}
	for(j = 0; j < 4; j++)
	{
		GroupPara.GroupAuthor.nRecord[j] = para->GroupAuthor.nRecord[j];
	}
	for(j = 0; j < 4; j++)
	{
		GroupPara.GroupAuthor.nPlayback[j] = para->GroupAuthor.nPlayback[j];
	}
	for(j = 0; j < 4; j++)
	{
		GroupPara.GroupAuthor.nBackUp[j] = para->GroupAuthor.nBackUp[j];
	}
	for(j = 0; j < 4; j++)
	{
		GroupPara.GroupAuthor.nPtzCtrl[j] = para->GroupAuthor.nPtzCtrl[j];
	}

	SBizManager* psBizManager = &g_sBizManager;
	return ModUserModifyGroup(psBizManager->hUser,&GroupPara);
}

s32 UserDeleteGroup(s8* pGroupName)
{
	SBizManager* psBizManager = &g_sBizManager;
	return ModUserDeleteGroup(psBizManager->hUser,pGroupName);
}

s32 UserGetGroupList(SBizUserListPara* para)//(s8** list, s32* nLength, s32* nRealLength)
{
	SBizManager* psBizManager = &g_sBizManager;
	return ModUserGetGroupList(psBizManager->hUser,para->list,&para->nLength,&para->nRealLength);
}

s32 UserGetGroupInfo(SBizUserGroupPara* para)
{
	if(para == NULL)
	{
		return -1;
	}
	
	SModUserGroupPara GroupPara;
	memset(&GroupPara, 0, sizeof(SModUserGroupPara));
	SBizManager* psBizManager = &g_sBizManager;
	if(0 != ModUserGetGroupInfo(psBizManager->hUser,para->GroupName,&GroupPara))
	{
		return -1;
	}
	
	//strcpy(para->GroupName, GroupPara.GroupName);
	strcpy(para->GroupDescript, GroupPara.GroupDescript);
	para->nDoWithUser = GroupPara.nDoWithUser;
	int j;
	for(j = 0; j < sizeof(GroupPara.GroupAuthor.nAuthor); j++)
	{
		para->GroupAuthor.nAuthor[j] = GroupPara.GroupAuthor.nAuthor[j];
	}
	for(j = 0; j < 4; j++)
	{
		para->GroupAuthor.nLiveView[j] = GroupPara.GroupAuthor.nLiveView[j];
	}
	for(j = 0; j < 4; j++)
	{
		para->GroupAuthor.nRemoteView[j] = GroupPara.GroupAuthor.nRemoteView[j];
	}
	for(j = 0; j < 4; j++)
	{
		para->GroupAuthor.nRecord[j] = GroupPara.GroupAuthor.nRecord[j];
	}
	for(j = 0; j < 4; j++)
	{
		para->GroupAuthor.nPlayback[j] = GroupPara.GroupAuthor.nPlayback[j];
	}
	for(j = 0; j < 4; j++)
	{
		para->GroupAuthor.nBackUp[j] = GroupPara.GroupAuthor.nBackUp[j];
	}
	for(j = 0; j < 4; j++)
	{
		para->GroupAuthor.nPtzCtrl[j] = GroupPara.GroupAuthor.nPtzCtrl[j];
	}

	return 0;
}

s32 UserAddUser(SBizUserUserPara* para)
{
	if(para == NULL)
	{
		return -1;
	}
	
	SModUserUserPara UserPara;
	
	strcpy(UserPara.UserName, para->UserName);
	strcpy(UserPara.Password, para->Password);
	strcpy(UserPara.GroupBelong, para->GroupBelong);
	strcpy(UserPara.UserDescript, para->UserDescript);
	UserPara.emLoginTwince = para->emLoginTwince;
	UserPara.emIsBindPcMac = para->emIsBindPcMac;
	UserPara.PcMacAddress = para->PcMacAddress;
	int j;
	for(j = 0; j < sizeof(UserPara.UserAuthor.nAuthor); j++)
	{
		UserPara.UserAuthor.nAuthor[j] = para->UserAuthor.nAuthor[j];
	}
	for(j = 0; j < 4; j++)
	{
		UserPara.UserAuthor.nLiveView[j] = para->UserAuthor.nLiveView[j];
	}
	for(j = 0; j < 4; j++)
	{
		UserPara.UserAuthor.nRemoteView[j] = para->UserAuthor.nRemoteView[j];
	}
	for(j = 0; j < 4; j++)
	{
		UserPara.UserAuthor.nRecord[j] = para->UserAuthor.nRecord[j];
	}
	for(j = 0; j < 4; j++)
	{
		UserPara.UserAuthor.nPlayback[j] = para->UserAuthor.nPlayback[j];
	}
	for(j = 0; j < 4; j++)
	{
		UserPara.UserAuthor.nBackUp[j] = para->UserAuthor.nBackUp[j];
	}
	for(j = 0; j < 4; j++)
	{
		UserPara.UserAuthor.nPtzCtrl[j] = para->UserAuthor.nPtzCtrl[j];
	}

	SBizManager* psBizManager = &g_sBizManager;
	return ModUserAddUser(psBizManager->hUser,&UserPara);
}

s32 UserModifyUser(SBizUserUserPara* para)
{
	if(para == NULL)
	{
		return -1;
	}
	
	SModUserUserPara UserPara;
	
	strcpy(UserPara.UserName, para->UserName);
	strcpy(UserPara.Password, para->Password);
	strcpy(UserPara.GroupBelong, para->GroupBelong);
	strcpy(UserPara.UserDescript, para->UserDescript);
	UserPara.emLoginTwince = para->emLoginTwince;
	UserPara.emIsBindPcMac = para->emIsBindPcMac;
	UserPara.PcMacAddress = para->PcMacAddress;
	
	int j;
	for(j = 0; j < sizeof(UserPara.UserAuthor.nAuthor); j++)
	{
		UserPara.UserAuthor.nAuthor[j] = para->UserAuthor.nAuthor[j];
	}
	for(j = 0; j < 4; j++)
	{
		UserPara.UserAuthor.nLiveView[j] = para->UserAuthor.nLiveView[j];
	}
	for(j = 0; j < 4; j++)
	{
		UserPara.UserAuthor.nRemoteView[j] = para->UserAuthor.nRemoteView[j];
	}
	for(j = 0; j < 4; j++)
	{
		UserPara.UserAuthor.nRecord[j] = para->UserAuthor.nRecord[j];
	}
	for(j = 0; j < 4; j++)
	{
		UserPara.UserAuthor.nPlayback[j] = para->UserAuthor.nPlayback[j];
	}
	for(j = 0; j < 4; j++)
	{
		UserPara.UserAuthor.nBackUp[j] = para->UserAuthor.nBackUp[j];
	}
	for(j = 0; j < 4; j++)
	{
		UserPara.UserAuthor.nPtzCtrl[j] = para->UserAuthor.nPtzCtrl[j];
	}
	
	SBizManager* psBizManager = &g_sBizManager;
	return ModUserModifyUser(psBizManager->hUser,&UserPara);
}

s32 UserDeleteUser(s8* pUserName)
{
	SBizManager* psBizManager = &g_sBizManager;
	return ModUserDeleteUser(psBizManager->hUser,pUserName);
}

s32 UserGetListUserRegisted(SBizUserListPara* para)//(s8** list, s32* nLength, s32* nRealLength)
{
	if(para == NULL)
	{
		return -1;
	}
	
	SBizManager* psBizManager = &g_sBizManager;
	return ModUserGetListUserRegisted(psBizManager->hUser,para->list,&para->nLength,&para->nRealLength);
}

s32 UserGetUserInfo(SBizUserUserPara* para)
{
	if(para == NULL)
	{
		return -1;
	}
	
	SModUserUserPara UserPara;
	memset(&UserPara, 0, sizeof(SModUserUserPara));
	SBizManager* psBizManager = &g_sBizManager;
	if(0 != ModUserGetUserInfo(psBizManager->hUser,para->UserName,&UserPara))
	{
		return -1;
	}
	
	//strcpy(para->UserName, UserPara.UserName);
	strcpy(para->Password, UserPara.Password);
	strcpy(para->GroupBelong, UserPara.GroupBelong);
	strcpy(para->UserDescript, UserPara.UserDescript);
	para->emLoginTwince = UserPara.emLoginTwince;
	para->emIsBindPcMac = UserPara.emIsBindPcMac;
	para->PcMacAddress = UserPara.PcMacAddress;
	int j;
	for(j = 0; j < sizeof(UserPara.UserAuthor.nAuthor); j++)
	{
		para->UserAuthor.nAuthor[j] = UserPara.UserAuthor.nAuthor[j];
	}
	for(j = 0; j < 4; j++)
	{
		para->UserAuthor.nLiveView[j] = UserPara.UserAuthor.nLiveView[j];
	}
	for(j = 0; j < 4; j++)
	{
		para->UserAuthor.nRemoteView[j] = UserPara.UserAuthor.nRemoteView[j];
	}
	for(j = 0; j < 4; j++)
	{
		para->UserAuthor.nRecord[j] = UserPara.UserAuthor.nRecord[j];
	}
	for(j = 0; j < 4; j++)
	{
		para->UserAuthor.nPlayback[j] = UserPara.UserAuthor.nPlayback[j];
	}
	for(j = 0; j < 4; j++)
	{
		para->UserAuthor.nBackUp[j] = UserPara.UserAuthor.nBackUp[j];
	}
	for(j = 0; j < 4; j++)
	{
		para->UserAuthor.nPtzCtrl[j] = UserPara.UserAuthor.nPtzCtrl[j];
	}

	return 0;
}

s32 UserUpdatePara(void)
{
	SBizUserPara para;
	SBizUserPara* psPara = &para;
	SModUserPara sConfig;
	SBizManager* psBizManager = &g_sBizManager;
	if(0 == ModUserUpdatePara(psBizManager->hUser,&sConfig))
	{
		psPara->nFlagGroupSetted = sConfig.nFlagGroupSetted;
		psPara->nFlagUserSetted[0] = sConfig.nFlagUserSetted[0];
		psPara->nFlagUserSetted[1] = sConfig.nFlagUserSetted[1];
		
		int i;
		for(i = 0; i < MOD_BIZ_MAX_USER_NUM; i++)
		{
			if((sConfig.nFlagUserSetted[i/64] >> (i%64)) & 1)
			{
				strcpy(psPara->sUserPara[i].UserName, sConfig.UserPara[i].UserName);
				strcpy(psPara->sUserPara[i].Password, sConfig.UserPara[i].Password);
				strcpy(psPara->sUserPara[i].GroupBelong, sConfig.UserPara[i].GroupBelong);
				strcpy(psPara->sUserPara[i].UserDescript, sConfig.UserPara[i].UserDescript);
				psPara->sUserPara[i].emLoginTwince = sConfig.UserPara[i].emLoginTwince;
				psPara->sUserPara[i].emIsBindPcMac = sConfig.UserPara[i].emIsBindPcMac;
				psPara->sUserPara[i].PcMacAddress = sConfig.UserPara[i].PcMacAddress;
				int j;
				for(j = 0; j < sizeof(sConfig.UserPara[i].UserAuthor.nAuthor); j++)
				{
					psPara->sUserPara[i].UserAuthor.nAuthor[j] = sConfig.UserPara[i].UserAuthor.nAuthor[j];
				}
				for(j = 0; j < 4; j++)
				{
					psPara->sUserPara[i].UserAuthor.nLiveView[j] = sConfig.UserPara[i].UserAuthor.nLiveView[j];
				}
				for(j = 0; j < 4; j++)
				{
					psPara->sUserPara[i].UserAuthor.nRemoteView[j] = sConfig.UserPara[i].UserAuthor.nRemoteView[j];
				}
				for(j = 0; j < 4; j++)
				{
					psPara->sUserPara[i].UserAuthor.nRecord[j] = sConfig.UserPara[i].UserAuthor.nRecord[j];
				}
				for(j = 0; j < 4; j++)
				{
					psPara->sUserPara[i].UserAuthor.nPlayback[j] = sConfig.UserPara[i].UserAuthor.nPlayback[j];
				}
				for(j = 0; j < 4; j++)
				{
					psPara->sUserPara[i].UserAuthor.nBackUp[j] = sConfig.UserPara[i].UserAuthor.nBackUp[j];
				}
				for(j = 0; j < 4; j++)
				{
					psPara->sUserPara[i].UserAuthor.nPtzCtrl[j] = sConfig.UserPara[i].UserAuthor.nPtzCtrl[j];
				}				
			}
		}
		
		for(i = 0; i < MOD_BIZ_MAX_GROUP_NUM; i++)
		{
			if((sConfig.nFlagGroupSetted >> i) & 1)
			{
				strcpy(psPara->sGroupPara[i].GroupName, sConfig.GroupPara[i].GroupName);
				strcpy(psPara->sGroupPara[i].GroupDescript, sConfig.GroupPara[i].GroupDescript);
				psPara->sGroupPara[i].nDoWithUser = sConfig.GroupPara[i].nDoWithUser;
				int j;
				for(j = 0; j < sizeof(sConfig.GroupPara[i].GroupAuthor.nAuthor); j++)
				{
					psPara->sGroupPara[i].GroupAuthor.nAuthor[j] = sConfig.GroupPara[i].GroupAuthor.nAuthor[j];
				}
				for(j = 0; j < 4; j++)
				{
					psPara->sGroupPara[i].GroupAuthor.nLiveView[j] = sConfig.GroupPara[i].GroupAuthor.nLiveView[j];
				}
				for(j = 0; j < 4; j++)
				{
					psPara->sGroupPara[i].GroupAuthor.nRemoteView[j] = sConfig.GroupPara[i].GroupAuthor.nRemoteView[j];
				}
				for(j = 0; j < 4; j++)
				{
					psPara->sGroupPara[i].GroupAuthor.nRecord[j] = sConfig.GroupPara[i].GroupAuthor.nRecord[j];
				}
				for(j = 0; j < 4; j++)
				{
					psPara->sGroupPara[i].GroupAuthor.nPlayback[j] = sConfig.GroupPara[i].GroupAuthor.nPlayback[j];
				}
				for(j = 0; j < 4; j++)
				{
					psPara->sGroupPara[i].GroupAuthor.nBackUp[j] = sConfig.GroupPara[i].GroupAuthor.nBackUp[j];
				}
				for(j = 0; j < 4; j++)
				{
					psPara->sGroupPara[i].GroupAuthor.nPtzCtrl[j] = sConfig.GroupPara[i].GroupAuthor.nPtzCtrl[j];
				}				
			}
		}
		
		return ConfigSetUserPara(psPara);
	}
	
	return -1;
}

