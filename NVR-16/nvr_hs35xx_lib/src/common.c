/******************************************************************************

	TL hslib local common
	
	2008-12-16	created by kong
	2010-11-11	modify by andyrew
	
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "hi_comm_vo.h"
#include "hi_comm_vi.h"
#include "hi_comm_venc.h"
#include "mpi_vb.h"
#include "mpi_sys.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_venc.h"

#include "common.h"

int tw286x_rd(int chip_id, unsigned int adr, unsigned char *out_val)
{
	unsigned int buf[DBG_MEM_RW_SIZE];
	
	buf[1] = chip_id;
	buf[2] = adr;
	
	//return 0;//csp modify
	
	if(ioctl(plib_gbl_info->fd_tw286x, TL_2865_RD, buf) < 0)
	{
		printf("common.c@tw286x_rd: TL_2865_RD error\n");
		return -1;
	}
	*out_val = buf[0];
	
	//printf("chip id:%d, adr:%x, data:%x\n\n", chip_id, adr, *out_val);
	
	return 0;
}

int tw286x_wr(int chip_id, unsigned int adr, unsigned char data)
{
	unsigned int buf[DBG_MEM_RW_SIZE];
	
	buf[1] = chip_id;
	buf[2] = adr;
	buf[3] = data;
	
	//return 0;//csp modify
    
	if(ioctl(plib_gbl_info->fd_tw286x, TL_2865_WR, buf) < 0)
	{
		printf("common.c@tw286x_wr: TL_2865_RD error\n");
		return -1;
	}
    
	//printf("chip id:%d, adr:%x, data:%x\n\n", chip_id, adr, data);
    
	return 0;
}

//csp modify
int nvp_wr(int chip_id, int bankvalue, unsigned int adr, unsigned char data)
{
	I2C_BANK_LOCK();
	
	tw286x_wr(chip_id, 0xFF, bankvalue);
	tw286x_wr(chip_id, adr, data);
	
	I2C_BANK_UNLOCK();
	
	return 0;
}

//csp modify
int nvp_rd(int chip_id, int bankvalue, unsigned int adr, unsigned char *out_val)
{
	I2C_BANK_LOCK();
	
	tw286x_wr(chip_id, 0xFF, bankvalue);
	tw286x_rd(chip_id, adr, out_val);
	
	I2C_BANK_UNLOCK();
	
	return 0;
}

void hs3521_wr(unsigned int reg, unsigned int data)
{
	unsigned int buf[DBG_MEM_RW_SIZE] = {0};
	
	buf[0] = 0x00;
	buf[1] = reg;
	buf[2] = data;
	
	if(ioctl(plib_gbl_info->fd_tl, TL_DEV_REG_RW, buf) < 0)
	{
		printf("hs3521_wr error!\n");
	}
}

unsigned int hs3521_rd(unsigned int reg, unsigned int *val)
{
	unsigned int buf[DBG_MEM_RW_SIZE] = {0};
	
	buf[0] = 0x01;
	buf[1] = reg;
	
	if(ioctl(plib_gbl_info->fd_tl, TL_DEV_REG_RW, buf) < 0)
	{
		printf("hs3521_rd error!\n");
	}
	
	*val = buf[0];
	
	return 0;
}

