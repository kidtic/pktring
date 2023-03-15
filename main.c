#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pktring.h"

#include <pthread.h>
#include <unistd.h>

#define DATPKTLEN 1024

PKT_RING* pring;
unsigned long recvpkt=0;
unsigned long errorpkt=0;
unsigned long losspkt=0;
void recvtask(void)
{
    char indata[DATPKTLEN];
    int ret=0;
    int bt=0;
    unsigned int recvcnt=1;
    while(1)
    {
        bt = get_pktring(pring,indata,DATPKTLEN);
        if(bt!=DATPKTLEN)continue;
        ret = checkpkt(indata,DATPKTLEN);
        if(ret == 0)
        {
            errorpkt++;
            recvcnt++;
            recvpkt++;
            //printf("check erro cnt:%d\n",errorpkt);
        }
        else if(recvcnt!=getpktcnt(indata)){
            recvcnt=getpktcnt(indata)+1;
            losspkt++;
            recvpkt++;
        }
        else{
                recvpkt++;
                recvcnt++;
        }
        //usleep(1);

    }
}
void recvspeed(void)
{
    unsigned long start,end;

    while(1)
    {
        start=recvpkt;
        sleep(1);
        end=recvpkt;
        printf("recv speed:%.3f MByte/s recvpkt:%d losspkt:%d errorpkt:%d \n",(double)(end-start)*DATPKTLEN/(1024.0*1024.0),recvpkt,losspkt,errorpkt);
    }

}

int main()
{
    int i;
    unsigned char cnt;
    unsigned char tmp;
    printf("Hello World!\n");
    pring = creat_pktring(DATPKTLEN);
    srand((int)time(0));

    char outdata[DATPKTLEN];
    unsigned int sendpktcnt=0;

    //crctest

    crc32_init();
    unsigned long crcd;
    crcd=crc32("1234567890",10);
    printf("CRC32=%08X\n",crcd);



    pthread_t sendtd;

    pthread_create(&sendtd,NULL,recvtask,0);

    pthread_create(&sendtd,NULL,recvspeed,0);

    for(i=0;i<DATPKTLEN;i++){
        outdata[i]=rand()%0xff;
    }
    while(1)
    {
        sendpktcnt++;
        outdata[rand()%DATPKTLEN]=rand()%0xff;
        /*for(i=10;i<DATPKTLEN/2;i++){
            outdata[i]=rand()%0xff;
        }*/
        updatepkt(outdata,DATPKTLEN,sendpktcnt);
        //------惨入误码
        //if(rand()%100==0)outdata[34]=rand()%0xff;
        //if(rand()%100==0){tmp=outdata[57];outdata[57]=outdata[56];outdata[56]=tmp;}
        while(put_pktring(pring,outdata,DATPKTLEN)==0);
        //usleep(10);
    }


#if 0
    for(i=0;i<2048;i++){
        outdata[i]=rand()%0xff;
    }
    updatepkt(outdata,2048,sendpktcnt);

    printf("check:%d\n",checkpkt(outdata,2048));
    for(i=0;i<2048;i++){
        outdata[i]=rand()%0xff;
    }
    printf("check:%d\n",checkpkt(outdata,2048));
    updatepkt(outdata,2048,sendpktcnt);
    printf("check:%d\n",checkpkt(outdata,2048));
    outdata[rand()%2000]=0;
    printf("check:%d\n",checkpkt(outdata,2048));
    updatepkt(outdata,2048,sendpktcnt);
    printf("check:%d\n",checkpkt(outdata,2048));
    outdata[1]=0;
    printf("check:%d\n",checkpkt(outdata,2048));
    updatepkt(outdata,2048,sendpktcnt);
    printf("check:%d\n",checkpkt(outdata,2048));
    outdata[1000]=0xff;
    printf("check:%d\n",checkpkt(outdata,2048));
    updatepkt(outdata,2048,sendpktcnt);
    printf("check:%d\n",checkpkt(outdata,2048));

    unsigned char chch[16]={0};
    updatepkt(chch,16,123456);

    for(i=0;i<16;i++){
        printf("0x%x ",chch[i]);
    }

    printf("crc:%x\n",(unsigned short)CRC16_USB(chch,14));


#endif


    return 0;
}





