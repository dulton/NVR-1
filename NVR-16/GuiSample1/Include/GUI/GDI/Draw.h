

#ifndef __DRAW_H__
#define __DRAW_H__

#include "APIs/DVRDEF.H"

enum BACKGROUND_MODE{
	BM_NORMAL,
	BM_TRANSPARENT,
	BM_CROCHET, //add langzi 2009-12-3 ±³¾°¹³±ßÍ¸Ã÷Ä£Ê½
};

enum COLOR_INDEX{
		VD_COLOR_WINDOW = 0,
		COLOR_POPUP,
		COLOR_CTRL,
		COLOR_CTRLSELECTED,
		VD_COLOR_WINDOWTEXT,
		COLOR_TITLETEXT,
		COLOR_FRAME,
		COLOR_FRAMESELECTED,
		COLOR_CTRLTEXT,
		COLOR_CTRLTEXTSELECTED,
		COLOR_CTRLTEXTDISABLED,
		COLOR_TRANSPARENT,
		COLOR_ALL
};

typedef void ( * PIXELPROC)(uchar *buf, VD_COLORDEV color);
typedef void ( * HLINEPROC)(uchar *buf, int w, VD_COLORDEV color);
typedef void ( * BITMAPPROC)(uchar *buf, int w, uchar *src, int mode, VD_COLORDEV color);

PIXELPROC SelectPixelProc(int rop, int step);
HLINEPROC SelectHLineProc(int rop, int step);
BITMAPPROC SelectBitmapProc(int rop, int step);

void rectBitmap(uchar *pbufdest, uchar *pbufsrc, int width, int height, uint pitch, int line, int bkmode, VD_COLORDEV transColor);
void rectRectSet(uchar *pbufdest, int width, int pitch, int top , int bottom, VD_COLORDEV brushcolor);

VD_COLORREF VD_GetSysColor(int index);
void SetSysColor(int index, VD_COLORREF color);
VD_COLORDEV MakeColor(int bc, int r, int g, int b);
VD_COLORDEV MakeColor(int bc, VD_COLORREF color);
VD_COLORREF ParseColor(int bc, VD_COLORDEV color);
VD_COLORREF ModulateColor(int bc, VD_COLORREF c, int r, int g, int b);

#endif //__DRAW_H__
