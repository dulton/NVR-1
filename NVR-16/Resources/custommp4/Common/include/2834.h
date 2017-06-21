#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sched.h>
#include <pthread.h>

#include <asm/types.h>

#include <linux/videodev2.h>
#include <linux/fb.h>

/* Function error codes */
#define SUCCESS        0
#define FAILURE        -1
#define ERR(fmt, args...) fprintf(stderr, "Encode Error: " fmt, ## args)

#define VIDEO_DISP_MODE			_IOR('V',BASE_VIDIOC_PRIVATE+21,int)
#define AUDIO_SET_VOLUME		_IOR('V',BASE_VIDIOC_PRIVATE+22,int)
#define FPGA_SPI_INIT			_IOR('V',BASE_VIDIOC_PRIVATE+23,int)
#define FPGA_SPI_SELECT_CHIP	_IOR('V',BASE_VIDIOC_PRIVATE+24,int)
#define FPGA_SPI_WAIT_INITB		_IOR('V',BASE_VIDIOC_PRIVATE+25,int)
#define FPGA_SPI_WAIT_DONE		_IOR('V',BASE_VIDIOC_PRIVATE+26,int)
#define FPGA_SPI_WRITE_ABIT	_IOR('V',BASE_VIDIOC_PRIVATE+27,int)
#define FPGA_SPI_WRITE_ABYTE	_IOR('V',BASE_VIDIOC_PRIVATE+28,int)
#define FBIO_MYFUN				_IOR('V',BASE_VIDIOC_PRIVATE+29,int)

#define	MASTER					0x00
#define	SLAVE1					0x1
#define SLAVE2					0x2
#define SLAVE3					0x3

//全局变量
static int gQuit, gFbfd;

//pci boot函数，读取dps配置空间，然后 分析文件，下载文件
void pciboot(void)
{
	FILE *myfd;
//	FILE *testfd;
	int i, j;	
	int retvalue;	
	int testint[6];
	int entrypoint, uisectionsize, uiloadaddr, uialteraaddr, rawdata, runaddr, filedata;

	//打开文件
	myfd = fopen(DSP_BIN_FILE, "rb");
	if (!myfd)
	{
		printf("\n dsp boot  open file error");
		return;
	}

	// get the dsp addr
	//map the windown 0 l2 ram
	testint[0] = 0x10;
	if (ioctl(gFbfd, FPGA_MAP_DRIVER_SELE,testint) == -1) 
		if (errno == EINVAL) {
		ERR("\n dsp boot get memory map error");

		//printf("the value is %x",testint[2]);
		}
			  
	       dsp_windown0 = (char *)mmap(0,0x800000,PROT_READ | PROT_WRITE, MAP_SHARED, gFbfd,0x0);

                 if(dsp_windown0==NULL)
	           {
                   		  printf("map failed~!");
			        return FAILURE;
                   }



	     //map the windown 1 ddr control
              testint[0] = 0x11;
		if (ioctl(gFbfd, FPGA_MAP_DRIVER_SELE,testint) == -1) 
              if (errno == EINVAL) {
                  ERR("\n dsp boot get memory map error");

			//printf("the value is %x",testint[2]);
		}
			  
	       dsp_windown1 = (char *)mmap(0,0x800000,PROT_READ | PROT_WRITE, MAP_SHARED, gFbfd,0x0);

                 if(dsp_windown1 ==NULL)
	           {
                   		  printf("map failed~!");
			        return FAILURE;
                   }


				 
                //map the windown 2  chip register
              testint[0] = 0x12;
		if (ioctl(gFbfd, FPGA_MAP_DRIVER_SELE,testint) == -1) 
              if (errno == EINVAL) {
                  ERR("\n dsp boot get memory map error");

			//printf("the value is %x",testint[2]);
		}
			  
	       dsp_windown2 = (char *)mmap(0,0x800000,PROT_READ | PROT_WRITE, MAP_SHARED, gFbfd,0x0);

                 if(dsp_windown2 ==NULL)
	           {
                   		  printf("map failed~!");
			        return FAILURE;
                   }

				 
                //map the windown 5  ddr space
              testint[0] = 0x15;
		if (ioctl(gFbfd, FPGA_MAP_DRIVER_SELE,testint) == -1) 
              if (errno == EINVAL) {
                  ERR("\n dsp boot get memory map error");

			//printf("the value is %x",testint[2]);
		}
			  
	       dsp_windown5 = (char *)mmap(0,0x800000,PROT_READ | PROT_WRITE, MAP_SHARED, gFbfd,0x0);

                 if(dsp_windown5 ==NULL)
	           {
                   		  printf("map failed~!");
			        return FAILURE;
                   }


     //读取 标识位


     printf("the device id 2 is %x",*(int*)(((int)(dsp_windown2)+0x1a100)) );


printf("\n dsp_windown 0 is %x",dsp_windown0);
printf("\n dsp_windown 2 is %x",dsp_windown2);

  //初始化dsp
OnTargetConnect();



  printf("\n begin to init the dsp gel file2 ");


  

	     //分析文件

		 retvalue = fread(&entrypoint,4,1,myfd);
		 
               retvalue = fread(&uisectionsize,4,1,myfd);
	        uisectionsize = dsp_swap_32(uisectionsize);
			






 while(uisectionsize)
	    	{


			printf("\n the first section is %x ",uisectionsize);
			
	    	      //得到目的加载地址
            	    	retvalue=  fread(&uiloadaddr,4,1,myfd);	
				  
				  
			//进行地址转换  
			uiloadaddr = dsp_swap_32(uiloadaddr);
		   	 uialteraaddr = pci_boot_addr_altera(uiloadaddr);

			
				  changeddrpage(uiloadaddr);
				  
	              //得到目的运行 地址
            	    	retvalue =  fread(&runaddr,4,1,myfd);


				 	
			for(i=0;i<uisectionsize; i+=4)
				{

				retvalue = fread(&rawdata,4,1,myfd);
				
				*((int*)uialteraaddr) = rawdata;
				
								
				 uialteraaddr +=4;
				 
				}
			
		
			
                      uisectionsize = 0x0;
					  
			
	    	       retvalue = fread(&uisectionsize,4,1,myfd);

				   if(retvalue <=0)
				   	{
				   		
				   	 break;
				   	}
			uisectionsize = dsp_swap_32(uisectionsize);	 
			
	    	}
	    	



                       *((int*)uialteraaddr) = 0; 

					   
		*((int*)((int)dsp_windown2 +0x40008) ) = entrypoint;



		
       *((int*)((int)dsp_windown2 +0x4000c) ) =dsp_swap_32(1);



// 最后要把dsp的ddr映射改成最开始的，不然会问题

 changeddrpage(0x80000000);


}