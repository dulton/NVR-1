#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

//#include "iflytype.h"

#include "common_basetypes.h"

#include "owsp_tl_def.h"
#include "MobileServerSdk.h"
#include "mobile.h"
//#include "mediastream.h"
#include "Netcomm.h"
#include "Netcommon.h"
//#include "mediacommon.h"
//#include "config.h"

//#include "3511gui.h"

typedef struct 
{
    int max_videos;
    int max_audios;
    u32 *p_subGop;
	u8 *p_status;

	int nVEncType;
	int yFactor;
	PStreamRequestCB pReqCB;
} dvr_t;

static dvr_t g_Dvr;

static int  deal_command(mobile_server_cmd_t* server_cmd, void *p_result);
static void  deal_Login (login_param_t login, cmd_result_t *p_result);
static void deal_get_stream_param(u32 channel, cmd_result_t *p_result);
static void deal_control(control_param_t control, cmd_result_t *p_result);
static void deal_channel_switch(channel_switch_t channel_switch, cmd_result_t *p_result);
//static int ptz_control(u8 chn, TCamInfo* pCamInfo);

static u32 getSubVideoGop(int chn);
static int setSubVideoGop(int chn, u32 gop);
static int setSubVideoForce(int chn);

extern PNetCommCommandDeal cmdProc_GetCmdCB(u32 nCmdID);

int mobile_init(int listenPort, int videoChns, int audioChns, PStreamRequestCB pCB, int nVEncType, int yFactor)
{
    int ret;
    int port = 26868;

    port = listenPort;
    if (0 == listenPort)
    {
        port = 26868;
    }

    memset(&g_Dvr, 0, sizeof(g_Dvr));

    g_Dvr.max_videos = videoChns;
    g_Dvr.max_audios = audioChns;

    g_Dvr.p_subGop = (u32 *) malloc(sizeof(u32) * videoChns);
    if (NULL == g_Dvr.p_subGop)
    {
        return -1;
    }
 

	g_Dvr.p_status = (u8 *) malloc(sizeof(u8) * videoChns);
    if (NULL == g_Dvr.p_status)
    {
    	free(g_Dvr.p_subGop);
        return -1;
    }
	
	memset(g_Dvr.p_subGop, 0, sizeof(u32) * videoChns);
	memset(g_Dvr.p_status, 0, sizeof(u8) * videoChns);
	
	printf("port %d vch %d achn %d \n", port, videoChns, audioChns);
	
    ret = initMobileServerSdk(port, videoChns, audioChns, (pfun_mobile_server_cmd_cb)deal_command);
    if (RET_FAILED == ret)
    {
        printf("initMobileServerSdk failed\n");
        return -1;
    }
	
    g_Dvr.pReqCB = pCB;
    g_Dvr.nVEncType = nVEncType;
    g_Dvr.yFactor = yFactor;
	
    return 0;
}

void mobile_send_media_frame(PFRAMEHDR pFrmHdr, u32 chn)
{
    if (pFrmHdr)
    {
		//printf("1 chn%d media data is %s\n", chn, (pFrmHdr->m_byMediaType == 98)?"video":"audio");
		//printf("2 media data len:%d\n",pFrmHdr->m_dwDataSize);
        if (pFrmHdr->m_byMediaType == g_Dvr.nVEncType)
        {
            if (chn < g_Dvr.max_videos)
            {
                videoDataParam_t videoData;
				
                videoData.encodeType = CODEC_H264;
                videoData.dataSize = pFrmHdr->m_dwDataSize;
                videoData.isKeyFrame = ((pFrmHdr->m_tVideoParam.m_bKeyFrame) ? 1 : 0);//pFrmHdr->m_tVideoParam.m_bKeyFrame;//((pFrmHdr->videoParam.keyFrame > 0) ? 1 : 0);
                videoData.p_Data = (char *)pFrmHdr->m_pData;
                videoData.frameID = pFrmHdr->m_dwFrameID;
                videoData.timeStamp = pFrmHdr->m_dwTimeStamp;
                sendVideoToMobileServer(chn, &videoData);
                //printf("3 media data width:%d,height:%d\n",pFrmHdr->m_tVideoParam.m_wVideoWidth,pFrmHdr->m_tVideoParam.m_wVideoHeight);
                //if(pFrmHdr->m_tVideoParam.m_bKeyFrame) printf("4 chn%d media data frame type:%s m_bKeyFrame:%d timeStamp:%lu\n",chn,(pFrmHdr->m_tVideoParam.m_bKeyFrame)?"I帧":"P帧",videoData.isKeyFrame,videoData.timeStamp);
            }
        }
        else
        {
        	//printf("audio data\n");
        }
        //printf("media timestamp:%u\n",pFrmHdr->timeStamp);
        //printf("media frameID:%u\n",pFrmHdr->frameID);
        //printf("media frameRate:%u\n",pFrmHdr->frameRate);
    }
}

static int  deal_command(mobile_server_cmd_t* server_cmd, void *p_rslt)
{
    if (NULL == p_rslt)
    {
        printf("deal command returnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn\n");
        return 0;
    }

	//printf("deal_command mobile %d\n", server_cmd->cmd);

	cmd_result_t* p_result = p_rslt;
	
    switch (server_cmd->cmd)
    {
        case MOBILE_SERVER_COMMAND_LOGIN:
            printf("userName:%s,pasword:%s\n",server_cmd->login.userName,server_cmd->login.password);
            deal_Login(server_cmd->login, p_result);
            //printf("deal_command %p result %d \n", deal_command, p_result->isSuccess);
            break;
        case MOBILE_SERVER_COMMAND_CONTROL:
            deal_control(server_cmd->control, p_result);
            break;
        case MOBILE_SERVER_COMMAND_SWITCH_CHANNEL:
            //printf("MOBILE_SERVER_COMMAND_SWITCH_CHANNEL deal_channel_switch\n");
            deal_channel_switch(server_cmd->channel_switch, p_result);
            break;
        case MOBILE_SERVER_COMMAND_GETPARAM_STREAM:
            deal_get_stream_param(server_cmd->channel, p_result);
            break;
        case MOBILE_SERVER_COMMAND_NULL:
            p_result->isSuccess = 1;
            p_result->failReason = 0;
        default:           
            p_result->isSuccess = 0;
            p_result->failReason = RET_REASON_ERR_RUN;
            break;
    }

    /*
    printf("deal_command %p result %08x %d %d %d %d %p\n", 
    	deal_command, 
    	*(u32*)&p_result->isSuccess,
		p_result->failReason,
		p_result->reserve[0],
		p_result->reserve[1],
		p_result->reserve[2],
		p_result
    );
    */
            
    return 0;
}

static void deal_Login (login_param_t login, cmd_result_t *p_result)
{
    if (p_result)
    {
    	PNetCommCommandDeal pCB;
		SRemoteCmdReq 		sReqIns;
		SRemoteCmdRslt 		sRslt;

		memset(&sReqIns, 0, sizeof(sReqIns));
		memset(&sRslt, 0, sizeof(sRslt));
		
		ifly_loginpara_t* 	pLoginInfo;	
		pLoginInfo = &sReqIns.sReq.loginpara;
		strcpy(pLoginInfo->username, login.userName);
		strcpy(pLoginInfo->loginpass, login.password);
		strcpy(pLoginInfo->macAddr, "00:00:00:00:00:01"); // 填充mac防止登陆检查报错
		
		pCB = cmdProc_GetCmdCB(CTRL_CMD_LOGIN);
		if(pCB)
		{
			printf("line %d err %d \n", __LINE__, p_result->isSuccess);
			pCB(&sReqIns, &sRslt);
		}
		else
		{
			printf("line %d err %d \n", __LINE__, p_result->isSuccess);
            p_result->isSuccess = 0;
            p_result->failReason = RET_REASON_ERR_PROTOCOL;
			
			return;
        }
        
        if (sRslt.nErrCode != 0)
        {
            p_result->isSuccess = 0;
            p_result->failReason = RET_REASON_ERR_PASSWD;
            
			printf("line %d err %d \n", __LINE__, sRslt.nErrCode);
			
            return;
        }
		printf("line %d err %d \n", __LINE__, p_result->isSuccess);
        
        p_result->isSuccess = 1;
        p_result->failReason = RET_REASON_SUCCESS;

        NETCOMM_DEBUG(1);
    }
}

static void deal_get_stream_param(u32 channel, cmd_result_t *p_result)
{
    if (p_result)
    {
        if (channel < g_Dvr.max_videos)
        {
    #if 1
	    	PNetCommCommandDeal 	pCB;
			SRemoteCmdReq 			sReqIns;
			SRemoteCmdRslt 			sRslt;
			ifly_SubStreamParam_t 	*pSubStreamParam;
			
			pCB = cmdProc_GetCmdCB(CTRL_CMD_GETSUBSTREAMPARAM);
			if(pCB)
			{
				pCB(&sReqIns, &sRslt);
			}
			else
				return;

			pSubStreamParam = &sRslt.sBasicInfo.SubStreamParam;
			
            TLV_V_StreamDataFormat *p_streamFormat = NULL;

            p_streamFormat = (TLV_V_StreamDataFormat *)p_result->p_paramExt;
            if (p_streamFormat)
            {
                p_streamFormat->videoChannel = channel;//p_owspStreamFormat->channel;
                if (channel >= g_Dvr.max_audios)
                {
                    p_streamFormat->dataType = OWSP_SDT_VIDEO_ONLY;
                }
                else
                {
                    p_streamFormat->dataType = OWSP_SDT_VIDEO_AUDIO;
                }
                
                p_streamFormat->dataType = OWSP_SDT_VIDEO_ONLY;
                p_streamFormat->videoFormat.bitrate = 64;//pSubStreamParam->sub_intraRate;//p_owspStreamFormat->video.bitRate;
                p_streamFormat->videoFormat.codecId = CODEC_H264;//p_owspStreamFormat->codecId
    	      	p_streamFormat->videoFormat.colorDepth = 24;//没有回调参数
    	      	p_streamFormat->videoFormat.framerate = 5;//pSubStreamParam->sub_framerate;//ViEncoderParam.frame_rate;
				
				//尺寸支持QCIF（176×144），192×176，QVGA(320×240)，CIF（352×288）。
				//192×176未知格式
                if (0 == pSubStreamParam->sub_flag)
                {
                    p_streamFormat->videoFormat.width = 176;
                    p_streamFormat->videoFormat.height = ((NTSC == g_Dvr.yFactor) ? 120 : 144);
                }
                else if(1 == pSubStreamParam->sub_flag)
                {
                    p_streamFormat->videoFormat.width = 320;//352;
                    p_streamFormat->videoFormat.height = 240;//((NTSC == g_Dvr.yFactor) ? 240 : 288);
                }
                
                /********** 	 vStreamDataFormat.audioFormat		  **********/
                //填充数据由对方提供2009-06-02
                p_streamFormat->audioFormat.samplesPerSecond = 8000;
                p_streamFormat->audioFormat.bitrate = 12200;
                p_streamFormat->audioFormat.waveFormat = WAVE_FORMAT_AMR_CBR;
                p_streamFormat->audioFormat.channelNumber = 1;
                p_streamFormat->audioFormat.blockAlign = 0;
                p_streamFormat->audioFormat.bitsPerSample = 16;
                p_streamFormat->audioFormat.frameInterval = 200;
            }
	#else
			ifly_record_t rs;
			GetRecordSet(&rs, channel);
			
            TLV_V_StreamDataFormat *p_streamFormat = NULL;
			
            p_streamFormat = (TLV_V_StreamDataFormat *)p_result->p_paramExt;
            if (p_streamFormat)
            {
                p_streamFormat->videoChannel = channel;//p_owspStreamFormat->channel;
                if (channel >= g_Dvr.max_audios)
                {
                    p_streamFormat->dataType = OWSP_SDT_VIDEO_ONLY;
                }
                else
                {
                    p_streamFormat->dataType = OWSP_SDT_VIDEO_AUDIO;
                }
                
                p_streamFormat->dataType = OWSP_SDT_VIDEO_ONLY;
                p_streamFormat->videoFormat.bitrate = rs.sub_bitrate;//p_owspStreamFormat->video.bitRate;
                p_streamFormat->videoFormat.codecId = CODEC_H264;//p_owspStreamFormat->codecId
    	      	p_streamFormat->videoFormat.colorDepth = 24;//没有回调参数
    	      	p_streamFormat->videoFormat.framerate = rs.sub_framerate;//ViEncoderParam.frame_rate;
				
				//尺寸支持QCIF（176×144），192×176，QVGA(320×240)，CIF（352×288）。
				//192×176未知格式
                if (0 == rs.sub_video_resolution)
                {
                    p_streamFormat->videoFormat.width = 176;
                    p_streamFormat->videoFormat.height = ((NTSC == getYFactor()) ? 120 : 144);
                }
                else if(1 == rs.sub_video_resolution)
                {
                    p_streamFormat->videoFormat.width = 352;
                    p_streamFormat->videoFormat.height = ((NTSC == getYFactor()) ? 240 : 288);
                }
                
                /********** 	 vStreamDataFormat.audioFormat		  **********/
                //填充数据由对方提供2009-06-02
                p_streamFormat->audioFormat.samplesPerSecond = 8000;
                p_streamFormat->audioFormat.bitrate = 12200;
                p_streamFormat->audioFormat.waveFormat = WAVE_FORMAT_AMR_CBR;
                p_streamFormat->audioFormat.channelNumber = 1;
                p_streamFormat->audioFormat.blockAlign = 0;
                p_streamFormat->audioFormat.bitsPerSample = 16;
                p_streamFormat->audioFormat.frameInterval = 200;
            }
	#endif
        }
    }
}

// ctl: 0-start, 1-stop
static void mbStreamReq( u8 nCh, u8 ctl )
{
	STcpStreamReq sStreamReq;
	memset(&sStreamReq, 0, sizeof(STcpStreamReq));
	
	sStreamReq.nChn=nCh;
	if(ctl)
	{
		sStreamReq.nAct=EM_START;
		sStreamReq.nType=EM_PREQ_SUB_MB;
	}
	else
	{
		sStreamReq.nAct=EM_STOP;
		sStreamReq.nType=EM_PREQ_SUB_MB;
	}
	
	g_Dvr.pReqCB(&sStreamReq);
}

static void deal_channel_switch(channel_switch_t channel_siwtch, cmd_result_t *p_result)
{
	u32 gop;
	
    if (p_result)
    {
        if (channel_siwtch.origin_chn < g_Dvr.max_videos && channel_siwtch.dest_chn < g_Dvr.max_videos)
        {
        	int nSubFr;
        #if 1
			//printf("### deal_channel_switch %d\n", channel_siwtch.operate);
			
	    	PNetCommCommandDeal 	pCB;
			SRemoteCmdReq 			sReqIns;
			SRemoteCmdRslt 			sRslt;
			ifly_SubStreamParam_t 	*pSubStreamParam;
			
			pCB = cmdProc_GetCmdCB(CTRL_CMD_GETSUBSTREAMPARAM);
			if(pCB)
			{
				pCB(&sReqIns, &sRslt);
			}
			else
				return;
			
			pSubStreamParam = &sRslt.sBasicInfo.SubStreamParam;
			nSubFr = pSubStreamParam->sub_framerate;
        #else
            p_result->reserve[0] = channel_siwtch.dest_chn;
			GetRecordSet(&rs, channel_siwtch.dest_chn);
			nSubFr = pSubStreamParam->sub_framerate;
        #endif
			
            switch (channel_siwtch.operate)
            {
                case CHANNEL_START:
					//获得通道设置
                     //对channel_siwtch.dest_chn设手机需要的gop
                    gop = getSubVideoGop(channel_siwtch.dest_chn);
                    g_Dvr.p_subGop[channel_siwtch.dest_chn] = gop;
                    gop = nSubFr * 5;
                    setSubVideoGop(channel_siwtch.dest_chn, gop);
					
                    //开始采集标志dest_chn
                    if( g_Dvr.p_status[channel_siwtch.dest_chn]==0 )
                    {
                    	g_Dvr.p_status[channel_siwtch.dest_chn] = 1;
                    	mbStreamReq(channel_siwtch.dest_chn, 1);
                    }
                    break;
                case CHANNEL_STOP:
					//结束采集标志origin_chn
                    if( g_Dvr.p_status[channel_siwtch.dest_chn]==1 )
                    {
                    	g_Dvr.p_status[channel_siwtch.dest_chn] = 0;
                    	mbStreamReq(channel_siwtch.dest_chn, 0);
                    }
                    
                    //对channel_siwtch.origin_chn设原先的gop
                    gop = g_Dvr.p_subGop[channel_siwtch.origin_chn];
                    setSubVideoGop(channel_siwtch.origin_chn, gop);
					break;
                case CHANNEL_SWITCH:
					////结束采集标志origin_chn
					g_Dvr.p_status[channel_siwtch.origin_chn] = 0;
                    mbStreamReq(channel_siwtch.origin_chn, 0);

                    //对channel_siwtch.origin_chn设原先的gop
                    gop = g_Dvr.p_subGop[channel_siwtch.origin_chn];
                    setSubVideoGop(channel_siwtch.origin_chn, gop);


                    //对channel_siwtch.dest_chn设手机需要的gop
                    gop = getSubVideoGop(channel_siwtch.dest_chn);
                    g_Dvr.p_subGop[channel_siwtch.dest_chn] = gop;
                    gop = nSubFr * 5;
                    setSubVideoGop(channel_siwtch.dest_chn, gop);

					//开始采集标志dest_chn
					g_Dvr.p_status[channel_siwtch.dest_chn] = 1;
                    mbStreamReq(channel_siwtch.dest_chn, 1);
                    break;
                case CHANNEL_NOCHANGE:
                    p_result->isSuccess = 1;
                    p_result->failReason = RET_REASON_SUCCESS;
                    return;
                case CHANNEL_DONE:
                {
                    //对channel_siwtch.dest_chn用强制关键帧
                    setSubVideoForce(channel_siwtch.dest_chn);
					
                    p_result->isSuccess = 1;
                    p_result->failReason = RET_REASON_SUCCESS;
                    return;
                }
                case CHANNEL_REJECT:
                    break;
                 default:
                    break;
            }
/*
            printf("DVRSDK_openSubVideoChannel ret=%d\n", ret);
            if (NETDVR_SUCCESS == ret)
            {
                p_result->isSuccess = 1;
                p_result->failReason = RET_REASON_SUCCESS;
            }
            else
            {
                p_result->isSuccess = 0;
                p_result->failReason = 0;//待处理
            }
*/
		}
        else
        {
            p_result->isSuccess = 0;
            p_result->failReason = RET_REASON_ERR_INVALID_CHANNEL;//待处理
        }
    }
}

static void  deal_control(control_param_t control, cmd_result_t *p_result)
{
	TCamInfo tCamInfo;
    int ret;
	int g_ptz_move_speed = HIGH_SPEED;

	if (control.channel >= g_Dvr.max_videos)
    {
        p_result->isSuccess = 0;
        p_result->failReason = RET_REASON_ERR_INVALID_CHANNEL;
        
        return;
    }
	
  	memset(&tCamInfo, 0, sizeof(TCamInfo));

	//云台控制
	//控制命令:
	//0-停止 1上 2下 3 左 4右 5自转 
	//6变倍加 7变倍减 8焦距加 9焦距减 10光圈加 
	//11光圈减 12灯光开 13灯光关 14雨刷开 15雨刷关 
	//16 快速 17常速 18慢速

#define NET_PTZCTRL_STOPALL		0
#define NET_PTZCTRL_UP			1
#define NET_PTZCTRL_DOWN		2
#define NET_PTZCTRL_LEFT		3
#define NET_PTZCTRL_RIGHT		4
#define NET_PTZCTRL_ZOOMPLUS	6
#define NET_PTZCTRL_ZOOMMINUS	7
#define NET_PTZCTRL_AUTO		5
#define NET_PTZCTRL_FOCUSPLUS	8
#define NET_PTZCTRL_FOCUSMINUS	9
#define NET_PTZCTRL_IRISPLUS	10
#define NET_PTZCTRL_IRISMINUS	11
#define NET_PTZCTRL_LIGHTON		12
#define NET_PTZCTRL_LIGHTOFF	13
#define NET_PTZCTRL_BRUSHON		14
#define NET_PTZCTRL_BRUSHOFF	15
#define NET_PTZCTRL_FAST		16
#define NET_PTZCTRL_NORMAL		17
#define NET_PTZCTRL_SLOW		18


    switch (control.cmdCode)
    {
        case OWSP_ACTION_MD_STOP:
			tCamInfo.m_byCmdId = NET_PTZCTRL_STOPALL;
            break;
        case OWSP_ACTION_ZOOMReduce:
			tCamInfo.m_byCmdId = NET_PTZCTRL_ZOOMMINUS;
			tCamInfo.m_abyParam[ZOOM_SPEED_POS] = g_ptz_move_speed;
            break;
        case OWSP_ACTION_ZOOMADD:
          	tCamInfo.m_byCmdId = NET_PTZCTRL_ZOOMPLUS;
			tCamInfo.m_abyParam[ZOOM_SPEED_POS] = g_ptz_move_speed; 
		    break;
        case OWSP_ACTION_FOCUSADD:
			tCamInfo.m_byCmdId = NET_PTZCTRL_FOCUSPLUS;
			tCamInfo.m_abyParam[FOCUS_SPEED_POS] = g_ptz_move_speed;
            break;
        case OWSP_ACTION_FOCUSReduce:
			tCamInfo.m_byCmdId = NET_PTZCTRL_FOCUSMINUS;
			tCamInfo.m_abyParam[FOCUS_SPEED_POS] = g_ptz_move_speed;
            break;
        case OWSP_ACTION_MD_UP:
			tCamInfo.m_abyParam[TILT_SPEED_POS] = g_ptz_move_speed;
			tCamInfo.m_byCmdId = NET_PTZCTRL_UP;	
            break;
        case OWSP_ACTION_MD_DOWN:
			tCamInfo.m_abyParam[TILT_SPEED_POS] = g_ptz_move_speed;
			tCamInfo.m_byCmdId = NET_PTZCTRL_DOWN;	
            break;
        case OWSP_ACTION_MD_LEFT:
			tCamInfo.m_abyParam[PAN_SPEED_POS] = g_ptz_move_speed;
			tCamInfo.m_byCmdId = NET_PTZCTRL_LEFT;	
            break;
        case OWSP_ACTION_MD_RIGHT:
			tCamInfo.m_abyParam[PAN_SPEED_POS] = g_ptz_move_speed;
			tCamInfo.m_byCmdId = NET_PTZCTRL_RIGHT; 
            break;	
        case OWSP_ACTION_Circle_Add:
			tCamInfo.m_byCmdId = NET_PTZCTRL_IRISPLUS;	
            break;	
        case OWSP_ACTION_Circle_Reduce:
			tCamInfo.m_byCmdId = NET_PTZCTRL_IRISMINUS; 
            break;		
        case OWSP_ACTION_AUTO_CRUISE:
			tCamInfo.m_byCmdId = NET_PTZCTRL_AUTO;	
            break;	
        case OWSP_ACTION_GOTO_PRESET_POSITION:
        case OWSP_ACTION_SET_PRESET_POSITION:
        case OWSP_ACTION_CLEAR_PRESET_POSITION:
        case OWSP_ACTION_ACTION_RESET:
        default:
            p_result->isSuccess = 0;
            p_result->failReason = RET_REASON_ERR_PROTOCOL;
            return;
    }

#if 1

	PNetCommCommandDeal 	pCB;
	SRemoteCmdReq 			sReqIns;
	SRemoteCmdRslt 			sRslt;
	ifly_PtzCtrl_t 			*pPtzCtrl;
	
	pCB = cmdProc_GetCmdCB(CTRL_CMD_PTZCTRL);
	if(pCB)
	{
    	printf("control.channel=%d cmdCode=%d\n", control.channel, tCamInfo.m_byCmdId);
		pPtzCtrl = &sReqIns.sReq.PtzCtrl;
		pPtzCtrl->chn = control.channel;
		pPtzCtrl->cmd = tCamInfo.m_byCmdId;
		pCB(&sReqIns, &sRslt);
	}
	else
		return;
	
	ret = sRslt.nErrCode;
#else
    //printf("before NETDVR_PtzControl\n");
    ret = ptz_control(control.channel, &tCamInfo);
    //printf("after NETDVR_PtzControl ret=%d\n", ret);      
#endif
    if (0 == ret)
    {
        p_result->isSuccess = 1;
        p_result->failReason = RET_REASON_SUCCESS;
    }
    else
    {
        p_result->isSuccess = 0;
        p_result->failReason = RET_REASON_ERR_RUN;
    }     
}

#if 0
extern int PTZfd;
int ptz_control(u8 chn, TCamInfo* pCamInfo)
{

	#ifdef CRUISE_INDEX_PRESETPOS
	wait_cruise_stop();
	#endif
	
	ifly_yuntai_t yt;
	TCamData tCamData;

	GetYunTaiParam(&yt, chn);
	
	
	if(set_speed(PTZfd, yt.baud_rate) == -1)
	{
		printf("do_ptz_control Set speed Error\n");
		return -1;
	}
	if(set_Parity(PTZfd, yt.data_bit, yt.stop_bit, 'N') == -1)
	{
		printf("do_ptz_control Set Parity Error\n");
		return -1;
	}
	
	pCamInfo->m_byType = yt.protocol;
	pCamInfo->m_dwId = yt.address;

	pCamInfo->m_abyParam[SENDER_ADDR_POS] = 0;
	pCamInfo->m_byCtrlWay = CAM_CTRLWAY_GENERAL;
	GetCamProtocol(pCamInfo, &tCamData);
	if (tCamData.m_dwLen) write(PTZfd, tCamData.m_abyData, tCamData.m_dwLen);

	return 0;
}
#endif

static u32 getSubVideoGop(int chn)
{
    return 0;
}

static int setSubVideoGop(int chn, u32 gop)
{
	return 0;

}

static int setSubVideoForce(int chn)
{
	return 0;

}

u8 mobile_get_status(u32 chn)
{
	//printf("status %d ch %d \n", g_Dvr.p_status[chn], chn);
    if (chn < g_Dvr.max_videos)
        return g_Dvr.p_status[chn];
    return 0;
}

