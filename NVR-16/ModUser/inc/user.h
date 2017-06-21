#ifndef _USER_H_
#define _USER_H_

typedef struct smoduserlogininfo
{
	u32 id;
	u8 UserName[USER_GROUP_NAME_MAX_LENGTH];
	u8 IsLocked;
	EMMODUSERLOGINFROM emFrom;
	u32 nPcIp;
	struct smoduserlogininfo* pNext;
}SModUserLoginedInfo;

typedef struct
{
	SModUserPara para;
	sem_t sem;
	u8 nLoginedUserNum;
	SModUserLoginedInfo* pUserLogined;
	SModUserBlackWhiteList sBlackWhiteList;
}SModUserManager;

s32 UserGetAuthority(SModUserManager* manager, u8* name, SModUserAuthority* author);
s32 UserLogoutUserWithId(SModUserLoginedInfo** pLoginInfo, u32 id);
u8* UserGetNameFromId(SModUserLoginedInfo* pLoginInfo, u32 id);
s32 UserIsSuperKey(u8* key);
s32 UserCheckLoginIp(SModUserManager* manager, SModUserLoginPara* para);
s32 UserCheckLoginMac(SModUserUserPara* pUserPara, SModUserLoginPara* para);
s32 UserCheckLoginWebPermit(SModUserUserPara* pUserPara, SModUserLoginPara* para);
s32 UserCheckLoginTwince(SModUserManager* manager, SModUserUserPara* pUserPara);
s32 UserCheckLoginPara(SModUserManager* manager, SModUserLoginPara* para);

#endif //_USER_H_

