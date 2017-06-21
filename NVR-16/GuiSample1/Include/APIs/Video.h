
#ifndef __VIDEO_API_H__
#define __VIDEO_API_H__

#include "DVRDEF.H"

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup VideoAPI API Video
/// 视频设置API。主要是视频输入参数，输出参数的设置，还有一些辅助的设置功能。
///	\n 调用流程图:
/// \code
///    =======================================================
///                |                            
///	      *Video2GetOutChannels       
///          VideoGetCaps            
///                |                            
///           VideoCreate                      
///      +---------|--------------+----------------------+     
///      |         |(In)          |(Out)                 |(Others)
///      |   VideoSetColor   VideoSwitch         VideoWriteRegister         
///      |   VideoSetCover VideoSetBkColor        VideoReadRegister
///      |   VideoSetVstd   VideoSetTVMargin        VideoMatrix
///      |         |        VideoSetTVColor      VideoSwitchTVMonitor
///      |         |       VideoSetTVAntiDither VideoSwitchVGAMonitor
///      |         |      VideoSetOutputAdapter          |
///      |         |      VideoSetOutputAdapter          |
///      |         |              |                      |
///      +---------|--------------+----------------------+          
///            VideoDestory   
///                |                            
///    =======================================================
/// \endcode
/// @{

/// 视频制式类型
enum video_standard_t {
	VIDEO_STANDARD_PAL = 0,	///< PAL制式。
	VIDEO_STANDARD_NTSC,	///< NTSC制式。
	VIDEO_STANDARD_SECAM	///< SECAM制式。
};

/// 视频颜色格式
typedef struct VIDEO_COLOR{
	uchar	Brightness;		///< 亮度，取值0-100。
	uchar	Contrast;		///< 对比度，取值0-100。
	uchar 	Saturation;		///< 饱和度，取值0-100。
	uchar 	Hue;			///< 色调，取值0-100。
	uchar 	Gain;			///< 增益，取值0-100。bit7置位表示自动增益，其他位被忽略。
	uchar	WhiteBalance;	///< 自动白电平控制，bit7置位表示开启自动控制.0x0,0x1,0x2分别代表低,中,高等级
	uchar	reserved[2];
}VIDEO_COLOR;

/// 视频区域结构
/// - 当表示整个视频区域的坐标时, 使用图形坐标体系, 也就是是指在图形坐标体系下视频边界
/// 的坐标, 结构成员的取值与图形分辨率, 图形和视频叠加时的偏移和缩放参数, 当前分割模
/// 式等都有关系;
/// - 当表示的是整个视频区域中的一个子区域, 则使用相对坐标, 相对于一个虚拟的分辨率
/// (0, 0, RELATIVE_MAX_X, RELATIVE_MAX_Y), 使用相对坐标可以在不知道实际视频分辨率的
/// 情况下设定视频的子区域, 可以保证一次转化误差在1象素以内. 转换公式如下(假设绝对分
/// 辨率为aw*ah):
/// 绝对坐标(ax, ay)转换成相对坐标(rx, ry):
/// \code
/// rx = ax * RELATIVE_MAX_X / aw;
/// ry = ay * RELATIVE_MAX_Y / ah;
/// \endcode
/// 相对坐标(rx, ry)转换成绝对坐标(ax, ay):
/// \code
/// ax = rx * aw / RELATIVE_MAX_X;
/// ay = ry * ah / RELATIVE_MAX_Y;
/// \endcode
/// 相对分辨率定义如下:
/// \code
/// #define RELATIVE_MAX_X 8192
/// #define RELATIVE_MAX_Y 8192
/// \endcode
///
typedef struct VIDEO_RECT
{
	ushort	left;			///< 左上角x坐标。
	ushort	top;			///< 左上角y坐标。
	ushort	right;			///< 右下角x坐标。
	ushort	bottom;			///< 右下角y坐标。
}VIDEO_RECT;

/// 视频覆盖操作类型
enum video_cover_t {
	VIDEO_COVER_WINDOW_CLEAR = 0,	///< 清除窗口覆盖区域。
	VIDEO_COVER_WINDOW_DRAW,		///< 绘制窗口覆盖区域。
	VIDEO_COVER_FULLSCREEN_DRAW,	///< 绘制全屏覆盖区域。
	VIDEO_COVER_FULLSCREEN_CLEAR	///< 清楚全屏覆盖区域。
};

/// 视频覆盖参数
typedef struct VIDEO_COVER_PARAM
{
	/// 覆盖的矩形区域，采用相对坐标体系。
	VIDEO_RECT	rect;

	/// 被覆盖区域显示的颜色。16进制表示为0x00BBGGRR，最高字节保留。
	uint		color;

	/// 区域操作的类型， 取video_cover_t类型的值。窗口和全屏操作应该是独立的， 
	/// 即窗口内的操作不影响全屏，全屏内的操作也不影响窗口。 清除区域时，其他
	/// 参数被忽略。
	int			enable;
}VIDEO_COVER_PARAM;

/// 视频切换类型
enum video_switch_t{
	VIDEO_SWITCH_MONITOR = 0,	///< 切换到监视输出。
	VIDEO_SWITCH_PLAY			///< 切换到回放输出。
};

/// 视频输出适配器类型
enum video_output_t{
	VIDEO_OUTPUT_AUTO = 0,	///< 自动检测设备。
	VIDEO_OUTPUT_VGA,		///< 切换到VGA输出。
	VIDEO_OUTPUT_TV,		///< 切换到TV输出。
	VIDEO_OUTPUT_VGA_LCD	///< 切换到LCD VGA输出。
};

/// 视屏设备特性结构
typedef struct VIDEO_CAPS
{
	/// 是否支持TV输出颜色设置。置1表示支持该枚举值对应的特性，置0表示不支持。
	uchar TVColor;

	/// 图像属性掩码
	/// \note 掩码位定义参考pic_attr
} VIDEO_CAPS;				


/// 得到视频输出的通道数目。
/// 
/// \param 无
/// \return视频输出通道数目。
int Video2GetOutChannels(void);


/// 创建视频设备
/// 
/// \param 无
/// \retval <0 创建失败
/// \retval 0 创建成功
int VideoCreate(void);
int Video2Create(int index);


/// 销毁视频设备
/// 
/// \param 无
/// \retval <0 销毁失败
/// \retval 0 销毁成功
int VideoDestory(void);
int Video2Destroy(int index);


/// 设置视频的颜色参数。
/// 
/// \param [in] channel 通道号。
/// \param [in] pColor 指向颜色结构VIDEO _ COLOR的指针。
/// \retval 0  设置成功
/// \retval <0  设置失败
int VideoSetColor(int channel, VIDEO_COLOR * pColor);

/// 图像支持的特性列表掩码
/// \note 作为图像功能列表属性mask 1:可设置 0:不可设置
enum pic_attr
{
 PIC_COLOR, ///< 亮度,对比度,色度,增益,饱和度
 PIC_BACKLIGHT,///< 背光补偿
 PIC_EXPOSAL, 	///< 曝光选择
 PIC_AUTOC2BW, ///< 自动彩黑转换
 PIC_MIRROR,		//镜像
 PIC_FLIP,		//翻转
 PIC_MASKNUM,  ///< 属性选项数
};
 
/// 视频控制结构
typedef struct VIDEO_CONTROL{
 uchar  Exposure; ///< 曝光模式 1-6:手动曝光等级; 0:自动曝光
 uchar  Backlight; ///< 背光补偿 1:打开补偿; 0:停止补偿
 uchar  AutoColor2BW; ///< 自动彩黑转换 1:打开转换; 0:停止转换
  uchar  Mirror;		//< 镜像  1 开， 0关

}VIDEO_CONTROL;
 
/// 设置视频的控制参数。
/// 
/// \param [in] channel 通道号。
/// \param [in] pColor 指向视频控制结构VIDEO_CONTROL的指针。
/// \retval 0  设置成功
/// \retval <0  设置失败
int VideoSetControl(int channel, VIDEO_CONTROL * pColor);


/// 设置覆盖区域。
/// 
/// \param [in] channel 通道号。
/// \param [in] index 遮挡区域号
/// \param [in] pParam 指向视频覆盖结构VIDEO_COVER_PARAM的指针。
/// \retval 0  设置成功。
/// \retval <0  设置失败。
int VideoSetCover(int channel, int index, VIDEO_COVER_PARAM *pParam);


/// 设置视频制式。
/// 
/// \param [in] channel 通道号。
/// \param [in] dwStandard 视频制式，取video_standard_t类型的值。
/// \retval 0  设置成功
/// \retval <0  设置失败
int VideoSetVstd(int channel,uint dwStandard);	


/// 写一个字节到视频设备寄存器。
/// 
/// \param [in] Chip 芯片序号。
/// \param [in] Page 页面序号。
/// \param [in] Register 寄存器序号。
/// \param [in] Data 要写入的值。
/// \return 无
void VideoWriteRegister(uchar Chip, uchar Page, uchar Register, uchar Data);


/// 从视频设备寄存器读一个字节。
/// 
/// \param [in] Chip 芯片序号。
/// \param [in] Page 页面序号。
/// \param [in] Register 寄存器序号。
/// \return读到的值。
uchar VideoReadRegister(uchar Chip, uchar Page, uchar Register);


/// 切换视频输出的类型。
/// \param [in] dwType 输出的类型，取下表中的值。
/// \retval 0  切换成功。
/// \retval <0  切换失败。
int VideoSwitch(uint dwType);
int Video2Switch(int index, uint dwType);


/// 设置TV输出的边界。视频显示在某些监视器上时， 4个边上会有损失。这个函数的目的
/// 就是设置4个边界压缩的比例，使视频落在监视器的可视区域内。调节范围0-100是一个
/// 示意值，实际上对应缩小的尺寸需要在底层做转换和限制。
/// 
/// \param [in] left 左边界比率，取值0-100， 默认值为0。
/// \param [in] top 上边界比率，取值0-100， 默认值为0。
/// \param [in] right 右边界比率，取值0-100， 默认值为0。
/// \param [in] bottom 下边界比率，取值0-100， 默认值为0。
/// \retval 0  设置成功。
/// \retval <0  设置失败。
int VideoSetTVMargin(uchar left, uchar top, uchar right, uchar bottom);


/// 设置TV输出的颜色。
/// 
/// \param [in] brightness 亮度，取值0-100， 默认值为50。
/// \param [in] contrast 对比度，取值0-100， 默认值为50。
/// \retval 0  设置成功。
/// \retval <0  设置失败。
int VideoSetTVColor(uchar brightness, uchar contrast);


/// 设置TV输出防抖动参数，部分老硬件会有抖动的问题。
/// 
/// \param [in] level 防抖动参数，抖动程度与取值不是比例关系，而是一个经验值。
///        取值0-100， 默认值为50。
/// \retval 0  设置成功。
/// \retval <0  设置失败。
int VideoSetTVAntiDither(uchar level);


/// 设置视频输出的适配器类型。
/// 
/// \param [in] type 适配器类型，取video_output_t类型的值。
/// \retval 0  设置成功。
/// \retval <0  设置失败。
int VideoSetOutputAdapter(uchar type);


/// 获取视频输出的适配器类型。
/// 
/// \param [out] ptype 适配器类型，取video_output_t类型的值。
/// \retval 0  获取成功。
/// \retval <0  获取失败。
int VideoGetOutputAdapter(uchar *ptype);


/// 连接视频矩阵的输出通道和输入通道。
/// 
/// \param [in]  channelIn 视频输入通道
/// \param [in]  channelOut 视频输出通道
/// \retval 0  获取成功。
/// \retval <0  获取失败。
int VideoMatrix(uchar channelIn,uchar channelOut);


/// 开关TV监视器
/// 
/// \param [in] open TRUE-开TV监视器，FALSE-关TV监视器
/// \retval 0  操作成功。
/// \retval <0  操作失败。
int VideoSwitchTVMonitor(VD_BOOL open);


/// 开关VGA监视器
/// 
/// \param [in] open TRUE-开VGA监视器，FALSE-关VGA监视器
/// \retval 0  操作成功。
/// \retval <0  操作失败。
int VideoSwitchVGAMonitor(VD_BOOL open);


/// 得到视频支持的特性。
/// 
/// \param [out] pCaps 指向视频特性结构VIDEO_CAPS的指针。
/// \retval 0  获取成功。
/// \retval <0  获取失败。
int VideoGetCaps(VIDEO_CAPS * pCaps);
int Video2GetCaps(int index, VIDEO_CAPS * pCaps);


/// 设置没有视频的监视画面的背景颜色.
/// 
/// \param [in] color 要设置的颜色，16进制表示为0x00BBGGRR，最高字节保留。
/// \retval 0  设置成功
/// \retval <0  设置失败
int VideoSetBkColor(uint color);
int Video2SetBkColor(int index, uint color);


/// 设置视频捕获分辨率
int VideoRecordSetSize(int channel, int size);
	/*
说明	开始自动检测

参数	无

返回值	=0  ：	  成功。
	其他值：  失败。

*/
int VideoAutoDetectStart(void);
/*
说明	停止自动检测


参数	无

返回值	=0  ：	  成功。
	其他值：  失败。
	*/
int VideoAutoDetectStop(void);

/// @} end of group


typedef struct tagWHITE_LEVEL_CAPS //Added by wangjun2-20080805
{
	int	 enable;		// 为1表示使能，为0时表示禁用
}WHITE_LEVEL_CAPS;

/*
	说明:获取白电平控制能力值
	返回值=0:成功
	其他值:失败
*/
int WhiteLevelGetCaps(WHITE_LEVEL_CAPS * pCaps);

/*
	set or get video mode
	SystemSetVideoMode() return 0 success
*/
enum video_standard_t SystemGetVideoMode(void);
int SystemSetVideoMode(enum video_standard_t videostand);

#ifdef __cplusplus
}
#endif

#endif //__VIDEO_API_H__

