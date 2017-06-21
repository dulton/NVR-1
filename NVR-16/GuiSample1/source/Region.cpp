

#include "GUI/GDI/Region.h"

#include <string.h>

#ifndef ABS
#define ABS(a) ((a)>0?(a):(-a))
#endif

///////////////////////////////////////////////////////////////////
////////////        Basic functions
void SetRect(VD_PRECT pRect, int l, int t, int r, int b)
{
	if(pRect)
	{
		pRect->left = l;
		pRect->top = t;
		pRect->right = r;
		pRect->bottom = b;
	}
}

VD_BOOL PtInRect(VD_PCRECT pRect, int x, int y)
{
	return (pRect && x >= pRect->left && x < pRect->right && y >= pRect->top && y < pRect->bottom);
}

VD_BOOL RectOverRect(VD_PCRECT pRect1, VD_PCRECT pRect2)
{
	return (pRect1->left >= pRect2->left && pRect1->top >= pRect2->top
		&& pRect1->right <= pRect2->right && pRect1->bottom <= pRect2->bottom);
}

VD_BOOL RectInRect(VD_PCRECT pRect1, VD_PCRECT pRect2)
{
	return (pRect1->left > pRect2->left && pRect1->top > pRect2->top
		&& pRect1->right < pRect2->right && pRect1->bottom < pRect2->bottom);
}

void SetRectEmpty(VD_PRECT pRect)
{
	if(pRect)
	{
		memset(pRect, 0, sizeof(VD_RECT));
	}
}

VD_BOOL IsRectEmpty(VD_PCRECT pRect)
{
	return (pRect->left == pRect->right || pRect->top == pRect->bottom);
}

VD_BOOL IsRectNull(VD_PCRECT pRect)
{
	return (pRect->left == 0 && pRect->top == 0 && pRect->right == 0 && pRect->bottom == 0);
}

void OffsetRect(VD_PRECT pRect, int x, int y)
{
	pRect->left += x;
	pRect->top += y;
	pRect->right += x;
	pRect->bottom += y;
}

VD_BOOL IntersectRect(VD_PRECT pRect, VD_PCRECT pSrc1, VD_PCRECT pSrc2)
{
	if(pRect)
	{
		pRect->left = MAX(pSrc1->left, pSrc2->left);
		pRect->top  = MAX(pSrc1->top, pSrc2->top);
		pRect->right = MIN(pSrc1->right, pSrc2->right);
		pRect->bottom = MIN(pSrc1->bottom, pSrc2->bottom);

		if(pRect->left >= pRect->right || pRect->top >= pRect->bottom)
		{
			SetRectEmpty(pRect);
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		return !(pSrc1->left >= pSrc2->right || pSrc1->right <= pSrc2->left
			||pSrc1->top >= pSrc2->bottom || pSrc1->bottom <= pSrc2->top);
	}
}

int SubtractRect(VD_PRECT pRect, VD_PCRECT pSrc1, VD_PCRECT pSrc2)
{
    VD_RECT src;
    int n = 0;
    
    src = *pSrc1;

    if (/*::IsRectEmpty(pSrc2) || */!IntersectRect(NULL, &src, pSrc2)) {
        n = 1;
        pRect[0] = src;
    }
    else {
        if(pSrc2->top > src.top)
        {
            pRect[n].left  = src.left;
            pRect[n].top   = src.top;
            pRect[n].right = src.right;
            pRect[n].bottom = pSrc2->top;
            n++;
            src.top = pSrc2->top;
        }
        if(pSrc2->bottom < src.bottom)
        {
            pRect[n].top  = pSrc2->bottom;
            pRect[n].left   = src.left;
            pRect[n].right = src.right;
            pRect[n].bottom = src.bottom;
            n++;
            src.bottom = pSrc2->bottom;
        }
        if(pSrc2->left > src.left)
        {
            pRect[n].left  = src.left;
            pRect[n].top   = src.top;
            pRect[n].right = pSrc2->left;
            pRect[n].bottom = src.bottom;
            n++;
        }
        if(pSrc2->right < src.right)
        {
            pRect[n].left  = pSrc2->right;
            pRect[n].top   = src.top;
            pRect[n].right = src.right;
            pRect[n].bottom = src.bottom;
            n++;
        }
    }

    return n;
}

void NormalizeRect(VD_PRECT pRect)
{
    int temp;

    if(pRect->left > pRect->right)
    {
         temp = pRect->left;
         pRect->left = pRect->right;
         pRect->right = temp;
    }

    if(pRect->top > pRect->bottom)
    {
         temp = pRect->top;
         pRect->top = pRect->bottom;
         pRect->bottom = temp;
    }
}

/* Line clipper */
/*
  This is a line-clipper using the algorithm by cohen-sutherland.

  It is modified to do pixel-perfect clipping. This means that it
  will generate the same endpoints that would be drawn if an ordinary
  bresenham line-drawer where used and only visible pixels drawn.

  It can be used with a bresenham-like linedrawer if it is modified to
  start with a correct error-term.
*/

#define OC_LEFT 1
#define OC_RIGHT 2
#define OC_TOP 4
#define OC_BOTTOM 8

/* Outcodes:
+-> x
|       |      | 
V  0101 | 0100 | 0110
y ---------------------
   0001 | 0000 | 0010
  ---------------------
   1001 | 1000 | 1010
        |      | 
 */
#define outcode(code, xx, yy) \
{\
  code = 0;\
 if (xx < cliprc->left)\
    code |= OC_LEFT;\
  else if (xx >= cliprc->right)\
    code |= OC_RIGHT;\
  if (yy < cliprc->top)\
    code |= OC_TOP;\
  else if (yy >= cliprc->bottom)\
    code |= OC_BOTTOM;\
}

/*
  Calculates |_ a/b _| with mathematically correct floor
  */
static int FloorDiv(int a, int b)
{
    int floor;
    if (b>0) {
        if (a>0) {
            return a /b;
        } else {
            floor = -((-a)/b);
            if ((-a)%b != 0)
                floor--;
        }
        return floor;
    } else {
        if (a>0) {
            floor = -(a/(-b));
            if (a%(-b) != 0)
                floor--;
            return floor;
        } else {
            return (-a)/(-b);
        }
    }
}
/*
  Calculates |^ a/b ^| with mathamatically correct floor
  */
static int CeilDiv(int a,int b)
{
    if (b>0)
        return FloorDiv(a-1,b)+1;
    else
        return FloorDiv(-a-1,-b)+1;
}

VD_BOOL LineClipper (const VD_RECT* cliprc, int *_x0, int *_y0, int *_x1, int *_y1)
{
    int first, last, code;
    int x0, y0, x1, y1;
    int x, y;
    int dx, dy;
    int xmajor;
    int slope;
    
    if (*_x0 == *_x1 && *_y0 == *_y1) { /* a pixel*/
        return PtInRect (cliprc, *_x0, *_y0);
    }
    else if (*_x0 == *_x1) { /* a vertical line */
        int *sy, *ey;

        if (*_y1 > *_y0) {
            sy = _y0;
            ey = _y1;
        }
        else {
            sy = _y1;
            ey = _y0;
        }

        if ( (*_x0 >= cliprc->right) || (*sy >= cliprc->bottom) || 
                    (*_x0 < cliprc->left) || (*ey < cliprc->top) )
            return FALSE;

        if ( (*_x0 >= cliprc->left) && (*sy >= cliprc->top) && 
                    (*_x0 < cliprc->right) && (*ey < cliprc->bottom) )
            return TRUE;
                    
        if (*sy < cliprc->top)
            *sy = cliprc->top;
        if (*ey >= cliprc->bottom)
            *ey = cliprc->bottom - 1;

        if (*ey < *sy)
            return FALSE;

        return TRUE;
    }
    else if (*_y0 == *_y1) { /* a horizontal line */
        int *sx, *ex;

        if (*_x1 > *_x0) {
            sx = _x0;
            ex = _x1;
        }
        else {
            sx = _x1;
            ex = _x0;
        }

        if ( (*sx >= cliprc->right) || (*_y0 >= cliprc->bottom) || 
                    (*ex < cliprc->left) || (*_y0 < cliprc->top) )
            return FALSE;

        if ( (*sx >= cliprc->left) && (*_y0 >= cliprc->top) && 
                    (*ex < cliprc->right) && (*_y0 < cliprc->bottom) )
            return TRUE;
                    
        if (*sx < cliprc->left)
            *sx = cliprc->left;
        if (*ex >= cliprc->right)
            *ex = cliprc->right - 1;

        if (*ex < *sx)
            return FALSE;

        return TRUE;
    }

    first = 0;
    last = 0;
    outcode (first, *_x0, *_y0);
    outcode (last, *_x1, *_y1);

    if ((first | last) == 0) {
        return TRUE; /* Trivially accepted! */
    }

    if ((first & last) != 0) {
        return FALSE; /* Trivially rejected! */
    }

    x0 = *_x0; y0 = *_y0;
    x1 = *_x1; y1 = *_y1;

    dx = x1 - x0;
    dy = y1 - y0;
  
    xmajor = (ABS (dx) > ABS (dy));
    slope = ((dx>=0) && (dy>=0)) || ((dx<0) && (dy<0));
  
    while (TRUE) {
        code = first;
        if (first == 0)
            code = last;

        if (code & OC_LEFT) {
            x = cliprc->left;
            if (xmajor) {
                y = *_y0 + FloorDiv (dy * (x - *_x0) * 2 + dx, 2 * dx);
            } else {
                if (slope) {
                    y = *_y0 + CeilDiv (dy * ((x - *_x0) * 2 - 1), 2 * dx);
                } else {
                    y = *_y0 + FloorDiv (dy * ((x - *_x0) * 2 - 1), 2 * dx);
                }
            }
        } else if (code & OC_RIGHT) {
            x = cliprc->right - 1;
            if (xmajor) {
                y = *_y0 +  FloorDiv (dy * (x - *_x0) * 2 + dx, 2 * dx);
            } else {
                if (slope) {
                    y = *_y0 + CeilDiv (dy * ((x - *_x0) * 2 + 1), 2 * dx) - 1;
                } else {
                    y = *_y0 + FloorDiv (dy * ((x - *_x0) * 2 + 1), 2 * dx) + 1;
                }
            }
        } else if (code & OC_TOP) {
            y = cliprc->top;
            if (xmajor) {
                if (slope) {
                    x = *_x0 + CeilDiv (dx * ((y - *_y0) * 2 - 1), 2 * dy);
                } else {
                    x = *_x0 + FloorDiv (dx * ((y - *_y0) * 2 - 1), 2 * dy);
                }
            } else {
                x = *_x0 +  FloorDiv (dx * (y - *_y0) * 2 + dy, 2 * dy);
            }
        } else { /* OC_BOTTOM */
            y = cliprc->bottom - 1;
            if (xmajor) {
                if (slope) {
                    x = *_x0 + CeilDiv (dx * ((y - *_y0) * 2 + 1), 2 * dy) - 1;
                } else {
                    x = *_x0 + FloorDiv (dx * ((y - *_y0) * 2 + 1), 2 * dy) + 1;
                }
            } else {
                x = *_x0 +  FloorDiv (dx * (y - *_y0) * 2 + dy, 2 * dy);
            }
        }

        if (first) {
            x0 = x;
            y0 = y;
            outcode (first, x0, y0);
        } else {
            x1 = x;
            y1 = y;
            last = code;
            outcode (last, x1, y1);
        }
    
        if ((first & last) != 0) {
            return FALSE; /* Trivially rejected! */
        }

        if ((first | last) == 0) {
            *_x0 = x0; *_y0 = y0;
            *_x1 = x1; *_y1 = y1;
            return TRUE; /* Trivially accepted! */
        }
    }
}

///////////////////////////////////////////////////////////////////
////////////        CSize
CSize::CSize()
{
}

CSize::CSize(int vw, int vh)
{
	w = vw;
	h = vh;
}

CSize::CSize(VD_PCSIZE pSize)
{
	if(pSize)
	{
		w = pSize->w;
		h = pSize->h;
	}
}

CSize::~CSize()
{
}

CSize::operator VD_PSIZE()
{
	return this;
}

///////////////////////////////////////////////////////////////////
////////////        CPoint
CPoint::CPoint()
{
}

CPoint::CPoint(int vx, int vy)
{
	x = vx;
	y = vy;
}

CPoint::CPoint(VD_PCPOINT pPoint)
{
	if(pPoint)
	{
		x = pPoint->x;
		y = pPoint->y;
	}
}

CPoint::~CPoint()
{
}

CPoint::operator VD_PPOINT()
{
	return this;
}

///////////////////////////////////////////////////////////////////
////////////        CRect
CRect::CRect()
{
}

CRect::CRect(int l, int t, int r, int b)
{
	SetRect(l, t, r, b);
}

CRect::CRect(VD_POINT point, VD_SIZE size)
{
	left = point.x;
	top = point.y;
	right = point.x + size.w;
	bottom = point.y + size.h;
}

CRect::CRect(VD_POINT point1, VD_POINT point2)
{
/*-----------------------------------------------------------------------
	参数检查
-----------------------------------------------------------------------*/
	point1.x = (point1.x > 0) ? point1.x : 0;
	point1.y = (point1.y > 0) ? point1.y : 0;
	point2.x = (point2.x > 0) ? point2.x : 0;
	point2.y = (point2.y > 0) ? point2.y : 0;

	if (point1.x <= point2.x)
	{
		left = point1.x;
		right = point2.x;
	}
	else
	{
		left = point2.x;
		right = point1.x;
	}

	if (point1.y <= point2.y)
	{
		top = point1.y;
		bottom = point2.y;
	}
	else
	{
		top = point2.y;
		bottom = point1.y;
	}
}

CRect::CRect(VD_PCRECT pRect)
{
	if(pRect)
	{
		left = pRect->left;
		top = pRect->top;
		right = pRect->right;
		bottom = pRect->bottom;
	}
}

CRect::~CRect()
{
}

CRect::operator VD_PRECT()
{
	return this;
}

void CRect::operator = (const VD_RECT rect)
{
	left = rect.left;
	top = rect.top;
	right = rect.right;
	bottom = rect.bottom;
}

VD_BOOL CRect::operator == (const VD_RECT& rect) const
{
	return (left == rect.left && top == rect.top && right == rect.right && bottom == rect.bottom);
}

VD_BOOL CRect::operator != (const VD_RECT& rect) const
{
	return (!(left == rect.left && top == rect.top && right == rect.right && bottom == rect.bottom));
}

void CRect::SetRect(int l, int t, int r, int b)
{
	::SetRect(this, l, t, r, b);
}

void CRect::SetRectEmpty()
{
	::SetRectEmpty(this);
}

int CRect::Width()
{
	return right-left;
}

int CRect::Height()
{
	return bottom-top;
}

VD_BOOL CRect::PtInRect(int x, int y)
{
	return ::PtInRect(this ,x ,y);
}

void CRect::OffsetRect(int x, int y)
{
	::OffsetRect(this, x, y);
}

VD_BOOL CRect::IntersectRect(VD_PCRECT pSrc1, VD_PCRECT pSrc2)
{
	return ::IntersectRect(this, pSrc1, pSrc2);
}

VD_BOOL CRect::IsRectEmpty()
{
	return ::IsRectEmpty(this);
}

void CRect::NormalizeRect()
{
	::NormalizeRect(this);
}

///////////////////////////////////////////////////////////////////
////////////        CRgn
CRgn::CRgn()
{
	m_Bound.SetRectEmpty();
}

CRgn::~CRgn()
{

}

void CRgn::SetRectRgn(VD_PCRECT pRect)
{
	if(pRect)
	{
		CreateRectRgn(pRect);
	}
	else
	{
		SetRgnEmpty();
	}
}

VD_BOOL CRgn::CreateRoundRectRgn(int x1, int y1, int x2, int y2, int x3, int y3)
{
	return FALSE;
}

void CRgn::CheckBound()
{
	PRECTLIST::iterator  pi, temp;

	if(m_List.empty())
	{
		SetRectEmpty (&m_Bound);
		return;
	}

	m_Bound = m_List.front();
	for (pi = m_List.begin(); pi != m_List.end(); pi++)
	{
        if(m_Bound.left > (*pi).left)
            m_Bound.left = (*pi).left;
   
        if(m_Bound.top > (*pi).top)
            m_Bound.top = (*pi).top;

        if(m_Bound.right < (*pi).right)
            m_Bound.right = (*pi).right;

        if(m_Bound.bottom < (*pi).bottom)
            m_Bound.bottom = (*pi).bottom;
    }
}

VD_BOOL CRgn::PtInRegion(int x, int y)
{
	if (!::PtInRect(m_Bound, x, y))
	{
		return FALSE;
	}
    PRECTLIST::iterator pi;
	for (pi = m_List.begin(); pi != m_List.end(); pi++)
	{
		if(::PtInRect(&(*pi), x, y))
		{
			return TRUE;
		}
    }
	return FALSE;
}

VD_BOOL CRgn::RectInRegion(VD_PCRECT pRect)
{
	if (!::IntersectRect(NULL, m_Bound, pRect))
	{
		return FALSE;
	}
    PRECTLIST::iterator pi;
	for (pi = m_List.begin(); pi != m_List.end(); pi++)
	{
		if(::IntersectRect(NULL, &(*pi), pRect))
		{
			return TRUE;
		}
    }
	return FALSE;
}

VD_BOOL CRgn::SubtractRect(VD_PCRECT pRect)
{
    PRECTLIST::iterator pi, temp;
    VD_RECT chips[4];
    int n;
    int i;
	CRect rect(pRect);

    rect.NormalizeRect();

    if (!::IntersectRect(NULL, m_Bound, rect))
	{
        return FALSE;
    }

	//整个区域都被减掉了
	if(::RectOverRect(m_Bound, rect))
	{
		SetRgnEmpty();
		return TRUE;
	}

	for (pi = m_List.begin(); pi != m_List.end();)
	{
        if (!::IntersectRect(NULL, &(*pi), rect))
		{ 
			pi++;
            continue;
        }

        n = ::SubtractRect(chips, &(*pi), rect);

		if(n == 0)
		{
			temp = pi;
			temp++;
			m_List.erase(pi);
			pi = temp;
			continue;
		}

		(*pi) = chips[0];
        for(i = 1; i<n; i++)
        {
			pi = m_List.insert(pi, chips[i]);
			pi++;
        }
		pi++;
    }

	//如果减去的矩形完全落在边界内, 则无须重新计算边界
	if(!::RectInRect(rect, m_Bound))
	{
		CheckBound ();
	}

    return TRUE;
}

VD_BOOL CRgn::IntersectRect(VD_PCRECT pRect)
{
    PRECTLIST::iterator pi, temp;
    CRect rect(pRect);
 
    NormalizeRect (&rect);

    if (!::IntersectRect(NULL, m_Bound, rect))
	{
		SetRgnEmpty();
        return TRUE;
    }

 	//如果相交的矩形完全覆盖边界, 则无需计算
	if(RectOverRect(m_Bound, rect))
	{
		return TRUE;
	}

   // intersect
	for (pi = m_List.begin(); pi != m_List.end();)
	{
       if(::IntersectRect (&(*pi), &(*pi), &rect))
	   {
			pi++;
	   }
	   else
	   {
			temp = pi;
			temp++;
			m_List.erase(pi);
			pi = temp;
	   }
    }

	CheckBound ();

    return TRUE;
}

VD_BOOL CRgn::UnionRect(VD_PCRECT pRect)
{
	VD_RECT rect;
	
	rect = *pRect;
	
	if (IsRectEmpty (&rect))
	{
		return FALSE;
	}
	
	NormalizeRect (&rect);
	
	m_List.push_back(rect);
	
	if(m_List.size() == 1)
	{
		m_Bound = rect;
	}
	else
	{
		m_Bound.left = MIN(m_Bound.left, rect.left);
		m_Bound.top = MIN(m_Bound.top, rect.top);
		m_Bound.right = MAX(m_Bound.right, rect.right);
		m_Bound.bottom = MAX(m_Bound.bottom, rect.bottom);
	}

	return TRUE;
}

VD_PCRECT CRgn::GetFirstRect()
{
	m_pCurrent = m_List.begin();
	if(m_pCurrent != m_List.end())
	{
		return &(*m_pCurrent);
	}
	return NULL;
}

VD_PCRECT CRgn::GetNextRect()
{
	m_pCurrent++;
	if(m_pCurrent != m_List.end())
	{
		return &(*m_pCurrent);
	}
	return NULL;
}

void CRgn::GetBoundRect(VD_PRECT pRect) const
{
	if(pRect)
	{
		*pRect = m_Bound;
	}
}

void CRgn::OffsetRgn(int x, int y)
{
    PRECTLIST::iterator pi;
	for (pi = m_List.begin(); pi != m_List.end(); pi++)
	{
		::OffsetRect(&(*pi), x, y);
    }

	::OffsetRect(&m_Bound, x, y);
}

VD_BOOL CRgn::CopyRgn(CRgn * pRgn)
{
	//拷贝矩形表
	m_List = pRgn->m_List;
	pRgn->GetBoundRect(&m_Bound);

    return TRUE;
}

VD_BOOL CRgn::SubtractRgn(CRgn * pRgn)
{
    PRECTLIST::iterator pi;
//	trace("SubtractRgn %d * %d\n", m_List.size(), pRgn->m_List.size());

    if (!::IntersectRect(NULL, m_Bound, pRgn->m_Bound))
	{
        return FALSE;
    }

	for (pi = pRgn->m_List.begin(); pi != pRgn->m_List.end(); pi++)
	{
		SubtractRect(&(*pi));
   }

	return FALSE;
}

VD_BOOL CRgn::IntersectRgn(CRgn * pRgn)
{
	VD_RECT rect;
	PRECTLIST tmp;
    PRECTLIST::iterator pi1, pi2;
//	trace("IntersectRgn %d * %d\n", m_List.size(), pRgn->m_List.size());
 
    if (!::IntersectRect(NULL, m_Bound, pRgn->m_Bound))
	{
		SetRgnEmpty();
        return TRUE;
    }

	for (pi1 = m_List.begin(); pi1 != m_List.end(); pi1++)
	{
		for (pi2 = pRgn->m_List.begin(); pi2 != pRgn->m_List.end(); pi2++)
		{
           if(::IntersectRect (&rect, &(*pi1), &(*pi2)))
			{
				tmp.push_back(rect);
			}
        }
    }
	m_List.swap(tmp);
	
	CheckBound();

    return TRUE;
}

VD_BOOL CRgn::UnionRgn(CRgn * pRgn)
{
//	SubtractRgn(pRgn); //先减去相交的区域

	m_List.insert(m_List.end(),pRgn->m_List.begin(), pRgn->m_List.end());
 
	//如果加入的区域落在边界内, 则无须重新计算边界
 	if(!::RectOverRect(pRgn->m_Bound, m_Bound))
	{
		CheckBound ();
	}

	return FALSE;
}

VD_BOOL CRgn::CreateRectRgn(VD_PCRECT pRect)
{
	if(::IsRectEmpty(pRect))
	{
		return FALSE;
	}

	m_List.clear();
	m_List.push_back(*pRect);

	m_Bound = *pRect;

    return TRUE;
}

void CRgn::Test()
{
	int i;
	CRect rect;
	CRgn rgn;
	SYSTEM_TIME systime;
	
	printf("CRgn::Test() random rectangles:\n");
	SystemGetCurrentTime(&systime);
	srand(systime.second);

	for(i=0; i<256; i++)
	{
		rect.SetRect(rand(), rand(), rand(), rand());
		UnionRect(rect);
	}

	printf("left rectangles count: %u\n", (uint)m_List.size());
	rgn.CopyRgn(this);
	IntersectRgn(&rgn);
	printf("left rectangles count: %u\n", (uint)m_List.size());
	SubtractRgn(&rgn);
	
	printf("left rectangles count: %u\n", (uint)m_List.size());
}

void CRgn::Dump()
{
    PRECTLIST::iterator pi;
	printf("CRgn::Dump() %d rectangles \n", (uint)m_List.size());
	for (pi = m_List.begin(); pi != m_List.end(); pi++)
	{
		printf("%4d, %4d, %4d, %4d\n", (*pi).left, (*pi).top, (*pi).right, (*pi).bottom);
    }
}

void CRgn::SetRgnEmpty()
{
	m_List.clear();
	m_Bound.SetRectEmpty();
}

VD_BOOL CRgn::IsRgnEmpty()
{
	return m_List.empty();
}

void CRgn::SwapRgn(CRgn * pRgn)
{
	VD_RECT temp = m_Bound;
	m_List.swap(pRgn->m_List);

	//交换边界, 避免计算
	m_Bound = pRgn->m_Bound;
	pRgn->m_Bound = temp;
}
