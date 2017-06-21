#ifndef __LS_PTZ_H
#define __LS_PTZ_H 1

#include "ls_type.h"

#define SUPPORT_NOT_INIT				-1
#define NOT_SUPPORT						0
#define SUPPORT							1



/* PTZ: command */
#define LS_PTZ_CMD_RESET          0x1000
#define LS_PTZ_CMD_LOCK           0x1001
#define LS_PTZ_CMD_UNLOCK         0x1002
#define LS_PTZ_CMD_UP             0x1003
#define LS_PTZ_CMD_UP_STOP        0x1004
#define LS_PTZ_CMD_DOWN           0x1005
#define LS_PTZ_CMD_DOWN_STOP      0x1006
#define LS_PTZ_CMD_LEFT           0x1007
#define LS_PTZ_CMD_LEFT_STOP      0x1008
#define LS_PTZ_CMD_RIGHT          0x1009
#define LS_PTZ_CMD_RIGHT_STOP     0x100a
#define LS_PTZ_CMD_UR             0x100b
#define LS_PTZ_CMD_UR_STOP        0x100c
#define LS_PTZ_CMD_UL             0x100d
#define LS_PTZ_CMD_UL_STOP        0x100e
#define LS_PTZ_CMD_DR             0x100f
#define LS_PTZ_CMD_DR_STOP        0x1010
#define LS_PTZ_CMD_DL             0x1011
#define LS_PTZ_CMD_DL_STOP        0x1012
#define LS_PTZ_CMD_ZOOMIN         0x1013
#define LS_PTZ_CMD_ZOOMIN_STOP    0x1014
#define LS_PTZ_CMD_ZOOMOUT        0x1015
#define LS_PTZ_CMD_ZOOMOUT_STOP   0x1016
#define LS_PTZ_CMD_IRISOPEN       0x1017
#define LS_PTZ_CMD_IRISOPEN_STOP  0x1018
#define LS_PTZ_CMD_IRISCLOSE      0x1019
#define LS_PTZ_CMD_IRISCLOSE_STOP 0x101a
#define LS_PTZ_CMD_FOCUSNEAR      0x101b
#define LS_PTZ_CMD_FOCUSNEAR_STOP 0x101c
#define LS_PTZ_CMD_FOCUSFAR       0x101d
#define LS_PTZ_CMD_FOCUSFAR_STOP  0x101e
#define LS_PTZ_CMD_ASS            0x101f
#define LS_PTZ_CMD_ASS_STOP       0x1020
#define LS_PTZ_CMD_POS_SET        0x1021
#define LS_PTZ_CMD_POS_CLEAR      0x1022
#define LS_PTZ_CMD_POS_CALL       0x1023
#define LS_PTZ_CMD_COMBINE        0x102c
#define LS_PTZ_CMD_COMBINE_STOP   0x102d

#define LS_PTZ_SPEED_MIN          1
#define LS_PTZ_SPEED_MAX          32

/*typedef struct
{
    uint32_t  command;
    uint32_t  param;
    time_t    time;
    void     *me;
} vas_receiver_onvif_ptz_info_t;
*/
#endif
