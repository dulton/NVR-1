#include "common_basetypes.h"
#include "common_msgcmd.h"

#include <unistd.h>

void *TestCmdDealFxn(void *arg)
{
    SMsgCmdHdr sCmdHdr = (SMsgCmdHdr)arg;
    int i;

    if (NULL == sCmdHdr)
    {
        printf("NULL == sCmdHdr\n");
        return NULL;
    }
    
    while (1)
    {
       if (0 == ReadMsgCmd(sCmdHdr, &i))
       {
           printf("ReadMsgCmd i=%u\n", i);
           AckMsgCmd(sCmdHdr);
       }
       else
       {
            usleep(0);
        }
    }
}

int main()
{
    u32 i = 0;

    PublicInit();

    HDDHDR hddHdr = PublicGetHddManage();
    if (NULL == hddHdr)
    {
        printf("PublicGetHddManage err\n");
    }
    else
    {
        printf("PublicGetHddManage ok\n");
    }
    
    SMsgCmdHdr sCmdHdr = CreateMsgCmd(sizeof(u32));

    int ret;
    pthread_t threadid;
    ret = pthread_create(&threadid, NULL, TestCmdDealFxn, sCmdHdr);

    for ( i = 0; i < 100; i++)
    {
        printf("WriteMsgCmd i=%lu\n", i);
        WriteMsgCmd(sCmdHdr, &i);
    }

    
    
    return 0;
}

