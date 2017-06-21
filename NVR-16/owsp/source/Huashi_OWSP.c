#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <pthread.h>

#include "owsp_tl_def.h"
#include "Huashi_OWSP.h"
#include <netinet/tcp.h>

#define NETSNDRCVFLAG	MSG_NOSIGNAL
//#define VERSION_MAJOR	4
//#define VERSION_MINOR	1

#define MAX_PART_SIZE (32 << 10)
#define MAX_VIDEO_FRAME_SIZE (70 << 10)
#define MAX_VIDEO_FRAME_SIZE_EX (100 << 10)

static int  Version_Info_Request(char *buf);
static int  User_Login_Request(char *  buf, owsp_command_t *p_cmd);
static int  Ptz_Control_Request(char *buf, owsp_command_t *p_cmd);
static  int  Switch_Channel_Request(char *buf, owsp_command_t *p_cmd);
static  int  Phone_Info_Request(u_int32 phone);
static  int Suspend_Channel_Request(u_int32 suspend);
static int recv_loop(int fd, char *buffer, unsigned int len);
static int SendData(owspLink_t *p_owspLink, char *databuffer, int size);
static int Net_select(int s, int sec, int usec, short x);

//#define RCV_DATA_FILE "rcv.dat"
#ifdef RCV_DATA_FILE
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

u_int32 owspRcvData(owspLink_t *p_owspLink, char *buf, u_int32 buf_len)
{
    if (NULL == p_owspLink)
    {
        return 0;
    }

    int sock = p_owspLink->sock;

    #ifdef RCV_DATA_FILE
    time_t curr;
    curr = time(NULL);
    char path[64];
    sprintf(path, "time%lu%s", curr, RCV_DATA_FILE);
    printf("before open path:%s\n", path);
    int fd = open(path, O_RDWR|O_CREAT);
    printf("after open\n");
    #endif
    
    owspMutex_t *p_Mutex = &p_owspLink->mutex;
	
    owspLockMutex(p_Mutex);
	
    OwspPacketHeader tmp_header;
	
    if (recv_loop(sock, (char *)&tmp_header, sizeof(tmp_header)) <= 0)
    {
        p_owspLink->isSockErr = 1;
        owspUnlockMutex(p_Mutex);
        return 0;
    }
	//printf("\nsizeof(tmp_header)=%d\n", sizeof(tmp_header));
	
    #ifdef RCV_DATA_FILE
    int ret = write(fd, (char *)&tmp_header, sizeof(tmp_header));
    printf("fd:%d, write %d bytes\n", fd, ret);
    #endif
    
    tmp_header.packet_length = ntohl(tmp_header.packet_length);
    if (buf_len < tmp_header.packet_length - sizeof(tmp_header.packet_seq))
    {
        p_owspLink->isSockErr = 1;
        owspUnlockMutex(p_Mutex);
        return 0;
    }
	//printf("\npacket_length=%u packet_seq=%u\n", tmp_header.packet_length, tmp_header.packet_seq);
	
    if (recv_loop(sock, buf, tmp_header.packet_length - sizeof(tmp_header.packet_seq)) <= 0)
    {
        p_owspLink->isSockErr = 1;
        owspUnlockMutex(p_Mutex);
        return 0;
    }
	
    #ifdef RCV_DATA_FILE
    ret = write(fd, buf, tmp_header.packet_length - sizeof(tmp_header.packet_seq));
    printf("fd:%d, write %d bytes\n", fd, ret);
    close(fd);
    #endif
	
    owspUnlockMutex(p_Mutex);
	
    return tmp_header.packet_length - sizeof(tmp_header.packet_seq);
}

owsp_ret_t owspParseData(char *buf, u_int32 buf_len, owsp_command_t *p_cmd)
{
    TLV_HEADER	*tlvHeaderTemp = NULL;
    TLV_HEADER	  tlvHeader;

    int	  iCount = 0;
    int	  iFlag = OWSP_SUCCESS;

    u_int32	ulResLen = 0;
    u_int32	ulReadLen = 0;
    //printf("owsp Parse Data\n");

    ulResLen = buf_len;

    while(ulResLen > 0)
    {
        tlvHeaderTemp = (TLV_HEADER *)(buf + ulReadLen);
        tlvHeader.tlv_len = tlvHeaderTemp->tlv_len;
        tlvHeader.tlv_type = tlvHeaderTemp->tlv_type;
        ulResLen -= sizeof(TLV_HEADER);
        ulReadLen += sizeof(TLV_HEADER);

        ++iCount;

        printf("TLV Sequece %d, Type %d, TLV Len %d\n", iCount, tlvHeader.tlv_type, tlvHeader.tlv_len);

        switch(tlvHeader.tlv_type)
        {
            case TLV_T_VERSION_INFO_REQUEST:
                iFlag = Version_Info_Request(buf + ulReadLen);
                //printf("The version is :%d\n",iFlag);
                break;
            case TLV_T_LOGIN_REQUEST:
                iFlag = User_Login_Request(buf + ulReadLen, p_cmd);

                //printf("The request is :%d\n",iFlag);
                break;
            case TLV_T_SENDDATA_REQUEST:
                printf("send data is \n");
                break;
            case TLV_T_SUSPENDSENDDATA_REQUEST:
                printf("suspend send data is \n");
                break;
            case TLV_T_DEVICE_FORCE_EXIT:
                printf("device eixt:\n");
                break;
            case TLV_T_CONTROL_REQUEST:
                printf("**********************Ptz_Control_Request*********************\n");
                iFlag = Ptz_Control_Request(buf + ulReadLen, p_cmd);
                break;
            case TLV_T_RECORD_REQUEST:
                break;
            case TLV_T_DEVICE_SETTING_REQUEST:
                break;
            case TLV_T_DEVICE_RESET:
                break;
            case TLV_T_CHANNLE_REQUEST:
                iFlag = Switch_Channel_Request(buf + ulReadLen, p_cmd);
                tlvHeader.tlv_len = sizeof(TLV_V_ChannelRequest);
                break;
            case TLV_T_SUSPEND_CHANNLE_REQUEST:
                iFlag = Suspend_Channel_Request(ulReadLen);

                printf("suspend Channel request is error\n");
                break;
            case TLV_T_VALIDATE_REQUEST:
            case TLV_T_DVS_INFO_REQUEST:
                break;
            case TLV_T_PHONE_INFO_REQUEST:
                iFlag = Phone_Info_Request(ulReadLen);

                printf("Phone request :%d\n",iFlag);
                break;
            default:
                iFlag = OWSP_FAILED;
            break;
        }

        if (OWSP_FAILED == iFlag)
        {
            return  OWSP_FAILED;
        }

        ulResLen -= tlvHeader.tlv_len;
        ulReadLen += tlvHeader.tlv_len;
    }

    return  OWSP_SUCCESS;
}


static int  Version_Info_Request(char *buf)
{
    TLV_V_VersionInfoRequest	*versionInfotemp = NULL;
    TLV_V_VersionInfoRequest	  versionInfo;
 
    //printf("OWSP Version_Info_Request Run\n");

    memset(&versionInfo, 0, sizeof(TLV_V_VersionInfoRequest));

    versionInfotemp = (TLV_V_VersionInfoRequest *) buf;
    versionInfo.versionMajor = versionInfotemp->versionMajor;
    versionInfo.versionMinor = versionInfotemp->versionMinor;

    //printf("versionMajor %d, versionMinor %d\n", versionInfo.versionMajor, versionInfo.versionMinor);

    return OWSP_SUCCESS;
}

static int  User_Login_Request(char *buf, owsp_command_t *p_cmd)
{
   TLV_V_LoginRequest   *LoginInfotemp = NULL;
    
   if (NULL == p_cmd)
   {
        return OWSP_FAILED;
   }  

    //printf("OWSP Login_Info_Request Run\n");

    LoginInfotemp = (TLV_V_LoginRequest *) buf;
    memcpy(&p_cmd->login, LoginInfotemp, sizeof(TLV_V_LoginRequest));
    p_cmd->cmd = OWSP_COMMAND_LOGIN;
    
    //printf ("LoginName:%s, LoginPaw:%s,LoginDer:%ld\n", p_cmd->login.userName, p_cmd->login.password, p_cmd->login.deviceId);
 
    //printf ("LoginFlag:%d,LoginChn:%d,LoginRerve:%p\n" , p_cmd->login.flag, p_cmd->login.channel, p_cmd->login.reserve);

    return OWSP_SUCCESS;
}

owsp_ret_t  owspInitLink(owspLink_t *p_owspLink)
{
    p_owspLink->chn = -1;
    p_owspLink->sock = INVALID_SOCK;
    p_owspLink->packet_seq = SEQ_INIT;
    p_owspLink->videoFrameCount = 0;
    p_owspLink->isSockErr = 0;
    owspInitMutex(&p_owspLink->mutex);

    return OWSP_SUCCESS;
}

owsp_ret_t  owspResponse(owspLink_t *p_owspLink, owspResponse_t *p_owspResp)
{
    char partBuff[MAX_PART_SIZE];
    u_int32 totalSize = sizeof(OwspPacketHeader) + sizeof(TLV_HEADER);
    u_int32 realSize = 0;
    char *pRealData = NULL;
    OwspPacketHeader *p_packetHeader;
    TLV_HEADER *p_tlvHeader;

    if (totalSize > MAX_PART_SIZE)
    {
        return OWSP_FAILED;
    }

    if (NULL == p_owspLink || NULL == p_owspResp)
    {
        return OWSP_FAILED;
    }
   
    printf("p_owspResp->type=%d\n", p_owspResp->type);

    p_packetHeader = (OwspPacketHeader *)partBuff;
    p_tlvHeader = (TLV_HEADER *)(partBuff + sizeof(OwspPacketHeader));
    pRealData = partBuff + totalSize;

    switch (p_owspResp->type)
    {
        case TLV_T_VERSION_INFO_REQUEST:
        {
           realSize = sizeof(TLV_V_VersionInfoRequest);
           if (realSize + totalSize > MAX_PART_SIZE)
            {
                 return OWSP_FAILED;
            }
            TLV_V_VersionInfoRequest *p_VerInfoRequest;
            p_VerInfoRequest = (TLV_V_VersionInfoRequest *) pRealData;
            p_VerInfoRequest->versionMajor = VERSION_MAJOR;
            p_VerInfoRequest->versionMajor = VERSION_MINOR;

            p_tlvHeader->tlv_type = p_owspResp->type;
            p_tlvHeader->tlv_len =  realSize;

            #if 0
            p_tlvHeader = (TLV_HEADER *)(pRealData + realSize);
            p_tlvHeader->tlv_type = TLV_T_DVS_INFO_REQUEST;
            p_tlvHeader->tlv_len =  sizeof(TLV_V_DVSInfoRequest);
            realSize += sizeof(TLV_HEADER);
            if (realSize + totalSize > MAX_PART_SIZE)
            {
                 return OWSP_FAILED;
            }            TLV_V_DVSInfoRequest *p_DvsInfoRequest;
            p_DvsInfoRequest = (TLV_V_DVSInfoRequest *) (pRealData + realSize);
            realSize += sizeof(TLV_V_DVSInfoRequest);
            if (realSize + totalSize > MAX_PART_SIZE)
            {
                 return OWSP_FAILED;
            }
            memset(p_DvsInfoRequest, 0, sizeof(TLV_V_DVSInfoRequest));
            p_DvsInfoRequest->channleNumber = p_owspResp->reserve[0];
			sprintf(p_DvsInfoRequest->companyIdentity, "%s", OWSP_QIYANG_P2P_COMPANY_IDENTITY);
			p_DvsInfoRequest->equipmentDate.m_year = 2010;
			p_DvsInfoRequest->equipmentDate.m_month = 4;
			p_DvsInfoRequest->equipmentDate.m_day = 1;
			
            p_tlvHeader = (TLV_HEADER *)(pRealData + realSize);
            p_tlvHeader->tlv_type = TLV_T_CHANNLE_ANSWER;
            p_tlvHeader->tlv_len =  sizeof(TLV_V_ChannelResponse);
            realSize += sizeof(TLV_HEADER);
            if (realSize + totalSize > MAX_PART_SIZE)
            {
                 return OWSP_FAILED;
            }            TLV_V_ChannelResponse *p_ChannelResponse;
            p_ChannelResponse = (TLV_V_ChannelResponse *) (pRealData + realSize);
            realSize += sizeof(TLV_V_ChannelResponse);
            if (realSize + totalSize > MAX_PART_SIZE)
            {
                 return OWSP_FAILED;
            }
            p_ChannelResponse->result = _RESPONSECODE_SUCC;
            p_ChannelResponse->currentChannel = p_owspLink->chn;
            p_ChannelResponse->reserve = 0;

            p_tlvHeader = (TLV_HEADER *)(pRealData + realSize);
            p_tlvHeader->tlv_type = TLV_T_STREAM_FORMAT_INFO;
            p_tlvHeader->tlv_len =  sizeof(TLV_V_StreamDataFormat);
            realSize += sizeof(TLV_HEADER);
            if (realSize + totalSize > MAX_PART_SIZE)
            {
                 return OWSP_FAILED;
            }
            TLV_V_StreamDataFormat *p_StreamDataFormatResponse;
            p_StreamDataFormatResponse = (TLV_V_StreamDataFormat *)(pRealData + realSize);
            realSize += sizeof(TLV_V_StreamDataFormat);
            if (realSize + totalSize > MAX_PART_SIZE)
            {
                 return OWSP_FAILED;
            }
            //owspStreamFormat_t *p_owspStreamFormat = (owspStreamFormat_t *)p_owspResp->reserve;
            memset(p_StreamDataFormatResponse, 0, sizeof(TLV_V_StreamDataFormat));
            p_StreamDataFormatResponse->videoChannel = p_owspLink->chn;//p_owspStreamFormat->channel;
            p_StreamDataFormatResponse->audioChannel = p_StreamDataFormatResponse->videoChannel;
            p_StreamDataFormatResponse->dataType = OWSP_SDT_VIDEO_ONLY;//p_owspStreamFormat->dataType;
            p_StreamDataFormatResponse->videoFormat.bitrate = 64 * 1024;//p_owspStreamFormat->video.bitRate;
			p_StreamDataFormatResponse->videoFormat.codecId = CODEC_H264;//p_owspStreamFormat->codecId
			p_StreamDataFormatResponse->videoFormat.colorDepth = 24;//没有回调参数
			p_StreamDataFormatResponse->videoFormat.framerate = 6;//ViEncoderParam.frame_rate;
			//尺寸支持QCIF（176×144），192×176，QVGA(320×240)，CIF（352×288）。
			//192×176未知格式
            if (0)
            {
                p_StreamDataFormatResponse->videoFormat.width = 176;
                p_StreamDataFormatResponse->videoFormat.height = ((0)?120:144);
            }
            else if(1)
            {
                p_StreamDataFormatResponse->videoFormat.width = 352;
                p_StreamDataFormatResponse->videoFormat.height = ((0)?240:288);
            }
            /********** 	 vStreamDataFormat.audioFormat		  **********/
            //填充数据由对方提供2009-06-02
            p_StreamDataFormatResponse->audioFormat.samplesPerSecond = 8000;
            p_StreamDataFormatResponse->audioFormat.bitrate = 12200;
            p_StreamDataFormatResponse->audioFormat.waveFormat = WAVE_FORMAT_AMR_CBR ;
            p_StreamDataFormatResponse->audioFormat.channelNumber = 1;
            p_StreamDataFormatResponse->audioFormat.blockAlign = 0;
            p_StreamDataFormatResponse->audioFormat.bitsPerSample = 16;
            p_StreamDataFormatResponse->audioFormat.frameInterval = 200;

            p_tlvHeader = (TLV_HEADER *)(pRealData + realSize);
            p_tlvHeader->tlv_type = TLV_T_LOGIN_ANSWER;
            p_tlvHeader->tlv_len =  sizeof(TLV_V_LoginResponse);
            realSize += sizeof(TLV_HEADER);
            TLV_V_LoginResponse *p_LoginResponse;
            p_LoginResponse = (TLV_V_LoginResponse *) (pRealData + realSize);
            realSize += sizeof(TLV_V_LoginResponse);
            if (realSize + totalSize > MAX_PART_SIZE)
            {
                 return OWSP_FAILED;
            }
            p_LoginResponse->result = _RESPONSECODE_SUCC;
            p_LoginResponse->reserve = 0;
            #endif
            break;
        }
        case TLV_T_VERSION_INFO_ANSWER:
        {
            realSize = sizeof(TLV_V_VersionInfoResponse);

            if (realSize + totalSize > MAX_PART_SIZE)
            {
                 return OWSP_FAILED;
            }
            
            TLV_V_VersionInfoResponse *p_VerInfoResponse;
            p_VerInfoResponse =(TLV_V_VersionInfoResponse *) pRealData;
            
            if (p_owspResp->isSuccess)
            {
                p_VerInfoResponse->result = _RESPONSECODE_SUCC;
                p_VerInfoResponse->reserve = 0;
            }
            else
            {
                p_VerInfoResponse->result = _RESPONSECODE_MAX_USER_ERROR;
                p_VerInfoResponse->reserve = 0;
            }

            p_tlvHeader->tlv_type = p_owspResp->type;
            p_tlvHeader->tlv_len =  realSize;

            break;
        }
        case TLV_T_DVS_INFO_REQUEST:
        {
            realSize = sizeof(TLV_V_DVSInfoRequest);

            if (realSize + totalSize > MAX_PART_SIZE)
            {
                 return OWSP_FAILED;
            }
    
            TLV_V_DVSInfoRequest *p_DvsInfoRequest;
            
            p_DvsInfoRequest = (TLV_V_DVSInfoRequest *) pRealData;
            memset(p_DvsInfoRequest, 0, sizeof(TLV_V_DVSInfoRequest));

            p_DvsInfoRequest->channleNumber = p_owspResp->reserve[0];
	     sprintf(p_DvsInfoRequest->companyIdentity, "%s", OWSP_QIYANG_P2P_COMPANY_IDENTITY);
	     p_DvsInfoRequest->equipmentDate.m_year = 2011;
	     p_DvsInfoRequest->equipmentDate.m_month = 3;
	     p_DvsInfoRequest->equipmentDate.m_day = 14;

            p_tlvHeader->tlv_type = p_owspResp->type;
            p_tlvHeader->tlv_len =  realSize;
             
            break;
        }
        case TLV_T_DVS_INFO_ANSWER:
        {
            realSize = sizeof(TLV_V_DVSInfoResponse);

            if (realSize + totalSize > MAX_PART_SIZE)
            {
                 return OWSP_FAILED;
            }
    
            TLV_V_DVSInfoResponse *p_DvsInfoResponse;
            p_DvsInfoResponse = (TLV_V_DVSInfoResponse *) pRealData;
            
            p_DvsInfoResponse->result = _RESPONSECODE_SUCC;
            p_DvsInfoResponse->reserve = 0;
             
            p_tlvHeader->tlv_type = p_owspResp->type;
            p_tlvHeader->tlv_len =  realSize;

            break;
        }
        case TLV_T_CHANNLE_ANSWER:
        {
            realSize = sizeof(TLV_V_ChannelResponse);

            if (realSize + totalSize > MAX_PART_SIZE)
            {
                 return OWSP_FAILED;
            }

            TLV_V_ChannelResponse *p_ChannelResponse;
            p_ChannelResponse = (TLV_V_ChannelResponse *) pRealData;

            if (p_owspResp->isSuccess)
            {
                p_ChannelResponse->result = _RESPONSECODE_SUCC;
                p_ChannelResponse->currentChannel = p_owspLink->chn;
                p_ChannelResponse->reserve = 0;
                
                owspLockMutex(&p_owspLink->mutex);
                if (p_owspLink->chn != p_ChannelResponse->currentChannel)
                {
                    p_owspLink->chn = p_ChannelResponse->currentChannel;
                    p_owspLink->videoFrameCount = 0;
                    printf("owspResponse videoFrameCount=0\n");
                }
                owspUnlockMutex(&p_owspLink->mutex);
            }
            else
            {
                p_ChannelResponse->result = _RESPONSECODE_INVALID_CHANNLE;
                p_ChannelResponse->currentChannel = p_owspLink->chn;
            }

            p_tlvHeader->tlv_type = p_owspResp->type;
            p_tlvHeader->tlv_len =  realSize;
            
            break;
        }
        case TLV_T_STREAM_FORMAT_INFO:
        {
            realSize = sizeof(TLV_V_StreamDataFormat);

            if (realSize + totalSize > MAX_PART_SIZE)
            {
                 return OWSP_FAILED;
            }

            TLV_V_StreamDataFormat *p_StreamDataFormatResponse;
            p_StreamDataFormatResponse = (TLV_V_StreamDataFormat *) pRealData;

            #if 1
            memcpy((void *)p_StreamDataFormatResponse, p_owspResp->p_paramExt, sizeof(TLV_V_StreamDataFormat));
            #else
            //owspStreamFormat_t *p_owspStreamFormat = (owspStreamFormat_t *)p_owspResp->reserve;
            memset(p_StreamDataFormatResponse, 0, sizeof(TLV_V_StreamDataFormat));

            p_StreamDataFormatResponse->videoChannel = 0;//p_owspStreamFormat->channel;
            p_StreamDataFormatResponse->audioChannel = p_StreamDataFormatResponse->videoChannel;
            p_StreamDataFormatResponse->dataType = OWSP_SDT_VIDEO_ONLY;//p_owspStreamFormat->dataType;
            p_StreamDataFormatResponse->videoFormat.bitrate = 64 * 1024;//p_owspStreamFormat->video.bitRate;
	     p_StreamDataFormatResponse->videoFormat.codecId = CODEC_H264;//p_owspStreamFormat->codecId
	     p_StreamDataFormatResponse->videoFormat.colorDepth = 24;//没有回调参数
	     p_StreamDataFormatResponse->videoFormat.framerate = 6;//ViEncoderParam.frame_rate;
			//尺寸支持QCIF（176×144），192×176，QVGA(320×240)，CIF（352×288）。
			//192×176未知格式
            if (0)
            {
                p_StreamDataFormatResponse->videoFormat.width = 176;
                p_StreamDataFormatResponse->videoFormat.height = ((0)?120:144);
            }
            else if(1)
            {
                p_StreamDataFormatResponse->videoFormat.width = 352;
                p_StreamDataFormatResponse->videoFormat.height = ((0)?240:288);
            }
            
            /********** 	 vStreamDataFormat.audioFormat		  **********/
            //填充数据由对方提供2009-06-02
            p_StreamDataFormatResponse->audioFormat.samplesPerSecond = 8000;
            p_StreamDataFormatResponse->audioFormat.bitrate = 12200;
            p_StreamDataFormatResponse->audioFormat.waveFormat = WAVE_FORMAT_AMR_CBR ;
            p_StreamDataFormatResponse->audioFormat.channelNumber =1;
            p_StreamDataFormatResponse->audioFormat.blockAlign = 0;
            p_StreamDataFormatResponse->audioFormat.bitsPerSample = 16;
            p_StreamDataFormatResponse->audioFormat.frameInterval = 200;
            #endif
            p_tlvHeader->tlv_type = p_owspResp->type;
            p_tlvHeader->tlv_len =  realSize;

            break;
        }
        case TLV_T_LOGIN_ANSWER:
        {
           realSize = sizeof(TLV_V_VersionInfoRequest);
           if (realSize + totalSize > MAX_PART_SIZE)
            {
                 return OWSP_FAILED;
            }
            TLV_V_VersionInfoRequest *p_VerInfoRequest;
            p_VerInfoRequest = (TLV_V_VersionInfoRequest *) pRealData;
            p_VerInfoRequest->versionMajor = VERSION_MAJOR;
            p_VerInfoRequest->versionMajor = VERSION_MINOR;
            p_tlvHeader->tlv_type = p_owspResp->type;
            p_tlvHeader->tlv_len =  realSize;

            p_tlvHeader = (TLV_HEADER *)(pRealData + realSize);
            p_tlvHeader->tlv_type = TLV_T_LOGIN_ANSWER;
            p_tlvHeader->tlv_len =  sizeof(TLV_V_LoginResponse);
            realSize += sizeof(TLV_HEADER);
            TLV_V_LoginResponse *p_LoginResponse;
            p_LoginResponse = (TLV_V_LoginResponse *) (pRealData + realSize);
            realSize += sizeof(TLV_V_LoginResponse);
            if (realSize + totalSize > MAX_PART_SIZE)
            {
                 return OWSP_FAILED;
            }
   
            p_LoginResponse->result = p_owspResp->failedReason;
            p_LoginResponse->reserve = 0;
            
            break;
        }
        default:
            return OWSP_FAILED;
    }
     
    p_packetHeader->packet_length = htonl(sizeof(TLV_HEADER) + realSize + 4);
    if (0 == p_owspLink->packet_seq)
    {
        p_owspLink->packet_seq = SEQ_INIT;
    }
    p_packetHeader->packet_seq = p_owspLink->packet_seq;
    totalSize += realSize;

    //printf("owspResponse before SendData\n");
    if (SendData(p_owspLink, partBuff, totalSize) == totalSize)
    {
        //printf("owspResponse ok\n");
        return OWSP_SUCCESS;
    }

    return OWSP_FAILED;
}


owsp_ret_t owspSendVideo(owspLink_t *p_owspLink, owspVideoData_t *p_owspVideo)
{
    char packBuff[MAX_VIDEO_FRAME_SIZE];
    OwspPacketHeader *p_PacketHeader;
    TLV_HEADER *p_TlvVframeInfohead;
    TLV_V_VideoFrameInfo *p_VFrameInfoHeader;
    TLV_HEADER *p_TlvVFrameDatahead;
    char * p_realData;
    u_int32 totalSize = sizeof(OwspPacketHeader) + sizeof(TLV_HEADER) * 2 + sizeof(TLV_V_VideoFrameInfo);
    u_int32 realSize;

    if (p_owspLink && p_owspVideo)
    {      
        realSize = p_owspVideo->dataSize;

        if (totalSize + realSize > MAX_VIDEO_FRAME_SIZE)
        {
            return OWSP_FAILED;
        }

        p_PacketHeader = (OwspPacketHeader *)packBuff;
        p_TlvVframeInfohead = (TLV_HEADER *) (packBuff + sizeof(OwspPacketHeader));
        p_VFrameInfoHeader = (TLV_V_VideoFrameInfo *)(((char *) p_TlvVframeInfohead) + sizeof(TLV_HEADER));
        p_TlvVFrameDatahead = (TLV_HEADER *)(((char *) p_VFrameInfoHeader) + sizeof(TLV_V_VideoFrameInfo));
        p_realData = packBuff + totalSize;

        if (0 == p_owspLink->packet_seq)
        {
            p_owspLink->packet_seq = SEQ_INIT;
        }
        p_PacketHeader->packet_seq = p_owspLink->packet_seq;
        p_PacketHeader->packet_length = htonl(totalSize + realSize - sizeof(p_PacketHeader->packet_length));

        p_TlvVframeInfohead->tlv_type = TLV_T_VIDEO_FRAME_INFO;
        p_TlvVframeInfohead->tlv_len = sizeof(TLV_V_VideoFrameInfo);

        p_VFrameInfoHeader->channelId = p_owspVideo->channel;
        p_VFrameInfoHeader->checksum = 0;
        p_VFrameInfoHeader->frameIndex = p_owspVideo->frameIndex;
        p_VFrameInfoHeader->reserve = 0;
        p_VFrameInfoHeader->time = p_owspVideo->timeStamp;

        if (p_owspVideo->iFrame)
        {
           p_TlvVFrameDatahead->tlv_type = TLV_T_VIDEO_IFRAME_DATA;
        }
        else
        {
            p_TlvVFrameDatahead->tlv_type = TLV_T_VIDEO_PFRAME_DATA;
        }
        p_TlvVFrameDatahead->tlv_len = realSize;

        memcpy(p_realData, p_owspVideo->p_data, realSize);

        totalSize += realSize;

        //printf("packet_seq=%d ch:%d size:%d type:%d frameid:%d, iframe:%d stamp:%u\n", p_owspLink->packet_seq, p_owspVideo->channel, p_owspVideo->dataSize
        //    , p_owspVideo->encType, p_owspVideo->frameIndex, p_owspVideo->iFrame, p_owspVideo->timeStamp);

        if (SendData(p_owspLink, packBuff, totalSize) == totalSize)
        {
            owspLockMutex(&p_owspLink->mutex);
            p_owspLink->videoFrameCount++;
            owspUnlockMutex(&p_owspLink->mutex);
            //printf("owspSendVideo ok videoFrameCount=%ld\n", p_owspLink->videoFrameCount);
            return OWSP_SUCCESS;
        }
    }

    return OWSP_FAILED;
}

static int SendData(owspLink_t *p_owspLink, char *databuffer, int size)
{
    int ret = 0;
    int wait_send_size = size;
    int sock = INVALID_SOCK;

    if (NULL == p_owspLink)
    {
        return -1;
    }

    sock = p_owspLink->sock;
    if (sock > 0)
    {
        owspMutex_t *p_Mutex;
        int	sec = 6;
        int 	usec = 0;
        short x = 1;//WRITE_STATUS;

        p_Mutex = &p_owspLink->mutex;

        //printf("SendData before LockMutex\n");
        owspLockMutex(p_Mutex);
        //printf("SendData after LockMutex\n");

        p_owspLink->packet_seq++;

        while(wait_send_size > 0)
        {
            if (wait_send_size >= MAX_PART_SIZE)
            {
                ret = Net_select(sock, sec, usec, x); 
                if (ret > 0)
                {
                    ret = send(sock, databuffer + size - wait_send_size, MAX_PART_SIZE, 0);
                    if (ret <= 0)
                    {
                        printf("Send data error*********1 errno = %d, err msg = %s\n", errno, strerror(errno));
                        p_owspLink->isSockErr = 1;
                        owspUnlockMutex(p_Mutex);
                        return size - wait_send_size;
                    }
                    else
                    {
                        //printf("wait_send_size=%d ret=%d MAX_PART_SIZE=%d\n", wait_send_size, ret, MAX_PART_SIZE);
                    }
                    
                    wait_send_size -= ret;
                }
            }
            else
            {
                ret = Net_select(sock, sec, usec, x); 
                if (ret > 0)
                {
                    ret = send(sock, databuffer + size - wait_send_size, wait_send_size, 0);
                    if (ret <= 0)
                    {
                        p_owspLink->isSockErr = 1;
                        printf("Send data error*********2\n");
                        owspUnlockMutex(p_Mutex);
                        return size - wait_send_size;
                    }
                    else
                    {
                        //printf("2******wait_send_size=%d ret=%d\n", wait_send_size, ret);
                    }

                    wait_send_size -= ret;
                }
            }
        }
        owspUnlockMutex(p_Mutex);
        //printf("wait_send_size=%d size=%d\n", wait_send_size, size);
        return size;
    }
    else
    {
        return -1;
    }
 }
    
static int Net_select(int s, int sec, int usec, short x)
{
    return 1;
#if 0
    int	st = errno;
    struct timeval	to;
    fd_set 	fs;
    to.tv_sec = sec;
    to.tv_usec = usec;
    FD_ZERO(&fs);
    FD_SET(s, &fs);

    switch(x)
    {
    	case READ_STATUS:
    		st = select(s+1, &fs, 0, 0, &to);
    		break;
    	case WRITE_STATUS:
    		st = select(s+1, 0, &fs, 0, &to);
              break;
    	case EXCPT_STATUS:
    		st = select(s+1, 0, 0, &fs, &to);
    		break;
	default:
		break;
     }
    return (st);
  #endif
}

static int  Ptz_Control_Request(char * buf, owsp_command_t *p_cmd)
{
    TLV_V_ControlRequest   *p_controlReq = NULL;

    if (NULL == p_cmd)
    {
        return OWSP_FAILED;
    }  

    p_controlReq = (TLV_V_ControlRequest *) buf;

    printf("PTZ Control deviceId:%lu channel:%d cmdCode:%d size:%d\n", 
		p_controlReq->deviceId, p_controlReq->channel, p_controlReq->cmdCode, p_controlReq->size);

    memcpy(&p_cmd->control, p_controlReq, sizeof(TLV_V_ControlRequest));
    
    p_cmd->cmd = OWSP_COMMAND_CONTROL;

    return OWSP_SUCCESS;

    return 0;
}

static  int  Switch_Channel_Request(char *buf, owsp_command_t *p_cmd)
{
    TLV_V_ChannelRequest   *channelReqPtr = NULL;
    
    if (NULL == p_cmd)
    {
        return OWSP_FAILED;
    }  

    channelReqPtr = (TLV_V_ChannelRequest *) buf;
    memcpy(&p_cmd->ch_req, channelReqPtr, sizeof(TLV_V_ChannelRequest));
    p_cmd->cmd = OWSP_COMMAND_SWITCH_CHANNEL;

    return OWSP_SUCCESS;
}

static  int  Phone_Info_Request(u_int32 phone)
{
   return 0;
}

static  int Suspend_Channel_Request(u_int32 suspend)
{
   return 0;
}

static int recv_loop(int fd, char *buffer, unsigned int len)
{
    unsigned int remain = len;
    unsigned int recvlen = 0;

    while(remain > 0)
    {
        int ret = recv(fd, (char *)(buffer+recvlen), remain, NETSNDRCVFLAG);
        if (ret <= 0)
        {
            printf("recv_loop failed:(%d,%s)\n",errno,strerror(errno));
            printf("recv_loop failed,ret=%d,remain=%d,recvlen=%d\n",ret,remain,recvlen);
            fflush(stdout);
            return -1;
        }

        remain -= ret;
        recvlen += ret;
    }
    
#if 0
    int i;
    for (i = 0; i < len; i++)
    {
        printf("%02x ", buffer[i]);
    }
    printf("\n");
#endif

    return len;
}

int  owspInitMutex(owspMutex_t *p_Mutex)
{
    return pthread_mutex_init(p_Mutex, NULL);
}

int  owspLockMutex(owspMutex_t *p_Mutex)
{
	return ( 0 == pthread_mutex_lock(p_Mutex) );
}

int  owspUnlockMutex(owspMutex_t *p_Mutex)
{
	return ( 0 == pthread_mutex_unlock(p_Mutex) );
}

