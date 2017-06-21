
#ifndef __TL_R9624_H_
#define __TL_R9624_H_

#define BASE_TL_PRIVATE			192		/* 192-255 are private */
#define HARDWARE_TYPE_1			101		/* 9624T */
#define HARDWARE_TYPE_3			103
#define HARDWARE_TYPE_2			102

#define TL_DEV_REG_RW 			_IOWR('V', BASE_TL_PRIVATE + 9, unsigned int)
#define TL_PW_VALID				_IOW('V', BASE_TL_PRIVATE + 10, unsigned char)
#define TL_AUDIO_SWITCH		_IOW('V', BASE_TL_PRIVATE + 12, unsigned int)
#define TL_RS485_CTL			       _IOW('V', BASE_TL_PRIVATE + 13, unsigned int)
#define TL_GET_FORMAT			_IOWR('V', BASE_TL_PRIVATE + 14, unsigned int)
#define TL_KEYPAD_CTL			_IOW('V', BASE_TL_PRIVATE + 15, unsigned int)
#define TL_ALARM_OUT			_IOW('V', BASE_TL_PRIVATE + 16, unsigned int)
#define TL_BUZZER_CTL			_IOW('V', BASE_TL_PRIVATE + 17, unsigned int)
#define TL_ALARM_IN				_IOWR('V', BASE_TL_PRIVATE + 18, unsigned int)
#define TL_SET_HARDWARE_TYPE	_IOWR('V', BASE_TL_PRIVATE + 19, int)
#define TL_SATA_STATUS		       _IOWR('V', BASE_TL_PRIVATE + 20, int)
#define TL_SCREEN_CTL			_IOW('V', BASE_TL_PRIVATE + 21, unsigned int)
#define TL_POWER_CTL			_IOW('V', BASE_TL_PRIVATE + 22, unsigned int)
#define TL_DVR_RST			       _IOWR('V', BASE_TL_PRIVATE + 23, int)
#define TL_TWRST_CTL			_IOWR('V', BASE_TL_PRIVATE + 24, int)

#endif
