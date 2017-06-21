#ifndef _BIZ_ERRCODE_H_
#define _BIZ_ERRCODE_H_

#define BIZ_SUCCESS 0 //成功

#define BIZ_ERR_INIT_TOOMUCH -1 //重复初始化
#define BIZ_ERR_INIT_LACKOFMEM -2 //初始化分配业务层管理结构体内存失败
#define BIZ_ERR_INIT_DISPATCH_OPEN -3 //开启业务层调度线程失败
#define BIZ_ERR_EVENT_NULL -4 //创建回调事件为空错误
#define BIZ_ERR_INNERMSG_FAILEDCMD -6 //事件发送失败

#define BIZ_ERR_INIT_CONFIG_INIT -50 //配置模块初始化失败 
#define BIZ_ERR_CONFIG_FAILEDCMD -51 //写命令失败

#define BIZ_ERR_INIT_SYSCOMPLEX_INIT -100 //配置模块初始化失败

#define BIZ_ERR_INIT_PREVIEW_INIT -150 //配置模块初始化失败
#define BIZ_ERR_PREVIEW_NULLPARA -151 //启动预览错误；参数不能为空
#define BIZ_ERR_PREVIEW_FAILEDCMD -152 //启动预览错误；参数不能为空

#define BIZ_ERR_INIT_ENCODE_INIT -200 //编码模块初始化失败

#define BIZ_ERR_INIT_RECORD_INIT -250 //录像模块初始化失败
#define BIZ_ERR_RECORD_FAILEDCMD -251 //录像写命令失败

#define BIZ_ERR_INIT_PLAYBACK_INIT -300 //回放模块初始化失败
#define BIZ_ERR_PLAYBACK_FAILEDCMD -301 //回放写命令失败

#define BIZ_ERR_INIT_PTZ_INIT -350 //云台模块初始化失败
#define BIZ_ERR_PTZ_FAILEDCMD -351 //写命令失败

#define BIZ_ERR_INIT_ALARM_INIT -400 //报警模块初始化失败
#define BIZ_ERR_INIT_SNAP_INIT -401//图片处理逻辑初始化失败 

#define BIZ_ERR_INIT_USER_INIT -450 //用户模块初始化失败
#define BIZ_ERR_USER_AUTHOR_NO -451 //用户无权限

#define BIZ_ERR_INIT_LOG_INIT -490 //日志模块初始化失败


#define BIZ_ERR_PARASET_NULLPARA -500 //参数设置错误；参数不能为空
#define BIZ_ERR_PARASET_FAILEDCMD -501 //参数设置命令失败

#define BIZ_ERR_PARAGET_NULLPARA -600 //参数获得错误；参数不能为空
#define BIZ_ERR_PARAGET_FAILEDCMD -601 //参数设置命令失败

#define BIZ_ERR_SYSCOMPLEXDTSET_NULLPARA -700
#define BIZ_ERR_SYSCOMPLEXDTGET_NULLPARA -701
#define BIZ_ERR_SYSCOMPLEXDTSTR_NULLPARA -702

#define BIZ_ERR_DEINIT_UNKNOWN -1000 //反初始化失败,因为业务层未经初始化


#define PRINT_HERE //printf(">>>>>>>>File:%s, Func:%s, Line:%d\n", __FILE__,__FUNCTION__,__LINE__);

#endif

