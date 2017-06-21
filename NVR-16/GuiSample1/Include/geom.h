#ifndef _GEOM_H_
#define _GEOM_H_

#include "GUI/GDI/Region.h"

//页面边框的宽度。
#define BORDER_WIDTH                       8

//标准按钮的宽度。
#define BUTTON_WIDTH                       100

//页面标题栏的高度。
#define TITLE_BAR_HEIGHT                   50

//控件间的标准间距。
#define CTRL_MARGIN                        5

//页面的用户区域的宽度。
#define CLIENT_WIDTH                       (m_Rect.Width()-BORDER_WIDTH*2)

//页面的用户区域的高度。
#define CLIENT_HEIGHT                      (m_Rect.Height()-BORDER_WIDTH-TITLE_BAR_HEIGHT)

//使用坐标、宽度和高度初始化CRect结构。
#define RECT(left,top,width,height)        CRect(left,top,left+width,top+height)

//以页面的用户区域（标题栏和边框内的区域）为原点初始化CRect结构。
#define CLIENT_RECT(left,top,width,height) RECT(left+BORDER_WIDTH,top+TITLE_BAR_HEIGHT,width,height)

//根据相对于relativeTo的Y轴偏移初始化CRect结构。
#define Y_RELATIVE_RECT(rect,relativeTo)   RECT(relativeTo.left,relativeTo.bottom+rect.top,rect.Width(),rect.Height())

//根据相对于relativeTo的X轴偏移初始化CRect结构。
#define X_RELATIVE_RECT(rect,relativeTo)   RECT(relativeTo.right+rect.left,relativeTo.top,rect.Width(),rect.Height())

//根据相对于relativeTo的X轴和Y轴偏移初始化CRect结构。
#define RELATIVE_RECT(rect,relativeTo)     X_RELATIVE_RECT(Y_RELATIVE_RECT(rect,relativeTo),relativeTo)

#endif // !_GEOM_H_

