

#ifndef __CLOCALES__
#define __CLOCALES__


#include <map>
#include <list>
#include "Language.h"
#include "System/File.h"
#include "Object.h"
#include "APIs/DVRDEF.H"
#include "config-x.h"

//#define UCS_FONT_MAGIC "RealVision ucs font"
#define UCS_FONT_MAGIC "dahua ucs font"

struct UCS_FONT_HEADER
{
	char  magic[16];	//标志
	uint size;		//字库总大小
	uint blocks;	//块数
};

struct UCS_FONT_BLOCK
{
	ushort start;		//编码起始值
	ushort end;		//编码结束值
	ushort width;		//点阵宽度
	ushort height;	//点阵高度
	uint roffs;	//字体点阵数据偏移
	uint xoffs;	//字体扩展数据偏移
};

enum FONTSIZE
{
	FONTSIZE_SMALL,
	FONTSIZE_NORMAL,
	FONTSIZE_ALL
};

enum FONT_STYLE
{
	FS_NORMAL = 0x0000,		///< 正常字体
	FS_BOLD = 0x0001,		///< 粗体字
	FS_SMALL = 0x0002,
	FS_ITALIC	= 0x0004,	///< 斜体字
	FS_OUTLINE	= 0x0008,	///< 环绕(勾边)效果
	FS_SCALING	= 0x0010,	///< 对不合适的点阵字体进行缩放
};

typedef std::map<VD_PCSTR, VD_PCSTR, strless<VD_PCSTR> > MAPSTRING;
class CLocales:public CObject{
public:
	PATTERN_SINGLETON_DECLARE(CLocales);
private:
	CLocales();
	~CLocales();
public:
	//装载一个字符的字体, 传入参数为UCS-2编码, 返回值为字体宽度
	uchar GetOneFontFromFile(ushort code, uchar *p);
	void SetLanguage(int index);
	ushort GetCharCode(VD_PCSTR pch, int *pn);
	VD_BOOL GetFontSize(VD_SIZE *pSize, FONTSIZE fontsize = FONTSIZE_NORMAL);
	uchar GetCharRaster(ushort code, uchar* p = NULL, FONTSIZE fontsize = FONTSIZE_NORMAL);
	//二分发查找常用汉字
	int FindCommonChar(ushort code);
	int GetTextExtent(VD_PCSTR str, int len=1024, FONT_STYLE fs = FS_NORMAL);

	MAPSTRING m_mapStrings;
private:
	int m_iLanguage;
	UCS_FONT_HEADER m_UFH;
	UCS_FONT_BLOCK *m_pUFB;
	VD_BOOL m_bFontValid;
	CFile m_FileFont;
//#ifdef ENC_SHOW_SMALL_FONT
	VD_BOOL m_bFontValidSmall;
	CFile m_FileFontSmall;
	UCS_FONT_BLOCK *m_pUFBSmall;
	UCS_FONT_HEADER m_UFHSmall;
	int m_nFontBytesSmall;
	VD_SIZE m_sizeFontSmall;
    uchar* m_pASCIIFontSmall;
//#endif

	CFile m_FileStrings;
//	char* m_pbuf;
	VD_SIZE m_sizeFont;
	int m_nFontBytes;
	int m_nCommonChars;			//常用字个数
	ushort* m_pCommonChars;		//常用字数组
	uchar* m_pCommonFont;		//常用字字体
	uchar* m_pASCIIFont;			//ASCII字符字体
	
#if 1//csp modify 20131208
private:
	int ReloadString(char *str, int debug = 0);
	int ReverseString(char *str, int left, int right, int debug = 0);
	int Utf8ToUnicode(char* pch, ushort* strUni, int nMaxLen);
	void ReorderUniStr(ushort* displaystr, int len);
	int IsArabic(ushort code);
	int IsHebrew(ushort code);
#endif
};

VD_PCSTR LOADSTR(VD_PCSTR key);
VD_PCSTR GetParsedString(VD_PCSTR key);
#define g_Locales (*CLocales::instance())

#endif


