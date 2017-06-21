#ifndef __TL_TW286x_H_
#define __TL_TW286x_H_

#define    BASE_TL_PRIVATE_286x	        200	/* 192-255 are private */

#define    	TL_2865_RD 					_IOWR('H', BASE_TL_PRIVATE_286x + 7, unsigned int)
#define    	TL_2865_WR 					_IOWR('H', BASE_TL_PRIVATE_286x + 8, unsigned int)
#define    	TL_PT8528_WR 				_IOWR('H', BASE_TL_PRIVATE_286x + 9, unsigned int)
#define    	TL_PT8528_RD 				_IOWR('H', BASE_TL_PRIVATE_286x + 10, unsigned int)
#define    	TL_INIT_PAL_NTSC 			_IOWR('H', BASE_TL_PRIVATE_286x + 11, unsigned int)
#define    	TL_VGA_RESOLUTION			_IOW('H', BASE_TL_PRIVATE_286x + 12, int)
#define    	TL_PCA9555_WR 				_IOWR('H', BASE_TL_PRIVATE_286x + 13, unsigned int)
#define    	TL_PCA9555_RD 				_IOWR('H', BASE_TL_PRIVATE_286x + 14, unsigned int)
#define  	TL_AUTO_PAL_NTSC			_IOWR('H', BASE_TL_PRIVATE_286x + 15, unsigned int)
#define		TL_AUTO_CHIP_TYPE			_IOWR('H', BASE_TL_PRIVATE_286x + 16, unsigned int)
#define 	TL_PRINTK_DEBUG				_IOWR('H', BASE_TL_PRIVATE_286x + 17, unsigned int)
#define    	TL_INIT_HDMI				_IOW('H', BASE_TL_PRIVATE_286x + 18, int)
#define    	TL_HDMI_HOT_PLUG			_IOWR('H', BASE_TL_PRIVATE_286x + 19, int)

typedef enum
{
    CHIP_AUTO,
    CHIP_TW2866_1,		
    CHIP_TW2868_1,		
    CHIP_LG1702_1,	
    CHIP_RN6266_1,	
    CHIP_MP2807_1,		
    CHIP_MIK2455_1,		
    CHIP_TW2866_2,		
    CHIP_TW2868_2,		
    CHIP_LG1702_2,		
    CHIP_RN6266_2,		
    CHIP_MP2807_2,	
    CHIP_MIK2455_2,	
    CHIP_NVP1108_1,
}tl_decode_chip_type_e;

#endif

