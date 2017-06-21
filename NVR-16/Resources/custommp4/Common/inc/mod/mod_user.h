#ifndef _MOD_USER_H_
#define _MOD_USER_H_


/*************Êèê‰æõÁªô‰∏öÂä°Â±ÇÁöÑ*****************/

#define USER_GROUP_NAME_MAX_LENGTH 15		//”√ªß√˚°¢◊È√˚À˘‘ –Ìµƒ◊Ó¥Û≥§∂»
#define USER_GROUP_PASSWORD_MAX_LENGTH 15	//”√ªß√‹¬Îµƒ◊Ó¥Û≥§∂»
#define MOD_USER_MAX_GROUP_NUM 16			//À˘÷ß≥÷µƒ◊Ó¥Ûµƒ”√ªß◊È ˝ƒø
#define MOD_USER_MAX_USER_NUM 8//128			//À˘÷ß≥÷µƒ◊Ó¥Ûµƒ”√ªß ˝ƒø

typedef void* UserCenter;

typedef enum
{
	EM_MODUSER_LOGINFROM_GUI = 0,
	EM_MODUSER_LOGINFROM_WEB,
	EM_MODUSER_LOGINFROM_CONSOLE,
	//
}EMMODUSERLOGINFROM;

typedef struct
{
	u8 UserName[USER_GROUP_NAME_MAX_LENGTH];
	u8 Password[USER_GROUP_PASSWORD_MAX_LENGTH];
	EMMODUSERLOGINFROM emFrom;
	u32 nPcIp;
	u64 nPcMac;
}SModUserLoginPara;

typedef enum //“‘œ¬¥¯”–◊¢ Õµƒ « µº ”√µΩµƒ£¨«“√ª”–Ω¯––µ•Õ®µ¿øÿ÷∆
{
	EM_MOD_USER_AUTHOR_SHUTDOWN = 0,		//πÿª˙
	EM_MOD_USER_AUTHOR_SYSTEM_RESET,		//≥ˆ≥ß…Ë÷√
	EM_MOD_USER_AUTHOR_SYSTEM_UPDATE,		//œµÕ≥…˝º∂
	EM_MOD_USER_AUTHOR_SYSTEM_MAINTAIN,
	EM_MOD_USER_AUTHOR_USER_MANAGE,	
	EM_MOD_USER_AUTHOR_AUDIO_TALK,			//”Ô“Ù∂‘Ω≤
	EM_MOD_USER_AUTHOR_DISK_MANAGE,			//”≤≈Ãπ‹¿Ì
	EM_MOD_USER_AUTHOR_DATA_MANAGE,			// ˝æ›π‹¿Ì
	EM_MOD_USER_AUTHOR_SYSINFO_VIEW,
	EM_MOD_USER_AUTHOR_LOG_VIEW,			//»’÷æ
	EM_MOD_USER_AUTHOR_LOG_DELETE,
	EM_MOD_USER_AUTHOR_LOG_BACKUP,	
	EM_MOD_USER_AUTHOR_REMOTE_LOGIN,		//‘∂≥Ãµ«¬º
	EM_MOD_USER_AUTHOR_CHN_NAME_SETUP,
	EM_MOD_USER_AUTHOR_SYSTEM_SETUP,		//œµÕ≥≈‰÷√
	EM_MOD_USER_AUTHOR_NORMAL_SETUP,
	EM_MOD_USER_AUTHOR_RECORD_SETUP,
	EM_MOD_USER_AUTHOR_ENCODE_SETUP,
	EM_MOD_USER_AUTHOR_PTZ_SETUP,
	EM_MOD_USER_AUTHOR_ALARM_SETUP,
	EM_MOD_USER_AUTHOR_NETWORK_SETUP,
	EM_MOD_USER_AUTHOR_VIDEOMOTION_SETUP,
	//
	EM_MOD_USER_AUTHOR_LIVEVIEW = 50,
	EM_MOD_USER_AUTHOR_REMOTEVIEW,
	EM_MOD_USER_AUTHOR_RECORD,				// ÷∂Ø¬ºœÒ
	EM_MOD_USER_AUTHOR_PLAYBACK,			//ªÿ∑≈
	EM_MOD_USER_AUTHOR_RECFILE_BACKUP,
	EM_MOD_USER_AUTHOR_PTZCTRL,				//‘∆Ã®øÿ÷∆
	//
	EM_MOD_USER_AUTHOR_NULL = 64,
}EMMODUSEROPERATE;

typedef enum						//return value from ModUserCheckAuthor()
{
	EM_MODUSER_AUTHOR_ERROR = -1,	//check failed
	EM_MODUSER_AUTHOR_NO = 0,		//user has no author
	EM_MODUSER_AUTHOR_YES = 1,		//user has author
}EMMODUSERIFAUTHOR;

typedef enum						//return value from UserCheckLoginPara()
{
	EM_MODUSER_LOGIN_ERR_NOTHISUSER = -2,
	EM_MODUSER_LOGIN_ERR_WRONGPASSWD = -3,
	EM_MODUSER_LOGIN_ERR_NOLOGINTWINCE = -4,
	EM_MODUSER_LOGIN_ERR_IP_NOPERMIT = -5,
	EM_MODUSER_LOGIN_ERR_MAC_NOPERMIT = -6,
	EM_MODUSER_LOGIN_ERR_NO_PERMIT_WEB_LOGIN = -7,
	//
	EM_MODUSER_LOGIN_ERR_SUPER_USER = -100,
	//
}EMMODUSERLOGINERR;

typedef enum						//return value from ModUserAddUser()
{
	EM_MODUSER_ADDUSER_INVALID_NAME = -2,
	EM_MODUSER_ADDUSER_EXIST_NAME = -3,		
	EM_MODUSER_ADDUSER_TOMAXNUM = -4,	
}EMMODUSERADDUSERERR;

typedef enum						//return value from ModUserDeleteUser
{
	EM_MODUSER_DELUSER_INVALID_NAME = -2,
	EM_MODUSER_DELUSER_USER_LOGINED = -3,	
}EMMODUSERDELUSERERR;

typedef struct
{
	u8 nAuthor[64];
	u64 nLiveView[4];
	u64 nRemoteView[4];
	u64 nRecord[4];
	u64 nPlayback[4];
	u64 nBackUp[4];
	u64 nPtzCtrl[4];
}SModUserAuthority;

typedef struct
{
	u8 GroupName[USER_GROUP_NAME_MAX_LENGTH];
	u8 GroupDescript[USER_GROUP_NAME_MAX_LENGTH<<1];
	u8 nDoWithUser;	//∂‘◊ÈΩ¯–––ﬁ∏ƒ∫Õ…æ≥˝ ±£¨ «∑Ò π≤Ÿ◊˜”¶”√µΩ∏√◊Èœ¬µƒÀ˘”–”√ªß£ª[0]:∑Ò£¨[∑«0]: «
	SModUserAuthority GroupAuthor;
}SModUserGroupPara;

typedef enum
{
	EM_MODUSER_BIND_PCMAC_NO = 0,
	EM_MODUSER_BIND_PCMAC_YES = 1,
}EMMODUSERBINDPCMAC;

typedef enum
{
	EM_MODUSER_LOGINTWINCE_YES = 0,
	EM_MODUSER_LOGINTWINCE_NO = 1,
}EMMODUSERLOGINTWINCE;

typedef struct
{
	u8 UserName[USER_GROUP_NAME_MAX_LENGTH];
	u8 Password[USER_GROUP_PASSWORD_MAX_LENGTH];
	u8 GroupBelong[USER_GROUP_NAME_MAX_LENGTH];
	u8 UserDescript[USER_GROUP_NAME_MAX_LENGTH<<1];
	EMMODUSERLOGINTWINCE emLoginTwince;
	SModUserAuthority UserAuthor;
	EMMODUSERBINDPCMAC emIsBindPcMac;
	u64 PcMacAddress;
}SModUserUserPara;

typedef struct
{
	u64 nFlagGroupSetted;
	u64 nFlagUserSetted[2];
	SModUserGroupPara GroupPara[MOD_USER_MAX_GROUP_NUM];
	SModUserUserPara UserPara[MOD_USER_MAX_USER_NUM];
}SModUserPara;


typedef enum				//∫⁄∞◊√˚µ•¿‡–Õ√∂æŸ
{
	EM_MODUSER_NONELIST = 0,//Œﬁ–ß
	EM_MODUSER_BLACKLIST,	//∫⁄√˚µ•
	EM_MODUSER_WHITELIST,	//∞◊√˚µ•
}EMMODUSERIPLISTTYPE;

typedef struct
{
	u64 nFirstIp;
	u64 nLastIp;
}SModUserIpSeg;

typedef struct
{
	EMMODUSERIPLISTTYPE emBlackOrWhite;
	SModUserIpSeg sList[10];
}SModUserBlackWhiteList;


UserCenter ModUserInit(SModUserPara* para);
s32 ModUserDeinit(UserCenter pCenter);
s32 ModUserLogin(UserCenter pCenter, SModUserLoginPara* para, u32* nUserId);
s32 ModUserLogout(UserCenter pCenter, u32 nUserId);
EMMODUSERIFAUTHOR ModUserCheckAuthor(UserCenter pCenter, EMMODUSEROPERATE emOperation, u32* para, u32 nUserId);
s32 ModUserAddGroup(UserCenter pCenter, SModUserGroupPara* para);
s32 ModUserModifyGroup(UserCenter pCenter, SModUserGroupPara* para);
s32 ModUserDeleteGroup(UserCenter pCenter, s8* pGroupName);
s32 ModUserGetGroupList(UserCenter pCenter, s8** list, s32* nLength, s32* nRealLength);
s32 ModUserGetGroupInfo(UserCenter pCenter, s8* pGroupName, SModUserGroupPara* para);
s32 ModUserAddUser(UserCenter pCenter, SModUserUserPara* para);
s32 ModUserModifyUser(UserCenter pCenter, SModUserUserPara* para);
s32 ModUserDeleteUser(UserCenter pCenter, s8* pUserName);
s32 ModUserGetListUserRegisted(UserCenter pCenter, s8** list, s32* nLength, s32* nRealLength);
//s32 ModUserGetListUserLogined(s8** list, s32* nLength, s32* nRealLength);
s32 ModUserGetUserInfo(UserCenter pCenter, s8* pUserName, SModUserUserPara* para);
s32 ModUserUpdatePara(UserCenter pCenter, SModUserPara* para);
//s32 ModUserLockUser(UserCenter pCenter, u32 nUserId);
//s32 ModUserUnlockUser(UserCenter pCenter, u32 nUserId);
s32 ModUserSetBlackWhiteList(UserCenter pCenter, SModUserBlackWhiteList* pList);

/********************‰∏öÂä°Â±ÇÊèê‰æõÁªìÊùü***************************/

#endif //_MOD_USER_H_
