//
//  "$Id: Defs.h 105 2008-11-26 11:29:09Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __GUI_DEFS_H__
#define __GUI_DEFS_H__

#define BMP_WIDTH	80
#define BMP_HEIGHT  80
#define TEXT_HEIGHT 24
#define TEXT_WIDTH  24
#define	TEXT_CHNWIDTH	30
#define ROW_HEIGHT  32
#define CTRL_HEIGHT	28
#define CTRL_HEIGHT1 20
#define CLIENT_X	20
#define CLIENT_Y	18		
#define CLIENT_OLD 	10		
#define TEXTBOX_X   4
#define SPLT_WIDTH		32
#define SPLT_HEIGHT		32

#define GUI_TIP_REGION_HEIGHT	(CTRL_HEIGHT * 2 + CLIENT_X * 2 )
#define GUI_TIP_MAINMENU_HEIGHT	(BMP_HEIGHT + TEXT_HEIGHT + 2 * CLIENT_X)

extern SYSTEM_CAPS_EX g_CapsEx;
extern int g_nCapture;
extern int g_nAlarmIn;
extern int g_nAlarmOut;

#endif
