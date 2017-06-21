#include <pthread.h>

#include "BasicUsageEnvironment.hh"
#include "liveMedia.hh"

#include "openRTSP.h"

static unsigned char g_rtspc_loop = 0;

class ourRTSPClient;

#define FLAG_RUN				0		//正在运行
#define FLAG_DESTORY			1		//正在销毁
#define FLAG_FREE				2		//已经销毁，可以复用

#define RTSP_FLAG_DISCONNECT	1
#define RTSP_FLAG_THREADOVER	2

typedef struct
{
	unsigned int flagState;
	unsigned int dwRTSPFlag;
	ourRTSPClient *rtspClient;
	TaskScheduler* scheduler;
	UsageEnvironment* env;
	pthread_mutex_t lock;
}rtsp_client_info;

#define MAX_RTSP_CLIENT_NUM	72

static rtsp_client_info g_rtspc_info[MAX_RTSP_CLIENT_NUM];

static unsigned int g_rtsp_client_count = MAX_RTSP_CLIENT_NUM;

extern "C" int ff_h264_decode_seq_parameter_set(unsigned char*in_buf,unsigned int buffer_size,int *h,int *w,int *num_ref_frames);

//buf:数据缓存
//start_bit:起始bit位置，从0
//返回后面的位置
unsigned int exp_golomb_decode(unsigned char *buf,unsigned int start_bit,unsigned int *value)
{
	unsigned char zero_bit=0;
	unsigned char add_bit=0;
	unsigned int char_num=0;
	//unsigned char bit_num=0;
	unsigned int sum_data=0;
	char_num = start_bit;
	//统计0比特
	while((buf[char_num/8] & ((1<<(7-char_num%8)))) == 0)
	{
		zero_bit++;
		char_num++;
		//printf("0");
	}
	zero_bit++;
	while(zero_bit)
	{
		sum_data <<= 1;
		if(buf[char_num/8] & ((1<<(7-char_num%8))))
		{
			sum_data += 1;
			add_bit = 0;
			//printf("1");
		}
		else
		{
			//printf("0");
		}
		char_num++;
		zero_bit--;
	}
	//printf("=%d\n",sum_data);
	*value = sum_data-1;
	return char_num;
} 

//in_buf:输入串行比特流，0x000001后面的数据。
//hegiht width num_ref_frames:传出参数，图像大小计算 height X width*num_ref_frames 
//返回 0：调用失败  1：成功
unsigned int get_h264_resolution(unsigned char*in_buf,int *height,int *width,int *num_ref_frames)
{
	unsigned char profile_ide;
	unsigned int chroma_format_idc,seq_parameter_set_id,bit_depth_luma_minus8,bit_depth_chroma_minus8;
	unsigned int log2_max_frame_num_minus4,pic_order_cnt_type,log2_max_pic_order_cnt_lsb_minus4;
	unsigned int offset_for_non_ref_pic,offset_for_top_to_bottom_field,num_ref_frames_in_pic_order_cnt_cycle,offset_for_ref_frame;
	unsigned int bit_site,i,j;
	
	//if(in_buf[0] != 0x67)
	if((in_buf[0] & 0x1f) != 0x7)
	{
		return 0;//nal header必须是0X67表示SEQ_PARAMETER_SET类型
	}
	
	profile_ide = in_buf[1];
	printf("profile_ide = %d\n",profile_ide);
	
	//忽略 constraint_set0_flag ， constraint_set1_flag， constraint_set2_flag，reserved_zero_5bits
	//忽略 level_idc
	printf("set=%d,level=%d\n",in_buf[2],in_buf[3]);
	bit_site = 32;//至此已经历的BIT数
	bit_site = exp_golomb_decode(in_buf,bit_site,&seq_parameter_set_id);//get seq_parameter_set_id
	printf("seq_parameter_set_id = %d\n",seq_parameter_set_id);
	
	//high profile//参考ffmpeg
	//if((profile_ide == 100) || (profile_ide == 110) || (profile_ide == 122) || (profile_ide == 144) || (profile_ide == 83))
	if(profile_ide >= 100)
	{
		//printf("bit_site=%d\n",bit_site);
		bit_site = exp_golomb_decode(in_buf,bit_site,&chroma_format_idc);//get chroma_format_idc
		//printf("chroma_format_idc = %d\n",chroma_format_idc);
		if(chroma_format_idc == 3)
		{
			bit_site++;//忽略residual_colour_transform_flag 1bit
		}
		bit_site = exp_golomb_decode(in_buf,bit_site,&bit_depth_luma_minus8);//get bit_depth_luma_minus8
		//printf("bit_depth_luma_minus8 = %d\n",bit_depth_luma_minus8);
		bit_site = exp_golomb_decode(in_buf,bit_site,&bit_depth_chroma_minus8);//get bit_depth_chroma_minus8
		//printf("bit_depth_chroma_minus8 = %d\n",bit_depth_chroma_minus8);
		bit_site++;//忽略qpprime_y_zero_transform_bypass_flag
		if(in_buf[bit_site/8] & (1 << (7-bit_site%8)) & 0xff)//if(seq_scaling_matrix_present_flag == 1)
		{
			bit_site++;
			//printf("seq_scaling_matrix_present_flag == 1\n");
			for(i = 0;i<8;i++)
			{
				if(in_buf[bit_site/8] & (1 << (7-bit_site%8)) & 0xff)
				{
					bit_site++;
					if(i<6)
					{
						for(j=0;j<16;j++)
						{
							bit_site = exp_golomb_decode(in_buf,bit_site,&bit_depth_chroma_minus8);//get bit_depth_chroma_minus8
							//printf("%d ",bit_depth_chroma_minus8);
						}
					}
					else
					{
						for(j=0;j<64;j++)
						{
							bit_site = exp_golomb_decode(in_buf,bit_site,&bit_depth_chroma_minus8);//get bit_depth_chroma_minus8
							//printf("%d ",bit_depth_chroma_minus8);
						}
					}
				}
				else bit_site++;
					//printf("\n");
			}
			//bit_site += 8;//忽略seq_scaling_list_present_flag[ 8 ]
		}
	}
	
	printf("index=%d\n",bit_site);
	
	bit_site = exp_golomb_decode(in_buf,bit_site,&log2_max_frame_num_minus4);//get log2_max_frame_num_minus4
	printf("log2_max_frame_num_minus4 = %d\n",log2_max_frame_num_minus4);
	bit_site = exp_golomb_decode(in_buf,bit_site,&pic_order_cnt_type);//get pic_order_cnt_type
	printf("pic_order_cnt_type = %d\n",pic_order_cnt_type);
	if(pic_order_cnt_type == 0)
	{
		bit_site = exp_golomb_decode(in_buf,bit_site,&log2_max_pic_order_cnt_lsb_minus4);//get log2_max_pic_order_cnt_lsb_minus4
	}
	else if(pic_order_cnt_type == 1)
	{
		bit_site++;//空位
		bit_site = exp_golomb_decode(in_buf,bit_site,&offset_for_non_ref_pic);//get offset_for_non_ref_pic
		bit_site = exp_golomb_decode(in_buf,bit_site,&offset_for_top_to_bottom_field);//get offset_for_top_to_bottom_field
		bit_site = exp_golomb_decode(in_buf,bit_site,&num_ref_frames_in_pic_order_cnt_cycle);//get num_ref_frames_in_pic_order_cnt_cycle
		for(i=0; i<num_ref_frames_in_pic_order_cnt_cycle; i++)
		{	
			bit_site = exp_golomb_decode(in_buf,bit_site,&offset_for_ref_frame);//get offset_for_ref_frame
		}
	}
	bit_site = exp_golomb_decode(in_buf,bit_site,&num_ref_frames_in_pic_order_cnt_cycle);//get num_ref_frames
	*num_ref_frames = num_ref_frames_in_pic_order_cnt_cycle;
	
	bit_site++;//忽略gaps_in_frame_num_value_allowed_flag
	
	bit_site = exp_golomb_decode(in_buf,bit_site,&num_ref_frames_in_pic_order_cnt_cycle);//get num_ref_frames
	//*height = num_ref_frames_in_pic_order_cnt_cycle;
	*width = (num_ref_frames_in_pic_order_cnt_cycle+1)*16;
	
	bit_site = exp_golomb_decode(in_buf,bit_site,&num_ref_frames_in_pic_order_cnt_cycle);//get num_ref_frames
	//*width = num_ref_frames_in_pic_order_cnt_cycle;
	*height = (num_ref_frames_in_pic_order_cnt_cycle+1)*16;
	
	//图像高度可能不对，后面需要处理完善
	//参考<<从H264的SPS中获取图像长宽_百度文库.mht>>
	//dec->pic_height =( 2 - dec->frame_mbs_only_flag ) * PicHeightInMapUnits * 16;
	
	return 1;
}

unsigned int get_stream_info(unsigned char *frame_data,unsigned int frame_len,int *height,int *width,int *num_ref_frames)
{
	unsigned int i;
	//printf("%x %x %x %x %x %x\n",frame_data[0],frame_data[1],frame_data[2],frame_data[3],frame_data[4],frame_data[5]);
	for(i=2;i<200;i++)
	{
		if(i >= frame_len)
		{
			return 0;
		}
		
		if((frame_data[i-2] == 0) && (frame_data[i-1] == 0) && (frame_data[i-0] == 1))
		{
			break;		
		}
	}
	if(i >= 200)
	{
		return 0;
	}
	i++;
	//printf("aaaaaaaaa\n");
	return get_h264_resolution(&frame_data[i],height,width,num_ref_frames);
}

void continueAfterOPTIONS(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);

// Used to iterate through each stream's 'subsessions', setting up each one:
void setupNextSubsession(RTSPClient* rtspClient);

// Used to shut down and close a stream (including its "RTSPClient" object):
void shutdownStream(RTSPClient* rtspClient, int exitCode = 1);

// A function that outputs a string that identifies each stream (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const RTSPClient& rtspClient)
{
	return env << "[URL:\"" << rtspClient.url() << "\"]: ";
}

// A function that outputs a string that identifies each subsession (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const MediaSubsession& subsession)
{
	return env << subsession.mediumName() << "/" << subsession.codecName();
}

// Define a class to hold per-stream state that we maintain throughout each stream's lifetime:

class StreamClientState
{
public:
	StreamClientState();
	virtual ~StreamClientState();
	
public:
	MediaSubsessionIterator* iter;
	MediaSession* session;
	MediaSubsession* subsession;
	TaskToken streamTimerTask;
	double duration;
};

// Implementation of "StreamClientState":

StreamClientState::StreamClientState()
	: iter(NULL), session(NULL), subsession(NULL), streamTimerTask(NULL), duration(0.0)
{
	
}

StreamClientState::~StreamClientState()
{
	if(iter != NULL)
	{
		delete iter;
		iter = NULL;
		
		//printf("StreamClientState::~StreamClientState-delete iter\n");
		//fflush(stdout);
	}
	
	if(session != NULL)
	{
		if(streamTimerTask != NULL)
		{
			// We also need to delete "session", and unschedule "streamTimerTask" (if set)
			UsageEnvironment& env = session->envir(); // alias
			
			env.taskScheduler().unscheduleDelayedTask(streamTimerTask);
		}
		
		Medium::close(session);
		session = NULL;
		
		//printf("StreamClientState::~StreamClientState-close session\n");
		//fflush(stdout);
	}
	
	printf("StreamClientState::~StreamClientState-over\n");
	fflush(stdout);
}

typedef int (*PYZCALLBACK)(void* pContext, uint32_t dwMsg, uint32_t dwParam1, uint32_t dwParam2, uint32_t dwParam3);

// If you're streaming just a single stream (i.e., just from a single URL, once), then you can define and use just a single
// "StreamClientState" structure, as a global variable in your application.  However, because - in this demo application - we're
// showing how to play multiple streams, concurrently, we can't do that.  Instead, we have to have a separate "StreamClientState"
// structure for each "RTSPClient".  To do this, we subclass "RTSPClient", and add a "StreamClientState" field to the subclass:

class ourRTSPClient: public RTSPClient
{
public:
	static ourRTSPClient* createNew(UsageEnvironment& env, char const* rtspURL,
					int verbosityLevel = 0,
					char const* applicationName = NULL,
					portNumBits tunnelOverHTTPPortNum = 0,
					char *user = NULL,
					char *pwd = NULL,
					unsigned char rtsp_over_tcp = 0,
					int chn = 0,
					RealStreamCB pCB = NULL,
					unsigned int dwContext = 0);
	
protected:
	// called only by createNew();
	ourRTSPClient(UsageEnvironment& env, char const* rtspURL,
					int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum,
					char *user, char *pwd, unsigned char rtsp_over_tcp, int chn, RealStreamCB pCB, unsigned int dwContext);
	
	virtual ~ourRTSPClient();
	
public:
	StreamClientState scs;
	
	Authenticator* ourAuthenticator;
	
	static unsigned int stream_count;//声明静态数据成员
	
	int channel;
	
	char rtsp_url[64];
	
	char username[32];
	char password[32];
	
	unsigned char stream_over_tcp;
	
	RealStreamCB pStreamCB;
	unsigned int dwStreamContext;
	
	int start;
	
	pthread_t m_hThread;
	//pthread_mutex_t m_hMutex;
	
	char m_bExited;
	
	//如果退出置为非0
	char eventLoopWatchVariable;
	
	PYZCALLBACK pMsgCB;
	void* pMsgContext;
	
	int Start();
	int Stop(int ExitCode = 1);
	
	void OnThread();
	
	void Destory();
	
	int RegisterCallback(PYZCALLBACK pCallback, void* pContext);
	int DoCallback(unsigned int dwMsg, uint32_t dwParam1, uint32_t deParam2, uint32_t dwParam3);
	
	//int LockThread(){return pthread_mutex_lock(&m_hMutex);}
	//int UnLockThread(){return pthread_mutex_unlock(&m_hMutex);}
};

// Implementation of "ourRTSPClient":

unsigned int ourRTSPClient::stream_count = 0;//定义并初始化静态数据成员

ourRTSPClient* ourRTSPClient::createNew(UsageEnvironment& env, char const* rtspURL,
					int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum,
					char *user, char *pwd, unsigned char rtsp_over_tcp, int chn, RealStreamCB pCB, unsigned int dwContext)
{
	return new ourRTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, user, pwd, rtsp_over_tcp, chn, pCB, dwContext);
}

ourRTSPClient::ourRTSPClient(UsageEnvironment& env, char const* rtspURL, 
				int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum,
				char *user, char *pwd, unsigned char rtsp_over_tcp, int chn, RealStreamCB pCB, unsigned int dwContext)
				: RTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1)
{
	//printf("ourRTSPClient::ourRTSPClient-1\n");
	
	ourAuthenticator = NULL;
	
	channel = chn;
	
	memset(rtsp_url, 0, sizeof(rtsp_url));
	memset(username, 0, sizeof(username));
	memset(password, 0, sizeof(password));
	
	if(rtspURL)
	{
		strcpy(rtsp_url, rtspURL);
	}
	
	if(user)
	{
		strcpy(username, user);
	}
	
	if(pwd)
	{
		strcpy(password, pwd);
	}
	
	stream_over_tcp = rtsp_over_tcp;
	
	if(user)
	{
		ourAuthenticator = new Authenticator(username, password);
		//printf("ourRTSPClient::ourRTSPClient-2,username=%s,password=%s,ourAuthenticator=0x%08x\n",username,password,(unsigned int)ourAuthenticator);
	}
	
	pStreamCB = pCB;
	dwStreamContext = dwContext;
	
	start = 0;
	
	m_hThread = 0;
	//pthread_mutexattr_t attr;
	//pthread_mutexattr_init(&attr);
	//pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);
	//pthread_mutex_init(&m_hMutex, &attr);
	
	m_bExited = False;
	
	eventLoopWatchVariable = 0;
	
	pMsgCB = NULL;
	pMsgContext = NULL;
	
	printf("ourRTSPClient::ourRTSPClient-over,stream_over_tcp=%d\n",stream_over_tcp);
}

ourRTSPClient::~ourRTSPClient()
{
	//printf("######ourRTSPClient::~ourRTSPClient-1######\n");
	
	if(ourAuthenticator != NULL)
	{
		delete ourAuthenticator;
		ourAuthenticator = NULL;
		
		//printf("######ourRTSPClient::~ourRTSPClient-2######\n");
	}
	
	pStreamCB = NULL;
	dwStreamContext = 0;
	
	if(start)
	{
		start = 0;
		
		ourRTSPClient::stream_count--;
		
		//printf("######ourRTSPClient::~ourRTSPClient-3######\n");
	}
	
	//pthread_mutex_destroy(&m_hMutex);
	
	printf("######ourRTSPClient::~ourRTSPClient-chn%d over######\n", channel);
	fflush(stdout);
}

void* ThreadPROCRTSP(void* pParam)
{
	ourRTSPClient* pClient = (ourRTSPClient*)pParam;
	//printf("Thread Callback Context = 0x%x\n", (unsigned int)pClient);
	pClient->OnThread();
	pthread_detach(pthread_self());
	return 0;
}

int ourRTSPClient::Start()
{
	if(m_hThread != 0)
	{
		Stop(0);
	}
	
	eventLoopWatchVariable = 0;
	int ret = pthread_create(&m_hThread, NULL, ThreadPROCRTSP, this);
	if(ret != 0)
	{
		m_hThread = 0;
		return -1;
	}
	
	return 0;
}

int ourRTSPClient::Stop(int ExitCode)
{
	if(m_hThread != 0)
	{
		//printf("ourRTSPClient::Stop-1 chn%d\n", channel);
		
		//LockThread();
		eventLoopWatchVariable = 1;
		//UnLockThread();
		
		//printf("ourRTSPClient::Stop-2 chn%d\n", channel);
		
		pthread_join(m_hThread, NULL);
		
		//printf("ourRTSPClient::Stop-3 chn%d\n", channel);
	}
	
	return 0;
}

void ourRTSPClient::OnThread()
{
	// Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
	// Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
	// Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
	ourRTSPClient *p = (ourRTSPClient *)this;
	//解决接入雄迈1080P摄像机的bug
	//p->sendOptionsCommand(continueAfterOPTIONS, p->ourAuthenticator);
	p->sendDescribeCommand(continueAfterDESCRIBE, p->ourAuthenticator);
	
	//printf("ourRTSPClient::OnThread-1 chn%d\n", channel);
	
	envir().taskScheduler().doEventLoop(&eventLoopWatchVariable);
	
	//printf("ourRTSPClient::OnThread-2 chn%d\n", channel);
	
	if(!m_bExited)
	{
		//printf("ourRTSPClient::OnThread-3 chn%d\n", channel);
		
		shutdownStream(this, 0);
		
		//printf("ourRTSPClient::OnThread-4 chn%d\n", channel);
	}
	
	//printf("ourRTSPClient::OnThread-5 chn%d\n", channel);
	
	m_hThread = 0;
	
	DoCallback(RTSP_FLAG_THREADOVER, channel, 0, 0);
	
	//printf("ourRTSPClient::OnThread-6 chn%d\n", channel);
}

void ourRTSPClient::Destory()
{
	Medium::close(this);
}

int ourRTSPClient::RegisterCallback(PYZCALLBACK pCallback, void* pContext)
{
	pMsgCB = pCallback;
	pMsgContext = pContext;
	return 0;
}

int ourRTSPClient::DoCallback(uint32_t dwMsg, uint32_t dwParam1, uint32_t dwParam2, uint32_t dwParam3)
{
	if(pMsgCB == NULL)
	{
		return -1;
	}
	return (*pMsgCB)(pMsgContext, dwMsg, dwParam1, dwParam2, dwParam3);
}

extern int DoStreamStateCallBack(int chn, real_stream_state_e msg);

int Callback(void* pContext, uint32_t dwMsg, uint32_t dwParam1, uint32_t dwParam2, uint32_t dwParam3)
{
	int chn = (int)dwParam1;
	if(chn >= (int)g_rtsp_client_count)
	{
		return -1;
	}
	
	//printf("Callback-1\n");
	
	pthread_mutex_lock(&g_rtspc_info[chn].lock);
	
	//printf("Callback-2\n");
	
	if(dwMsg == RTSP_FLAG_DISCONNECT)
	{
		g_rtspc_info[chn].dwRTSPFlag |= RTSP_FLAG_DISCONNECT;
	}
	else if(dwMsg == RTSP_FLAG_THREADOVER)
	{
		g_rtspc_info[chn].dwRTSPFlag |= RTSP_FLAG_THREADOVER;
	}
	
	if(((g_rtspc_info[chn].dwRTSPFlag & RTSP_FLAG_THREADOVER) == RTSP_FLAG_THREADOVER)
		&& ((g_rtspc_info[chn].dwRTSPFlag & RTSP_FLAG_DISCONNECT) == RTSP_FLAG_DISCONNECT))
	{
		g_rtspc_info[chn].rtspClient->Destory();
		g_rtspc_info[chn].rtspClient = NULL;
		g_rtspc_info[chn].flagState = FLAG_FREE;
		g_rtspc_info[chn].dwRTSPFlag = 0;
	}
	
	//printf("Callback-3\n");
	
	pthread_mutex_unlock(&g_rtspc_info[chn].lock);
	
	//printf("Callback-4\n");
	
	if(dwMsg == RTSP_FLAG_DISCONNECT || dwMsg == RTSP_FLAG_THREADOVER)
	{
		DoStreamStateCallBack(chn, REAL_STREAM_STATE_LOST);
	}
	
	//printf("Callback-5\n");
	
	return 0;
}

// Define a data sink (a subclass of "MediaSink") to receive the data for each subsession (i.e., each audio or video 'substream').
// In practice, this might be a class (or a chain of classes) that decodes and then renders the incoming audio or video.
// Or it might be a "FileSink", for outputting the received data into a file (as is done by the "openRTSP" application).
// In this example code, however, we define a simple 'dummy' sink that receives incoming data, but does nothing with it.

class DummySink: public MediaSink
{
public:
	static DummySink* createNew(UsageEnvironment& env,
						MediaSubsession& subsession,	// identifies the kind of data that's being received
						char const* streamId = NULL,	// identifies the stream itself (optional)
						int size = 0);					// identifies the stream buffer size
	
private:
	// called only by "createNew()"
	DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId, int size);
	
	virtual ~DummySink();
	
	static void afterGettingFrame(void* clientData, unsigned frameSize,
						unsigned numTruncatedBytes,
						struct timeval presentationTime,
						unsigned durationInMicroseconds);
	void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
						struct timeval presentationTime, unsigned durationInMicroseconds);
	
private:
	// redefined virtual functions:
	virtual Boolean continuePlaying();
	
private:
	MediaSubsession& fSubsession;
	char* fStreamId;
	
public:
	u_int8_t* fReceiveBuffer;
	unsigned int fRecvBufferSize;
	
	unsigned int m_width;
	unsigned int m_height;
	unsigned int m_num_ref_frames;
};

// Implementation of "DummySink":

// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:
//#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 100000
//#define DUMMY_SINK_RECEIVE_BUFFER_SIZE (256*1024-4)

DummySink* DummySink::createNew(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId, int size)
{
	return new DummySink(env, subsession, streamId, size);
}

DummySink::DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId, int size)
	: MediaSink(env), fSubsession(subsession)
{
	m_width = 0;
	m_height = 0;
	m_num_ref_frames = 0;
	
	fReceiveBuffer = NULL;
	fStreamId = NULL;
	
	if(size == 0)
	{
		fRecvBufferSize = (256*1024-4);
	}
	else
	{
		fRecvBufferSize = size;
	}
	
	if(strcmp(fSubsession.mediumName(), "video") == 0)
	{
		//printf("video subsession\n");
	}
	else if(strcmp(fSubsession.mediumName(), "audio") == 0)
	{
		//printf("audio subsession\n");
		fRecvBufferSize /= 20;
		fRecvBufferSize = (fRecvBufferSize+3)/4*4;
	}
	
	//printf("DummySink::DummySink mediumName=%s codecName=%s w=%d h=%d fRecvBufferSize=%u\n",fSubsession.mediumName(),fSubsession.codecName(),fSubsession.videoWidth(),fSubsession.videoHeight(),fRecvBufferSize);
	printf("DummySink::DummySink mediumName=%s codecName=%s fRecvBufferSize=%u\n",fSubsession.mediumName(),fSubsession.codecName(),fRecvBufferSize);
	
	fReceiveBuffer = new u_int8_t[4 + fRecvBufferSize];
	if(fReceiveBuffer == NULL)
	{
		printf("DummySink::DummySink error - no space\n");
		return;
	}
	
	fStreamId = strDup(streamId);
	
	#if 1
	//if(strcmp(fSubsession.mediumName(), "video") == 0 && strcmp(fSubsession.codecName(), "H264") == 0)
	if(strcasecmp(fSubsession.mediumName(), "video") == 0 && strcasecmp(fSubsession.codecName(), "H264") == 0)
	{
		unsigned int num = 0;
		SPropRecord* pSPS = parseSPropParameterSets(subsession.fmtp_spropparametersets(), num);
		if(pSPS != NULL && num > 0)
		{
			unsigned char *pbuf = (unsigned char *)fReceiveBuffer;
			unsigned int *p = (unsigned int *)pbuf;
			*p = 0x01000000;
			memcpy(pbuf+4, pSPS[0].sPropBytes, pSPS[0].sPropLength);
			p = (unsigned int *)(pbuf+4+pSPS[0].sPropLength);
			*p = 0x01000000;
			if(num > 1)
			{
				memcpy(pbuf + pSPS[0].sPropLength + 8, pSPS[1].sPropBytes, pSPS[1].sPropLength);
			}
			//printf("parseSPropParameterSets num=%d\n",num);
			
			#if 0
			printf("sps:( ");
			for(unsigned int i=0;i<4+pSPS[0].sPropLength;i++)
			{
				printf("%02x ",pbuf[i]);
			}
			printf(")\n");
			#endif
			
			int w = 0;
			int h = 0;
			int num_ref_frames = 0;
			
			//get_h264_resolution(pbuf+4, &h, &w, &num_ref_frames);
			
			//if(get_h264_resolution(pbuf+4, &h, &w, &num_ref_frames))
			if(ff_h264_decode_seq_parameter_set(pbuf+5, pSPS[0].sPropLength-1, &h, &w, &num_ref_frames) == 0)
			{
				if(h == 1088)
				{
					h = 1080;
				}
				
				m_width = w;
				m_height = h;
				m_num_ref_frames = num_ref_frames;
				
				printf("DummySink:ff_h264_decode_seq_parameter_set w=%d h=%d num_ref_frames=%d\n",w,h,num_ref_frames);
			}
			else
			{
				printf("DummySink:ff_h264_decode_seq_parameter_set failed\n");
				
				printf("sps data[%d bytes]:( ", pSPS[0].sPropLength);
				unsigned char *p = (unsigned char *)pSPS[0].sPropBytes;
				for(unsigned int k=0;k<pSPS[0].sPropLength;k++)
				{
					printf("%02x ",p[k]);
				}
				printf(")\n");
				
				#if 0
				if(num > 1)
				{
					printf("sps data[%d bytes]:( ", pSPS[1].sPropLength);
					unsigned char *p = (unsigned char *)pSPS[1].sPropBytes;
					for(unsigned int k=0;k<pSPS[1].sPropLength;k++)
					{
						printf("%02x ",p[k]);
					}
					printf(")\n");
				}
				#endif
				
				m_width = 0;
				m_height = 0;
				m_num_ref_frames = 1;
			}
		}
		else
		{
			printf("<warning>:pSPS=0x%08x,num=%d\n",(unsigned int)pSPS,num);
			
			m_width = 0;
			m_height = 0;
			m_num_ref_frames = 1;
		}
	}
	else
	{
		printf("no video???no H264???\n");
	}
	#endif
	
	fReceiveBuffer[0] = 0x00;
	fReceiveBuffer[1] = 0x00;
	fReceiveBuffer[2] = 0x00;
	fReceiveBuffer[3] = 0x01;
	fReceiveBuffer += 4;
}

DummySink::~DummySink()
{
	if(fReceiveBuffer)
	{
		delete[] (fReceiveBuffer - 4);
		fReceiveBuffer = NULL;
	}
	
	if(fStreamId)
	{
		delete[] fStreamId;
		fStreamId = NULL;
	}
}

void DummySink::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
					struct timeval presentationTime, unsigned durationInMicroseconds)
{
	DummySink* sink = (DummySink*)clientData;
	sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

// If you don't want to see debugging output for each received frame, then comment out the following line:
//#define DEBUG_PRINT_EACH_RECEIVED_FRAME 1
extern "C" unsigned int getTimeStamp();

void DummySink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
					struct timeval presentationTime, unsigned /*durationInMicroseconds*/)
{
	// We've just received a frame of data.  (Optionally) print out information about it:
#ifdef DEBUG_PRINT_EACH_RECEIVED_FRAME
	if(fStreamId != NULL)
	{
		envir() << "Stream \"" << fStreamId << "\"; ";
	}
	
	envir() << fSubsession.mediumName() << "/" << fSubsession.codecName() << ":\tReceived " << frameSize << " bytes";
	
	if(numTruncatedBytes > 0)
	{
		envir() << " (with " << numTruncatedBytes << " bytes truncated)";
	}
	
	char uSecsStr[6+1]; // used to output the 'microseconds' part of the presentation time
	sprintf(uSecsStr, "%06u", (unsigned)presentationTime.tv_usec);
	envir() << ".\tPresentation time: " << (int)presentationTime.tv_sec << "." << uSecsStr;
	
	if(fSubsession.rtpSource() != NULL && !fSubsession.rtpSource()->hasBeenSynchronizedUsingRTCP())
	{
		envir() << "!"; // mark the debugging output to indicate that this presentation time is not RTCP-synchronized
	}
#ifdef DEBUG_PRINT_NPT
	envir() << "\tNPT: " << fSubsession.getNormalPlayTime(presentationTime);
#endif
	envir() << "\n";
#endif
	
	if(numTruncatedBytes)
	{
		printf("openrtsp warning:rtsp frameSize=%d numTruncatedBytes=%d\n",frameSize,numTruncatedBytes);
	}
	
	ourRTSPClient *p = (ourRTSPClient *)(fSubsession.miscPtr);
	if(p != NULL)
	{
		int chn = p->channel;
		
		pthread_mutex_lock(&g_rtspc_info[chn].lock);
		
		RealStreamCB pStreamCB = p->pStreamCB;
		unsigned int dwContext = p->dwStreamContext;
		
		pthread_mutex_unlock(&g_rtspc_info[chn].lock);
		
		//if(pStreamCB != NULL && strcmp(fSubsession.mediumName(), "video") == 0 && strcmp(fSubsession.codecName(), "H264") == 0)
		if(pStreamCB != NULL && strcasecmp(fSubsession.mediumName(), "video") == 0 && strcasecmp(fSubsession.codecName(), "H264") == 0)
		{
			real_stream_s stream;
			//memset(&stream, 0, sizeof(stream));
			stream.chn = chn;
			stream.data = fReceiveBuffer - 4;
			stream.len = frameSize + 4;
			stream.pts = (unsigned long long)1000000*presentationTime.tv_sec + presentationTime.tv_usec;
			stream.media_type = MEDIA_PT_H264;
		#if 0
			
			if (stream.chn == 0 && stream.len > 5)
			{
				unsigned int t = getTimeStamp(); //ms
				printf("\t time: %llu, local pts: %u, length: %d, 0x%x 0x%x 0x%x 0x%x 0x%x\n", stream.pts, t, stream.len,\
					stream.data[0], \
					stream.data[1], \
					stream.data[2], \
					stream.data[3], \
					stream.data[4]);
			}
		#endif
			if((fReceiveBuffer[0] & 0x1f) == 0x07)
			{
				stream.frame_type = REAL_FRAME_TYPE_I;
				//if(chn == 0)
				//{
				//	printf("chn%d frame type:I\n",chn);
				//}
			}
			else
			{
				stream.frame_type = REAL_FRAME_TYPE_P;
				//if(chn == 0)
				//{
				//	printf("chn%d frame type:P\n",chn);
				//}
			}
			stream.rsv = m_num_ref_frames;
			stream.mdevent = 0;
			int w = 0;
			int h = 0;
			if(IPC_GetStreamResolution(chn,&w,&h) < 0)
			{
				w = h = 0;
			}
			if(m_width == 0 || m_height == 0)
			{
				stream.width = w;
				stream.height = h;
				
				m_width = w;
				m_height = h;
				
				//printf("GetStreamResolution w=%d h=%d\n",w,h);
				
			}
			else if(w <= 0 || h <= 0 || (w%8) || (h%8) || w > 1920 || h > 1200)
			{
				stream.width = m_width;
				stream.height = m_height;
				
			}
			else
			{
				stream.width = m_width;//w;
				stream.height = m_height;//h;
				
			}
			//if(chn == 3 || chn == 19)
			//if(chn == 19)
			//{
			//	//printf("chn%d rtsp stream w=%d h=%d\n",chn,stream.width,stream.height);
			//}
			//stream.rsv = 0;
			//stream.mdevent = 0;
			//stream.width = fSubsession.videoWidth();
			//stream.height = fSubsession.videoHeight();
			//printf("rtsp video w=%d h=%d\n",fSubsession.videoWidth(),fSubsession.videoHeight());
			//if(chn == 0)
			//{
			//	printf("chn%d video tv_sec=%ld tv_usec=%ld pts=%llu time=%s", chn, presentationTime.tv_sec, presentationTime.tv_usec, stream.pts, ctime((time_t *)&presentationTime.tv_sec));
			//}
			//printf("chn%d rtsp data,len=%d\n",chn,stream.len);
			
			pStreamCB(&stream, dwContext);
		}
		else if(strcasecmp(fSubsession.mediumName(), "audio") == 0 && (strcasecmp(fSubsession.codecName(), "PCMA") == 0 || strcasecmp(fSubsession.codecName(), "PCMU") == 0))
		{
			/*static int fp = 0;
			static int fp_flag = 0;
			static int sum = 0;
			if(fp_flag == 0)//打开文件
			{
				fp = open("/root/pcma0",O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR|S_IWUSR);
				if(fp < 0)
					printf("open pcma file error\n");
				else
					fp_flag = 1;
			}
			
			if(fp_flag > 0)
			{
				sum += write(fp,fReceiveBuffer,frameSize);
				//printf("write pcma %d %d %d\n",fp_flag,frameSize,sum);
				fp_flag++;
				if(fp_flag == 500) {
					printf("write %d pcma\n",sum);
					fp_flag = -1;
				}
			}*/
			
			//printf("chn%d rtsp mediumName:%s codecName:%s\n", chn, fSubsession.mediumName(),fSubsession.codecName());
			
			real_stream_s stream;
			memset(&stream, 0, sizeof(stream));
			stream.chn = chn;
			stream.media_type = (strcasecmp(fSubsession.codecName(), "PCMA") == 0) ? MEDIA_PT_G711 : MEDIA_PT_PCMU;
			stream.frame_type = REAL_FRAME_TYPE_I;
			stream.pts = (unsigned long long)1000000*presentationTime.tv_sec + presentationTime.tv_usec;
			stream.data = fReceiveBuffer;
			stream.len = frameSize;
			stream.rsv = 8000;
			
			if(pStreamCB != NULL && chn < (int)(g_rtsp_client_count/2))
			{
				pStreamCB(&stream, dwContext);
			}
		}
		#if 0 //test
		if(strcasecmp(fSubsession.mediumName(), "audio") == 0)
		{
			printf("chn%d rtsp mediumName:%s codecName:%s\n", chn, fSubsession.mediumName(),fSubsession.codecName());
		}
		#endif 
	}
	else
	{
		//pthread_mutex_unlock(&g_rtspc_info[chn].lock);
	}

	 
	
	// Then continue, to request the next frame of data:
	continuePlaying();
}

Boolean DummySink::continuePlaying()
{
	if(fSource == NULL)
	{
		return False; // sanity check (should not happen)
	}
	
	// Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
	fSource->getNextFrame(fReceiveBuffer, fRecvBufferSize,
				afterGettingFrame, this,
				onSourceClosure, this);
	return True;
}

// Implementation of the other event handlers:

void subsessionAfterPlaying(void* clientData)
{
	//printf("subsessionAfterPlaying-1\n");
	//fflush(stdout);
	
	MediaSubsession* subsession = (MediaSubsession*)clientData;
	RTSPClient* rtspClient = (RTSPClient*)(subsession->miscPtr);
	
	// Begin by closing this subsession's stream:
	if(subsession->sink != NULL)
	{
		Medium::close(subsession->sink);
		subsession->sink = NULL;
	}
	
	// Next, check whether *all* subsessions' streams have now been closed:
	MediaSession& session = subsession->parentSession();
	MediaSubsessionIterator iter(session);
	while((subsession = iter.next()) != NULL)
	{
		if(subsession->sink != NULL)
		{
			return; // this subsession is still active
		}
	}
	
	// All subsessions' streams have now been closed, so shutdown the client:
	shutdownStream(rtspClient);
}

void subsessionByeHandler(void* clientData)
{
	MediaSubsession* subsession = (MediaSubsession*)clientData;
	RTSPClient* rtspClient = (RTSPClient*)subsession->miscPtr;
	
	UsageEnvironment& env = rtspClient->envir(); // alias
	
	env << *rtspClient << "Received RTCP \"BYE\" on \"" << *subsession << "\" subsession\n";
	
	// Now act as if the subsession had closed:
	subsessionAfterPlaying(subsession);
}

void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString)
{
	do
	{
		UsageEnvironment& env = rtspClient->envir(); // alias
		StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias
		
		if(resultCode != 0)
		{
			env << *rtspClient << "Failed to set up the \"" << *scs.subsession << "\" subsession: " << resultString << "\n";
			break;
		}
		
		//env << *rtspClient << "Set up the \"" << *scs.subsession
		//	<< "\" subsession (client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1 << ")\n";
		
		// Having successfully setup the subsession, create a data sink for it, and call "startPlaying()" on it.
		// (This will prepare the data sink to receive data; the actual flow of data from the client won't start happening until later,
		// after we've sent a RTSP "PLAY" command.)
		
		ourRTSPClient *p = (ourRTSPClient *)rtspClient;
		int size = 500*1024-4;//1024*1024-4;
		if(p->channel >= (int)(g_rtsp_client_count/2))
		{
			size = 200*1024-4;
		}
		
		scs.subsession->sink = DummySink::createNew(env, *scs.subsession, rtspClient->url(), size);
		// perhaps use your own custom "MediaSink" subclass instead
		if(scs.subsession->sink == NULL || ((DummySink *)(scs.subsession->sink))->fReceiveBuffer == NULL)
		{
			env << *rtspClient << "Failed to create a data sink for the \"" << *scs.subsession
				<< "\" subsession: " << env.getResultMsg() << "\n";
			break;
		}
		
		//env << *rtspClient << "Created a data sink for the \"" << *scs.subsession << "\" subsession\n";
		
		scs.subsession->miscPtr = rtspClient; // a hack to let subsession handle functions get the "RTSPClient" from the subsession 
		scs.subsession->sink->startPlaying(*(scs.subsession->readSource()), subsessionAfterPlaying, scs.subsession);
		
		// Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
		if(scs.subsession->rtcpInstance() != NULL)
		{
			scs.subsession->rtcpInstance()->setByeHandler(subsessionByeHandler, scs.subsession);
			//printf("has rtcp\n");//y
		}
		else
		{
			//printf("no rtcp\n");//n
		}
	}while(0);
	
	if(resultString)
	{
		delete[] resultString;
		resultString = NULL;
	}
	
	// Set up the next subsession, if any:
	setupNextSubsession(rtspClient);
}

void streamTimerHandler(void* clientData)
{
	ourRTSPClient* rtspClient = (ourRTSPClient*)clientData;
	StreamClientState& scs = rtspClient->scs; // alias
	
	scs.streamTimerTask = NULL;
	
	// Shut down the stream:
	shutdownStream(rtspClient);
}

void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString)
{
	UsageEnvironment& env = rtspClient->envir(); // alias
	//StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias
	
	Boolean success = False;
	
	do
	{
		if(resultCode != 0)
		{
			env << *rtspClient << "Failed to start playing session: " << resultString << "\n";
			break;
		}
		
		#if 0//csp modify 20130808
		// Set a timer to be handled at the end of the stream's expected duration (if the stream does not already signal its end
		// using a RTCP "BYE").  This is optional.  If, instead, you want to keep the stream active - e.g., so you can later
		// 'seek' back within it and do another RTSP "PLAY" - then you can omit this code.
		// (Alternatively, if you don't want to receive the entire stream, you could set this timer for some shorter value.)
		if(scs.duration > 0)
		{
			unsigned const delaySlop = 2; // number of seconds extra to delay, after the stream's expected duration.  (This is optional.)
			scs.duration += delaySlop;
			unsigned uSecsToDelay = (unsigned)(scs.duration*1000000);
			scs.streamTimerTask = env.taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc*)streamTimerHandler, rtspClient);
		}
		#endif
		
		//env << *rtspClient << "Started playing session";
		//if(scs.duration > 0)
		//{
		//	env << " (for up to " << scs.duration << " seconds)";
		//}
		//env << "...\n";
		
		success = True;
	}while(0);
	
	if(resultString != NULL)
	{
		delete[] resultString;
		resultString = NULL;
	}
	
	if(!success)
	{
		printf("continueAfterPLAY failed\n");
		
		// An unrecoverable error occurred with this stream.
		shutdownStream(rtspClient);
	}
	else
	{
		printf("continueAfterPLAY success\n");
		
		ourRTSPClient *p = (ourRTSPClient *)rtspClient;
		p->start = 1;
		
		ourRTSPClient::stream_count++;
	}
}

// By default, we request that the server stream its data using RTP/UDP.
// If, instead, you want to request that the server stream via RTP-over-TCP, change the following to True:
//#define REQUEST_STREAMING_OVER_TCP False

void setupNextSubsession(RTSPClient* rtspClient)
{
	UsageEnvironment& env = rtspClient->envir(); // alias
	StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias
	
	scs.subsession = scs.iter->next();
	if(scs.subsession != NULL)
	{
		if(!scs.subsession->initiate())
		{
			env << *rtspClient << "Failed to initiate the \"" << *scs.subsession << "\" subsession: " << env.getResultMsg() << "\n";
			setupNextSubsession(rtspClient); // give up on this subsession; go to the next one
		}
		else
		{
			//env << *rtspClient << "Initiated the \"" << *scs.subsession
			//	<< "\" subsession (client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1 << ")\n";
			
			// Continue setting up this subsession, by sending a RTSP "SETUP" command:
			//rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP, False, REQUEST_STREAMING_OVER_TCP);
			ourRTSPClient *p = (ourRTSPClient *)rtspClient;
			rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP, False, p->stream_over_tcp);
		}
		return;
	}
	
	// We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
	if(scs.session->absStartTime() != NULL)
	{
		// Special case: The stream is indexed by 'absolute' time, so send an appropriate "PLAY" command:
		rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY, scs.session->absStartTime(), scs.session->absEndTime());
	}
	else
	{
		//csp modify 20130808
		//scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
		scs.duration = 0.0;
		rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY);
	}
}

// Implementation of the RTSP 'response handlers':

void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString)
{
	do
	{
		//printf("continueAfterDESCRIBE-1\n");
		
		UsageEnvironment& env = rtspClient->envir(); // alias
		StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias
		
		if(resultCode != 0)
		{
			env << *rtspClient << "Failed to get a SDP description: " << resultString << "\n";
			delete[] resultString;
			break;
		}
		
		char* const sdpDescription = resultString;
		//env<<"********************************************************"<< "\n";
		//env << *rtspClient << "Got a SDP description:\n" << sdpDescription << "\n";
		//env<<"**************************end***************************"<< "\n";
		// Create a media session object from this SDP description:
		scs.session = MediaSession::createNew(env, sdpDescription);
		
		delete[] sdpDescription; // because we don't need it anymore
		
		if(scs.session == NULL)
		{
			env << *rtspClient << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
			break;
		}
		else if(!scs.session->hasSubsessions())
		{
			env << *rtspClient << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
			break;
		}
		
		//printf("continueAfterDESCRIBE-2\n");
		
		// Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
		// calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
		// (Each 'subsession' will have its own data source.)
		scs.iter = new MediaSubsessionIterator(*scs.session);
		setupNextSubsession(rtspClient);
		
		return;
	}while(0);
	
	// An unrecoverable error occurred with this stream.
	shutdownStream(rtspClient);
}

void continueAfterOPTIONS(RTSPClient* rtspClient, int resultCode, char* resultString)
{
	if(resultString != NULL)
	{
		delete[] resultString;
		resultString = NULL;
	}
	
	// Next, get a SDP description for the stream:
	ourRTSPClient *p = (ourRTSPClient *)rtspClient;
	p->sendDescribeCommand(continueAfterDESCRIBE, p->ourAuthenticator);
}

#define RTSP_CLIENT_VERBOSITY_LEVEL	0// 1	// by default, print verbose output from each "RTSPClient"

ourRTSPClient* openURL(UsageEnvironment& env, char const* progName, char const* rtspURL, char *user, char *pwd, unsigned char rtsp_over_tcp, int chn, RealStreamCB pCB, unsigned int dwContext)
{
	// Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
	// to receive (even if more than stream uses the same "rtsp://" URL).
	ourRTSPClient* rtspClient = ourRTSPClient::createNew(env, rtspURL, RTSP_CLIENT_VERBOSITY_LEVEL, progName, 0, user, pwd, rtsp_over_tcp, chn, pCB, dwContext);
	if(rtspClient == NULL)
	{
		env << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " << env.getResultMsg() << "\n";
		return NULL;
	}
	
	if(rtspClient->Start() != 0)
	{
		rtspClient->Destory();
		return NULL;
	}
	
	rtspClient->RegisterCallback(Callback, 0);
	
	return rtspClient;
}

void continueAfterTEARDOWN(RTSPClient* rtspClient, int resultCode, char* resultString)
{
	//printf("continueAfterTEARDOWN, pid:%ld\n", pthread_self());
	//fflush(stdout);
	
	if(resultString)
	{
		delete[] resultString;
		resultString = NULL;
	}
	
	shutdownStream(rtspClient);
	
	//printf("continueAfterTEARDOWN-over\n");
	//fflush(stdout);
}

void continueAfterPAUSE(RTSPClient* rtspClient, int resultCode, char* resultString)
{
	if(resultString)
	{
		delete[] resultString;
		resultString = NULL;
	}
	
	ourRTSPClient* p = (ourRTSPClient *)rtspClient;
	p->sendTeardownCommand(*p->scs.session, continueAfterTEARDOWN, p->ourAuthenticator);
}

void shutdownStream(RTSPClient* rtspClient, int exitCode)
{
	//printf("shutdownStream, pid:%ld\n", pthread_self());
	//fflush(stdout);
	
	UsageEnvironment& env = rtspClient->envir(); // alias
	StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias
	
	//printf("shutdownStream-1\n");
	//fflush(stdout);
	
	// First, check whether any subsessions have still to be closed:
	if(scs.session != NULL)
	{
		Boolean someSubsessionsWereActive = False;
		MediaSubsessionIterator iter(*scs.session);
		MediaSubsession* subsession;
		
		//printf("shutdownStream-2\n");
		//fflush(stdout);
		
		while((subsession = iter.next()) != NULL)
		{
			//printf("shutdownStream-3\n");
			//fflush(stdout);
			
			if(subsession->sink != NULL)
			{
				//printf("shutdownStream-4\n");
				//fflush(stdout);
				
				Medium::close(subsession->sink);
				subsession->sink = NULL;
				
				//printf("shutdownStream-5\n");
				//fflush(stdout);
				
				if(subsession->rtcpInstance() != NULL)
				{
					subsession->rtcpInstance()->setByeHandler(NULL, NULL); // in case the server sends a RTCP "BYE" while handling "TEARDOWN"
				}
				
				//printf("shutdownStream-6\n");
				//fflush(stdout);
				
				someSubsessionsWereActive = True;
			}
		}
		
		//printf("shutdownStream-7\n");
		//fflush(stdout);
		
		if(someSubsessionsWereActive)
		{
			printf("shutdownStream-TEARDOWN\n");
			fflush(stdout);
			
			// Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
			// Don't bother handling the response to the "TEARDOWN".
			rtspClient->sendTeardownCommand(*scs.session, NULL);
		}
		
		//printf("shutdownStream-8\n");
		//fflush(stdout);
	}
	
	env << *rtspClient << "Closing the stream.\n";
	
	ourRTSPClient *p = (ourRTSPClient *)rtspClient;
	p->eventLoopWatchVariable = 1;
	if(exitCode == 1)
	{
		usleep(3000*1000);
	}
	p->m_bExited = True;
	p->DoCallback(RTSP_FLAG_DISCONNECT, p->channel, exitCode, 0);
	
	//Medium::close(rtspClient);
	//Note that this will also cause this stream's "StreamClientState" structure to get reclaimed.
	
	//printf("shutdownStream-over\n");
	//fflush(stdout);
}

int RTSPC_Init(unsigned int max_client_num)
{
	g_rtsp_client_count = (max_client_num > MAX_RTSP_CLIENT_NUM) ? MAX_RTSP_CLIENT_NUM : max_client_num;
	
	int i = 0;
	for(i = 0; i < MAX_RTSP_CLIENT_NUM; i++)
	{
		g_rtspc_info[i].flagState = FLAG_FREE;
		g_rtspc_info[i].dwRTSPFlag = 0;
		g_rtspc_info[i].rtspClient = NULL;
		g_rtspc_info[i].scheduler = NULL;
		g_rtspc_info[i].env = NULL;
	}
	for(i = 0; i < (int)g_rtsp_client_count; i++)
	{
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);
		pthread_mutex_init(&g_rtspc_info[i].lock, &attr);
		pthread_mutexattr_destroy(&attr);
	}
	
	g_rtspc_loop = 1;
	
	return 0;
}

int RTSPC_DeInit()
{
	g_rtspc_loop = 0;
	return 0;
}

int RTSPC_Start(int chn, RealStreamCB pCB, unsigned int dwContext, char* streamInfo, unsigned int dwIp, unsigned short wPort, char *user, char *pwd, unsigned char rtsp_over_tcp)
{
	if(chn >= (int)g_rtsp_client_count)
	{
		return -1;
	}
	
	char rtspURL[64];
	memset(rtspURL, 0, sizeof(rtspURL));
	
	struct in_addr serv;
	serv.s_addr = dwIp;
	
	if(wPort == 554)
	{
		sprintf(rtspURL, "rtsp://%s", inet_ntoa(serv));
	}
	else
	{
		sprintf(rtspURL, "rtsp://%s:%d", inet_ntoa(serv), wPort);
	}
	
	if(streamInfo)
	{
		strcat(rtspURL, "/");
		strcat(rtspURL, streamInfo);
	}
	
	return RTSPC_Startbyurl(chn, pCB, dwContext, rtspURL, user, pwd, rtsp_over_tcp);
}

int RTSPC_Startbyurl(int chn, RealStreamCB pCB, unsigned int dwContext, char* rtspURL, char *user, char *pwd, unsigned char rtsp_over_tcp)
{
	//rtsp_over_tcp = 0;//for debug
	
	if(chn >= (int)g_rtsp_client_count)
	{
		return -1;
	}
	
	char url[128];
	memset(url, 0, sizeof(url));
	//sprintf(url, "rtsp://%s:%s@%s", user, pwd, rtspURL+7);
	strcpy(url, rtspURL);
	//if(rtsp_over_tcp)
	//{
	//	strcat(url, "?tcp");
	//}
	rtspURL = url;
	
	//printf("chn%d RTSPC_Startbyurl(%s) - begin (%s)\n", chn, rtspURL, rtsp_over_tcp ? "tcp" : "udp");
	fflush(stdout);
	
	pthread_mutex_lock(&g_rtspc_info[chn].lock);
	
	if(g_rtspc_info[chn].flagState == FLAG_RUN)
	{
		ourRTSPClient *p = g_rtspc_info[chn].rtspClient;
		if(strcmp(rtspURL, p->rtsp_url) || 
			strcmp(user, p->username) || 
			strcmp(pwd, p->password) || 
			pCB != p->pStreamCB || 
			dwContext != p->dwStreamContext || 
			rtsp_over_tcp != p->stream_over_tcp)
		{
			printf("chn%d RTSPC_Startbyurl param change:(%s,%s)(%s,%s)(%s,%s)(0x%08x,0x%08x)(%d,%d)(%d,%d)\n", 
					chn, 
					rtspURL, p->rtsp_url, 
					user, p->username, 
					pwd, p->password, 
					(unsigned int)pCB, (unsigned int)p->pStreamCB, 
					dwContext, p->dwStreamContext, 
					rtsp_over_tcp, p->stream_over_tcp);
			pthread_mutex_unlock(&g_rtspc_info[chn].lock);
			RTSPC_Stop(chn);
			pthread_mutex_lock(&g_rtspc_info[chn].lock);
		}
		else
		{
			//printf("chn%d RTSPC_Startbyurl - not change\n", chn);
			pthread_mutex_unlock(&g_rtspc_info[chn].lock);
			return 0;
		}
	}
	else if(g_rtspc_info[chn].flagState == FLAG_DESTORY)
	{
		//printf("chn%d RTSPC_Startbyurl - destroying\n", chn);
		pthread_mutex_unlock(&g_rtspc_info[chn].lock);
		return -1;
	}
	
	if(g_rtspc_info[chn].scheduler == NULL)
	{
		g_rtspc_info[chn].scheduler = BasicTaskScheduler::createNew();
	}
	if(g_rtspc_info[chn].env == NULL)
	{
		g_rtspc_info[chn].env = BasicUsageEnvironment::createNew(*g_rtspc_info[chn].scheduler);
	}
	
	ourRTSPClient* p = openURL(*g_rtspc_info[chn].env, "NVR", rtspURL, user, pwd, rtsp_over_tcp, chn, pCB, dwContext);
	if(p == NULL)
	{
		if(g_rtspc_info[chn].env != NULL)
		{
			g_rtspc_info[chn].env->reclaim();
			g_rtspc_info[chn].env = NULL;
		}
		if(g_rtspc_info[chn].scheduler != NULL)
		{
			delete g_rtspc_info[chn].scheduler;
			g_rtspc_info[chn].scheduler = NULL;
		}
		pthread_mutex_unlock(&g_rtspc_info[chn].lock);
		return -1;
	}
	
	g_rtspc_info[chn].rtspClient = p;
	g_rtspc_info[chn].flagState = FLAG_RUN;
	g_rtspc_info[chn].dwRTSPFlag = 0;
	
	pthread_mutex_unlock(&g_rtspc_info[chn].lock);
	
	//printf("chn%d RTSPC_Startbyurl - over\n", chn);
	fflush(stdout);
	
	return 0;
}

int RTSPC_Stop(int chn)
{
	if(chn >= (int)g_rtsp_client_count)
	{
		return -1;
	}
	
	int bExit = False;
	
	//printf("chn%d RTSPC_Stop - begin\n", chn);
	
	pthread_mutex_lock(&g_rtspc_info[chn].lock);
	
	ourRTSPClient* p = g_rtspc_info[chn].rtspClient;
	if(p != NULL)
	{
		//printf("chn%d RTSPC_Stop - 1\n", chn);
		if(g_rtspc_info[chn].flagState == FLAG_RUN)
		{
			g_rtspc_info[chn].flagState = FLAG_DESTORY;
			bExit = True;
		}
		//printf("chn%d RTSPC_Stop - 2, bExit: %d\n", chn, bExit);
		pthread_mutex_unlock(&g_rtspc_info[chn].lock);
		
		if(bExit)
		{
			p->Stop(0);
			//printf("chn%d RTSPC_Stop - 3\n", chn);
		}
	}
	else
	{
		//printf("chn%d RTSPC_Stop - not opened\n", chn);
		g_rtspc_info[chn].flagState = FLAG_FREE;
		pthread_mutex_unlock(&g_rtspc_info[chn].lock);
		return 0;
	}

	
	pthread_mutex_lock(&g_rtspc_info[chn].lock);
	
	
	
	while(g_rtspc_info[chn].rtspClient != NULL || g_rtspc_info[chn].flagState != FLAG_FREE)
	{
		usleep(1);
	}
	
	pthread_mutex_unlock(&g_rtspc_info[chn].lock);
	
	//printf("chn%d RTSPC_Stop - over\n", chn);
	
	return 0;
}

int RTSPC_GetLinkStatus(int chn)
{
	if(chn >= (int)g_rtsp_client_count)
	{
		return 0;
	}
	
	pthread_mutex_lock(&g_rtspc_info[chn].lock);
	
	int ret = (g_rtspc_info[chn].rtspClient != NULL && g_rtspc_info[chn].flagState == FLAG_RUN);
	
	pthread_mutex_unlock(&g_rtspc_info[chn].lock);
	
	return ret;
}

