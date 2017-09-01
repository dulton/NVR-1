#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netdb.h>
#include <limits.h>
#include <errno.h>

// file description
#include "CmdProc.h"
#include "Cmdcallback.h"
#include "diskmanage.h"
#include "netcommon.h"
#include "mod_config.h"

#include "biz.h"
#include "sg_platform.h"
#include "Mod_syscomplex.h"
#include "ipcamera.h"


static u8 bModInited = 0;//模块是否已经初始化
static time_t inittime = 0;

static PNetCommStateNotify pNotify = NULL;

#define RCV_TIMEOUT	(5)//秒，超时限制

extern int ConnectWithTimeout(u32 dwHostIp, u16 wHostPort, u32 dwTimeOut, u16 *pwErrorCode);


static int upload_sg_fd[2] = {-1, -1};
static int Gheart_interval_minute = 5;

static pthread_mutex_t upload_sg_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
消息头:
固定为20字节
6字节：QWCMD:
4字节：消息内容长度(包含消息头长度)
10字节：保留
*/
#define MSGSTR 

#pragma pack(push,1)	//把原来对齐方式设置压栈，并设新的设置为1

typedef struct
{
	char start[6];
	int msg_size;//包括消息头和消息体
	char reserved[10];
} MSG_HEAD;

#pragma pack(pop)    //恢复原先的对齐方式
#define BODYSIZE (256<<10)

static s32 upload_msg(int sockfd, SSG_MSG_TYPE *pmsg)
{
	MSG_HEAD head;
	memset(&head, 0, sizeof(head));
	memcpy(head.start, "QWCMD:", 6);
	char msg_body[1024];
	int ret = CTRL_SUCCESS;
	
	switch (pmsg->type)
	{
		case EM_PIC_ALARM_LINK_UPLOAD:		// 1 报警联动图像
		case EM_PIC_MAINTAIN_UPLOAD:			// 2 保养上传图像
		case EM_PIC_DAILY_UPLOAD:				// 3 日常上传图像
		case EM_PIC_TEST_UPLOAD:				// 4 测试上传图像
		case EM_PIC_ACCEPT_UPLOAD:			// 33 验收上传图像
		{
		} break;
		case EM_PIC_SERVER_GET:				// 5 主动调阅图像 1
		{
		} break;
		case EM_DVR_STARTUP:					// 19 DVR系统启动
		case EM_DVR_EXIT_NORMOAL:			// 20 DVR系统退出
		case EM_DVR_EXIT_ABNORMAL:			// 21 DVR异常退出
		case EM_DVR_EXIT_ILLEGALITY:			// 28 DVR非法退出
		{
		} break;
		case EM_DVR_PARAM_SET:				// 22 DVR参数设置
		case EM_DVR_PARAM_SAVE:				// 23 DVR参数保存
		{
		} break;
		case EM_DVR_VLOST:					// 24 DVR视频丢失
		case EM_DVR_VMOTION:					// 25 DVR移动侦测
		case EM_DVR_ALARM_EXT:				// 26 DVR外部触发
		case EM_DVR_MOTION:					// 34 偏位报警 1
		case EM_DVR_HDD_ERR:					// 41 DVR磁盘错误
		{
		} break;
		case EM_DVR_ALARM_RESUME:			// 27 系统报警解除
		{
			
		} break;
		case EM_REPAIR_CHECK:					// 29 系统维修签到
		case EM_MAINTAIN_CHECK:				// 30 系统维保签到
		{
		} break;
		case EM_DVR_PLAYBACK:					// 31 DVR本地回放
		case EM_REMOTE_PLAYBACK:				// 32 DVR远程回放
		{
		} break;
		case EM_PIC_ALARM_UPLOAD_UNREC:		// 36 报警上传图像（未录像或移动侦测）
		case EM_PIC_MAINTAIN_UPLOAD_UNREC:	// 37 保养上传图像（未录像或移动侦测）
		case EM_PIC_DAILY_UPLOAD_UNREC:		// 38 日常上传图像（当日未录像）
		case EM_PIC_TEST_UPLOAD_UNREC:		// 39 测试上传图像（未录像或移动侦测）
		case EM_PIC_ACCEPT_UPLOAD_UNREC:		// 40 验收上传图像（未录像或移动侦测）
		{
		} break;
		case EM_HEART_TIMEOUT:				// 42 系统心跳超时
		case EM_HEART_RESUME:				// 43 系统心跳恢复
		{
		} break;
		case EM_VEDIO_OTHER:					// 44视频其他事件
		{
		} break;
		default:
			printf("%s unknow cmd type: %d\n", __FUNCTION__, pmsg->type);
	}
	
	if (head.msg_size != 0)
	{
		if (loopsend(sockfd, (char *)&head, sizeof(head)) != sizeof(head))
		{
			printf("%s send msg head failed\n", __FUNCTION__);
			ret = CTRL_FAILED_NETSND;
		}
		else
		{
			if (loopsend(sockfd, (char *)&msg_body, head.msg_size-sizeof(head)) != head.msg_size-sizeof(head))
			{
				printf("%s send msg body failed\n", __FUNCTION__);
				ret = CTRL_FAILED_NETSND;
			}
		}
	}

	return ret;
}

//服务器可能没有发送回应直接关闭SOCK，此时recv返回0，本函数认为失败，
//上层逻辑忽略接收失败
static int looprecv_withtimeout(int sockfd, char *buf, unsigned int rcvsize,int ustimeout)
{
	int ret = CTRL_SUCCESS; 
	int curlen = 0;
	fd_set set;
	struct timeval timeout, starttime, curtime;
	u64 starttm, curtm;

	gettimeofday(&starttime, NULL);
	starttm = starttime.tv_sec*1000*1000 + starttime.tv_usec;
	//printf("%s starttime: %u.%u\n", __func__, starttime.tv_sec, starttime.tv_usec);

	while (curlen < rcvsize)
	{
		FD_ZERO(&set);
		FD_SET(sockfd,&set);
		//timeout.tv_sec = 0;
		//timeout.tv_usec = 10*1000;//10ms
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		ret = select(sockfd+1,&set,NULL,NULL,&timeout);
		if (ret < 0)
		{
			printf("%s select failed\n", __FUNCTION__);
			ret = CTRL_FAILED_NETRCV;
			break;
		}
		else if(ret)
		{
			if (FD_ISSET(sockfd, &set))
			{
				ret = recv(sockfd, buf+curlen, rcvsize-curlen, 0);
				if(ret < 0)
				{
					printf("%s recv failed\n", __FUNCTION__);
					ret = CTRL_FAILED_NETRCV;
					break;
				}
				else if(ret == 0)//服务器关闭了sock
				{
					ret = CTRL_FAILED_NETRCV;
					printf("%s SG-server close sock, recvlen: %d\n", __FUNCTION__, curlen);
					break;
				}

				curlen += ret;
			}
			else
			{
				printf("%s sockfd not in set\n", __FUNCTION__);
				ret = CTRL_FAILED_NETRCV;
				break;
			}
		}
		
		//成功和超时都要看看总时间是否超时
		gettimeofday(&curtime, NULL);
		curtm = curtime.tv_sec*1000*1000 + curtime.tv_usec;
		//printf("%s curtime: %u.%u\n", __func__, curtime.tv_sec, curtime.tv_usec);
		
		//if ((starttime.tv_sec + ustimeout/(1000*1000) < curtime.tv_sec) 
		//	||((starttime.tv_sec + ustimeout/(1000*1000) == curtime.tv_sec) && (starttime.tv_usec + ustimeout%(1000*1000) < curtime.tv_usec)))
		if (starttm + ustimeout < curtm) 
		{
			printf("%s recv timeout\n", __FUNCTION__);
			ret = CTRL_FAILED_TIMEOUT;
			break;
		}
	}

	if (curlen == rcvsize)
		ret = CTRL_SUCCESS;
	else
		ret = CTRL_FAILED_NETRCV;

	return ret;
}

/*
//0: 上传成功
//1: 服务器连接失败
//2: 发送失败
//3: 接收失败
//4: DNS解析失败
//5: 图片信息有误
//6: 图片base64转码失败
//7: 获取磁盘信息失败
//8: 申请内存失败
*/
//图片上传监管平台成功或失败，告知界面显示给用户
s32 netComm_NotifySGuploadState(u32 chn, EM_MSG_TYPE type, u8 bState )
{
	SNetStateNotify sNotify;

	if (!bModInited )//|| (time(NULL) - inittime < 2*60 ) )
		return 1;

	//printf("%s chn: %d, type: %d, bState: %d\n", __func__, chn, type, bState);
	
	memset(&sNotify, 0, sizeof(sNotify));
	
	sNotify.eAct 		= EM_NET_STATE_SG;
	sNotify.sRslt.state 	=  chn;
	sNotify.sRslt.nErr 	= bState;

	if (pNotify != NULL)
	{
		if ( (type == EM_PIC_MAINTAIN_UPLOAD) 
			|| (type == EM_PIC_TEST_UPLOAD)
			|| (type == EM_PIC_ACCEPT_UPLOAD) 
			|| (type == EM_REPAIR_CHECK)
			|| (type == EM_MAINTAIN_CHECK) 
			|| (type == EM_VideoMonitor_Fault)
			//||((type >= EM_SYS_Other) && (type <= EM_ElectronicPatrol_Fault))
			)
		{
			pNotify((void*)&sNotify);
		}
	}
	
	return 0;
}

//监管平台请求上传
s32 netComm_NotifySGReqSnap(u32 chn_mask)
{
	SNetStateNotify sNotify;

	if (!bModInited )//|| (time(NULL) - inittime < 2*60 ) )
		return 1;

	printf("%s chn mask: 0x%x\n", __func__, chn_mask);
	
	memset(&sNotify, 0, sizeof(sNotify));
	
	sNotify.eAct			= EM_NET_STATE_SGReqSnap;
	sNotify.SGReqChnMask	= chn_mask;

	if (pNotify != NULL)
	{
		pNotify((void*)&sNotify);		
	}
	
	return 0;
}

/*
若收到的心跳包含GetImage字段，
DVR将立刻用OperationCmd发送指定各个通道的实时图片；
OperationCmd中GUID属性填写服务器回传的GUID编号；
Type属性为5(Type=5);需上传通道为"，"隔开的通道编号。

取出通道号
<GetImage Channel="1,2,3" GUID="1C164A158FFDED4FAFC6A37DC57017BF"/>
*/
s32 proc_svr_req(char *pmsg)
{
	int ret = 0;
	char *s; 
	char *e; //start end
	int chn, chn_mask;
	
	s = strstr(pmsg, "GetImage");
	if (NULL == s)
	{
		//printf("%s \"GetImage\" not found\n", __func__);
		return 1;
	}

	s = strchr(s, '\"');  // " 上引号
	if (NULL == s)
	{
		//printf("%s \" not found\n", __func__);
		return 1;
	}

	chn_mask = 0;
	s += 1;
	do {		
		if ((*s >= '0') && (*s <= '9'))
		{
			chn = strtol(s, &e, 10);
			if ((errno == ERANGE && (chn == LONG_MAX || chn == LONG_MIN)) || (errno != 0 && chn == 0)) 
			{
				printf("%s strtol %s\n", __func__, strerror(errno));
				break;
			}

			//printf("%s strtol chn: %d\n", __func__, chn);
			chn_mask |= 1<<chn;

			if (*e != ',') //"1,2,3"  逗号间隔
			{
				break;				
			}

			s = e+1;
		}
		else
		{
			break;
		}
	} while(1);

	if (chn_mask)
	{
		ret = netComm_NotifySGReqSnap(chn_mask);
	}
	
	return ret;
}

static char SvrGuid[128];
//时间必须作为参数，可以指定，因为要上传之前的预录图片
/*
tm:
	= 0，就用当前时间
	!=0，那就图片时间
SnapData, DataSize
	图片数据位置和长度
*/
s32 upload_sg_proc(SSG_MSG_TYPE *pmsg, time_t tm, void *SnapData, unsigned int DataSize, int GUID)
{
	int ret = CTRL_SUCCESS;
	u32 dwServerIp;
	SModConfigSGParam para;
	//printf("%s type: %d\n", __func__, pmsg->type);

	//得到服务器IP
	ret = ModConfigGetParam(EM_CONFIG_PARA_SG, &para, 0);
	if (ret == 0)
	{
		if (!para.RunEnable)
		{
			return CTRL_SUCCESS;
		}
		
		//printf("%s SG Platform ip: %s\n", __func__, para.CenterIP);
	
		if ((para.CenterIP[0] >= '0') && (para.CenterIP <= '9'))//点分10进制IP
		{
			dwServerIp = (u32)inet_addr(para.CenterIP);
		}
		else
		{
			struct hostent *hptr = NULL;
			//printf("%s gethostbyname 1\n", __func__);
			if((hptr = gethostbyname(para.CenterIP)) == NULL)
			{
				printf("%s gethostbyname error for host:%s, h_errno: %d\n", __func__, para.CenterIP, h_errno);
				netComm_NotifySGuploadState(pmsg->chn, pmsg->type, 4);
				return CTRL_FAILED_PARAM;
			}
			//printf("SG ServerIP Address :%s\n",inet_ntoa(*((struct in_addr *)hptr->h_addr)));

			dwServerIp = *((u32 *)hptr->h_addr);
		}
		
		u16 wServerPort = 5901;
		u16 wErrCode;
		u32 dwTimeOut = RCV_TIMEOUT * 1000;//ms

		//建立连接
		//printf("%s connect 1, %d\n", __func__, time(NULL));
		int sockfd = ConnectWithTimeout(dwServerIp,wServerPort,dwTimeOut,&wErrCode);//88888888888888888888888888888
		if(sockfd < 0)
		{
			printf("%s connect ServreIp: %s, Port: %d failed\n",__FUNCTION__, 
				inet_ntoa(*((struct in_addr *)&dwServerIp)), wServerPort);
			//printf("%s connect 2, %d\n", __func__, time(NULL));
			netComm_NotifySGuploadState(pmsg->chn, pmsg->type, 1);
			return CTRL_FAILED_PARAM; 
		}
		//printf("%s connect ServreIp: %s, Port: %d success\n",__FUNCTION__, 
		//		inet_ntoa(*((struct in_addr *)&dwServerIp)), wServerPort);
		
#if 1
		//填充消息头
		MSG_HEAD head;
		memset(&head, 0, sizeof(head));
		memcpy(head.start, "QWCMD:", 6);
		//消息头中数据长度在之后赋值

		//填充消息体
		int len = 0;
		char *msg_body = (char *)malloc(BODYSIZE);//88888888888888888888888888888888888888888888888888
		
		if (msg_body == NULL)
		{
			printf("%s msg_body malloc failed\n", __func__);
			close(sockfd);
			netComm_NotifySGuploadState(pmsg->chn, pmsg->type, 8);
			
			return CTRL_FAILED_OUTOFMEMORY;
		}
		memset(msg_body, 0, BODYSIZE);
		//填充起始字节
		strcpy(msg_body, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
		len = strlen(msg_body);
		sprintf(msg_body+len, "<Agent ID=\"%s\" Type=\"SG\" Ver=\"1.3.0.0\">\n", para.AgentID);
		strcat(msg_body, "<GetTicks/>\n");
		
		//准备日期和时间
		SDateTime TriggerTime;
		char sTriggerTime[30];
		
		if (tm == 0)//没有指定，就用当前时间
		{
			ModSysComplexDTGet_TZ(&TriggerTime);
		}
		else	//指定了时间，可能是之前图片预录的时间
		{
			ModSysComplexConvertToDT(tm, &TriggerTime);
		}
		
		sprintf(sTriggerTime, "%d-%d-%d %d:%d:%d",\
						TriggerTime.nYear, \
						TriggerTime.nMonth, \
						TriggerTime.nDay, \
						TriggerTime.nHour, \
						TriggerTime.nMinute, \
						TriggerTime.nSecode);
		
		//准备GUID
		int tmpGUID;
		char sGUID[32];
		if (GUID == 0)
		{
			srand(time(NULL));
			do {
				tmpGUID = rand();
			} while(tmpGUID == 0);
		}
		else
		{
			tmpGUID = GUID;
		}
		
		sprintf(sGUID, "%d", tmpGUID);

		//准备Note
		//Note 转换base64
		char *tmp = NULL;
		if (pmsg->note[0] != '\0')
		{
			tmp = base64_encode_v1(pmsg->note, strlen(pmsg->note));
			if (NULL == tmp)
			{
				close(sockfd);
				free(msg_body);
				return -1;
			}
			
			strcpy(pmsg->note, tmp);
			free(tmp);
			tmp = NULL;
		}
		//printf("base64: %s\n", pmsg->note);

		len = strlen(msg_body);
		switch (pmsg->type)
		{
			case EM_PIC_ALARM_LINK_UPLOAD:		// 1 报警联动图像
			case EM_PIC_MAINTAIN_UPLOAD:		// 2 保养上传图像
			case EM_PIC_DAILY_UPLOAD:			// 3 日常上传图像
			case EM_PIC_TEST_UPLOAD:			// 4 测试上传图像
			case EM_PIC_SERVER_GET:				// 5 主动调阅图像 
			case EM_PIC_ACCEPT_UPLOAD:			// 33 验收上传图像
			{
				if (EM_PIC_SERVER_GET == pmsg->type)
				{
					strcpy(sGUID, SvrGuid);
				}

				int yg = pmsg->type;
				if (pmsg->type == EM_PIC_ALARM_LINK_UPLOAD)
				{
					//yg = 4;
					//strcpy(pmsg->note, "5rWL6K+V5Zu+5YOP5LiK5Lyg");
				}
				
				sprintf(msg_body+len, "<OperationCmd Type=\"%d\" Channel=\"%d\" TriggerTime=\"%s\" Note=\"%s\" GUID=\"%s\">\n", \
									yg, /*pmsg->type, */ \
									pmsg->chn, \
									sTriggerTime, \
									pmsg->note, \
									sGUID);
				len = strlen(msg_body);

				//填充图片
				//图片转换base64
			#if 0
				if (SnapData == NULL)
				{
					printf("%s chn%d, type: %d, SnapData == NULL\n", __func__, pmsg->chn, pmsg->type);
					close(sockfd);
					free(msg_body);
					netComm_NotifySGuploadState(pmsg->chn, pmsg->type, 5);
					return -1;
				}
				if (DataSize == 0)
				{
					printf("%s chn%d, type: %d, DataSize == 0\n", __func__, pmsg->chn, pmsg->type);
					close(sockfd);
					free(msg_body);
					netComm_NotifySGuploadState(pmsg->chn, pmsg->type, 5);
					return -1;
				}
				//printf("%s invoke base64_encode_v1\n", __func__);
				tmp = base64_encode_v1(SnapData, DataSize);
				if (NULL == tmp)
				{
					printf("%s chn%d, type: %d, base64_encode_v1 failed\n", __func__, pmsg->chn, pmsg->type);
					close(sockfd);
					free(msg_body);
					netComm_NotifySGuploadState(pmsg->chn, pmsg->type, 6);
					return -1;
				}
				//printf("type: %d,pic-base64: %d-%d\n", pmsg->type, DataSize, strlen(tmp));
				
				strcat(msg_body, tmp);
				free(tmp);
				tmp = NULL;
			#else
				if (SnapData && DataSize)
				{
					tmp = base64_encode_v1(SnapData, DataSize);
					if (NULL == tmp)
					{
						printf("%s chn%d, type: %d, base64_encode_v1 failed\n", __func__, pmsg->chn, pmsg->type);
						close(sockfd);
						free(msg_body);
						netComm_NotifySGuploadState(pmsg->chn, pmsg->type, 6);
						return -1;
					}					
					
					strcat(msg_body, tmp);
					free(tmp);
					tmp = NULL;
				}
				#if 0
				else
				{
					printf("%s type :%d snap to base64 failed, pSnapData: %p, len: %d\n", \
						__func__, pmsg->type, SnapData, DataSize);
					strcat(msg_body, "\n");
				}
				#endif
			#endif
				
				//len = strlen(msg_body);
				strcat(msg_body, "</OperationCmd>\n");
				strcat(msg_body, "</Agent>\n");
				head.msg_size = sizeof(head)+strlen(msg_body);
			} break;
			case EM_SYS_Other:					// 11 系统其他故障
			case EM_Perimeter_Fault:			// 12 周界报警故障
			case EM_Networking_Fault:			// 13 联网报警故障
			case EM_LocalAlarm_Fault:			// 14 本地报警故障
			case EM_VideoMonitor_Fault:			// 15 视频监控故障
			case EM_BuildingIntercom_Fault:		// 16 楼宇对讲故障
			case EM_AccessControl_Fault:		// 17 出入控制故障
			case EM_ElectronicPatrol_Fault:		// 18 电子巡更故障
			case EM_DVR_STARTUP:				// 19 DVR系统启动
			case EM_DVR_EXIT_NORMOAL:			// 20 DVR系统退出
			case EM_DVR_EXIT_ABNORMAL:			// 21 DVR异常退出
			case EM_DVR_PARAM_SET:				// 22 DVR参数设置
			case EM_DVR_PARAM_SAVE:				// 23 DVR参数保存
			case EM_DVR_VLOST:					// 24 DVR视频丢失
			case EM_DVR_VMOTION:				// 25 DVR移动侦测
			case EM_DVR_ALARM_EXT:				// 26 DVR外部触发
			case EM_DVR_ALARM_RESUME:			// 27 系统报警解除
			case EM_DVR_EXIT_ILLEGALITY:		// 28 DVR非法退出
			case EM_REPAIR_CHECK:				// 29 系统维修签到
			case EM_MAINTAIN_CHECK:				// 30 系统维保签到
			{
				sprintf(msg_body+len, "<OperationCmd Type=\"%d\" Channel=\"%d\" TriggerTime=\"%s\" Note=\"%s\" GUID=\"%s\">\n", \
									pmsg->type, \
									pmsg->chn, \
									sTriggerTime, \
									pmsg->note, \
									sGUID);
				strcat(msg_body, "</OperationCmd>\n");
				strcat(msg_body, "</Agent>\n");
				head.msg_size = sizeof(head)+strlen(msg_body);
				
			} break;
			
			case EM_DVR_MOTION:					// 34 偏位报警 1
			case EM_DVR_HDD_ERR:				// 41 DVR磁盘错误
			{
			} break;
			case EM_DVR_PLAYBACK:				// 31 DVR本地回放
			case EM_REMOTE_PLAYBACK:			// 32 DVR远程回放
			{
			} break;
			case EM_PIC_ALARM_UPLOAD_UNREC:		// 36 报警上传图像（未录像或移动侦测）
			case EM_PIC_MAINTAIN_UPLOAD_UNREC:	// 37 保养上传图像（未录像或移动侦测）
			case EM_PIC_DAILY_UPLOAD_UNREC:		// 38 日常上传图像（当日未录像）
			case EM_PIC_TEST_UPLOAD_UNREC:		// 39 测试上传图像（未录像或移动侦测）
			case EM_PIC_ACCEPT_UPLOAD_UNREC:	// 40 验收上传图像（未录像或移动侦测）
			{
			} break;
			case EM_HEART_TIMEOUT:				// 42 系统心跳超时
			case EM_HEART_RESUME:				// 43 系统心跳恢复
			{
			} break;
			case EM_VEDIO_OTHER:					// 44视频其他事件
			{
			} break;
			case EM_HEART:	// 心跳，非SG平台需要类型，用于本系统
			{
				//计算硬盘空间
				u64 nTotal, nFree;
				nTotal = nFree = 0;
				int i;
				SStoreDevManage sStDevMgr;
				memset( &sStDevMgr, 0, sizeof(sStDevMgr) );
				
				SDevInfo sDevInfo[64]; //zlb20111117 去掉部分malloc
				sStDevMgr.psDevList = sDevInfo;
				memset(sStDevMgr.psDevList, 0, sizeof(SDevInfo)*64);
				
				if (0 == ModSysComplexDMGetInfo(&sStDevMgr, 8) )
				{
					if(sStDevMgr.nDiskNum>0)
					{
						for( i=0; i<sStDevMgr.nDiskNum; i++ )
						{
							nTotal += sStDevMgr.psDevList[i].nTotal;
							nFree += sStDevMgr.psDevList[i].nFree;
						}
					}
					nTotal *= 1024;
					nFree *= 1024;
					//printf("upload heart nTotal: %lld, nFree: %lld\n", nTotal, nFree);
					//去掉之前加入的"<GetTicks/>\r"
					len -= strlen("<GetTicks/>\n");
					sprintf(msg_body+len, "<DVRHeart State=\"0\" TotalSpace=\"%lld\" FreeSpace=\"%lld\">\n", \
								nTotal, \
								nFree);

					strcat(msg_body, "init,hicore,pppoed\n");
					strcat(msg_body, "</DVRHeart>\n");
					strcat(msg_body, "<GetTicks/>\n");
					strcat(msg_body, "</Agent>\n");
					head.msg_size = sizeof(head)+strlen(msg_body);
				}
				else
				{
					printf("%s: ModSysComplexDMGetInfo() failed\n", __FUNCTION__);
					close(sockfd);
					free(msg_body);
					netComm_NotifySGuploadState(pmsg->chn, pmsg->type, 7);
					return -1;
				}
				
			} break;
			default:
				printf("%s unknow cmd type: %d\n", __FUNCTION__, pmsg->type);
		}
#endif
		//打印msgbody，调试
		if ( ((pmsg->type >= EM_PIC_ALARM_LINK_UPLOAD) &&(pmsg->type <= EM_PIC_TEST_UPLOAD)) \
					|| (pmsg->type == EM_PIC_ACCEPT_UPLOAD) ) 
		{
			
		}
		else
		{
			//printf("%s msgbody: \n%s\n", __func__, msg_body);
		}
		//printf("%s msgbody: \n%s\n", __func__, msg_body);
		//发送消息
		if (head.msg_size != 0)
		{
			if (loopsend(sockfd, (char *)&head, sizeof(head)) != sizeof(head))
			{
				printf("%s send msg head failed\n", __FUNCTION__);
				close(sockfd);
				free(msg_body);
				netComm_NotifySGuploadState(pmsg->chn, pmsg->type, 2);
				return CTRL_FAILED_NETSND;
			}
			else
			{
				//printf("%s msg_head success\n", __func__);
				//printf("%s msg_body size: %d\n", __func__, head.msg_size-sizeof(head));
				if (loopsend(sockfd, (char *)msg_body, head.msg_size-sizeof(head)) != head.msg_size-sizeof(head))
				{
					printf("%s send msg body failed\n", __FUNCTION__);
					close(sockfd);
					free(msg_body);
					netComm_NotifySGuploadState(pmsg->chn, pmsg->type, 2);
					return CTRL_FAILED_NETSND;
				}
				//printf("%s msg_body success\n", __func__);
			}
			
			
		}
		//printf("yg sg send msg success\n");

	//接收
		//消息头
		ret = CTRL_SUCCESS;

		//printf("%s recv head 1, %d\n", __func__, time(NULL));
		ret = looprecv_withtimeout(sockfd, (char *)&head, sizeof(head), RCV_TIMEOUT *1000*1000);
		if (ret != CTRL_SUCCESS)
		{
			printf("%s recv msg head failed\n", __FUNCTION__);
			printf("%s recv head 2, %d\n", __func__, time(NULL));
			close(sockfd);
			free(msg_body);
			//忽略接收失败
			//netComm_NotifySGuploadState(pmsg->chn, pmsg->type, 3);
 			return ret;
		}
		
		//printf("%s recv msg head success, msg_size: %d\n", __FUNCTION__, head.msg_size);
		//printf("%s recv head 2, %d\n", __func__, time(NULL));
		//消息体
		int recvsize = head.msg_size - sizeof(head);
		char *recvbuf = (char *)malloc(recvsize+1);//88888888888888888888888888888888888888888888
		if (recvbuf == NULL)
		{
			printf("%s malloc recvbuf failed\n", __FUNCTION__);
			close(sockfd);
			free(msg_body);
			//忽略接收失败
			//netComm_NotifySGuploadState(pmsg->chn, pmsg->type, 8);
			return CTRL_FAILED_OUTOFMEMORY;
		}
		recvbuf[recvsize] = '\0';//

		//printf("%s recv body 1, %d\n", __func__, time(NULL));
		ret = looprecv_withtimeout(sockfd, recvbuf, recvsize, RCV_TIMEOUT *1000*1000);
		if (ret != CTRL_SUCCESS)
		{
			printf("%s recv msg body failed\n", __FUNCTION__);
			printf("%s recv body 2, %d\n", __func__, time(NULL));
			close(sockfd);
			free(msg_body);
			free(recvbuf);

			//忽略接收失败
			//netComm_NotifySGuploadState(pmsg->chn, pmsg->type, 3);
			return ret;
		}
		//printf("%s recv msg body success\n", __FUNCTION__);
		//printf("%s recv body 2, %d\n", __func__, time(NULL));
		//printf("recv msg body: %s\n", recvbuf);
		
		char *p, *q;
		
		if (EM_HEART == pmsg->type)
		{
			p = strstr(recvbuf, "Ticks Value=");
			if (p != NULL)
			{
				Gheart_interval_minute = atoi(p+strlen("Ticks Value=")+1);
				//printf("yg Gheart_interval_minute: %d\n", Gheart_interval_minute);
			}

			//处理监管平台主动调阅图像
			proc_svr_req(recvbuf);
			//记录SvrGuid
			q = SvrGuid;
			p = strstr(recvbuf, "GUID");
			if (p != NULL)
			{
				p += strlen("GUID")+2;
				while( *p != '\"')
				{
					*q++ = *p++;
				}
				*q = '\0';
			}
			else
			{
				//printf("%s \"GUID\" not found\n", __func__);
				memset(SvrGuid, 0, sizeof(SvrGuid));
			}
		}

		
		
		close(sockfd);
		free(msg_body);
		free(recvbuf);
		
	}
	else
	{
		printf("%s: ModConfigGetParam failed\n", __FUNCTION__);
	}
	netComm_NotifySGuploadState(pmsg->chn, pmsg->type, 0);

	//printf("%s upload sg platform success\n", __func__);
	return ret;
}


void UploadSGPlatformFxn(void *para)
{
	int ret;
	SSG_MSG_TYPE msg, heartmsg;
	
	fd_set action_fds;
	struct timeval tv;
	time_t cur, nextheart;

	time(&nextheart);
	
	while(1)
	{
		FD_ZERO(&action_fds);
		FD_SET(upload_sg_fd[0], &action_fds);
		
		tv.tv_sec  = 1;
		tv.tv_usec = 0;
		
		ret = select(upload_sg_fd[0] + 1, &action_fds, NULL, NULL, &tv);

		if (ret > 0) 
		{
			if(FD_ISSET(upload_sg_fd[0], &action_fds)) 
			{
				if (read(upload_sg_fd[0], &msg, sizeof(msg)) != sizeof(msg))
				{
					printf("%s: read pipo failed\n", __FUNCTION__);
					continue;
				}
				//printf("%s msgtype: %d\n",__func__, msg.type);
				upload_sg_proc(&msg, 0, NULL, 0, 0);//只处理消息，图片上传专门处理
				/*
				if ( ((msg.type >= EM_PIC_ALARM_LINK_UPLOAD) &&(msg.type <= EM_PIC_TEST_UPLOAD)) \
					|| (msg.type == EM_PIC_ACCEPT_UPLOAD) ) 
				{
					//printf("%s pic\n",__func__);
					upload_sg_pic(&msg);
				}
				else
				{
					//printf("%s other\n",__func__);
					upload_sg_proc(&msg, NULL, 0);
				}
				*/
			}
		}
		
		time(&cur);
		if (cur > nextheart + Gheart_interval_minute*60-10)//提前10秒
		{
			nextheart = cur;
					
			memset(&heartmsg, 0, sizeof(heartmsg));
			heartmsg.type = EM_HEART;
			heartmsg.chn = 0;
			//upload_sg(&heartmsg);
			upload_sg_proc(&heartmsg, 0, NULL, 0, 0);
			
		}
	}
}

s32 net_sg_init(PSNetCommCfg pCfg)
{
	int ret, i;
	pthread_t tid;
	//SModConfigUserPara para;

	//ret = ModConfigGetParam(EM_CONFIG_PARA_SG, &para, 0);
	ret = pipe(upload_sg_fd);
	if(ret < 0)//init pipe fd for transFd
	{
		printf("upload_sg_fd pipe:failed,errno=%d,result=%s\n", errno, strerror(errno));
		return ret;
	}
	ret = pthread_create(&tid, NULL, (void*)UploadSGPlatformFxn, NULL);
	if(0 != ret)
	{
		printf("error: pthread_create,  func:%s\n", __FUNCTION__);		
		return ret;
	}

	pNotify = pCfg->pNotify;
	if ( NULL == pNotify )
	{
		printf("%s NULL == pNotify, Initialization failed\n", __func__);
		return -1;
	}

	inittime = time(NULL);
	bModInited = 1;
	
	printf("%s init ok!!!!!!!!!!!!!!\n", __func__);
	
	return ret;	
}

inline void net_write_upload_sg(SSG_MSG_TYPE *pmsg)
{
	pthread_mutex_lock(&upload_sg_mutex);
	//printf("write_pipe:key=%d\n",key);
	write(upload_sg_fd[1], pmsg, sizeof(SSG_MSG_TYPE));
	pthread_mutex_unlock(&upload_sg_mutex);
}

void upload_sg(SSG_MSG_TYPE *pmsg)
{
	if (bModInited == 0)
	{
		printf("%s msg type: %d, sg_platform module has not been initialized, wait......\n", __func__, pmsg->type);
		return;
	}
	//printf("%s msg type: %d\n", __func__, pmsg->type);
	net_write_upload_sg(pmsg);
}

//base64 编解码
#if 1
const char base[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
char *base64_encode_v1(const char* data, int data_len)
{ 
	//int data_len = strlen(data); 
	int prepare = 0; 
	int ret_len; 
	int temp = 0; 
	char *ret = NULL; 
	char *f = NULL; 
	int tmp = 0; 
	char changed[4]; 
	int i = 0; 
	ret_len = data_len / 3; 
	temp = data_len % 3; 
	
	if (temp > 0) 
	{ 
		ret_len += 1; 
	} 
	ret_len = ret_len*4 + 1; 
	ret = (char *)malloc(ret_len); 
	//printf("%s ret_len: %d\n", __func__, ret_len);
	if ( ret == NULL) 
	{ 
		printf("No enough memory.\n"); 
		return ret; 
	} 
	memset(ret, 0, ret_len); 
	f = ret; 
	while (tmp < data_len) 
	{ 
		temp = 0; 
		prepare = 0; 
		memset(changed, '\0', 4); 
		while (temp < 3) 
		{ 
		//printf("tmp = %d\n", tmp); 
			if (tmp >= data_len) 
			{ 
				break; 
			} 
			prepare = ((prepare << 8) | (data[tmp] & 0xFF)); 
			tmp++; 
			temp++; 
		} 
		prepare = (prepare<<((3-temp)*8)); 
		//printf("before for : temp = %d, prepare = %d\n", temp, prepare); 
		for (i = 0; i < 4 ;i++ ) 
		{ 
			if (temp < i) 
			{ 
				changed[i] = 0x40; 
			} 
			else 
			{ 
				changed[i] = (prepare>>((3-i)*6)) & 0x3F; 
			} 
			*f = base[changed[i]]; 
			//printf("%.2X", changed[i]); 
			f++; 
		} 
	} 
	*f = '\0'; 

	return ret; 

} 
/* */ 
static char find_pos(char ch)   
{ 
    char *ptr = (char*)strrchr(base, ch);//the last position (the only) in base[] 
    return (ptr - base); 
} 
/* */ 
char *base64_decode_v1(const char *data, int data_len) 
{ 
    int ret_len = (data_len / 4) * 3; 
    int equal_count = 0; 
    char *ret = NULL; 
    char *f = NULL; 
    int tmp = 0; 
    int temp = 0; 
    char need[3]; 
    int prepare = 0; 
    int i = 0; 
    if (*(data + data_len - 1) == '=') 
    { 
        equal_count += 1; 
    } 
    if (*(data + data_len - 2) == '=') 
    { 
        equal_count += 1; 
    } 
    if (*(data + data_len - 3) == '=') 
    {//seems impossible 
        equal_count += 1; 
    } 
    switch (equal_count) 
    { 
    case 0: 
        ret_len += 4;//3 + 1 [1 for NULL] 
        break; 
    case 1: 
        ret_len += 4;//Ceil((6*3)/8)+1 
        break; 
    case 2: 
        ret_len += 3;//Ceil((6*2)/8)+1 
        break; 
    case 3: 
        ret_len += 2;//Ceil((6*1)/8)+1 
        break; 
    } 
    ret = (char *)malloc(ret_len); 
    if (ret == NULL) 
    { 
        printf("No enough memory.\n"); 
        exit(0); 
    } 
    memset(ret, 0, ret_len); 
    f = ret; 
    while (tmp < (data_len - equal_count)) 
    { 
        temp = 0; 
        prepare = 0; 
        memset(need, 0, 4); 
        while (temp < 4) 
        { 
            if (tmp >= (data_len - equal_count)) 
            { 
                break; 
            } 
            prepare = (prepare << 6) | (find_pos(data[tmp])); 
            temp++; 
            tmp++; 
        } 
        prepare = prepare << ((4-temp) * 6); 
        for (i=0; i<3 ;i++ ) 
        { 
            if (i == temp) 
            { 
                break; 
            } 
            *f = (char)((prepare>>((2-i)*8)) & 0xFF); 
            f++; 
        } 
    } 
    *f = '\0'; 
    return ret; 
}

#endif

