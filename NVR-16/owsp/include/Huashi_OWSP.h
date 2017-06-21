#ifndef _HUASHI_OWSP_
#define _HUASHI_OWSP_

#define OWSP_MAX_RCV_LENGTH 1024

#define SEQ_INIT 1

#define INVALID_SOCK (-1)
#define SOCK_ERROR (-1)

typedef enum
{
    OWSP_SUCCESS = 0, 
    OWSP_FAILED
 } owsp_ret_t;

enum
{
    OWSP_COMMAND_LOGIN = TLV_T_LOGIN_ANSWER,
    OWSP_COMMAND_SWITCH_CHANNEL = TLV_T_CHANNLE_ANSWER,
    OWSP_COMMAND_CONTROL = TLV_T_CONTROL_ANSWER,
};

typedef struct _OWSP_COMMAND_
{
    int cmd;
    union
    {
        TLV_V_LoginRequest login;
        TLV_V_ChannelRequest ch_req;
        TLV_V_ControlRequest control;
    };
    void *p_paramExt;
} owsp_command_t;

typedef pthread_mutex_t owspMutex_t;

typedef struct _owspLinks_
{
    int chn;
    int sock;
    u_int32 packet_seq;
    u_int32 videoFrameCount;
    u_int8 isSockErr;
    
    owspMutex_t mutex;
} owspLink_t;

typedef struct
{
    int type;
    int failedReason;
    u_int8 isSuccess;
    u_int8 reserve[3];
    char *p_paramExt;
} owspResponse_t;

typedef struct
{
    char * p_data;
    u_int32 dataSize;
    u_int32 timeStamp;
    u_int32 frameIndex;
    int encType;
    int channel;
    u_int8 iFrame;
} owspVideoData_t;

typedef struct
{
    int channel;
    
} owspStreamFormat_t;

u_int32 owspRcvData(owspLink_t *p_owspLink, char *buf, u_int32 buf_len);
owsp_ret_t owspParseData(char *buf, u_int32 buf_len, owsp_command_t *p_cmd);
owsp_ret_t  owspInitLink(owspLink_t *p_owspLink);
owsp_ret_t  owspResponse(owspLink_t *p_owspLink, owspResponse_t *p_owspResp);
owsp_ret_t owspSendVideo(owspLink_t *p_owspLink, owspVideoData_t *p_owspVideo);

int owspInitMutex(owspMutex_t *p_Mutex);
int owspLockMutex(owspMutex_t *p_Mutex);
int owspUnlockMutex(owspMutex_t *p_Mutex);
#endif


