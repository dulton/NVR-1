#ifndef __BACKUP_H_
#define __BACKUP_H_

#include "common_basetypes.h"
#include "mod_syscomplex.h"

#ifdef __cplusplus
extern "C" {
#endif

// #define

#define TEMP_DEBUG_DROP_IT
#define DVD_RW_DMA
#define HISI_DVDR

// typdef


// extern ..
extern s32 modSysCmplx_Backup_USB_Avi( u32 nFileNum, SRecfileInfo* pRecInfoList );
extern s32 modSysCmplx_Backup_DVDR(
		u32 nFileNum,
		SRecfileInfo* pRecInfoList,
		EMBACKUPDVDRWMODE mode);

#ifdef __cplusplus
}
#endif


#endif // __BACKUP_H_
