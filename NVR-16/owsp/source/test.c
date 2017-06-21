#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "owsp_tl_def.h"
#include "MobileServerSdk.h"

#include "iflytype.h"
#include "LocalSdk.h"

#define CTRL_CMD_BASE 10000
#define CTRL_CMD_MAKE_KEYFRAME CTRL_CMD_BASE+113  //强制主/子码流产生一个关键帧

typedef struct
{
u8 chn; //通道 0开始
u8 type; // 0 主码流 1子码流
u8 reserved[6]; //预留
} ifly_makekeyframe_t;  // 8 Byte


typedef struct 
{
    int hDev;
    int max_videos;
    int max_audios;
    u32 *p_subGop;
} dvr_t;

static dvr_t g_Dvr;

static void DealMediaFrameCB(pFrameHeadr pFrmHdr, unsigned int dwContext);

static void  deal_command(mobile_server_cmd_t server_cmd, cmd_result_t *p_result);
static void  deal_Login (login_param_t login, cmd_result_t *p_result);
static void deal_get_stream_param(int channel, cmd_result_t *p_result);
static void deal_control(control_param_t control, cmd_result_t *p_result);
static void deal_channel_switch(channel_switch_t channel_switch, cmd_result_t *p_result);
static void loop_command(int is_test);

int main(int argc, char *argv[])
{
    int ret;
    int hdev;
    int port = 26868;
    struct NETDVR_DeviceInfo_t si;

    if (argc > 1)
    {
        port = atoi(argv[1]);
        if (0 == port)
        {
            port = 26868;
        }
    }

    signal(SIGPIPE,SIG_IGN);//忽略SIGPIPE信号

    ret = DVRSDK_startup(&hdev);
    if (NETDVR_SUCCESS != ret)
    {
    	printf("DVRSDK_startup failed\n");

    	return -1;
    }

    memset(&g_Dvr, 0, sizeof(g_Dvr));
    g_Dvr.hDev = hdev;

    ret = NETDVR_GetDeviceInfo(hdev, &si);
    if (NETDVR_SUCCESS != ret)
    {
    	printf("NETDVR_GetDeviceInfo failed\n");
    	return -1;
    }
    printf("GetDeviceInfo success:(%d,%d,%d,%d,%08x,%d)\n", si.maxChnNum, si.maxAudioNum, si.maxAlarmInNum, si.maxAlarmOutNum, si.deviceIP
        ,si.devicePort);

    g_Dvr.max_videos = si.maxChnNum;
    g_Dvr.max_audios = si.maxAudioNum;

    g_Dvr.p_subGop = (u32 *) malloc(sizeof(u32) * si.maxChnNum);
    if (NULL == g_Dvr.p_subGop)
    {
        return -1;
    }

    ret = initMobileServerSdk(port, si.maxChnNum, si.maxAudioNum, (pfun_mobile_server_cmd_cb)deal_command);
    if (RET_FAILED == ret)
    {
        printf("initMobileServerSdk failed\n");
        return -1;
    }

    if (argc > 2)
    {
        loop_command(1);
    }
    else
    {
        loop_command(0);
    }
    
    return 0;
}

static void DealMediaFrameCB(pFrameHeadr pFrmHdr, unsigned int dwContext)
{
    if (pFrmHdr)
    {
        //int ret;
        int chn = dwContext;

        //printf("chn %d media data is %s\n", chn + 1, (pFrmHdr->mediaType == MEDIA_TYPE_H264)?"video":"audio");
        //printf("media data len:%d\n",pFrmHdr->dataSize);
        if (pFrmHdr->mediaType == MEDIA_TYPE_H264)
        {
            if (chn < g_Dvr.max_videos)
            {
                videoDataParam_t videoData;

                videoData.encodeType = CODEC_H264;
                videoData.dataSize = pFrmHdr->dataSize;
                videoData.isKeyFrame = pFrmHdr->videoParam.keyFrame;//((pFrmHdr->videoParam.keyFrame > 0) ? 1 : 0);
                videoData.p_Data = pFrmHdr->pData;
                videoData.frameID = pFrmHdr->frameID;
                videoData.timeStamp = pFrmHdr->timeStamp;
                sendVideoToMobileServer(chn, &videoData);
                //printf("media data width:%d,height:%d\n",pFrmHdr->videoParam.videoWidth,pFrmHdr->videoParam.videoHeight);
            }
            
            //printf("media data frame type:%s\n",(pFrmHdr->videoParam.keyFrame)?"I帧":"P帧");
        }
        else
        {
  
           // printf("audio data\n");
        }
        //printf("media timestamp:%u\n",pFrmHdr->timeStamp);
        //printf("media frameID:%u\n",pFrmHdr->frameID);
        //printf("media frameRate:%u\n",pFrmHdr->frameRate);
    }
}

void loop_command(int is_test)
{
    while(1)
    {
        if (!is_test)
        {
            sleep(1);
        }
        else
        {
            char cmdline[512];
            printf("[user]#");

            memset(cmdline,0,sizeof(cmdline));

            fgets(cmdline, sizeof(cmdline), stdin);

            cmdline[strlen(cmdline)-1] = '\0';

            if(strcmp(cmdline,"closewd") == 0)
            {

            }
            else if(strcmp(cmdline,"exit") == 0)
            {
                exit(1);
            }
        }
    }
}

static void  deal_command(mobile_server_cmd_t server_cmd, cmd_result_t *p_result)
{
    if (NULL == p_result)
    {
        return;
    }
    
    switch (server_cmd.cmd)
    {
        case MOBILE_SERVER_COMMAND_LOGIN:
            printf("userName:%s,pasword:%s\n",server_cmd.login.userName,server_cmd.login.password);
            deal_Login(server_cmd.login, p_result);
            break;
        case MOBILE_SERVER_COMMAND_CONTROL:
            deal_control(server_cmd.control, p_result);
            break;
        case MOBILE_SERVER_COMMAND_SWITCH_CHANNEL:
            printf("MOBILE_SERVER_COMMAND_SWITCH_CHANNEL deal_channel_switch\n");
            deal_channel_switch(server_cmd.channel_switch, p_result);
            break;
        case MOBILE_SERVER_COMMAND_GETPARAM_STREAM:
            deal_get_stream_param(server_cmd.channel, p_result);
            break;
        case MOBILE_SERVER_COMMAND_NULL:
             p_result->isSuccess = 1;
            p_result->failReason = 0;
        default:           
            p_result->isSuccess = 0;
            p_result->failReason = RET_REASON_ERR_RUN;
            break;
    }
}

static void deal_Login (login_param_t login, cmd_result_t *p_result)
{
    int ret;
    
    if (p_result)
    {
        struct NETDVR_UserNumber_t userMng;
        
        ret = NETDVR_GetUserInfo(g_Dvr.hDev, &userMng);//login
        if (NETDVR_SUCCESS == ret)
        {
            int i;
            int userNum = sizeof(struct NETDVR_UserNumber_t) / sizeof(struct NETDVR_userInfo_t);

            for (i = 0; i < userNum; i++)
            {
                if (strlen(userMng.userinfo[i].name) > 0)
                {
                    if (0 == strcmp(userMng.userinfo[i].name, login.userName))
                    {
                        break; 
                    }
                }
            }

            if (i >= userNum)
            {
                p_result->isSuccess = 0;
                p_result->failReason = RET_REASON_NOUSER;
                return;
            }
            
            if (strcmp(userMng.userinfo[i].password, login.password))
            {
                p_result->isSuccess = 0;
                p_result->failReason = RET_REASON_ERR_PASSWD;
                return;
            }
            
            p_result->isSuccess = 1;
            p_result->failReason = RET_REASON_SUCCESS;
        }
        else
        {
            p_result->isSuccess = 0;
            p_result->failReason = RET_REASON_ERR_NOLOGIN;
        }

        #if 0
        switch (ret)
        {
            case NETDVR_SUCCESS:
                p_result->isSuccess = 1;
                p_result->failReason = RET_REASON_SUCCESS;
                break;
            case NETDVR_ERR_NOUSER:
                break;
            case 	NETDVR_ERR_PASSWD:
                break;
            case NETDVR_ERR_MACADDR:
                break;
            case NETDVR_ERR_RELOGIN:
                break;
            case NETDVR_ERR_NOLOGIN:
                break;
            default:
                break;
        }
        #endif
    }
}

static void deal_get_stream_param(int channel, cmd_result_t *p_result)
{
    int ret;
    
    if (p_result)
    {
        if (channel < g_Dvr.max_videos)
        {
            struct NETDVR_SubStreamParam_t subStreamParam;
            struct NETDVR_systemParam_t systemParam;
            TLV_V_StreamDataFormat *p_streamFormat = NULL;

            memset(&systemParam, 0, sizeof(systemParam));
            memset(&subStreamParam, 0, sizeof(subStreamParam));
            
            ret = NETDVR_getSystemParams(g_Dvr.hDev, &systemParam);
			
            ret = NETDVR_GetSubStreamParam(g_Dvr.hDev, channel, &subStreamParam);
            
            if (NETDVR_SUCCESS == ret)
            {
                p_streamFormat = (TLV_V_StreamDataFormat *)p_result->p_paramExt;
                if (p_streamFormat)
                {
                    p_streamFormat->videoChannel = channel;//p_owspStreamFormat->channel;
                    p_streamFormat->audioChannel = channel;
                    if (channel >= g_Dvr.max_audios)
                    {
                        p_streamFormat->dataType = OWSP_SDT_VIDEO_ONLY;
                    }
                    else
                    {
                        p_streamFormat->dataType = OWSP_SDT_VIDEO_AUDIO;
                    }
                    p_streamFormat->dataType = OWSP_SDT_VIDEO_ONLY;
                    p_streamFormat->videoFormat.bitrate = subStreamParam.sub_bitrate;//p_owspStreamFormat->video.bitRate;
                    p_streamFormat->videoFormat.codecId = CODEC_H264;//p_owspStreamFormat->codecId
					p_streamFormat->videoFormat.colorDepth = 24;//没有回调参数
					p_streamFormat->videoFormat.framerate = subStreamParam.sub_framerate;//ViEncoderParam.frame_rate;
					
					//尺寸支持QCIF（176×144），192×176，QVGA(320×240)，CIF（352×288）。
					//192×176未知格式
                    if (NETDVR_SUBFLAG_QCIF == subStreamParam.sub_flag)
                    {
                        p_streamFormat->videoFormat.width = 176;
                        p_streamFormat->videoFormat.height = ((NETDVR_VIDEOFORMAT_NTSC == systemParam.video_format) ? 120 : 144);
                    }
                    else if(NETDVR_SUBFLAG_CIF == subStreamParam.sub_flag)
                    {
                        p_streamFormat->videoFormat.width = 352;
                        p_streamFormat->videoFormat.height = ((NETDVR_VIDEOFORMAT_NTSC == systemParam.video_format) ? 240 : 288);
                    }
                    
                    /********** 	 vStreamDataFormat.audioFormat		  **********/
                    //填充数据由对方提供2009-06-02
                    p_streamFormat->audioFormat.samplesPerSecond = 8000;
                    p_streamFormat->audioFormat.bitrate = 12200;
                    p_streamFormat->audioFormat.waveFormat = WAVE_FORMAT_AMR_CBR ;
                    p_streamFormat->audioFormat.channelNumber =1;
                    p_streamFormat->audioFormat.blockAlign = 0;
                    p_streamFormat->audioFormat.bitsPerSample = 16;
                    p_streamFormat->audioFormat.frameInterval = 200;
                }
            }
        }
    }
}

static void deal_channel_switch(channel_switch_t channel_siwtch, cmd_result_t *p_result)
{
    int ret = NETDVR_ERROR;

    if (p_result)
    {       
        if (channel_siwtch.origin_chn < g_Dvr.max_videos && channel_siwtch.dest_chn < g_Dvr.max_videos)
        {
            p_result->reserve[0] = channel_siwtch.dest_chn;

            struct NETDVR_SubStreamParam_t subStreamParam;
            
            switch (channel_siwtch.operate)
            {
                case CHANNEL_START:
                     //对channel_siwtch.dest_chn设手机需要的gop
                    memset(&subStreamParam, 0, sizeof(subStreamParam));
                    ret = NETDVR_GetSubStreamParam(g_Dvr.hDev, channel_siwtch.dest_chn, &subStreamParam);
                    g_Dvr.p_subGop[channel_siwtch.dest_chn] = subStreamParam.sub_intraRate;
                    subStreamParam.sub_intraRate = subStreamParam.sub_framerate * 5;
                    subStreamParam.chn = channel_siwtch.dest_chn,
                    ret = NETDVR_SetSubStreamParam(g_Dvr.hDev, &subStreamParam);
                    
                    ret = DVRSDK_openSubVideoChannel(g_Dvr.hDev, channel_siwtch.dest_chn, DealMediaFrameCB, channel_siwtch.dest_chn);
                    
                    break;
                case CHANNEL_STOP:
                    ret = DVRSDK_closeSubVideoChannel(g_Dvr.hDev, channel_siwtch.origin_chn);             
                    //对channel_siwtch.origin_chn设原先的gop
                    memset(&subStreamParam, 0, sizeof(subStreamParam));
                    ret = NETDVR_GetSubStreamParam(g_Dvr.hDev, channel_siwtch.origin_chn, &subStreamParam);
                    subStreamParam.sub_intraRate = g_Dvr.p_subGop[channel_siwtch.origin_chn];
                    subStreamParam.chn = channel_siwtch.origin_chn,
                    ret = NETDVR_SetSubStreamParam(g_Dvr.hDev, &subStreamParam);
                    
                    break;
                case CHANNEL_SWITCH:
                    ret = DVRSDK_closeSubVideoChannel(g_Dvr.hDev, channel_siwtch.origin_chn);
                    //对channel_siwtch.origin_chn设原先的gop
                    memset(&subStreamParam, 0, sizeof(subStreamParam));
                    ret = NETDVR_GetSubStreamParam(g_Dvr.hDev, channel_siwtch.origin_chn, &subStreamParam);
                    subStreamParam.sub_intraRate = g_Dvr.p_subGop[channel_siwtch.origin_chn];
                    subStreamParam.chn = channel_siwtch.origin_chn,
                    ret = NETDVR_SetSubStreamParam(g_Dvr.hDev, &subStreamParam);

                    //对channel_siwtch.dest_chn设手机需要的gop
                    memset(&subStreamParam, 0, sizeof(subStreamParam));
                    ret = NETDVR_GetSubStreamParam(g_Dvr.hDev, channel_siwtch.dest_chn, &subStreamParam);
                    g_Dvr.p_subGop[channel_siwtch.dest_chn] = subStreamParam.sub_intraRate;
                    subStreamParam.sub_intraRate = subStreamParam.sub_framerate * 5;
                    subStreamParam.chn = channel_siwtch.dest_chn,
                    ret = NETDVR_SetSubStreamParam(g_Dvr.hDev, &subStreamParam);
                   
                    ret = DVRSDK_openSubVideoChannel(g_Dvr.hDev, channel_siwtch.dest_chn, DealMediaFrameCB, channel_siwtch.dest_chn);
                    break;
                case CHANNEL_NOCHANGE:
                    p_result->isSuccess = 1;
                    p_result->failReason = RET_REASON_SUCCESS;
                    return;
                case CHANNEL_DONE:
                {
                    //对channel_siwtch.dest_chn用强制关键帧
                    ifly_makekeyframe_t keyframe_request;
                    memset(&keyframe_request, 0, sizeof(keyframe_request));
                    keyframe_request.chn = channel_siwtch.dest_chn;
                    keyframe_request.type = 1;
                    ret = NETDVR_sendExtendCmd(g_Dvr.hDev, CTRL_CMD_MAKE_KEYFRAME, &keyframe_request, sizeof(keyframe_request), NULL, 0);
                    
                    p_result->isSuccess = 1;
                    p_result->failReason = RET_REASON_SUCCESS;
                    return;
                }
                case CHANNEL_REJECT:
                    break;
                 default:
                    break;
            }

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
    struct NETDVR_PtzCtrl_t ptzParam;
    int ret;

    if (control.channel >= g_Dvr.max_videos)
    {
        p_result->isSuccess = 0;
        p_result->failReason = RET_REASON_ERR_INVALID_CHANNEL;
        
        return;
    }

    memset(&ptzParam, 0, sizeof(struct NETDVR_PtzCtrl_t));
    
    ptzParam.chn = control.channel;

    printf("control.channel=%d cmdCode=%d\n", control.channel, control.cmdCode);

    switch (control.cmdCode)
    {
        case OWSP_ACTION_MD_STOP:
            ptzParam.cmd = NETDVR_PTZ_COM_STOP;
            break;
        case OWSP_ACTION_ZOOMReduce:
            ptzParam.cmd = NETDVR_PTZ_COM_ZOOMSUBTRACT;
            break;
        case OWSP_ACTION_ZOOMADD:
            ptzParam.cmd = NETDVR_PTZ_COM_ZOOMADD;
            break;
        case OWSP_ACTION_FOCUSADD:
            ptzParam.cmd = NETDVR_PTZ_COM_FOCUSADD;
            break;
        case OWSP_ACTION_FOCUSReduce:
            ptzParam.cmd = NETDVR_PTZ_COM_FOCUSSUBTRACT;
            break;
        case OWSP_ACTION_MD_UP:
            ptzParam.cmd = NETDVR_PTZ_COM_MOVEUP;
            break;
        case OWSP_ACTION_MD_DOWN:
            ptzParam.cmd = NETDVR_PTZ_COM_MOVEDOWN;
            break;
        case OWSP_ACTION_MD_LEFT:
            ptzParam.cmd = NETDVR_PTZ_COM_MOVELEFT;
            break;
        case OWSP_ACTION_MD_RIGHT:
            ptzParam.cmd = NETDVR_PTZ_COM_MOVERIGHT;
            break;	
        case OWSP_ACTION_Circle_Add:
            ptzParam.cmd = NETDVR_PTZ_COM_APERTUREADD;
            break;	
        case OWSP_ACTION_Circle_Reduce:
            ptzParam.cmd = NETDVR_PTZ_COM_APERTURESUBTRACT;
            break;		
        case OWSP_ACTION_AUTO_CRUISE:
            ptzParam.cmd = NETDVR_PTZ_COM_ROTATION;
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

    //printf("before NETDVR_PtzControl\n");
    ret = NETDVR_PtzControl(g_Dvr.hDev, &ptzParam);
    //printf("after NETDVR_PtzControl ret=%d\n", ret);
    if (NETDVR_SUCCESS == ret)
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

