
#ifndef _TOOL_CUSTOM_H_
#define _TOOL_CUSTOM_H_

//create by lbzhu, 2011-11-23
#include "common_basetypes.h"

//for nVideoResolution 取值
#define VIDENC_RESOL_QCI 0 //PAL:176x144, NTSC:176x112
#define VIDENC_RESOL_CIF 1 //PAL:352x288, NTSC:352x240
#define VIDENC_RESOL_HALF_D1 2  //PAL:704*288, NTSC:704x240	
#define VIDENC_RESOL_4CIF 3 //PAL:704x576, NTSC:704x480
#define VIDENC_RESOL_D1 4 //PAL:704x576, NTSC:704x480

//for nEncodeType 取值
//视频编码类型
#define VIDENC_FORMAT_MJPEG 26 //Motion JPEG 暂不支持
#define VIDENC_FORMAT_H261 31 //H.261 暂不支持
#define VIDENC_FORMAT_MP2 33 //MPEG2 video 暂不支持
#define VIDENC_FORMAT_H263 34 //H.263 暂不支持
#define VIDENC_FORMAT_MP4 97 //MPEG-4 暂不支持
#define VIDENC_FORMAT_H264 98 //H.264 

//for nCbrOrVbr 取值
#define VIDENC_VBR 0 //变码率
#define VIDENC_CBR 1 //定码率

//for nPicLevel 取值
#define VIDENC_PIC_LEVEL_BEST 0
#define VIDENC_PIC_LEVEL_BETTER 1
#define VIDENC_PIC_LEVEL_GOOD 2
#define VIDENC_PIC_LEVEL_NORMAL 3
#define VIDENC_PIC_LEVEL_BAD 4
#define VIDENC_PIC_LEVEL_WORSE 5
//#define VIDENC_PIC_LEVEL_WORST 6

//for nChnStart/nChnEnd 取值
#define SET_ALLCHN 255 //表示针对所有通道
//!= SET_ALLCHN 表示针对具体某个通道

//================================================
typedef struct
{
	u8 nChnStart; //起始目标通道//参考for nChnStart/nChnEnd
	u8 nChnEnd; //结束目标通道//参考for nChnStart/nChnEnd

	u8 nFrameRate; //编码帧率
	u8 nReserve0;
	u32 nBitRate; //编码位率 以KB为单位 即（字节数为nBitRate << 10）
	u8 nVideoResolution;	//参考 for nVideoResolution 取值
	u8 nEncodeType;  //参考 for nEncodeType 取值 当前仅支持VIDENC_FORMAT_H264
	u8 nCbrOrVbr;  //参考 for nCbrOrVbr 取值
	u8 nPicLevel; //参考for nPICLEVEL 取值
	u32 nGop; //关键帧间隔//暂不支持
	u32 nMinQP; //最小QP值//暂不支持
	u32 nMaxQP;	 //最大QP值//暂不支持
	u8 nReserve[8];
} SVidEncParam;

typedef struct
{
	u8 nChnStart; //起始目标通道//参考for nChnStart/nChnEnd
	u8 nChnEnd; //结束目标通道//参考for nChnStart/nChnEnd

	u8 nHue; //各通道色度(数字：0-255)
	u8 nSaturation; //饱和度(数字：0-255)
	u8 nContrast; //对比度(数字：0-255)
	u8 nBrightness; //亮度(数字：0-255)

	u8 nReserve[10];
} SVidPreviewColor;

//for nVideoStandard
#define VIDEO_STANDARD_PAL 12
#define VIDEO_STANDARD_NTSC 10

#define LANGUAGE_SET_NUM_MAX 30

//for nLanguageId 取值
#define LANGUAGE_ID_ENG 0 //英语
#define LANGUAGE_ID_SCH 1 //简中
#define LANGUAGE_ID_TCH 2 //繁中
#define LANGUAGE_ID_HAN 3 //韩语
#define LANGUAGE_ID_JAP 4 //日语
#define LANGUAGE_ID_GER 5 //德语
#define LANGUAGE_ID_RUS 6 //俄语
#define LANGUAGE_ID_FRE 7 //法语
#define LANGUAGE_ID_POR 8 //葡萄牙语
#define LANGUAGE_ID_TUR 9 //土耳其语
#define LANGUAGE_ID_SPA 10 //西班牙语
#define LANGUAGE_ID_ITA 11 //意大利语
#define LANGUAGE_ID_POL 12 //波兰
#define LANGUAGE_ID_POS 13 //波斯
#define LANGUAGE_ID_THAI 14//泰语

//csp modify 20121224
#define LANGUAGE_ID_MAGYAR 15//匈牙利语
#define LANGUAGE_ID_SLOVAKIA 16//斯洛伐克语
#define LANGUAGE_ID_VIETNAM 17//越南语
#define LANGUAGE_ID_GREECE 18//希腊语
//lcy add
#define LANGUAGE_ID_HEBREW 19//希伯来语

//csp modify 20121224
//#define LANGUAGE_ID_NUM 15 //语言支持数
//#define LANGUAGE_ID_NUM 16 //语言支持数
//#define LANGUAGE_ID_NUM 17 //语言支持数
//#define LANGUAGE_ID_NUM 18 //语言支持数
//#define LANGUAGE_ID_NUM 19 //语言支持数
#define LANGUAGE_ID_NUM 20 //语言支持数

typedef struct
{
	u8 nLanguageIdNum; //实际语言数
	u8 nLanguageIdDefault; //参考for nLanguageId 取值
	u8 nLanguageIdList[LANGUAGE_SET_NUM_MAX]; //参考for nLanguageId 取值
	u8 reserve[32];
} SLanguageParam;

//=========下面的类型分配需要好好管理，值要唯一===

#define EMCUSTOMTYPE int

//EMCUSTOMTYPE Begin=================
#define EM_CUSTOM_BASE 0
#define EM_CUSTOM_LOGO_START 1 //开机logo 定制类型
#define EM_CUSTOM_LOGO_APP 2  //应用系统logo 定制类型
#define EM_CUSTOM_PANEL_REMOTE_CTRL 3 //面板遥控器定制

#define EM_CUSTOM_DEVICE_MODEL 10 //设备型号
#define EM_CUSTOM_DEVICE_VIDEOSTAND 11 //设备视频源制式
#define EM_CUSTOM_DEVICE_LANGUAGE 12 //语言及默认值等省定
#define EM_CUSTOM_DEVICE_GETFONT 13 //设备端有效

//语言翻译定制(50~99)
#define EM_CUSTOM_TRANSLATE_BASE 50
#define EM_CUSTOM_TRANSLATE_ENG (EM_CUSTOM_TRANSLATE_BASE + LANGUAGE_ID_ENG) //英语
#define EM_CUSTOM_TRANSLATE_SCH (EM_CUSTOM_TRANSLATE_BASE + LANGUAGE_ID_SCH) //简中
#define EM_CUSTOM_TRANSLATE_TCH (EM_CUSTOM_TRANSLATE_BASE + LANGUAGE_ID_TCH) //繁中
#define EM_CUSTOM_TRANSLATE_HAN (EM_CUSTOM_TRANSLATE_BASE + LANGUAGE_ID_HAN) //韩语
#define EM_CUSTOM_TRANSLATE_JAP (EM_CUSTOM_TRANSLATE_BASE + LANGUAGE_ID_JAP) //日语
#define EM_CUSTOM_TRANSLATE_GER (EM_CUSTOM_TRANSLATE_BASE + LANGUAGE_ID_GER) //德语
#define EM_CUSTOM_TRANSLATE_RUS (EM_CUSTOM_TRANSLATE_BASE + LANGUAGE_ID_RUS) //俄语
#define EM_CUSTOM_TRANSLATE_FRE (EM_CUSTOM_TRANSLATE_BASE + LANGUAGE_ID_FRE) //法语
#define EM_CUSTOM_TRANSLATE_POR (EM_CUSTOM_TRANSLATE_BASE + LANGUAGE_ID_POR) //葡萄牙语
#define EM_CUSTOM_TRANSLATE_TUR (EM_CUSTOM_TRANSLATE_BASE + LANGUAGE_ID_TUR) //土耳其语
#define EM_CUSTOM_TRANSLATE_SPA (EM_CUSTOM_TRANSLATE_BASE + LANGUAGE_ID_SPA) //西班牙语
#define EM_CUSTOM_TRANSLATE_ITA (EM_CUSTOM_TRANSLATE_BASE + LANGUAGE_ID_ITA) //意大利语
#define EM_CUSTOM_TRANSLATE_POL (EM_CUSTOM_TRANSLATE_BASE + LANGUAGE_ID_POL) //波兰
#define EM_CUSTOM_TRANSLATE_POS (EM_CUSTOM_TRANSLATE_BASE + LANGUAGE_ID_POS) //波斯
#define EM_CUSTOM_TRANSLATE_THAI (EM_CUSTOM_TRANSLATE_BASE + LANGUAGE_ID_THAI) //泰语

//csp modify 20121224
#define EM_CUSTOM_TRANSLATE_MAGYAR		(EM_CUSTOM_TRANSLATE_BASE + LANGUAGE_ID_MAGYAR) //匈牙利语
#define EM_CUSTOM_TRANSLATE_SLOVAKIA	(EM_CUSTOM_TRANSLATE_BASE + LANGUAGE_ID_SLOVAKIA) //斯洛伐克语
#define EM_CUSTOM_TRANSLATE_VIETNAM		(EM_CUSTOM_TRANSLATE_BASE + LANGUAGE_ID_VIETNAM) //越南语
#define EM_CUSTOM_TRANSLATE_GREECE		(EM_CUSTOM_TRANSLATE_BASE + LANGUAGE_ID_GREECE) //希腊语
//lcy add
#define EM_CUSTOM_TRANSLATE_HEBREW		(EM_CUSTOM_TRANSLATE_BASE + LANGUAGE_ID_HEBREW) //希伯来语

#define EM_CUSTOM_TRANSLATE_TOP 99

#define EM_CUSTOM_DEVICE_IP 100  //设备默认IP
#define EM_CUSTOM_DEVICE_MAC 101 //设备默认MAC
#define EM_CUSTOM_DEVICE_GATEWAY 102  //设备默认网关

//for 通道定制
#define EM_CUSTOM_VIDENC_MAIN_BASE 1000  //主码流编码参数默认值
#define EM_CUSTOM_VIDENC_MAIN_TOP 1099  //最大主码流定制号
	
#define EM_CUSTOM_VIDENC_SUB_BASE 1100  //子码流编码参数默认值
#define EM_CUSTOM_VIDENC_SUB_TOP 1199  //最大子码流定制号
	
#define EM_CUSTOM_PREVIEW_COLOR_BASE 1200  //预览图像参数默认值
#define EM_CUSTOM_PREVIEW_COLOR_TOP 1299 //最大预览图像参数定制号

#define EM_CUSTOM_ALL 0xffffffff  //表示所有定制类型(当需要对所有定制处理时有效)
//EMCUSTOMTYPE End=================

typedef union //定制参数
{
	char strFilePath[256]; //EM_CUSTOM_LOGO_START, EM_CUSTOM_LOGO_APP, EM_CUSTOM_TRANSLATE_XXX
	u8 strData[32]; //EM_CUSTOM_DEVICE_MODEL, EM_CUSTOM_DEVICE_IP, EM_CUSTOM_DEVICE_MAC, EM_CUSTOM_DEVICE_GATEWAY
	u8 nVideoStandard; //EM_CUSTOM_DEVICE_VIDEOSTAND
	SVidEncParam sVidEncParam;//EM_CUSTOM_VIDENC_MAIN_BASE~EM_CUSTOM_VIDENC_MAIN_TOP, EM_CUSTOM_VIDENC_SUB_BASE ~ EM_CUSTOM_VIDENC_SUB_TOP
	SVidPreviewColor sVidPreviewColor; //EM_CUSTOM_PREVIEW_COLOR_BASE ~ EM_CUSTOM_PREVIEW_COLOR_TOP
	SLanguageParam sLanguageParam; //EM_CUSTOM_DEVICE_LANGUAGE
} SCustomData;

//==========下面两个结构体请保持不变===============
typedef struct
{
	EMCUSTOMTYPE emType; //参考EMCUSTOMTYPE 对应所使用的SCustomData参考SCustomData
	u32 nOffSet;	//相对于本定制分区的偏移量
	u32 nLen; //定制文件长度
} SCustomInfo;
//================================================

//StrFileName为升级文件;设备端则填入空即可
s32 CustomOpen(char* strFileName);
s32 CustomGenerate(EMCUSTOMTYPE emType, SCustomData* psCustomData); //设置但不立即保存
s32 CustomCancel(EMCUSTOMTYPE emType); //取消某项定制
s32 CustomGetCount(void);
s32 CustomGetInfo(s32 nIndex, SCustomInfo* psCustomInfo);
s32 CustomGetLogo(s32 nIndex, u32 nMaxLen, u8* pData, u32 *pDataLen); //获得logo文件数据使用
s32 CustomGetFile(s32 nIndex, char* strFileOut); //导出数据到文件
s32 CustomGetFile2(EMCUSTOMTYPE emType, char* strFileOut); //导出数据到文件
s32 CustomGetData(s32 nIndex, SCustomData* psCustomData); //该接口不支持EM_CUSTOM_LOGO_APP/EM_CUSTOM_LOGO_START/EM_CUSTOM_TRANSLATE_XXX类型数据
s32 CustomGetData2(EMCUSTOMTYPE emType, SCustomData* psCustomData); //该接口不支持EM_CUSTOM_LOGO_APP/EM_CUSTOM_LOGO_START/EM_CUSTOM_TRANSLATE_XXX类型数据

s32 CustomSave(void); //保存设置
s32 CustomClose(u8 nSave); //关闭(nSave决定是否保存改动)
#endif

