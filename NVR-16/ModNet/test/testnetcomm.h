#ifndef __TESTNETCOMM_H_
#define __TESTNETCOMM_H_


//图像编码类型
#define  MEDIA_TYPE_H264	 (u8)98//H.264//可能是109?
#define  MEDIA_TYPE_MP4	     (u8)97//MPEG-4
#define  MEDIA_TYPE_H261	 (u8)31//H.261
#define  MEDIA_TYPE_H263	 (u8)34//H.263
#define  MEDIA_TYPE_MJPEG	 (u8)26//Motion JPEG
#define  MEDIA_TYPE_MP2		 (u8)33//MPEG2 video


//wrchen 091130+
#define VIDEO_RESOLUTION_D1 0
#define VIDEO_RESOLUTION_CIF 1
#define VIDEO_RESOLUTION_QCIF 2
#define VIDEO_RESOLUTION_HALF_D1 3
//wrchen 091130-


//语音编码类型
#define	 MEDIA_TYPE_MP3	     (u8)96//mp3
#define  MEDIA_TYPE_PCMU	 (u8)0//G.711 ulaw
#define  MEDIA_TYPE_PCMA	 (u8)8//G.711 Alaw
#define	 MEDIA_TYPE_G7231	 (u8)4//G.7231
#define	 MEDIA_TYPE_G722	 (u8)9//G.722
#define	 MEDIA_TYPE_G728	 (u8)15//G.728
#define	 MEDIA_TYPE_G729	 (u8)18//G.729
#define	 MEDIA_TYPE_RAWAUDIO (u8)19//raw audio
//#define  MEDIA_TYPE_ADPCM	 (u8)20//adpcm
#define  MEDIA_TYPE_ADPCM	 (u8)21//adpcm//wrchen 091117

#endif // __TESTNETCOMM_H_
