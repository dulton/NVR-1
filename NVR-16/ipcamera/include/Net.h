/******************************************************************************
* ÏµÍ³Ãû³Æ£ºNet
* ÎÄ¼şÃû³Æ£ºNet.h
* °æ±¾    £º	V2.0.0.0.2
* ËµÃ÷	  £ºÍøÂç¿ª·¢°ü½Ó¿Ú,±¾SDKÎªºÍ±àÂëÆ÷½»»¥Ìá¹©½Ó¿Ú
			¸ÃÎÄ¼ş°üº¬Áù²¿·Ö£º
			Ò»¡¢Ã¶¾ÙÀàĞÍ¶¨Òå£»
			¶ş¡¢»Øµ÷º¯Êı£»
			Èı¡¢½Ó¿Ú½á¹¹ÀàĞÍ¶¨Òå£»
			ËÄ¡¢ÒôÊÓÆµÍøÂçÖ¡Í·£»
			Îå¡¢º¯Êı½Ó¿Ú¶¨Òå£»
* ÆäËûËµÃ÷: ÎŞ
******************************************************************************/
#ifndef  __NET_H__
#define  __NET_H__

#define PACKED  __attribute__((packed, aligned(1)))
#define PACKED4 __attribute__((packed, aligned(4)))


/*********************************  Ò»¡¢Ã¶¾ÙÀàĞÍ¶¨Òå  ******************************/

//1£¬Êı×ÖÊÓÆµ·şÎñÆ÷»úÆ÷ÀàĞÍ
typedef enum	DVS_MACHINE_TYPE_
{
	NONE = 0xff,
}DVS_MACHINE_TYPE;

//2£¬ÊÓÆµ±àÂë¸ñÊ½									
typedef enum  _ENCODE_VIDEO_TYPE
{
	EV_TYPE_NONE		= 0xFFFF,
}ENCODE_VIDEO_TYPE;

//3£¬Éı¼¶ÀàĞÍ
typedef enum _UPDATE_TYPE
{
	UPDATE_KERNEL,					//Éı¼¶ÄÚºË
	UPDATE_YUNTAI1,				//Éı¼¶ÔÆÌ¨Ğ­Òé1
	UPDATE_YUNTAI2,				//Éı¼¶ÔÆÌ¨Ğ­Òé2
	UPDATE_YUNTAI3,				//Éı¼¶ÔÆÌ¨Ğ­Òé3
	UPDATE_YUNTAI4,				//Éı¼¶ÔÆÌ¨Ğ­Òé4
	UPDATE_YUNTAI5,				//Éı¼¶ÔÆÌ¨Ğ­Òé5
	UPDATE_YUNTAI6,				//Éı¼¶ÔÆÌ¨Ğ­Òé6
	UPDATE_OCX,						//Éı¼¶¿Ø¼ş
	UPDATE_WEBPAGE,				//Éı¼¶Ò³Ãæ
	UPDATE_PATHFILE,				//Éı¼¶ÌØ¶¨Ä¿Â¼ÎÄ¼ş
}UPDATE_TYPE;


//5£¬Á¬½Ó×´Ì¬
typedef enum _CONNECT_STATUS
{
	CONNECT_STATUS_NONE,			//Î´Á¬½Ó
	CONNECT_STATUS_OK,				//ÒÑ¾­Á¬½Ó
}CONNECT_STATUS;

//6£¬×ª·¢ÀàĞÍ
typedef enum _RELAY_TYPE
{
	RELAY_LOGON,					//×ª·¢µÇÂ¼ÉèÖÃ
	RELAY_PREVIEW,					//×ª·¢ÊÓÆµÔ¤ÀÀÊı¾İ
	RELAY_TALK,						//×ª·¢¶Ô½²
	RELAY_AUDIOBRD,				//×ª·¢ÓïÒô¹ã²¥
	RELAY_QUERY						//×ª·¢µÄ²éÑ¯
}RELAY_TYPE;

//7£¬Í¨ÖªÓ¦ÓÃ³ÌĞò´°¿ÚÏûÏ¢ÃüÁî
typedef enum _MSG_NOTIFY
{

    MSG_CONNECT_CLOSE,             //µÇÂ¼Á¬½Ó¹Ø±Õ 
    MSG_CHANNEL_CLOSE,             //Í¨µÀÁ¬½Ó¹Ø±Õ 
    MSG_TALK_CLOSE,                //¶Ô½²Á¬½Ó¹Ø±Õ 
    MSG_ALARM_OUTPUT,              //±¨¾¯Êä³ö                   
    MSG_UPDATE_SEND_PERCENT,       //Éı¼¶³ÌĞò·¢ËÍ°Ù·Ö±È 
    MSG_UPDATE_SAVE_PERCENT,       //Éı¼¶Ğ´Èë·¢ËÍ°Ù·Ö±È
    MSG_VIEWPUSH_CLOSE ,           //½âÂëÆ÷ÊÓÆµÊäÈë¹Ø±Õ 
   	MSG_BROADCAST_ADD_FAILURE,     //¼ÓÈëÓïÒô¹ã²¥×éÊ§°Ü 
   	MSG_BROADCAST_CLOSE,           //ÓïÒô¹ã²¥ÖĞÒ»¸ö¶Ï¿ª 
   	MSG_SENSOR_CAPTURE,            //Ì½Í·´¥·¢µÄ×¥ÅÄ 
   	MSG_COM_DATA,                  //´®¿Ú²É¼¯Êı¾İ 
   	MSG_ALARM_LOST,                //±¨¾¯ÏûÊ§                   
    MSG_ALARM_OUTPUT_NEW,          //±¨¾¯Êä³ö(ĞÂ)               
    MSG_ALARM_LOST_NEW,            //±¨¾¯ÏûÊ§(ĞÂ)               
    MSG_PICCHN_CLOSE,              //×¥ÅÄÍ¨µÀÁ¬½Ó¹Ø±Õ 
}MSG_NOTIFY;


//8£¬´íÎóÂë
typedef enum _ERR_CODE
{
	ERR_SUCCESS,					//²Ù×÷³É¹¦
	ERR_FAILURE,					//²Ù×÷Ê§°Ü
	ERR_REFUSE_REQ,				//ÇëÇó±»¾Ü¾ø
	ERR_USER_FULL,					//µÇÂ¼ÓÃ»§ÒÑÂú
	ERR_PREVIEW_FULL,				//Ô¤ÀÀÓÃ»§ÒÑÂú
	ERR_TASK_FULL,					//ÏµÍ³ÈÎÎñ·±Ã¦£¬´ı»á³¢ÊÔÁ¬½Ó
	ERR_CHANNEL_NOT_EXIST,			//Òª´ò¿ªµÄÍ¨µÀ²»´æÔÚ»òÒÑÂú
	ERR_DEVICE_NAME,				//´ò¿ªµÄÉè±¸²»´æÔÚ
	ERR_IS_TALKING,				//ÕıÔÚ¶Ô½²
	ERR_QUEUE_FAILUE,				//¶ÓÁĞ³ö´í
	ERR_USER_PASSWORD,				//ÓÃ»§Ãû»òÃÜÂëºÍÏµÍ³²»Æ¥Åä
	ERR_SHARE_SOCKET,				//socket ´íÎó
	ERR_RELAY_NOT_OPEN,			//×ª·¢ÇëÇóµÄ·şÎñ»¹Î´´ò¿ª
	ERR_RELAY_MULTI_PORT,			//×ª·¢¶à²¥¶Ë¿Ú´íÎó
	ERR_VIEWPUSH_CHANNEL_USING,	//ÊÓÆµÊäÈëµÄÍ¨µÀÒÑ¾­±»Õ¼ÓÃ
	ERR_VIEWPUSH_DECODE_TYPE,		//ÊÓÆµÊäÈëÍ¨µÀµÄ½âÂë¸ñÊ½´íÎó£¬0Í¨µÀ(4cif,2cif,cif),1Í¨µÀ(2cif,cif),2Í¨µÀ(cif),3Í¨µÀ(cif)
	ERR_AUTO_LINK_FAILURE,			//×ª·¢µÄ×Ô¶¯Á¬½ÓÊ§°Ü
	ERR_NOT_LOGON,
	ERR_IS_SETTING,
	ERR_COMMAND_FAILURE,
	
	ERR_INVALID_PARAMETER=100,		//ÊäÈë²ÎÊıÎŞĞ§
	ERR_LOGON_FAILURE,				//µÇÂ¼Ê§°Ü
	ERR_TIME_OUT,					//²Ù×÷³¬Ê±
	ERR_SOCKET_ERR,				//SOCKET´íÎó
	ERR_NOT_LINKSERVER,			//»¹Î´Á¬½Ó·şÎñÆ÷
	ERR_BUFFER_EXTCEED_LIMIT,		//Ê¹ÓÃ»º³å³¬¹ıÏŞÖÆ	
	ERR_LOW_PRIORITY,				//²Ù×÷È¨ÏŞ²»×ã
	ERR_BUFFER_SMALL,				//»º³åÌ«Ğ¡
	ERR_IS_BUSY,					//ÏµÍ³ÈÎÎñÕıÃ¦
	ERR_UPDATE_FILE,				//Éı¼¶ÎÄ¼ş´íÎó
	ERR_UPDATE_UNMATCH,			//Éı¼¶ÎÄ¼şºÍ»úÆ÷²»Æ¥Åä
	ERR_PORT_INUSE,				//¶Ë¿Ú±»Õ¼ÓÃ
	ERR_RELAY_DEVICE_EXIST,		//Éè±¸ÃûÒÑ¾­´æÔÚ
	ERR_CONNECT_REFUSED,			//Á¬½ÓÊ±±»¾Ü¾ø
	ERR_PROT_NOT_SURPPORT,			//²»Ö§³Ö¸ÃĞ­Òé

	ERR_FILE_OPEN_ERR,            //´ò¿ªÎÄ¼şÊ§°Ü
	ERR_FILE_SEEK_ERR,            //fseekÊ§°Ü 
	ERR_FILE_WRITE_ERR,           //fwriteÊ§°Ü 
	ERR_FILE_READ_ERR,            //freadÊ§°Ü 
	ERR_FILE_CLOSING,             //ÕıÔÚ¹Ø±ÕÎÄ¼ş 
	
}ERR_CODE;

//9£¬²ÎÊı²Ù×÷ÃüÁî
typedef enum _CMD_NET
{
	//±àÂëÆ÷ÃüÁî
	CMD_GET_ALL_PARAMETER,			//µÃµ½ËùÓĞ±àÂëÆ÷²ÎÊı
	CMD_SET_DEFAULT_PARAMETER,	//»Ö¸´ËùÓĞ±àÂëÆ÷Ä¬ÈÏ²ÎÊı
	CMD_SET_RESTART_DVS,			//ÖØÆô±àÂëÆ÷
	CMD_GET_SYS_CONFIG,			//»ñÈ¡ÏµÍ³ÉèÖÃ
	CMD_SET_SYS_CONFIG,			//ÉèÖÃÏµÍ³ÉèÖÃ
	CMD_GET_TIME,					//»ñÈ¡±àÂëÆ÷Ê±¼ä
	CMD_SET_TIME,					//ÉèÖÃ±àÂëÆ÷Ê±¼ä
	CMD_GET_AUDIO_CONFIG,			//»ñÈ¡ÒôÆµÉèÖÃ
	CMD_SET_AUDIO_CONFIG,			//ÉèÖÃÒôÆµÉèÖÃ
	CMD_GET_VIDEO_CONFIG,			//»ñÈ¡ÊÓÆµÉèÖÃ
	CMD_SET_VIDEO_CONFIG,			//ÉèÖÃÊÓÆµÉèÖÃ
	CMD_GET_VMOTION_CONFIG,		//»ñÈ¡ÒÆ¶¯Õì²âÉèÖÃ
	CMD_SET_VMOTION_CONFIG,		//ÉèÖÃÒÆ¶¯Õì²âÉèÖÃ
	CMD_GET_VMASK_CONFIG,			//»ñÈ¡Í¼ÏñÆÁ±ÎÉèÖÃ
	CMD_SET_VMASK_CONFIG,			//ÉèÖÃÍ¼ÏñÆÁ±ÎÉèÖÃ
	CMD_GET_VLOST_CONFIG,			//»ñÈ¡ÊÓÆµ¶ªÊ§ÉèÖÃ
	CMD_SET_VLOST_CONFIG,			//ÉèÖÃÊÓÆµ¶ªÊ§ÉèÖÃ
	CMD_GET_SENSOR_ALARM,			//»ñÈ¡Ì½Í·±¨¾¯Õì²âÉèÖÃ
	CMD_SET_SENSOR_ALARM,			//ÉèÖÃÌ½Í·±¨¾¯Õì²âÉèÖÃ
	CMD_GET_USER_CONFIG,			//»ñÈ¡ÓÃ»§ÉèÖÃ
	CMD_SET_USER_CONFIG,			//ÉèÖÃÓÃ»§ÉèÖÃ
	CMD_GET_NET_CONFIG,			//»ñÈ¡ÍøÂçÉèÖÃ½á¹¹
	CMD_SET_NET_CONFIG,			//ÉèÖÃÍøÂçÉèÖÃ½á¹¹
	CMD_GET_COM_CONFIG,			//»ñÈ¡´®¿ÚÉèÖÃ
	CMD_SET_COM_CONFIG,			//ÉèÖÃ´®¿ÚÉèÖÃ
	CMD_GET_YUNTAI_CONFIG,			//»ñÈ¡ÄÚÖÃÔÆÌ¨ĞÅÏ¢
	CMD_SET_YUNTAI_CONFIG,			//ÉèÖÃÄÚÖÃÔÆÌ¨ĞÅÏ¢
	CMD_GET_VIDEO_SIGNAL_CONFIG,	//»ñÈ¡ÊÓÆµĞÅºÅ²ÎÊı£¨ÁÁ¶È¡¢É«¶È¡¢¶Ô±È¶È¡¢±¥ºÍ¶È£©
	CMD_SET_VIDEO_SIGNAL_CONFIG,	//ÉèÖÃÊÓÆµĞÅºÅ²ÎÊı£¨ÁÁ¶È¡¢É«¶È¡¢¶Ô±È¶È¡¢±¥ºÍ¶È£©
	CMD_SET_PAN_CTRL,				//ÔÆÌ¨¿ØÖÆ
	CMD_SET_COMM_SENDDATA,			//Í¸Ã÷Êı¾İ´«Êä
	CMD_SET_COMM_START_GETDATA,	//¿ªÊ¼²É¼¯Í¸Ã÷Êı¾İ
	CMD_SET_COMM_STOP_GETDATA,	//Í£Ö¹²É¼¯Í¸Ã÷Êı¾İ
	CMD_SET_OUTPUT_CTRL,			//¼ÌµçÆ÷¿ØÖÆ
	CMD_SET_PRINT_DEBUG,			//µ÷ÊÔĞÅÏ¢¿ª¹Ø
	CMD_SET_ALARM_CLEAR,			//Çå³ı±¨¾¯
	CMD_GET_ALARM_INFO,			//»ñÈ¡±¨¾¯×´Ì¬ºÍ¼ÌµçÆ÷×´Ì¬
	CMD_SET_TW2824,				//ÉèÖÃ¶à»­ÃæĞ¾Æ¬²ÎÊı(±£Áô)
	CMD_SET_SAVE_PARAM,			//ÉèÖÃ±£´æ²ÎÊı
	CMD_GET_USERINFO,				//»ñÈ¡µ±Ç°µÇÂ½µÄÓÃ»§ĞÅÏ¢
	CMD_GET_DDNS,					//»ñÈ¡DDNS
	CMD_SET_DDNS,					//ÉèÖÃDDNS
	CMD_GET_CAPTURE_PIC,			//Ç°¶Ë×¥ÅÄ
	CMD_GET_SENSOR_CAP,			//»ñÈ¡´¥·¢×¥ÅÄÉèÖÃ
	CMD_SET_SENSOR_CAP,			//ÉèÖÃ´¥·¢×¥ÅÄÉèÖÃ
	CMD_GET_EXTINFO,				//»ñÈ¡À©Õ¹ÅäÖÃ
	CMD_SET_EXTINFO,				//ÉèÖÃÀ©Õ¹ÅäÖÃ
	CMD_GET_USERDATA,				//»ñÈ¡ÓÃ»§ÅäÖÃ
	CMD_SET_USERDATA,				//ÉèÖÃÓÃ»§ÅäÖÃ
	CMD_GET_NTP,					//»ñÈ¡NTPÅäÖÃ
	CMD_SET_NTP,					//ÉèÖÃNTPÅäÖÃ
	CMD_GET_UPNP,					//»ñÈ¡UPNPÅäÖÃ
	CMD_SET_UPNP,					//ÉèÖÃUPNPÅäÖÃ
	CMD_GET_MAIL,					//»ñÈ¡MAILÅäÖÃ
	CMD_SET_MAIL,					//ÉèÖÃMAILÅäÖÃ
	CMD_GET_ALARMNAME,				//»ñÈ¡±¨¾¯ÃûÅäÖÃ
	CMD_SET_ALARMNAME,				//ÉèÖÃ±¨¾¯ÃûÅäÖÃ
	CMD_GET_WFNET,					//»ñÈ¡ÎŞÏßÍøÂçÅäÖÃ
	CMD_SET_WFNET,					//ÉèÖÃÎŞÏßÍøÂçÅäÖÃ
	CMD_GET_SEND_DEST,				//ÉèÖÃÊÓÆµ¶¨Ïò·¢ËÍÄ¿±ê»ú
	CMD_SET_SEND_DEST,				//ÉèÖÃÊÓÆµ¶¨Ïò·¢ËÍÄ¿±ê»ú
	CMD_GET_AUTO_RESET,			//È¡µÃ¶¨Ê±ÖØĞÂ×¢²á
	CMD_SET_AUTO_RESET,			//ÉèÖÃ¶¨Ê±ÖØĞÂ×¢²á
	CMD_GET_REC_SCHEDULE,			//È¡µÃÂ¼Ïñ²ßÂÔ
	CMD_SET_REC_SCHEDULE,			//ÉèÖÃÂ¼Ïñ²ßÂÔ
	CMD_GET_DISK_INFO,				//È¡µÃ´ÅÅÌĞÅÏ¢
	CMD_SET_MANAGE,				//ÉèÖÃÃüÁîºÍ²Ù×÷
	CMD_GET_CMOS_REG,				//È¡µÃCMOS²ÎÊı
	CMD_SET_CMOS_REG,				//ÉèÖÃCMOS²ÎÊı
	CMD_SET_SYSTEM_CMD,			//ÉèÖÃÖ´ĞĞÃüÁî
	CMD_SET_KEYFRAME_REQ,         //70.ÉèÖÃ¹Ø¼üÖ¡ÇëÇó
    CMD_GET_CONFIGENCPAR,         //71.È¡µÃÊÓÆµ²ÎÊı
    CMD_SET_CONFIGENCPAR,         //72.ÉèÖÃÊÓÆµ²ÎÊ
    CMD_GET_ALL_PARAMETER_NEW,    //73.»ñÈ¡ËùÓĞ²ÎÊı
    CMD_FING_LOG,                  //74.²éÕÒÈÕÖ¾(²éÑ¯·½Ê½:0£­È«²¿£¬1£­°´ÀàĞÍ£¬2£­°´Ê±¼ä£¬3£­°´Ê±¼äºÍÀàĞÍ 0xFF-¹Ø±Õ±¾´ÎËÑË÷)
    CMD_GET_LOG,                   //75.¶ÁÈ¡²éÕÒµ½µÄÈÕÖ¾ 
    CMD_GET_SUPPORT_AV_FMT,       //76.»ñÈ¡Éè±¸Ö§³ÖµÄ±àÂë¸ñÊ½¡¢¿í¸ß¼°ÒôÆµ¸ñÊ½
    CMD_GET_VIDEO_CONFIG_NEW,     //77.»ñÈ¡ÊÓÆµ²ÎÊı£­new
    CMD_SET_VIDEO_CONFIG_NEW,     //78.ÉèÖÃÊÓÆµ²ÎÊı£­new
    CMD_GET_VMOTION_CONFIG_NEW,   //79.»ñÈ¡ÒÆ¶¯±¨¾¯²ÎÊı£­new
    CMD_SET_VMOTION_CONFIG_NEW,   //80.ÉèÖÃÒÆ¶¯±¨¾¯²ÎÊı£­new
    CMD_GET_VLOST_CONFIG_NEW,     //81.»ñÈ¡ÊÓÆµ¶ªÊ§±¨¾¯²ÎÊı£­new
    CMD_SET_VLOST_CONFIG_NEW,     //82.ÉèÖÃÊÓÆµ¶ªÊ§±¨¾¯²ÎÊı£­new
    CMD_GET_SENSOR_ALARM_NEW,     //83.»ñÈ¡Ì½Í·±¨¾¯²ÎÊı£­new
    CMD_SET_SENSOR_ALARM_NEW,     //84.ÉèÖÃÌ½Í·±¨¾¯²ÎÊı£­new
    CMD_GET_NET_ALARM_CONFIG,     //85.»ñÈ¡ÍøÂç¹ÊÕÏ±¨¾¯²ÎÊı
    CMD_SET_NET_ALARM_CONFIG,     //86.ÉèÖÃÍøÂç¹ÊÕÏ±¨¾¯²ÎÊı
    CMD_GET_RECORD_CONFIG,        //87.»ñÈ¡¶¨Ê±Â¼Ïñ²ÎÊı
    CMD_SET_RECORD_CONFIG,        //88.¶¨Ê±Â¼Ïñ²ÎÊı
    CMD_GET_SHOOT_CONFIG,         //89.»ñÈ¡¶¨Ê±×¥ÅÄ²ÎÊı
    CMD_SET_SHOOT_CONFIG,         //90.ÉèÖÃ¶¨Ê±×¥ÅÄ²ÎÊı
    CMD_GET_FTP_CONFIG,           //91.»ñÈ¡FTP²ÎÊı
    CMD_SET_FTP_CONFIG,           //92.ÉèÖÃFTP²ÎÊı
    CMD_GET_RF_ALARM_CONFIG,      //93.»ñÈ¡ÎŞÏß±¨¾¯²ÎÊı
    CMD_SET_RF_ALARM_CONFIG,      //94.ÉèÖÃÎŞÏß±¨¾¯²ÎÊı
    CMD_GET_EXT_DATA_CONFIG,      //95.»ñÈ¡ÆäËüÀ©Õ¹²ÎÊı(ÈçÆ½Ì¨ÉèÖÃÆäËü²ÎÊı)
    CMD_SET_EXT_DATA_CONFIG,      //96.ÉèÖÃÆäËüÀ©Õ¹²ÎÊı(ÈçÆ½Ì¨ÉèÖÃÆäËü²ÎÊı)
    CMD_GET_FORMAT_PROCESS,       //97.»ñÈ¡Ó²ÅÌ¸ñÊ½»¯½ø¶È
    CMD_GET_PING_CONFIG,          //98.PING ÉèÖÃ»ñÈ¡
    CMD_SET_PING_CONFIG,          //99.PING ÉèÖÃÉèÖÃ
	//½âÂëÆ÷ÃüÁî
	DDCMD_GET_ALL_PARAMETER = 100,	//»ñÈ¡½âÂëÆ÷ËùÓĞÉèÖÃ
	DDCMD_GET_TIME,				//»ñÈ¡ÏµÍ³Ê±¼ä
	DDCMD_SET_TIME,				//ÉèÖÃÏµÍ³Ê±¼ä
	DDCMD_GET_SYS_CONFIG,			//»ñÈ¡ÏµÍ³ÅäÖÃ
	DDCMD_SET_SYS_CONFIG,			//ÉèÖÃÏµÍ³ÅäÖÃ
	DDCMD_GET_NET_CONFIG,			//»ñÈ¡ÍøÂçÅäÖÃ
	DDCMD_SET_NET_CONFIG,			//ÉèÖÃÍøÂçÅäÖÃ
	DDCMD_GET_COM_CONFIG,			//»ñÈ¡´®¿ÚÅäÖÃ
	DDCMD_SET_COM_CONFIG,			//ÉèÖÃ´®¿ÚÅäÖÃ
	DDCMD_GET_VIDEO_CONFIG,		//»ñÈ¡ÊÓÆµÅäÖÃ
	DDCMD_SET_VIDEO_CONFIG,		//ÉèÖÃÊÓÆµÅäÖÃ
	DDCMD_GET_ALARM_OPT,			//»ñÈ¡±¨¾¯Ñ¡Ïî
	DDCMD_SET_ALARM_OPT,			//ÉèÖÃ±¨¾¯Ñ¡Ïî
	DDCMD_GET_USER_INFO,			//»ñÈ¡ÓÃ»§ÉèÖÃĞÅÏ¢
	DDCMD_SET_USER_INFO,			//ÉèÖÃÓÃ»§ÉèÖÃĞÅÏ¢
	DDCMD_GET_ALARM_RECORD,		//»ñÈ¡±¨¾¯¼ÇÂ¼ĞÅÏ¢
	DDCMD_GET_ADRRESS_BOOK,		//»ñÈ¡µØÖ·±¡ÅäÖÃ
	DDCMD_SET_ADRRESS_BOOK,		//ÉèÖÃµØÖ·±¡ÅäÖÃ
	DDCMD_SET_COMM,				//ÉèÖÃ·¢ËÍ´®¿ÚÊı¾İ
	DDCMD_SET_CMD,					//ÉèÖÃÍ¸Ã÷µÄÃüÁî
	DDCMD_GET_YUNTAI_INFO,			//»ñÈ¡ÔÆÌ¨ĞÅÏ¢
	DDCMD_GET_YUNTAI_CONFIG,		//»ñÈ¡ÔÆÌ¨ÅäÖÃ
	DDCMD_SET_YUNTAI_CONFIG,		//ÉèÖÃÔÆÌ¨ÅäÖÃ
	DDCMD_GET_ONELINK_ADDR,		//»ñÈ¡½âÂëÆ÷µ¥Â·Á¬½ÓµÄĞÅÏ¢
	DDCMD_SET_ONELINK_ADDR,		//ÉèÖÃ½âÂëÆ÷µ¥Â·Á¬½ÓµÄĞÅÏ¢
	DDCMD_GET_CYCLELINK_ADDR,		//»ñÈ¡½âÂëÆ÷Ñ­»·Á¬½ÓµÄĞÅÏ¢
	DDCMD_SET_CYCLELINK_ADDR,		//ÉèÖÃ½âÂëÆ÷Ñ­»·Á¬½ÓµÄĞÅÏ¢
	DDCMD_GET_EXTINFO,				//»ñÈ¡À©Õ¹ÅäÖÃ
	DDCMD_SET_EXTINFO,				//ÉèÖÃÀ©Õ¹ÅäÖÃ
	DDCMD_GET_NTP,					//»ñÈ¡NTPÅäÖÃ
	DDCMD_SET_NTP,					//ÉèÖÃNTPÅäÖÃ
	DDCMD_GET_UPNP,				//»ñÈ¡UPNPÅäÖÃ
	DDCMD_SET_UPNP,				//ÉèÖÃUPNPÅäÖÃ
	DDCMD_GET_MAIL,				//»ñÈ¡MAILÅäÖÃ
	DDCMD_SET_MAIL,				//ÉèÖÃMAILÅäÖÃ
	DDCMD_GET_ALARMNAME,			//»ñÈ¡±¨¾¯ÃûÅäÖÃ
	DDCMD_SET_ALARMNAME,			//ÉèÖÃ±¨¾¯ÃûÅäÖÃ
	DDCMD_GET_WFNET,				//»ñÈ¡ÎŞÏßÍøÂçÅäÖÃ
	DDCMD_SET_WFNET,				//ÉèÖÃÎŞÏßÍøÂçÅäÖÃ
	DDCMD_GET_SEND_DEST,			//ÉèÖÃÊÓÆµ¶¨Ïò·¢ËÍÄ¿±ê»ú
	DDCMD_SET_SEND_DEST,			//ÉèÖÃÊÓÆµ¶¨Ïò·¢ËÍÄ¿±ê»ú

	CMD_GET_VPN_CONFIG = 200,		//200.»ñÈ¡VPNÉèÖÃ²ÎÊı
	CMD_SET_VPN_CONFIG = 201,		//201.ÉèÖÃVPN²ÎÊı
	CMD_GET_3G_CONFIG  = 202,		//»ñÈ¡3G²ÎÊı
	CMD_SET_3G_CONFIG  = 203,      //ÉèÖÃ3G²ÎÊı
	CMD_GET_GPS_CONFIG = 204,
	CMD_SET_GPS_CONFIG = 205,
	CMD_GET_3G_DIALCTRL= 206,
	CMD_SET_3G_DIALCTRL= 207,	
	
	//²ÎÊıÀ©Õ¹===================
	CMD_GET_IR_CONFIG = 400,		//»ñÈ¡ºìÍâĞÅÏ¢ÅäÖÃ
	CMD_SET_IR_CONFIG,				//ÉèÖÃºìÍâĞÅÏ¢ÅäÖÃ
	CMD_GET_ALL_CONFIGPARAM,		//»ñÈ¡ËùÓĞ²ÎÊı
	CMD_SET_FORMATTING, 		//¸ñÊ½»¯

	CMD_GET_VI_SENSOR=1000,
	CMD_SET_VI_SENSOR,
	CMD_GET_VI_SCENE,
	CMD_SET_VI_SCENE,
	CMD_GET_VI_CFG,
	CMD_SET_VI_CFG,
}CMD_NET;

typedef enum _RELAY_CHECK_RET
{
	RCRET_SUCCESS = 0,
	RCRET_FAILURE = -1,
	RCRET_AUTO_LINK = 0x0101,	
}RELAY_CHECK_RET;



/*********************************  ¶ş¡¢»Øµ÷º¯Êı  ******************************/

//1£¬ÊµÊ±ÒôÊÓÆµÊı¾İÁ÷»Øµ÷
typedef int  ( *ChannelStreamCallback)(HANDLE hOpenChannel,void *pStreamData,UINT dwClientID,void *pContext,ENCODE_VIDEO_TYPE encodeVideoType,ULONG frameno);

//2£¬ÊµÊ±¶Ô½²ÒôÆµÊı¾İÁ÷»Øµ÷
typedef int  ( *TalkStreamCallback)(void *pTalkData,UINT nTalkDataLen,void *pContext);

//3£¬ÏûÏ¢Í¨Öª£¬Í¨Öªµ÷ÓÃ
typedef int  ( *MsgNotifyCallback)(UINT dwMsgID,UINT ip,UINT port,HANDLE hNotify,void *pPar);

//4£¬¼ì²éÓÃ»§£¬¿Í»§¶ËµÇÂ½Ê±¼ì²é
typedef int  (*CheckUserPsw)(const CHAR *pUserName,const CHAR *pPsw);

//5£¬¿Í»§¶Ë´«µİµÄÏûÏ¢
typedef int  (*ServerMsgReceive)(ULONG ip,ULONG port,CHAR *pMsgHead);

//6£¬Éı¼¶
typedef int	 (*ServerUpdateFile)(int nOperation,int hsock,ULONG ip,ULONG port,int nUpdateType,CHAR *pFileName,CHAR *pFileData,int nFileLen);


//7£¬×ª·¢·şÎñµÄÓÃ»§¼ì²â»Øµ÷
typedef int	 (*RelayCheckUserCallback)(RELAY_TYPE relayType,UINT dwClientIP,USHORT wClientPort,CHAR *pszUserName,CHAR *pszPassword,CHAR *pszDeviceName,UINT dwRequstChannel,INT bOnline,CHAR *pDeviceIP,UINT *pdwDevicePort,CHAR *pContext);

//8£¬ÖĞĞÄ·şÎñÆ÷Ä£Ê½ÓÃ»§¼ì²â»Øµ÷
typedef int	 (*CenterCheckUserCallback)(INT bOnLine,DVS_MACHINE_TYPE machineType,UINT dwDeviceID,UINT dwChannelNum,UINT ip,USHORT port,CHAR *pszDeviceName,CHAR *pszUserName,CHAR *pszPassword,LPVOID pNetPar);

//9£¬ËÑË÷Â¼ÏñNVS»Øµ÷
typedef void (*SearchRecNVSCallback)(CHAR *szNvsBuffer,int nBufferLen);

//10£¬ËÑË÷Â¼ÏñÎÄ¼ş
//typedef void (WINAPI *SearchRecFileCallback)(void *pRecFile);
typedef void  (*SearchRecFileCallback)(UINT dwClientID,void *pRecFile);

//11£¬¿âÏûÏ¢»Øµôº¯Êı
typedef int	 (*MessageNotifyCallback)(UINT wParam, UINT lParam);



/******************************  Èı¡¢½Ó¿Ú½á¹¹ÀàĞÍ¶¨Òå  ***************************/
#ifndef AV_INFO_DEFINE
#define AV_INFO_DEFINE

//1£¬ÊÓÆµÒôÆµ²ÎÊı
typedef struct _AV_INFO
{
    //ÊÓÆµ²ÎÊı
    UINT			nVideoEncodeType;		//ÊÓÆµ±àÂë¸ñÊ½
    UINT			nVideoHeight;			//ÊÓÆµÍ¼Ïñ¸ß
    UINT			nVideoWidth;			//ÊÓÆµÍ¼Ïñ¿í
    //ÒôÆµ²ÎÊı
    UINT			nAudioEncodeType;		//ÒôÆµ±àÂë¸ñÊ½
    UINT			nAudioChannels;			//Í¨µÀÊı
    UINT			nAudioBits;				//Î»Êı
    UINT			nAudioSamples;			//²ÉÑùÂÊ
}AV_INFO,*PAV_INFO;

#endif //AV_INFO_DEFINE


//2£¬ÒôÊÓÆµÊı¾İÖ¡Í·
typedef struct _FRAME_HEAD
{
	USHORT	zeroFlag;				// 0
	UCHAR   oneFlag;				// 1
	UCHAR	streamFlag;				// Êı¾İÖ¡±êÖ¾ FRAME_FLAG_VP£¬FRAME_FLAG_VI£¬FRAME_FLAG_A
	
	ULONG	nByteNum;				//Êı¾İÖ¡´óĞ¡
	ULONG	nTimestamp;				//Ê±¼ä´Á
}FRAME_HEAD;

//3,±¨¾¯Êä³ö

typedef struct _ALARM_STATUS_OUTPUT_NEW
{
	unsigned char year;

	unsigned char month;

	unsigned char day;

	unsigned char week;

	unsigned char hour;

	unsigned char minute;

	unsigned char second;

	unsigned char millsecond; 

	unsigned int SensorAlarm;
	unsigned int MotionAlarm;
	unsigned int ViLoseAlarm;
	unsigned int RFSensorAlarm;
	unsigned int NetAlarm;

	unsigned int SensorAlarmRec[MAX_SENSOR_NUM];
	unsigned int MotionAlarmRec[MAX_VIDEO_NUM];
	unsigned int ViLoseAlarmRec[MAX_VIDEO_NUM];
	unsigned int RFSensorAlarmRec[MAX_RF_SENSOR_NUM];
	unsigned int NetAlarmRec;

	unsigned int OutputStatus;

	unsigned int reserved[19];
}ALARM_STATUS_OUTPUT_NEW;


//4£¬±¨¾¯Í¨ÖªĞÅÏ¢½á¹¹

typedef struct _ALARM_MSG_NOTIFY_NEW
{
	HANDLE hLogonServer;

	UINT dwClientID;

	UINT dwServerIP;

	UINT dwServerPort;

	ALARM_STATUS_OUTPUT_NEW alarmStatus;
}ALARM_MSG_NOTIFY_NEW;



//5£¬´ò¿ªÊÓÆµÍ¨µÀ²ÎÊı

typedef struct _OPEN_CHANNEL_INFO_EX
{
    ULONG                         dwClientID;                       //»Øµ÷²ÎÊı	(¶ÔÓ¦»Øµ÷º¯ÊıÀïµÄdwClientID)
    UINT                          nOpenChannel:8;                   //Í¨µÀºÅ£¨0 ¡« 3£©
    UINT                          nSubChannel:8;                    //0: ´ò¿ªÖ÷ÂëÁ÷      1: ´ò¿ª´ÓÂëÁ÷
    UINT                          res:16;                            //±¸ÓÃ
    NET_PROTOCOL_TYPE             protocolType;                     //Á¬½Ó·½Ê½£¨TCP¡¢UDP¡¢¶à²¥£©  
    ChannelStreamCallback         funcStreamCallback;              //ÒôÊÓÆµÁ÷Êı¾İ»Øµ÷º¯Êı 
    void                          *pCallbackContext;               //»Øµ÷²ÎÊı2(¶ÔÓ¦»Øµ÷º¯ÊıÀïµÄpContext) 
}OPEN_CHANNEL_INFO_EX;

//6£¬´ò¿ªÊÓÆµÍ¨µÀ²ÎÊı
typedef struct _OPEN_VIEWPUSH_INFO
{
	UINT					dwClientID;
	UINT					nViewPushChannel;
	NET_PROTOCOL_TYPE		protocolType;
	AV_INFO				avInformation;
	UINT					nMulticastAddr;
	UINT					nMulticastPort;
	UINT					nScreenCount;
	UINT					nScreenIndex;
}OPEN_VIEWPUSH_INFO;

//7£¬´ò¿ªµÄ·şÎñÆ÷ĞÅÏ¢
typedef struct _SERVER_INFO
{
	HANDLE					hServer;
	CHAR					szServerIP[MAX_IP_NAME_LEN+1];
	UINT					nServerPort;
	CHAR					szDeviceName[DVS_NAME_LEN+1];
	UINT					nDeviceID;
	CHAR					szUserName[USER_NAME_LEN+1];
	CHAR					szUserPassword[USER_PASSWD_LEN+1];
	UINT					dwClientID;
	CONNECT_STATUS			logonStatus;
	UINT					nVersion;
	UINT					nLogonID;
	UINT					nPriority;
	UINT					nServerChannelNum;
	UINT					nLanguageNo;
	DVS_MACHINE_TYPE		nMachineType;
	INT						bPalStandard;
	UINT					nMulticastAddr;
	UINT					nMulticastPort;
	AV_INFO					avInformation[MAX_VIDEO_NUM];
}SERVER_INFO;

//8£¬´ò¿ªµÄÍ¨µÀĞÅÏ¢
typedef struct _CHANNEL_INFO
{
	HANDLE					hOpenChannel;
	CHAR					szServerIP[MAX_IP_NAME_LEN+1];
	UINT					nServerPort;
	CHAR					szDeviceName[DVS_NAME_LEN+1];
	CHAR					szUserName[USER_NAME_LEN+1];
	CHAR					szUserPassword[USER_PASSWD_LEN+1];
	UINT					dwClientID;
	CONNECT_STATUS			openStatus;
	UINT					nVersion;
	UINT					nOpenID;
	UINT					nPriority;
	UINT					nOpenChannelNo;
	UINT					nMulticastAddr;
	UINT					nMulticastPort;
	AV_INFO				avInformation;
	ENCODE_VIDEO_TYPE		encodeVideoType;
	NET_PROTOCOL_TYPE		protocolType;
	ChannelStreamCallback	funcStreamCallback;
	void					*pCallbackContext;
	UINT					dwDeviceID;	//V4.0 add
}CHANNEL_INFO;

//9£¬´ò¿ªµÄ½âÂëÆ÷ÊäÈëÍ¨µÀĞÅÏ¢
typedef struct _VIEWPUSH_INFO
{
	HANDLE				hOpenChannel;
	CHAR				szServerIP[MAX_IP_NAME_LEN+1];
	UINT				nServerPort;
	CHAR				szDeviceName[DVS_NAME_LEN+1];
	CHAR				szUserName[USER_NAME_LEN+1];
	CHAR				szUserPassword[USER_PASSWD_LEN+1];
	UINT				dwClientID;
	CONNECT_STATUS		openStatus;
	UINT				nVersion;
	UINT				nOpenID;
	UINT				nPriority;
	UINT				nOpenChannelNo;
	UINT				nMulticastAddr;
	UINT				nMulticastPort;
	AV_INFO			avInformation;
	ENCODE_VIDEO_TYPE	encodeVideoType;
	NET_PROTOCOL_TYPE	protocolType;
	DVS_MACHINE_TYPE	nMachineType;
	UINT				dwChannelNum;	//½âÂëÆ÷×î´óÂ·Êı
}VIEWPUSH_INFO;

//10£¬¶Ô½²µÄĞÅÏ¢
typedef struct _SS_TALK_INFO 
{
	HANDLE				hServer;
	CHAR				szServerIP[MAX_IP_NAME_LEN+1];
	UINT				nServerPort;
	CHAR				szDeviceName[DVS_NAME_LEN+1];
	CHAR				szUserName[USER_NAME_LEN+1];
	CHAR				szUserPassword[USER_PASSWD_LEN+1];
	ULONG		version;	
	ULONG		nMachineType;
	CONNECT_STATUS		logonStatus;
	//audio parameter
	UINT				nAudioEncodeType;
    UINT				nAudioChannels;
    UINT				nAudioBits;
    UINT				nAudioSamples;
}TALKING_INFO;

//11£¬ÓïÒô¹ã²¥µÄÓÃ»§ĞÅÏ¢
typedef struct _BROADCAST_USER
{
	HANDLE	hBroadcastUser;
	CHAR	szServerIP[MAX_IP_NAME_LEN+1];
	UINT	port;
	CHAR	szDeviceName[DVS_NAME_LEN+1];	
	CHAR	szUserName[USER_NAME_LEN+1];
	CHAR	szUserPassword[USER_PASSWD_LEN+1];
	INT	bConnect;
	//SOCKET	hSock;
	//PVOID	pPar;
	//add v4.2
	UINT				machineType;
	ENCODE_VIDEO_TYPE	audioType;
}BROADCAST_USER;

//12£¬×ª·¢ÊÓÆµ·şÎñÆ÷
typedef struct _RELAY_NVS
{
	RELAY_TYPE			relayType;
	CHAR				szServerIP[MAX_IP_NAME_LEN+1];
	USHORT				wServerPort;
	CHAR				szDeviceName[DVS_NAME_LEN+1];
	UINT				dwChannelNo;
	UINT				dwCount;
}RELAY_NVS;	

//13£¬°Ù·Ö±ÈÏûÏ¢Í¨Öª
typedef struct _PERCENT_NOTIFY
{
	HANDLE				hLogon;
	UINT				dwClientID;
	UINT				dwPercent;
}PERCENT_NOTIFY;

//14£¬ÊÓÆµÎÄ¼ş²ÎÊı
typedef struct _FILE_INFO
{
	CHAR                szFileName[MAX_PATH];
	CHAR				szServerIP[MAX_IP_NAME_LEN+1];
	USHORT				wServerPort;
	CHAR				szUserName[USER_NAME_LEN+1];
	CHAR				szUserPassword[USER_PASSWD_LEN+1];
	INT                bRelay     ; // ´ËÎÄ¼şÊÇ·ñÍ¨¹ı×ª·¢
}FILE_INFO;

//15£¬´ò¿ªÊÓÆµÎÄ¼ş²ÎÊı
typedef struct _OPEN_FILE_INFO
{
	UINT				dwClientID  ;
	UINT				nOpenChannel;
	NET_PROTOCOL_TYPE	protocolType;
	CHAR				szDeviceName[DVS_NAME_LEN+1];
	FILE_INFO        hhFile      ;       
	CHAR                szOpenMode[5];
	UINT				dwSocketTimeOut;
	
    // Êä³ö
	UINT				dwFileSize;         // ÎÄ¼ş´óĞ¡
	UINT				dwStartTime;        // ²¥·Å¿ªÊ¼Ê±¼ä(ºÁÃë)
	UINT				dwEndTime;          // ²¥·Å½áÊøÊ±¼ä(ºÁÃë)
	
	UINT				nVideoEncodeType;	//ÊÓÆµ±àÂë¸ñÊ½
	UINT				nAudioEncodeType;	//ÒôÆµ±àÂë¸ñÊ½	
}OPEN_FILE_INFO;

//16£¬Æô¶¯ËÑË÷·ûºÏÌõ¼şµÄNVS
typedef struct _SEARCH_REC_NVS
{
	UINT				dwClientID  ;
	// ´æ´¢Â¼ÏñÎÄ¼şµÄÌõ¼ş
	CHAR                Date[11];			// Ìõ¼ş1 Â¼ÏñÈÕÆÚ yyyy-mm-dd
    UCHAR                recType ;			// Ìõ¼ş2 Â¼ÏñÀàĞÍ: 0-ËùÓĞ£¬1-±¨¾¯£¬2-ÊÖ¶¯£¬3-¶¨Ê±
	
	//SearchRecNVSCallback	funcSearchRecNvsCallback;	
}SEARCH_REC_NVS;

// 17£¬Æô¶¯ËÑË÷·ûºÏÌõ¼şµÄÂ¼ÏñÎÄ¼ş
typedef struct _SEARCH_REC_FILE
{
	UINT				dwClientID  ;

	// ´æ´¢Â¼ÏñÎÄ¼şµÄÌõ¼ş
    CHAR                szDir[MAX_PATH]; 	// Ìõ¼ş1: " Datae\\Ip-NVS\\Camera\\"
	CHAR				szTime1[6];      	// Ìõ¼ş2 Ê±¼ä¶Î1 hh:mm
	CHAR				szTime2[6];      	// Ìõ¼ş2 Ê±¼ä¶Î2 hh:mm
    UCHAR                recType ;	    	 // Ìõ¼ş3 Â¼ÏñÀàĞÍ: 0-ËùÓĞ£¬1-±¨¾¯£¬2-ÊÖ¶¯£¬3-¶¨Ê±
	
	SearchRecFileCallback	funcSearchRecFileCallback;
}SEARCH_REC_FILE;

//18£¬×ª·¢²éÑ¯
typedef struct _RELAY_QUERY_INFO
{
	UINT		dwSize;
	UINT		dwServerLogonNum;
	UINT		dwServerPreviewNum;
	UINT		dwServerTalkNum;
	UINT		dwServerBrdNum;
	UINT		dwClientLogonNum;
	UINT		dwClientPreviewNum;
	UINT		dwClientTalkNum;
	UINT		dwClientBrdNum;
	CHAR		reserve[16];
}RELAY_QUERY_INFO;



typedef struct _SEARCH_SER_INFO
{ 
	char				userName[USER_NAME_LEN+1];
	char				userPassword[USER_PASSWD_LEN+1];
	DVS_MACHINE_TYPE	nDeviceType;
	char				szDeviceName[64];
	unsigned long		ipLocal;
	unsigned char		macAddr[6];
	unsigned short		wPortWeb;
	unsigned short		wPortListen;
	unsigned long		ipSubMask;
	unsigned long		ipGateway;
	unsigned long		ipMultiAddr;
	unsigned long		ipDnsAddr;
	unsigned short		wMultiPort;
	int					nChannelNum;
}SEARCH_SER_INFO;

//========================================================================
//				ËÄ¡¢ ÒôÊÓÆµÍøÂçÖ¡Í·
//========================================================================

//1£¬Êı¾İÖ¡±êÖ¾
#define FRAME_FLAG_VP		0x0b	//ÊÓÆµµÄPÖ¡
#define FRAME_FLAG_VI		0x0e	//ÊÓÆµµÄIÖ¡
#define FRAME_FLAG_A		0x0d	//ÒôÆµÖ¡

//À©Õ¹Ö¡Í·
#define	EXT_HEAD_FLAG	0x06070809
#define	EXT_TAIL_FLAG	0x0a0b0c0d

//ÒôÆµ±àÂëËã·¨
typedef enum  _PT_AENC_TYPE_E
{
	PT_AENC_NONE   = 0x0,
	PT_AENC_G726   = 0x01,
	PT_AENC_G722   = 0x02,
	PT_AENC_G711A  = 0x03,
	PT_AENC_ADPCM  = 0x04,
	PT_AENC_MP3    = 0x05,
	PT_AENC_PCM    = 0x06,
	PT_AENC_G711U  = 0x07,
	PT_AENC_AACLC  = 0x08,
	PT_AENC_AMRNB  = 0x09,
}PT_AENC_TYPE_E;


//ÊÓÆµ±àÂëËã·¨
typedef enum  _PT_VENC_TYPE_E
{
	PT_VENC_NONE   = 0x0,
	PT_VENC_H264   = 0x01,
	PT_VENC_MPEG4  = 0x02,
	PT_VENC_MJPEG  = 0x03,
	PT_VENC_JPEG   = 0x04,
}PT_VENC_TYPE_E;



//ÊÓÆµ²ÎÊı
typedef struct _EXT_FRAME_VIDEO
{
	unsigned short	nVideoEncodeType;	//ÊÓÆµ±àÂëËã·¨
	unsigned short	nVideoWidth;		//ÊÓÆµÍ¼Ïñ¿í
	unsigned short	nVideoHeight;		//ÊÓÆµÍ¼Ïñ¸ß
	unsigned char   nPal;               //ÖÆÊ½
	unsigned char   bTwoFeild;			//ÊÇ·ñÊÇÁ½³¡±àÂë(Èç¹ûÊÇÁ½³¡±àÂë£¬PC¶ËĞèÒª×ödeinterlace)
	unsigned char   nFrameRate;			//Ö¡ÂÊ
	unsigned char   szReserve[7];		//

} EXT_FRAME_VIDEO;

//ÒôÆµ²ÎÊı
typedef struct _EXT_FRAME_AUDIO
{
	unsigned short	nAudioEncodeType;	//ÒôÆµ±àÂëËã·¨
	unsigned short	nAudioChannels;		//Í¨µÀÊı
	unsigned short	nAudioBits;			//Î»Êı
	unsigned char   szReserve[2];
	unsigned long	nAudioSamples;		//²ÉÑùÂÊ 	
	unsigned long	nAudioBitrate;		//ÒôÆµ±àÂëÂëÂÊ
} EXT_FRAME_AUDIO;

typedef union _EXT_FRAME_TYPE
{
	EXT_FRAME_VIDEO	szFrameVideo;
	EXT_FRAME_AUDIO	szFrameAudio;
} EXT_FRAME_TYPE;

typedef struct _EXT_FRAME_HEAD
{
    unsigned long	nStartFlag;			//À©Õ¹Ö¡Í·ÆğÊ¼±êÊ¶
    unsigned short	nVer;				//°æ±¾
    unsigned short	nLength;			//À©Õ¹Ö¡Í·³¤¶È
	EXT_FRAME_TYPE	szFrameInfo;		
	unsigned long   nTimestamp;			//ÒÔºÁÃëÎªµ¥Î»µÄÊ±¼ä´Á
	unsigned long	nEndFlag;			//À©Õ¹Ö¡Í·ÆğÊ¼±êÊ¶
}EXT_FRAME_HEAD;




typedef INT  (*StreamWriteCheck)(int nOperation,const CHAR *pUserName,const CHAR *pPsw,ULONG ip,ULONG port,OPEN_VIEWPUSH_INFO viewPushInfo,HANDLE hOpen);
typedef INT (*CallbackServerFind)(SEARCH_SER_INFO *pSearchInfo);

/********************************  Îå¡¢º¯Êı½Ó¿Ú¶¨Òå  *****************************/

//Æô¶¯·şÎñ
ERR_CODE		NET_Startup(USHORT nBasePort,MsgNotifyCallback msgCallback,CheckUserPsw checkUserPswCallback,ServerUpdateFile updateCallback,ServerMsgReceive msgCmdCallback,StreamWriteCheck streamWriteCheckCallback,ChannelStreamCallback funcChannelCallback);
//¹Ø±Õ·şÎñ
ERR_CODE		NET_Cleanup();


//µÇÂ¼·şÎñÆ÷
ERR_CODE		NET_LogonServer(IN  CHAR *pServerIP,IN  UINT nServerPort,IN  CHAR *pDeviceName,IN	 CHAR *pUserName,IN	 CHAR *pUserPassword,IN UINT dwClientID,OUT  HANDLE *hLogonServer);
//×¢Ïú·şÎñÆ÷
ERR_CODE		NET_LogoffServer(IN  HANDLE hServer);
//¶ÁÈ¡µÇÂ¼·şÎñÆ÷ĞÅÏ¢
ERR_CODE		NET_ReadServerInfo(IN  HANDLE hServer,OUT  SERVER_INFO *serverInfo);

//³ÌĞòÉı¼¶
ERR_CODE		NET_Update(IN HANDLE hServer,IN UPDATE_TYPE nUpdateType,IN CHAR *pFilePathName);


//¶ÁÈ¡µÇÂ¼·şÎñÆ÷ÅäÖÃĞÅÏ¢
ERR_CODE		NET_GetServerConfig(IN  HANDLE hServer,IN  CMD_NET nConfigCommand,OUT  CHAR *pConfigBuf,IN OUT  UINT *nConfigBufSize,IN  OUT  UINT *dwAppend);
//ÉèÖÃµÇÂ¼·şÎñÆ÷ÅäÖÃĞÅÏ¢
ERR_CODE		NET_SetServerConfig(IN  HANDLE hServer,IN  CMD_NET nConfigCommand,IN   CHAR *pConfigBuf,IN  UINT nConfigBufSize,IN  UINT dwAppend);


//´ò¿ªÊÓÆµÍ¨µÀ
ERR_CODE		NET_OpenChannel(IN  CHAR *pServerIP,IN  UINT nServerPort,IN  CHAR *pDeviceName,IN  CHAR *pUserName,IN  CHAR *pUserPassword,IN  OPEN_CHANNEL_INFO_EX *pOpenInfo,OUT  HANDLE *hOpenChannel);
//¹Ø±ÕÊÓÆµÍ¨µÀ
ERR_CODE		NET_CloseChannel(IN  HANDLE hOpenChannel);

//¶ÁÈ¡´ò¿ªÊÓÆµÍ¨µÀĞÅÏ¢
ERR_CODE		NET_ReadChannelInfo(IN  HANDLE hOpenChannel,OUT  CHANNEL_INFO *channelInfo);


//ÇëÇó´ò¿ª¶Ô½²
//ERR_CODE		NET_TalkRequsest(IN  CHAR *pServerIP,IN  UINT nServerPort,IN  CHAR *pDeviceName,IN	 CHAR *pUserName,IN	 CHAR *pUserPassword,IN TalkStreamCallback funcTalkCallback,IN void *pContext);
ERR_CODE		NET_TalkRequsest(IN  char *pServerIP, IN  UINT nServerPort,IN  char *pDeviceName,IN	 char *pUserName,IN	 char *pUserPassword,IN TalkStreamCallback funcTalkCallback,IN void *pContext, OUT  TALKHANDLE *hTalkback);

//½áÊø¶Ô½²
//ERR_CODE		NET_TalkStop();
ERR_CODE		NET_TalkStop(IN  TALKHANDLE hTalkback);

//¶ÁÈ¡´ò¿ª¶Ô½²ĞÅÏ¢
//ERR_CODE		NET_TalkReadInfo(TALKING_INFO *talkInfo);
//·¢ËÍ¶Ô½²Êı¾İ
//ERR_CODE		NET_TalkSend(IN CHAR *pTalkData,IN UINT nDataLen);
ERR_CODE		NET_TalkSend(IN  TALKHANDLE hTalkback, IN char *pTalkData,IN UINT nDataLen);


//ËÑË÷ÍøÉÏÉè±¸
ERR_CODE		NET_SearchAllServer(UINT nTimeWait,	 CallbackServerFind  funcServerFind);
//ÉèÖÃÊÓÆµ·şÎñÆ÷ÍøÂçÅäÖÃ
ERR_CODE		NET_ConfigServer(UCHAR macAddr[6],CHAR *pUserName,CHAR *pUserPassword,
													   CHAR *pIP,CHAR *pDeviceName,CHAR *pSubMask,CHAR *pGateway,CHAR *pMultiAddr,
													   CHAR *pDnsAddr,USHORT wPortWeb,USHORT wPortListen,USHORT wPortMulti);

void NET_UpdatePercentNotify(int hsock,ULONG nPercent);

ERR_CODE        NET_SetNetApiSupportVersion(int nVersion);


#endif


