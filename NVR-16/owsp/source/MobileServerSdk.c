#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>

#include "owsp_tl_def.h"
#include "MobileServerSdk.h"
#include "Huashi_OWSP.h"

#define MAX_LINKS_FOR_ONE_CHANNEL 5

static int g_listen_port = 101;
static int g_video_channels = 16;
static int g_audio_channels = 16;
static int g_max_links = 80;
static int g_server_sock = INVALID_SOCK;
   
static owspLink_t *gp_links = NULL;    

pfun_mobile_server_cmd_cb gp_cmd_callback = NULL;
   
static int startMobileServerListen(void);
static void* mobileServerListenFxn(void* arg);
static int create_listen_sock(u_int16 listen_port, u_int16 max_links);
static int waitDataLink(int *p_linkId);
static void transelate_command(owspLink_t *p_owspLink, mobile_server_cmd_t *p_dstCmd, owsp_command_t *p_srcCmd);
static int getFreeLink(void);
static int addToSucceessLinks(owspLink_t *p_owspLink, int freeIndex);
//static void removeLink(owspLink_t *p_owspLink);
static int getChannelLinks(int chn);

#define MBSDK_PRINT(x)	 // printf("line %d err %d \n", __LINE__, x)

int initMobileServerSdk(u_int16 port, int video_channels, int audio_channels, pfun_mobile_server_cmd_cb p_callback)
{
    int i;
	
    printf("func %s line %d\n", __FUNCTION__, __LINE__);
    
    g_listen_port = port;
    g_video_channels = video_channels;
    g_audio_channels = audio_channels;
    gp_cmd_callback = p_callback;
    
	MBSDK_PRINT(sizeof(owspLink_t));
	MBSDK_PRINT(g_max_links);
	
    g_max_links = g_video_channels * MAX_LINKS_FOR_ONE_CHANNEL;
    gp_links = (owspLink_t *)malloc(sizeof(owspLink_t) * g_max_links);
	
    if (NULL == gp_links)
    {
        return RET_FAILED;
    }
	
	MBSDK_PRINT(0);
	
	//printf("initMobileServerSdk\n");
	memset(gp_links, 0, sizeof(owspLink_t) * g_max_links);
	
    for (i = 0; i < g_max_links; i++)
    {
        owspInitLink(&gp_links[i]);
    }
	
	MBSDK_PRINT(0);
	
    return startMobileServerListen();
}

static int startMobileServerListen(void)
{
    int ret;
    pthread_t listen;
    
    ret = pthread_create(&listen, NULL, mobileServerListenFxn, NULL);
    if (ret)
    {
        return RET_FAILED;
    }
    
	MBSDK_PRINT(0);
	
	return RET_SUCCESS;
}

static void* mobileServerListenFxn(void* arg)
{
	int tmp_sock = INVALID_SOCK;
	int ret;
	int addrlen;
	int hTmp = INVALID_SOCK;
	int linkId;
	owsp_command_t cmd;
	mobile_server_cmd_t tmp_cmd;
	owspResponse_t owspResp;
	owspLink_t *p_tmpLink;
	cmd_result_t tmp_result;
	char buf[OWSP_MAX_RCV_LENGTH];
	
	printf("$$$$$$$$$$$$$$$$$$mobileServerListenFxn id:%d\n",getpid());
    
	MBSDK_PRINT(0);
	
    tmp_sock = create_listen_sock(g_listen_port, g_max_links);
    if (INVALID_SOCK == tmp_sock)
    {
        printf("mobileServerListenFxn INVALID_SOCK == tmp_sock\n");
        return NULL;
    }
	
	MBSDK_PRINT(0);
	
    g_server_sock = tmp_sock;
	
    while (NULL == gp_cmd_callback)
    {
    	printf("........\n");
        usleep(50 * 1000);
    }
	
    //监听新连接及成功连接的命令
    while (1)
    {
        //等待有请求的连接
        tmp_sock = waitDataLink(&linkId);

        if (INVALID_SOCK == tmp_sock)
        {
             continue;
        }
        
    	//printf("func %s line %d %p\n", __FUNCTION__, __LINE__, &tmp_result);
        
        if (tmp_sock == g_server_sock)
        {
            //accept临时sock
            struct timeval timeout;
            struct sockaddr_in clientaddr;
            
	     	addrlen = sizeof(struct sockaddr_in);
            timeout.tv_sec = 10;
            timeout.tv_usec = 0;
         	
            hTmp = accept(g_server_sock, (struct sockaddr *)&clientaddr, (socklen_t*)&addrlen);
            
            printf("get link\n");
			
            //新连接参数设置
           setsockopt(hTmp, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(struct timeval));
#if 0
	//设置NODELAY选项
	int optval = 1;
	ret = setsockopt( hTmp, IPPROTO_TCP/*SOL_SOCKET*/, TCP_NODELAY, (char *)&optval, sizeof(optval) );
	if( SOCK_ERROR == ret )
	{
		close(hTmp);
		continue;
	}

	//设置SO_LINGER为零(亦即linger结构中的l_onoff域设为非零,但l_linger为0),便
	//不用担心closesocket调用进入“锁定”状态(等待完成),不论是否有排队数据未发
	//送或未被确认。这种关闭方式称为“强行关闭”，因为套接字的虚电路立即被复位，尚
	//未发出的所有数据都会丢失。在远端的recv()调用都会失败，并返回WSAECONNRESET错误。
	{
		struct linger m_sLinger;
		m_sLinger.l_onoff = 1;  //(在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)
		m_sLinger.l_linger = 1; //(容许逗留的时间为0秒)				//dong	100424		m_sLinger.l_linger = 0; 
		ret = setsockopt(hTmp,SOL_SOCKET,SO_LINGER,(char*)&m_sLinger,sizeof(struct linger));
		if ( SOCK_ERROR == ret )
		{
    		close(hTmp);
    		continue;
		}
	}
#endif
            owspLink_t tmpLink;
            owspInitLink(&tmpLink);
            //printf("tmpLink ok\n");
          
            //接收数据
            tmpLink.sock = hTmp;
            ret = owspRcvData(&tmpLink, buf, sizeof(buf));
            if (0 == ret)
            {
                close(hTmp);
                printf("Rcv is error:%d\n",ret);
                continue;
            }
            
            printf("Rcv is ok:%d\n", ret);
            
            //解析登录命令及通道号
            ret = owspParseData(buf, ret, &cmd);
            if (OWSP_FAILED == ret)
            {
                close(hTmp);
                continue;
            }
            tmpLink.chn = cmd.login.channel;
            printf("tmpLink.chn %d\n", tmpLink.chn);

            //命令转换
            transelate_command( &tmpLink, &tmp_cmd, &cmd);

            //获得空闲连接
            int freeIndex = getFreeLink();
            
            //满则回复失败
            if (-1 == freeIndex)
            {
                //owspResponse(type, param);
                close(hTmp);
                continue;              
            }  
              
            //回调处理登录
            (*gp_cmd_callback)(&tmp_cmd, (void*)&tmp_result);

    		/*
			printf("gp_cmd_callback %p tmp_result.isSuccess %08x %d %d %d %d &tmp_result %p\n", 
				gp_cmd_callback, *(unsigned long*)(&tmp_result.isSuccess),
				tmp_result.failReason,
				tmp_result.reserve[0],
				tmp_result.reserve[1],
				tmp_result.reserve[2],
				&tmp_result
		    );
		    */
		    
            owspResp.type = TLV_T_VERSION_INFO_REQUEST;//TLV_T_VERSION_INFO_ANSWER;
            owspResp.isSuccess = tmp_result.isSuccess;
            owspResp.failedReason = tmp_result.failReason;
            owspResp.reserve[0] = g_video_channels;
            owspResponse(&tmpLink, &owspResp);

            //登录失败给客户端反及关闭临时sock
            //printf("tmp_result.isSuccess %d \n", tmp_result.isSuccess);
            if (0 == tmp_result.isSuccess)
            {
                owspResp.failedReason = tmp_result.failReason;
                owspResp.type = TLV_T_LOGIN_ANSWER;
                owspResponse(&tmpLink, &owspResp);
                close(hTmp);
                continue;              
            }
            else
            {
                if (tmpLink.chn >= g_video_channels)
                {
                    owspResp.isSuccess = 0;
                    owspResp.failedReason = _RESPONSECODE_INVALID_CHANNLE;
                    owspResp.type = TLV_T_LOGIN_ANSWER;
                    owspResponse(&tmpLink, &owspResp);
                    close(hTmp);
                    continue;                     
                }
            }

            printf("login ok\n");

            //登录成功则进行成功反馈并加入到成功连接
            #if 1
            //printf("owspResponse ***************1\n");

            owspResp.type = TLV_T_DVS_INFO_REQUEST;//TLV_T_DVS_INFO_ANSWER;
            owspResp.failedReason = tmp_result.failReason;
            owspResp.reserve[0] = g_video_channels;
            owspResponse(&tmpLink, &owspResp);
            //printf("owspResponse ***************2\n");

            owspResp.type = TLV_T_CHANNLE_ANSWER;
            owspResp.failedReason = tmp_result.failReason;
            owspResponse(&tmpLink, &owspResp);

            TLV_V_StreamDataFormat streamFormat;
            memset(&streamFormat, 0, sizeof(streamFormat));
            tmp_cmd.cmd = MOBILE_SERVER_COMMAND_GETPARAM_STREAM;
            tmp_cmd.channel = tmpLink.chn;
            tmp_result.p_paramExt = (char*)&streamFormat;
            gp_cmd_callback(&tmp_cmd, (void*)&tmp_result);
            
            owspResp.type = TLV_T_STREAM_FORMAT_INFO;
            owspResp.failedReason = tmp_result.failReason;
            owspResp.p_paramExt = tmp_result.p_paramExt;
            owspResponse(&tmpLink, &owspResp);
            
            owspResp.type = TLV_T_LOGIN_ANSWER;
            owspResp.failedReason = tmp_result.failReason;
            owspResponse(&tmpLink, &owspResp);
            #endif
            //printf("owspResponse ok\n");

            //回调处理音视频请求并进行成功失败回复
            //tmp_cmd == 音视频请求
            int tmpDestChannels = getChannelLinks(tmpLink.chn);
            if (0 == tmpDestChannels)
            {
                tmp_cmd.cmd = MOBILE_SERVER_COMMAND_SWITCH_CHANNEL;
                tmp_cmd.channel_switch.origin_chn = tmpLink.chn;
                tmp_cmd.channel_switch.dest_chn = tmpLink.chn;
                tmp_cmd.channel_switch.operate = CHANNEL_START;
                //printf("MOBILE_SERVER_COMMAND_SWITCH_CHANNEL CHANNEL_START ok\n");
            }
            else
            {
                tmp_cmd.cmd = MOBILE_SERVER_COMMAND_SWITCH_CHANNEL;
                tmp_cmd.channel_switch.origin_chn = cmd.login.channel;
                tmp_cmd.channel_switch.dest_chn = cmd.login.channel;
                tmp_cmd.channel_switch.operate = CHANNEL_DONE;
            }

            addToSucceessLinks(&tmpLink, freeIndex);  

            gp_cmd_callback(&tmp_cmd, (void*)&tmp_result);
            //printf("gp_cmd_callback ok\n");
        }
        else    
        {
            p_tmpLink = &gp_links[linkId];
            //接收数据
            ret = owspRcvData(p_tmpLink, buf, sizeof(buf));
            if (0 == ret)
            {
                p_tmpLink->isSockErr = 1;
                //removeLink(p_tmpLink);
                printf("Rcv is error 2:%d\n",ret);
                continue;
            }
             
            //解析登录命令及通道号
            ret = owspParseData(buf, ret, &cmd);
            if (OWSP_FAILED == ret)
            {
                p_tmpLink->isSockErr = 1;
                printf("owspParseData failed ************2\n");
                //removeLink(p_tmpLink);
                continue;
            }
            //printf("owspParseData ok ************2\n");

            transelate_command(p_tmpLink, &tmp_cmd, &cmd);

            //printf("transelate_command ok ************1\n");

            //回调处理
            gp_cmd_callback(&tmp_cmd, (void*)&tmp_result);

             //printf("gp_cmd_callback ok ************1\n");

            owspResp.type = cmd.cmd;
            owspResp.isSuccess = tmp_result.isSuccess;
            owspResp.failedReason = tmp_result.failReason;
            owspResp.reserve[0] = tmp_result.reserve[0];
            owspResp.reserve[1] = tmp_result.reserve[1];
            owspResp.reserve[2] = tmp_result.reserve[2];
            owspResponse(p_tmpLink, &owspResp);
            //printf("owspResponse ok ************1\n");
        }
    }

    return NULL;
}

static int create_listen_sock(u_int16 listen_port, u_int16 max_links)
{
    int listenfd;
    struct sockaddr_in serveraddr;

    //printf("func %s line %d listen_port %d\n", __FUNCTION__, __LINE__, listen_port);
    
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == INVALID_SOCK)
    {
    	return INVALID_SOCK;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = 0;//htonl(INADDR_ANY);
    serveraddr.sin_port = htons(listen_port);
    
    if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)))
    {
       close(listenfd);
	return INVALID_SOCK;
    }

    if (listen(listenfd, max_links))
    {
        close(listenfd);
	return INVALID_SOCK;
    }
    
    return listenfd;
}

static int waitDataLink(int *p_linkId)
{
    fd_set set;
    int i;
    int ret;
    int max_fd = -1;

    if (NULL == p_linkId)
    {
        return INVALID_SOCK;
    }

    FD_ZERO(&set);
    FD_SET(g_server_sock, &set); 
    max_fd = g_server_sock;


    if (gp_links)
    {
        for (i = 0; i < g_max_links; i++)
        {
            if (gp_links[i].sock != INVALID_SOCK)
            {
           		//printf("i %d gp_links[i].sock %d \n", i, gp_links[i].sock);
           		
                if (gp_links[i].isSockErr)
                {
                    mobile_server_cmd_t tmp_cmd;
                    cmd_result_t tmp_result1;
                    
                    tmp_cmd.cmd = MOBILE_SERVER_COMMAND_SWITCH_CHANNEL;
                    tmp_cmd.channel_switch.dest_chn = gp_links[i].chn;
                    tmp_cmd.channel_switch.origin_chn = tmp_cmd.channel_switch.dest_chn;
                    tmp_cmd.channel_switch.operate = CHANNEL_STOP;
                    
                    gp_cmd_callback(&tmp_cmd, (void*)&tmp_result1);

                    if (tmp_result1.isSuccess)
                    {
                        owspLockMutex(&gp_links[i].mutex);
                        close(gp_links[i].sock);
                       
                        owspInitLink(&gp_links[i]);
                        owspUnlockMutex(&gp_links[i].mutex);
                        continue;
                    }
                }
            
                FD_SET(gp_links[i].sock, &set);
                if (gp_links[i].sock > max_fd)
                {
                    max_fd = gp_links[i].sock;
                }
            }
        }
    }

    //堵塞超时等待
    //判断哪个sock有数据
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    ret = select(max_fd + 1, &set, NULL, NULL, &timeout); 
    if (0 == ret || SOCK_ERROR == ret)
    {
        return INVALID_SOCK;
    }

    printf("func %s line %d \n", __FUNCTION__, __LINE__);

    if (g_server_sock != INVALID_SOCK && FD_ISSET(g_server_sock, &set))
    {
        return g_server_sock;
    }

    if (gp_links)
    {
        for (i = 0; i < g_max_links; i++)
        {
            if (gp_links[i].sock != INVALID_SOCK && FD_ISSET(gp_links[i].sock, &set))
            {
                *p_linkId = i;
                return gp_links[i].sock;
            }
        }
    }

    return INVALID_SOCK;
}

static int getChannelLinks(int chn)
{
    int i;
    int count = 0;
    
    if (gp_links)
    {
        for (i = 0; i < g_max_links; i++)
        {
            if (chn == gp_links[i].chn && INVALID_SOCK != gp_links[i].sock)
            {
                count++;
            }
        }
    }

    return count;
}

static void transelate_command(owspLink_t *p_owspLink, mobile_server_cmd_t *p_dstCmd, owsp_command_t *p_srcCmd)
{
    if (p_srcCmd && p_dstCmd && p_owspLink)
    {
        int chn = p_owspLink->chn;
        
        switch (p_srcCmd->cmd)
        {
            case OWSP_COMMAND_LOGIN:
                 p_dstCmd->cmd = MOBILE_SERVER_COMMAND_LOGIN;
                 strcpy(p_dstCmd->login.userName, p_srcCmd->login.userName);
                 strcpy(p_dstCmd->login.password, p_srcCmd->login.password);  
                 break;
            case OWSP_COMMAND_SWITCH_CHANNEL:
            {
                int orgin_chn = p_srcCmd->ch_req.sourceChannel;
                int dest_chn = p_srcCmd->ch_req.destChannel;
                int orginChannellinkCount = 0;
                int destChannellinkCount = 0;

                p_dstCmd->channel_switch.origin_chn = orgin_chn;
                p_dstCmd->channel_switch.dest_chn = dest_chn;

                p_dstCmd->cmd = MOBILE_SERVER_COMMAND_SWITCH_CHANNEL;
                if (chn == orgin_chn && orgin_chn != dest_chn)
                {
                    orginChannellinkCount = getChannelLinks(orgin_chn);
                    destChannellinkCount = getChannelLinks(dest_chn);
                           
                    if (orginChannellinkCount <= 1)
                    {
                        if (destChannellinkCount <= 0)
                        {
                            p_dstCmd->channel_switch.operate = CHANNEL_SWITCH;
                        }
                        else
                        {
                            p_dstCmd->channel_switch.operate = CHANNEL_STOP;
                        }
                    }
                    else
                    {
                        if (destChannellinkCount <= 0)
                        {
                            p_dstCmd->channel_switch.operate = CHANNEL_START;
                        }
                        else
                        {
                            p_dstCmd->channel_switch.operate = CHANNEL_DONE;
                        }
                    }
                }
                else
                {
                    if (orgin_chn == dest_chn)
                    {
                        p_dstCmd->channel_switch.operate = CHANNEL_NOCHANGE;
                    }
                    else
                    {
                        p_dstCmd->channel_switch.operate = CHANNEL_REJECT;
                    }
                }
                break;
            }
            case OWSP_COMMAND_CONTROL:
            {
                p_dstCmd->cmd = MOBILE_SERVER_COMMAND_CONTROL;

                memcpy(&p_dstCmd->control, &p_srcCmd->control, sizeof(p_srcCmd->control));
                
                break;
            }
            default:
                 break;
        }    
    }
}


void sendVideoToMobileServer(int chn, videoDataParam_t *p_VideoData)
{
    if (p_VideoData)
    {
        owspVideoData_t owspVideo;
        int i;
        
        // MAX_TLV_LEN 最大包长
        if (p_VideoData->dataSize <=  MAX_TLV_LEN)
        {
            owspVideo.dataSize = p_VideoData->dataSize;
            owspVideo.frameIndex = p_VideoData->frameID;
            owspVideo.iFrame = p_VideoData->isKeyFrame;
            owspVideo.p_data = p_VideoData->p_Data;
            owspVideo.timeStamp = p_VideoData->timeStamp;
            owspVideo.encType = p_VideoData->encodeType;
            owspVideo.channel = chn;
            //printf("sendVideoToMobileServer size=%d\n", p_VideoData->dataSize);
            
            for (i = 0; i < g_max_links; i++)
            {
                if (chn == gp_links[i].chn && INVALID_SOCK != gp_links[i].sock)
                {
                    //printf("i=%d iFrame=%d\n", i, owspVideo.iFrame);
                    
                    if (0 == gp_links[i].videoFrameCount && 0 == owspVideo.iFrame)
                    {
                        continue;
                    }
                    owspSendVideo(&gp_links[i], &owspVideo);
                    
                    //printf("after owspSendVideo\n");
                }
            }
        }
        else //> MAX_TLV_LEN
        {
            printf("data too long\n");
        }
    }
}

static int getFreeLink(void)
{
    int freeIndex = 0;
    
    for (freeIndex = 0; freeIndex < g_max_links; freeIndex++)
    {       
        if (INVALID_SOCK == gp_links[freeIndex].sock && -1 == gp_links[freeIndex].chn)
        {
           return freeIndex;
        }
    }
    return -1;
}


static int addToSucceessLinks(owspLink_t *p_owspLink, int freeIndex)
{
    if (freeIndex >= 0 && freeIndex < g_max_links && p_owspLink)
    {
        int chn = p_owspLink->chn;
        int sock = p_owspLink->sock;
        
        if (INVALID_SOCK == gp_links[freeIndex].sock && -1 == gp_links[freeIndex].chn)
        {
            //printf("freeIndex=%d\n", freeIndex);
            gp_links[freeIndex].isSockErr = 0;
            gp_links[freeIndex].packet_seq = p_owspLink->packet_seq;
            gp_links[freeIndex].videoFrameCount = 0;
            gp_links[freeIndex].sock = sock;
            gp_links[freeIndex].chn = chn;

            printf("add success freeIndex=%d\n", freeIndex);

            return RET_SUCCESS;
        }
    }
    printf("add failed\n");
    
    return RET_FAILED;
}

#if 0
static void removeLink(owspLink_t *p_link)
{
    if (p_link)
    {
        owspMutex_t * p_Mutex = &p_link->mutex;
        
        owspLockMutex(p_Mutex);
        close(p_link->sock);
        p_link->sock = INVALID_SOCK;
        p_link->chn = -1;
        p_link->isSockErr = 0;
        p_link->packet_seq = SEQ_INIT;
        owspUnlockMutex(p_Mutex);
    }
}
#endif

