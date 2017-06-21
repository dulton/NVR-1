

#ifndef __MOUSE_API_H__
#define __MOUSE_API_H__

#include "DVRDEF.H"

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup MouseAPI API Mouse
/// 鼠标和触摸屏设备API。
///	\n 调用流程图:
/// \code
///    ===========================
///                |
///           MouseCreate
///                |--------+
///           MouseGetData  |
///                |--------+
///           MouseDestory
///                |
///    ===========================
/// \endcode
/// @{

#define MOUSE_LBUTTON			0x1			///< 左键状态掩码。
#define MOUSE_RBUTTON			0x2			///< 右键状态掩码。
#define MOUSE_MBUTTON			0x4			///< 中键状态掩码。
#define MOUSE_TOUCHSCREEN		0x80		///< 触摸屏数据。

/// 鼠标输入数据结构
typedef struct MOUSE_DATA
{
	/// 鼠标左右中3个按键的状态，置1表示按键按下，置0表示按键弹起。与宏\b MOUSE_XXX
	/// 相与得到对应按键的状态。最高位对应触摸屏标志，置1表示为触摸屏数据，置0
	/// 表示鼠标数据。 触摸屏按下弹起动作处理为左键。
	unsigned char	key;

#if 1
	/// x坐标偏移，正数向右，负数向左，取值[-32767, 32768]。鼠标使用。
	signed short x;

	/// y坐标偏移，正数向上，负数向下，取值[-32767, 32768]。鼠标使用。
	signed short y;

	/// 滚轮偏移，正数向后，负数向前，取值[-32767, 32768]。鼠标使用。
	signed short z;
#else
	/// x坐标偏移，正数向右，负数向左，取值[-127,128]。鼠标使用。
	signed char		x;

	/// y坐标偏移，正数向上，负数向下，取值[-127,128]。鼠标使用。
	signed char		y;

	/// 滚轮偏移，正数向后，负数向前，取值[-127,128]。鼠标使用。
	signed char		z;
#endif

	/// 绝对x坐标，正方向向右。触摸屏使用。
	unsigned short	ax;

	/// 绝对y坐标，正方向向下。触摸屏使用。
	unsigned short	ay;	
} MOUSE_DATA;


/// 创建鼠标设备
/// 
/// \param 无
/// \retval <0 创建失败
/// \retval 0 创建成功
int MouseCreate(void);


/// 销毁鼠标设备
/// 
/// \param 无
/// \retval <0 销毁失败
/// \retval 0 销毁成功
int MouseDestory(void);


/// 一直等到读到一个完整的鼠标事件才返回，否则阻塞。如果鼠标外设被拔掉或不存在，
/// 立即返回错误。出错时应用程序需要定时重试，周期不小于500ms，以实现热插拔功能。
/// 
/// \param [out] pData 指向鼠标时间结构MOUSE_DATA的指针。
/// \retval 0 读成功
/// \retval <0 读失败
int MouseGetData(MOUSE_DATA * pData);

/// @} end of group

#ifdef __cplusplus
}
#endif

#endif //__MOUSE_API_H__

