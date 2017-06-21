
#ifndef __REGION_H__
#define __REGION_H__

#include "APIs/System.h"
#include "System/pool_allocator.h"

#if defined(WIN32)
	#pragma warning (disable : 4786)
#endif
#include <list>

#define RECT_OFFSET(rect,relativeTo) CRect(relativeTo.left+rect.left, relativeTo.top+rect.top,relativeTo.left+rect.right,relativeTo.top+rect.bottom)


class CPoint : public VD_POINT
{
public:
	CPoint();
	CPoint(int vx, int vy);
	CPoint(VD_PCPOINT pPoint);
	~CPoint();
	operator VD_PPOINT();
};

class CSize : public VD_SIZE
{
public:
	CSize();
	CSize(int vw, int vh);
	CSize(VD_PCSIZE pSize);
	~CSize();
	operator VD_PSIZE();
};

class CRect : public VD_RECT
{
public:
	CRect();
	CRect(int l, int t, int r, int b);
	CRect(VD_POINT point, VD_SIZE size);
	CRect(VD_POINT point1, VD_POINT point2);
	CRect(VD_PCRECT pRect);
	~CRect();
	void operator = (const VD_RECT rect);
	VD_BOOL operator == (const VD_RECT& rect) const;
	VD_BOOL operator != (const VD_RECT& rect) const;
	operator VD_PRECT();
	int Width();
	int Height();
	VD_BOOL PtInRect(int x, int y);
	VD_BOOL IntersectRect(VD_PCRECT pSrc1, VD_PCRECT pSrc2);
	VD_BOOL IsRectEmpty();
	void NormalizeRect();
	void OffsetRect(int x, int y);
	void SetRect(int l, int t, int r, int b);
	void SetRect(VD_PCRECT pRect);
	void SetRectEmpty();
};

typedef std::list<VD_RECT, pool_allocator<VD_RECT> > PRECTLIST;

class CRgn
{
public:
	CRgn();
	VD_BOOL CreateRectRgn(VD_PCRECT pRect);
	VD_BOOL CreateEllipticRgn(VD_PCRECT pRect);
	VD_BOOL CreatePolygonRgn(VD_PPOINT pPoints, int nCount, int nMode);
	VD_BOOL CreateRoundRectRgn(int x1, int y1, int x2, int y2, int x3, int y3);
	virtual ~CRgn();

	void Test();
	void Dump();
	void SetRectRgn(VD_PCRECT pRect);
	void SetRgnEmpty();
	VD_BOOL IsRgnEmpty();

public:
	//
	VD_BOOL PtInRegion(int x, int y);
	VD_BOOL RectInRegion(VD_PCRECT pRect);
	//rect operations
	VD_BOOL SubtractRect(VD_PCRECT pRect);
	VD_BOOL IntersectRect(VD_PCRECT pRect);
	VD_BOOL UnionRect(VD_PCRECT pRect);

	//rect 
	VD_PCRECT GetFirstRect();
	VD_PCRECT GetNextRect();
	void GetBoundRect(VD_PRECT pRect) const;

	//region combine 
	VD_BOOL CopyRgn(CRgn * pRgn);
	VD_BOOL SubtractRgn(CRgn * pRgn);
	VD_BOOL IntersectRgn(CRgn * pRgn);
	VD_BOOL UnionRgn(CRgn * pRgn);
	void OffsetRgn(int x, int y);
	void SwapRgn(CRgn * pRgn);

private:
	void CheckBound();
	
private:
	CRect	m_Bound;
	PRECTLIST m_List;
	PRECTLIST::iterator m_pCurrent;
};

void SetRect(VD_PRECT pRect, int l, int t, int r, int b);
VD_BOOL PtInRect(VD_PCRECT pRect, int x, int y);
VD_BOOL RectOverRect(VD_PCRECT pRect1, VD_PCRECT pRect2);
VD_BOOL RectInRect(VD_PCRECT pRect1, VD_PCRECT pRect2);
void SetRectEmpty(VD_PRECT pRect);
VD_BOOL IsRectEmpty(VD_PCRECT pRect);
VD_BOOL IsRectNull(VD_PCRECT pRect);
void OffsetRect(VD_PRECT pRect, int x, int y);
VD_BOOL IntersectRect(VD_PRECT pRect, VD_PCRECT pSrc1, VD_PCRECT pSrc2);
int SubtractRect(VD_PRECT pRect, VD_PCRECT pSrc1, VD_PCRECT pSrc2);
void NormalizeRect(VD_PRECT pRect);
VD_BOOL LineClipper (const VD_RECT* cliprc, int *_x0, int *_y0, int *_x1, int *_y1);

#endif //__REGION_H__
