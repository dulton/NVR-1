// file description
#include "biz_net.h"
#include "custommp4.h"
#include "Mod_syscomplex.h"
#include "ExcuteCmd.h"
#include "biz_snap.h"

//** macro

//** typedef 

//** local var
static SBNOnlineUser aOnlineUser[BIZ_NET_ONLINE_MAX];
static SNetCommCfg sNCCfg;

//** global var

//** local functions
static void BizNetLogWriteCB(SNetLog* pLog);
void biz_NetGetRandomMAC( char* pMAC );
static s32 bizNet_RemoteOpCB(SRemoteOpCB* para);

//** global 

void bizNet_RequestPreviewCB(STcpStreamReq* param)
{
	u8 ch;
	
	if(param==NULL)
	{
		printf("null param, cannot continue to req stream!!\n");
	}
	
	ch = param->nChn;
	
	if(EM_START==param->nAct)
	{
		switch(param->nType)
		{
			case EM_PREQ_MAIN:
			{
				//printf("request main stream:(%d,%d,%d)\n",param->nChn,param->nType,param->nAct);
	        	SendBizInnerMsgCmd(EM_BIZMSG_NET_REQ_PREVIEW_MAIN_START, (void *)param);
			} break;
			case EM_PREQ_SUB:
			{
	        	SendBizInnerMsgCmd(EM_BIZMSG_NET_REQ_PREVIEW_SUB_START, (void *)param);
			} break;
			case EM_PREQ_SUB_MB:
			{
	        	SendBizInnerMsgCmd(EM_BIZMSG_NET_REQ_PREVIEW_SUB_MB_START, (void *)param);
			} break;
			case EM_PREQ_AUDIO:
			{
	        	SendBizInnerMsgCmd(EM_BIZMSG_NET_REQ_PREVIEW_AUDIO_START, (void *)param);
			} break;
			case EM_PREQ_VOIP:
			{
	        	SendBizInnerMsgCmd(EM_BIZMSG_NET_REQ_VOIP_START, (void *)param);			
			} break;
			case EM_PREQ_THIRD://csp modify 20130423
			{
				SendBizInnerMsgCmd(EM_BIZMSG_NET_REQ_PREVIEW_THIRD_START, (void *)param);
			} break;
			default:
			{
				printf("start %d chn type %d preview act %d!\n", ch, param->nType, param->nAct);
			} break;
		}
	}
	else if(EM_STOP==param->nAct)
	{
		switch(param->nType)
		{
			case EM_PREQ_MAIN://csp modify 20130423//add
			case EM_PREQ_MAX:
			{
	        		SendBizInnerMsgCmd(EM_BIZMSG_NET_REQ_PREVIEW_STOP, (void *)param);
			} break;
			case EM_PREQ_AUDIO:
			{
	        		SendBizInnerMsgCmd(EM_BIZMSG_NET_REQ_PREVIEW_AUDIO_STOP, (void *)param);
			} break;
			case EM_PREQ_VOIP:
			{
	        		SendBizInnerMsgCmd(EM_BIZMSG_NET_REQ_VOIP_STOP, (void *)param);
			} break;
			case EM_PREQ_SUB:
			{
	        		SendBizInnerMsgCmd(EM_BIZMSG_NET_REQ_PREVIEW_SUB_STOP, (void *)param);
			} break;
			case EM_PREQ_SUB_MB:
			{
	        		SendBizInnerMsgCmd(EM_BIZMSG_NET_REQ_PREVIEW_SUB_MB_STOP, (void *)param);
			} break;
			case EM_PREQ_THIRD://csp modify 20130423
			{
					SendBizInnerMsgCmd(EM_BIZMSG_NET_REQ_PREVIEW_THIRD_STOP, (void *)param);
			} break;
			default:
			{
				printf("stop %d chn type %d preview act %d!\n", ch, param->nType, param->nAct);
			} break;
		}
	}
	else
	{
		printf("no such action to do!!\n");
	}
	//printf("!!!stream req %d chn type %d preview act %d!\n", ch, param->nType, param->nAct);
}

BOOL is_i_frame(u8 video_type)
{
	//printf("frame type=%d\n",video_type);
	if(3 == video_type)
	{
		return video_type;
	}
	else
	{
	#define FRAME_TYPE_I 5
		//return (FRAME_TYPE_I == video_type);
		return (FRAME_TYPE_I == video_type)?3:0;
	}

	//pw
	//return (FRAME_TYPE_I == video_type)?3:0;
}

void bizData_NetStateNotify(void* p)
{
	if( !p ) return;
	
	SNetStateNotify* para = (SNetStateNotify*)p;	
	
    SBizEventPara sBizEventPara;
    
	//printf("bizData_NetStateNotify enter, op %d !\n", para->eAct);
	
	switch(para->eAct)
	{
		case EM_NET_STATE_DHCP:
		{
			if(para->sRslt.state==0)
			{
				sBizEventPara.emType	= EM_BIZ_EVENT_NET_STATE_DHCP;
			}
			else
				sBizEventPara.emType	= EM_BIZ_EVENT_NET_STATE_DHCP_STOP;
			
		} break;
		case EM_NET_STATE_PPPOE:
		{     
			if(para->sRslt.state==0)
			{
				sBizEventPara.emType	= EM_BIZ_EVENT_NET_STATE_PPPOE;
			}
			else
				sBizEventPara.emType	= EM_BIZ_EVENT_NET_STATE_PPPOE_STOP;
				
		} break;
		case EM_NET_STATE_MAIL:
		{     
			//if(para->sRslt.state==0)
			{
				sBizEventPara.emType	= EM_BIZ_EVENT_NET_STATE_MAIL;
			}
		} break;
		case EM_NET_STATE_DDNS:
		{  
			if(para->sRslt.state==0)
			{
				sBizEventPara.emType	= EM_BIZ_EVENT_NET_STATE_DDNS;
			}
			else
				sBizEventPara.emType	= EM_BIZ_EVENT_NET_STATE_DDNS_STOP;
				
		} break;
		case EM_NET_STATE_CONN:
		{  
			//if(para->sRslt.state==0)
			{
				sBizEventPara.emType	= EM_BIZ_EVENT_NET_STATE_CONN;
			}
		} break;
		case EM_NET_STATE_UPDATEMAINBOARDSTART:
		{
			//printf("%s 1\n", __func__);
			sBizEventPara.emType	= EM_BIZ_EVENT_NET_STATE_UPDATEMAINBOARDSTART;
		} break;
		case EM_NET_STATE_UPDATEPANNELSTART:
		{
			sBizEventPara.emType	= EM_BIZ_EVENT_NET_STATE_UPDATEPANNELSTART;
		} break;
		case EM_NET_STATE_UPDATESTARTLOGOSTART:
		{
			sBizEventPara.emType	= EM_BIZ_EVENT_NET_STATE_UPDATESTARTLOGOSTART;
		} break;
		case EM_NET_STATE_UPDATEAPPLOGOSTART:
		{
			sBizEventPara.emType	= EM_BIZ_EVENT_NET_STATE_UPDATEAPPLOGOSTART;
		} break;
		case EM_NET_STATE_FORMAT_START:
		{
			sBizEventPara.emType	= EM_BIZ_EVENT_NET_STATE_FORMATSTART;
		} break;
		case EM_NET_STATE_SG: //上传监管平台返回状态
		{
			sBizEventPara.emType	= EM_BIZ_EVENT_NET_STATE_SGUPLOAD;
		} break;
		case EM_NET_STATE_SGReqSnap: //监管平台请求上传
		{
			u8 chn;
			//printf("SG Platform Server request snap chn mask: 0x%x\n", para->SGReqChnMask);

			for (chn=0; chn<32; chn++)
			{
				if (para->SGReqChnMask & (1<<chn))
				{
					RequestSnap(chn, TypeSvrReq, NULL, NULL);
				}
			}
			
		} break;
	}
	
	if(para->eAct != EM_NET_STATE_FORMAT_START)
	{
		if(0 == para->sRslt.nErr )
		{
			sBizEventPara.emBizResult = EM_BIZ_NET_STATE_SUCCESS;
		}
		else
		{
			sBizEventPara.emBizResult = EM_BIZ_NET_STATE_FAIL;
		}

		if (para->eAct == EM_NET_STATE_SG)
		{
			u32 tmp = para->sRslt.state;//chn
			tmp = ((tmp&0xff) << 8) | para->sRslt.nErr;
			
			sBizEventPara.emBizResult = tmp;
		}
		/*
		if (para->eAct == EM_NET_STATE_SGReqSnap)
		{
			sBizEventPara.emBizResult =  para->SGReqChnMask;
		}
		*/
	}
	else
	{
		sBizEventPara.sBizFormat.nDiskIdx = para->sRslt.state;
		if(0 == para->sRslt.nErr )
		{
			sBizEventPara.sBizFormat.emBizResult = EM_BIZ_NET_STATE_SUCCESS;
		}
		else
		{
			sBizEventPara.sBizFormat.emBizResult = EM_BIZ_NET_STATE_FAIL;
		}
	}
	
    SendBizEvent(&sBizEventPara);
}

s32 NetInit(void* pHddHdr, SNetSysPara* pPara)
{
	int i;
	s32 rtn = 0;
	
	// clear online user info 
	for( i=0; i<BIZ_NET_ONLINE_MAX; i++ )
	{
		aOnlineUser[i].nId = -1;
	}
	
	// get some parameters
	SBizParaTarget sParaTgtIns;
	SBizNetPara sCfgIns;
	
	//printf("BizNetReadCfg %d\n", __LINE__);
	
	memset(&sNCCfg, 0, sizeof(sNCCfg));
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_NETPARA;
	rtn |= BizGetPara(&sParaTgtIns, &sCfgIns);
	if(0 == rtn)
	{
		sNCCfg.sParaIns.DhcpFlag	= sCfgIns.DhcpFlag;
		
		sNCCfg.sParaIns.DDNSFlag	= sCfgIns.DDNSFlag;
		if(sNCCfg.sParaIns.DDNSFlag)
		{
			strcpy(sNCCfg.sParaIns.DDNSDomain, sCfgIns.DDNSDomain);
			strcpy(sNCCfg.sParaIns.DDNSUser, sCfgIns.DDNSUser);
			strcpy(sNCCfg.sParaIns.DDNSPasswd, sCfgIns.DDNSPasswd);
		}
		//csp modify 20130322
		sNCCfg.sParaIns.UpdateIntvl = sCfgIns.UpdateIntvl;
		
		//csp modify 20130321
		sNCCfg.sParaIns.UPNPFlag = sCfgIns.UPNPFlag;
		
		//printf("dhcp flag %d\n", sCfgIns.DhcpFlag);
		sNCCfg.sParaIns.HostIP		= sCfgIns.HostIP;
		sNCCfg.sParaIns.GateWayIP	= sCfgIns.GateWayIP;
		sNCCfg.sParaIns.Submask		= sCfgIns.Submask;
		memcpy( sNCCfg.sParaIns.MAC, sCfgIns.MAC, sizeof(sCfgIns.MAC) );
		sNCCfg.sParaIns.DNSIP		= sCfgIns.DNSIP;
		sNCCfg.sParaIns.DNSIPAlt	= sCfgIns.DNSIPAlt;
		sNCCfg.sParaIns.PPPOEFlag	= sCfgIns.PPPOEFlag;
		memcpy( sNCCfg.sParaIns.PPPOEUser, sCfgIns.PPPOEUser, sizeof(sNCCfg.sParaIns.PPPOEUser) );
		memcpy( sNCCfg.sParaIns.PPPOEPasswd, sCfgIns.PPPOEPasswd, sizeof(sNCCfg.sParaIns.PPPOEPasswd ) );
		
		sNCCfg.sParaIns.TCPPort 	= sCfgIns.TCPPort<=0?8630:sCfgIns.TCPPort;	
		sNCCfg.sParaIns.TCPMaxConn 	= 
			(sCfgIns.TCPMaxConn>pPara->nChNum||sCfgIns.TCPMaxConn<=0)?pPara->nChNum:sCfgIns.TCPMaxConn;
	    sNCCfg.sParaIns.HttpPort 	= sCfgIns.HttpPort<=0?80:sCfgIns.HttpPort;
	    sNCCfg.sParaIns.MobilePort	= sCfgIns.MobilePort<=0?101:sCfgIns.MobilePort;
	    
		sNCCfg.nAllLinkMax 			= STREAM_LINK_MAXNUM;			// 码流数上限
		sNCCfg.nSubStreamMax 		= 
			sCfgIns.nSubStreamMax<=0?pPara->nChNum:sCfgIns.nSubStreamMax; // 4;			// 子码流数上限
		sNCCfg.nSendBufNumPerChn 	= 3;//sCfgIns.nSendBufNumPerChn<=0?6:sCfgIns.nSendBufNumPerChn; // 3		// 每通道发送buff数
		sNCCfg.nFrameSizeMax		= pPara->nFrameSizeMax<=0?MAX_FRAME_SIZE:pPara->nFrameSizeMax; // MAX_FRAME_SIZE;			// 帧buff大小上限
		
		sNCCfg.yFactor 				= pPara->nVs;				// 制式
		sNCCfg.nVoipMode			= MEDIA_TYPE_PCMU;//sCfgIns.nVoipMediaType ; //MEDIA_TYPE_PCMU;
		sNCCfg.nVideoMediaType 		= MEDIA_TYPE_H264;//sCfgIns.nVideoMediaType; // MEDIA_TYPE_H264;
		sNCCfg.nAudioMediaType 		= MEDIA_TYPE_PCMU;//sCfgIns.nAudioMediaType; 
		sNCCfg.nVideoFrameRate 		= 25;
		sNCCfg.nAudioFrameRate 		= 33;
		sNCCfg.nAudioMode 			= AUDIO_SAMPLE_BITWIDTH;//sCfgIns.nAudioSampleMode;
		
		biz_NetGetRandomMAC(sNCCfg.sParaIns.MAC);
		if(strcmp(sNCCfg.sParaIns.MAC, sCfgIns.MAC) != 0)
		{
			memcpy( sCfgIns.MAC, sNCCfg.sParaIns.MAC, sizeof(sCfgIns.MAC) );
			
			BizSetPara(&sParaTgtIns, &sCfgIns);
		}
	}
	else
	{
		memset(sNCCfg.sParaIns.MAC, 0, sizeof(sNCCfg.sParaIns.MAC));
		biz_NetGetRandomMAC(sNCCfg.sParaIns.MAC);
		
		//sNCCfg.sParaIns.HostIP		= sCfgIns.HostIP;
		//sNCCfg.sParaIns.GateWayIP	= sCfgIns.GateWayIP;
		//sNCCfg.sParaIns.Submask		= sCfgIns.Submask;
		
		sNCCfg.sParaIns.DDNSFlag	= 0;
		sNCCfg.sParaIns.TCPPort		= 8630;
		sNCCfg.sParaIns.TCPMaxConn 	= 4;
	    sNCCfg.sParaIns.HttpPort 	= 80;
	    sNCCfg.sParaIns.MobilePort	= 101;
	    
		sNCCfg.nAllLinkMax 			= STREAM_LINK_MAXNUM;			// 码流数上限
		sNCCfg.nSubStreamMax 		= 4;							// 子码流数上限
		sNCCfg.nSendBufNumPerChn 	= 3;							// 每通道发送buff数
		sNCCfg.nFrameSizeMax		= MAX_FRAME_SIZE;				// 帧buff大小上限

		sNCCfg.yFactor 				= PAL;							// 制式

		sNCCfg.nVoipMode			= MEDIA_TYPE_PCMU;
		sNCCfg.nVideoMediaType 		= MEDIA_TYPE_H264;
		sNCCfg.nAudioMediaType 		= AUDIO_SAMPLE_BITWIDTH;
		sNCCfg.nVideoFrameRate 		= 25;
		sNCCfg.nAudioFrameRate 		= 33;
		sNCCfg.nAudioMode 			= MEDIA_TYPE_PCMU;
	}
	
	//printf("TCPport %d HttpPort %d \n", sNCCfg.sParaIns.TCPPort, sNCCfg.sParaIns.HttpPort);
	
	sNCCfg.pDiskMgr 			= pHddHdr;
	sNCCfg.pReqKeyCB 			= NULL;
	sNCCfg.pReqStreamCB 		= bizNet_RequestPreviewCB;
	sNCCfg.pRpCB 				= bizNet_remoteplayCB;
	sNCCfg.pVOIPIn 				= NULL;
	sNCCfg.pVOIPOut 			= NULL;
	sNCCfg.nChnMax 				= pPara->nChNum;
	sNCCfg.pWriteLogCB			= BizNetLogWriteCB;
	sNCCfg.pRFomat				= bizNet_RemoteOpCB;
	sNCCfg.pRUpdate				= bizNet_RemoteOpCB;
	sNCCfg.pNotify				= bizData_NetStateNotify;
	
	//csp modify
	SBizDvrInfo sDvrInfo;
	memset(&sDvrInfo, 0, sizeof(sDvrInfo));
	ConfigGetDvrInfo(0,&sDvrInfo);
	if(0 == strcmp("R3104HD",sDvrInfo.sproductnumber))
	{
		sNCCfg.nVideoMaxWidth = 1920;
		sNCCfg.nVideoMaxHeight = 1080;
	}
	else if(0 == strcmp("NR2116",sDvrInfo.sproductnumber))//NVR used
	{
		sNCCfg.nVideoMaxWidth = 768;//1280;
		sNCCfg.nVideoMaxHeight = 576;//720;
	}
	else if(0 == strcmp("NR3116",sDvrInfo.sproductnumber))//NVR used
	{
		sNCCfg.nVideoMaxWidth = 768;//1280;
		sNCCfg.nVideoMaxHeight = 576;//720;
	}
	else if(0 == strcmp("NR3132",sDvrInfo.sproductnumber))//NVR used
	{
		sNCCfg.nVideoMaxWidth = 768;//1280;
		sNCCfg.nVideoMaxHeight = 576;//720;
	}
	else if(0 == strcmp("NR3124",sDvrInfo.sproductnumber))//NVR used
	{
		sNCCfg.nVideoMaxWidth = 768;//1280;
		sNCCfg.nVideoMaxHeight = 576;//720;
	}
	else if(0 == strcmp("NR1004",sDvrInfo.sproductnumber))//NVR used
	{
		sNCCfg.nVideoMaxWidth = 768;//1280;
		sNCCfg.nVideoMaxHeight = 576;//720;
	}
	else if(0 == strcmp("NR1008",sDvrInfo.sproductnumber))//NVR used
	{
		sNCCfg.nVideoMaxWidth = 768;//1280;
		sNCCfg.nVideoMaxHeight = 576;//720;
	}
#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
	//csp modify 20130620
	else if(0 == strcmp("R3110HDW",sDvrInfo.sproductnumber))
	{
		sNCCfg.nVideoMaxWidth = 1920;
		sNCCfg.nVideoMaxHeight = 1080;
	}
	else if(0 == strcmp("R3106HDW",sDvrInfo.sproductnumber))
	{
		sNCCfg.nVideoMaxWidth = 1920;
		sNCCfg.nVideoMaxHeight = 1080;
	}
	else
	{
		sNCCfg.nVideoMaxWidth = 960;
		sNCCfg.nVideoMaxHeight = 576;
	}
#else
	else if(0 == strcmp("R9516S",sDvrInfo.sproductnumber))
	{
		sNCCfg.nVideoMaxWidth = 352;
		sNCCfg.nVideoMaxHeight = 288;
	}
	else
	{
		sNCCfg.nVideoMaxWidth = 704;
		sNCCfg.nVideoMaxHeight = 576;
	}
#endif
	
	// register preview cmd callback
	bizNet_ExcmdRegCB();
	
	//printf("nChnMax %d\n", sNCCfg.nChnMax);
	//yaogang modify 20150324 
	//跃天: 1 nvr，2 轮巡解码器，3 切换解码器
	sNCCfg.nNVROrDecoder = pPara->nNVROrDecoder;
	#if 1
	rtn |= NetCommInit(&sNCCfg);
	if(0 != rtn)
	{
		printf("BizNetInit failed!\n");
	}
	
	SNetMultiCastInfo sNetMultiCastInfo;
	
	sNetMultiCastInfo.deviceIP 			= sCfgIns.HostIP; 					//设备IP  
	sNetMultiCastInfo.devicePort 		= sCfgIns.TCPPort;					//设备端口 
	strcpy( sNetMultiCastInfo.device_name, pPara->device_name );			//设备名称
	strcpy( sNetMultiCastInfo.device_mode, pPara->device_mode );			//设备型号
	sNetMultiCastInfo.maxChnNum			= pPara->maxChnNum;					//最大通道数
	sNetMultiCastInfo.maxAduioNum		= pPara->maxAduioNum;				//最大音频数
	sNetMultiCastInfo.maxSubstreamNum	= sCfgIns.nSubStreamMax;			//最大子码流数
	sNetMultiCastInfo.maxPlaybackNum	= pPara->maxPlaybackNum;			//最大回放数
	sNetMultiCastInfo.maxAlarmInNum		= pPara->maxAlarmInNum;				//最大报警输入数
	sNetMultiCastInfo.maxAlarmOutNum	= pPara->maxAlarmOutNum;			//最大报警输出数
	sNetMultiCastInfo.maxHddNum			= pPara->maxHddNum;					//最大硬盘数
	sNetMultiCastInfo.nNVROrDecoder = pPara->nNVROrDecoder;	//区别NVR和解码器--- 跃天
	
	NetComm_InitMultiCastInfo(&sNetMultiCastInfo);
	#endif
	
	//csp modify 20121130
	do_web_link(pPara->nLangId);
	
	return rtn?1:0;
}

void bizNet_remoteplayCB(
	void* pFileHandle,
	PSRmtPlayFileOp pParam
)
{
	//printf("Enter bizNet_remoteplayCB ... \n");
 	void* pTmpFileHandle = NULL;
	
	if(!pParam)
	{
		printf("NULL param!!!\n");

		return;
	}
	EM_RMTPLAY_OP eOp = pParam->eOp;
	EM_FILEOP_SRC eOpSrc = pParam->eOpSrc;
	
	switch(eOp)
	{
		case EM_RMTPLAY_OPEN:
		{
			if(eOpSrc==EM_FILEOP_REPLAY)
			{
				pTmpFileHandle = (void*)custommp4_open(
					pParam->OpenSeek.szFilePath, 
					O_R, 
					pParam->OpenSeek.nOffset
				);
			}
			else if(eOpSrc==EM_FILEOP_DOWN)
			{				
				pTmpFileHandle = fopen(
					pParam->OpenSeek.szFilePath,"rb");
			}
			
			pParam->OpenSeek.pFile = (void*)pTmpFileHandle;
		}
		break;
		case EM_RMTPLAY_SEEK:
		{
			if(pFileHandle)
			{
				if(eOpSrc==EM_FILEOP_REPLAY)
				{
					pParam->nOpRslt = 
						custommp4_seek_to_time_stamp(pFileHandle, pParam->OpenSeek.nOffset);
				}
				else if(eOpSrc==EM_FILEOP_DOWN)
				{				
					pParam->nOpRslt = fseek(pFileHandle,pParam->OpenSeek.nOffset,SEEK_SET);
				}
			}
		}
		break;
		case EM_RMTPLAY_CLOSE:
		{
			if(pFileHandle)
			{
				if(eOpSrc==EM_FILEOP_REPLAY)
				{
					custommp4_close(pFileHandle);
				}
				else if(eOpSrc==EM_FILEOP_DOWN)
				{				
					fclose(pFileHandle);
				}
			}
		}
		break;
		case EM_RMTPLAY_GETFRAME:
		{
			if(pFileHandle)
			{
				if(eOpSrc==EM_FILEOP_REPLAY)
				{
					if(pFileHandle)
						pParam->GetFrame.nFrameSize = custommp4_read_one_media_frame(
									pFileHandle,
									pParam->GetFrame.pFrameBuf,
									pParam->GetFrame.nBufSize,
									&pParam->GetFrame.nStartTime,
									&pParam->GetFrame.bKey,
									&pParam->GetFrame.nPts,
									&pParam->GetFrame.nMediaType
								);

						// debug by lanston
						#if 0
						if(pParam->GetFrame.nFrameSize>0)
						{
							debug_write(pParam->GetFrame.pFrameBuf, pParam->GetFrame.nFrameSize);
						}
						#endif
						#if 0
						printf("frame size %d\n", pParam->GetFrame.nFrameSize);
						printf("nStartTime %d\n", pParam->GetFrame.nStartTime);
						printf("frame bKey %d\n", pParam->GetFrame.bKey);
						printf("frame nPts %lld\n", pParam->GetFrame.nPts);
						printf("nMediaType %d\n", pParam->GetFrame.nMediaType);
						#endif
						//pParam->GetFrame.nPts = pParam->GetFrame.nPts/1000;
						
						//usleep(40*1000);
								
					pParam->GetFrame.bKey = is_i_frame(pParam->GetFrame.bKey);
				}
				else if(eOpSrc==EM_FILEOP_DOWN)
				{
					pParam->GetFrame.nFrameSize
						= fread(pParam->GetFrame.pFrameBuf,
							1,pParam->GetFrame.nBufSize,pFileHandle);
				}
			}
		}
		break;
		case EM_RMTPLAY_GetTimeLength:
		{
			if(pFileHandle)
				pParam->GetTimeLen.nTotalTime = custommp4_total_time(pFileHandle);
		}
		break;
		case EM_RMTPLAY_GetMediaFormat:
		{
			if(pFileHandle)
			{
				pParam->GetFormat.nWidth = custommp4_video_width(pFileHandle);
				pParam->GetFormat.nHeight = custommp4_video_height(pFileHandle);
			}
		}
		break;
	}
	return;
}

void BizNetStartDHCP()
{
	NetComm_StartDHCP();
}

int BizSetHWAddr(char *pBuf)
{
	return SetHWAddr(pBuf);
}

void BizNetStopDHCP()
{
	NetComm_StopDHCP();
}

s32 BizNetStartPPPoE(char *pszUser, char* pszPasswd)
{
	return NetComm_PPPoE_Start(pszUser, pszPasswd);
}

s32 BizNetStopPPPoE()
{
	return NetComm_PPPoE_Stop(0);
}

//csp modify 20130321
s32 BizNetStartUPnP(u8 upnp_enable)
{
	return NetComm_RestartUPnP(upnp_enable);
}
s32 BizNetStopUPnP()
{
	return NetComm_RestartUPnP(0);
}

s32 BizNetReadCfgEx( SBizNetPara* pCfg )
{
	if(NULL == pCfg) return -1;
	
	SNetPara sPIns;
	
	NetComm_ReadParaEx(&sPIns);
	
	// later change if structure changed
	// 
	memcpy( pCfg, &sPIns, sizeof(SBizNetPara) );
	
	return 0;
}

s32 BizNetReadCfg( SBizNetPara* pCfg )
{
	SNetPara sPIns;
	
	NetComm_ReadPara((SNetPara *)&sPIns);

	// later change if structure changed
	// 
	memcpy( pCfg, &sPIns, sizeof(SBizNetPara) );
	
	return 0;
}

s32 BizNetWriteCfg( SBizNetPara* pCfg )
{
	SBizNetPara sPIns;
	
	if( ConfigGetNetPara(0, &sPIns, 0) == 0 )
	{	
		sPIns.HostIP = pCfg->HostIP;
		sPIns.Submask = pCfg->Submask;
		sPIns.GateWayIP = pCfg->GateWayIP;
		sPIns.DNSIP = pCfg->DNSIP;
		sPIns.DNSIPAlt = pCfg->DNSIPAlt;
		sPIns.DhcpFlag = pCfg->DhcpFlag;
		memcpy(sPIns.MAC, pCfg->MAC, sizeof(sPIns.MAC));

		// http端口改变则重启web服务
		if(sPIns.HttpPort != pCfg->HttpPort)
		{
			if(0 == NetComm_WebRestart(pCfg->HttpPort) )
			{
				sPIns.HttpPort = pCfg->HttpPort;
			}
		}
	}
	
	return NetComm_WritePara((SNetPara*)&sPIns);
}

s32 BizNetAddLoginUser( SBNOnlineUser* pUser )
{
	int i;
	
	if(pUser)
	{
		for( i=0; i<BIZ_NET_ONLINE_MAX; i++ )
		{
			if(aOnlineUser[i].nId == -1)
			{
				memcpy(&aOnlineUser[i], pUser, sizeof(SBNOnlineUser));
				break;
			}
		}

		return 0;

	}
	else
		return -1;
}

s32 BizNetDelLoginUser( u32 nId )
{
	int i;
	for( i=0; i<BIZ_NET_ONLINE_MAX; i++ )
	{
		if(aOnlineUser[i].nId == nId)
		{
			aOnlineUser[i].nId = -1;
			break;
		}
	}

	return 0;
}

s32 BizNetGetLoginUserID( SBNOnlineUser* pUser )
{
	int i;
	if(pUser)
	{
		for( i=0; i<BIZ_NET_ONLINE_MAX; i++ )
		{
			if( strcmp(aOnlineUser[i].szName, pUser->szName) == 0 &&
				aOnlineUser[i].nIP  == pUser->nIP                 &&
				aOnlineUser[i].nMac == pUser->nMac 
			)
			{
				pUser->nId = aOnlineUser[i].nId;
				break;
			}
		}

		return 0;

	}
	else
		return -1;
	
}

s32 BizNetMAC2Str( u64 nMAC, char* pMAC )
{
	int i=0,n;

	while(nMAC > 0)
	{
		n = nMAC % 16;

	    if( n < 10 ) pMAC[i] = n + '0';
	  	else 		 pMAC[i] = n - 10 + 'A';
		
	  	nMAC /= 16;
	  	i++;
	  	
	  	if( 2 == (i%3) )
	  	{
			pMAC[i++] = ':';
	  	}
	}

	pMAC[i]='\0';

	n = i - 1;
	i = 0;
	while(i < n)
	{
		char t = pMAC[i];

	  	pMAC[i] = pMAC[n];
	  	pMAC[n] = t;

	  	i++;
	  	n--;
	}

	return 0;
}

// bug exist!!!
// todo fix
s32 BizNetStr2MAC( char *pMac, u64* pnMAC )
{
	u8  nByte;
	int i;

	char szTmp[3] = {0};

	*pnMAC = 0;
	for(i=0; i<6; i++)
	{
		szTmp[0] = pMac[3*i];
		szTmp[1] = pMac[3*i+1];

		nByte = strtol(szTmp, 0, 16);

		*pnMAC |= nByte<<((5-i)*8);
	}

	return 0;
}


EM_BIZNET_STATE BizNetGetState() 
{
	return (EM_BIZNET_STATE)NetComm_GetState();
}

s32 BizNetSendMail(SBizMailContent* pMail)
{
	SMailContent sMailIns;
	memset(&sMailIns, 0, sizeof(sMailIns));
	
	sMailIns.pSrcUsr = pMail->pSrcUsr;
	sMailIns.pSrcBox = pMail->pSrcBox;
	sMailIns.pToUsr = pMail->pToUsr;
	sMailIns.pToBox = pMail->pToBox;
	sMailIns.pUserName = pMail->pUserName;
	sMailIns.pPasswd = pMail->pPasswd;
	
	sMailIns.pSmtp = pMail->pSmtp;
	sMailIns.pSubject = pMail->pSubject;
	sMailIns.pBody = pMail->pBody;
	sMailIns.bAttach = pMail->bAttach;
	
	return NetComm_SendMail((SMailContent*)pMail);
}

s32 BizNetDdnsCtrl( EM_BIZ_DDNS_ACT eAct, sBizNetDdnsPara* pPara )
{
	s32 			err = 0;
	SNetDdnsPara 	sStartIns;
	
	if(eAct == EM_BIZDDNS_START || eAct == EM_BIZDDNS_REGIST)
	{
		if(!pPara) return -1;
		
		NetComm_DDNS_Install(pPara->eProt);
		
		/*
		switch(pPara->eProt)
		{
			case EM_BIZDDNS_PROT_TL:
			{
				NetComm_DDNS_Install(EM_NET_DDNS_TL);
			} break;
			case EM_BIZDDNS_PROT_3322:
			{
				NetComm_DDNS_Install(EM_NET_DDNS_3322);
			} break;
			case EM_BIZDDNS_PROT_DYDNS:
			{
				NetComm_DDNS_Install(EM_NET_DDNS_DYDNS);
			} break;
			case EM_BIZDDNS_PROT_DVRNET://cw_ddns
			{
				NetComm_DDNS_Install(EM_NET_DDNS_DVRNET);
			} break;
			case EM_BIZDDNS_PROT_NOIP:
			{
				NetComm_DDNS_Install(EM_NET_DDNS_NOIP);
			} break;
			default:
				return -1;
		}
		*/
	}
	
	switch(eAct)
	{
		case EM_BIZDDNS_START:
		{
			memset(&sStartIns, 0, sizeof(sStartIns));
			
			if(pPara)
			{
				strcpy(sStartIns.szDomain, pPara->szDomain);
				strcpy(sStartIns.szUser, pPara->szUser);
				strcpy(sStartIns.szPasswd, pPara->szPasswd);
				err = NetComm_DDNS_Start(&sStartIns);
			}
			else
				err = NetComm_DDNS_Start(NULL);
		} break;
		case EM_BIZDDNS_REGIST:
		{
			memset(&sStartIns, 0, sizeof(sStartIns));
			
			if(pPara)
			{
				strcpy(sStartIns.szDomain, pPara->szDomain);
				strcpy(sStartIns.szUser, pPara->szUser);
				strcpy(sStartIns.szPasswd, pPara->szPasswd);
				err = NetComm_DDNS_Register(&sStartIns);
			}
			else
				err = NetComm_DDNS_Register(NULL);
		
		} break;
		case EM_BIZDDNS_STOP:
		{
			err = NetComm_DDNS_Stop();
		} break;
		default:
			return -1;
	}
	
	return err;
}

void BizNetLogWriteCB(SNetLog* pLog)
{
	if(!pLog) return;
	
	printf("Write Net Log, id: %d\n", pLog->nLogId);
	
	SBizLogInfo info; 
	memset(&info, 0, sizeof(info));
	
	switch(pLog->nLogId)
	{
		case EM_NETLOG_UPDATE:
		{
			info.nMasterType = BIZ_LOG_MASTER_SYSCONTROL;
			info.nSlaveType = BIZ_LOG_SLAVE_UPDATE;
		} break;
		case EM_NETLOG_MUTE:
		{
			info.nMasterType = BIZ_LOG_MASTER_SYSCONTROL;
			info.nSlaveType = BIZ_LOG_SLAVE_MUTE;
		} break;
		case EM_NETLOG_START_BACKUP:
		{
			info.nMasterType = BIZ_LOG_MASTER_BACKUP;
			info.nSlaveType = BIZ_LOG_SLAVE_START_BACKUP;
		} break;
		case EM_NETLOG_PB_STOP:
		{
			info.nMasterType = BIZ_LOG_MASTER_PLAYBACK;
			info.nSlaveType = BIZ_LOG_SLAVE_PB_STOP;
		} break;
		case EM_NETLOG_PB_PAUSE:
		{
			info.nMasterType = BIZ_LOG_MASTER_PLAYBACK;
			info.nSlaveType = BIZ_LOG_SLAVE_PB_PAUSE;
		} break;
		case EM_NETLOG_PB_RESUME:
		{
			info.nMasterType = BIZ_LOG_MASTER_PLAYBACK;
			info.nSlaveType = BIZ_LOG_SLAVE_PB_RESUME;
		} break;
		case EM_NETLOG_PB_FORWARD:
		{
			info.nMasterType = BIZ_LOG_MASTER_PLAYBACK;
			info.nSlaveType = BIZ_LOG_SLAVE_PB_FORWARD;
		} break;
		case EM_NETLOG_PB_BACKWARD:
		{
			info.nMasterType = BIZ_LOG_MASTER_PLAYBACK;
			info.nSlaveType = BIZ_LOG_SLAVE_PB_BACKWARD;
		} break;
		//csp modify
		default:
			break;
	}
	
	struct in_addr addr;
	memcpy(&addr, &pLog->ip, 4);
	
	printf("net log %d %s\n", __LINE__, inet_ntoa(addr));
	
	sprintf(info.aIp, "%s", inet_ntoa(addr));	
	strcpy(info.aIp, inet_ntoa(addr));
	
	SNetUser sUser;
	memset(&sUser, 0, sizeof(sUser));
	
	sUser.ip = pLog->ip;
	sUser.sock = pLog->sockfd;
	
	if( 0 != NetComm_NetUserCtrl(0, &sUser) )
		memset(info.aUsername, 0, sizeof(info.aUsername));
	else
		strcpy(info.aUsername, sUser.name);
	
	printf("log write user %s\n", sUser.name);
	
	BizWriteLog(&info);
	
	return;
}

void biz_NetGetRandomMAC( char* pMAC )
{
	int 	i,j,m;
	int 	k = 2;
	char 	c[2];
	char 	*strptr = NULL;
	char 	tempstr[4] = {0};
	char 	*endptr;
	u8 mac;

	if(!pMAC) return;

	//printf("original mac %s \n", pMAC);
	
	srand((unsigned int)time(NULL));

	for(i=0; i<6; i++)
	{
		memset(tempstr, 0, sizeof(tempstr));
		strncpy(tempstr, &pMAC[3*i], 2);
		mac = strtoul(tempstr, &endptr, 16);
		if(mac != 0) break;
	}
	
	if(i != 6)
		return;
	
	pMAC[0] = '0';
	pMAC[1] = '0';

	for(i=0; i<5; i++)
	{
		pMAC[k]=':';
		j = rand()%256;

		c[0] = j/16;
		c[1] = j%16;
		for(m=0; m<2; m++)
		{
			pMAC[k+m+1] = (c[m]>9)?('A'+c[m]-10):('0'+c[m]);
			//printf("%d %c \n", c[1], pMAC[k+m+1]);
		}
		k += 3;
	}
	
	pMAC[17]='\0';
	
	printf("current mac %s\n", pMAC);
}

s32 BizNetReqConnStatus(SBizNetConnStatus* psNCS)
{
	return NetComm_ReqConnStatus((SNetConnStatus*)psNCS);
}

u8 BizNetGetVoipState( void )
{
	return NetComm_GetVoipState();
}

void BizNetUpdateProgressNoitfyCB(u8 nProgress)
{
	NetComm_UpdateProgNotify(nProgress);
}

s32 bizNet_RemoteOpCB(SRemoteOpCB* para)
{
	s32 rtn = 0;
	
	if(!para) return -1;
	
	printf("%d operation %d\n", __LINE__, para->eOp);
	switch(para->eOp)
	{
		case EM_NETFORMAT:
		{
			int i;
			u8 nDiskIdx = para->Format.nIdx;
		    SStoreDevManage sStoreDevManage;
			sStoreDevManage.psDevList = malloc(sizeof(SDevInfo)*64);

			if(sStoreDevManage.psDevList)
			{
				rtn = ModSysComplexDMGetInfo(&sStoreDevManage, 64);
				/*
				if(0 == rtn && nDiskIdx < 64 && sStoreDevManage.psDevList[nDiskIdx].nType==0)
				{
					ModLogSetFormatFlag(1);//csp modify 20140812

					rtn = ModSysComplexDMFormat(
					sStoreDevManage.psDevList[nDiskIdx].strDevPath, 
					EM_FORMAT_FAT32_IFV
					);

					ModLogSetFormatFlag(0);//csp modify 20140812
				}
				*/
				//yaogang modify 20141128	
				if(0 == rtn && nDiskIdx < 64 )//&& sStoreDevManage.psDevList[nDiskIdx].nType==0)
				{
					for(i=0; i<sStoreDevManage.nDiskNum; i++)
					{
						if(nDiskIdx == sStoreDevManage.psDevList[i].disk_logic_idx-1)
						{
							ModLogSetFormatFlag(1);//csp modify 20140812
							printf("remote format disk %d, path:%s\n", nDiskIdx, sStoreDevManage.psDevList[i].strDevPath);
							rtn = ModSysComplexDMFormat(
							sStoreDevManage.psDevList[i].strDevPath, 
							EM_FORMAT_FAT32_IFV
							);
							
							ModLogSetFormatFlag(0);//csp modify 20140812
							break;
						}
					}
				}
				free(sStoreDevManage.psDevList);
			}
		} break;
		case EM_NETUPDATE:
		{
			rtn = ModSysComplexUpgrade(para->Update.eTgt, para->Update.eWay, para->Update.sfilepath);
		} break;
		default:
			rtn = -1;
		break;
	}

	return rtn;
}

void BizSysSetRefreshDiskStatue(u8 flag)
{
	SetRefreshDiskStatue(flag);
}

u8 BizSysGetCheckDiskStatue()
{
	return GetCheckDiskStatue();
}

void BizNetUploadAlarm(u32 key)
{
	upload_alarm(key);
}

void BizSetUploadAlarmPara(u8 type, u8 id, u8 state)
{
	u32 alarmkey = (type << 16) | (id << 8) | state;
	BizNetUploadAlarm(alarmkey);
}

