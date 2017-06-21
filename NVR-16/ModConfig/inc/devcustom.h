#ifndef _DEVCUSTOM_H_
#define _DEVCUSTOM_H_

#define DVR_CUSTOM_PATH  "/tmp/data/cfg/DvrCustom.ini"

//配置保存
#define DVR_CONFIG_INFO 			"DvrCfg"
#define DVR_CONFIG_INFO_SAVETYPE	"CfgWay"
#define DVR_CONFIG_INFO_FORMAT		"CfgFormat"
#define DVR_CONFIG_INFO_FLASHDEV	"CfgFlashDev"
#define DVR_CONFIG_INFO_FILEPATH	"CfgPath"
#define DVR_CONFIG_INFO_DEFAULTPATH	"CfgPathDefault"
#define DVR_CONFIG_INFO_ZIP			"CfgZip"
#define DVR_CONFIG_INFO_OFFSET		"Offset"
#define DVR_CONFIG_INFO_LENGTH		"Length"
//

//DVR属性
#define DVR_PROPERTY				"DvrProperty"
#define DVR_PROPERTY_VERSION		"Version"
#define DVR_PROPERTY_MODEL			"Model"
#define DVR_PROPERTY_PRODUCTNUMBER	"Productnumber" //9624
#define DVR_PROPERTY_PREVIEWNUM		"PreviewNum"
#define DVR_PROPERTY_VIDEOMAINNUM	"VidMainNum"
#define DVR_PROPERTY_VIDEOSUBNUM	"VidSubNum"
#define DVR_PROPERTY_AUDIONUM		"AudNum"
#define DVR_PROPERTY_SNAPNUM		"SnapNum"
#define DVR_PROPERTY_VOIPNUM		"VoipNum"
#define DVR_PROPERTY_RECNUM			"RecNum"
#define DVR_PROPERTY_SENSORNUM		"SensorNum"
#define DVR_PROPERTY_ALARMOUTNUM	"AlarmoutNum"
#define DVR_PROPERTY_BUZZNUM		"BuzzNum"
#define DVR_PROPERTY_MAXHDDNUM		"MaxHddNum"
#define DVR_PROPERTY_MAINFRAMESIZE	"MaxMainFrameSize"
#define DVR_PROPERTY_SUBFRAMESIZE	"MaxSubFrameSize"
#define DVR_PROPERTY_AUDIOFRAMESIZE	"MaxAudFrameSize"
#define DVR_PROPERTY_SNAPFRAMESIZE	"MaxSnapFrameSize"
//
#define DVR_PROPERTY_OSDOPERATOR	"OsdOperator"
#define DVR_PROPERTY_STROSDNUM		"StrOsdNum"
#define DVR_PROPERTY_IMGOSDNUM		"ImgOsdNum"
#define DVR_PROPERTY_RECTOSDNUM		"RectOsdNum"
#define DVR_PROPERTY_RECOSDRGB		"RectOsdRgb"
#define DVR_PROPERTY_LINEOSDNUM		"LineOsdNum"
#define DVR_PROPERTY_REFWIDTH		"RefWidth"
#define DVR_PROPERTY_REFHEIGHT		"RefHeight"
//
#define DVR_PROPERTY_AUDIO_ENCTYPE		"AudioEncType"
#define DVR_PROPERTY_AUDIO_SAMPLERATE	"SampleRate"
#define DVR_PROPERTY_AUDIO_SAMPLEDURAT	"SampleDuration"
#define DVR_PROPERTY_AUDIO_SAMPLESIZE	"SampleSize"
#define DVR_PROPERTY_AUDIO_BITWIDTH		"BitWidth"
//
#define DVR_PROPERTY_VOIP_ENCTYPE		"VoipEncType"
#define DVR_PROPERTY_VOIP_SAMPLERATE	"VoipSampleRate"
#define DVR_PROPERTY_VOIP_SAMPLEDURAT	"VoipSampleDuration"
#define DVR_PROPERTY_VOIP_SAMPLESIZE	"VoipSampleSize"
#define DVR_PROPERTY_VOIP_BITWIDTH		"VoipBitWidth"
//
#define DVR_PROPERTY_MD_AREA_ROWS		"MDAreaRows"
#define DVR_PROPERTY_MD_AREA_COLS		"MDAreaCols"
#define DVR_PROPERTY_MD_SENSE_MIN		"MDSenseMin"
#define DVR_PROPERTY_MD_SENSE_MAX		"MDSenseMax"
//
#define DVR_PROPERTY_SCH_SEGMENTS		"SchSegments"
#define DVR_PROPERTY_OUTPUT_SYNC		"OutputSync"
#define DVR_PROPERTY_DEVICE_TYPE	 	"NVROrDecoder"	//区别NVR和解码器--- 跃天

//

//取值列表
#define DVR_CFGPATTERN				"CfgPattern"
#define DVR_CFGPATTERN_LANGSUP "LangSup" // 语言翻译字符串标志(&CfgPtn.LangEng &CfgPtn.Sch CfgPtn.LangTch.....)
#define DVR_CFGPATTERN_LANGSRC "LangSrc" // 支持的语言资源文件列表(StringsEng.txt StringsSch.txt StringsTch.txt....)
#define DVR_CFGPATTERN_PREVIEWMODE "PreviewMode" // 模式翻译字符串标志(&CfgPtn.Split1 &CfgPtn.Split4 &CfgPtn.Split6.....)
#define DVR_CFGPATTERN_BITRATE "BitRate" // 位率翻译字符串标志(&CfgPtn.Brt8 &CfgPtn.Brt16 &CfgPtn.Brt32.....)
#define DVR_CFGPATTERN_BITTYPE "BitType" // 位率类型翻译字符串标志(&CfgPtn.Cbr &CfgPtn.Vbr)
#define DVR_CFGPATTERN_PICLEVEL "PicLevel" // 图像质量翻译字符串标志(&CfgPtn.PicBest &CfgPtn.PicBetter &CfgPtn.PicGood &CfgPtn.Normal &CfgPtn.PicBad  &CfgPtn.PicWorse &CfgPtn.Picworst)
#define DVR_CFGPATTERN_VMAINRESOLH_NUM "VMainResolH" // 主码流通用支持的分辨率字符串标志(&CfgPtn.Qcif &CfgPtn.Cif &CfgPtn.HD1 &CfgPtn.4cif &CfgPtn.D1)
#define DVR_CFGPATTERN_VMAINRESOL "VMainResol" // 主码流通用支持的分辨率字符串标志(&CfgPtn.Qcif &CfgPtn.Cif &CfgPtn.HD1 &CfgPtn.4cif &CfgPtn.D1)
#define DVR_CFGPATTERN_VMAINRESOL2 "VMainResol2"// 主码流通用支持的分辨率字符串标志(&CfgPtn.Qcif &CfgPtn.Cif &CfgPtn.HD1 &CfgPtn.4cif &CfgPtn.D1)
#define DVR_CFGPATTERN_VSUBRESOL "VSubResol" // 子码流通用支持的分辨率字符串标志(&CfgPtn.Qcif &CfgPtn.Cif &CfgPtn.HD1 &CfgPtn.4cif &CfgPtn.D1)
#define DVR_CFGPATTERN_VMOBRESOL "VMobResol" // 手机码流通用支持的分辨率字符串标志(&CfgPtn.Cif &CfgPtn.D1 &CfgPtn.720P &CfgPtn.1080P)

#define DVR_CFGPATTERN_FPSPAL "FpsPal" // P制常用下帧率翻译字符串标志(&CfgPtn.Fps1 &CfgPtn.Fps2 &CfgPtn.Fps3.....)
#define DVR_CFGPATTERN_FPSNTSC "FpsNtsc" // N制常用下帧率翻译字符串标志(&CfgPtn.Fps1 &CfgPtn.Fps2 &CfgPtn.Fps3.....)
#define DVR_CFGPATTERN_FPSPALD1 "FpsPalD1" // P制D1下帧率翻译字符串标志(&CfgPtn.Fps1 &CfgPtn.Fps2 &CfgPtn.Fps3.....)
#define DVR_CFGPATTERN_FPSNTSCD1 "FpsNtscD1" // N制D1下帧率翻译字符串标志(&CfgPtn.Fps1 &CfgPtn.Fps2 &CfgPtn.Fps3.....)
#define DVR_CFGPATTERN_FPSPALCIF "FpsPalCif" // P制Cif下帧率翻译字符串标志(&CfgPtn.Fps1 &CfgPtn.Fps2 &CfgPtn.Fps3.....)
#define DVR_CFGPATTERN_FPSNTSCCIF "FpsNtscCif" // N制Cif下帧率翻译字符串标志(&CfgPtn.Fps1 &CfgPtn.Fps2 &CfgPtn.Fps3.....)
#define DVR_CFGPATTERN_STREAMTYPE "StreamType" // 录像码流类型翻译字符串标志(&CfgPtn.Vid &CfgPtn.AV)
#define DVR_CFGPATTERN_VIDEOSTANDARD "VideoStandard" // 视频制式翻译字符串标志(&CfgPtn.PAL &CfgPtn.NTSC)
#define DVR_CFGPATTERN_OUTPUT "Output" // 视频输出翻译字符串标志(&CfgPtn.Cvbs &CfgPtn.Vga800x600 &CfgPtn.Vga1024x768 &CfgPtn.Vga1280x1024)
#define DVR_CFGPATTERN_SWITCHPICTURE "DWellSwitchPicture"  //轮巡画面选择 
/*
CfgPtn.DWellSinglePicture 单画面
CfgPtn.DWellFourPictures 四画面
CfgPtn.DWellNinePictures 九画面
*/

#define DVR_CFGPATTERN_LOCKTIME "LockTime" // 锁定超时间选项翻译字符串标志(&CfgPtn.NeverLock &CfgPtn.30s &CfgPtn.60s &CfgPtn.3x60s &CfgPtn.5x60s)
#define DVR_CFGPATTERN_DATEFORMAT "DateFormat" // 日期格式翻译字符串标志(&CfgPtn.YYYYMMDD &CfgPtn.MMDDYYYY &CfgPtn.DDMMYYYY)
#define DVR_CFGPATTERN_TIMESTANDARD "TimeStandard" // 时间制式翻译字符串标志(&CfgPtn.24h &CfgPtn.12h)
#define DVR_CFGPATTERN_SITEMAINOUT "SiteMainOut" // 现场主输出翻译字符串标志(&CfgPtn.SiteOut1x1 &CfgPtn.SiteOut2x2....)
#define DVR_CFGPATTERN_INTERVAL "Interval" // 时间间隔翻译字符串标志
#define DVR_CFGPATTERN_SENSORTYPE "SensorType" // 传感器类型翻译字符串标志(&CfgPtn.LowLevel/&CfgPtn.NormalClose &CfgPtn.HighLevel/&CfgPtn.NormalOpen)
#define DVR_CFGPATTERN_ALARMOUTTYPE "AlarmoutType" // 报警输出类型翻译字符串标志(&CfgPtn.LowLevel/&CfgPtn.NormalClose &CfgPtn.HighLevel/&CfgPtn.NormalOpen)
#define DVR_CFGPATTERN_DELAY "Delay" // 延时时间选项翻译字符串标志(&CfgPtn.5s &CfgPtn.10s &CfgPtn.20s &CfgPtn.30s &CfgPtn.60s &CfgPtn.120s &CfgPtn.Always) 
#define DVR_CFGPATTERN_PRETIME "PreTime" // 
#define DVR_CFGPATTERN_RECDELAY "RecDelay" // 录像延时时间选项翻译字符串标志(&CfgPtn.10s &CfgPtn.15s &CfgPtn.20s &CfgPtn.30s &CfgPtn.60s &CfgPtn.2x60 &CfgPtn.3x60 &CfgPtn.5x60) 
#define DVR_CFGPATTERN_RECEXPIRE "RecExpire" // 录像过期天数选项翻译字符串标志(&CfgPtn.1day &CfgPtn.2days...&CfgPtn.60days &CfgPtn.Never) 
#define DVR_CFGPATTERN_SCHEDULETYP "ScheduleTyp" // 布防类型选项翻译字符串标志(&CfgPtn.Weekly &CfgPtn.Monthly &CfgPtn.Everyday) 
#define DVR_CFGPATTERN_PTZLINKTYPE "PtzLinkType" // 云台联动类型选项翻译字符串标志(&CfgPtn.NULL &CfgPtn.LinkPreset &CfgPtn.LinkPatrol &CfgPtn.LinkLocus)  
#define DVR_CFGPATTERN_BAUDRATE "BaudRate" // 波特率选项翻译字符串标志(&CfgPtn.Baud110 &CfgPtn.Baud300 &CfgPtn.Baud600 &CfgPtn.Baud1200 &CfgPtn.Baud2400 &CfgPtn.Baud4800 &CfgPtn.Baud9600 &CfgPtn.Baud19200 &CfgPtn.Baud38400 &CfgPtn.Baud57600 &CfgPtn.Baud115200 &CfgPtn.Baud230400 &CfgPtn.Baud460800 &CfgPtn.Baud921600)
#define DVR_CFGPATTERN_DATABIT "DataBit" // 数据位类型选项翻译字符串标志(&CfgPtn.Databit8 &CfgPtn.Databit7 &CfgPtn.Databit6)  
#define DVR_CFGPATTERN_STOPBIT "StopBit" // 停止位类型选项翻译字符串标志(&CfgPtn.StopBit1 &CfgPtn.StopBit2)
#define DVR_CFGPATTERN_CHECKTYPE "CheckType" // 校验类型类型选项翻译字符串标志(&CfgPtn.CheckNull &CfgPtn.CheckOdd &CfgPtn.CheckEven)
#define DVR_CFGPATTERN_FLOWCTRLTYPE "FlowCtrlType" // 流控方式类型选项翻译字符串标志(&CfgPtn.Null &CfgPtn.Hardware &CfgPtn.XonXoff)
#define DVR_CFGPATTERN_DDNSDOMAIN "DDNSDomain" // DDNS域名字符串(popdvr 3322 dyndns)
#define DVR_CFGPATTERN_DDNSUPINTVL "DDNSUpdateTime" // DDNS IP更新间隔字符串标志
#define DVR_CFGPATTERN_VIDEOSRCTYPE "VideoSrcType" // 视频输入源 字符串标志
//
//yaogang modify 20141210 ShenGuang
//取值列表
#define DVR_CFGPATTERN_SNAP_RES "SnapRes"
#define DVR_CFGPATTERN_SNAP_QUALITY "SnapQuality"
#define DVR_CFGPATTERN_SNAP_INTERVAL "SnapInterval"
#define DVR_CFGPATTERN_RepairName "RepairName" //报修系统名称
#define DVR_CFGPATTERN_RepairType "RepairType"//报修系统类型
#define DVR_CFGPATTERN_Maintain "Maintain"//维护维修
#define DVR_CFGPATTERN_Test "Test"//测试
#define DVR_CFGPATTERN_AlarmInput "AlarmInput" //报警图片配置: 输入端子


#endif //_DEVCUSTOM_H_

