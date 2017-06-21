#ifndef _RECORD_H_
#define _RECORD_H_

typedef enum				//业务层发送消息类型枚举
{
	EM_REC_V_MOTION = 0,			//移动侦测
	EM_REC_V_ALARM,					//报警输入
	EM_REC_V_IPCEXTALARM,
	EM_REC_V_BIND,					//视频遮挡
	EM_REC_V_LOST,					//视频丢失
	EM_REC_NETLINK_LOST,			//网络断连
	EM_REC_ABNORMAL,				//异常
	EM_REC_SETRECCHN_MOTION = 10,	//设置移动侦测触发的录像通道
	EM_REC_SETRECCHN_ALARM,			//设置报警输入触发的录像通道
	EM_REC_SETRECCHN_BIND,			//设置视频遮挡触发的录像通道
	EM_REC_SETRECCHN_LOST,			//设置视频丢失触发的录像通道
	EM_REC_SETRECCHN_NETLINK_LOST,	//设置视频丢失触发的录像通道
	EM_REC_SETRECCHN_ABNORMAL,		//设置异常情况触发的录像通道
	EM_REC_TIMER = 30,				//定时
	EM_REC_V_MANUAL = 100,			//手动录像
	EM_REC_PAUSEREC,				//暂停某通道所有类型录像
	EM_REC_RESUMEREC,				//恢复某通道录像
	EM_REC_STOPCHN,					//停止某通道所有类型录像
	//EM_REC_STOPALLCHN,			//停止所有通道所有类型录像
	EM_REC_SETPRETIME,				//设置预录时间
	EM_REC_SETDELAYTIME,			//设置延时时间
	EM_REC_SETSCHPARA,				//设置布防参数	
}EMRECBIZMSGTYPE;

typedef enum				//录像状态枚举
{
	EM_RECSTATE_IDLE = 0,	//空闲
	EM_RECSTATE_RUNNING,	//录像
	EM_RECSTATE_PREREC,		//预录
	EM_RECSTATE_DELAY,		//延时
}EMRECSTATE;

typedef enum				//录像状态转换消息枚举
{
	EM_REC_IDLE_RUN = 200,	//空闲->录像
	EM_REC_IDLE_PRE,		//空闲->预录 
	EM_REC_RUN_IDLE,		//录像->空闲
	EM_REC_RUN_DELAY,		//录像->延时
	EM_REC_DELAY_RUN,		//延时->录像
	EM_REC_DELAY_IDLE,		//延时->空闲 
	EM_REC_PRE_IDLE,		//预录->空闲 
	EM_REC_PRE_RUN,			//预录->录像
	EM_REC_UPDATEFILEINFO,	//更新录像文件信息(录像类型)
	EM_REC_READ_BUF = 220,	//应该读buf写入文件（buf使用率达到一定值）
	EM_REC_FORCE_END,		//暂停/停止录像时强制关闭录像文件
	EM_REC_TYPE_CHANGE_END,//录像文件类型变化时强制关闭录像文件
	EM_REC_RESUME_REC,		//恢复正常录像
	//csp modify
	EM_REC_FLUSH,
}EMRECSTATUSMSGTYPE;

typedef struct msg_header
{
	u8 nChn;				//通道号
	EMRECBIZMSGTYPE nMsg;	//消息类型枚举EMRECBIZMSGTYPE/EMRECSTATUSMSGTYPE
	u8 nKey;				// 1:��ʼ0:ֹͣ
	void* data;				//参数，相对于模块层主要是用于设置各类延时时间，业务层用于设置报警触发的通道，其它时候使用时设为NULL
}SModRecMsgHeader;

typedef struct mr_setrecchn
{
	u8 nChn;
	u64 nMotion;
	u64 nAlarm;
	u64 nBind;
	u64 nLost;
	u64 nNetlink;
	u64 nAbnomal;
}SModRecChnTrigger;

s32 RecordCreateBuf(u8 chn, u32 length);
s32 RecordDestroyBuf(u8 chn);
s32 RecordWriteFrameToBuf(SModRecRecordHeader* header);
//yaogang modify 20170218 in shanghai  add MaxFrameSize
s32 RecordReadFrameFromBuf(u8 chn, u8* frame_data, s32 MaxFrameSize);
u8 RecordGetBufUseRate(u8 chn);
s32 RecordResetBuf(u8 chn);
s32 RecordUpdateBufReadOffset(SModRecRecordHeader* header);	//有预录时，实时更新读指针
s32 RecordSkipOneFrameFromBuf(u8 chn);

s32 RecordCreateMsgQueue(u8 msgid);
s32 RecordDestroyMsgQueue(u8 msgid);
s32 RecordWriteMsgQueue(u8 msgid, SModRecMsgHeader* header);
s32 RecordReadMsgQueue(u8 msgid, SModRecMsgHeader* header);

s32 RecordDealRecMsg(SModRecMsgHeader* sMsg);
s32 RecordUpdateRecStatus(u8 chn);
void RecordStatusManagerFxn();
s32 RecordOpenNewFile(custommp4_t** file, partition_index** p_ptn_index, char* recfilename, u32* offset, u8 chn);
s32 RecordGetFrameSize(EMRECENCRESOL emResolution, u16* width, u16* height);
void RecordMainFxn(void* ID);
void RecordGetDiskFull(u8* pFull);
//yaogang modify for bad disk
//void Record_get_disk_use_info(u8 pdiskuse[16]);
void Record_get_disk_use_info(u8 pdiskuse[16], u32 *prec_disk_err_bitmask);

//yaogang modify 20150313
typedef enum  {
	TypeRecSnapWillStart,	//��Ҫ���������м�����״̬
	TypeRecSnapStart,		//�ڲ����ݳ�ʼ��
	TypeRecSnapRun,		//��������
    	TypeRecSnapStop,		//�����������ʽ��ʱ��Ҫ�ر�
    	TypeRecSnapIdle,		//�������ʽ�������еȴ�����
    	TypeRecSnapRst,		//����������
} SnapFxnMsgType;

typedef struct
{
	u8 cause;		//0: format 1:hotplug
	SnapFxnMsgType type;
}SSnapFxnMsg;//32 byte

s32 RecordSnapCreateMsgQueue(u32 length);
s32 RecordSnapDestroyMsgQueue();
s32 RecordSnapWriteMsgQueue(SSnapFxnMsg *pmsg);
s32 RecordSnapReadMsgQueue(SSnapFxnMsg *pmsg);

#endif //_RECORD_H_

