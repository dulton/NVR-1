/*add by liu*/
#include <stdio.h>
#include <string.h>
#include "GUI/Pages/PageStream.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageInfoFrameWork.h"
#include "../../Common/inc/tlsdk/Ipcamera.h"
/*
const char* szStreamTitle[5] =
{
	"&CfgPtn.Channel",
	"&CfgPtn.MainResolution",
	"&CfgPtn.BandWidth",
	"&CfgPtn.SubResolution",
	"&CfgPtn.BandWidth",
};
*/
//yaogang modify 20140918
std::string szStreamTitle[5];

const char* szChannel[2*MAX_STREAM_INFO] = {
	"1","2","3","4","5","6","7","8",
	"9","10","11","12","13","14","15","16",
};

//yaogang modify 20150916
const char* szTotal[4] = 
{
	"&CfgPtn.TotalStream",	//当前使用带宽
	//解码板用
	"&CfgPtn.TotalCapacity",	//总接入带宽
	//NVR用
	"&CfgPtn.TotalCapacity2",	//音视频总带宽
	"&CfgPtn.MainCapacity",	//主码流存储带宽
};
//解码板用
const char* szTotalCapacity = "144Mbps";
//NVR用
const char* szTotalCapacity2 = "216Mbps";
const char* szMainCapacity = "160Mbps";



/*
const char* szTotalCapacity[2] =
{
	"144Mbps", //16*8Mbps
	"57600",
};
*/


								
CPageStream::CPageStream( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ )
:CPageFrame(pRect, psz, icon, pParent)
{
	SetMargin(0,0,0,0);
	m_page = 0;
	m_maxPage = 2;
	int i, j;
	CRect rt;
	
	//yaogang modify 20150916
	nNVROrDecoder = 0; 
	//跃天: 1 nvr，2 轮巡解码器，3 切换解码器
	
	SBizDvrInfo DvrInfo;
	if (BizGetDvrInfo(&DvrInfo) == 0)
	{
		nNVROrDecoder = DvrInfo.nNVROrDecoder;
	}
	else
	{
		printf("********************%s -- nNVROrDecoder invalid\n", __func__);
	}
	
////yaogang modify 20140918
	szStreamTitle[0] = GetParsedString("&CfgPtn.Channel");//通道
	szStreamTitle[1] = GetParsedString("&CfgPtn.MainStream");//主码流分辨率
	szStreamTitle[1] += GetParsedString("&CfgPtn.Resolution");
	szStreamTitle[2] = GetParsedString("&CfgPtn.MainStream");//主码流带宽
	szStreamTitle[2] += GetParsedString("&CfgPtn.BandWidth");
	szStreamTitle[3] = GetParsedString("&CfgPtn.SubStream");//子码流分辨率
	szStreamTitle[3] += GetParsedString("&CfgPtn.Resolution");
	szStreamTitle[4] = GetParsedString("&CfgPtn.SubStream");//子码流带宽
	szStreamTitle[4] += GetParsedString("&CfgPtn.BandWidth");
//

	/*蓝色外框*/
	pFrame = CreateTableBox(CRect(17, 60, m_Rect.Width()-17, m_Rect.Height()-17), this, 1, 1);
	pFrame->SetFrameColor(VD_RGB(56,108,148));
	/*内框*/
	//标题框
	//通道|  主码流分辨率|  带宽(Kb/s)  |   辅码流分辨率|   带宽(Kb/s)
	CRect tableRT1(22, 68, m_Rect.Width()-22, m_Rect.Height()-310);

	//yaogang modify 20140918
	pTab1 = CreateTableBox(tableRT1, this, 5, 1);//5列1行
	pTab1->SetColWidth(0, TEXT_WIDTH*3);
	pTab1->SetColWidth(1, TEXT_WIDTH*6);//主码流分辨率
	pTab1->SetColWidth(2, TEXT_WIDTH*5);//主码流带宽
	pTab1->SetColWidth(3, TEXT_WIDTH*6);//子码流分辨率
	//pTab1->SetColWidth(4, TEXT_WIDTH*4);//子码流带宽

	
	for (i=0; i<5; i++)
	{
		pTab1->GetTableRect(i, 0, &rt);
		//yaogang modify 20140918
		pStream1[i] = CreateStatic(CRect(rt.left+tableRT1.left+3, rt.top+tableRT1.top, rt.right+tableRT1.left, rt.bottom+tableRT1.top), this, szStreamTitle[i].c_str());
		pStream1[i]->SetBkColor(VD_RGB(67,77,87));
		pStream1[i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT));
		
		pStream1[i]->SetTextAlign(VD_TA_XCENTER|VD_TA_YCENTER);
	}
	//pTab1->SetColWidth(0, 20);
	
	//各通道框
	CRect tableRT2;
	tableRT2.left = 22;
	tableRT2.top = tableRT1.bottom;
	tableRT2.right = m_Rect.Width()-22-25;
	tableRT2.bottom = m_Rect.Height()-80;
	//yaogang modify 20140918
	pTab2 = CreateTableBox(tableRT2, this, 5, MAX_STREAM_INFO);//5列8行
	pTab2->SetColWidth(0, TEXT_WIDTH*3);
	pTab2->SetColWidth(1, TEXT_WIDTH*6);//主码流分辨率
	pTab2->SetColWidth(2, TEXT_WIDTH*5);//主码流带宽
	pTab2->SetColWidth(3, TEXT_WIDTH*6);//子码流分辨率
	//pTab1->SetColWidth(4, TEXT_WIDTH*4);//子码流带宽

	/*
	for (i=0; i<2; i++)
	{
		pTab2->SetColWidth(i, tableRT1.Width()/3);
	}
	*/
	for (j = 0; j < 5; j++)//列
	{
		for (i=0; i<MAX_STREAM_INFO; i++)//行
		{
			pTab2->GetTableRect(j, i, &rt);
			pStream2[i+j*MAX_STREAM_INFO] = CreateStatic(CRect(rt.left+tableRT2.left+3, rt.top+tableRT2.top, rt.right+tableRT2.left, rt.bottom+tableRT2.top), this, "");
			pStream2[i+j*MAX_STREAM_INFO]->SetTextAlign(VD_TA_XCENTER|VD_TA_YCENTER);
		}
	}
	for (i=0; i<MAX_STREAM_INFO; i++)
	{
		pStream2[i]->SetText(szChannel[i]);
	}
	/*
	for (i=0; i<MAX_STREAM_INFO; i++)
	{
		CRect rt;
		pTab2->GetTableRect(0, i, &rt);
		pStream2[i] = CreateStatic(CRect(rt.left+tableRT2.left+3, rt.top+tableRT2.top, rt.right+tableRT2.left, rt.bottom+tableRT2.top), this, szChannel[i]);
		pStream2[i]->SetTextAlign(VD_TA_XCENTER|VD_TA_YCENTER);
	}
	for (i=0; i<MAX_STREAM_INFO; i++)
	{
		CRect rt;
		pTab2->GetTableRect(1, i, &rt);
		pStream2[i+MAX_STREAM_INFO] = CreateStatic(CRect(rt.left+tableRT2.left+3, rt.top+tableRT2.top, rt.right+tableRT2.left, rt.bottom+tableRT2.top), this, "");
		pStream2[i+MAX_STREAM_INFO]->SetTextAlign(VD_TA_XCENTER|VD_TA_YCENTER);
	}
	for (i=0; i<MAX_STREAM_INFO; i++)
	{
		CRect rt;
		pTab2->GetTableRect(2, i, &rt);
		pStream2[i+2*MAX_STREAM_INFO] = CreateStatic(CRect(rt.left+tableRT2.left+3, rt.top+tableRT2.top, rt.right+tableRT2.left, rt.bottom+tableRT2.top), this, "");
		pStream2[i+2*MAX_STREAM_INFO]->SetTextAlign(VD_TA_XCENTER|VD_TA_YCENTER);
	}
	*/
	//yaogang modify 20150916
	//统计框
	if (nNVROrDecoder != 1) //== 2解码板
	{
		CRect tableRT3;
		tableRT3.left = 22;
		tableRT3.top = tableRT2.bottom;
		tableRT3.right = m_Rect.Width()-22;
		tableRT3.bottom = m_Rect.Height()-25;
		//yaogang modify 20140918
		pTab3 = CreateTableBox(tableRT3, this, 2, 2);//2列2行

		for (i=0; i<2; i++)
		{
			//CRect rt;
			pTab3->GetTableRect(0, i, &rt);
			pStream3[i] = CreateStatic(CRect(rt.left+tableRT3.left+3, rt.top+tableRT3.top, rt.right+tableRT3.left, rt.bottom+tableRT3.top), this, szTotal[i]);
			pStream3[i]->SetTextAlign(VD_TA_XCENTER|VD_TA_YCENTER);
		}
		for (i=0; i<2; i++)
		{
			//CRect rt;
			pTab3->GetTableRect(1, i, &rt);
			pStream3[i+2] = CreateStatic(CRect(rt.left+tableRT3.left+3, rt.top+tableRT3.top, rt.right+tableRT3.left, rt.bottom+tableRT3.top), this, "");
			pStream3[i+2]->SetTextAlign(VD_TA_XCENTER|VD_TA_YCENTER);
			
		}
		pStream3[3]->SetText(szTotalCapacity);
	}
	else //NVR
	{
		CRect tableRT3, tableRT4;
		tableRT3.left = 22;
		tableRT3.top = tableRT2.bottom;
		tableRT3.right = m_Rect.Width()-22;
		tableRT3.bottom = m_Rect.Height()-50;
		
		pTab3 = CreateTableBox(tableRT3, this, 2, 1);//2列1行当前使用带宽

		pTab3->GetTableRect(0, 0, &rt);
		pStream3[0] = CreateStatic(CRect(rt.left+tableRT3.left+3, rt.top+tableRT3.top, rt.right+tableRT3.left, rt.bottom+tableRT3.top), this, szTotal[0]);
		pStream3[0]->SetTextAlign(VD_TA_XCENTER|VD_TA_YCENTER);

		pTab3->GetTableRect(1, 0, &rt);
		pStream3[2] = CreateStatic(CRect(rt.left+tableRT3.left+3, rt.top+tableRT3.top, rt.right+tableRT3.left, rt.bottom+tableRT3.top), this, "");
		pStream3[2]->SetTextAlign(VD_TA_XCENTER|VD_TA_YCENTER);
		
		//音视频总带宽----- 主码流存储带宽
		tableRT4.left = 22;
		tableRT4.top = tableRT3.bottom;
		tableRT4.right = m_Rect.Width()-22;
		tableRT4.bottom = m_Rect.Height()-25;
		
		pTab4 = CreateTableBox(tableRT4, this, 4, 1);//2列1行当前使用带宽

		pTab4->GetTableRect(0, 0, &rt);
		pStream4[0] = CreateStatic(CRect(rt.left+tableRT4.left+3, rt.top+tableRT4.top, rt.right+tableRT4.left, rt.bottom+tableRT4.top), this, szTotal[2]);
		pStream4[0]->SetTextAlign(VD_TA_XCENTER|VD_TA_YCENTER);

		pTab4->GetTableRect(1, 0, &rt);
		pStream4[1] = CreateStatic(CRect(rt.left+tableRT4.left+3, rt.top+tableRT4.top, rt.right+tableRT4.left, rt.bottom+tableRT4.top), this, szTotalCapacity2);
		pStream4[1]->SetTextAlign(VD_TA_XCENTER|VD_TA_YCENTER);

		pTab4->GetTableRect(2, 0, &rt);
		pStream4[2] = CreateStatic(CRect(rt.left+tableRT4.left+3, rt.top+tableRT4.top, rt.right+tableRT4.left, rt.bottom+tableRT4.top), this, szTotal[3]);
		pStream4[2]->SetTextAlign(VD_TA_XCENTER|VD_TA_YCENTER);

		pTab4->GetTableRect(3, 0, &rt);
		pStream4[3] = CreateStatic(CRect(rt.left+tableRT4.left+3, rt.top+tableRT4.top, rt.right+tableRT4.left, rt.bottom+tableRT4.top), this, szMainCapacity);
		pStream4[3]->SetTextAlign(VD_TA_XCENTER|VD_TA_YCENTER);
	}
	//yaogang modify 20140918
	/*
	for (i=0; i<2; i++)
	{
		//CRect rt;
		pTab3->GetTableRect(2, i, &rt);
		pStream3[i+4] = CreateStatic(CRect(rt.left+tableRT3.left+3, rt.top+tableRT3.top, rt.right+tableRT3.left, rt.bottom+tableRT3.top), this, "");
		pStream3[i+4]->SetTextAlign(VD_TA_XCENTER|VD_TA_YCENTER);
	}
	*/
	/*滑条*/
	pScroll = CreateScrollBar(CRect(tableRT2.right, //左边界
									tableRT2.top,//顶部
									tableRT2.right+25, //右边界
									tableRT2.bottom), //底边界
									this,
									scrollbarY,
									0,
									(m_maxPage-1)*10, 
									1, 
									(CTRLPROC)&CPageStream::OnTrackMove);


}

/*
这个要加上，不然编译报错
 undefined reference to `vtable for CPageStream'
 collect2: ld returned 1 exit status
*/
CPageStream::~CPageStream()
{

}

//add by liu chn:0-15
int CPageStream::ShowStreamInfo(uint param)
{
	int StreamKbps[2] = {0};//主、子码流
	int chn = 0;
	char Kbpsbuffer[11] = {0};//eg: xxxx kbps
	int totalStreamKbps = 0;
	int video_width[2] = {0};//主、子码流
	int video_height[2]  = {0};//主、子码流
	//printf("***********ShowStreamInfo\n");
	char resulotion[10];//eg: 1920*1080
	int issub;
	int test;
	
	for(chn=0; chn<2*MAX_STREAM_INFO; chn++)
	{
		for (issub = 0; issub< 2; issub++)
		{
			StreamKbps[issub] = Get_StreamKbps(chn + issub*2*MAX_STREAM_INFO);
			if(-1 == StreamKbps[issub])
			{
				printf("ShowStreamInfo:Stream channel  0-31\n");
				return -1;
			}
			//yaogang modify 20140819
			IPC_GetStreamResolution(chn + issub*2*MAX_STREAM_INFO, &video_width[issub], &video_height[issub]);
			test = video_width[issub];
			if (StreamKbps[issub] == 0)
			{
				//printf("yg chn%02d video_width: %d\n", chn + issub*2*MAX_STREAM_INFO, video_width[issub]);
				video_height[issub] = video_width[issub] = 0;
			}
			totalStreamKbps += StreamKbps[issub];

			if (video_width[issub] == 0)
			{
				//printf("yg video_width==0, chn%02d real_width: %d, StreamKbps: %d\n", \
					//chn + issub*2*MAX_STREAM_INFO, test, StreamKbps[issub]);
			}
			
		}
		switch(m_page)
		{
			case 0:
					if(chn<MAX_STREAM_INFO)
					{
						sprintf(resulotion, "%d*%d", video_width[0], video_height[0]);//主码流分辨率
						pStream2[chn + MAX_STREAM_INFO]->SetText(resulotion);
						
						sprintf(Kbpsbuffer,"%d Kbps",StreamKbps[0]);//主码流带宽
						pStream2[chn + 2*MAX_STREAM_INFO]->SetText(Kbpsbuffer);
						
						sprintf(resulotion, "%d*%d", video_width[1], video_height[1]);//子码流分辨率
						pStream2[chn + 3*MAX_STREAM_INFO]->SetText(resulotion);
						
						sprintf(Kbpsbuffer,"%d Kbps",StreamKbps[1]);//子码流带宽
						pStream2[chn + 4*MAX_STREAM_INFO]->SetText(Kbpsbuffer);
						//memset(Kbpsbuffer,0,7);
					}					
					break;
			case 1:
					if(chn>=MAX_STREAM_INFO)
					{	
						sprintf(resulotion, "%d*%d", video_width[0], video_height[0]);//主码流分辨率
						pStream2[chn]->SetText(resulotion);
						
						sprintf(Kbpsbuffer,"%d Kbps",StreamKbps[0]);//主码流带宽
						pStream2[chn + MAX_STREAM_INFO]->SetText(Kbpsbuffer);
						
						sprintf(resulotion, "%d*%d", video_width[1], video_height[1]);//子码流分辨率
						pStream2[chn + 2*MAX_STREAM_INFO]->SetText(resulotion);
						
						sprintf(Kbpsbuffer,"%d Kbps",StreamKbps[1]);//子码流带宽
						pStream2[chn + 3*MAX_STREAM_INFO]->SetText(Kbpsbuffer);
					}
					break;
			default:
					break;


		}
	}
	
	
	sprintf(Kbpsbuffer,"%d Kbps",totalStreamKbps);
	pStream3[2]->SetText(Kbpsbuffer);
	//pStream3[3]->SetText(szTotalCapacity);
	
	return 0;
	
	
}


void CPageStream::OnTrackMove()
{
	int i;
	int pos = pScroll->GetPos();
    if (m_page==pos) {
        return;
    }
    m_page = pos;
	
	if(0 == m_page)//显示通道1-8
	{
		for (i=0; i<MAX_STREAM_INFO; i++)
		{
			pStream2[i]->SetText(szChannel[i]);
		}

	}
	else //显示通道9-16
	{
		for (i=0; i<MAX_STREAM_INFO; i++)
		{
			pStream2[i]->SetText(szChannel[i+MAX_STREAM_INFO]);
		}

	}
	ShowStreamInfo(0);
}


VD_BOOL CPageStream::UpdateData(UDM mode)
{
	
	if(UDM_OPEN == mode)
	{
		m_StreamTimer = new CTimer("Stream_Timer");
		m_StreamTimer->Start(this, (VD_TIMERPROC)&CPageStream::ShowStreamInfo, 1000,1000);

	}
	else if(UDM_CLOSED == mode)
	{
		m_StreamTimer->Stop();
	}
	
	return TRUE;
}




