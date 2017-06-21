#ifndef _PAGEPTZTYPES_H_
#define _PAGEPTZTYPES_H_

#define TOURPATH_MAX	32
#define TOURPIONT_MAX	128
#define PTZMOVESPEED_MAX	10

//SBizTourPath
typedef struct
{
    uchar nPresetPos; //‘§÷√µ„∫≈
    unsigned short nDwellTime; //∂∫¡Ù ±º‰
    uchar nSpeed; //ÀŸ∂»π≤10µµ
} SGuiTourPoint;

typedef struct
{
    uchar nPathNo;
    SGuiTourPoint sTourPoint[TOURPIONT_MAX];
} SGuiTourPath;

//SBizPtzPara
typedef struct
{
	uchar nIsPresetSet[TOURPIONT_MAX]; //È¢ÑÁΩÆÁÇπ‰ΩçÁΩÆÊúÄÂ§ö128‰∏™//0Ë°®Á§∫‰∏çÂ≠òÂú® Ôºõ1Ë°®Á§∫Â≠òÂú®
	SGuiTourPath sTourPath[TOURPATH_MAX];
} SGuiPtzAdvancedPara;

typedef struct
{
	int nPresetCount[TOURPIONT_MAX];
	int nTourPathCount[TOURPATH_MAX];
}sGuiPtzAdvanPara;

typedef struct
{
	u8 bEnable;
	u8 nAddress;
	u8 nBaudRate;
	u8 nProtocol;
} STabParaUart;

#endif

