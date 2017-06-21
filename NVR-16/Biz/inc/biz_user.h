#ifndef _BIZ_USER_H_
#define _BIZ_USER_H_

#include "biz.h"
#include "mod_user.h"
#include "biz_manager.h"

s32 UserInit(u32 nEnable, SBizUserPara* psPara);
s32 UserLogin(SBizUserLoginPara* para, u32* nUserId);
s32 UserLogout(u32 nUserId);
EMBIZUSERCHECKAUTHOR UserCheckAuthor(EMBIZUSEROPERATION emOperation, u32* para, u32 nUserId);
s32 UserAddGroup(SBizUserGroupPara* para);
s32 UserModifyGroup(SBizUserGroupPara* para);
s32 UserDeleteGroup(s8* pGroupName);
s32 UserGetGroupList(SBizUserListPara* para);//(s8** list, s32* nLength, s32* nRealLength);
s32 UserGetGroupInfo(SBizUserGroupPara* para);
s32 UserAddUser(SBizUserUserPara* para);
s32 UserModifyUser(SBizUserUserPara* para);
s32 UserDeleteUser(s8* pUserName);
s32 UserGetListUserRegisted(SBizUserListPara* para);//(s8** list, s32* nLength, s32* nRealLength);
s32 UserGetUserInfo(SBizUserUserPara* para);
s32 UserUpdatePara(void);
//s32 UserSetBlackWhiteList(SBizUserBlackWhiteList* pList);

#endif //_BIZ_USER_H_

