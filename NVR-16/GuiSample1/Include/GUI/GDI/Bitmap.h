
#ifndef __BITMAP_H__
#define __BITMAP_H__

#include "APIs/DVRDEF.H"

typedef struct {
	int		width;
	int		height;
	int		format;
	uchar*	bits;
	int		reffrence;
} VD_BITMAP;

#if defined(WIN32)
#pragma pack(2)
#endif
struct VD_tagBITMAPFILEHEADER{
	ushort    bfType;
	uint   bfSize;
	ushort    bfReserved1;
	ushort    bfReserved2;
	uint   bfOffBits;
}
//#ifdef LINUX
#ifndef WIN32
__attribute__((packed))
#endif
;

#if defined(WIN32)
#pragma pack()
#endif
typedef struct VD_tagBITMAPFILEHEADER VD_BITMAPFILEHEADER;

typedef struct VD_tagBITMAPINFOHEADER{
	uint      biSize;
	int       biWidth;
	int       biHeight;
	ushort       biPlanes;
	ushort       biBitCount;
	uint      biCompression;
	uint      biSizeImage;
	int       biXPelsPerMeter;
	int       biYPelsPerMeter;
	uint      biClrUsed;
	uint      biClrImportant;
} 
//#ifdef LINUX
#ifndef WIN32
__attribute__((packed))
#endif
VD_BITMAPINFOHEADER
;

typedef struct VD_tagRGBQUAD {
	uchar    rgbBlue; 
	uchar    rgbGreen; 
	uchar    rgbRed; 
	uchar    rgbReserved; 
} VD_RGBQUAD;

typedef struct VD_tagBITMAPINFO { 
	VD_BITMAPINFOHEADER bmiHeader; 
	VD_RGBQUAD          bmiColors[1]; 
} 
//#ifdef LINUX
#ifndef WIN32
__attribute__((packed))
#endif
VD_BITMAPINFO
; 

VD_BITMAP* VD_LoadBitmap(VD_PCSTR name);
void ReleaseBitmap(VD_BITMAP* pBitmap);
void SaveScreenBitmap(VD_PCRECT pcRect, VD_PCSTR filename);
void ReplaceBitmapBits(VD_BITMAP* pBitmap, VD_COLORREF matchColor, VD_COLORREF replaceColor);
void ReplaceBitmapBits(VD_BITMAP* pBitmap, VD_COLORREF matchColor, VD_BITMAP * replaceBitmap, int start_x = 0, int start_y = 0);
VD_COLORREF GetBitmapBkColor(VD_BITMAP* pBitmap, VD_COLORREF defaultColor);


#endif
