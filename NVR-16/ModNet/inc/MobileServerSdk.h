#ifndef _MOBILE_SERVER_H_
#define _MOBILE_SERVER_H_

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    RET_SUCCESS = 0, 
    RET_FAILED
};

enum
{
    RET_REASON_SUCCESS = _RESPONSECODE_SUCC,
    RET_REASON_NOUSER = _RESPONSECODE_USER_PWD_ERROR,
    RET_REASON_ERR_PASSWD = _RESPONSECODE_USER_PWD_ERROR,
    RET_REASON_ERR_NOLOGIN = _RESPONSECODE_DEVICE_OFFLINE,
    RET_REASON_ERR_INVALID_CHANNEL = _RESPONSECODE_INVALID_CHANNLE,
    RET_REASON_ERR_PROTOCOL = _RESPONSECODE_PROTOCOL_ERROR,
    RET_REASON_ERR_RUN = _RESPONSECODE_TASK_DISPOSE_ERROR,
};

enum
{
    MOBILE_SERVER_COMMAND_NULL = 1,
    MOBILE_SERVER_COMMAND_LOGIN,
    MOBILE_SERVER_COMMAND_GETPARAM_STREAM,
    MOBILE_SERVER_COMMAND_CONTROL,
    MOBILE_SERVER_COMMAND_SWITCH_CHANNEL,
};

enum
{
    CHANNEL_START,
    CHANNEL_STOP,
    CHANNEL_SWITCH,
    CHANNEL_DONE,
    CHANNEL_NOCHANGE,
    CHANNEL_REJECT,
};

typedef struct 
{
    char userName[STR_LEN_32];			
    char password[STR_LEN_16];
    u_int8 flag;//RTSP
} login_param_t;

typedef TLV_V_ControlRequest control_param_t;

typedef struct
{
    int operate;
    int origin_chn;
    int dest_chn;
} channel_switch_t;

typedef struct _mobile_server_cmd_
{
    int cmd;
    union 
    {
        login_param_t login;
        int channel;
        control_param_t control;
        channel_switch_t channel_switch;
    };
    void *p_paramExt;
} mobile_server_cmd_t;

typedef struct
{
    int failReason;
    u_int8 isSuccess;
    u_int8 reserve[3];
    char *p_paramExt;
} cmd_result_t;

typedef struct
{
    int encodeType;
    u_int32 dataSize;
    char *p_Data;
    u_int32 timeStamp;
    u_int32 frameID;
    u_int8 isKeyFrame;
} videoDataParam_t;

typedef struct
{
    int encodeType;
    u_int32 dataSize;
    char *p_Data;
} audioDataParam_t;

typedef int  (* pfun_mobile_server_cmd_cb)(mobile_server_cmd_t* server_cmd, void *result);

int  initMobileServerSdk(u_int16 port, int video_channels, int audio_channels, pfun_mobile_server_cmd_cb p_callback);
void sendVideoToMobileServer(int chn, videoDataParam_t *p_VideoData);
void sendAudioToMobileServer(int chn, audioDataParam_t *p_AudioData);
void showAllLinks(void);

#ifdef __cplusplus
}
#endif

#endif

