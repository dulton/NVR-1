

#ifndef __DC_H__
#define __DC_H__

#include "MultiTask/Guard.h"
#include "System/Locales.h"
#include "Devices/DevGraphics.h"
#include "Draw.h"
#include "Region.h"
#include "Bitmap.h"
#include "Cursor.h"

#define TRIGO_CONST1 383	//
#define TRIGO_CONST2 707	//
#define TRIGO_CONST3 924	//

#define ROW_SPACE 4

enum dc_type
{
	DC_SCREEN = 0,			//Ö÷ÆÁÄ»
	DC_SCREEN1,			//¸¨ÖúÆÁÄ»
	DC_MEMORY
};

enum RASTER_OPERATION{
	ROP_COPY = 0,		//copy operation
	ROP_AND,			//and operation
	ROP_OR,				//or operation
	ROP_XOR,			//xor operation
};

enum RECT_STYLE{
	RS_NORMAL = 0x0000, 		//normal
		RS_SUNKEN = 0x0001, 	//sunken
		RS_RAISED = 0x0002, 	//raised
		RS_HOLLOW = 0x0004,		//hollow, no fill
		RS_FLAT	= 0X0008,		//flat, no frame
};

enum VD_TEXT_ALIGN{
		VD_TA_LEFT = 0x0000,
		VD_TA_XCENTER = 0x0001,
		VD_TA_RIGHT = 0x0002,
		VD_TA_TOP = 0x0000,
		VD_TA_YCENTER = 0x0010,
		VD_TA_BOTTOM = 0x0020,
		VD_TA_XLEFT = 0x0003,

		VD_TA_LEFTTOP = 0x0000,
		VD_TA_CENTER = 0x0011,
		VD_TA_CHANGELINE = 0x0100,
};

enum MODULATE_STYLE{
};

enum VD_PEN_STYLE{
	VD_PS_SOLID = 0x0000,		//solid
	VD_PS_DOT,					//dot
};

enum BRUSH_STYLE{
	BS_NORMAL = 0x0000,
};

//enum FONT_STYLE{
//	FS_NORMAL = 0x0000,
//	FS_BOLD = 0x0001,
//	FS_SMALL = 0x0002,
//};

class CDC
{
public:
	CDC(int type = DC_SCREEN, int width = 0, int height = 0);
	virtual ~CDC();

	void Update();

	//color and style
	void SetRop(int rop= ROP_COPY);
	void SetFont(VD_COLORREF color, FONT_STYLE style=FS_NORMAL);
	void SetFontSize(int height, double ratio);
	void SetPen(VD_COLORREF color, int style=VD_PS_SOLID, int width=1);
	void SetBrush(VD_COLORREF color, int style=BS_NORMAL);
	void SetBrush2(VD_COLORREF color, int style=BS_NORMAL);
	
	void SetTextAlign(int align = VD_TA_LEFTTOP);
	void SetBkMode(int mode);
	int GetBkMode();
	void SetBkColor(VD_COLORREF color);
	void SetRgnStyle(int style);
	void SetPenWidth(int width);
	int GetPenWidth();
	//draw funcionts
	inline void SetPixel(int x, int y, VD_COLORREF color);
	VD_COLORREF GetPixel(int x, int y);
	void LineTo(int x, int y);
	void MoveTo(int x, int y);
	void VD_TextOut(VD_PCRECT pRect, VD_PCSTR str, int len=1024);
	/*int GetTextExtent(PCSTR str, int len=1024);*/
	void Rectangle(VD_PCRECT pRect, int ea = 0, int eb = 0);
	void Trapezoid(int x1, int y1, int w1, int x2, int y2, int w2);
	void Bitmap(VD_PCRECT pRect, VD_BITMAP * pBitmap, int start_x = 0, int start_y = 0);
	
	//void FillRgn(CRgn* pRgn);//csp modify
	//void LineInRgn(CRgn* pRgn, int x1, int y1, int x2, int y2);//csp modify
	
	void FillRect(VD_PCRECT pRect, VD_COLORREF color);
	
	//void LineInRect(VD_PCRECT pRect, int x1, int y1, int x2, int y2);//csp modify
	
	//bit operation
	VD_BOOL BitBlt(VD_PCRECT pRect, CDC* pDC, int start_x = 0, int start_y = 0);

	//region clipping operations
	VD_BOOL SubtractRgn(CRgn * pRgn);
	VD_BOOL IntersectRgn(CRgn * pRgn);
	VD_BOOL UnionRgn(CRgn * pRgn);
	VD_BOOL GetRgn(CRgn * pRgn);
	VD_BOOL SetRgn(CRgn * pRgn);
	VD_BOOL SwapRgn(CRgn * pRgn);
	VD_BOOL SubtractRect(VD_PCRECT pRect);
	VD_BOOL IntersectRect(VD_PCRECT pRect);
	VD_BOOL UnionRect(VD_PCRECT pRect);
	void SetBound(VD_PCRECT pRect);
	
	// Coordinate Functions
	void DPtoLP(VD_PPOINT pPoints, int nCount = 1) const;
	void DPtoLP(VD_PRECT pRect) const;
	void LPtoDP(VD_PPOINT pPoints, int nCount = 1) const;
	void LPtoDP(VD_PRECT pRect) const;

	//dump
	void DumpRgn(){m_Rgn.Dump();};
	void WideBitmap(VD_PCRECT pTRect, VD_BITMAP * pBitmap, VD_PCRECT pSRect = NULL);
	void StyledBitmap(VD_PCRECT rect, VD_BITMAP *pBitmap, VD_PCRECT areaEnlarge);

	void Enable(VD_BOOL flag);
	void Lock();
	void UnLock();

	void StringToBitmap(uchar* bitBuffer, int w, int h, VD_PCSTR str, int len);

	void SetLanguage(int index);

private:
	void _FillRgn(CRgn* pRgn);
	void _LineInRgn(CRgn* pRgn, int x1, int y1, int x2, int y2, VD_BOOL bUpdateCursor = FALSE);
	void _TextInRgn(CRgn* pRgn, int x, int y, VD_PCSTR str, int len);
	void _BitmapInRgn(CRgn* pRgn, VD_BITMAP * pBitmap, int line, int offs_x, int offs_y);
	void _FillRect(VD_PCRECT pRect);
	void _LineInRect(VD_PCRECT pRect, int x1, int y1, int x2, int y2);
	void _TextInRect(VD_PCRECT pRect, int x, int y, VD_PCSTR str, int len);
	void _BitmapInRect(VD_PCRECT pRect, VD_BITMAP * pBitmap, int line, int offs_x, int offs_y);
	void _WideBitmapInRect(VD_PCRECT pRect, VD_BITMAP * pBitmap, int line, int offs_x, int offs_y);

private:
	int m_PenWidth;
	int m_PenStyle;
	VD_COLORDEV m_FontColor;
	FONT_STYLE m_FontStyle;
	VD_COLORDEV m_BrushColor;
	int m_BrushStyle;
	VD_COLORDEV m_KeyColor;
	int m_BitmapMode;
	int m_TextAlign;
	int m_RgnStyle;

	int m_CS;
	CMutex m_Mutex;

	uchar* m_pBuffer;
	int m_Pitch;
	int m_BPP;
	int m_Height;
	int m_Width;
	PIXELPROC m_pPixelProc;
	HLINEPROC m_pHLineProc;
	BITMAPPROC m_pBitmapProc;

	VD_BOOL m_bEnable;
	
	int m_nType;
	CRect m_Bound;		//±ß½ç
	CRgn m_Rgn;
	VD_POINT m_Pos;//µ±Ç°×ø±ê
	VD_SIZE m_FontSize;

	VD_COLORDEV m_TransColor;
	int m_ROP;
	VD_COLORDEV m_BkColor;
	int m_BkMode;
	VD_COLORDEV m_PenColor;
	VD_COLORDEV m_PenColorDark;
	VD_COLORDEV m_PenColorLight;

	CLocales* m_locals;
	CCursor* m_pCursor;

	//csp modify
private:
	static VD_BOOL m_bModify;
public:
	static VD_BOOL GetModify();
	static void SetModify(VD_BOOL flag);
};

#endif //__DC_H__
