#include "diskmanage.h"
#include "partitionindex.h"
#include "hddcmd.h"
#include "common.h"
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

#define PARAOUT
typedef char s8;
#include "mod_config.h"


//yaogang modify for bad disk
static char str_bad_disk_flag[] = "Able was I ere I saw Elba";
//yaogang modify for bad disk
typedef struct 
{
	char index;
	char b_valid;
	char b_in_system;
	u32 time;
	char disk_sn[64];//硬盘序列号、唯一
} SBadDiskItem;


static u8 disk_full_policy = DISK_FULL_COVER;


volatile static u8 g_HotPlugFlag = 0;

int set_disk_hotplug_flag(u8 flag)
{
	g_HotPlugFlag = flag;
	printf("set_disk_hotplug_flag:%d\n",flag);
	return 0;
}

//const static int index_phy2log[MAX_HDD_NUM+2] = {4, 10, 9, 8, 7, 5, 3, 6, 2, 1}; //MaoRong
//const static int index_phy2log[MAX_HDD_NUM+2] = {8, 6, 7, 9, 10, 3, 1, 2, 4, 5}; //YueTian 10 sata
//3个sata总线，前两个HUB 1扩5，只用到了4个，
//未用到的物理序号对应的逻辑序号从12开始
//E-STAT: phy 10, log 9
const static int index_phy2log[11] = {7, 5, 6, 8, 12, 3, 1, 2, 4, 13, 9}; //YueTian  8 sata



BOOL GetDiskSN(int fd, char *sn, int nLimit)
{
	struct hd_driveid id;
	if(ioctl(fd, HDIO_GET_IDENTITY, &id) < 0)
	{
		perror("HDIO_GET_IDENTITY");
		return FALSE;
	}
	
	strncpy(sn, (char *)id.serial_no, nLimit);
	//printf("Disk Number = %s\n", id.serial_no);
	return TRUE;
}

int GetDiskPhysicalIndex(int fd)
{
	int Bus = -1;
	if(ioctl(fd, SCSI_IOCTL_GET_BUS_NUMBER, &Bus) < 0)
	{
		perror("SCSI_IOCTL_GET_BUS_NUMBER failed");
		return -1;
	}
	//printf("###Bus:0x%08x\n",Bus);
	
	struct sg_id
	{
		long	l1;	/* target(scsi_device_id) | lun << 8 | channel << 16 | host_no << 24 */
		long	l2;	/* Unique id */
	}sg_id;
	
	if(ioctl(fd, SCSI_IOCTL_GET_IDLUN, &sg_id) < 0)
	{
		perror("SCSI_IOCTL_GET_IDLUN failed");
		return -1;
	}
	//printf("###id:(0x%08lx,0x%08lx)\n",sg_id.l1,sg_id.l2);
	
	int phy_idx = (Bus * 5) + ((sg_id.l1 >> 16) & 0xff);
	//printf("disk physical index = %d\n", phy_idx);
	
	return phy_idx;
}

int GetDiskLogicIndex(int phy_idx)
{
	int logic_idx = 255;
	int size = sizeof(index_phy2log)/sizeof(index_phy2log[0]);

	if (phy_idx < size)
	{
		logic_idx = index_phy2log[phy_idx];
	}
	else
	{
		printf("error: %s phy_idx(%d) out of range[0,%d)\n", 
			__func__, phy_idx, size);
	}
	
	return logic_idx;
}

BOOL is_sata_disk(int fd)
{
	char host[64];
	memset(host, 0, sizeof(host));
	
	*(int*)host = 63;
	if(ioctl(fd, SCSI_IOCTL_PROBE_HOST, &host) < 0)
	{
		perror("SCSI_IOCTL_PROBE_HOST failed");
		return FALSE;
	}
	
	//hdd:host=sata_sil hi3515-ahci-device ahci_platform
	//usb:host=SCSI emulation for USB Mass Storage devices
	host[63] = '\0';
	//printf("SCSI_IOCTL_PROBE_HOST-1:%s\n", host);
	if((strstr(host, "ahci")) || (strstr(host, "sata_sil")) || (strstr(host, "hi3515-ahci-device")) || (strstr(host, "atp862x_His")))
	{
		return TRUE;
	}
	
	return FALSE;
}

BOOL is_usb_disk(int fd)
{
	char host[64];
	memset(host, 0, sizeof(host));
	
	*(int*)host = 63;
	if(ioctl(fd, SCSI_IOCTL_PROBE_HOST, &host) < 0)
	{
		perror("SCSI_IOCTL_PROBE_HOST failed");
		return FALSE;
	}
	
	//hdd:host=sata_sil hi3515-ahci-device ahci_platform
	//usb:host=SCSI emulation for USB Mass Storage devices
	host[63] = '\0';
	//printf("SCSI_IOCTL_PROBE_HOST-2:%s\n", host);
	if(strstr(host, "USB") || strstr(host, "usb"))
	{
		return TRUE;
	}
	
	return FALSE;
}

//yaogang modify for bad disk
int is_mark_bad_by_config(const char *disk_sn)
{
	SModConfigBadDisk disk_item;
	int i;
	int ret = 0;
	
	for (i=0; i<MAX_HDD_NUM; ++i)
	{		
		ret = ModConfigGetParam(EM_CONFIG_BAD_DISK, &disk_item, i);
		if (ret)
		{
			printf("Error: %s ModConfigGetParam failed, i: %d\n", __func__, i);
			continue;
		}

		if (disk_item.time)//!= 0 : 表示坏盘表中该行记录有效
		{
			if (strstr(disk_sn, disk_item.disk_sn))
			{
				printf("%s (%s) = true\n", __func__, disk_sn);
				return 1;
			}
		}
	}

	printf("%s (%s) = false\n", __func__, disk_sn);
	return 0;
}


int is_in_system(disk_manager *hdd_manager, const char *disk_sn)
{
	HddInfo *phinfo = NULL;
	int i;
	
	for(i = 0; i < MAX_HDD_NUM; i++)
	{
		phinfo = &hdd_manager->hinfo[i];
		
		if(phinfo->is_disk_exist
			&& strstr(phinfo->disk_sn, disk_sn))
		{
			break;
		}
	}

	return i<MAX_HDD_NUM ? 1:0;
}
//将坏盘序列号和当前时间写入配置文件
/*
写入策略
1、如果配置文件坏盘表中还有空位，则写入空位
2、否则写入已经不在当前系统中且时间上最早的位置
*/
void mark_bad_disk_2_config(disk_manager *hdd_manager, const char *disk_sn)
{
	SBadDiskItem disk_list[MAX_HDD_NUM];
	SModConfigBadDisk disk_item;
	int i;
	int ret = 0;
	u32 earliest_time = 0;
	int earliest_line = -1;
	int first_invalid_line = -1;
	int write_rec_line = -1;

	memset(disk_list, 0, sizeof(disk_list));
	for (i=0; i<MAX_HDD_NUM; ++i)
	{
		disk_list[i].index = i;
		
		ret = ModConfigGetParam(EM_CONFIG_BAD_DISK, &disk_item, i);
		if (ret)
		{
			printf("Error: %s ModConfigGetParam failed, i: %d\n", __func__, i);
			return;
		}

		if (disk_item.time)//!= 0 : 表示坏盘表中该行记录有效
		{
			//printf("%d disk_sn: (%s)\n", i, disk_item.disk_sn);
			//已经记录了
			//测试发现disk_sn 前面有很多空格，而disk_item.disk_sn 没有
			//所以用strstr
			if (strstr(disk_sn, disk_item.disk_sn))
			{
				printf("%s disk_sn(%s) already marked\n", __func__, disk_sn);
				return ;
			}	
			
			disk_list[i].b_valid = 1;
			disk_list[i].time = disk_item.time;
			strcpy(disk_list[i].disk_sn, disk_item.disk_sn);
			disk_list[i].b_in_system = is_in_system(hdd_manager, disk_item.disk_sn);

			if (!disk_list[i].b_in_system) //已经不在当前系统中，找最早的记录
			{
				if (earliest_time == 0
					|| earliest_time > disk_list[i].time)
				{
					earliest_time = disk_list[i].time;
					earliest_line = i;					
				}
			}
		}
		else //== 0 : 表示坏盘表中该行记录无效
		{
			if (first_invalid_line == -1)
			{
				first_invalid_line = i;
			}
		}
	}

	if (first_invalid_line != -1)//有空记录位
	{
		write_rec_line = first_invalid_line;
	}
	else
	{
		write_rec_line = earliest_line;
	}

	printf("%s first_invalid_line: %d, earliest_line: %d, write_rec_line: %d\n",
		__func__, first_invalid_line, earliest_line, write_rec_line);
	
	if (write_rec_line != -1 && write_rec_line < MAX_HDD_NUM)
	{
		disk_item.time = time(NULL);
		strcpy(disk_item.disk_sn, disk_sn);
		
		ModConfigSetParam(EM_CONFIG_BAD_DISK, &disk_item, write_rec_line);
	}	
}

/*
如何标记坏盘：
	在磁盘保留区中 MBR 后的一个扇区(即第二扇区)写入特定一段文字，作为标记。
	这段特定文字是回文，出自拿破仑：“Able was I ere I saw Elba” (在我看到Elba岛之前，我曾所向无敌)
	
注意：执行磁盘格式化操作会擦除该标记
*/
int is_mark_bad_by_sector(const char *disk_name)
{
	char str[1024];
	int fd = open(disk_name, O_RDWR);
	
	if (fd < 0)
	{
		printf("Error: %s open %s failed\n", __func__, disk_name);
		return 1;
	}

	if (1024 != read(fd, str, 1024))
	{
		printf("Error: %s read failed\n", __func__);
		
		close(fd);
		return 1;
	}

	close(fd);

	char *pmark = str+512;//标记在第二扇区
	if (strncmp(pmark, str_bad_disk_flag, strlen(str_bad_disk_flag)) == 0)
	{
		printf("%s (%s) = true\n", __func__, disk_name);
		return 1;
	}

	printf("%s (%s) = false\n", __func__, disk_name);
	return 0;
}

void mark_bad_disk_2_sector(const char *disk_name)
{
	int fd = open(disk_name, O_RDWR);
	if (fd < 0)
	{
		printf("Error: %s open %s failed\n", __func__, disk_name);
		return ;
	}

	if (lseek(fd, 512, SEEK_SET) < 0)
	{		
		printf("Error: %s lseek failed\n", __func__);
		
		goto Done;		
	}

	if (sizeof(str_bad_disk_flag) != write(fd, str_bad_disk_flag, sizeof(str_bad_disk_flag)))
	{
		printf("Error: %s write failed\n", __func__);
		
		goto Done;
	}

	printf("%s to %s success\n", __func__, disk_name);
Done:
	
	close(fd);
}

//yaogang modify 20170218 in shanghai
int init_disk_manager(disk_manager *hdd_manager)
{
	int ret = -1;
	int fd = -1;
	int i = 0;
	
	DiskInfo dinfo;
	int nDiskFound = 0;
	int phy_idx = 0;
	u8 storage_type = 's';
			
	char DiskSN[64];	
	char diskname[64];
	char devname[64];
	char mountname[64];
	
	memset(hdd_manager,0,sizeof(disk_manager));
	
	int k = 0;
	for(k = 0; k < (MAX_HDD_NUM + 2); k++)
	{
		sprintf(diskname,"/dev/sd%c",'a'+k);
		ret = ifly_diskinfo(diskname, &dinfo);
		if(ret == 0)
		{
			fd = open(diskname, O_RDONLY);
			if(fd == -1)
			{
				printf("init_disk_manager %s is not exist\n",diskname);
				return -1;
			}
			
			storage_type = 's';
			
			if (is_sata_disk(fd))
			{
				printf("%s found HDD: %s, cap: %dM\n", __func__, diskname, (int)(dinfo.capability/(1024*1024)));
				
				storage_type = 's';			
				phy_idx = GetDiskPhysicalIndex(fd);

				memset(DiskSN, 0, sizeof(DiskSN));
				GetDiskSN(fd, DiskSN, sizeof(DiskSN)-1);				
				
				close(fd);
				fd = -1;
			}
			else
			{
				if (is_usb_disk(fd)) //U盘不是用来录像的
				{
					printf("%s found UDISK: %s, cap: %dM\n", __func__, diskname, (int)(dinfo.capability/(1024*1024)));
				}
				else
				{
					printf("error: %s found disk: %s, unknow type, neither HDD nor UDISK", __func__, diskname);
				}

				close(fd);
				fd = -1;
				continue;
			}

			/*
			int ret = is_sata_disk(fd);			
			if(!ret)
			{
				ret = is_usb_disk(fd);
				if(ret)
				{
					storage_type = 'u';
				}
				else //yaogang modify 20170218 in shanghai
				{
					printf("error: %s found HDD: %s, but neither HDD nor UDISK.", , __func__, diskname);

					close(fd);
					return -1;
				}
			}
			
			
			memset(DiskSN, 0, sizeof(DiskSN));
			
			if(storage_type == 's')
			{
				phy_idx = GetDiskPhysicalIndex(fd);
				GetDiskSN(fd, DiskSN, sizeof(DiskSN)-1);
			}
			
			close(fd);
			*/
			
			//yaogang modify 20170218 in shanghai
			/*
			if(!ret)//既不是SATA盘，也不是U盘或移动硬盘
			{
				return -1;
			}
			
			if(storage_type == 'u')//U盘不是用来录像的
			{
				continue;
			}
			*/
			//sleep(2);//这里有必要加延时吗???
			memset(&hdd_manager->hinfo[nDiskFound], 0, sizeof(HddInfo));//yaogang modify 20170218 in shanghai
			
			hdd_manager->hinfo[nDiskFound].is_disk_exist = 1;			
			hdd_manager->hinfo[nDiskFound].storage_type = storage_type;
			hdd_manager->hinfo[nDiskFound].disk_physical_idx = phy_idx;
			hdd_manager->hinfo[nDiskFound].disk_logic_idx = GetDiskLogicIndex(phy_idx);
			hdd_manager->hinfo[nDiskFound].disk_system_idx = k;
			strcpy(hdd_manager->hinfo[nDiskFound].disk_name, diskname);
			strcpy(hdd_manager->hinfo[nDiskFound].disk_sn, DiskSN);

			#if 1
			
			//坏盘判断
			// 1、第二扇区是否有坏盘标记
			// 2、配置文件是否有坏盘标记
			int is_mark_bad_by_sector_flag = is_mark_bad_by_sector(hdd_manager->hinfo[nDiskFound].disk_name);
			int is_mark_bad_by_config_flag = is_mark_bad_by_config(hdd_manager->hinfo[nDiskFound].disk_sn);
			
			if (is_mark_bad_by_sector_flag || is_mark_bad_by_config_flag)
			{
				hdd_manager->hinfo[nDiskFound].is_bad_disk = 1;

				if (!is_mark_bad_by_sector_flag)
				{
					mark_bad_disk_2_sector(hdd_manager->hinfo[nDiskFound].disk_name);
				}

				if (!is_mark_bad_by_config_flag)
				{
					mark_bad_disk_2_config(hdd_manager, hdd_manager->hinfo[nDiskFound].disk_sn);
				}
			}
			
			#endif
		
			for(i = 0; i < MAX_PARTITION_NUM; i++)
			{
				sprintf(devname,"%s%d",diskname,i+1);
				fd = open(devname, O_RDONLY);
				if (fd < 0)
				{
					printf("%s open %s failed, %s\n", __func__, devname, strerror(errno));

					//yaogang modify for bad disk
					//continue;
					break;
				}
				
				close(fd);

				 //yaogang modify 20170218 in shanghai
				//hdd_manager->hinfo[nDiskFound].is_partition_exist[i] = 1;
				sprintf(mountname,"rec/%c%d",'a'+nDiskFound,i+1);//newdisk???
				
				int rtn = mkdir(mountname,1);
				if(rtn)
				{
					//已经存在。 机器在客户那边是不会出现的，
					//只会出现在测试阶段，不重启系统只重新启动myapp
					if (EEXIST == errno)
					{
						umount_user(mountname);
					}
					else
					{
						printf("%s mkdir %s failed, errno: %d,%s\n", __func__, mountname, errno, strerror(errno));

						break;
					}
				}
				
				//printf("before mount_user_0:%s...\n",devname);
				if (0 != mount_user(devname, mountname))
				{
					break;
				}
				//printf("after mount_user_0:%s...\n",devname);
				
				if (1 == init_partition_index(&hdd_manager->hinfo[nDiskFound].ptn_index[i], mountname))
				{//printf("after init_partition_index_0:%s...\n",mountname);
				
					hdd_manager->hinfo[nDiskFound].disk_total += (u32)(get_partition_total_space(&hdd_manager->hinfo[nDiskFound].ptn_index[i])/1000);
					hdd_manager->hinfo[nDiskFound].disk_free += (u32)(get_partition_free_space(&hdd_manager->hinfo[nDiskFound].ptn_index[i])/1000);

					hdd_manager->hinfo[nDiskFound].is_partition_exist[i] = 1;
					
					printf("%s init_partition_index %s success\n", __func__, devname);
				}
				else
				{
					printf("%s init_partition_index %s failed\n", __func__, devname);

					break;
				}
			}
			
			//yaogang modify for bad disk
			if (i < MAX_PARTITION_NUM)//期间出错
			{					
				for(i = 0; i < MAX_PARTITION_NUM; i++)
				{	
					if (hdd_manager->hinfo[nDiskFound].is_partition_exist[i])
					{
						destroy_partition_index(&hdd_manager->hinfo[nDiskFound].ptn_index[i]);
						sprintf(mountname,"rec/%c%d",'a'+nDiskFound,i+1);
						umount_user(mountname);

						hdd_manager->hinfo[nDiskFound].is_partition_exist[i] = 0;
					}
				}

				hdd_manager->hinfo[nDiskFound].disk_total = 0;
				hdd_manager->hinfo[nDiskFound].disk_free = 0;
			}
			
			
			nDiskFound++;
			if(nDiskFound == MAX_HDD_NUM)
			{
				break;
			}
		}
	}
	
	printf("%s: nDiskFound = %d\n", __func__, nDiskFound);
	
	return 1;
}

int get_disk_info(disk_manager *hdd_manager,int disk_index)
{
	int i;
	//printf("get_disk_info\n");
	dbgprint("get_disk_info:i addr:0x%08x,disk_index addr:0x%08x,hdd_manager addr:0x%08x\n",(int)&i,(int)&disk_index,(int)&hdd_manager);
	HddInfo *phinfo = &hdd_manager->hinfo[disk_index];
	if(phinfo->is_disk_exist)
	{
		phinfo->disk_total = 0;
		phinfo->disk_free = 0;
		for(i=0;i<MAX_PARTITION_NUM;i++)
		{
			//printf("%s yg 1\n", __func__);
			phinfo->disk_total += (u32)(get_partition_total_space(&phinfo->ptn_index[i])/1000);
			//printf("%s yg 2\n", __func__);
			phinfo->disk_free += (u32)(get_partition_free_space(&phinfo->ptn_index[i])/1000);
			//printf("%s disk%d:total=%ld,free=%ld\n",__func__,disk_index,phinfo->total,phinfo->free);
			dbgprint("disk%d:total=%ld,free=%ld\n",disk_index,phinfo->disk_total,phinfo->disk_free);
		}
	}
	return 1;
}

partition_index* get_rec_path(disk_manager *hdd_manager,char *pPath,u32 *open_offset,int chn,u8 *pdisk_system_idx)
{
	int i,j;
	int file_no,sect_offset;
	HddInfo *phinfo;
	u32 min_end_time = (u32)(-1);
	u32 end_time;
	int cover_disk = -1;
	int cover_ptn = -1;
	
	for(i=0;i<MAX_HDD_NUM;i++)
	{
		phinfo = &hdd_manager->hinfo[i];
		//if(phinfo->is_disk_exist)
		if(phinfo->is_disk_exist && !phinfo->is_bad_disk)//yaogang modify for bad disk
		{
			for(j=0;j<MAX_PARTITION_NUM;j++)
			{
				if(phinfo->is_partition_exist[j])
				{
					dbgprint("before:get_chn_next_segment\n");
					if(get_chn_next_segment(&phinfo->ptn_index[j],chn,&file_no,&sect_offset))
					{
						dbgprint("after1:get_chn_next_segment\n");
						dbgprint("disk partition=sd%c%d,file_no=%d,sect_offset=%d,chn=%d\n",'a'+i,j+1,file_no,sect_offset,chn);
						//printf("disk partition=sd%c%d,file_no=%d,sect_offset=%d,chn=%d\n",'a'+i,j+1,file_no,sect_offset,chn);
						//printf("%s:%u--%s\n", __func__, phinfo->disk_system_idx, phinfo->disk_name);
						sprintf(pPath,"rec/%c%d/fly%05d.ifv",'a'+i,j+1,file_no);
						*open_offset = sect_offset;//新文件在文件容器中的开始位置
						dbgprint("***no cover:new record file name:%s,open_offset=%d,chn=%d***\n",pPath,*open_offset,chn);
						
						*pdisk_system_idx = phinfo->disk_system_idx+1;//sda--1 sdb--2
						
						return &phinfo->ptn_index[j];
					}
					dbgprint("after2:get_chn_next_segment\n");
				}
			}
		}
	}
	if(disk_full_policy == DISK_FULL_COVER)
	{
		for(i=0;i<MAX_HDD_NUM;i++)
		{
			phinfo = &hdd_manager->hinfo[i];
			//if(phinfo->is_disk_exist)
			if(phinfo->is_disk_exist && !phinfo->is_bad_disk)//yaogang modify for bad disk
			{
				for(j=0;j<MAX_PARTITION_NUM;j++)
				{
					if(phinfo->is_partition_exist[j])
					{
						if(get_first_full_file_end_time(&phinfo->ptn_index[j],&end_time))
						{
							if(end_time < min_end_time)
							{
								min_end_time = end_time;
								cover_disk = i;
								cover_ptn = j;
								dbgprint("******************* get_first_full_file_end_time ok.\n");
							}
						}
					}
				}
			}
		}
	}
	if(cover_disk != -1)
	{
		phinfo = &hdd_manager->hinfo[cover_disk];
		if(get_chn_next_segment_force(&phinfo->ptn_index[cover_ptn],chn,&file_no,&sect_offset))
		{
			dbgprint("disk partition=sd%c%d,file_no=%d,sect_offset=%d,chn=%d\n",'a'+cover_disk,cover_ptn+1,file_no,sect_offset,chn);
				sprintf(pPath,"rec/%c%d/fly%05d.ifv",'a'+cover_disk,cover_ptn+1,file_no);
			*open_offset = sect_offset;
			dbgprint("***cover:new record file name:%s,open_offset=%d,chn=%d***\n",pPath,*open_offset,chn);
			//printf("***cover:new record file name:%s,open_offset=%d,chn=%d***\n", pPath,*open_offset,chn);
			//解决所有硬盘录满后所有硬盘的状态显示空闲，但实际上依然在录像
			*pdisk_system_idx = phinfo->disk_system_idx+1;//sda--1 sdb--2
			
			return &phinfo->ptn_index[cover_ptn];
		}
	}
	return NULL;
}

int set_policy_when_disk_full(u8 policy)
{
	if(policy >= DISK_FULL_COVER) policy = DISK_FULL_COVER;
	disk_full_policy = policy;
	return 1;
}

void run(recfileinfo_t* pData,int left,int right)
{
	int i,j;
	u32 middle;
	recfileinfo_t iTemp;
	i = left;
	j = right;
	middle = pData[(left+right)/2].start_time;//pData[(left+right)/2]; //求中间值
	do{
		while((pData[i].start_time > middle) && (i < right))//从左扫描小于中值的数
			i++;
		while((pData[j].start_time < middle) && (j > left))//从右扫描小于中值的数
			j--;
		if(i<=j)//找到了一对值
		{
			//交换
			iTemp = pData[j];
			pData[j] = pData[i];
			pData[i] = iTemp;
			i++;
			j--;
		}
	}while(i<=j);//如果两边扫描的下标交错，就停止（完成一次）
	//当左边部分有值(left<j)，递归左半边
	if(left<j)
		run(pData,left,j);
	//当右边部分有值(right>i)，递归右半边
	if(right>i)
		run(pData,i,right);
}

void QuickSort(recfileinfo_t * pData,int count)
{
	run(pData,0,count-1);
}

int sort_file_with_start_time(recfileinfo_t *fileinfo_buf,int nums)
{
	//printf("sort 1\n");
	QuickSort(fileinfo_buf,nums);
	//printf("sort 2\n");
	return 1;
	
	int i,j;
	recfileinfo_t tmp;
	for(i=0;i<nums-1;i++)
	{
		for(j=0;j<nums-1-i;j++)
		{
			//if(fileinfo_buf[j].start_time > fileinfo_buf[j+1].start_time)
			if(fileinfo_buf[j].start_time < fileinfo_buf[j+1].start_time)//wrchen 081223
			{
				tmp = fileinfo_buf[j];
				fileinfo_buf[j] = fileinfo_buf[j+1];
				fileinfo_buf[j+1] = tmp;
			}
		}
	}
}

//yaogang
void snaprun(recsnapinfo_t* pData,int left,int right)
{
	int i,j;
	u32 middle;
	recsnapinfo_t iTemp;
	i = left;
	j = right;
	middle = pData[(left+right)/2].start_time;//pData[(left+right)/2]; //求中间值
	do{
		while((pData[i].start_time > middle) && (i < right))//从左扫描大于中值的数
			i++;
		while((pData[j].start_time < middle) && (j > left))//从右扫描小于中值的数
			j--;
		if(i<=j)//找到了一对值
		{
			//交换
			iTemp = pData[j];
			pData[j] = pData[i];
			pData[i] = iTemp;
			i++;
			j--;
		}
	}while(i<=j);//如果两边扫描的下标交错，就停止（完成一次）
	//当左边部分有值(left<j)，递归左半边
	if(left<j)
		snaprun(pData,left,j);
	//当右边部分有值(right>i)，递归右半边
	if(right>i)
		snaprun(pData,i,right);
}

void SnapQuickSort(recsnapinfo_t * pData,int count)
{
	snaprun(pData,0,count-1);
}

int sort_snap_with_start_time(recsnapinfo_t *fileinfo_buf,int nums)
{
	//printf("sort 1\n");
	SnapQuickSort(fileinfo_buf,nums);
	//printf("sort 2\n");
	return 1;
	/*
	int i,j;
	recsnapinfo_t tmp;
	for(i=0;i<nums-1;i++)
	{
		for(j=0;j<nums-1-i;j++)
		{
			//if(fileinfo_buf[j].start_time > fileinfo_buf[j+1].start_time)
			if(fileinfo_buf[j].start_time > fileinfo_buf[j+1].start_time)//wrchen 081223
			{
				tmp = fileinfo_buf[j];
				fileinfo_buf[j] = fileinfo_buf[j+1];
				fileinfo_buf[j+1] = tmp;
			}
		}
	}
	*/
}


#ifndef WIN32
#include <sys/time.h>
#endif

//#define PRINT_SEARCH_TIME

//csp modify 20140822
//extern int tl_power_atx_check();

int search_all_rec_file(disk_manager *hdd_manager,search_param_t *search,recfileinfo_t *fileinfo_buf,int max_nums)
{
	u8 i,j;
	HddInfo *phinfo;
	int search_count = 0;
	int ret = 0;
	
	//csp modify 20140822
	if(search->end_time < search->start_time)
	{
		return 0;
	}
	//csp modify 20140822
	/*
	unsigned char cur_atx_flag = tl_power_atx_check();
	if(cur_atx_flag == 0)//电池供电
	{
		return 0;
	}
	*/
	//csp modify 20140822
	if(g_HotPlugFlag)
	{
		return 0;
	}
	
	#ifdef PRINT_SEARCH_TIME
	struct timeval start,end;
	long span;
	#endif
	
	for(i=0;i<MAX_HDD_NUM;i++)
	{
		phinfo = &hdd_manager->hinfo[i];
		
		if(phinfo->is_disk_exist)
		{
			for(j=0;j<MAX_PARTITION_NUM;j++)
			{
				if(phinfo->is_partition_exist[j])
				{
					//printf("start search chn is %d\n",search->channel_no);
					ret = search_rec_file(&phinfo->ptn_index[j], search, fileinfo_buf + search_count, max_nums-search_count, i, j);
					
					if(ret < 0)
					{						
						#ifdef PRINT_SEARCH_TIME
						gettimeofday(&start,NULL);
						#endif
						
						sort_file_with_start_time(fileinfo_buf,max_nums);//wrchen 090520
						
						#ifdef PRINT_SEARCH_TIME
						gettimeofday(&end,NULL);
						span = (end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);
						printf("sort 1.0 files,%d,span:%ld\n",max_nums,span);
						#endif
						
						return -1;
					}
					else if((max_nums-search_count) == ret)
					{						
						search_count += ret;
						sort_file_with_start_time(fileinfo_buf,search_count);//wrchen 090520
						return search_count;
					}
					search_count += ret;
				}
			}
		}
	}
		
	if(search_count)
	{
		#ifdef PRINT_SEARCH_TIME
		gettimeofday(&start,NULL);
		#endif
		
		sort_file_with_start_time(fileinfo_buf,search_count);
		
		#ifdef PRINT_SEARCH_TIME
		gettimeofday(&end,NULL);
		span = (end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);
		printf("sort 1.1 %d files,span:%ld\n",search_count,span);
		#endif
	}
	
	return search_count;
}

int get_rec_file_mp4_name(recfileinfo_t *fileinfo,char *filename,u32 *open_offset)
{
	*open_offset = fileinfo->offset;
	sprintf(filename,"rec/%c%d/fly%05d.mp4",'a'+fileinfo->disk_no,fileinfo->ptn_no+1,fileinfo->file_no);
	return 1;
}

int get_rec_file_name(recfileinfo_t *fileinfo,char *filename,u32 *open_offset)
{
	*open_offset = fileinfo->offset;
	sprintf(filename,"rec/%c%d/fly%05d.ifv",'a'+fileinfo->disk_no,fileinfo->ptn_no+1,fileinfo->file_no);
	return 1;
}

int get_snap_file_name(recsnapinfo_t *fileinfo,char *filename,u32 *open_offset)
{
	*open_offset = fileinfo->offset;
	sprintf(filename,"rec/%c%d/pic%05d.ifv",'a'+fileinfo->disk_no,fileinfo->ptn_no+1,fileinfo->file_no);
	return 1;
}


//yaogang modify 20150105 for snap
partition_index * get_partition(disk_manager *hdd_manager, u8 nDiskNo, u8 nPtnNo)
{
	HddInfo *phinfo = NULL;
	partition_index *index = NULL;

	if (hdd_manager)
	{
		phinfo = &hdd_manager->hinfo[nDiskNo];
		if(phinfo->is_disk_exist)
		{
			index = &phinfo->ptn_index[nPtnNo];
			if (index->valid == 0)
			{
				index = NULL;
			}
		}
	}
	return index;
}

partition_index * get_pic_rec_partition(disk_manager *hdd_manager)
{
	HddInfo *phinfo;
	partition_index *index = NULL;
	int i, j;

	partition_index *earliest_index = NULL;
	time_t tmin = 0;
	
	for(i=0;i<MAX_HDD_NUM;i++)
	{
		phinfo = &hdd_manager->hinfo[i];
		//yaogang modify for bad disk
		if(phinfo->is_disk_exist && !phinfo->is_bad_disk)
		{
			for(j=0;j<MAX_PARTITION_NUM;j++)
			{
				if(phinfo->is_partition_exist[j])//phinfo->ptn_index[j]
				{
					index = &phinfo->ptn_index[j];
					if (index->valid)
					{
						lock_partition_index(index);
						//在一次循环中顺便找到结束时间最早的分区，以便后面使用
						if (NULL == earliest_index)
						{
							earliest_index = index;
							tmin = index->pic_header.end_sec;
						}
						else
						{
							if (tmin > index->pic_header.end_sec)
							{
								earliest_index = index;
								tmin = index->pic_header.end_sec;
							}
						}
						//该分区还有未用完的文件容器
						if (index->pic_header.file_cur_no < index->pic_header.file_nums)
						{
							printf("find partition: %s%d, file_cur_no: %d\n", phinfo->disk_name, j, index->pic_header.file_cur_no);
							unlock_partition_index(index);
							return index;
						}
						unlock_partition_index(index);
					}
				}
			}
		}
	}
	//所有分区的文件容器都用完了
	//回卷，找到结束时间最早的分区
	//从该分区第1 个文件容器开始覆盖
	//并且此时该分区的其他文件容器也无效了
	printf("all of partition full, Rollback\n");	
	
	if (NULL == earliest_index)
	{
		printf("Rollback failed, system disk not exist\n");
	}
	else
	{
		init_partition_pic_index(earliest_index);
	}
	return earliest_index;
}

int search_all_rec_snap(disk_manager *hdd_manager,search_param_t *search,recsnapinfo_t *snapinfo_buf,int max_nums)
{
	u8 i,j;
	HddInfo *phinfo;
	int search_count = 0;
	int ret = 0;
	
	//csp modify 20140822
	if(search->end_time < search->start_time)
	{
		return 0;
	}
	//csp modify 20140822
	/*
	unsigned char cur_atx_flag = tl_power_atx_check();
	if(cur_atx_flag == 0)//电池供电
	{
		return 0;
	}
	*/
	//csp modify 20140822
	if(g_HotPlugFlag)
	{
		return 0;
	}
	
	#ifdef PRINT_SEARCH_TIME
	struct timeval start,end;
	long span;
	#endif
	
	for(i=0;i<MAX_HDD_NUM;i++)
	{
		phinfo = &hdd_manager->hinfo[i];
		
		if(phinfo->is_disk_exist)
		{
			for(j=0;j<MAX_PARTITION_NUM;j++)
			{
				if(phinfo->is_partition_exist[j])
				{
					//printf("start search chn is %d\n",search->channel_no);
					ret = search_rec_snap(&phinfo->ptn_index[j], search, snapinfo_buf + search_count, max_nums-search_count, i, j);
					
					if(ret < 0)
					{						
						#ifdef PRINT_SEARCH_TIME
						gettimeofday(&start,NULL);
						#endif
						
						sort_snap_with_start_time(snapinfo_buf,max_nums);//wrchen 090520
						
						#ifdef PRINT_SEARCH_TIME
						gettimeofday(&end,NULL);
						span = (end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);
						printf("sort 1.0 files,%d,span:%ld\n",max_nums,span);
						#endif
						
						return -1;
					}
					else if((max_nums-search_count) == ret)
					{						
						search_count += ret;
						sort_snap_with_start_time(snapinfo_buf,search_count);//wrchen 090520
						return search_count;
					}
					search_count += ret;
				}
			}
		}
	}
		
	if(search_count)
	{
		#ifdef PRINT_SEARCH_TIME
		gettimeofday(&start,NULL);
		#endif
		
		sort_snap_with_start_time(snapinfo_buf,search_count);
		
		#ifdef PRINT_SEARCH_TIME
		gettimeofday(&end,NULL);
		span = (end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);
		printf("sort 1.1 %d files,span:%ld\n",search_count,span);
		#endif
	}
	
	return search_count;
}

//预录
//第一个硬盘的第一个分区的126/127文件容器做预录
partition_index * get_pre_rec_partition(disk_manager *hdd_manager)
{
	HddInfo *phinfo;
	partition_index *index = NULL;
	int i, j;
	
	for(i=0;i<MAX_HDD_NUM;i++)
	{
		phinfo = &hdd_manager->hinfo[i];
		//yaogang modify for bad disk
		//if(phinfo->is_disk_exist)
		if(phinfo->is_disk_exist && !phinfo->is_bad_disk)
		{
			for(j=0;j<MAX_PARTITION_NUM;j++)
			{
				if(phinfo->is_partition_exist[j])//phinfo->ptn_index[j]
				{
					index = &phinfo->ptn_index[j];
					if (index->valid)
					{
						return index;
					}
				}
			}
		}
	}

	return NULL;
}

