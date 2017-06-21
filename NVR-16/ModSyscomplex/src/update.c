//** include
//

#include <string.h>
#ifdef WIN32
#include <fcntl.h>
#include <io.h>
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/hdreg.h>
#include <scsi/scsi.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#endif
#include <limits.h>
#include "mkupdate_img.h"

#include "update.h"
#include "ToolUpgrade.h"

#ifdef CFG_SAVE_IN_FLASH
#include <linux/types.h>
#include <mtd/mtd-user.h>
#include <sys/mount.h>

#define CFG_IN_FLASH_OFT	0xE0000
//static int flash_erase_size  = 0x20000;
#endif

//** macros
//

#define UPDATE_PRT(x) printf("%s %d \n", __FUNCTION_, __LINE__);

#define UBOOT_SIZE		(384*1024)
#define PANEL_BUF_SIZE	256
#define DEV_PANEL		"/dev/panel"

#define IGNORE_UBOOT // not update uboot.bin
//#define BLOCK_SIZE	0x20000
#define BUFSIZE 0x20000 // BLOCK_SIZE

#define UPDATE_SUCCESS				0
#define UPDATE_FAILED_FILEERROR		-1
#define UPDATE_FAILED_OUTOFMEMORY	-2
#define UPDATE_FAILED_FLASHDEVICE	-3
#define UPDATE_FAILED_FLASHSTATE	-4
#define UPDATE_FAILED_FLASHERASE	-5
#define UPDATE_FAILED_FLASHWRITE	-6
#define UPDATE_FAILED_FLASHCMP		-7


//** typedef 
//

typedef struct image_header {
	u32	ih_size;	/* Image Data Size		*/
	u32	ih_dcrc;	/* Image Data CRC Checksum	*/
	u8	ih_name[32];	/* Image Name		*/
} image_header_t;

typedef struct
{
	u64 curpos;
	u64 totalpos;
	s32 cancel;
}PACK_NO_PADDING ifly_copy_progress_t;


//** local

static const u32 crc32_table[256] = {
	0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
		0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
		0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
		0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
		0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
		0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
		0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
		0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
		0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
		0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
		0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
		0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
		0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
		0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
		0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
		0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
		0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
		0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
		0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
		0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
		0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
		0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
		0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
		0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
		0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
		0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
		0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
		0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
		0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
		0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
		0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
		0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
		0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
		0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
		0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
		0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
		0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
		0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
		0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
		0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
		0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
		0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
		0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
		0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
		0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
		0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
		0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
		0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
		0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
		0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
		0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
		0x2d02ef8dL
};

static ifly_copy_progress_t tUpdate;
static u32 updated_size = 0, update_total_size = 0;

//** global
SModSyscplx sModSysIns;

//** local functions
//
static 
s32 UpgradeProgressNotify(u64 process_total_size, u64 process_cur_pos);
static BOOL VerifyPanel(char *filename);
static s32 update_panel(char *filename);
static u32 crc32_dvr(u32 val, const void *ss, s32 len);
static s32 update_mainboard(char *filename, s32 *total_size);
//static int mount_user(char *mounted_path, char *user_path);

//** global functions
//
extern BOOL is_usb_disk(int fd);
extern s32 modSysCmplx_UpdateNotify(
	SProgress *	sProg,
	s32			evt,
	s32 		emRslt );

#define UPDATEPRT(x)		//printf("[update] %d %d \n", __LINE__, x)
#define MOUNT_DIR_PATH_LEN	sizeof("myusb/")

s32 modSysCmplx_Update_USB_Do(EMUPGRADETARGET emUpdateTarget, char* pPath, void* extra)
{
	s32  ret = -1;
	char filedir[12]    = {0};
	char usb_device[32] = {0};
	struct stat stat_buf;
	SProgress sProg;
	char filepath[MODSYS_PATH_LEN_MAX+MOUNT_DIR_PATH_LEN];
	
	if(NULL == pPath)
	{
		sprintf(filedir, "myusb/");
		mkdir(filedir, 1);

		ret = get_usb_device(usb_device);
		if(ret != 0)
		{
			printf("usb device not found \n");
			goto UPDATE_ERR;
		}

		switch(emUpdateTarget)
		{
			case EM_UPGRADE_MAINBOARD:
			{
				sprintf(filepath, "%smainboard.bin", filedir);
			}break;
			case EM_UPGRADE_PANEL:
			{
				//csp modify
				//sprintf(filepath, "%smainboard.bin", filedir);
				sprintf(filepath, "%spanel.bin", filedir);
			}break;
			case EM_UPGRADE_STARTLOGO:
			{
				sprintf(filepath, "%sstartlogo.jpg", filedir);
			}break;
			case EM_UPGRADE_APPLOGO:
			{
				sprintf(filepath, "%sapplogo.jpg", filedir);
			}break;
			default:
			{
				ret = -1;
				printf("not support upgrade type ! \n");
				goto UPDATE_ERR;
			}
		}
	}
	else
	{
		if( strncmp(&pPath[1], "tmp2", 4)!=0 && strncmp(pPath, "rec", 3)!=0)
		{
			sprintf(filepath, "myusb/%s", pPath);
			ret = get_usb_device(usb_device);
			if(ret != 0)
			{
				printf("usb device not found\n");
				goto UPDATE_ERR;
			}
		}
		else
		{
			sprintf(filepath, "%s", pPath);
		}
	}

	if (0 != access(filepath, 0))
	{
		if(strlen(filedir)>0)
			umount_user(filedir);
		
		ret = -1; // firmware not found!!!

		printf("%s not found \n", filepath);
		
		goto UPDATE_ERR;
	}
	
	updated_size = 0;
	update_total_size = 0;

	int width = 0, height = 0;

	UPDATEPRT(emUpdateTarget);
	
	switch(emUpdateTarget)
	{
		case EM_UPGRADE_MAINBOARD:
		{
			printf("update_mainboard 000\n");
			ret = update_mainboard(filepath, (s32 *)&update_total_size);
		} break;
		case EM_UPGRADE_PANEL:
		{
			printf("update_panel 000\n");
			
			if (!VerifyPanel(filepath))
			{
				//remove(filepath); //Delete by Lirl on Dec/26/2011
				ret = -1;
				goto UPDATE_ERR;
			}
			
			ret = stat(filepath, &stat_buf);
			if (ret)
			{
				ret = -1;
				printf("stat %s failed!\n", filepath);
				goto UPDATE_ERR;
			}
			
			update_total_size = stat_buf.st_size;
			tUpdate.curpos = 0;
			tUpdate.totalpos = update_total_size;
			
			sModSysIns.pDisablePanelCB(0);// 升级前面板前屏蔽除升级过程中的按键响应		
			ret = update_panel(filepath);		
			if(ret!=0) sModSysIns.pDisablePanelCB(1);// 升级完毕或失败则恢复响应
			
			sleep(3);
		} break;
		case EM_UPGRADE_STARTLOGO:
		{
			///*
			sModSysIns.pGetUpdateSizeCB(filepath, &width, &height);

			UPDATEPRT(width);
			UPDATEPRT(height);
			
			if(width!=800 || height!=600 || width == 0 || height == 0)
			{
				ret = -1;
				printf("not support upgrade start logo size ! \n");
				goto UPDATE_ERR;				
			}
			//*/
			printf("update_start logo 000\n");
			ret = UpgradeStartLogo(filepath, UpgradeProgressNotify, "/dev/mtd/1", 0, 0x20000);
		} break;
		case EM_UPGRADE_APPLOGO:
		{
			///*
			sModSysIns.pGetUpdateSizeCB(filepath, &width, &height);
			if(width>1024 || height>768  || width == 0 || height == 0)
			{
				ret = -1;
				printf("not support upgrade app logo size ! \n");
				goto UPDATE_ERR;				
			}
			//*/
			printf("update_app logo 000\n");		
			ret = UpgradeAppLogo(filepath, UpgradeProgressNotify, "/dev/mtd/5", 0, 0x100000);
		} break;
		default:
		{
			ret = -1;
			printf("not support upgrade type ! \n");
			goto UPDATE_ERR;
		}
	}

	//printf("%s over, ret: %d  yaogang\n", __func__, ret);
	//fflush(stdout);

	sProg.lTotalSize = tUpdate.totalpos;
	sProg.lCurSize = tUpdate.curpos;
	
	UPDATEPRT(sProg.lTotalSize);
	
UPDATE_ERR:

	if(ret==0)
	{
		sProg.lCurSize = sProg.lTotalSize;
		modSysCmplx_UpdateNotify(
				&sProg,
				EM_SYSEVENT_UPGRADE_DONE,
				EM_UPGRADE_SUCCESS
				);
	}
	else
	{
		modSysCmplx_UpdateNotify(
				&sProg,
				EM_SYSEVENT_UPGRADE_DONE,
				EM_UPGRADE_FAILED_UNKNOW
				);
	}

	//printf("%s UpdateNotify over\n", __func__);
	//fflush(stdout);
	
	MODSYS_DEBUG(ret);
	
	return ret;
}

#ifdef HISI_3515
#define USB_MAX_PTN_NUM	8
s32 get_usb_device(char * usb_device)
{
	char  usb_index = 'a';
	int fd = -1, ret = 0, i;

	umount_user("myusb");//zlbfix20110715 update
	//printf("%s yaogang\n", __func__);
	
	while (usb_index <= 'n')
	{
		//printf("enter while.\n");
		//有些U盘比较特殊
		sprintf(usb_device,"/dev/sd%c",usb_index);
		fd = open(usb_device, O_RDONLY);
		ret = is_usb_disk(fd);
		close(fd);
		if (!ret) {
			ret = -1;
			++usb_index;
			continue;
		}

		for(i=1;i<=USB_MAX_PTN_NUM;i++)
		{
			sprintf(usb_device,"/dev/sd%c%d",usb_index,i);
			ret = modsys_MountUser(usb_device,"myusb");
			if(ret == 0) break;
			//printf("mount %s failed.\n", usb_device);
		}
		if(ret == 0) break;

		sprintf(usb_device,"/dev/sd%c",usb_index);
		ret = modsys_MountUser(usb_device,"myusb");
		if(ret == 0) break;

		++usb_index;
	}

	return ret;
}
#else
#define USB_MAX_PTN_NUM	8
s32 get_usb_device(char * usb_device)
{
	s32 ret;
	DIR * dir;
	struct dirent * ptr;

	if(!usb_device)
	{
		printf("\n usb_device null!\n");

		return -1;
	}

	dir =opendir("/proc/scsi/usb-storage");
	if(dir != NULL)
	{
		while((ptr = readdir(dir))!=NULL)
		{
			printf("d_name: %s\n",ptr->d_name);
			if(strcmp(ptr->d_name,".")&&strcmp(ptr->d_name,"..")) break;
		}
		closedir(dir);
		printf("22 d_name: %s\n",ptr->d_name);
		if(strcmp(ptr->d_name,".")&&strcmp(ptr->d_name,".."))
		{
			sprintf(usb_device,"/dev/scsi/host%s/bus0/target0/lun0/part1",ptr->d_name);
			printf("usb u %s\n",usb_device);
			ret = modsys_MountUser(usb_device,"myusb");
			if(ret<0)
			{
				sprintf(usb_device,"/dev/scsi/host%s/bus0/target0/lun0/disc",ptr->d_name);
				printf("usb 2 u %s\n",usb_device);
				ret = modsys_MountUser(usb_device,"myusb");
				if(ret<0)
				{
					printf("mount sub 2 error ret=%d\n",ret);
				}
			}
		}
		else
		{
			printf("33 d_name: %s\n",ptr->d_name);
		}
	}
	else
	{
		printf("open :%s, error errno=%d,errstr:%s\n","/proc/scsi/usb-storage",errno, strerror(errno));
	}
	
	return ret;
}
#endif

s32 UpgradeProgressNotify(u64 process_total_size, u64 process_cur_pos)
{
	SProgress sProg;

	sProg.lTotalSize = process_total_size;
	sProg.lCurSize = process_cur_pos;
	
	modSysCmplx_UpdateNotify(
		&sProg,
		EM_SYSEVENT_UPGRADE_RUN,
		EM_UPGRADE_RUNNING );
	
	/*
	if(process_total_size > 0)
		return (process_cur_pos * 100)/process_total_size;
	else
		return 0;*/

	return 0;
}

static s32 update_mainboard(char *filename, s32 *total_size)
{
	#if 1
	if (0 == UpgradeCheckFile(SIMPLE_MODEL_CHK, filename))
	{
		return UpgradeToFlash(filename, UpgradeProgressNotify);
	}
	else
	{
		return -1;
	}
	#else
	v20_update_flash(filename);
	#endif
	
	#if 0
	#define BUF_SIZE 1024

	FILE *fp_in = NULL;
	img_head_t img_head;
	u32 file_count, offset, size;
	u32 checksum, nMagic;
	s32 i, ret, len;
	u8 buf[BUF_SIZE];

	if (!filename || !total_size)
	{
		printf("update_mainboard error  **************** 1\n");
		return UPDATE_FAILED_FILEERROR;
	}

	fp_in = fopen(filename, "r");
	if (!fp_in)
	{
		printf("open file %s error ****************** 2\n", filename);
		return UPDATE_FAILED_FILEERROR;
	}

	ret = fread((char *)&img_head, sizeof(char), sizeof(img_head_t), fp_in);
	if (ret != sizeof(img_head_t))
	{
		printf("read img_head file error ****************** 3\n");
		fclose(fp_in);
		return UPDATE_FAILED_FILEERROR;
	}

	if (img_head.endia)
	{
		printf("image file build use big-endia\n");
	}
	else
	{
		printf("image file build use little-endia\n");
	}

	nMagic = get_val(img_head.endia, img_head.magic);
	if ((FILE_MAGIC_VAL != nMagic) && (FILE_MAGIC_VAL_1 != nMagic))
	{
		printf("image file magic val not correct(0x%X)\n", nMagic);
		
		fclose(fp_in);
		return UPDATE_FAILED_FILEERROR;
	}

	file_count = get_val(img_head.endia, img_head.file_count);
	printf("file_count = %d\n", file_count);
	checksum = 0;
	for (i = 0; i < file_count; i++)
	{
		offset = get_val(img_head.endia, img_head.file_info[i].offset);
		size = get_val(img_head.endia, img_head.file_info[i].size);
		*total_size += size;
		ret = fseek(fp_in, offset, SEEK_SET);
		if(ret < 0){
			printf("fseek fail\n");
			fclose(fp_in);
			return UPDATE_FAILED_FILEERROR;
		}

		while (size > 0)
		{
			len = (size > BUF_SIZE) ? BUF_SIZE : size;
			len = fread(buf, sizeof(char), len, fp_in);
			if (len < 0){
				printf("fseek fail\n");
				fclose(fp_in);
				return UPDATE_FAILED_FILEERROR;
			}

			if (len == 0)
			{
				break;
			}
			size -= len;
			checksum = crc32_dvr(checksum, buf, len);
		}
	}

	tUpdate.totalpos = *total_size;
	printf("~~~~~~~~~~~~~~check this %d\n", tUpdate.totalpos);

	//exit(1);

	if (checksum == get_val(img_head.endia, img_head.crc))
	{
		printf("crc check ok\n");
	}
	else
	{
		printf("crc check fail\n");
		fclose(fp_in);
		return UPDATE_FAILED_FILEERROR;
	}
#ifdef CHECK_UPDATE
//check updatefile
#if	defined(TL_9504_CIF)
	ret = check_update_file("R9504C", img_head.file_info[0].name);
#elif defined(TL_9504_D1)
	ret = check_update_file("R9504D", img_head.file_info[0].name);
#else
	ret = check_update_file("R9508", img_head.file_info[0].name);
#endif
	if(ret < 0)
	{
		printf("check update file eror!\n");
		fclose(fp_in);
		return UPDATE_FAILED_FILEERROR;
	}
	else
	{
		printf("\n!!check update file right!!\n\n");
	}
#endif
	for (i = 0; i < file_count; i++)
	{
		char mtd_name[16];
		mtd_name[0] = '\0';
		printf("update filename = %s\n", img_head.file_info[i].name);
		if ((0 == strcmp(img_head.file_info[i].name, "rootfs_hs3515.ext2.gz.uboot"))
			|| (0 == strcmp(img_head.file_info[i].name, "rootfs_hs3520.ext2.gz.uboot")))
		{
			strcpy(mtd_name, "/dev/mtd/2");
		}
		else if ((0 == strcmp(img_head.file_info[i].name, "uImage_hs3515"))
			|| (0 == strcmp(img_head.file_info[i].name, "uImage_hs3520")))
		{
			strcpy(mtd_name, "/dev/mtd/1");
		}
		#ifndef IGNORE_UBOOT
		else if (0 == strcmp(img_head.file_info[i].name, "u-boot.bin"))
		{
			strcpy(mtd_name, "/dev/mtd/0");
		}
		#endif
		#ifdef CONFIG_VERSION_NEW
		else if (0 == strcmp(img_head.file_info[i].name, "config.ini")) {
			strcpy(mtd_name, "/dev/mtd/02");
		}
		#endif
		else
		{
			printf("unknow file type (%s) skip!\n", img_head.file_info[i].name);
			continue;
		}

		offset = get_val(img_head.endia, img_head.file_info[i].offset);
		size = get_val(img_head.endia, img_head.file_info[i].size);
		#if 0 // by lansotn temp
		ret = 0;
		#else
		ret = flash_updata(mtd_name, fp_in, offset, size);
		if (UPDATE_SUCCESS != ret)
		{
			printf("%s update failed\n", mtd_name);
			fclose(fp_in);
			return ret;
		}
		#endif
	}
	
	fclose(fp_in);
#endif
	
	return UPDATE_SUCCESS;
}

BOOL VerifyPanel(char *filename)
{
	FILE *fp;
	char buffer[PANEL_BUF_SIZE];
	image_header_t *hdr;
	s32 len,readlen=0;
	u32 checksum, csum;
	struct stat stbuf;

	if(!filename)
		return FALSE;

	if(stat(filename, &stbuf))
	{
		perror("stat");
		return FALSE;
	}

	fp = fopen(filename, "r");
	if(!fp)
	{
		perror("fopen");
		return FALSE;
	}

	fseek(fp,stbuf.st_size - sizeof(image_header_t),SEEK_SET);

	len = fread(buffer, sizeof(char), sizeof(image_header_t), fp);

	if(len != sizeof(image_header_t))
	{
		printf("Unsupport format!\n");
		fclose(fp);
		return FALSE;
	}

	hdr = (image_header_t *)buffer;

	csum = hdr->ih_dcrc;
	//find1
	//csum= CT_LE_L(csum);

	//printf("size=0x%08x, 0x%08x\n", htonl(hdr->ih_size), hdr->ih_size);
	rewind(fp);
	checksum = 0;
	while((len=fread(buffer, sizeof(char), PANEL_BUF_SIZE, fp)))
	{
		readlen += len;

		if(readlen >(stbuf.st_size - sizeof(image_header_t)))
		{
			len = len - (readlen -(stbuf.st_size - sizeof(image_header_t)));
			checksum = crc32_dvr(checksum, (char *)buffer, len);
			break;
		}
		checksum = crc32_dvr(checksum, (char *)buffer, len);
	}

	if(checksum != csum)
	{
		printf("checksum=%x, csum=0x%08x\n", checksum, csum);
		printf("Bad checksum!\n");
		fclose(fp);
		return FALSE;
	}
	else
	{
		printf("Verifying Checksum ...OK\n");
	}

	fclose(fp);

	return TRUE;
}

static s32 update_panel(char *filename)
{
	FILE *fp = NULL;
	s32 len,times = 0;
	s32 filesize = 0;
	struct stat stbuf;
	s32 tmpFd;
	//unsigned short data;
	
	if(!filename)
		return UPDATE_FAILED_FILEERROR;
	
	if(stat(filename, &stbuf))
	{
		perror("stat");
		return UPDATE_FAILED_FILEERROR;
	}
	tUpdate.totalpos = stbuf.st_size;
	//printf("size=%llu\n",tUpdate.totalpos);
	fp = fopen(filename, "r");
	if(!fp)
	{
		perror("fopen");
		return UPDATE_FAILED_FILEERROR;
	}
	filesize = stbuf.st_size - sizeof(image_header_t);
	times = (filesize + 1) >> 1;
	
	#if 1//csp modify
	usleep(1500*1000);
	#endif
	
	/*******************/
	tmpFd = open("/dev/panel", O_RDWR);
	printf("2 panel fd=%d\n",tmpFd);
	if(tmpFd == -1)
	{
		printf("unable to open panel\n");
		fclose(fp);
		return UPDATE_FAILED_FILEERROR;
	}
	
#if 1
	s32 num;
	unsigned char tmp;
	unsigned char buf[256], buf2[30];
	
	SProgress sProg;
	
	// 1.change to ISP
	printf("step 1: change to ISP\n");
	buf[0] = 1 << 0;
	buf[1] = 1 << 7;
	write(tmpFd, buf, 2);
	
	tUpdate.curpos = tUpdate.totalpos/6;
	sProg.lTotalSize = tUpdate.totalpos;
	sProg.lCurSize = tUpdate.curpos;
	
	modSysCmplx_UpdateNotify(
			&sProg,
			EM_SYSEVENT_UPGRADE_RUN,
			EM_UPGRADE_RUNNING
			);
	
	#if 1//csp modify
	sleep(4);//wait for ready	//新的电源控制方式，需要延时至少9秒
	#else
	sleep(1);
	sleep(1);
	#endif
	
	tUpdate.curpos = tUpdate.totalpos/3;
	
	sProg.lCurSize = tUpdate.curpos;
	modSysCmplx_UpdateNotify(
			&sProg,
			EM_SYSEVENT_UPGRADE_RUN,
			EM_UPGRADE_RUNNING
			);
	
	#if 1//csp modify
	printf("wait for ready... 10 seconds\n");
	sleep(6);//wait for ready	//新的电源控制方式，需要延时至少9秒
	#else
	sleep(1);
	//sleep(3);	//wait for ready
	#endif
	
	// 2.send update cmd
	printf("step 2: send update cmd\n");
	
	buf[0] = 0x81;
	buf[1] = 0x0;
	write(tmpFd, buf, 2);
	
	int trycounter = 0;
	while(1)
	{
	    num = read(tmpFd, &tmp, 1);
	    if(num > 0)
	    {
	    	if((tmp & 0x7f) == 0x14)
			{
				break;
			}
	    }
	    usleep(100*1000);
		
	    trycounter++;
		if(trycounter>100)
		{
			fclose(fp);
			close(tmpFd);
			printf("update panel over.\n");
			return UPDATE_FAILED_FILEERROR;
		}
	}
	
	#if 1
	// 3.send IAP data
	printf("step 3: send IAP data\n");
	while(1)
	{
		len = fread(buf2, sizeof(char), sizeof(buf2), fp);
		if(len <= 0)
		{
			break;
		}
		
		buf[0] = 0x84;
		buf[1] = len;	//len must less than 128
		memcpy(&buf[2], buf2, len);
		write(tmpFd, buf, len+2);
		while(1)
		{
		    num = read(tmpFd, &tmp, 1);
		    if(num > 0)
		    {
		    	if((tmp & 0x7f) == 0x14)
				{
					break;
				}
		    }
		    
		    usleep(100*1000);
		}
		
		tUpdate.curpos += len/2;
		
		sProg.lCurSize = tUpdate.curpos;
		modSysCmplx_UpdateNotify(
				&sProg,
				EM_SYSEVENT_UPGRADE_RUN,
				EM_UPGRADE_RUNNING
				);
	}
	
	// 4.go IAP
	printf("step 4: go IAP\n");
	buf[0] = 0x82;
	buf[1] = 0x0;
	num = write(tmpFd, buf, 2);
	
	tUpdate.curpos = tUpdate.totalpos*19/20;
	
	sProg.lCurSize = tUpdate.curpos;
	modSysCmplx_UpdateNotify(
			&sProg,
			EM_SYSEVENT_UPGRADE_RUN,
			EM_UPGRADE_RUNNING
			);
	#endif

#else

#endif
	
	fclose(fp);
	close(tmpFd);
	
	printf("update panel over.\n");
	
	return UPDATE_SUCCESS;
}

#if 0
static u32 get_val(u32 in_file_endia, s32 data)
{
	s32 local_endia = 1;

	local_endia = ((*((char *)&local_endia)) == 1) ? (0) : (~0);

	if(local_endia == in_file_endia)
		return data;

	return ((data&0xFF00)<<8) | ((data &0xFF)<<24) | ((data&0xFF0000)>>8) | ((data&0xFF000000)>>24);
}
#endif

static u32 crc32_dvr(u32 val, const void *ss, s32 len)
{
	const unsigned char *s = ss;

	val = val ^ 0xffffffffL;
	while (--len >= 0)
		val = crc32_table[(val ^ *s++) & 0xff] ^ (val >> 8);

	return val ^ 0xffffffffL;
}

#ifdef CHECK_UPDATE

#if 0//csp modify
void get_board_cfg(unsigned long *flash_size, unsigned long *ddr_size)
{
	int fd, rtn;
	int idx;
	char buf[32];
	struct mtd_info_user mtd;

	*flash_size = 0;
	*ddr_size = 0;

	idx = 0;
	while(1){
		sprintf(buf, "/dev/mtd%d", idx++);
		fd = open(buf, O_SYNC | O_RDWR);
		if (fd < 0){
			break;
		}
		rtn = ioctl(fd, MEMGETINFO, &mtd);
		if (rtn < 0){
			perror("MEMGETINFO");
			close(fd);
			break;
		}
		*flash_size += mtd.size;
		close(fd);
	}

	{
		#define TL_DEV_REG_RW 			_IOWR('V', 192 + 9, unsigned int)
		unsigned int buf[5] = {0};
		fd = open("/dev/tl_R9508", O_RDWR);
		if (fd < 0) {
			perror("tl_hs3515");
			return;
		}

		buf[0] = 0x01;
		buf[1] = 0x20110010;
		if(ioctl(fd, TL_DEV_REG_RW,buf) < 0){
			printf("hs3515_rd error!\n");
			close(fd);
			return;
		}
		if(buf[0] & (1<<7)){
			*ddr_size = 256*1024*1024;
		}else{
			*ddr_size = 128*1024*1024;
		}
		close(fd);
	}
}

s32 check_update_file(char *model, char *cfg_parm)
{
	s32 	len, i;
	s32 	ret = -1;
	char 	*p = cfg_parm;
	char 	c_model[32];
	unsigned long c_fsize = 0, c_dsize = 0;
	unsigned long flash_size = 0, ddr_size = 0;

	get_board_cfg(&flash_size, &ddr_size);

	if(model == NULL || p == NULL)
	{
		ret = -1;
		goto out;
	}

	len = strlen(p);
	if(len < 4){ret = -2; goto out;}

	if(memcmp("CFG_", p, 4)){ ret = -3; goto out;}
	p += 4;
	if(*p == '\0'){ret = -4; goto out;}

	if(*p++ == 'F'){
		ret = 0;
		goto out;
	}
	
	if(*p++ != '_'){ret = -5; goto out;}

	for(i = 0; *p != '\0' && *p != '_';)
	{
		c_model[i++] = *p++;
	}
	
	if(memcmp(c_model, model, strlen(model))){ ret = -6; goto out;}

	if(*p++ == '\0'){ ret = -7; goto out;}
	while(*p != '\0' && *p >= '0' && *p <= '9')
	{
		c_fsize *= 10;
		c_fsize += *p++ - '0';
	}
	
	if(*p == '\0'){ ret = -8; goto out;}
	if(*p == 'm' || *p == 'M'){c_fsize *= 1*1024*1024;}
	else if(*p == 'k' || *p == 'K'){c_fsize *= 1*1024;}

	if(c_fsize > flash_size){ ret = -9; goto out;}

	if(*++p != '_'){ ret = -10; goto out;}
	if(*++p == '\0'){ ret = -11; goto out;}

	while(*p != '\0' && *p >= '0' && *p <= '9')
	{
		c_dsize *= 10;
		c_dsize += *p++ - '0';
	}
	
	if(*p == '\0'){ ret = -12; goto out;}
	if(*p == 'm' || *p == 'M'){c_dsize *= 1*1024*1024;}
	else if(*p == 'k' || *p == 'K'){c_dsize *= 1*1024;}

	if(c_dsize > ddr_size){ ret = -13; goto out;}

	ret = 0;

out:
	printf("\nret=%d, modle=%s,fs1=0x%lx,ds1=0x%lx, cfg_param=%s\n", ret, model, flash_size, ddr_size, cfg_parm);

	return ret;
}
#endif

#endif

#if 0
s32 flash_updata(char *target, FILE *fp_in, u32 offset, u32 size)
{
	s32 	fd;
	s32 	rtn;
	char 	*buf;
	s32 	len;
	BOOL 	bSameUboot	= FALSE;
	BOOL 	bUboot 		= FALSE;
	u32	 	nMtdWrOft 	= 0;
	struct mtd_info_user mtd;
	struct erase_info_user erase;
	//SEventPara sEvParaIns;

	printf("(bk dbf)flash_updata: target=%s, fp_in offset=%u, size=%u\n", target, offset, size);

	if(!target || !fp_in)
	{
		return UPDATE_FAILED_FILEERROR;
	}

	rtn = fseek(fp_in, offset, SEEK_SET);
	if (rtn < 0)
	{
		printf("fseek fail\n");
		fclose(fp_in);
		return UPDATE_FAILED_FILEERROR;
	}

	fd = open(target, O_SYNC | O_RDWR);
	if (fd < 0)
	{
		perror(target);
		close(fd);
		return UPDATE_FAILED_FILEERROR;
	}

	rtn = ioctl(fd, MEMGETINFO, &mtd);
	if (rtn < 0)
	{
		perror("MEMGETINFO");
		close(fd);
		return UPDATE_FAILED_FLASHSTATE;
	}
	printf("%s info: max size=0x%X\n", target, mtd.size);

	if (size > mtd.size)
	{
		printf("size = %d too large for %s!\n", size, target);
		close(fd);
		return UPDATE_FAILED_FILEERROR;
	}

	buf = (char *)malloc(BUFSIZE);
	if (NULL == buf)
	{
		printf("no memory!\n");
		close(fd);
		return UPDATE_FAILED_OUTOFMEMORY;
	}

	erase.start = 0x0;
	/*erase.length = stat_buf.st_size;
	if(stat_buf.st_size % mtd.erasesize)
		erase.length += (mtd.erasesize - (stat_buf.st_size % mtd.erasesize));*/
	erase.length = mtd.size;

	//pw 2010/9/3

	if(strcmp(target, "/dev/mtd/0") == 0) bUboot = TRUE;

	#ifdef CFG_SAVE_IN_FLASH
	if(bUboot)
	{
		erase.length = mtd.size - 0x20000;
	}

	#ifdef CONFIG_VERSION_NEW
	else if (strcmp(target, "/dev/mtd/02") == 0) {
		erase.start = CFG_IN_FLASH_OFT + BACKUP_OFFSET;
		erase.length = BACKUP_OFFSET;
	}
	#endif

	#endif

	#ifdef UPDATE_IGNORE_UBOOT
	if(bUboot)
	{
		char buf_udisk[UBOOT_SIZE];
		char buf_flash[UBOOT_SIZE];

		fread(buf_udisk, sizeof(char), UBOOT_SIZE, fp_in);

		lseek(fd, 0 , SEEK_SET);
		read(fd, buf_flash, UBOOT_SIZE);

		if( 0==memcmp(buf_udisk, buf_flash, UBOOT_SIZE) )
		{
			if(erase.length>UBOOT_SIZE)
			{
				erase.length -= UBOOT_SIZE;
				erase.start	 += UBOOT_SIZE;
			}
			bSameUboot = TRUE;

			printf("check uboot and 2 uboots is same!!!\n");

			printf("Just erase the mtd/0 from %x to %x, write size %x\n",
					erase.start, erase.start+erase.length, erase.length);

			rtn = fseek(fp_in, offset+UBOOT_SIZE, SEEK_SET);
			if (rtn < 0)
			{
				printf("fseek fail\n");
				fclose(fp_in);
				return UPDATE_FAILED_FILEERROR;
			}
			nMtdWrOft = UBOOT_SIZE;
			size = erase.length;

			tUpdate.totalpos -= UBOOT_SIZE;
		}
		else
		{
			printf("check uboot and 2 uboots different!!!\n");

			rtn = fseek(fp_in, offset, SEEK_SET);
			if (rtn < 0)
			{
				printf("fseek fail\n");
				fclose(fp_in);
				return UPDATE_FAILED_FILEERROR;
			}
		}
	}
	#endif

	printf("erase length = 0x%X\nstart erase...\n", erase.length);
	rtn = ioctl (fd, MEMERASE, &erase);
	if (rtn < 0)
	{
		perror("erase failed");
		free(buf);
		close(fd);
		return UPDATE_FAILED_FLASHERASE;
	}

	updated_size += size >> 1;

	printf("erase ok!\n");

	printf("start write flash...\n");

	#ifdef UPDATE_IGNORE_UBOOT
	lseek(fd, nMtdWrOft , SEEK_SET);
	#else
	lseek(fd, 0 , SEEK_SET);
	#endif

	SProgress sProg;

	sProg.lTotalSize = tUpdate.totalpos;

	while (size > 0)
	{
		len = (size > BUFSIZE) ? BUFSIZE : size;
		len = fread(buf, sizeof(char), len, fp_in);
		if (len < 0)
		{
			printf("write len = %d\n", len);
			free(buf);
			close(fd);
			return UPDATE_FAILED_FILEERROR;
		}

		write(fd, buf, len);
		usleep(5*1000);
		updated_size += (len >> 1);
		tUpdate.curpos = updated_size;
		size -= len;

		sProg.lCurSize = tUpdate.curpos;
		modSysCmplx_UpdateNotify(
			&sProg,
			EM_SYSEVENT_UPGRADE_RUN,
			EM_UPGRADE_RUNNING );
	}

	free(buf);
	printf("write flash ok!\n");
	close(fd);

	return UPDATE_SUCCESS;
}
#endif

int umount_user(char *user_path)
{
	if(user_path == NULL)
	{
		return -1;
	}

	char path[256];
	if(realpath(user_path,path) == NULL)
	{
		printf("umount_user %s:realpath failed 2\n", user_path);
		return -1;
	}
	printf("umount_user:path:%s\n",path);

	s32 curstat = umount(path);
	if(curstat)
	{
		if(errno == 22)//Invalid argument
		{
			printf("umount_user:umount failed1,errno=%d,error=%s\n",errno,strerror(errno));
			return 0;
		}
		else
		{
			printf("umount_user:umount failed2,errno=%d,error=%s\n",errno,strerror(errno));
			return -1;
		}
	}

	printf("umount_user:umount success\n");

	return 0;
}

void modsys_assert( char * file_name, u32 line_no )
{
	printf( "\n[MODSYS] Assert failed: %s, line %u\n",
	       file_name, line_no );
	abort( );
}

void modsysCmplx_SetProductModel(char* updatemodel)
{
	UpgradeSetModel(updatemodel);
}

void modsysCmplx_SetBreakBackup( int type )
{
	pthread_mutex_trylock(&sModSysIns.mtxBreakBackup);//cw_backup
	sModSysIns.bBreakBackup = type;
	pthread_mutex_unlock(&sModSysIns.mtxBreakBackup);
	/*
	if(!sModSysIns.bBreakBackup)
	{
		pthread_mutex_trylock(&sModSysIns.mtxBreakBackup);
		sModSysIns.bBreakBackup = TRUE;
		pthread_mutex_unlock(&sModSysIns.mtxBreakBackup);
	}
	*/
}

void modsysCmplx_ClrBreakBackup( void )
{
	if(sModSysIns.bBreakBackup)
	{
		pthread_mutex_trylock(&sModSysIns.mtxBreakBackup);
		sModSysIns.bBreakBackup = FALSE;
		pthread_mutex_unlock(&sModSysIns.mtxBreakBackup);
	}
}

BOOL modsysCmplx_QueryBreakBackup( void )
{	
	while(sModSysIns.bBreakBackup==2)//cw_backup
		usleep(500);
	return sModSysIns.bBreakBackup;
}

