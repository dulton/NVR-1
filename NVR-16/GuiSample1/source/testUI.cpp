// testUI.cpp : Defines the entry point for the console application.
//

#include <signal.h>

#include "MultiTask/Semaphore.h"
#include "Devices/DevGraphics.h"

#include <list>
#include <deque>
#include <vector>

#include "Main.h"

#include "GUI/GUI.h"
#include "GUI/Pages/PageStart.h"

#include "biz.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "sg_platform.h"
#include <time.h>
//#include <linux/time.h>

#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
#else
#include "mkp_vd.h"
#endif

#include "hifb.h"
#include "lib_preview.h"

#include "GUI/Pages/BizData.h"
#include "diskmanage.h"
#include "GUI/Pages/PageMessageBox.h"
#include "biz.h"
#include <linux/watchdog.h>

//#include "lib_common.h"
//#include "lib_preview.h"
//#include "lib_misc.h"

#include "hi_comm_vb.h"
#include "hi_comm_sys.h"
#include "mpi_vb.h"
#include "mpi_sys.h"
#include "mpi_vdec.h"
#include "mpi_venc.h"
#include "hi_comm_vdec.h"
#include "hi_comm_vo.h"
#include "hi_comm_vi.h"
#include "hi_comm_venc.h"
#include "mpi_vo.h"

//版本发布时打开此宏,进行temp目录删除操作
//#define RELEASE

#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
extern "C" int tw286x_wr(int chip_id, unsigned char adr, unsigned char data);
extern "C" int tw286x_rd(int chip_id, unsigned int adr, unsigned char *out_val);
#endif

#ifdef WIN32
extern "C"
{
    void InitApiLib();
}
//#elif defined(LINUX)
#else
void SignalFunc(int argc)
{
	
}
#endif

static int GetMaxChnNum()
{
	return GetVideoMainNum();
}

PATTERN_SINGLETON_IMPLEMENT(CChallenger);

CChallenger::CChallenger()
{
	
}

CChallenger::~CChallenger()
{
	
}

typedef enum hiVOU_DEV_E
{
	HD = 0,
	AD = 1,
	SD = 2,
	VOU_DEV_BUTT,
}VOU_DEV_E;

typedef enum hiVOU_GLAYER_E
{
	G0 = 0,
	G1 = 1,
	G2 = 2,
	G3 = 3,
	HC = 4,
	VOU_GLAYER_BUTT,
}VOU_GLAYER_E;

#if 0
BOOL setOsdTransparency(int nfd,u8 trans)
{
	int fd;
	BOOL bShow = TRUE;
	HIFB_ALPHA_S stAlpha;
	
	fd = nfd;
	
	if(fd == -1)
	{
		printf("Failed to open fb window \n");
		return FALSE;
	}
	
	if(trans)
	{
		bShow = FALSE;
		
		if(ioctl(fd, FBIOGET_SHOW_HIFB, &bShow) == -1)
		{
			printf("Error reading fb show information.\n");
			close(fd);
			return FALSE;
		}
		else
		{
			bShow = !bShow;
			//printf("get fb show 1 success\n");
		}
		
		if(bShow)
		{
			if(ioctl(fd, FBIOPUT_SHOW_HIFB, &bShow) == -1)
			{
				printf("Error reading variable information.\n");
				close(fd);
				return FALSE;
			}
			else
			{
				//printf("set fb show 1 success\n");
			}
		}
		
		stAlpha.bAlphaEnable = HI_TRUE;
		stAlpha.bAlphaChannel = HI_FALSE;
		stAlpha.u8Alpha0 = 0x0;
		stAlpha.u8Alpha1 = trans;
		if (ioctl(fd, FBIOPUT_ALPHA_HIFB,  &stAlpha) < 0)
		{
			printf("Set alpha failed!\n");
			close(fd);
			return -1;
		}
	}
	else
	{
		bShow = FALSE;
	}
	
	return TRUE;
}
#endif

#if 0
int initStateOSD()
{
	HIFB_ALPHA_S stAlpha;
	struct fb_var_screeninfo varInfo;
	int size;
	u16* pmap;
	int fd;

#if 1
	VD_BIND_S stBind;
	/*---------1 bind graphic layer G4(here used to show cursor) to HD, defaultly----------*/
	/*1.1 open vd fd*/
	fd = open("/dev/vd", O_RDWR, 0);
	if(fd < 0)
	{
		printf("open vd failed!\n");
		return -1;
	}

	stBind.s32GraphicId = HC;

	stBind.DevId = HD;

	if (ioctl(fd, VD_SET_GRAPHIC_BIND, &stBind) != HI_SUCCESS)
	{
		printf("[fd:%d]set bind glayer %d to dev %d failed!\n",fd,HC,HD);
		close(fd);
		return -1;
	}

	/*1.3 bind G1 to SD*/
	stBind.s32GraphicId = G1;
	stBind.DevId = SD;
	if (ioctl(fd, VD_SET_GRAPHIC_BIND, &stBind) != HI_SUCCESS)
	{
		printf("[fd:%d]set bind glayer %d to dev %d failed!\n",fd,HC,SD);
		close(fd);
		return -1;
	}
	#endif
	
	fd = open("/dev/fb4", O_RDWR);

	setOsdTransparency(fd ,0xff);
	
	//printf();
	if(fd == -1)
	{
		printf("Failed to open fb device\n");
		return FALSE;
	}
	
	stAlpha.bAlphaEnable = HI_TRUE;
	stAlpha.bAlphaChannel = HI_FALSE;
	stAlpha.u8Alpha0 = 0x0;
	stAlpha.u8Alpha1 = 0x90;
	if (ioctl(fd, FBIOPUT_ALPHA_HIFB,  &stAlpha) < 0)
	{
		printf("Set alpha failed!\n");
		close(fd);
		return -1;
	}
		
	if(ioctl(fd, FBIOGET_VSCREENINFO, &varInfo) == -1)
	{
		printf("Failed ioctl FBIOGET_VSCREENINFO on\n");
		close(fd);
		return FALSE;
	}

	varInfo.xres = varInfo.xres_virtual = 800;
	varInfo.yres = varInfo.yres_virtual = 600;
	varInfo.bits_per_pixel = 16;

	if(ioctl(fd, FBIOPUT_VSCREENINFO, &varInfo) == -1)
	{
		printf("Failed ioctl FBIOPUT_VSCREENINFO on \n");
		close(fd);
		return FALSE;
	}

	size = 800 * 600 * 2;//varInfo.xres * varInfo.yres * varInfo.bits_per_pixel / 8;
	//pmap = (u16 *) mmap(NULL, size,PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);

	
	//memset(pmap ,0x00,size);

	
	close(fd);

	
	fd = open("/dev/fb1", O_RDWR);
		//printf();
	if(fd == -1)
	{
		printf("Failed to open fb device\n");
		return FALSE;
	}
	

	stAlpha.bAlphaEnable = HI_TRUE;
	stAlpha.bAlphaChannel = HI_FALSE;
	stAlpha.u8Alpha0 = 0x0;
	stAlpha.u8Alpha1 = 0xff;
	if (ioctl(fd, FBIOPUT_ALPHA_HIFB,  &stAlpha) < 0)
	{
		printf("Set alpha failed!\n");
		close(fd);
		return -1;
	}
		
	if(ioctl(fd, FBIOGET_VSCREENINFO, &varInfo) == -1)
	{
		printf("Failed ioctl FBIOGET_VSCREENINFO on\n");
		close(fd);
		return FALSE;
	}

	varInfo.xres = varInfo.xres_virtual = 720;
	varInfo.yres = varInfo.yres_virtual = 576;
	varInfo.bits_per_pixel = 16;

	if(ioctl(fd, FBIOPUT_VSCREENINFO, &varInfo) == -1)
	{
		printf("Failed ioctl FBIOPUT_VSCREENINFO on \n");
		close(fd);
		return FALSE;
	}

	size = 720 * 576 * 2;//varInfo.xres * varInfo.yres * varInfo.bits_per_pixel / 8;
	pmap = (u16 *) mmap(NULL, size,PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);
	if (pmap == MAP_FAILED)
	{
	
		close(fd);
		return FALSE;
	}
	
	memset(pmap ,0x00,size);

	close(fd);

	

	fd = open("/dev/fb0", O_RDWR);

	//printf();
	if(fd == -1)
	{
		printf("Failed to open fb device\n");
		return FALSE;
	}
	
	//HIFB_ALPHA_S stAlpha;
	//struct fb_var_screeninfo varInfo;

	stAlpha.bAlphaEnable = HI_TRUE;
	stAlpha.bAlphaChannel = HI_FALSE;
	stAlpha.u8Alpha0 = 0x0;
	stAlpha.u8Alpha1 = 0xff;
	if (ioctl(fd, FBIOPUT_ALPHA_HIFB,  &stAlpha) < 0)
	{
		printf("Set alpha failed!\n");
		close(fd);
		return -1;
	}
		
	if(ioctl(fd, FBIOGET_VSCREENINFO, &varInfo) == -1)
	{
		printf("Failed ioctl FBIOGET_VSCREENINFO on\n");
		close(fd);
		return FALSE;
	}

	varInfo.xres = varInfo.xres_virtual = 800;
	varInfo.yres = varInfo.yres_virtual = 600;
	varInfo.bits_per_pixel = 16;

	if(ioctl(fd, FBIOPUT_VSCREENINFO, &varInfo) == -1)
	{
		printf("Failed ioctl FBIOPUT_VSCREENINFO on \n");
		close(fd);
		return FALSE;
	}

	size = 800 * 600 * 2;//varInfo.xres * varInfo.yres * varInfo.bits_per_pixel / 8;
	pmap = (u16 *) mmap(NULL, size,PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);
	if (pmap == MAP_FAILED)
	{
	
		close(fd);
		return FALSE;
	}
	
	memset(pmap ,0x00,size);

	fd = open("/dev/fb2", O_RDWR);
		//printf();
	if(fd == -1)
	{
		printf("Failed to open fb device\n");
		return FALSE;
	}
	

	stAlpha.bAlphaEnable = HI_TRUE;
	stAlpha.bAlphaChannel = HI_FALSE;
	stAlpha.u8Alpha0 = 0x0;
	stAlpha.u8Alpha1 = 0xff;
	if (ioctl(fd, FBIOPUT_ALPHA_HIFB,  &stAlpha) < 0)
	{
		printf("Set alpha failed!\n");
		close(fd);
		return -1;
	}
		
	if(ioctl(fd, FBIOGET_VSCREENINFO, &varInfo) == -1)
	{
		printf("Failed ioctl FBIOGET_VSCREENINFO on\n");
		close(fd);
		return FALSE;
	}

	varInfo.xres = varInfo.xres_virtual = 720;
	varInfo.yres = varInfo.yres_virtual = 576;
	varInfo.bits_per_pixel = 16;

	if(ioctl(fd, FBIOPUT_VSCREENINFO, &varInfo) == -1)
	{
		printf("Failed ioctl FBIOPUT_VSCREENINFO on \n");
		close(fd);
		return FALSE;
	}

	size = 720 * 576 * 2;//varInfo.xres * varInfo.yres * varInfo.bits_per_pixel / 8;
	pmap = (u16 *) mmap(NULL, size,PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);
	if (pmap == MAP_FAILED)
	{
	
		close(fd);
		return FALSE;
	}
	
	memset(pmap ,0x00,size);

	fd = open("/dev/fb0", O_RDWR);

	//printf();
	if(fd == -1)
	{
		printf("Failed to open fb device\n");
		return FALSE;
	}
	
	//HIFB_ALPHA_S stAlpha;
	//struct fb_var_screeninfo varInfo;

	stAlpha.bAlphaEnable = HI_TRUE;
	stAlpha.bAlphaChannel = HI_FALSE;
	stAlpha.u8Alpha0 = 0x0;
	stAlpha.u8Alpha1 = 0xff;
	if (ioctl(fd, FBIOPUT_ALPHA_HIFB,  &stAlpha) < 0)
	{
		printf("Set alpha failed!\n");
		close(fd);
		return -1;
	}
		
	if(ioctl(fd, FBIOGET_VSCREENINFO, &varInfo) == -1)
	{
		printf("Failed ioctl FBIOGET_VSCREENINFO on\n");
		close(fd);
		return FALSE;
	}

	varInfo.xres = varInfo.xres_virtual = 800;
	varInfo.yres = varInfo.yres_virtual = 600;
	varInfo.bits_per_pixel = 16;

	if(ioctl(fd, FBIOPUT_VSCREENINFO, &varInfo) == -1)
	{
		printf("Failed ioctl FBIOPUT_VSCREENINFO on \n");
		close(fd);
		return FALSE;
	}

	size = 800 * 600 * 2;//varInfo.xres * varInfo.yres * varInfo.bits_per_pixel / 8;
	pmap = (u16 *) mmap(NULL, size,PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);
	if (pmap == MAP_FAILED)
	{
	
		close(fd);
		return FALSE;
	}
	
	memset(pmap ,0x00,size);


	
	fd = open("/dev/fb1", O_RDWR);
		//printf();
	if(fd == -1)
	{
		printf("Failed to open fb device\n");
		return FALSE;
	}
	

	stAlpha.bAlphaEnable = HI_TRUE;
	stAlpha.bAlphaChannel = HI_FALSE;
	stAlpha.u8Alpha0 = 0x0;
	stAlpha.u8Alpha1 = 0xff;
	if (ioctl(fd, FBIOPUT_ALPHA_HIFB,  &stAlpha) < 0)
	{
		printf("Set alpha failed!\n");
		close(fd);
		return -1;
	}
		
	if(ioctl(fd, FBIOGET_VSCREENINFO, &varInfo) == -1)
	{
		printf("Failed ioctl FBIOGET_VSCREENINFO on\n");
		close(fd);
		return FALSE;
	}

	varInfo.xres = varInfo.xres_virtual = 720;
	varInfo.yres = varInfo.yres_virtual = 576;
	varInfo.bits_per_pixel = 16;

	if(ioctl(fd, FBIOPUT_VSCREENINFO, &varInfo) == -1)
	{
		printf("Failed ioctl FBIOPUT_VSCREENINFO on \n");
		close(fd);
		return FALSE;
	}

	size = 720 * 576 * 2;//varInfo.xres * varInfo.yres * varInfo.bits_per_pixel / 8;
	pmap = (u16 *) mmap(NULL, size,PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);
	if (pmap == MAP_FAILED)
	{
	
		close(fd);
		return FALSE;
	}
	
	memset(pmap ,0x00,size);
}
#endif

void TestFBShow()
{
	HI_BOOL show = HI_FALSE;
	
	int fd = -1;
	char tmp[20] = {0};

	for(int i = 4; i >-1;i--)
	{
		printf("before sleep.....\n");
		sleep(5);
		printf("after  sleep.....\n");
		memset(tmp, 0, sizeof(tmp));
		sprintf(tmp, "/dev/fb%d", i);
		fd = open(tmp, O_RDWR);
		if(fd == -1)
		{
			printf("Failed to open fb device [%s]\n", tmp);
		}
		else
		{
			if(ioctl(fd, FBIOPUT_SHOW_HIFB, &show) == -1)
			{
				printf("Failed ioctl FBIOPUT_SHOW_HIFB [%s][%d] \n", tmp,show);
			}
			close(fd);
		}
		show = (show == HI_FALSE?HI_TRUE:HI_FALSE);
		i++;
	}
}

extern void bizData_CreateCDC();
extern void bizData_resumeRecord();

void ShowNoAuth(void)//cw_auth
{
	//csp modify
	//MessageBox("&CfgPtn.Noauth", "&CfgPtn.WARNING" , MB_OK|MB_ICONWARNING);
	MessageBox("&CfgPtn.Noauth", "&CfgPtn.WARNING" , MB_NULL|MB_ICONWARNING);
	
	//csp modify
	if(CDC::GetModify())
	{
		GraphicsSyncVgaToCvbs();
		CDC::SetModify(FALSE);
	}
}

void CChallenger::Initialize(int argc, char * argv[])
{
    g_ThreadManager.RegisterMainThread(ThreadGetID());// 注册主线程

	g_TimerManager.Start();//yaogang modify 20140913
	
	sem_t* sem = NULL;
	GetSemForDC(&sem);
	sem_init(sem, 0, 0);
	
	g_GUI.HideScreen();//清除framebuffer,否则执行下面会花屏
	
	SBizInitPara sBizInitPara;
	sBizInitPara.nModMask = 0xffffffff;// & (~MASK_BIZ_MOD_ALARM);
	
	
	sBizInitPara.pfnBizEventCb = BizEventCB;
	
	printf("#########################################################################before BizInit-1\n");
	BizInit(&sBizInitPara);//打一次初始化(配置模块)//why???
	
	printf("#########################################################################before g_GUI.Start\n");
	g_GUI.Start();//初始化GUI
	
#if 1//csp modify 20130101
	printf("#########################################################################before g_GUI.CreatePages\n");
    	g_GUI.CreatePages();//创建所有页面
#endif
	
#if 1//csp modify 20130101
	//printf("#########################################################################before bizData_CreateCDC\n");
	bizData_CreateCDC();
	//printf("after bizData_CreateCDC\n");
#endif
	
	//printf("#########################################################################before BizSetNoAuthFuncCB\n");
	BizSetNoAuthFuncCB(ShowNoAuth);//cw_auth
	
	//printf("#########################################################################before SetPrevicwModeFuncCB\n");
	SetPrevicwModeFuncCB(SetCurPreviewMode_CW);//cw_preview
	
	//printf("#########################################################################before RegLedCtrlFUN\n");
	RegLedCtrlFUN(RecLedCtrl);
	
	printf("#########################################################################before BizInit-2\n");
	SBizDvrInfo DvrInfo;
	if (BizGetDvrInfo(&DvrInfo) == 0)
	{
		if (DvrInfo.nNVROrDecoder != 1) //== 2解码器版本
		{
			//yaogang modify 20150324
			sBizInitPara.nModMask &= ~(MASK_BIZ_MOD_RECORD|MASK_BIZ_MOD_PLAYBACK|MASK_BIZ_MOD_SNAP);
		}
	}
	
    	BizInit(&sBizInitPara);//再次初始化(其他模块)//why???
    
	//g_GUI.Start();//初始化GUI
	
#if 0//csp modify 20130101
	//printf("#########################################################################before g_GUI.CreatePages\n");
    g_GUI.CreatePages();//创建所有页面
#endif
	
	SetSystemLockStatus(0);//cw_reboot
	
#if 0//csp modify 20130101
	//printf("#########################################################################before bizData_CreateCDC\n");
	bizData_CreateCDC();
	//printf("after bizData_CreateCDC\n");
#endif
	
	sem_post(sem);
	usleep(100*1000);
	
#if 1//csp modify 20130101
	//printf("#########################################################################before EnableCursor\n");
	CPage** page = GetPage();
	CPageDesktop* pPageDesktop = (CPageDesktop *)page[EM_PAGE_DESKTOP];
	pPageDesktop->Open();//打开桌面
	CCursor::instance(0)->EnableCursor(CDevMouse::instance()->HasFound());//add by nike.xie 20091020//在此处设置鼠标状态
#endif
	
   	SBizPreviewPara sBPP;
    int nChMax = GetMaxChnNum();
    switch(nChMax)
    {
        case 4:
            sBPP.emBizPreviewMode = EM_BIZPREVIEW_4SPLITS;
            break;
        case 8:
            sBPP.emBizPreviewMode = EM_BIZPREVIEW_9SPLITS;
            break;
        case 10:
            sBPP.emBizPreviewMode = EM_BIZPREVIEW_10SPLITS;
            break;
		 case 6://csp modify
            sBPP.emBizPreviewMode = EM_BIZPREVIEW_6SPLITS;
            break;
        case 16:
            sBPP.emBizPreviewMode = EM_BIZPREVIEW_16SPLITS;
            break;
        case 24:
			sBPP.emBizPreviewMode = EM_BIZPREVIEW_25SPLITS;
            //sBPP.emBizPreviewMode = EM_BIZPREVIEW_24SPLITS;
            break;
        case 32:
            sBPP.emBizPreviewMode = EM_BIZPREVIEW_36SPLITS;
            break;
        default:
            sBPP.emBizPreviewMode = EM_BIZPREVIEW_4SPLITS;
            break;                
    }
	//sBPP.emBizPreviewMode = EM_BIZPREVIEW_4SPLITS;
	//printf("StartPreview,PreviewMode:%d\n",sBPP.emBizPreviewMode);
	sBPP.nModePara = 0;
	BizStartPreview(&sBPP);
	printf("BizStartPreview[%d] finished!!!\n",sBPP.emBizPreviewMode);
	
	SetCurPreviewMode_CW(sBPP.emBizPreviewMode);
	
#if 0//csp modify 20130101
	bizData_resumeRecord();
#endif
	
	SBizCfgPatrol psPara;
	BizGetIsPatrolPara(0, &psPara, 0);
	
	if(1 == psPara.nIsPatrol)
	{
		ControlPatrol(1);
	}
	
#if 1//csp modify 20130101
	bizData_resumeRecord();
#endif
}

void* HideHddInfo(void* para)
{
	pthread_detach(pthread_self());
	
	sleep(5);
	
	//printf("^^^^^^^^^^^^^^^^^^^^^Here hide hdd info####################################################\n");
	
	g_GUI.onFrontboardIntput(XM_KEYDOWN,KEY_RIGHT,0);
	g_GUI.onFrontboardIntput(XM_KEYDOWN,KEY_RET,0);
	
	return 0;
}

void* CheckHddInfo(void* para)
{
	//printf("$$$$$$$$$$$$$$$$$$CheckHddInfo id:%d\n",getpid());
	
	pthread_detach(pthread_self());//csp modify
	
	//检查硬盘线程直接退出
	//pthread_exit(0);
	
	//printf("^^^^^^^^^^^^^^^^^^^^^Here show hdd info####################################################\n");
#if 1
	u8 bDiskIsExist = 0;
	u8 bDiskIsErr = 0;
	u8 bDiskIsUnFormat = 0;

	CheckAllHDD(&bDiskIsExist, &bDiskIsErr, &bDiskIsUnFormat);
	printf("%s bDiskIsExist: %d, bDiskIsErr: %d, bDiskIsUnFormat: %d\n", 
		__func__, bDiskIsExist, bDiskIsErr, bDiskIsUnFormat);

	if(!bDiskIsExist)
	{
		MessageBox("&CfgPtn.DiskIsNotExist", "&CfgPtn.WARNING" , MB_OK|MB_ICONWARNING);
	}
	else
	{
		if (bDiskIsErr)
		{
			MessageBox("&CfgPtn.CheckedHDDErr", "&CfgPtn.Error" , MB_OK|MB_ICONERROR);
		}
		else if (bDiskIsUnFormat)
		{
			MessageBox("&CfgPtn.CheckedHDDNotFormated", "&CfgPtn.Info" , MB_OK|MB_ICONINFORMATION);
		}
		else
		{
			char tmp[100] = {0};
			sprintf(tmp, "%s : %d", GetParsedString("&CfgPtn.DiskNumberChecked"), bDiskIsExist);
			MessageBox(tmp, "&CfgPtn.Info" , MB_OK|MB_ICONINFORMATION);
		}
	}
	
#else	
	//检测是否有硬盘,无则弹出提示框
	bDiskIsExist = GetHardDiskNum();
	if(!bDiskIsExist)
	{
		MessageBox("&CfgPtn.DiskIsNotExist", "&CfgPtn.WARNING" , MB_OK|MB_ICONWARNING);
	}
	else
	{
		else if (!CheckAllHDDFormated())
		{
			MessageBox("&CfgPtn.CheckedHDDNotFormated", "&CfgPtn.Info" , MB_OK|MB_ICONINFORMATION);
		}
		else
		{
			char tmp[100] = {0};
			sprintf(tmp, "%s : %d", GetParsedString("&CfgPtn.DiskNumberChecked"), bDiskIsExist);
			MessageBox(tmp, "&CfgPtn.Info" , MB_OK|MB_ICONINFORMATION);
		}
	}
#endif	
	//add by Lirl on Nov/17/2011
	SBizEventPara sBizEventPara;
	memset(&sBizEventPara,0,sizeof(SBizEventPara));
	
	//刷新桌面通道号名称
	int i;
	for(i = 0; i < GetMaxChnNum(); i++)
	{
		sBizEventPara.sBizRecStatus.nChn = i;
		sBizEventPara.emType = EM_BIZ_EVENT_LIVE_REFRESH;
		BizSendBizEvent(&sBizEventPara);
	}
	//end
	
	return 0;
}

static int wd_fd = -1;
static int wd_closed = 0;
//static sem_t semForWD;
#define DEV_WD "/dev/watchdog"

void *watchdogThrFxn(void *arg)
{
	wd_fd = open(DEV_WD, O_RDWR, 0);
	if(wd_fd == -1)
	{
		printf("Cannot open %s (%s)\n", DEV_WD, strerror(errno));
		return 0;
	}
	//sem_init(&semForWD, 0, 1);
	//printf("watchdogThrFxn going ........\n");
	printf("$$$$$$$$$$$$$$$$$$watchdogThrFxn id:%d\n",getpid());
	while(1)
	{
		//sem_wait(&semForWD);
		if(!wd_closed)
		{
			ioctl(wd_fd, WDIOC_KEEPALIVE, NULL);
		}
		//sem_post(&semForWD);
		sleep(10);
	}
	close(wd_fd);
	return 0;
}

#if 1//csp modify
void term_exit(int signo)
{
	time_t cur;
	cur = time(NULL);
	//printf("term_exit:system time:%s\n",  ctime(&cur));	
	printf("!!!!!!recv signal(%d),SIGBUS=%d,SIGPIPE=%d,at %s",signo,SIGBUS,SIGPIPE,ctime(&cur));
	if(signo != 17)//子进程结束//SIGCHLD
	{
		//SIGINT=2;//SIGTSTP=20
		if(signo != SIGINT && signo != SIGTSTP && signo != 21 && signo != SIGQUIT && signo != SIGWINCH)
		{
			//sleep(10);
			printf("process quit!!!\n");
			exit(-1);
		}
		else
		{
			//ignore "CTRL+C" "CTRL+Z"
			printf("???\n");
		}
	}
}
#endif

#if 0
void* TestResetThrxd(void* arg)
{
	static u8 bGotCmd = 0;
	
	if(!bGotCmd)
	{
		bGotCmd = 1;
		
		printf("ResetNotify......\n");
		fflush(stdout);
		
		SBizEventPara sBizEventPara;
		sBizEventPara.emType = EM_BIZ_EVENT_NET_CHANGEPARA_RESET;
		BizEventCB(&sBizEventPara);
		
		printf("ResetThrxd sleep-1\n");
		fflush(stdout);
		
		sleep(5);
		
		printf("ResetThrxd sleep-2\n");
		fflush(stdout);
		
		BizSysComplexExit(EM_BIZSYSEXIT_RESTART);
		
		printf("RESTART\n");
		fflush(stdout);
	}
	
	return NULL;
}
#endif

//extern "C" time_t read_rtc(int utc);

int main(int argc, char *argv[])
{
	//read_rtc(1);
	
	int err = 0;
	
#if 0//csp modify
#ifdef WIN32
	InitApiLib();
#else
	//signal(SIGPIPE,SIG_IGN);//忽略SIGPIPE信号
	//signal(SIGPIPE, SignalFunc); //原来是接到SIGPIPE消息后，调用函数SignalFunc, 但signal捕获一次后就不再捕获了，所以不能用
	//直接忽略SIGPIPE消息//Modified by billhe at 2009-9-17
	struct sigaction action;//信号处理结构体
	action.sa_handler = SignalFunc;//产生信号时的处理函数
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	sigaction(SIGPIPE,&action,NULL);//信号类型
#endif
#endif
	
#if 1//csp modify
	//注册信号处理函数
	int i;
	for(i=1;i<32;i++)
	{
		if(i == SIGBUS) continue;
		if(i == SIGPIPE) continue;
		if(i == 11) continue;
		if(signal(i,term_exit) == SIG_ERR)
		{
			printf("Register signal(%d) handler failed\n",i);
		}
	}
	
	//ignore BROKE_PIPE signal
	signal(SIGPIPE,SIG_IGN);//忽略SIGPIPE信号
#endif
	
//#if defined(CHIP_HISI351X_2X) && defined(_R9516S_CX25838_CX25828_)//csp modify
	//
//#endif
	
#if 1//csp modify
	pthread_t pid;
	pthread_create(&pid, NULL, watchdogThrFxn, NULL);
#endif
	
	//printf("before Initialize\n");
	g_Challenger.Initialize(argc, argv);//Challenger:挑战者
	
/*
	//yzw add 有时会出现机器起来后录像没有通道名
	SBizCfgStrOsd bizStrOsd;
	SBizParaTarget sParaTgtIns;
	sParaTgtIns.emBizParaType = EM_BIZ_STROSD;
	for(int i = 0; i < GetVideoMainNum(); i++)
	{
		sParaTgtIns.nChn = i;
		int rtn = BizGetPara(&sParaTgtIns, &bizStrOsd);
		if(0==rtn)
		{
			BizSetPara(&sParaTgtIns, &bizStrOsd);
		}
	}
	//end
*/
	SBizDvrInfo sInfo;
	if (BizGetDvrInfo(&sInfo))
	{
		printf("%s BizGetDvrInfo failed\n", __func__);
		exit(-1);
	}

	char tmp[30] = {0};
	strcpy(tmp, sInfo.sproductnumber);
	
    //开机向导
    if(GetShowGuide())
    {
		
    	CPage** page = GetPage();
		
		if((0 == strcasecmp(tmp, "NR3132")) || (0 == strcasecmp(tmp, "NR3124")))
		{
			usleep(200*1000);//等待预览回调刷新通道名OSD
			
			#if 0
	    	//刷新桌面通道号名称
			int i;
			for(i = 0; i < GetMaxChnNum(); i++)
			{
				SBizEventPara sBizEventPara;
				memset(&sBizEventPara,0,sizeof(SBizEventPara));
				
				sBizEventPara.sBizRecStatus.nChn = i;
				sBizEventPara.emType = EM_BIZ_EVENT_LIVE_REFRESH;
				BizSendBizEvent(&sBizEventPara);
				
				//printf("###chn%d name:%s\n",i,(((CPageDesktop*)page[EM_PAGE_DESKTOP])->m_vChannelName[i])->GetText());
			}
			//end
			#endif
		}
		
		if (sInfo.nNVROrDecoder == 1) //NVR
		{
			printf("yghahaha **************************************\n");
			page[EM_PAGE_BASICCFG]->UpdateData(UDM_GUIDE);
			page[EM_PAGE_BASICCFG]->Open();
			
			//page[EM_PAGE_BASICCFG]->UpdateData(UDM_EMPTY);
		}
    }
	else
	{
		if (sInfo.nNVROrDecoder == 1) //NVR
		{
			pthread_t pid_tmp = 0;
			pthread_create(&pid_tmp, NULL, CheckHddInfo, NULL);
			//CheckHddInfo(NULL);//csp modify
		}
	}
	
	//csp modify
	if(CDC::GetModify())
	{
		GraphicsSyncVgaToCvbs();
		CDC::SetModify(FALSE);
	}
	
	#if 0//csp modify
	pthread_t pid;
	pthread_create(&pid, NULL, watchdogThrFxn, NULL);
	#endif

    BizSysComplexSetProductModel(tmp);
	
	//#ifdef _JMV_
	#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521) || defined(_JMV_)
	if(!GetShowGuide())
	{
		if (sInfo.nNVROrDecoder == 1) //NVR
		{
			pthread_t pid_tmp = 0;
			pthread_create(&pid_tmp, NULL, HideHddInfo, NULL);//手动上传按键，去除窗口
		}
	}
	#endif
	
    //Modify by Lirl on Jan/05/2012
    //删除没用的数据,省出内存空间
    #if defined(RELEASE) || defined(RELEASE_VERSION)
    rmMemData();//why???//删除UI目录里面的一些文件
    #endif
	
#if 1//#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify
	if(1)
#else
	if(BizVerifySN() == 0)//csp modify
#endif
	{
		if (sInfo.nNVROrDecoder == 1) //NVR
		{
			usleep(1000*1000);
			BizRecordSetWorkingEnable(1);//使能录像模块yzw
		}
	}
	else//csp modify
	{
		CPage** page = GetPage();
        if(page && page[EM_PAGE_DESKTOP])
        {
        	CPageDesktop *pDesktop = (CPageDesktop *)page[EM_PAGE_DESKTOP];
			if(pDesktop->pStaticVerifySNResult)
			{
				pDesktop->pStaticVerifySNResult->Show(TRUE);
			}
        }
	}
	
#if 0//csp modify
	usleep(1000*1000);
	BizRecordSetWorkingEnable(1);//使能录像模块yzw
#endif
	
	usleep(100*1000);
	BizAlarmWorkingEnable(1);//使能报警模块yzw
	
	BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_POWER_ON);//csp modify
	
	//TestFBShow();
	
	//sleep(10);
	//printf("\n\n&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&ResetThrxd\n");
	//fflush(stdout);
	//pthread_t resetThrxd;
	//pthread_create(&resetThrxd, NULL, TestResetThrxd, NULL);
	
#if 1//NVR TEST
	#include "lib_common.h"
	#include "lib_preview.h"
	
	#if 0
	ipc_node* head = NULL;
	int ret = IPC_Search(&head, PRO_TYPE_ONVIF, 0);
	if(ret < 0 || head == NULL)
	{
		//return -1;
	}
	else
	{
		//tl_preview(PREVIEW_CLOSE, 0);
		//tl_preview(PREVIEW_1D1, 0);
		
		sleep(8);
		
		printf("\n\n\nstart request rtsp stream......\n");
		
		head->ipcam.enable = 1;
		//head->ipcam.wPort = 554;
		tl_set_ipcamera(0, &head->ipcam);
		
		IPC_Free(head);
		head = NULL;
	}
	#endif
	
	//csp modify 20131213
	IPC_SetTimeZone(GetTimeZone(), GetTimeSyncToIPC(), 0);
	
	sleep(8);
	
	u8 ipc_flag = 0;
	
	int nChMax = GetMaxChnNum();
	for(int idx = 0; idx < nChMax; idx++)
	{
		SBizIPCameraPara cam;
		SBizIPCameraPara *psPara = &cam;
		
		memset(&cam, 0, sizeof(cam));
		cam.channel_no = idx;
		
		SBizParaTarget bizTar;
		bizTar.emBizParaType = EM_BIZ_IPCAMERAPARA;
		bizTar.nChn = idx;
		BizGetPara(&bizTar, (void *)&cam);
		
		ipc_unit ipcam;
		ipcam.channel_no = psPara->channel_no;
		ipcam.enable = psPara->enable;
		ipcam.protocol_type = psPara->protocol_type;
		ipcam.trans_type = psPara->trans_type;
		ipcam.stream_type = psPara->stream_type;
		ipcam.ipc_type = psPara->ipc_type;
		ipcam.dwIp = psPara->dwIp;
		ipcam.wPort = psPara->wPort;
		strcpy(ipcam.user, psPara->user);
		strcpy(ipcam.pwd, psPara->pwd);
		strcpy(ipcam.address, psPara->address);
		//yaogang modify 20141010
		ipcam.main_audio_sw = psPara->main_audio_sw;
		ipcam.sub_audio_sw = psPara->sub_audio_sw;
		
		if(ipcam.enable)
		{
			tl_set_ipcamera(idx, &ipcam);
			ipc_flag = 1;
		}
	}
	//跃天不需要开机无IPC，自动搜索
	#if 0
	if(ipc_flag == 0)
	{
		ipc_node *m_ipc_head = NULL;
		unsigned int pro_type = (unsigned int)(-1);//PRO_TYPE_ONVIF;
		unsigned char check_conflict = 0;//1;//(i == 6) ? 1 : 0;
		int ret = IPC_Search(&m_ipc_head, pro_type, check_conflict);
		if(ret < 0 || m_ipc_head == NULL)
		{
			
		}
		else
		{
			int count = 0;
			ipc_node *p = m_ipc_head;
			while(p)
			{
				SBizIPCameraPara ipcam;
				memset(&ipcam, 0, sizeof(ipcam));
				
				ipcam.channel_no = count;
				ipcam.enable = 1;//p->ipcam.enable;
				ipcam.protocol_type = p->ipcam.protocol_type;
				ipcam.trans_type = p->ipcam.trans_type;
				ipcam.stream_type = p->ipcam.stream_type;
				ipcam.ipc_type = p->ipcam.ipc_type;
				ipcam.dwIp = p->ipcam.dwIp;
				ipcam.wPort = p->ipcam.wPort;
				strcpy(ipcam.user, p->ipcam.user);
				strcpy(ipcam.pwd, p->ipcam.pwd);
				struct in_addr host;
				host.s_addr = ipcam.dwIp;
				sprintf(ipcam.address, "http://%s:%d/onvif/device_service", inet_ntoa(host), ipcam.wPort);
				
				SBizParaTarget bizTar;
				bizTar.emBizParaType = EM_BIZ_IPCAMERAPARA;
				bizTar.nChn = ipcam.channel_no;
				BizSetPara(&bizTar, (void *)&ipcam);
				
				count++;
				if(count >= nChMax)
				{
					break;
				}
				
				p = p->next;
			}
			
			IPC_Free(m_ipc_head);
			m_ipc_head = NULL;
		}
	}
	#endif
#endif
	
	time_t app_start_time = time(NULL);
	
	//yaogang modify 20141217
	//系统已经成功启动，上传深广平台
	printf("NVR Start & Upload SG Platform Heart***************************\n");
	SSG_MSG_TYPE msg;
	SBizParaTarget sParaTgtIns;
	SBizSGParam sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_SGPARA;
	
	if (BizGetPara(&sParaTgtIns, &sCfgIns) == 0)
	{
		if (sCfgIns.normal_exit == 0)//系统异常退出
		{
			memset(&msg, 0, sizeof(msg));
			msg.type = EM_DVR_EXIT_ABNORMAL;
			msg.chn = 0;
			strcpy(msg.note, GetParsedString("&CfgPtn.Exit_Abnormal"));//上一次系统异常退出
			upload_sg(&msg);
		}

		sCfgIns.normal_exit = 0;//写入0，如果正常退出会再次写入1
		BizSetPara(&sParaTgtIns, &sCfgIns);
	}
	
	memset(&msg, 0, sizeof(msg));
	msg.type = EM_DVR_STARTUP;
	msg.chn = 0;
	strcpy(msg.note, GetParsedString("&CfgPtn.StartWork"));
	upload_sg(&msg);

	
	memset(&msg, 0, sizeof(msg));
	msg.type = EM_HEART;
	msg.chn = 0;
	upload_sg(&msg);
	
	//
#if 0//yaogang test CLOCK_MONOTONIC
	struct timespec time1 = { 0, 0 };

	while (1)
	{
		clock_gettime(CLOCK_MONOTONIC, &time1);  
    	printf("CLOCK_MONOTONIC: %d, %d\n", time1.tv_sec, time1.tv_nsec);
		
		sleep(1);
	}
#else
	
	char cmdline[64];
	while(1)
	{	
		char chSeps[8] = " :\t";
		char *pchToken;
		//printf("[user]#");
		printf("->");//csp modify
		//PRINT("here");
		memset(cmdline,0,sizeof(cmdline));
		//PRINT("here");
		fgets(cmdline,sizeof(cmdline),stdin);
		//PRINT("here");
		cmdline[strlen(cmdline)-1] = '\0';
		
		if(strcmp(cmdline,"haltwd") == 0)//if(strcmp(cmdline,"wdover") == 0 || strcmp(cmdline,"closewd") == 0)//csp modify
		{
            if(wd_fd == -1)
            {
                printf("%s is not opened\n", DEV_WD);
                continue;
            }
			//printf("closewd......\n");
			printf("disable watchdog,WDIOS_DISABLECARD=%d......\n",WDIOS_DISABLECARD);
            //sem_wait(&semForWD);
			{
				wd_closed = 1;
				
				int ret = ioctl(wd_fd, WDIOC_KEEPALIVE, NULL);//csp modify
				if(ret < 0)
				{
					printf("watchdog WDIOC_KEEPALIVE failed\n");
				}
				else
				{
					printf("watchdog WDIOC_KEEPALIVE success\n");
				}
				
				sleep(1);//csp modify
				
				int value = WDIOS_DISABLECARD;
				ret = ioctl(wd_fd, WDIOC_SETOPTIONS, &value);
				if(ret < 0)
				{
					printf("disable watchdog failed,error:(%d,%s)\n",errno,strerror(errno));
				}
				else
				{
					printf("disable watchdog success\n");
				}
				
				//close(wd_fd);
				//wd_fd = -1;
			}
            //sem_post(&semForWD);
		}
		else if(strcmp(cmdline,"runwd") == 0)//else if(strcmp(cmdline,"wdrevive") == 0 || strcmp(cmdline,"startwd") == 0)//csp modify
		{	
			if(wd_fd == -1) 
			{
				printf("%s is not opened\n", DEV_WD);
				continue;
			}
			//printf("startwd......\n");
			printf("enable watchdog......\n");
			//sem_wait(&semForWD);
			{
				wd_closed = 0;
				ioctl(wd_fd, WDIOC_SETOPTIONS, WDIOS_ENABLECARD);
			}
			//sem_post(&semForWD);
		}
		else if(strcmp(cmdline,"abort") == 0)//else if(strcmp(cmdline,"exit") == 0)//csp modify
		{
			printf("app exit......\n");
			system("killall webs");
			exit(1);
		}
		else if(strcmp(cmdline,"ifconfig") == 0)//csp modify
		{
			//printf("ifconfig......\n");
			system("ifconfig");
			printf("->\n");
			system("route");
			printf("->\n");
			system("cat /etc/resolv.conf");
			printf("->\n");
		}
		else if(strcmp(cmdline,"dmesg") == 0)
		{
			system("dmesg");
			printf("->\n");
		}
		else if(strcmp(cmdline,"time") == 0)
		{
			printf("system startup time:%s",ctime(&app_start_time));
			time_t t = time(NULL);
			printf("now time:%s",ctime(&t));
		}
		else if(strcmp(cmdline,"ps") == 0)//csp modify
		{
			//printf("ps......\n");
			system("ps");
			printf("->\n");
		}
		else if(strcmp(cmdline,"help") == 0)
		{
			printf("->");//csp modify
		}
		else if(strcmp(cmdline,"alarmledon") == 0)
		{
			RecLedCtrl(1,ALARM_LED_CTRL);
			printf("control alarm led on->");
		}
		else if(strcmp(cmdline,"alarmledoff") == 0)
		{
			RecLedCtrl(0,ALARM_LED_CTRL);
			printf("control alarm led off->");
		}
		#if 0//csp modify
		else if(strcmp(cmdline,"iamover") == 0)
		{
			system("killall webs");
			exit(1);
		}
		else if(strcmp(cmdline,"hifb") == 0)
		{
			int fd = open("/dev/fb4", O_RDWR);
			int tmp = 0;
			HIFB_ALPHA_S stAlpha;
			scanf("%d",&tmp);
			stAlpha.bAlphaEnable = tmp==0?HI_FALSE:HI_TRUE;
			scanf("%d",&tmp);
			stAlpha.u8Alpha0 = tmp;
			scanf("%d",&tmp);
			stAlpha.u8Alpha1 = tmp;
			scanf("%d",&tmp);
			stAlpha.bAlphaChannel = tmp==0?HI_FALSE:HI_TRUE;
			scanf("%d",&tmp);
			stAlpha.u8GlobalAlpha = tmp;
			printf("%d,0x%x,0x%x,%d,0x%x\n",stAlpha.bAlphaEnable,stAlpha.u8Alpha0,stAlpha.u8Alpha1,stAlpha.bAlphaChannel,stAlpha.u8GlobalAlpha);
			if (ioctl(fd, FBIOPUT_ALPHA_HIFB, &stAlpha) < 0)
			{
				perror("FBIOPUT_ALPHA_HIFB");
			}
			close(fd);
		}
		#endif
		else
		{
			//printf("Invalid cmd!\n");
			
			VO_CSC_S stpubcscparm;
			
			sprintf(chSeps, " ,\t");
			pchToken = strtok(cmdline, chSeps);
			if(pchToken == NULL)
			{
				//printf("Try \"help\" for more information.\n");
			}
			else if(strcmp(pchToken, "hdrd") == 0)
			{
				pchToken = strtok(NULL, chSeps);
				if(pchToken == NULL) 
				{
					//printf("Try \"help\" for more information.\n");
				}
				else if(strcmp(pchToken, "lum") == 0)
				{
				#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
				#else
					stpubcscparm.enCSCType=VO_CSC_LUMA;
					int result00 = HI_MPI_VO_GetDevCSC(0, &stpubcscparm);
					if(0 == result00)
					{
						printf("Get VO_CSC_LUMA=%d--\n",stpubcscparm.u32Value);
					}
					else
					{
						printf("Get VO_CSC_LUMA---err--\n");
					}
				#endif
				}
				else if(strcmp(pchToken, "con") == 0) 
				{
				#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
				#else
					stpubcscparm.enCSCType=VO_CSC_CONTR;
					int result10 = HI_MPI_VO_GetDevCSC(0, &stpubcscparm);
					if(0 == result10)
					{
						printf("Get VO_CSC_CONTR=%d--\n",stpubcscparm.u32Value);
					}
					else
					{
						printf("Get VO_CSC_CONTR---err--\n");
					}
				#endif
				}
				else if(strcmp(pchToken, "hue") == 0) 
				{
				#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
				#else
					stpubcscparm.enCSCType=VO_CSC_HUE;
					int result20 = HI_MPI_VO_GetDevCSC(0, &stpubcscparm);
					if(0 == result20)
					{
						printf("Get VO_CSC_HUE=%d--\n",stpubcscparm.u32Value);
					}
					else
					{
						printf("Get VO_CSC_HUE---err--\n");
					}
				#endif
				}
				else if(strcmp(pchToken, "sat") == 0) 
				{
				#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
				#else
					stpubcscparm.enCSCType=VO_CSC_SATU;
					int result30 = HI_MPI_VO_GetDevCSC(0, &stpubcscparm);
					if(0 == result30)
					{
						printf("Get VO_CSC_SATU=%d--\n",stpubcscparm.u32Value);
					}
					else
					{
						printf("Get VO_CSC_SATU---err--\n");
					}
				#endif
				}
			}
			else if(strcmp(pchToken, "hdwr") == 0)
			{
				pchToken = strtok(NULL, chSeps);
				if(pchToken == NULL) 
				{
					//printf("Try \"help\" for more information.\n");
				}
				else if(strcmp(pchToken, "lum") == 0)
				{
					pchToken = strtok(NULL, chSeps);
					if(pchToken == NULL)
					{
						//printf("Try \"help\" for more information.\n");
					}
					else
					{
					#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
					#else
						int result01;					
						result01 = atoi(pchToken);
						stpubcscparm.enCSCType=VO_CSC_LUMA;
						stpubcscparm.u32Value = result01;
						result01 = HI_MPI_VO_SetDevCSC(0, &stpubcscparm);
						if(0 == result01)
						{
							printf("Set VO_CSC_LUMA=%d--\n",stpubcscparm.u32Value);
						}
						else
						{
							printf("Set VO_CSC_LUMA---err--\n");
						}
					#endif
					}
				}	
				else if(strcmp(pchToken, "con") == 0)
				{
					pchToken = strtok(NULL, chSeps);
					if(pchToken == NULL)
					{
						//printf("Try \"help\" for more information.\n");
					}
					else
					{
					#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
					#else
						int result11;					
						result11 = atoi(pchToken);
						stpubcscparm.enCSCType=VO_CSC_CONTR;
						stpubcscparm.u32Value = result11;
						result11 = HI_MPI_VO_SetDevCSC(0, &stpubcscparm);
						if(0 == result11)
						{
							printf("Set VO_CSC_CONTR=%d--\n",stpubcscparm.u32Value);
						}
						else
						{
							printf("Set VO_CSC_CONTR---err--\n");
						}
					#endif
					}
				}
				else if(strcmp(pchToken, "hue") == 0)
				{
					pchToken = strtok(NULL, chSeps);
					if(pchToken == NULL)
					{
						//printf("Try \"help\" for more information.\n");
					}
					else
					{
					#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
					#else
						int result21;
						result21 = atoi(pchToken);
						stpubcscparm.enCSCType=VO_CSC_HUE;
						stpubcscparm.u32Value = result21;
						result21 = HI_MPI_VO_SetDevCSC(0, &stpubcscparm);
						if(0 == result21)
						{
							printf("Set VO_CSC_HUE=%d--\n",stpubcscparm.u32Value);
						}
						else
						{
							printf("Set VO_CSC_HUE---err--\n");
						}
					#endif
					}
				}
				else if(strcmp(pchToken, "sat") == 0)
				{
					pchToken = strtok(NULL, chSeps);
					if(pchToken == NULL)
					{
						//printf("Try \"help\" for more information.\n");
					}
					else
					{
					#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
					#else
						int result31;
						result31 = atoi(pchToken);
						stpubcscparm.enCSCType=VO_CSC_SATU;
						stpubcscparm.u32Value = result31;
						result31 = HI_MPI_VO_SetDevCSC(0, &stpubcscparm);
						if(0 == result31)
						{
							printf("Set VO_CSC_SATU=%d--\n",stpubcscparm.u32Value);
						}
						else
						{
							printf("Set VO_CSC_SATU---err--\n");
						}
					#endif
					}
				}
			}
			#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
			else if(strcmp(pchToken, "wr2866") == 0)
			{
				int chip;
				pchToken = strtok(NULL,chSeps);
				if(pchToken == NULL)
				{
					printf("usage: wr2866 chip addr value\n->");
					continue;
				}
				chip = strtol(pchToken,NULL,10);
				
				int addr;
				pchToken = strtok(NULL,chSeps);
				if(pchToken == NULL)
				{
					printf("usage: wr2866 chip addr value\n->");
					continue;
				}
				addr = strtol(pchToken,NULL,16);
				
				int value;
				pchToken = strtok(NULL,chSeps);
				if(pchToken == NULL)
				{
					printf("usage: wr2866 chip addr value\n->");
					continue;
				}
				value = strtol(pchToken,NULL,16);
				
				printf("wr2866:chip=%d,addr=0x%02x,value=0x%02x\n",chip,addr,value);
				
				tw286x_wr(chip, addr, value);
				//printf("tw286x_wr, addr=%d\n", addr);
			}
			else if(strcmp(pchToken, "rd2866") == 0)
			{
				int chip;
				pchToken = strtok(NULL,chSeps);
				if(pchToken == NULL)
				{
					printf("usage: rd2866 chip addr\n->");
					continue;
				}
				chip = strtol(pchToken,NULL,10);
				
				int addr;
				pchToken = strtok(NULL,chSeps);
				if(pchToken == NULL)
				{
					printf("usage: rd2866 chip addr\n->");
					continue;
				}
				addr = strtol(pchToken,NULL,16);
								
				unsigned char value = 0xff;
				tw286x_rd(chip, addr, &value);
				printf("rd2866:chip=%d,addr=0x%02x,value=0x%02x\n",chip,addr,value);
			}
			else if(strcmp(pchToken, "hoft") == 0)
			{
				pchToken = strtok(NULL, chSeps);
				if(pchToken == NULL)
				{
					printf("usage: hoft channel value\n->");
					continue;
				}
				int channel = strtol(pchToken,NULL,10);
				
				pchToken = strtok(NULL, chSeps);
				if(pchToken == NULL)
				{
					printf("usage: hoft channel value\n->");
					continue;
				}
				int value = strtol(pchToken,NULL,16);
				
				int chip = channel/8;
				int page = (channel%8)/4;
				int addr = (channel%4)*0x10+0x0A;
				
				printf("hoft:channel=%d,chip=%d,page=%d,addr=%d,value=0x%02x\n",channel,chip,page,addr,value);
				
				tw286x_wr(chip, 0x40, page);
				tw286x_wr(chip, addr, value);
			}
			else if(strcmp(pchToken, "voft") == 0)
			{
				pchToken = strtok(NULL, chSeps);
				if(pchToken == NULL)
				{
					printf("usage: voft channel value\n->");
					continue;
				}
				int channel = strtol(pchToken,NULL,10);
				
				pchToken = strtok(NULL, chSeps);
				if(pchToken == NULL)
				{
					printf("usage: voft channel value\n->");
					continue;
				}
				int value = strtol(pchToken,NULL,16);
				
				int chip = channel/8;
				int page = (channel%8)/4;
				int addr = (channel%4)*0x10+0x08;
				
				printf("voft:channel=%d,chip=%d,page=%d,addr=%d,value=0x%02x\n",channel,chip,page,addr,value);
				
				tw286x_wr(chip, 0x40, page);
				tw286x_wr(chip, addr, value);
			}
			#endif
		}
	}
#endif
	
	CSemaphore sem;
	sem.Pend();
	
	return 0;
}

