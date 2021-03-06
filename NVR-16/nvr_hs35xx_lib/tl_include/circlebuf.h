#ifndef _CIRCLEBUF_H_
#define _CIRCLEBUF_H_

typedef struct bufinfo
{
	s32 nBufId;//每个buf分配一个id
	u8* nBufAddr;
	u32 nWrOffset;
	u32 nRdOffset;
	u32 nWrOffsetPast;
	u32 nRdOffsetPast;
	u32 nLength;
	u32 nLenUsed;
	sem_t sem;
}SCircleBufInfo;

enum EMERRORCODE					//鍑芥暟杩斿洖閿欒鐮佹灇涓�
{
	EM_REC_SUCCESS = 0,				//鎴愬姛
	EM_REC_INVALID_PARA = 1000,		//鏃犳晥鍙傛暟
	EM_REC_SEMINIT_FAILED, 			//sem_init error
	EM_REC_MALLOC_ERR,				//malloc澶辫触
	EM_REC_BUF_NO_SPACE,			//鍐檅uf鏃舵暟鎹お闀匡紝buf鍓╀綑绌洪棿涓嶈冻
	EM_REC_BUF_NO_DATA,				//璇籦uf鏃讹紝瑕佽鍙栫殑闀垮害澶т簬buf鍐呭凡瀛樻暟鎹殑鎬婚暱
	EM_REC_RECORDCREATEBUF,			//鍒涘缓褰曞儚缂撳啿澶辫触
	EM_REC_RECORDDESTROYBUF,		//閿�姣佸綍鍍忕紦鍐插け璐�
	EM_REC_FRAMETOOBIG,				//甯ф暟鎹お闀匡紝缂撳啿鍖烘斁涓嶄笅浜�
	EM_REC_NODISK,					//娌℃湁纭洏锛屼笉杩涜鍒濆鍖�
	EM_REC_HASNOTINIT,				//杩樻病鏈夋墽琛岃繃鍒濆鍖�
	EM_REC_HASBEENINIT,				//宸茬粡鎵ц杩囧垵濮嬪寲
	EM_REC_HASBEENDEINIT,			//宸茬粡鎵ц杩囧弽鍒濆鍖�
	EM_REC_CANNOTOPENNEWFILE,		//涓嶈兘鎵撳紑/鍒涘缓涓�涓柊鏂囦欢鐢ㄤ簬褰曞儚
	EM_REC_ERRGETRECPATH,			//鑾峰緱褰曞儚鏂囦欢璺緞閿欒
	EM_REC_ERROPENNEWFILE,			//鎵撳紑鏂囦欢澶辫触
};


s32 CreateCircleBuf(SCircleBufInfo* info);
s32 DestroyCircleBuf(SCircleBufInfo* info);
s32 ResetCircleBuf(SCircleBufInfo* info);
//s32 GetBufWriteOffset(SCircleBufInfo* info);
//s32 GetBufReadOffset(SCircleBufInfo* info);
s32 WriteDataToBuf(SCircleBufInfo* info, u8* data_addr, u32 length);
s32 ReadDataFromBuf(SCircleBufInfo* info, u8* data_addr, u32 length);
s32 ResumeCircleBufToPast(SCircleBufInfo* info, u8 nWr, u8 nRd);
s32 SkipCircleBuf(SCircleBufInfo* info, u8 nWr, u8 nRd, u32 nSkipLen);

#endif //_CIRCLEBUF_H_

