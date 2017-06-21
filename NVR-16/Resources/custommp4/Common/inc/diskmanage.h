#ifndef _DISK_MANAGE_H_
#define _DISK_MANAGE_H_

#include "partitionindex.h"
//#include "ctrlprotocol.h"

#define HISI_3515//debug temp

#define MAX_HDD_NUM 		8
#define MAX_PARTITION_NUM	2//4		//3T4T

#define	DISK_FULL_STOP		0
#define DISK_FULL_COVER		1

typedef struct
{
	u8 is_disk_exist;
	u8 is_bad_disk;//yaogang modify for bad disk
	u8 is_partition_exist[MAX_PARTITION_NUM];
	partition_index ptn_index[MAX_PARTITION_NUM];
	u32 disk_total;
	u32 disk_free;
	u8 storage_type;//'s'->sata, 'u'->'usb'
	u8 disk_physical_idx;//主板上硬盘的物理编号
	u8 disk_logic_idx;//主板上硬盘的逻辑编号
	u8 disk_system_idx;//系统检测到的硬盘编号sda->0, sdb->1...
	char disk_name[32];//硬盘设备名/dev/sda, /dev/sdb...
	char disk_sn[64];//硬盘序列号、唯一
}HddInfo;

typedef struct
{
	HddInfo hinfo[MAX_HDD_NUM];
}disk_manager;

typedef struct
{
	u8 hformat[MAX_HDD_NUM];
}disk_is_format;

int init_disk_format_flagfile(disk_is_format *hdd_format,disk_manager *hdd_manager);
int set_policy_when_disk_full(u8 policy);
int init_disk_manager(disk_manager *hdd_manager);
int get_disk_info(disk_manager *hdd_manager,int disk_index);

//pw 2010/10/20
int get_udisk_info(disk_manager *hdd_manager);

int search_all_rec_file(disk_manager *hdd_manager,search_param_t *search,recfileinfo_t *fileinfo_buf,int max_nums);
partition_index* get_rec_path(disk_manager *hdd_manager,char *pPath,u32 *open_offset,int chn,u8 *pdisk_system_idx);
int get_rec_file_mp4_name(recfileinfo_t *fileinfo,char *filename,u32 *open_offset);	
int get_rec_file_name(recfileinfo_t *fileinfo,char *filename,u32 *open_offset);
BOOL is_sata_disk(int fd);
BOOL is_usb_disk(int fd);
int GetDiskPhysicalIndex(int fd);
int GetDiskLogicIndex(int phy_idx);
BOOL GetDiskSN(int fd, char *sn, int nLimit);

int set_disk_hotplug_flag(u8 flag);

//yaogang modify 20141225
int get_snap_file_name(recsnapinfo_t *fileinfo,char *filename,u32 *open_offset);
partition_index * get_pic_rec_partition(disk_manager *hdd_manager);
int search_all_rec_snap(disk_manager *hdd_manager,search_param_t *search,recsnapinfo_t *snapinfo_buf,int max_nums);

//yaogang modify 20150105 for snap
partition_index * get_partition(disk_manager *hdd_manager, u8 nDiskNo, u8 nPtnNo);

//预录
//第一个硬盘的第一个分区的126/127文件容器做预录
partition_index * get_pre_rec_partition(disk_manager *hdd_manager);


#endif

