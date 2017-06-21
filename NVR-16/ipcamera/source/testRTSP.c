#include "openRTSP.h"

#include <stdio.h>
#include <unistd.h>

static FILE *fp[2] = {NULL, NULL};

int DealRtspStreamData(real_stream_s *stream, unsigned int dwContext)
{
	if(stream == NULL)
	{
		return -1;
	}
	
	printf("[app] rtsp frame chn:%d context:%d size:%d\n",stream->chn,dwContext,stream->len);
	
	#if 0
	printf("Hello RTSP...\n");
	
	if(stream->len < 16)
	{
		printf("H264 Data : ( ");
		
		unsigned int i;
		for(i = 0; i < stream->len; i++)
		{
			printf("%02x ", stream->data[i]);
		}
		
		printf(")\n");
	}
	
	FILE *fp = (FILE *)dwContext;
	
	//unsigned char framehead[4] = {0x00, 0x00, 0x00, 0x01};
	//fwrite(framehead, sizeof(framehead), 1, fp);
	
	fwrite(stream->data, stream->len, 1, fp);
	#endif
	
	return 0;
}

int main(int argc, char **argv)
{
	//fp[0] = fopen("stream1.264", "wb");
	//fp[1] = fopen("stream2.264", "wb");
	
	RTSPC_Init(32);
	
	RTSPC_Startbyurl(0, DealRtspStreamData, (unsigned int)fp[0], (char *)"rtsp://192.168.1.189:554/11", (char *)"admin", (char *)"admin", 1);
	RTSPC_Startbyurl(1, DealRtspStreamData, (unsigned int)fp[1], (char *)"rtsp://192.168.1.189:554/12", (char *)"admin", (char *)"admin", 1);
	
	//RTSPC_Startbyurl(0, DealRtspStreamData, (unsigned int)fp[0], (char *)"rtsp://192.168.1.154/user=admin_password=tlJwpbo6_channel=1_stream=0.sdp?real_stream", (char *)"admin", (char *)"admin", 1);
	
	int count = 0;
	while(1)
	{
		sleep(1);
		
		//if(++count > 15)
		//{
		//	break;
		//}
	}
	
	printf("test over.\n");
	
	RTSPC_Stop(0);
	//RTSPC_Stop(1);
	
	RTSPC_DeInit();
	
	//fclose(fp[0]);
	//fclose(fp[1]);
	
	count = 0;
	while(1)
	{
		sleep(1);
		
		if(++count > 3)
		{
			break;
		}
	}
	
	printf("proc quit!\n");
	
	return 0;
}

