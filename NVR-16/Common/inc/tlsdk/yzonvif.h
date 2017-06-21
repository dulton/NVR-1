/*
 *  The interface of onvif.
 *  Copyright (C)  2012 - 2013  ls
 *  From Shanghai yuanzhe Inc... 
 *
 * 
 */

#ifndef _YZONVIF_H_
#define _YZONVIF_H_



/** data type declaration and struct, union, enum section **/

typedef struct DeviceAddrsStack
{
    char **stack; /* device address container */
    char **uuid;   //uuid
    int len;	  /* device count */
}DeviceAddrsStack;

/**
 * Initialize the device.
 * @param device  pointer that points to all device addresses we found.
 * @return if success  return 0, else return other.
 * @remarks if we do not need device we must destory it.
 */
extern int discovery(DeviceAddrsStack*  device);

/**
 * destory the malloc data form discovery.
 * @param device.
 * @return void.
 */
extern void DestoryDevice(DeviceAddrsStack *device);




typedef struct DeviceAddrs{
    char* addr;		   /*a device address  */
}DeviceAddrs;


typedef struct MediaPtzAddr
{
    char *media_addr;   
    char *ptz_addr;
}MediaPtzAddr;



/**
 * Get media and ptz address 
 * @param username -> the onvif camera machine's user name.
 * @param password -> the onvif camera machine's user password.
 * @param device -> the onvif camera machine address.
 * @param media ->  return the device media and ptz address.
 * @return success 0 error -1.
 * @remarks if we do not need media we must destory it.
 */
extern int GetMediaPtzAddr(char *username,char *password ,DeviceAddrs *device, MediaPtzAddr* media);

/**
 * destory the malloc data form GetMediaPtzAddr.
 * @param media.
 * @return void.
 */
extern void DestoryMediaAddr(MediaPtzAddr *media);
/**
 *  System factory default
 * @param username -> the onvif camera machine's user name.
 * @param password -> the onvif camera machine's user password.
 * @param device -> the onvif camera machine address.
 * @param type ->  if type = 0 hard factory default type = 1 soft factory default
 * @return success 0 error -1.
 * @remarks we must wait a little seconds.
 */
extern int  SetSystemFactoryDefault(char *username,char *password ,DeviceAddrs *device,int type);
/**
 * system reboot 
 * @param username -> the onvif camera machine's user name.
 * @param password -> the onvif camera machine's user password.
 * @param device -> the onvif camera machine address.
 * @return success 0 error -1.
 * @remarks we must wait a little seconds.
 */
extern int  SystemReboot(char *username,char *password ,DeviceAddrs *device);


typedef struct DateAndTime
{
    int fromNPT;
    int Year;
    int Month;
    int Day;
    int Hour;
    int Minute;
    int Second;
}DateAndTime;

/**
 * Get system date and time  
 * @param username -> the onvif camera machine's user name.
 * @param password -> the onvif camera machine's user password.
 * @param device -> the onvif camera machine address.
 * @param  dateAndTime -> return the date and time infomation. 
 * @return success 0 error -1.
 */
extern int GetSystemDateAndTime(char *username,char *password,DeviceAddrs *device,DateAndTime *dateAndTime);


/**
 * Get the device's ip 
 * @param username -> the onvif camera machine's user name.
 * @param password -> the onvif camera machine's user password.
 * @param device -> the onvif camera machine address.
 * @param  ip -> return device ip address. 
 * @return success 0 error -1.
 */
extern int GetDeviceIp(char *username,char *password ,DeviceAddrs *device,char *ip);

/**
 * set the device's ip 
 * @param username -> the onvif camera machine's user name.
 * @param password -> the onvif camera machine's user password.
 * @param device -> the onvif camera machine address.
 * @param ip ->  input device ip address. 
 * @return success 0 error -1.
 */
extern int SetDeviceIp(char *username,char *password,DeviceAddrs *device , const char *ip);

/**
 * get the device's gateway
 * @param username -> the onvif camera machine's user name.
 * @param password -> the onvif camera machine's user password.
 * @param device -> the onvif camera machine address.
 * @param gateway ->  return device gateway address. 
 * @return success 0 error -1.
 */
extern int GetDeviceGateway(char *username,char *password ,DeviceAddrs *device,char *gateway);

/**
 * set the device's gateway
 * @param username -> the onvif camera machine's user name.
 * @param password -> the onvif camera machine's user password.
 * @param device -> the onvif camera machine address.
 * @param gateway ->  input device gateway address. 
 * @return success 0 error -1.
 */
extern int SetDeviceGateway(char *username,char *password ,DeviceAddrs *device,const char *gateway);


typedef struct IPGatewayAddr
{
    char *ip;
    char *gateway;
}IPGatewayAddr;

/**
 * obtain the device ip and gateway
 * @param username -> the onvif camera machine's user name.
 * @param password -> the onvif camera machine's user password.
 * @param device -> the onvif camera machine address.
 * @param ip ->  return  device ip and gateway address. 
 * @return get ip and gateway success return 0;
 * @return get ip success gateway error return -2;
 * @return get ip error gateway success return -1;
 * @return both error return -3.
 * @remarks if we do not need ip we must destory it.
 */
extern int GetDeviceIpGateway(char *username,char *password, DeviceAddrs *device , IPGatewayAddr *ip);

/**
 * destory the malloc data form GetDeviceIpGateway.
 * @param ip.
 * @return void.
 */
extern void DestoryIPGateway(IPGatewayAddr *ip);

typedef struct NtpServer
{
    char **stack;
    int len;        /*number of ntp server*/
    int FromDHCP;   /*from DHCP 1 ,from Ntp 0*/
    int fromNtp;    /*from DHCP 0 ,from Ntp 1*/
}NtpServer;
/**
 * get the device's ntp server
 * @param username -> the onvif camera machine's user name.
 * @param password -> the onvif camera machine's user password.
 * @param device -> the onvif camera machine address.
 * @param ntpserver ->  return  ntpserver address list. 
 * @return success 0 error -1.
 * @remarks if we do not need ip we must destory it.
 */
extern int GetNtpserver(char *username,char *password ,DeviceAddrs *device,NtpServer *ntpserver);

/**
 * destory the malloc data form GetNtpserver.
 * @param ntpserver.
 * @return void.
 */
extern void DestoryNtpServer(NtpServer *ntpserver);

/**
 * set the device's Ntp server
 * @param username -> the onvif camera machine's user name.
 * @param password -> the onvif camera machine's user password.
 * @param device -> the onvif camera machine address.
 * @param ip ->  input ntp server ip. 
 * @return success 0 error -1.
 */
extern int SetNtpserver(char *username,char *password ,DeviceAddrs *device,const char *ip);

/**
 * set the  Ntp server enabled
 * @param username -> the onvif camera machine's user name.
 * @param password -> the onvif camera machine's user password.
 * @param device -> the onvif camera machine address.
 * @return success 0 error -1.
 */
extern int SetNtpEnable(char *username,char *password ,DeviceAddrs *device);

/**
 * set the Ntp server disabled
 * @param username -> the onvif camera machine's user name.
 * @param password -> the onvif camera machine's user password.
 * @param device -> the onvif camera machine address.
 * @return success 0 error -1.
 */
extern int SetNtpDisable(char *username,char *password ,DeviceAddrs *device);


/**
 *  Ntp server is enabled
 * @param username -> the onvif camera machine's user name.
 * @param password -> the onvif camera machine's user password.
 * @param device -> the onvif camera machine address.
 * @return enabled 1 ,disabled 0 ,error -1.
 */
extern int  IsNtpOn(char *username,char *password ,DeviceAddrs *device);

/**
 *  synchronization with NTP server by Manual
 * @param username -> the onvif camera machine's user name.
 * @param password -> the onvif camera machine's user password.
 * @param device -> the onvif camera machine address.
 * @return  success return 0,NTP not open return -1, other return -2
 */
extern int TimeSynchronizationWithNTP(char *username,char *password ,DeviceAddrs *device);


/*media*/
typedef struct ProfileToken{
    char ** token;
    int len ;
}ProfileToken;

/**
 * get the device's profile token
 * @param username -> the onvif camera machine's user name.
 * @param password -> the onvif camera machine's user password.
 * @param media -> the address from function GetMediaPtzAddr.
 * @param profile ->  return   profile token . 
 * @return success 0 error -1.
 * @remarks if we do not need profile we must destory it.
 */
extern int  GetProfileToken(char* username,char* password, MediaPtzAddr *media,ProfileToken *profile);

/**
 * destory the malloc data form GetProfileToken.
 * @param profile.
 * @return void.
 */
extern void DestoryProfileToken(ProfileToken * profile);


typedef struct MediaUri{
    char **uri;
    int len;
}MediaUri;


/**
 * get the device's stream uri.
 * @param username -> the onvif camera machine's user name.
 * @param password -> the onvif camera machine's user password.
 * @param media -> the address from function GetMediaPtzAddr.
 * @param profile -> profile from function GetProfileToken. 
 * @return success 0 error -1.
 * @remarks if we do not need uri we must destory it.
 */
extern int GetStreamUri(char * username, char *password,MediaPtzAddr *media, ProfileToken* profile,MediaUri* uri );


/**
 * destory the malloc data form GetStreamUri.
 * @param uri.
 * @return void.
 */
extern void DestoryMediaUri(MediaUri *uri);

#define CONTINUOUSMOVE 0x01
#define GOTOPRESET 0x02
#define SETPRESET 0x03
#define STOP 0x04

typedef struct PTZContrl
{
    float ptz_x;     /* pan speed(-1,1) */   
    float ptz_y;     /* tile speed(-1,-1) */
    float ptz_zoom;  /* zoom speed(-1,1) */ 
    char *PresetToken;   /*preset token*/
    int option;          /*four option CONTINUOUSMOVE  GOTOPRESET SETPRESET STOP */
}PTZContrl;              

/**
 * control device move 
 * @param username -> the onvif camera machine's user name.
 * @param password -> the onvif camera machine's user password.
 * @param media -> the address from function GetMediaPtzAddr.
 * @param profile ->  media profile from function GetProfileToken . 
 * @param  ptzCommand->  use it to control device. 
 * @return success 0 error -1.
 */
extern int OnvifPTZ(char*username,char *password,MediaPtzAddr* media,ProfileToken* profile,PTZContrl* ptzCommand);














#endif
