#ifndef __VIO_COMMON_H
#define __VIO_COMMON_H

#include <lib_common.h>

typedef enum LIB_vo_mode_e
{
    VO_MODE_1MUX  = 0,
    VO_MODE_4MUX = 1,
    VO_MODE_9MUX = 2,
    VO_MODE_16MUX = 3,
    VO_MODE_2SDI_8960 = 4,
    VO_MODE_2SDI = 5,
    VO_MODE_25MUX = 6,
    VO_MODE_36MUX = 7,
    VO_MODE_BUTT
}LIB_VO_MODE_E;

HI_S32 vio_enable_vo_all(LIB_VO_MODE_E enMode);

HI_S32 vio_set_vo_ch_attr_all(HI_U32 u32ScreemDiv);

HI_S32 vio_bind_vi2vo_all(VI_CHN first_ViChn, int ChnNum, VO_CHN first_VoChn, VPSS_GRP first_VpssGrp, int SubViChn_flag);
HI_S32 vio_unbind_vi2vo_all(VI_CHN first_VoChn, int ChnNum);

HI_S32 SAMPLE_COMM_VO_StartDevLayer(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr, HI_U32 u32SrcFrmRate);
HI_S32 SAMPLE_COMM_VO_StopDevLayer(VO_DEV VoDev);
HI_S32 SAMPLE_COMM_VO_HdmiStart(VO_INTF_SYNC_E enIntfSync);
HI_S32 SAMPLE_COMM_VO_HdmiStop(HI_VOID);
HI_S32 SAMPLE_COMM_VO_StartChn(VO_DEV VoDev,VO_INTF_SYNC_E enIntfSync,LIB_VO_MODE_E enMode);
HI_S32 SAMPLE_COMM_VO_StopChn(VO_DEV VoDev,LIB_VO_MODE_E enMode);
HI_S32 SAMPLE_COMM_VO_BindVpss(VO_DEV VoDev,VO_CHN VoChn,VPSS_GRP VpssGrp,VPSS_CHN VpssChn);
HI_S32 SAMPLE_COMM_VO_UnBindVpss(VO_DEV VoDev,VO_CHN VoChn);
HI_S32 SAMPLE_COMM_VO_BindVi(VO_DEV VoDev, VO_CHN VoChn, VI_CHN ViChn);
HI_S32 SAMPLE_COMM_VO_UnBindVi(VO_DEV VoDev, VO_CHN VoChn);
HI_S32 SAMPLE_COMM_VO_GetWH(VO_INTF_SYNC_E enIntfSync, HI_U32 *pu32W,HI_U32 *pu32H, HI_U32 *pu32Frm);

#ifdef FAST_SWITCH_PREVIEW
HI_S32 adjust_vo_ch_attr(VO_DEV VoDev, LIB_VO_MODE_E enMode,int first_Chn,int ChnNum);
#endif

#endif

