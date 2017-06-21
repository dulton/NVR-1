
#ifndef __CURSOR_H__
#define __CURSOR_H__

#include "Bitmap.h"
#include "MultiTask/Guard.h"
#include "GUI/GDI/Region.h"
#include "Devices/DevGraphics.h"

enum CURSOR_STYLE{
	CR_NORMAL =	0,		//普通
		CR_BUSY,		//忙
		CR_WAIT,		//等待
		CR_MOVE,		//移动
		CR_SIZE1,		//正斜拖动
		CR_SIZE2,		//反斜拖动
		CR_SIZE3,		//左右拖动
		CR_SIZE4,		//上下拖动
		CR_ZOOMIN,		//放大
		CR_ALL			//全部
};

#if defined(WIN32)
#pragma pack(2)
#endif
struct tagSINGLEMONOCURSOR{
	ushort	cdreserved;
	ushort	cdtype;
	ushort	cdcount;
	uchar	bwidth;
	uchar	bheight;
	uchar	bcolorcount;
	uchar	breserved;
	ushort	wxhotspot;
	ushort	wyhotspot;
	uint	lbytesinres;
	uint	dwimageoffset;
	VD_BITMAPINFOHEADER crheader;
	VD_RGBQUAD crcolors[2];
}
//#ifdef LINUX
#ifndef WIN32
__attribute__((packed))
#endif
;

#if defined(WIN32) 
#pragma pack()
#endif
typedef struct tagSINGLEMONOCURSOR SINGLEMONOCURSOR;

typedef struct tagCURSOR_SHAPE
{
	int		valid;
	uint	color1;
	uint	color2;
	ushort	xhotspot;
	ushort	yhotspot;
	uchar*	andbits;
	uchar*	xorbits;
}CURSOR_SHAPE;

class CCursor
{
	VD_BOOL m_bCursorEnabled;
	CRect old_cursor_rect;
	CRect new_cursor_rect;
	CRect old_cursor_rect_valid;
	CRect new_cursor_rect_valid;
	ushort cursor_x;
	ushort cursor_y;
	int	 cursor_index;
	VD_BOOL m_bCursorShow;
	uchar * m_pSavedBits;
	uchar * m_pCursorBits;
	VD_BOOL cursor_pos_changed;
	uchar * m_BufferCursor;
	CMutex m_Mutex;
	CRect m_Rect;
	static GRAPHICS_SURFACE m_Surface;
	static CURSOR_SHAPE cursor_shape[CR_ALL];
	VD_BOOL bEnable;

public:
	CCursor(int index);
	~CCursor();

	static VD_BOOL VD_LoadCursor(VD_PCSTR name, int index);
	void SetCursor(int index);
	int GetCursor();
	void SetCursorPos(int x, int y, VD_BOOL update = FALSE);
	void GetCursorPos(VD_PPOINT pPoint);
	void ShowCursor(VD_BOOL bShow);
	void EnableCursor(VD_BOOL bFlag);
	//void Enable(VD_BOOL bFlag);
	void UpdateCursor(VD_BOOL bShow, VD_PCRECT pcRect);
	void UpdateCursor(VD_BOOL bShow, int x1, int y1, int x2, int y2);
	static CCursor* instance(int index = 0); 

private:
	void ShowCursor();
	void HideCursor();
	void CursorBitBlt(uchar *screen, uchar *mem, VD_PCRECT pRect, VD_BOOL bSave);
};

#endif

