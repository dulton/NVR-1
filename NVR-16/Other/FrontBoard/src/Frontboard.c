#include "public.h"
#include "mod_common.h"
#include "Frontboard.h"
#include "board.h"
//#include "mod_config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//csp modify
static int bEnable = 1;

//#ifdef CHIP_HISI351X_2X
//#define longse
//#endif

#if defined(longse) || defined(HI3520D)
#define OSDRV_MODULE_VERSION_STRING "NULL"
/* define device name */
#define HIIR_DEVICE_NAME "Hi_IR"

/* Ioctl definitions */
#define IR_IOC_SET_BUF               0x01
#define IR_IOC_SET_ENABLE_KEYUP      0x02// 1:check keyup
#define IR_IOC_SET_ENABLE_REPKEY     0x03// 1:check repkey, 0:hardware behave
#define IR_IOC_SET_REPKEY_TIMEOUTVAL 0x04
#define IR_IOC_SET_FORMAT            0x10
#define IR_IOC_SET_CODELEN           0x11
#define IR_IOC_SET_FREQ              0x12
#define IR_IOC_SET_LEADS             0x13
#define IR_IOC_SET_LEADE             0x14
#define IR_IOC_SET_SLEADE            0x15
#define IR_IOC_SET_CNT0_B            0x16
#define IR_IOC_SET_CNT1_B            0x17

#define IR_IOC_GET_CONFIG            0x20

#define IR_IOC_ENDBG                 0x31
#define IR_IOC_DISDBG                0x32

/* device parameter */
typedef struct
{
	unsigned short int leads_min;
	unsigned short int leads_max;
	unsigned short int leade_min;
	unsigned short int leade_max;
	unsigned short int cnt0_b_min;
	unsigned short int cnt0_b_max;
	unsigned short int cnt1_b_min;
	unsigned short int cnt1_b_max;
	unsigned short int sleade_min;
	unsigned short int sleade_max;
	unsigned short int code_len;
	unsigned short int codetype;
	unsigned short int frequence;
}hiir_dev_param;

#define HIIR_DEFAULT_FREQ 24//24M

/* DEFINE KEY STATE */
#define HIIR_KEY_DOWN 0x00
#define HIIR_KEY_UP   0x01

typedef struct
{
	unsigned long irkey_datah;			//IR 接收解码数据的高16 位寄存器
	unsigned long irkey_datal;			//IR 接收解码数据的低32 位寄存器，比如收到键值: 0xe718f708
										//分成高16位和低16位，每个字节的和等于0xFF, 0xF7 + 0x08 = 0xFF
										//其中低16位表示用户码，是一个固定值，因此可以忽略低16位
										//高16位表示按键数据，只需要取低八位的值就可以了，如 0x18
	unsigned long irkey_state_code;		//表示按键按下或松开，1表示松开，0表示按下
}irkey_info_s;
#endif

#define FILE_PATH_LOGIC_TO_PHYSIC			"/tmp2/panel.ini"
#define LOGIC_TO_PHYSIC_FOR_FRONTBOARD   	"panel"
#define LOGIC_TO_PHYSIC_FOR_REMOTE    		"remote"
#define FILE_PATH_LOGIC_TO_PHYSIC_DEFAULT	"/tmp/data/cfg/panel.ini"

#ifdef longse
static SFrontBoardKeyInfo sKeyTableLongIRLogicToPhysic[] = 
{
	{KEYCODE_FROM_PANEL_POWER,		0xff,	30,		"power"},//STANDBY
	{KEYCODE_FROM_PANEL_POWER,		0xff,	95,		"power"},//LOGIN/LOCK
	
	{KEYCODE_FROM_PANEL_1,			0xff,	29,		"num_1"},
	{KEYCODE_FROM_PANEL_2,			0xff,	28,		"num_2"},
	{KEYCODE_FROM_PANEL_3,			0xff,	31,		"num_3"},
	{KEYCODE_FROM_PANEL_4,			0xff,	25,		"num_4"},
	{KEYCODE_FROM_PANEL_5,			0xff,	24,		"num_5"},
	{KEYCODE_FROM_PANEL_6,			0xff,	91,		"num_6"},
	{KEYCODE_FROM_PANEL_7,			0xff,	21,		"num_7"},
	{KEYCODE_FROM_PANEL_8,			0xff,	20,		"num_8"},
	{KEYCODE_FROM_PANEL_9,			0xff,	87,		"num_9"},
	{KEYCODE_FROM_PANEL_0,			0xff,	16,		"num_0"},
	
	{KEYCODE_FROM_PANEL_MUTE,		0xff,	17,		"mute"},//AUDIO
	{KEYCODE_FROM_PANEL_MUTE,		0xff,	83,		"mute"},//MUTE
	
	{KEYCODE_FROM_PANEL_MULTI,		0xff,	13,		"multi"},
	{KEYCODE_FROM_PANEL_PTZ,		0xff,	79,		"ptz"},
	
	{KEYCODE_FROM_PANEL_FASTB,		0xff,	9,		"fastback"},
	{KEYCODE_FROM_PANEL_FAST,		0xff,	10,		"fastforward"},
	{KEYCODE_FROM_PANEL_SLOW,		0xff,	8,		"slowforward"},
	
	{KEYCODE_FROM_PANEL_PLAY,		0xff,	5,		"play"},
	{KEYCODE_FROM_PANEL_STEP,		0xff,	6,		"step"},
	{KEYCODE_FROM_PANEL_PAUSE,		0xff,	4,		"pause"},
	{KEYCODE_FROM_PANEL_STOP,		0xff,	71,		"stop"},
	{KEYCODE_FROM_PANEL_PLAY,		0xff,	1,		"play"},//SEARCH
	
	{KEYCODE_FROM_PANEL_MENU,		0xff,	93,		"menu"},
	{KEYCODE_FROM_PANEL_RECORD,		0xff,	0,		"record"},
	
	{KEYCODE_FROM_PANEL_CHN_PLUS,	0xff,	67,		"chn_add"},//+
	{KEYCODE_FROM_PANEL_CHN_MINUS,	0xff,	94,		"chn_sub"},//-
//	{KEYCODE_FROM_PANEL_VOL_PLUS,	0xff,		67,		"vol_add"},//+
//	{KEYCODE_FROM_PANEL_VOL_MINUS,0xff,		94,		"vol_sub"},//-
//	{67, DATA_VOL_PLUS},
//	{94, DATA_VOL_MINUS},
	
	{KEYCODE_FROM_PANEL_FN,			0xff,	12,		"edit"},//EXTRA//辅助
	{KEYCODE_FROM_PANEL_SHIFT,		0xff,	70,		"shift"},
	
	{KEYCODE_FROM_PANEL_UP,			0xff,	68,		"up"},
	{KEYCODE_FROM_PANEL_DOWN,		0xff,	77,		"down"},
	{KEYCODE_FROM_PANEL_LEFT,		0xff,	11,		"left"},
	{KEYCODE_FROM_PANEL_RIGHT,		0xff,	74,		"right"},
	{KEYCODE_FROM_PANEL_ENTER,		0xff,	72,		"enter"},
	{KEYCODE_FROM_PANEL_ESC,		0xff,	7,		"esc"},
	
	{KEYCODE_FROM_PANEL_1,			0xff,	19,		"CH1"},
	{KEYCODE_FROM_PANEL_2,			0xff,	80,		"CH2"},
	{KEYCODE_FROM_PANEL_3,			0xff,	81,		"CH3"},
	{KEYCODE_FROM_PANEL_4,			0xff,	82,		"CH4"},
};
#endif

/***********面板和遥控的物理键值转换为逻辑键值******/
static SFrontBoardKeyInfo sKeyTablePanelLogicToPhysic[] = 
{
#ifdef longse
	/*逻辑键值,面板物理键值,遥控物理键值,描述*/
//	{KEYCODE_FROM_PANEL_0,			0xff,	0x18,		"num_0"},
//	{KEYCODE_FROM_PANEL_1,			0xff,	0x01,		"num_1"},
//	{KEYCODE_FROM_PANEL_2,			0xff,	0x02,		"num_2"},
//	{KEYCODE_FROM_PANEL_3,			0xff,	0x03,		"num_3"},
//	{KEYCODE_FROM_PANEL_4,			0xff,	0x04,		"num_4"},
//	{KEYCODE_FROM_PANEL_5,			0xff,	0x05,		"num_5"},
//	{KEYCODE_FROM_PANEL_6,			0xff,	0x06,		"num_6"},
//	{KEYCODE_FROM_PANEL_7,			0xff,	0x07,		"num_7"},
//	{KEYCODE_FROM_PANEL_8,			0xff,	0x08,		"num_8"},
//	{KEYCODE_FROM_PANEL_9,			0xff,	0x09,		"num_9"},
	//
	{KEYCODE_FROM_PANEL_CHN_PLUS,	0xff,	0x19,		"chn_add"},//+
	{KEYCODE_FROM_PANEL_CHN_MINUS,	0xff,	0x1c,		"chn_sub"},//-
	{KEYCODE_FROM_PANEL_MULTI,		0x29,	0x14,		"multi"},
	//
	{KEYCODE_FROM_PANEL_UP,			0x16,	0x12,		"up"},
	{KEYCODE_FROM_PANEL_DOWN,		0x13,	0x1d,		"down"},
	{KEYCODE_FROM_PANEL_LEFT,		0x12,	0x0f,		"left"},
	{KEYCODE_FROM_PANEL_RIGHT,		0x14,	0x15,		"right"},
	//
	{KEYCODE_FROM_PANEL_ENTER,		0x10,	0x00,		"enter"},
	{KEYCODE_FROM_PANEL_ESC,		0x1a,	0x1e,		"esc"},
	//
	{KEYCODE_FROM_PANEL_MENU,		0x1d,	0x10,		"menu"},
	{KEYCODE_FROM_PANEL_RECORD,		0x2b,	0x0e,		"record"},
	{KEYCODE_FROM_PANEL_PLAY,		0x1b,	0x11,		"play"},
	{KEYCODE_FROM_PANEL_PLAY,		0x31,	0xff,		"play"},//SEARCH
	{KEYCODE_FROM_PANEL_PTZ,		0x32,	0x16,		"ptz"},
	//
	{KEYCODE_FROM_PANEL_STOP,		0xff,	0x0d,		"stop"},
	{KEYCODE_FROM_PANEL_PAUSE,		0x19,	0x13,		"pause"},
	{KEYCODE_FROM_PANEL_STEP,		0x19,	0x13,		"step"},
	{KEYCODE_FROM_PANEL_PREV,		0xff,	0xff,		"prev"},
	{KEYCODE_FROM_PANEL_NEXT,		0xff,	0xff,		"next"},
	{KEYCODE_FROM_PANEL_FAST,		0x18,	0x54,		"fastforward"},
	{KEYCODE_FROM_PANEL_SLOW,		0x1e,	0x0b,		"slowforward"},
	{KEYCODE_FROM_PANEL_FASTB,		0x17,	0x17,		"fastback"},
	//
	{KEYCODE_FROM_PANEL_FN,			0xff,	0x1f,		"edit"},//EXTRA//辅助
	{KEYCODE_FROM_PANEL_SHIFT,		0xff,	0xff,		"shift"},
	{KEYCODE_FROM_PANEL_CLEAR,		0xff,	0xff,		"clean"},
	{KEYCODE_FROM_PANEL_POWER,		0xff,	0x0c,		"power"},//STANDBY
	{KEYCODE_FROM_PANEL_POWER,		0xff,	0x0a,		"power"},//LOGIN/LOCK
	//
	{KEYCODE_FROM_PANEL_VOIP,		0xff,	0xff,		"voip"},
	{KEYCODE_FROM_PANEL_MUTE,		0xff,	0x1a,		"mute"},//AUDIO
	{KEYCODE_FROM_PANEL_MUTE,		0xff,	0x1b,		"mute"},
	//
	{KEYCODE_FROM_PANEL_0,			0x00,	0x18,		"num_0"},
	{KEYCODE_FROM_PANEL_1,			0x3a,	0x01,		"num_1"},
	{KEYCODE_FROM_PANEL_2,			0x3b,	0x02,		"num_2"},
	{KEYCODE_FROM_PANEL_3,			0x3c,	0x03,		"num_3"},
	{KEYCODE_FROM_PANEL_4,			0x3d,	0x04,		"num_4"},
	{KEYCODE_FROM_PANEL_5,			0x3e,	0x05,		"num_5"},
	{KEYCODE_FROM_PANEL_6,			0x06,	0x06,		"num_6"},
	{KEYCODE_FROM_PANEL_7,			0x07,	0x07,		"num_7"},
	{KEYCODE_FROM_PANEL_8,			0x08,	0x08,		"num_8"},
	{KEYCODE_FROM_PANEL_9,			0x09,	0x09,		"num_9"},
	//{KEYCODE_FROM_PANEL_POWER,	0x11,	0xff,			"power"},//STANDBY
	{KEYCODE_FROM_PANEL_ESC,		0x11,	0xff,		"power"},//STANDBY
	{KEYCODE_FROM_PANEL_MENU,		0x24,	0xff,		"menu"},
	{KEYCODE_FROM_PANEL_ENTER,		0x2d,	0xff,		"enter"},
	//
	{KEYCODE_FROM_PANEL_DEV,		0xff,	0xff,		"dev"},
	//
#else
	/*逻辑键值,面板物理键值,遥控物理键值,描述*/
	{KEYCODE_FROM_PANEL_0,			0xff,	0xff,		"num_0"},
	{KEYCODE_FROM_PANEL_1,			0xff,	0xff,		"num_1"},
	{KEYCODE_FROM_PANEL_2,			0xff,	0xff,		"num_2"},
	{KEYCODE_FROM_PANEL_3,			0xff,	0xff,		"num_3"},
	{KEYCODE_FROM_PANEL_4,			0xff,	0xff,		"num_4"},
	{KEYCODE_FROM_PANEL_5,			0xff,	0xff,		"num_5"},
	{KEYCODE_FROM_PANEL_6,			0xff,	0xff,		"num_6"},
	{KEYCODE_FROM_PANEL_7,			0xff,	0xff,		"num_7"},
	{KEYCODE_FROM_PANEL_8,			0xff,	0xff,		"num_8"},
	{KEYCODE_FROM_PANEL_9,			0xff,	0xff,		"num_9"},
	//
	{KEYCODE_FROM_PANEL_CHN_PLUS,	0xff,	0xff,		"chn_add"},
	{KEYCODE_FROM_PANEL_CHN_MINUS,	0xff,	0xff,		"chn_sub"},
	{KEYCODE_FROM_PANEL_MULTI,		0xff,	0xff,		"multi"},
	//
	{KEYCODE_FROM_PANEL_UP,			0xff,	0xff,		"up"},
	{KEYCODE_FROM_PANEL_DOWN,		0xff,	0xff,		"down"},
	{KEYCODE_FROM_PANEL_LEFT,		0xff,	0xff,		"left"},
	{KEYCODE_FROM_PANEL_RIGHT,		0xff,	0xff,		"right"},
	//
	{KEYCODE_FROM_PANEL_ENTER,		0xff,	0xff,		"enter"},
	{KEYCODE_FROM_PANEL_ESC,		0xff,	0xff,		"esc"},
	//
	{KEYCODE_FROM_PANEL_MENU,		0xff,	0xff,		"menu"},
	{KEYCODE_FROM_PANEL_RECORD,		0xff,	0xff,		"record"},
	{KEYCODE_FROM_PANEL_PLAY,		0xff,	0xff,		"play"},
	{KEYCODE_FROM_PANEL_PTZ,		0xff,	0xff,		"ptz"},
	//
	{KEYCODE_FROM_PANEL_STOP,		0xff,	0xff,		"stop"},
	{KEYCODE_FROM_PANEL_PAUSE,		0xff,	0xff,		"pause"},
	{KEYCODE_FROM_PANEL_STEP,		0xff,	0xff,		"step"},
	{KEYCODE_FROM_PANEL_PREV,		0xff,	0xff,		"prev"},
	{KEYCODE_FROM_PANEL_NEXT,		0xff,	0xff,		"next"},
	{KEYCODE_FROM_PANEL_FAST,		0xff,	0xff,		"fastforward"},
	{KEYCODE_FROM_PANEL_SLOW,		0xff,	0xff,		"slowforward"},
	{KEYCODE_FROM_PANEL_FASTB,		0xff,	0xff,		"fastback"},
	//
	{KEYCODE_FROM_PANEL_FN,			0xff,	0xff,		"edit"},
	{KEYCODE_FROM_PANEL_SHIFT,		0xff,	0xff,		"shift"},
	{KEYCODE_FROM_PANEL_CLEAR,		0xff,	0xff,		"clean"},
	{KEYCODE_FROM_PANEL_POWER,		0xff,	0xff,		"power"},
	//
	{KEYCODE_FROM_PANEL_VOIP,		0xff,	0xff,		"voip"},
	{KEYCODE_FROM_PANEL_MUTE,		0xff,	0xff,		"mute"},
	//
	{KEYCODE_FROM_PANEL_DEV,		0xff,	0xff,		"dev"},
	//
#endif
};


/*******面板逻辑键值转换为界面的键值**********/
static u8 nKeyTablePanelToUI[][2] = 
{
//test
//{KEYCODE_FROM_PANEL_MULTI,	KEYCODE_FOR_UI_FAST},
//{KEYCODE_FROM_PANEL_MENU,	KEYCODE_FOR_UI_2},
//{KEYCODE_FROM_PANEL_ESC,	KEYCODE_FOR_UI_3},
//{KEYCODE_FROM_PANEL_RECORD, KEYCODE_FOR_UI_STEPF},
//{KEYCODE_FROM_PANEL_PLAY,	KEYCODE_FOR_UI_5},
//{KEYCODE_FROM_PANEL_PTZ,	KEYCODE_FOR_UI_SLOW},
//{KEYCODE_FROM_PANEL_PREV,	KEYCODE_FOR_UI_8},
//{KEYCODE_FROM_PANEL_NEXT,	KEYCODE_FOR_UI_9},
//{KEYCODE_FROM_PANEL_FAST,	KEYCODE_FOR_UI_FUNC},
//{KEYCODE_FROM_PANEL_FASTB,	KEYCODE_FOR_UI_SHIFT},
//{KEYCODE_FROM_PANEL_CHN_PLUS,	KEYCODE_FOR_UI_STOP},
//{KEYCODE_FROM_PANEL_CHN_MINUS,	KEYCODE_FOR_UI_PAUSE},
//end*/
{KEYCODE_FROM_PANEL_0,	KEYCODE_FOR_UI_0},
{KEYCODE_FROM_PANEL_1,	KEYCODE_FOR_UI_1},
{KEYCODE_FROM_PANEL_2,	KEYCODE_FOR_UI_2},
{KEYCODE_FROM_PANEL_3,	KEYCODE_FOR_UI_3},
{KEYCODE_FROM_PANEL_4,	KEYCODE_FOR_UI_4},
{KEYCODE_FROM_PANEL_5,	KEYCODE_FOR_UI_5},
{KEYCODE_FROM_PANEL_6,	KEYCODE_FOR_UI_6},
{KEYCODE_FROM_PANEL_7,	KEYCODE_FOR_UI_7},
{KEYCODE_FROM_PANEL_8,	KEYCODE_FOR_UI_8},
{KEYCODE_FROM_PANEL_9,	KEYCODE_FOR_UI_9},
{KEYCODE_FROM_PANEL_10,	KEYCODE_FOR_UI_10},
{KEYCODE_FROM_PANEL_11,	KEYCODE_FOR_UI_11},
{KEYCODE_FROM_PANEL_12,	KEYCODE_FOR_UI_12},
{KEYCODE_FROM_PANEL_13,	KEYCODE_FOR_UI_13},
{KEYCODE_FROM_PANEL_14,	KEYCODE_FOR_UI_14},
{KEYCODE_FROM_PANEL_15,	KEYCODE_FOR_UI_15},
{KEYCODE_FROM_PANEL_16,	KEYCODE_FOR_UI_16},
//
{KEYCODE_FROM_PANEL_CHN_PLUS,	KEYCODE_FOR_UI_CHNPLUS},
{KEYCODE_FROM_PANEL_CHN_MINUS,	KEYCODE_FOR_UI_CHNMINUS},
//
{KEYCODE_FROM_PANEL_10PLUS,	KEYCODE_FOR_UI_10PLUS},
{KEYCODE_FROM_PANEL_1_4,	KEYCODE_FOR_UI_SPLIT4},
{KEYCODE_FROM_PANEL_PIC8,	KEYCODE_FOR_UI_SPLIT8},
{KEYCODE_FROM_PANEL_1_9,	KEYCODE_FOR_UI_SPLIT9},
{KEYCODE_FROM_PANEL_PIC16,	KEYCODE_FOR_UI_SPLIT16},
{KEYCODE_FROM_PANEL_MULTI,	KEYCODE_FOR_UI_SPLIT},
//
{KEYCODE_FROM_PANEL_UP,		KEYCODE_FOR_UI_UP},
{KEYCODE_FROM_PANEL_DOWN,	KEYCODE_FOR_UI_DOWN},
{KEYCODE_FROM_PANEL_LEFT,	KEYCODE_FOR_UI_LEFT},
{KEYCODE_FROM_PANEL_RIGHT,	KEYCODE_FOR_UI_RIGHT},
//
{KEYCODE_FROM_PANEL_ENTER,	KEYCODE_FOR_UI_RET},
{KEYCODE_FROM_PANEL_ESC,	KEYCODE_FOR_UI_ESC},
//
{KEYCODE_FROM_PANEL_MENU,	KEYCODE_FOR_UI_MENU},
{KEYCODE_FROM_PANEL_RECORD,	KEYCODE_FOR_UI_REC},
{KEYCODE_FROM_PANEL_PLAY,	KEYCODE_FOR_UI_PLAY},
{KEYCODE_FROM_PANEL_PTZ,	KEYCODE_FOR_UI_PTZ},
//
{KEYCODE_FROM_PANEL_STOP,	KEYCODE_FOR_UI_STOP},
{KEYCODE_FROM_PANEL_PAUSE,	KEYCODE_FOR_UI_PAUSE},
{KEYCODE_FROM_PANEL_STEP,	KEYCODE_FOR_UI_STEPF},
{KEYCODE_FROM_PANEL_PREV,	KEYCODE_FOR_UI_PREV},
{KEYCODE_FROM_PANEL_NEXT,	KEYCODE_FOR_UI_NEXT},
{KEYCODE_FROM_PANEL_FAST,	KEYCODE_FOR_UI_FAST},
{KEYCODE_FROM_PANEL_SLOW,	KEYCODE_FOR_UI_SLOW},
{KEYCODE_FROM_PANEL_FASTB,	KEYCODE_FOR_UI_BACK},
//
{KEYCODE_FROM_PANEL_FS_OL,	KEYCODE_FOR_UI_LEFT},
{KEYCODE_FROM_PANEL_FS_OR,	KEYCODE_FOR_UI_RIGHT},
//csp modify 20130326
{KEYCODE_FROM_PANEL_FS_IL,	KEYCODE_FOR_UI_UP},
{KEYCODE_FROM_PANEL_FS_IR,	KEYCODE_FOR_UI_DOWN},
//{KEYCODE_FROM_PANEL_FS_IL,	KEYCODE_FOR_UI_PREV},
//{KEYCODE_FROM_PANEL_FS_IR,	KEYCODE_FOR_UI_NEXT},
//
{KEYCODE_FROM_PANEL_FN,		KEYCODE_FOR_UI_FUNC},
{KEYCODE_FROM_PANEL_SHIFT,	KEYCODE_FOR_UI_SHIFT},
{KEYCODE_FROM_PANEL_CLEAR,	KEYCODE_FOR_UI_SHIFT},
{KEYCODE_FROM_PANEL_POWER,	KEYCODE_FOR_UI_SHUT},
//
{KEYCODE_FROM_PANEL_VOIP,	KEYCODE_FOR_UI_VOIP},
{KEYCODE_FROM_PANEL_MUTE,	KEYCODE_FOR_UI_MUTE},
//csp modify
{KEYCODE_FROM_PANEL_SEARCH,	KEYCODE_FOR_UI_SEARCH},
//
};

static BoardCenter g_Center = NULL;
static u32 g_DVR_ID_REAL = 0xffffffff;//主机的实际ID
static u32 g_DVR_ID_SET = 0xff;//遥控设备传下来的ID,默认是255,这样跟大部分的遥控默认ID匹配,可不用设置ID即可使用遥控

BoardCenter BoardInit(void)
{
	SBoardManager* sManager = calloc(1, sizeof(SBoardManager));
	
	//csp modify
	memset(sManager, 0, sizeof(SBoardManager));
	sManager->nKeyIn = 0;
	
	if(0 != sem_init(&sManager->nDataSem, 0, 1))
	{
		free(sManager);
		return NULL;
	}
	
	return (BoardCenter)sManager;
}

s32 AddBoardDevice(BoardCenter center, s8* pDevPath)
{
	if((!center) || (!pDevPath))
	{
		fprintf(stderr, "Invalid param! function:%s\n", __FUNCTION__);
		return -1;
	}
	
	#ifdef longse
	s32 fd = -1;
	if(strcmp(pDevPath,"/dev/ttyAMA2") == 0)//"/dev/ttyAMA2"
	{
		//fd = open (pDevPath, O_RDWR | O_NOCTTY);
		fd = open(pDevPath, O_RDWR | O_NOCTTY | O_NONBLOCK);
		if(fd > 0)
		{
			set_speed(fd, 9600);
			if(set_Parity(fd, 8, 1, 'N') < 0)
			{
				printf("Set Parity Error\n");
				//exit(1);
			}
		}
	}
	else//"/dev/Hi_IR"
	{
		//fd = open(pDevPath, O_RDWR);
		fd = open(pDevPath, O_RDWR | O_NONBLOCK);
	}
	#else
	//s32 fd = open(pDevPath, O_RDWR);
	s32 fd = open(pDevPath, O_RDWR | O_NONBLOCK, 0);
	#endif
	if(fd < 0)
	{
		fprintf(stderr, "open panel device [%s] failed! function:%s\n", pDevPath, __FUNCTION__);
		//exit(1);//csp modify
		return -1;
	}
	else
	{
		fprintf(stderr, "%s:open panel device [%s] success!\n", __FUNCTION__, pDevPath);
	}
	
	SBoardManager* sManager = (SBoardManager*)center;
	if(sManager->nBoardNum == 0xff)
	{
		fprintf(stderr, "panel device has received max number! function:%s\n", __FUNCTION__);
		close(fd);
		return -1;
	}
	sem_wait(&sManager->nDataSem);
	sManager->nBoardFd = realloc(sManager->nBoardFd, (sManager->nBoardNum + 1) * sizeof(s32));
	sManager->nBoardFd[sManager->nBoardNum] = fd;
	sManager->nBoardNum++;
	sem_post(&sManager->nDataSem);
	printf("%s:finish,BoardNum:%d\n", __FUNCTION__, sManager->nBoardNum);
	return 0;
}

/*
void UpdateDvrAddress(BoardCenter center)
{
	if(!center)
	{
		fprintf(stderr, "Invalid param! function:%s\n", __FUNCTION__);
		return;
	}

	static u64 id = 0;
	static u8 flag = 0;
	s32 ret = 0;
	SModConfigSystemParam para;
	memset(&para, 0, sizeof(para));
	while(1)
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_SYSTEM, &para, -1);
		if(ret != 0)
		{
			//fprintf(stderr, "Error! function:%s\n", __FUNCTION__);
			//return;
			usleep(500 * 1000);
		}
		else
		{
			break;
		}
	}
	u64 id2 = (u64)para.nDevId;
	if((id != id2) || (flag == 0))
	{
		SBoardManager* sManager = (SBoardManager*)center;
		memset(sManager->nData, 0, sizeof(sManager->nData));
		u16 tmp = (u16)id2 | 0x8000;
		sManager->nData[0] = tmp >> 8;
		sManager->nData[1] = (u8)tmp;
		printf("Adress data: 0x%02x%02x\n", sManager->nData[0], sManager->nData[1]);
		ret = WriteBoardDevice(center, 2);
		if(ret != 0)
		{
			fprintf(stderr, "Error! function:%s\n", __FUNCTION__);
			return;
		}
		id = id2;
	}
	flag = 1;
}*/

//s32 PhysicToLogicKey(u8 boardtype, s8 * src, s8 * des)
//{
//	return 0;
//}

s32 ChangePhysicKeyToLogicKey(u8 boardtype, s8 * src, s8 * des)
{
	if((!src) || (!des))
	{
		fprintf(stderr, "Invalid param! function:%s\n", __FUNCTION__);
		return -1;
	}

	s32 i = 0, j = 0;
	switch(boardtype)
	{
		case 0: //frontboard
			for(i = 0; i < 32-1; i++)
			{
				if((*(src+i)==0xfd) || (*(src+i)==0xfe))
				{
					if((*(src+i+1)==0xfd) || (*(src+i+1)==0xfe))
					{
						continue;
					}
					if((*(src+i+1)==0xfc) && ((i>=30) || ((*(src+i+2)!=0xfd) && (*(src+i+2)!=0xfe) && (*(src+i+2)!=0xfc))))
					{
						continue;
					}
					
					//csp modify 20130613
					//if(*(src+i+1)==0xfc)
					//{
					//	i++;
					//}
					
					if(*(src+i)==0xfd) //遥控
					{
						//csp modify 20130613
						if(*(src+i+1)==0xfc)
						{
							i++;
						}
						
						printf("遥控器按键值:(0x%02x,%d)\n",*(src+i+1),*(src+i+1));
						for(j = 0; j < sizeof(sKeyTablePanelLogicToPhysic)/sizeof(SFrontBoardKeyInfo); j++)
						{
							if(sKeyTablePanelLogicToPhysic[j].nPhysicKeyRemote == (*(src+i+1)))
							{
								*des = sKeyTablePanelLogicToPhysic[j].nLogicKey;
								*(des+1) = 1;
								break;
							}
						}
						if(j >= sizeof(sKeyTablePanelLogicToPhysic)/sizeof(SFrontBoardKeyInfo))
						{
							*des = 0;
						}
					}
					else //面板
					{
						//csp modify 20130613
						if(*(src+i+1)==0xfc)
						{
							i++;
						}

						printf("yg NVR read frontboard keyvalue: (0x%02x,%d)\n",*(src+i+1),*(src+i+1));
						for(j = 0; j < sizeof(sKeyTablePanelLogicToPhysic)/sizeof(SFrontBoardKeyInfo); j++)
						{
							if(sKeyTablePanelLogicToPhysic[j].nPhysicKeyFront == (*(src+i+1)))
							{
								printf("key 2: %s, value: 0x%x, LogicKey: %d\n", sKeyTablePanelLogicToPhysic[j].nDescription, \
									sKeyTablePanelLogicToPhysic[j].nPhysicKeyFront, \
									sKeyTablePanelLogicToPhysic[j].nLogicKey);
								*des = sKeyTablePanelLogicToPhysic[j].nLogicKey;
								*(des+1) = 0;
								break;
							}
						}
						if(j >= sizeof(sKeyTablePanelLogicToPhysic)/sizeof(SFrontBoardKeyInfo))
						{
							*des = 0;
						}
					}
					
					i++;
				}
				else
				{
					continue;
				}
			}
			break;
		default:
			break;
	}
	
	return 0;
}

s32 ChangeBoardCodeToUiCode(u8 boardtype, s8* src, s8* des)
{
	if((!src) || (!des))
	{
		fprintf(stderr, "Invalid param! function:%s\n", __FUNCTION__);
		return -1;
	}

	s32 i = 0;
	switch(boardtype)
	{
		case 0:	//frontboard
			if(*src == 0)
			{
				*des = 0;
				break;
			}
			while(1)
			{
				if((i++ == 32) || (*src++ == 0))
				{
					break;
				}
			}
			if(i == 33)
			{
				*des = 0;
			}
			else
			{
				u8 tmp = *(src - 2);
				//printf("cw***receive new key : %d\n", tmp);
				for(i = 0; i < sizeof(nKeyTablePanelToUI)/(2 * sizeof(u8)); i++)
				{
					if(nKeyTablePanelToUI[i][0] == tmp)
					{
						*des = nKeyTablePanelToUI[i][1];
						break;
					}
				}
			}
			break;
		default:
			break;
	}
	
	return 0;
}

void ReadBoardDeviceFxn(BoardCenter* center)
{
	if((!center) || (!(*center)))
	{
		return;
	}
	
	printf("$$$$$$$$$$$$$$$$$$ReadBoardDeviceFxn id:%d\n",getpid());
	
	SBoardManager* sManager = (SBoardManager*)*center;
	u8 i = 0;
	s8 tmp[32];
	s32 ret = 0;
#ifndef longse
	#if defined(CHIP_HISI3531) || (defined(CHIP_HISI3521) && !defined(HI3520D)) || defined(_JMV_)// || defined(_JUAN_)
	#else
	s32 ret2 = 0;
	u16 id = 0xffff;
	#endif
	time_t time_power = 0;
#endif
	
	printf("pthread ReadBoardDeviceFxn going ......\n");
	
	while(*center)
	{
		//printf("ReadBoardDeviceFxn - 1\n");
		
		while(!FrontBoardReqEnable()) sleep(1);
		
		//printf("ReadBoardDeviceFxn - 2\n");
		
#if 0//#ifdef longse
		fd_set set;
		FD_ZERO(&set);
		
		int max_fd_num = -1;
		
		for(i = 0; i < sManager->nBoardNum; i++)
		{
			if(sManager->nBoardFd[i] > 0)
			{
				FD_SET(sManager->nBoardFd[i],&set);
				
				if(sManager->nBoardFd[i] > max_fd_num)
				{
					max_fd_num = sManager->nBoardFd[i];
				}
			}
		}
		
		if(max_fd_num < 0)
		{
			usleep(100*1000);
			continue;
		}
		
		//linux平台下timeout会被修改以表示剩余时间,故每次都要重新赋值
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		int rtn = select(max_fd_num+1,&set,NULL,NULL,&timeout);
		if(rtn < 0)
		{
			//printf("x");
			usleep(100*1000);
			continue;
		}
		if(rtn == 0)
		{
			//printf(".");
			usleep(1);
			continue;
		}
#endif
		
		sem_wait(&sManager->nDataSem);
		
		//UpdateDvrAddress(*center);
		
		//printf("ReadBoardDeviceFxn - 3, BoardNum = %d\n", sManager->nBoardNum);
		
		for(i = 0; i < sManager->nBoardNum; i++)
		{
		#if 0//#ifdef longse
			if(sManager->nBoardFd[i] <= 0 || !FD_ISSET(sManager->nBoardFd[i],&set))
			{
				continue;
			}
		#endif
			
		#ifdef longse
			memset(tmp, 0, sizeof(tmp));
			
			if(i == 0)//前面板
			{
				int remain = 10;
				int recvlen = 0;
				while(remain > 0)
				{
					ret = read(sManager->nBoardFd[i], tmp+recvlen, 1);
					if(ret <= 0)
					{
						if(recvlen > 0)
						{
							continue;
						}
						else
						{
							break;
						}
					}
					if(tmp[recvlen] == 0x0A)
					{
						remain -= ret;
						recvlen += ret;
						break;
					}
					remain -= ret;
					recvlen += ret;
				}
				ret = recvlen;
				if(ret < 10 || (tmp[8] != 0x0D || tmp[9] != 0x0A) || (tmp[0] != 0x24))
				{
					if(ret > 0)
					{
						printf("ReadBoardDeviceFxn - 4.2, len = %d, key value : ( ", ret);
						int j = 0;
						for(j=0;j<ret;j++)
						{
							printf("%02x ",tmp[j]);
						}
						printf(")\n");
					}
					
					#if 0
					close(sManager->nBoardFd[i]);
					sManager->nBoardFd[i] = -1;
					sManager->nBoardFd[i] = open("/dev/ttyAMA2", O_RDWR | O_NOCTTY | O_NONBLOCK);
					if(sManager->nBoardFd[i] > 0)
					{
						set_speed(sManager->nBoardFd[i], 9600);
						if(set_Parity(sManager->nBoardFd[i], 8, 1, 'N') < 0)
						{
							printf("Set Parity Error\n");
							//exit(1);
						}
					}
					printf("reopen Panel device\n");
					#endif
					continue;
				}
				
				#if 0
				if(ret > 0)
				{
					printf("ReadBoardDeviceFxn - 4.2, len = %d, key value : ( ", ret);
					int j = 0;
					for(j=0;j<ret;j++)
					{
						printf("%02x ",tmp[j]);
					}
					printf(")\n");
				}
				#endif
				
				u8 key = ((tmp[4] - 0x30) * 16 + (tmp[5] - 0x30)) & 0xff;
				if(key == 0x0 && tmp[1] == 0x32/* && tmp[2] == 0x3F && tmp[3] == 0x3F*/)
				{
					//心跳键
					continue;
				}
				//csp modify 20130319
				if(key == 0xff)
				{
					//心跳键
					continue;
				}
				printf("Panel key : 0x%02x\n", key);
				
				memset(tmp, 0, sizeof(tmp));
				
				s8 *des = tmp;
				int j = 0;
				for(j = 0; j < sizeof(sKeyTablePanelLogicToPhysic)/sizeof(SFrontBoardKeyInfo); j++)
				{
					if(sKeyTablePanelLogicToPhysic[j].nPhysicKeyFront == key)
					{
						*des = sKeyTablePanelLogicToPhysic[j].nLogicKey;
						*(des+1) = 0;
						break;
					}
				}
				if(j >= sizeof(sKeyTablePanelLogicToPhysic)/sizeof(SFrontBoardKeyInfo))
				{
					*des = 0;
				}
				
				*(tmp+1) = 0;
				memset(sManager->nData, 0, sizeof(sManager->nData));
				//memcpy(sManager->nData, &tmp[ret-1], 1);
				if(0 != ChangeBoardCodeToUiCode(0, tmp, sManager->nData))
				{
					printf("error: ChangeBoardCodeToUiCode\n");
				}
				else
				{
					//printf("after translate : [%d] or [0x%02x]\n", sManager->nData[0], sManager->nData[0]);
					sManager->nKeyIn = 1;
				}
			}
			
			if(i == 1)//遥控器
			{
				//printf("ReadBoardDeviceFxn - 4, before read...\n");
				ret = read(sManager->nBoardFd[i], tmp, sizeof(tmp));
				//printf("ReadBoardDeviceFxn - 4.1, ret = %d\n", ret);
				if(ret > 0)
				{
					//printf("ReadBoardDeviceFxn - 4.2, ret = %d, key value : (0x%02x, %d)\n", ret, *tmp, *tmp);
					//printf("ReadBoardDeviceFxn - 4.2, len = %d, key value : ( ", ret);
					//int j = 0;
					//for(j=0;j<ret;j++)
					//{
						//printf("%02x ",tmp[j]);
					//}
					//printf(")\n");
				}
				if(ret <= 0)
				{
					//usleep(1);
					continue;
				}
				
				irkey_info_s irkey_to_user;
				if(ret < sizeof(irkey_info_s))
				{
					close(sManager->nBoardFd[i]);
					sManager->nBoardFd[i] = -1;
					sManager->nBoardFd[i] = open("/dev/Hi_IR", O_RDWR | O_NONBLOCK);
					printf("reopen IR device\n");
					continue;
				}
				memcpy(&irkey_to_user,tmp,sizeof(irkey_info_s));
				if(irkey_to_user.irkey_state_code == HIIR_KEY_UP)
				{
					//printf("IR key up###\n");
					continue;
				}
				//printf("IR key down\n");
				//printf("irkey_datah:0x%08lx, irkey_datal:0x%08lx, irkey_state_code: 0x%lx\n", 
				//	irkey_to_user.irkey_datah, irkey_to_user.irkey_datal, irkey_to_user.irkey_state_code);
				
				//if((irkey_to_user.irkey_datal & 0xffff) != 0xfb04)
				if(((irkey_to_user.irkey_datal & 0xffff) != 0xfb04)	&& ((irkey_to_user.irkey_datal & 0xffff) != 0xf708))
				{
					//遥控器用户码不匹配
					continue;
				}
				
				u8 key = ((irkey_to_user.irkey_datal & 0xffff0000) >> 16) & 0xff;
				printf("IR key : 0x%02x\n", key);
				
				memset(tmp, 0, sizeof(tmp));
				
				s8 *des = tmp;
				int j = 0;
				if((irkey_to_user.irkey_datal & 0xffff) == 0xfb04)
				{
					for(j = 0; j < sizeof(sKeyTablePanelLogicToPhysic)/sizeof(SFrontBoardKeyInfo); j++)
					{
						if(sKeyTablePanelLogicToPhysic[j].nPhysicKeyRemote == key)
						{
							*des = sKeyTablePanelLogicToPhysic[j].nLogicKey;
							*(des+1) = 1;
							break;
						}
					}
					if(j >= sizeof(sKeyTablePanelLogicToPhysic)/sizeof(SFrontBoardKeyInfo))
					{
						*des = 0;
					}
				}
				else
				{
					for(j = 0; j < sizeof(sKeyTableLongIRLogicToPhysic)/sizeof(SFrontBoardKeyInfo); j++)
					{
						if(sKeyTableLongIRLogicToPhysic[j].nPhysicKeyRemote == key)
						{
							*des = sKeyTableLongIRLogicToPhysic[j].nLogicKey;
							*(des+1) = 1;
							break;
						}
					}
					if(j >= sizeof(sKeyTableLongIRLogicToPhysic)/sizeof(SFrontBoardKeyInfo))
					{
						*des = 0;
					}
				}
				
				*(tmp+1) = 0;
				memset(sManager->nData, 0, sizeof(sManager->nData));
				//memcpy(sManager->nData, &tmp[ret-1], 1);
				if(0 != ChangeBoardCodeToUiCode(0, tmp, sManager->nData))
				{
					printf("error: ChangeBoardCodeToUiCode\n");
				}
				else
				{
					//printf("after translate : [%d] or [0x%02x]\n", sManager->nData[0], sManager->nData[0]);
					sManager->nKeyIn = 1;
				}
			}
		#else
			#ifdef HI3520D
			if(i == 1)//遥控器
			{
				//printf("ReadBoardDeviceFxn - 4, before read...\n");
				ret = read(sManager->nBoardFd[i], tmp, sizeof(tmp));
				//printf("ReadBoardDeviceFxn - 4.1, ret = %d\n", ret);
				if(ret > 0)
				{
					//printf("ReadBoardDeviceFxn - 4.2, ret = %d, key value : (0x%02x, %d)\n", ret, *tmp, *tmp);
					//printf("ReadBoardDeviceFxn - 4.2, len = %d, key value : ( ", ret);
					//int j = 0;
					//for(j=0;j<ret;j++)
					//{
						//printf("%02x ",tmp[j]);
					//}
					//printf(")\n");
				}
				if(ret <= 0)
				{
					//usleep(1);
					continue;
				}
				
				irkey_info_s irkey_to_user;
				if(ret < sizeof(irkey_info_s))
				{
					close(sManager->nBoardFd[i]);
					sManager->nBoardFd[i] = -1;
					sManager->nBoardFd[i] = open("/dev/Hi_IR", O_RDWR | O_NONBLOCK, 0);
					printf("reopen IR device\n");
					continue;
				}
				memcpy(&irkey_to_user,tmp,sizeof(irkey_info_s));
				if(irkey_to_user.irkey_state_code == HIIR_KEY_UP)
				{
					//printf("IR key up###\n");
					continue;
				}
				//printf("IR key down\n");
				//printf("irkey_datah:0x%08lx, irkey_datal:0x%08lx, irkey_state_code: 0x%lx\n", 
				//	irkey_to_user.irkey_datah, irkey_to_user.irkey_datal, irkey_to_user.irkey_state_code);
				
				#if 0
				//if((irkey_to_user.irkey_datal & 0xffff) != 0xfb04)
				if(((irkey_to_user.irkey_datal & 0xffff) != 0xfb04)	&& ((irkey_to_user.irkey_datal & 0xffff) != 0xf708))
				{
					//遥控器用户码不匹配
					continue;
				}
				#endif
				
				u8 key = ((irkey_to_user.irkey_datal & 0xffff0000) >> 16) & 0xff;
				printf("IR key : 0x%02x\n", key);
				
				memset(tmp, 0, sizeof(tmp));
				
				s8 *des = tmp;
				int j = 0;
				for(j = 0; j < sizeof(sKeyTablePanelLogicToPhysic)/sizeof(SFrontBoardKeyInfo); j++)
				{
					if(sKeyTablePanelLogicToPhysic[j].nPhysicKeyRemote == key)
					{
						*des = sKeyTablePanelLogicToPhysic[j].nLogicKey;
						*(des+1) = 1;
						break;
					}
				}
				if(j >= sizeof(sKeyTablePanelLogicToPhysic)/sizeof(SFrontBoardKeyInfo))
				{
					*des = 0;
				}
				
				if(*tmp == KEYCODE_FROM_PANEL_DEV)
				{
					id = 0;
					continue;
				}
				else if((id != 0xffff) && (*tmp >= KEYCODE_FROM_PANEL_0) && (*tmp <= KEYCODE_FROM_PANEL_9))
				{
					id = id*10 + *tmp - KEYCODE_FROM_PANEL_0;
					continue;
				}
				else if((id != 0xffff) && (*tmp == KEYCODE_FROM_PANEL_ENTER))
				{
					g_DVR_ID_SET = id;
					printf("func:%s, g_DVR_ID_SET = %d\n", __FUNCTION__, g_DVR_ID_SET);
					continue;
				}
				else
				{
					id = 0xffff;
					if((g_DVR_ID_SET != g_DVR_ID_REAL) && (*(tmp+1)))
					{
						//printf("remote control id is not the dvr id!\n");
						continue;
					}
				}
				
				if(*tmp == KEYCODE_FROM_PANEL_POWER)
				{
					continue;
				}
				time_power = 0;
				
				*(tmp+1) = 0;
				memset(sManager->nData, 0, sizeof(sManager->nData));
				//memcpy(sManager->nData, &tmp[ret-1], 1);
				if(0 != ChangeBoardCodeToUiCode(0, tmp, sManager->nData))
				{
					printf("error: ChangeBoardCodeToUiCode\n");
				}
				else
				{
					//printf("after translate : [%d] or [0x%02x]\n", sManager->nData[0], sManager->nData[0]);
					sManager->nKeyIn = 1;
				}
				
				continue;
			}
			#endif
			
			memset(tmp, 0, sizeof(tmp));
			
			//printf("ReadBoardDeviceFxn - 4, before read...\n");
			ret = read(sManager->nBoardFd[i], tmp, sizeof(tmp));
			//printf("ReadBoardDeviceFxn - 4.1, ret = %d\n", ret);
			if(ret > 0)
			{
				//printf("ReadBoardDeviceFxn - 4.2, ret = %d, key value : (0x%02x, %d)\n", ret, *tmp, *tmp);
				//printf("ReadBoardDeviceFxn - 4.2, len = %d, key value : ( ", ret);
				//int j = 0;
				//for(j=0;j<ret;j++)
				//{
					//printf("%02x ",tmp[j]);
				//}
				//printf(")\n");
			}
			
			#if defined(CHIP_HISI3531) || (defined(CHIP_HISI3521) && !defined(HI3520D)) || defined(_JMV_)// || defined(_JUAN_)
			if(ret <= 0)
			{
				usleep(10*1000);
				continue;
			}
			#else
			if(i == 0)//前面板
			{
				if((ret <= 0) || ((*tmp != 0xfe) && (*tmp != 0xfd)))
				{
					continue;
				}
				while(ret<2)
				{
					usleep(5000);
					ret2 = read(sManager->nBoardFd[i], tmp+ret, sizeof(tmp)-ret);
					//printf("ReadBoardDeviceFxn - 5, ret2 = %d\n", ret2);
					if(ret2 <= 0)
					{
						continue;
					}
					//printf("ret2 = %d\n", ret2);
					ret += ret2;
				}
			}
			#endif
			
			//printf("ReadBoardDeviceFxn - 6, ret = %d\n", ret);
			
			if(ret > 0)
			{
				//printf("panel key:( ");
				//int k = 0;
				//for(k=0;k<ret;k++)
				//{
				//	printf("%02x ",tmp[k]);
				//}
				//printf(")\n");
				
				//printf("receive %d bytes '%x %x' from board!\n", ret,*tmp,*(tmp+1));
				
				if(ret < sizeof(tmp))
				{
					#if defined(CHIP_HISI3531) || (defined(CHIP_HISI3521) && !defined(HI3520D)) || defined(_JMV_)// || defined(_JUAN_)
					u8 key = tmp[ret-1];
					printf("recv panel msg:(0x%02x,0x%02x,%d)\n",key,key&0x7f,key&0x7f);
					key &= 0x7f;
					*tmp = key;
					*(tmp+1) = 0;//面板
					#else
					ChangePhysicKeyToLogicKey(0, tmp, tmp);
					//printf("recv panel msg:(0x%02x,%d)\n",*tmp,*tmp);
					if(*tmp == KEYCODE_FROM_PANEL_DEV)
					{
						id = 0;
						continue;
					}
					else if((id != 0xffff) && (*tmp >= KEYCODE_FROM_PANEL_0) && (*tmp <= KEYCODE_FROM_PANEL_9))
					{
						id = id*10 + *tmp - KEYCODE_FROM_PANEL_0;
						continue;
					}
					else if((id != 0xffff) && (*tmp == KEYCODE_FROM_PANEL_ENTER))
					{
						g_DVR_ID_SET = id;
						printf("func:%s, g_DVR_ID_SET = %d\n", __FUNCTION__, g_DVR_ID_SET);
						continue;
					}
					else
					{
						id = 0xffff;
						if((g_DVR_ID_SET != g_DVR_ID_REAL) && (*(tmp+1)))
						{
							//printf("remote control id is not the dvr id!\n");
							
							continue;
						}
					}
					#endif
					
					if(*tmp == KEYCODE_FROM_PANEL_POWER)
					{
						if(time_power == 0)
						{
							time(&time_power);
							
							#if defined(CHIP_HISI3531) || (defined(CHIP_HISI3521) && !defined(HI3520D)) || defined(_JMV_)// || defined(_JUAN_)
							//sManager->nData[0] = CMD_H8_RESET;
							//sManager->nData[1] = 0x1;
							//WriteBoardDevice(g_Center, 2);
							#else
							//回传power键的物理键值，供面板开机用
							u8 j = 0;
							for(j = 0; j < sizeof(sKeyTablePanelLogicToPhysic)/sizeof(SFrontBoardKeyInfo); j++)
							{
								if(sKeyTablePanelLogicToPhysic[j].nLogicKey == KEYCODE_FROM_PANEL_POWER)
								{
									sManager->nData[0] = FRONTBOARD_CMD_H8_PANEL_POWER;
									sManager->nData[1] = sKeyTablePanelLogicToPhysic[j].nPhysicKeyFront & 0xff;
									WriteBoardDevice(g_Center, 2);
									printf(" NVR power: 0x%02x\n", sManager->nData[1]);
									//why???
									sManager->nData[0] = FRONTBOARD_CMD_H8_IR_POWER;
									sManager->nData[1] = sKeyTablePanelLogicToPhysic[j].nPhysicKeyRemote & 0xff;
									WriteBoardDevice(g_Center, 2);
									break;
								}
							}
							#endif
						}
						if(time(NULL) - time_power < 3)
						{
							continue;
						}
						else if(time(NULL) - time_power > 4)
						{
							time_power = 0;
							continue;
						}
					}
					time_power = 0;
					
					*(tmp+1) = 0;
					memset(sManager->nData, 0, sizeof(sManager->nData));
					//memcpy(sManager->nData, &tmp[ret-1], 1);
					if(0 != ChangeBoardCodeToUiCode(0, tmp, sManager->nData))
					{
						printf("error: ChangeBoardCodeToUiCode\n");
					}
					else
					{
						printf("after translate : [%d] or [0x%02x]\n", sManager->nData[0], sManager->nData[0]);
						sManager->nKeyIn = 1;
					}
				}
			}
		#endif
		}
		
		sem_post(&sManager->nDataSem);
		
		usleep(1000);//usleep(50 * 1000);//csp modify
	}
	
	printf("pthread ReadBoardDeviceFxn ending ......\n");
}

s32 WriteBoardDevice(BoardCenter center, s32 nBytes)
{
#ifdef longse
	return nBytes;
#else
	if((!center) || (nBytes > 4))
	{
		fprintf(stderr, "Invalid param! function:%s\n", __FUNCTION__);
		return -1;
	}
	
	//csp modify
	if(!bEnable)
	{
		printf("WriteBoardDevice error : front board disable\n");
		return -1;
	}
	
	SBoardManager* sManager = (SBoardManager*)center;
	
	u8 i = 0;
	s32 ret = 0;
	for(i = 0; i < sManager->nBoardNum; i++)
	{
		//csp modify 20131001
		if(i != 0)
		{
			continue;
		}
		
		ret = write(sManager->nBoardFd[i], sManager->nData, nBytes);
		if(ret != nBytes)
		{
			if(i == 0 || (i > 0 && ret < 0))
			{
				fprintf(stderr, "Warnning: %s [ID = %d] failed!\n", __FUNCTION__, i+1);
			}
		}
	}
	return ret;
#endif
}

void InitKeyTablePanelLogicToPhysic()
{
	//printf("InitKeyTablePanelLogicToPhysic-0\n");
	
#ifdef longse
	if(0 == access(FILE_PATH_LOGIC_TO_PHYSIC, F_OK))//if(0 != access(FILE_PATH_LOGIC_TO_PHYSIC, F_OK))//csp modify
	{
		//remove(FILE_PATH_LOGIC_TO_PHYSIC);//csp modify
	}
#else
	//printf("InitKeyTablePanelLogicToPhysic-1\n");
	
	#if defined(CHIP_HISI3531) || (defined(CHIP_HISI3521) && !defined(HI3520D)) || defined(_JMV_)// || defined(_JUAN_)
	return;
	#endif
	
	//printf("InitKeyTablePanelLogicToPhysic-2\n");
	
	int i = 0;
	int ret = 0;
	int value = 0;
	char filepath[80] = {0};
	
	strcpy(filepath, FILE_PATH_LOGIC_TO_PHYSIC);	
	if(0 != access(FILE_PATH_LOGIC_TO_PHYSIC, F_OK))
	{
		strcpy(filepath, FILE_PATH_LOGIC_TO_PHYSIC_DEFAULT);
	}
	
	//printf("InitKeyTablePanelLogicToPhysic-3\n");
	
	for(i = 0; i < sizeof(sKeyTablePanelLogicToPhysic)/sizeof(SFrontBoardKeyInfo); i++)
	{
		ret = PublicIniGetValue(filepath, 
								LOGIC_TO_PHYSIC_FOR_FRONTBOARD, 
								sKeyTablePanelLogicToPhysic[i].nDescription, 
								&value);
		if(0 == ret)
		{
			sKeyTablePanelLogicToPhysic[i].nPhysicKeyFront = (u16)value;
		}
		else
		{
			sKeyTablePanelLogicToPhysic[i].nPhysicKeyFront = 0xffff;
		}
		//printf("file: %s, %s=0x%x\n",filepath, sKeyTablePanelLogicToPhysic[i].nDescription,sKeyTablePanelLogicToPhysic[i].nPhysicKeyFront);
		
		ret = PublicIniGetValue(filepath, 
								LOGIC_TO_PHYSIC_FOR_REMOTE, 
								sKeyTablePanelLogicToPhysic[i].nDescription, 
								&value);
		if(0 == ret)
		{
			sKeyTablePanelLogicToPhysic[i].nPhysicKeyRemote = (u16)value;
		}
		else
		{
			sKeyTablePanelLogicToPhysic[i].nPhysicKeyRemote = 0xffff;
		}
	}
	
	if(0 == strcmp(filepath, FILE_PATH_LOGIC_TO_PHYSIC))
	{
		remove(FILE_PATH_LOGIC_TO_PHYSIC);//csp modify
	}
#endif
}

int FrontboardCreate( void )
{
	if((NULL == g_Center) && (NULL == (g_Center = BoardInit())))
	{
		fprintf(stderr, "BoardInit failed! function:%s\n", __FUNCTION__);
		return -1;
	}
	
	int ret = 0;
#ifdef longse
	ret = AddBoardDevice(g_Center, "/dev/ttyAMA2");
	ret = AddBoardDevice(g_Center, "/dev/Hi_IR");
#else
	ret = AddBoardDevice(g_Center, "/dev/panel");
	#ifdef HI3520D
	ret = AddBoardDevice(g_Center, "/dev/Hi_IR");
	#endif
#endif
	if(ret != 0)
	{
		fprintf(stderr, "AddBoardDevice failed! function:%s\n", __FUNCTION__);
	}
	else
	{
		//printf("AddBoardDevice finish\n");
	}
	
	if(ret == 0)
	{
		//printf("InitKeyTablePanelLogicToPhysic start\n");
		InitKeyTablePanelLogicToPhysic();
		
		SBoardManager* sManager = (SBoardManager*)g_Center;
		sManager->nData[0] = 0x82;
		sManager->nData[1] = 0x0;
		WriteBoardDevice(g_Center, 2);
		
		//#ifndef CHIP_HISI3521
		sManager->nData[0] = 0x0;
		sManager->nData[1] = 0x0;
		WriteBoardDevice(g_Center, 2);
		//#endif
		
		//fprintf(stderr, "Start Create thread ... ...  function:%s\n", __FUNCTION__);
		pthread_t pid;
		ret = pthread_create(&pid, NULL, (void*)ReadBoardDeviceFxn, &g_Center);
		//fprintf(stderr, "End Create thread ... ...  function:%s\n", __FUNCTION__);
	}
	
	if(0 != ret)
	{
		SBoardManager* sManager = (SBoardManager*)g_Center;
		free(sManager->nBoardFd);
		sem_destroy(&sManager->nDataSem);//csp modify
		free(sManager);
		g_Center = NULL;
	}
	
	return ret;
}

int FrontboardDestory( void )
{
	SBoardManager* sManager = (SBoardManager*)g_Center;
	free(sManager->nBoardFd);
	//need close fd
	sem_destroy(&sManager->nDataSem);//csp modify
	free(sManager);
	g_Center = NULL;
	return 0;
}

int FrontboardRead( void* pData, uint nBytes )
{
	if((NULL == pData) || (nBytes > 4) || (NULL == g_Center))
	{
		fprintf(stderr, "%s Invalid param,pData=0x%08x,nBytes=%d,g_Center=0x%08x\n", __FUNCTION__, (unsigned int)pData, nBytes, (unsigned int)g_Center);
		return -1;
	}
	
	//csp modify
	if(!bEnable)
	{
		return -1;
	}
	
	SBoardManager* sManager = (SBoardManager*)g_Center;
	if(0 == sManager->nKeyIn)
	{
		//memset(pData, 0, sizeof(sManager->nData));//cw_panel//csp modify
		return -1;
	}
	
	sem_wait(&sManager->nDataSem);
	if(0 == sManager->nKeyIn)
	{
		//memset(pData, 0, sizeof(sManager->nData));//cw_panel//csp modify
		sem_post(&sManager->nDataSem);
		return -1;
	}
	memcpy(pData, sManager->nData, nBytes);
	memset(sManager->nData, 0, sizeof(sManager->nData));
	sManager->nKeyIn = 0;
	sem_post(&sManager->nDataSem);
	
	return 0;
}

int FrontboardWrite( void* pData, uint nBytes )
{
#ifdef longse
	return nBytes;
#else
	if((NULL == pData) || (nBytes > 4) || (NULL == g_Center))
	{
		fprintf(stderr, "Invalid param! function:%s\n", __FUNCTION__);
		return -1;
	}
	
	//csp modify
	if(!bEnable)
	{
		printf("FrontboardWrite error : front board disable\n");
		return -1;
	}
	
	SBoardManager* sManager = (SBoardManager*)g_Center;
	
	//sem_wait(&sManager->nDataSem);//cw_panel
	sem_wait(&sManager->nDataSem);//csp modify
	
	sManager->nKeyIn = 0;//csp modify
	
	memset(sManager->nData, 0, sizeof(sManager->nData));
	memcpy(sManager->nData, pData, nBytes);
#if 0	
	int i;
	for(i=0; i<nBytes; i++)
		printf("\t pData[%d] = %d\n", i, sManager->nData[i]);
#endif	
	s32 ret = WriteBoardDevice(g_Center, nBytes);
	memset(sManager->nData, 0, sizeof(sManager->nData));
	
	//sManager->nKeyIn = 0;//csp modify
	
	//sem_post(&sManager->nDataSem);//cw_panel
	sem_post(&sManager->nDataSem);//csp modify
	
	return ret;
#endif
}

void FrontBoardSetDvrId(u32 nId)
{
	static u8 flag = 1;
	if(flag)
	{
		flag = 0;
		g_DVR_ID_SET = nId;//每次开机后无需重新设置ID即可使用遥控器,不过会造成能控制ID不匹配的机器
	}
	g_DVR_ID_REAL = nId;
	//printf("func:%s, g_DVR_ID_REAL = %d\n", __FUNCTION__, g_DVR_ID_REAL);
}

void FrontBoardGetReUsedKey(u8 key, u8* pKeyNew)
{
	if(pKeyNew)
	{
		*pKeyNew = key;
		u8 i = 0;		
		for(i = 0; i < sizeof(nKeyTablePanelToUI)/(2 * sizeof(u8)); i++)
		{
			if(nKeyTablePanelToUI[i][1] == key)
			{
				u8 j = 0;
				for(j = 0; j < sizeof(sKeyTablePanelLogicToPhysic)/sizeof(SFrontBoardKeyInfo); j++)
				{
					if(sKeyTablePanelLogicToPhysic[j].nLogicKey == nKeyTablePanelToUI[i][0])
					{
						u8 k = 0;
						for(k = j+1; k < sizeof(sKeyTablePanelLogicToPhysic)/sizeof(SFrontBoardKeyInfo); k++)
						{
							if(sKeyTablePanelLogicToPhysic[j].nPhysicKeyFront == sKeyTablePanelLogicToPhysic[k].nPhysicKeyFront)
							{
								u8 x = 0;
								for(x = 0; x < sizeof(nKeyTablePanelToUI)/(2 * sizeof(u8)); x++)
								{
									if(nKeyTablePanelToUI[x][0] == sKeyTablePanelLogicToPhysic[k].nLogicKey)
									{
										*pKeyNew = nKeyTablePanelToUI[x][1];								
										return;
									}
								}
								break;
							}
						}
						break;
					}
				}
				break;
			}
		}
	}
}

//csp modify
//static int bEnable = 1;

void FrontBoardEnable( int bEn )
{
	bEnable = bEn;
}

int FrontBoardReqEnable( void )
{
	return bEnable;
}

