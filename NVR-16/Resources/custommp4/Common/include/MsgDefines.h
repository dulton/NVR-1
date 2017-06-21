//#include "global.h"

#pragma pack(push,1)

#define VSWC_CONTROL_HEADER_LEN                     0x07
#define VSWC_PING_RESPOND_LEN						0x23

#define VSWC_PROTOCOL_VERSION                       0x0100
#define VSWC_VIDEO_FORMAT                           0x01

// --------------------------------------------------------------------------
// ****************              GUID                ****************
// --------------------------------------------------------------------------

#define  		GUID_VIDEO						0x01 // 0x01 1 AD 1 stream 
												// 0x03 1 AD 2 stream
												// 0x04 2 AD 1 stream
												// 0x05 2 AD 2 stream

#define 		GUID_AUDIO_DECODER              0x0A
#define 		GUID_AUDIO_ENCODER              0x09

#define 		GUID_485						0x0C
#define			GUID_232						0x02
#define         GUID_INPUTPIN                   0x06///第一个报警输入
#define			GUID_INPUTPIN2					0x08///第二个报警输入
#define			GUID_INPUTPIN3					0x0B///第三个报警输入
#define         GUID_INPUTPIN4					0x16///第四个报警输入	
#define         GUID_INPUTPIN5					0x18///第五个报警输入	
#define         GUID_INPUTPIN6					0x1B///第六个报警输入	

#define         GUID_OUTPUTPIN					0x07///第一个报警输出
#define         GUID_OUTPUTPIN2					0x17///第二个报警输出 
#define         GUID_OUTPUTPIN3					0x27///第三个报警输出 

#define 		GUID_MOTION_DETECTION 			0x3B
#define 		GUID_PREALERT_STORAGE 			0x3D



// --------------------------------------------------------------------------
// ****************         VSWC Config Value Types          ****************
// --------------------------------------------------------------------------
#define VSWC_VALUE_TYPE_INVALID                     0x00
#define VSWC_VALUE_TYPE_CHAR                        0x01
#define VSWC_VALUE_TYPE_SHORT                       0x02
#define VSWC_VALUE_TYPE_INT                         0x03
#define VSWC_VALUE_TYPE_STRING                      0x04
#define VSWC_VALUE_TYPE_UINT                        0x05
#define VSWC_VALUE_TYPE_GUID                        0x06

// --------------------------------------------------------------------------
// ****************              Proprietary                ****************
// --------------------------------------------------------------------------
#define VSWC_IP_ADDRESS_ID						0x0001	//UNIT		
#define VSWC_SUBNET_ID							0x0002	//UNIT		
#define VSWC_GATEWAY_ID							0x0003	//UNIT		
#define VSWC_NETWORK_SECTIONID                  0x3005

#define VSWC_PORT_ID                            0x0001	//UNIT		
#define VSWC_DISCOVERYIP_ID						0x0003	//UNIT		
#define VSWC_CENTER_SECTIONID                   0x300F

#define VSWC_RATE_CONTROL_MODE_ID               0x0029	//UNIT		
#define VSWC_VENCODER_SECTIONID                 0x3004

#define VSWC_AUDIO_CODEC_MODE_ID                0x0006	//UNIT		0--ppt 1-full duplex
#define VSWC_AENCODER_SECTIONID                 0x3007

// --------------------------------------------------------------------------
// ****************              Capabilities                ****************
// --------------------------------------------------------------------------
#define VSWC_ENTITY_VIDEODECODER                    0x01
#define VSWC_ENTITY_VIDEOENCODER                    0x02
#define VSWC_ENTITY_AUDIODECODER                    0x03
#define VSWC_ENTITY_AUDIOENCODER                    0x04
#define VSWC_ENTITY_SERIALPORT                      0x05
#define VSWC_ENTITY_INPUTPIN                        0x06
#define VSWC_ENTITY_OUTPUTPIN                       0x07
#define VSWC_ENTITY_PTZMOTOR                        0x08////////暂时不用
#define VSWC_ENTITY_PTZCONTROL                      0x09////////暂时不用
#define VSWC_ENTITY_ARCHIVER                        0x0A////////暂时不用
#define VSWC_ENTITY_MOTION_DETECTOR                 0x0D
#define VSWC_ENTITY_PRE_ALERT_STORAGE               0x0E


// --------------------------------------------------------------------------
// ****************              Msg  Defines                ****************
// --------------------------------------------------------------------------

#define VSWC_MSG_TYPE_D_PING_REQ			0x01
#define VSWC_MSG_TYPE_D_PING_RESP			0x02
#define VSWC_MSG_TYPE_IC_CAPABILITIES_REQ	0x03
#define VSWC_MSG_TYPE_IC_CAPABILITIES_RESP	0x04
#define VSWC_MSG_TYPE_MC_STARTDEVICE_REQ	0x05
#define VSWC_MSG_TYPE_MC_STOPDEVICE_REQ		0x06
#define VSWC_MSG_TYPE_MC_SETCONFIG_REQ      0x07

#define VSWC_MSG_TYPE_MC_GETCONFIG_REQ				0x08
#define VSWC_MSG_TYPE_MC_GETCONFIG_RESP				0x09
#define VSWC_MSG_TYPE_MC_SENDCOMMAND_REQ            0x0A
#define VSWC_MSG_TYPE_E_NOTIFY                      0x0B

#define VSWC_MSG_TYPE_MC_SETPROPCONFIG_REQ          0x0E
#define VSWC_MSG_TYPE_MC_GETPROPCONFIG_REQ          0x0F
#define VSWC_MSG_TYPE_MC_GETPROPCONFIG_RESP         0x10

#define VSWC_MSG_TYPE_D_CONTENT_TYPE_SWITCH_REQ		0x13
#define VSWC_MSG_TYPE_D_CONTENT_TYPE_SWITCH_RESP	0x14


#define VSWC_MSG_TYPE_MC_STARTDEVICEEX_REQ			0x15
/*******************************************************/
#define VSWC_MSG_TYPE_FLASHROMUPDATE_REQ			0x19
#define VSWC_MSG_TYPE_WRITEMACANDIP_REQ             0x25 
#define VSWC_MSG_TYPE_TEST_REQ						0x26 
/****************************************************/         

#define VSWC_MSG_TYPE_E_SUBSCRIBE_REQ				0x0C
#define VSWC_MSG_TYPE_ER_ERROR						0x0D



#define VSWC_MSG_TYPE_D_GETCONFIG_REQ               0x70
#define VSWC_MSG_TYPE_D_GETCONFIG_RESP              0x71
#define VSWC_MSG_TYPE_D_SETNETWORK_REQ              0x72
#define VSWC_MSG_TYPE_D_SETSERIAL_REQ               0x73
/*******************************************
* TEST DEFINE
*/
#define VSWC_MSG_TYPE_TEST_232				0X00
#define VSWC_MSG_TYPE_TEST_484				0X01
#define VSWC_MSG_TYPE_TEST_ALARMIN_FIRST	0X02
#define VSWC_MSG_TYPE_TEST_ALARMIN_SECOND	0X03
#define VSWC_MSG_TYPE_TEST_ALARMOUT			0X04
#define VSWC_MSG_TYPE_TEST_START			0X05
#define VSWC_MSG_TYPE_TEST_RESULT			0X06

// --------------------------------------------------------------------------
// ****************              Conn Type  Defines          ****************
// --------------------------------------------------------------------------

#define	VSWC_CONNECTION_NULL			0x00
#define VSWC_CONNECTION_UDP_UNICAST		0x01
#define VSWC_CONNECTION_UDP_MULTICAST	0x02
#define VSWC_CONNECTION_TCPSERVER		0x04
#define VSWC_CONNECTION_TCPCLIENT		0x08
#define VSWC_CONNECTION_TCP				0x0C
#define VSWC_CONNECTION_UDP_BROADCAST	0x10
#define VSWC_CONNECTION_SSL				0x20
#define VSWC_CONNECTION_TCP_VOLATILE	0x8C //(0x80|0x0C)
#define VSWC_CONNECTION_UDP_VOLATILE	0x81 //(0x80|0x01)
#define VSWC_CONNECTION_RTP_VOLATILE	0xC0 //(0x80|0x40)


// --------------------------------------------------------------------------
// ****************              System Config               ****************
// --------------------------------------------------------------------------
#define CONFIG_SYSTEM_REBOOTREQUIRED                0x5001  // (uint) 0=false 1=true
#define CONFIG_SYSTEM_DEVICE_NAME                   0x5002  // (string) R/W user-defined name for device
#define CONFIG_SYSTEM_DEVICE_TYPE                   0x5003  // (string) R manufacturer-defined device type
#define CONFIG_SYSTEM_UTC_DATETIME                  0x5004  // (string) R/W UTC time in system
#define CONFIG_SYSTEM_GMT_OFFSET					0x5005  // (int) GMT offset
#define CONFIG_SYSTEM_COUNTRY_CODE					0x5006  // (uint) R/W country in which the unit is be used
#define CONFIG_SYSTEM_COUNTRY_CODE_CAP				0x5007  // (string) R/O Country code list in the multi-string format


// --------------------------------------------------------------------------
// ****************               Video Config               ****************
// --------------------------------------------------------------------------
#define CONFIG_VIDEO_ATTRIBUT_BRIGHTNESS            0x1001
#define CONFIG_VIDEO_ATTRIBUT_CONTRAST              0x1002
#define CONFIG_VIDEO_ATTRIBUT_GAIN_U                0x1003
#define CONFIG_VIDEO_ATTRIBUT_GAIN_V                0x1004
#define CONFIG_VIDEO_ATTRIBUT_HUE                   0x1005
#define CONFIG_VIDEO_ATTRIBUT_MOTION_ENC            0x1006
#define CONFIG_VIDEO_ATTRIBUT_SATURATION            0x1007

#define CONFIG_VIDEO_COMPRESSION_DATA_FORMAT        0x1011
#define CONFIG_VIDEO_COMPRESSION_BLOCK_REFRESH      0x1012
#define CONFIG_VIDEO_COMPRESSION_FRAME_RATE         0x1013
#define CONFIG_VIDEO_COMPRESSION_OVERLAY_RATE       0x1014
#define CONFIG_VIDEO_COMPRESSION_QUANTIZATION       0x1015
#define CONFIG_VIDEO_COMPRESSION_BIT_RATE           0x1016
#define CONFIG_VIDEO_COMPRESSION_INTRA_INTERVAL     0x1017
#define CONFIG_VIDEO_COMPRESSION_MIN_QUANTIZATION   0x1018


#define CONFIG_VIDEO_INFO_ANALOG_FORMAT             0x1021
#define CONFIG_VIDEO_INFO_SUPPORTED_DATA_FORMAT     0x1022
#define CONFIG_VIDEO_INFO_SOURCE_INPUT              0x1023

// --------------------------------------------------------------------------
// ****************               Audio Config               ****************
// --------------------------------------------------------------------------
#define CONFIG_AUDIO_ATTRIBUT_PITCH                 0x2001
#define CONFIG_AUDIO_ATTRIBUT_VOLUME                0x2002

#define CONFIG_AUDIO_COMPRESSION_INPUT_TYPE         0x2010
#define CONFIG_AUDIO_COMPRESSION_DATA_FORMAT        0x2011////////////////
#define CONFIG_AUDIO_COMPRESSION_SAMPLING_RATE      0x2012///////////////
#define CONFIG_AUDIO_COMPRESSION_CHANNEL            0x2013////////////////
#define CONFIG_AUDIO_COMPRESSION_GAIN               0x2014
#define CONFIG_AUDIO_COMPRESSION_SAMPLE_BITS        0x2015/////////////////

#define CONFIG_AUDIO_INFO_SUPPORTED_DATA_FORMAT     0x2021

#define CONFIG_AUDIO_PLAYMODE                       0x2030
//////////////////////////////////////////////////////////////////////////////////


// --------------------------------------------------------------------------
// ****************              Serial Port Config          ****************
// --------------------------------------------------------------------------
#define CONFIG_SERIAL_PORT_BAUD_RATE                0x3001
#define CONFIG_SERIAL_PORT_DATA_BITS                0x3002
#define CONFIG_SERIAL_PORT_PARITY                   0x3003
#define CONFIG_SERIAL_PORT_STOP_BITS                0x3004
#define CONFIG_SERIAL_PORT_READ_ONCE                0x3005
#define CONFIG_SERIAL_PORT_WRITE_ONCE               0x3006
#define CONFIG_SERIAL_PORT_READ_INTERCHAR_TIMEOUT   0x3007
#define CONFIG_SERIAL_PORT_READ_TOTAL_TIMEOUT       0x3008
#define CONFIG_SERIAL_PORT_LINE_DRIVER              0x3009

#define CONFIG_SERIAL_HANDSHAKE                     0x3011
#define CONFIG_SERIAL_FLOW_SOFTWARE                 0x3012
#define CONFIG_SERIAL_FLOW_CTS                      0x3013
#define CONFIG_SERIAL_FLOW_DSR                      0x3014
#define CONFIG_SERIAL_CONTROL_RTS                   0x3015
#define CONFIG_SERIAL_CONTROL_DTR                   0x3016
#define CONFIG_SERIAL_CONTROL_SOFTWARE              0x3017


// --------------------------------------------------------------------------
// ****************         Motion Detector Config       ****************
// --------------------------------------------------------------------------
#define CONFIG_MOTD_UPPER_THRESHOLD                   0xa001  // (uint)   R/W Number of macroblocks in motion to start an alert
#define CONFIG_MOTD_LOWER_THRESHOLD                   0xa002  // (uint)   R/W Number of macroblocks in motion to stop an alert
#define CONFIG_MOTD_NB_FRAMES                         0xa003  // (uint)   R/W Number of consecutive frames with motion to start/stop an alert
#define CONFIG_MOTD_VECT_LENGTH                       0xa004  // (uint)   R/W Minimal length of a motion vector to be considered in motion
#define CONFIG_MOTD_LEFT_TO_RIGHT                     0xa005  // (uint)   R/W Detect motion from left to right (1:Enabled, 0:Disabled)
#define CONFIG_MOTD_RIGHT_TO_LEFT                     0xa006  // (uint)   R/W Detect motion from right to left (1:Enabled, 0:Disabled)
#define CONFIG_MOTD_UP_TO_DOWN                        0xa007  // (uint)   R/W Detect motion from up to down (1:Enabled, 0:Disabled)
#define CONFIG_MOTD_DOWN_TO_UP                        0xa008  // (uint)   R/W Detect motion from down to up (1:Enabled, 0:Disabled)
#define CONFIG_MOTD_MASK_STRING                       0xa009  // (huge string)   R/W String representing the mask (format:"nbInMask;nbOut;nbIn;nbOut....
							      //" ex:"0;440;440" = first 440 macroblocks out of mask and last 440 in)
#define CONFIG_MOTD_MASK_ENABLED                      0xa00a  // (uint)   R/W 1:Enabled 0:Disabled
#define CONFIG_MOTD_STATE                             0xa00b  // (uinl)   R/W 1:Enabled 0:Disabled
#define CONFIG_MOTD_INPUT_INDEX                       0xa00c  // (uinl)   R/O Video input index (starting at 0)


// --------------------------------------------------------------------------
// ****************         PreAlertStorage Config       ****************
// --------------------------------------------------------------------------
#define CONFIG_PREA_MEMSPACE                          0xb001  // (uint)   R/W Size of pre buffer in kbytes
#define CONFIG_PREA_STATE                             0xb002  // (uint)   R/W 1:Enabled 0:Disabled
#define CONFIG_PREA_RECORDING_STATE                   0xb003  // (uint)   R/W 1:Enabled 0:Disabled
#define CONFIG_PREA_INPUT_INDEX                       0xb004  // (uinl)   R/O Video input index (starting at 0)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



// --------------------------------------------------------------------------
// ****************              IO Pin Config               ****************
// --------------------------------------------------------------------------
#define CONFIG_IO_PIN_STATE                         0x4001  // (uint) R/W: 0=low 1=high 


//******************************************************************************************//
//                                  Parameter Values                     
//******************************************************************************************//

// --------------------------------------------------------------------------
// ****************                Video Formats             ****************
// --------------------------------------------------------------------------
#define VSWC_VIDEO_FORMAT_NONE                 0x00
#define VSWC_VIDEO_FORMAT_CIF                  0x04
#define VSWC_VIDEO_FORMAT_QCIF                 0x08
#define VSWC_VIDEO_FORMAT_2CIF                 0x40
#define VSWC_VIDEO_FORMAT_4CIF                 0x80

// --------------------------------------------------------------------------
// ****************                Video Analog Formats      ****************
// --------------------------------------------------------------------------
#define VSWC_VIDEO_ANALOG_FORMAT_NONE               0x00
#define VSWC_VIDEO_ANALOG_FORMAT_NTSC               0x01
#define VSWC_VIDEO_ANALOG_FORMAT_PAL                0x02

// --------------------------------------------------------------------------
// ****************           Serial Parity Value            ****************
// --------------------------------------------------------------------------
#define VSWC_SERIAL_PARITY_NONE                     'N'
#define VSWC_SERIAL_PARITY_ODD                      'O'
#define VSWC_SERIAL_PARITY_EVEN                     'E'
#define VSWC_SERIAL_PARITY_MARK                     'M'
#define VSWC_SERIAL_PARITY_SPACE                    'S'

// --------------------------------------------------------------------------
// ****************           Serial Line Driver             ****************
// --------------------------------------------------------------------------
#define VSWC_SERIAL_LINE_DRIVER_AUTO                0x00
#define VSWC_SERIAL_LINE_DRIVER_RS232               0x01
#define VSWC_SERIAL_LINE_DRIVER_RS422               0x02



// --------------------------------------------------------------------------
// ****************                  Commands                ****************
// --------------------------------------------------------------------------

// Video commands (VSWC_ENTITY_VIDEOENCODER)
#define VSWC_COMMAND_SEND_KEYFRAME                  0x01  // Argument 1 is the number of Key Frame to send
// Other commands
#define VSWC_COMMAND_PROPRIETARY                    0x02  // For proprietary commands

#define   ID_COMMAND_NONE            0
#define   ID_COMMAND_REBOOT          1
#define   ID_COMMAND_SAVESETTINGS    2
#define   ID_COMMAND_LOADDEFAULT     3


// --------------------------------------------------------------------------
// ****************           Content Type Value             ****************
// --------------------------------------------------------------------------
#define	VSWC_CONTENT_TYPE_NONE                      0x00
#define VSWC_CONTENT_TYPE_VSIP                      0x01
#define VSWC_CONTENT_TYPE_STREAM                    0x02
#define VSWC_CONTENT_TYPE_VSIP_SSL_CLIENT           0x03
#define VSWC_CONTENT_TYPE_VSIP_SSL_SERVER           0x04
#define VSWC_CONTENT_TYPE_STREAM_SSL_CLIENT         0x05
#define VSWC_CONTENT_TYPE_STREAM_SSL_SERVER         0x06

// --------------------------------------------------------------------------
// ****************           VSWC Event Types               ****************
// --------------------------------------------------------------------------
#define VSWC_EVENT_TYPE_INVALID                     0x00    // Invalid
   // Input pin events
#define VSWC_EVENT_TYPE_INPUTPIN_STATECHANGE        0x01    // Input pin state change
#define VSWC_EVENT_TYPE_VIDEOINPUT_STATECHANGE      0x02    // Camera input state change
#define VSWC_EVENT_TYPE_MOTD_ALERTCHANGE            0x03    // Motion Detection alert change
#define VSWC_EVENT_TYPE_TEMP_ALERTCHANGE            0x04    // Temperature alert change
#define VSWC_EVENT_TYPE_MIDTEMP_ALERTCHANGE         0x05    // Temp 1st threshold alert change

// --------------------------------------------------------------------------
// ****************           VSWC Event Arguments           ****************
// --------------------------------------------------------------------------
   // VSWC_EVENT_TYPE_INPUTPIN_STATECHANGE arguments
#define VSWC_EVENTARG_INPUTPIN_STATEON              0x0000    // ON state
#define VSWC_EVENTARG_INPUTPIN_STATEOFF             0x0001    // OFF state

   // VSWC_EVENT_TYPE_VIDEOINPUT_STATECHANGE arguments
#define VSWC_EVENTARG_VIDEOINPUT_STATEOFF           0x0000    // OFF state
#define VSWC_EVENTARG_VIDEOINPUT_STATEON_COLOR      0x0001    // ON with color state
#define VSWC_EVENTARG_VIDEOINPUT_STATEON_NOCOLOR    0x0002    // ON without color state

#define VSWC_EVENTARG_MOTD_ALERT_ON                 0x0000    // ON state
#define VSWC_EVENTARG_MOTD_ALERT_OFF                0x0001    // OFF state

#define VSWC_EVENTARG_TEMP_ALERT_LOW                0x0000    //Temperature below max threshold
#define VSWC_EVENTARG_TEMP_ALERT_HIGH               0x0001    //Temperature over max threshold

#define VSWC_EVENTARG_MIDTEMP_ALERT_LOW             0x0000    //Temperature below 1st threshold
#define VSWC_EVENTARG_MIDTEMP_ALERT_HIGH            0x0001    //Temperature over 1st threshold

typedef struct _VSWC_CONTROL_HEADER
{
	unsigned short			m_Version;
	unsigned char			m_MsgType;
	unsigned short			m_TransNum;
	unsigned short			m_MsgLen;	
}VSWC_CONTROL_HEADER;

typedef struct _VSWC_VIDEO_HEADER
{
	unsigned short			m_SeqNum;
	unsigned short			m_PackLen;
	unsigned char			m_Format;
	unsigned char			m_Info;
}VSWC_VIDEO_HEADER;

typedef struct _VSWC_PING_REQUEST_MSG
{
	VSWC_CONTROL_HEADER 	m_Header;
	unsigned int          	m_CenterIp;
	unsigned short          m_CenterPort;
	unsigned char           m_ConnType;
}VSWC_PING_REQUEST_MSG;

/***************************************************/
typedef struct _VSWC_MACANDIP_REQUEST_MSG
{
	VSWC_CONTROL_HEADER 	m_Header;
	GUID                    m_DeviceGUID;
	unsigned short          m_Mac1;
	unsigned short          m_Mac2;
	unsigned short          m_Mac3;
	unsigned int            m_Ip;
}VSWC_MACANDIP_REQUEST_MSG;

/***************************************************/
typedef struct _VSWC_FLASHROMUPDATE_REQ_MSG
{
	VSWC_CONTROL_HEADER		m_Header;
	GUID					m_DeviceGUID;
	unsigned int			m_updateflag;
}VSWC_FLASHROMUPDATE_REQ_MSG;
/************************************************/

/************************************************/
typedef struct _VSWC_TEST_REQ_MSG
{ 
	VSWC_CONTROL_HEADER		m_Header;
	GUID                    m_DeviceGUID;
	unsigned int			m_cmdflag;
 }VSWC_TEST_REQ_MSG;
/*********************************************/

/*********************************************/
typedef struct _VSWC_TEST_RESP_MSG
{
	unsigned int			TestType;
	unsigned int			TestResp;
	unsigned int			TestResult[3];
}VSWC_TEST_RESP_MSG;
/*********************************************/

typedef struct _VSWC_PING_RESPOND_MSG
{
	VSWC_CONTROL_HEADER 	m_Header;
	unsigned int            m_UnitIP;
	unsigned short          m_VswcPort;
	unsigned char           m_ConnType;
	GUID                    m_UnitGUID;
	unsigned short          m_ManuID;
	unsigned short          m_UnitType;
	unsigned char           m_Status;	
}VSWC_PING_RESPOND_MSG;

struct VSWC_GET_CAPABILITIES_REQUEST_MSG
{
	VSWC_CONTROL_HEADER 	m_Header;	
};

typedef struct _VSWC_CAPABILITY
{
	unsigned char 			m_CapaType;
	GUID					m_GUID;
	unsigned short 			m_ManID;
	unsigned short 			m_Ver;
}VSWC_CAPABILITY;

typedef struct _VSWC_GET_CAPABILITIES_RESPOND_MSG
{
	VSWC_CONTROL_HEADER 	m_Header;	
	unsigned short          m_CapaCount;
	/////////..............list of capas	
}VSWC_GET_CAPABILITIES_RESPOND_MSG;

typedef struct _VSWC_START_CAPA_REQUEST_MSG
{
	VSWC_CONTROL_HEADER 	m_Header;	
	GUID                    m_CapaGuid;
	unsigned  int           m_TargetIP;
	unsigned short          m_TargetPort;
	unsigned short          m_LocalPort;
	unsigned char           m_ConnType;
}VSWC_START_CAPA_REQUEST_MSG;

typedef struct _VSWC_STOP_CAPA_REQUEST_MSG
{
	VSWC_CONTROL_HEADER 	m_Header;
	GUID                    m_capaGUID;	
}VSWC_STOP_CAPA_REQUEST_MSG;

typedef struct _VSWC_EVENT_SUBSCRIBE_REQUEST_MSG
{
	VSWC_CONTROL_HEADER 	m_Header;
	unsigned int            m_AlarmCenterIP;
	unsigned short          m_AlarmPort;
	unsigned char           m_ConnType;
}VSWC_EVENT_SUBSCRIBE_REQUEST_MSG;
//////////////////////////////////////////////////////////
//·	Start Device 
//·	Start Device Extended 
//·	Stop Device 
//·	Stop Device Extended 
//·	Set Configuration 
//·	Set Proprietary Configuration 
//·	Send Command 
//·	Send Command Extended
//·	Event Subscribe 
//·	Event Subscribe Extended
////////////////////////////////////////////////////////

typedef struct _VSWC_ERROR_RESPOND_MSG
{
	VSWC_CONTROL_HEADER			m_Header;
	unsigned char				m_ReqType;
	unsigned int				m_ErrorCode;	////0 - success   1- fail
}VSWC_ERROR_RESPOND_MSG;

typedef struct _VSWC_MEDIA_CONTROL_GETCONFIGREQUEST_MSG
{
	VSWC_CONTROL_HEADER			VSWCHeader;
	GUID						guid;
	unsigned short				ListCount;
	unsigned short				ConfigId;
	
}VSWC_MEDIA_CONTROL_GETCONFIGREQUEST_MSG;

typedef struct _CONFIG_DATA_STRING
{
	unsigned short				ValueSize;
	unsigned char				*pszValue;
	
}CONFIG_DATA_STRING, *PCONFIG_DATA_STRING;   

union CONFIG_DATA
{
	unsigned char				cValue;
	unsigned short				sValue;
	unsigned int				uiValue;
	int							iValue;   
	CONFIG_DATA_STRING			szValue;
};

typedef struct _VSWC_MEDIA_CONTROL_CONFIG
{
	unsigned short				Id;
	unsigned char				ValueType;
	////////////////////////config.......
	
}VSWC_MEDIA_CONTROL_CONFIG;

typedef struct _VSWC_MEDIA_CONTROL_GETCONFIGRESPONSE_MSG
{
	VSWC_CONTROL_HEADER			m_Header;
	GUID						guid;
	unsigned short				ListCount;
	VSWC_MEDIA_CONTROL_CONFIG	ConfigList[1];
	
}VSWC_MEDIA_CONTROL_GETCONFIGRESPONSE_MSG;

typedef struct _VSWC_MEDIA_CONTROL_PROPRIETARY_CONFIG
{
	unsigned short			Id;
	unsigned char			ValueType;
	//////////////CONFIG_DATA
	////.....................
	unsigned short			SectionId;
	unsigned int			NameSize;
	unsigned char*			pszName;
	unsigned int			HelpStringSize;
	unsigned char*			pszHelpString;
	
}VSWC_MEDIA_CONTROL_PROPRIETARY_CONFIG;

typedef struct _VSWC_MEDIA_CONTROL_GETPROPCONFIGREQUEST_MSG
{
   VSWC_CONTROL_HEADER		VSWCHeader;
   GUID              		guid;
   unsigned short    		ListCount;
   VSWC_MEDIA_CONTROL_PROPRIETARY_CONFIG ConfigList[1];

}VSWC_MEDIA_CONTROL_GETPROPCONFIGREQUEST_MSG;

typedef struct _VSWC_MEDIA_CONTROL_GETPROPCONFIGRESPONSE_MSG
{
   VSWC_CONTROL_HEADER		VSWCHeader;
   GUID              		guid;
   unsigned short    		ListCount;
   VSWC_MEDIA_CONTROL_PROPRIETARY_CONFIG ConfigList[1];

}VSWC_MEDIA_CONTROL_GETPROPCONFIGRESPONSE_MSG;

typedef struct _VSWC_MEDIA_CONTROL_SENDCOMMAND_MSG
{
   VSWC_CONTROL_HEADER    	VSWCHeader;
   GUID           			guid;
   unsigned char  			CommandCode;
   long           			Arg1;
   long           			Arg2;

}VSWC_MEDIA_CONTROL_SENDCOMMAND_MSG;

typedef struct _VSWC_EVENT_NOTIFY_MSG
{
   VSWC_CONTROL_HEADER    	VSWCHeader;
   GUID           			guid;
   unsigned short 			EventType;
   unsigned int   			EventArgument;

}VSWC_EVENT_NOTIFY_MSG;

typedef struct _VSWC_SETCONFIGREQUEST_MSG
{
   VSWC_CONTROL_HEADER		VSWCHeader;
   GUID              		guid;
   unsigned short    		ListCount;
}VSWC_SETCONFIGREQUEST_HEAD_MSG;

typedef struct _VSWC_SETPROPCONFIGREQUEST_HEAD_MSG
{
	VSWC_CONTROL_HEADER		VSWCHeader;
	GUID              		guid;
	unsigned short    		ListCount;
}VSWC_SETPROPCONFIGREQUEST_HEAD_MSG;

typedef struct _VSWC_MEDIA_CONTROL_STARTDEVICEREQUESTEX_MSG
{
   VSWC_CONTROL_HEADER		VSWCHeader;
   GUID						guid;
   unsigned int				TxIpAddress;
   unsigned short			TxPort;
   unsigned short			RxPort;
   unsigned char			ConnectionType;
   GUID						TargetGUID;

}VSWC_MEDIA_CONTROL_STARTDEVICEREQUESTEX_MSG;

typedef struct _VSWC_D_CONTENTTYPE_SWITCHREQUEST_MSG
{
   	VSWC_CONTROL_HEADER    	VSWCHeader;
	GUID			  		SourceGUID;
	unsigned char	  		ContentType;
	GUID			  		TargetGUID;
}VSWC_D_CONTENTTYPE_SWITCHREQUEST_MSG;

///////////////////////////////////////////////////////////////////////////////
typedef struct _VSWC_D_GET_CONFIG_REQ
{ 
	VSWC_CONTROL_HEADER		VSWCHeader;
	GUID                	m_DeviceGUID;
}VSWC_D_GET_CONFIG_REQ, *PVSWC_D_GET_CONFIG_REQ;

#if 0
//原来的定义
typedef struct _VSWC_D_GET_CONFIG_RESP
{
	VSWC_CONTROL_HEADER		VSWCHeader;
	GUID                	m_DeviceGUID;
	unsigned char			m_DeviceType;
	unsigned int        	m_Mask;
	unsigned int        	m_Gate;
	unsigned int        	m_CenterIP;
	unsigned short      	m_485Baud;
	unsigned short      	m_232Baud;
}VSWC_D_GET_CONFIG_RESP, *PVSWC_D_GET_CONFIG_RESP;
#else
//csp modify
typedef struct _VSWC_D_GET_CONFIG_RESP
{
	VSWC_CONTROL_HEADER		VSWCHeader;
	
	//下面两个成员的顺序与原来定义的相反
	unsigned char			m_DeviceType;
	GUID                	m_DeviceGUID;
	
	unsigned int        	m_Mask;
	unsigned int        	m_Gate;
	unsigned int        	m_CenterIP;
	unsigned short      	m_485Baud;
	unsigned short      	m_232Baud;
}VSWC_D_GET_CONFIG_RESP, *PVSWC_D_GET_CONFIG_RESP;
#endif

typedef struct _VSWC_D_SET_NETWORK_REQ
{
	VSWC_CONTROL_HEADER		VSWCHeader;
	GUID                	m_DeviceGUID;
	unsigned int        	m_Mask;
	unsigned int        	m_Gate;
	unsigned int        	m_UnitIP;	
	unsigned int        	m_CenterIP;
}VSWC_D_SET_NETWORK_REQ, *PVSWC_D_SET_NETWORK_REQ;

typedef struct _VSIP_D_SET_SERIAL_REQ
{
	VSWC_CONTROL_HEADER		VSWCHeader;
	GUID					m_DeviceGUID;
	unsigned char      		m_SerialType;
	unsigned short			m_Baud;
}VSWC_D_SET_SERIAL_REQ, *PVSWC_D_SET_SERIAL_REQ;
//////////////////////////////////////////////////////////////////////////////

#pragma pack( pop )
