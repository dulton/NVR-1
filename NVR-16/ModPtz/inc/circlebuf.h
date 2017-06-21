#ifndef _CIRCLEBUF_H_
#define _CIRCLEBUF_H_

typedef struct bufinfo
{
	s32 nBufId;			//每个buf分配一个id
	u8* nBufAddr;		//buf的首地址
	u32 nWrOffset;		//记录buf的写偏移量
	u32 nRdOffset;		//记录buf的读偏移量
	u32 nWrOffsetPast;
	u32 nRdOffsetPast;
	u32 nLength;		//buf的总长度
	u32 nLenUsed;		//buf的已用长度
	sem_t sem;
}SCircleBufInfo;

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
