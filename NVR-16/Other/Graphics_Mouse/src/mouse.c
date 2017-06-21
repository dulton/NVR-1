
#include "DVRDEF.H"

#include "APIs/Mouse.h"

#include "tl_mouse.h"

static int fd = -1;

int MouseCreate(void)
{
	if (fd > 0)
	{
		return 0;
	}
	
#if 1//csp modify 20140207//支持无线鼠标
	if ((fd = open("/dev/event1", O_RDONLY)) < 0)//O_RDWR
	{
		if ((fd = open(DEV_MOUSE, O_RDONLY)) < 0)
		{
			//printf("open mouse dev failed!\n");
			return -1;
		}
		else
		{
			//printf("open mouse success.\n");
		}
	}
	else
	{
		//printf("open mouse succed!!!\n");
	}
#else
	if ((fd = open(DEV_MOUSE, O_RDONLY)) < 0)//O_RDWR
	{
		return -1;
	}
#endif
    
	return 0;
}

int MouseDestory(void)
{   
	int ret;

	ret = close(fd);
    
	fd = -1;

	return ret; 

}


int MouseGetData(MOUSE_DATA *pData)
{
	int ret, nread;
	
	if(fd < 0)
	{
	    if(MouseCreate() < 0)
	    {
	        return -1;
	    }
	}
	
#if 0
	fd_set action_fds;
	struct timeval tv;

	FD_ZERO(&action_fds);
	FD_SET(fd, &action_fds);
	
	tv.tv_sec  = 0;
	tv.tv_usec = 200*1000;


	int ret = select(fd + 1, &action_fds, NULL, NULL, &tv);
		
	if (ret <= 0)
	{
		
		if(ret < 0)
		{
			printf("select error:wait user event,ret=%d,errcode=%d,errstr=%s,mouseFd=%d\n",ret,errno,strerror(errno), fd);
			
			if(fd != -1 && FD_ISSET(fd,&action_fds))
			{
				printf("mouse error\n");
			}
		}
		
		return -1;
	}
#endif
	
#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
	struct mouse_input_event ev;
	nread = read(fd, &ev, sizeof(struct mouse_input_event));    
	if (nread > 0)
	{
	    if(nread != sizeof(struct mouse_input_event))
	    {
	        printf("read mouse failed nread=%d sizeof(struct mouse_input_event)=%d\n"
	            , nread, sizeof(struct mouse_input_event));
	        return -1;
	    }	
		
		//printf("type[%d],code[%d],val[%d]\n",ev.type,ev.code,ev.value);
		
		unsigned char tmp = pData->key;
		memset(pData, 0, sizeof(MOUSE_DATA));
		pData->key= tmp;
		if(ev.type == 1)//按键操作
		{
			if(ev.code == 272)//左键
			{
				if(ev.value)
				{
					pData->key |= 0x1;
					//printf("Lbutton down\n");
				}
				else
				{
					pData->key &= ~0x1;
					//printf("Lbutton up\n");
				}
			}
			else if(ev.code == 273)//右键
			{
				if(ev.value)
				{
					pData->key |= 0x2;
				}
				else
				{
					pData->key &= ~0x2;
				}
			}
			else if(ev.code == 274)//中键
			{
				if(ev.value)
				{
					pData->key |= 0x4;
				}
				else
				{
					pData->key &= ~0x4;
				}
			}
			else
			{
				return -2;
			}
		}
		else if(ev.type == 2)//move操作
		{
			if(ev.code == 0)//左右移动
			{
				pData->x = (short)ev.value;
			}
			else if(ev.code == 1)//上下移动
			{
				pData->y = (short)-ev.value;
			}
			else if(ev.code == 8)//滚轮
			{
				pData->z = (short)-ev.value;
			}
			else
			{
				return -2;
			}
		    //pData->ax	= ev.abs_x;
		    //pData->ay	= ev.abs_y;
	    }
	    else
	    {
	    	return -2;
	    }
	    ret = 0;
	}
#else
	printf("mouse hi3531 else\n");
	struct mousedev_motion ev;
	nread = read(fd, &ev, sizeof(struct mousedev_motion));    
	if (nread > 0)
	{
	    if(nread != sizeof(struct mousedev_motion))
	    {
	        printf("read mouse failed nread=%d sizeof(struct mousedev_motion)=%d\n"
	            , nread, sizeof(struct mousedev_motion));
	        return -1;
	    }

	    pData->key	= (ev.buttons & 0x7);
	printf("mouse key: 0x%x\n", pData->key);
	    pData->x = (short)ev.dx;
	    pData->y = (short)-ev.dy;
	    pData->z = (short)ev.dz;

	    pData->ax	= ev.abs_x;
	    pData->ay	= ev.abs_y;

	    ret = 0;
	}
#endif
	else
	{
	    MouseDestory();
	    ret = -1;
	}

	return ret;
} 

