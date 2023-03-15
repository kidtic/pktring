//循环缓冲区

#include "pktring.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


PKT_RING* creat_pktring(unsigned int maxpktLen)
{
    int i=0;
    PKT_RING* pRet=(PKT_RING*)malloc(sizeof (PKT_RING));
    if(pRet == NULL)return NULL;

    //给pkt区分配内存
    pRet->maxpktSize=maxpktLen;
    pRet->pIn=0;
    pRet->pOut=0;
    for(i=0;i<PKTRING_MAX_NUM;i++)
    {
        pRet->pktbuf[i].pktLen=0;
        pRet->pktbuf[i].pRead=0;
        pRet->pktbuf[i].pPktBuffer=PKT_MALLOC(maxpktLen);
        if(pRet->pktbuf[i].pPktBuffer == NULL)
        {
            PKTRING_DEBUG("creat_pktring error\n");
            while (i)
            {
                i--;
                PKT_FREE(pRet->pktbuf[i].pPktBuffer);
            }
            return NULL;
        }
    }


    return pRet;
}

void delate_pktring(PKT_RING* pRing)
{
    int i;
    for(i=0;i<PKTRING_MAX_NUM;i++)
    {
         PKT_FREE(pRing->pktbuf[i].pPktBuffer);
    }
    free(pRing);
}

unsigned int size_pktring(PKT_RING* pRing)
{
    if(pRing->pIn >= pRing->pOut){
        return pRing->pIn - pRing->pOut;
    }
    else{
        return PKTRING_MAX_NUM - (pRing->pOut - pRing->pIn);
    }
}

int isFull_pktring(PKT_RING* pRing){
    if(pRing->pIn+1 == pRing->pOut)
    {
        return 1;
    }
    if(pRing->pOut==0 && pRing->pIn == (PKTRING_MAX_NUM-1))
    {
        return 1;
    }
    return 0;
}
int isEmpty_pktring(PKT_RING* pRing){
    if(pRing->pIn == pRing->pOut){
        return 1;
    }
    return 0;
}

//将数据拷贝到pktring中的一个pkt区。pkt+1
unsigned int put_pktring(PKT_RING* pRing,char* buf,unsigned int size)
{
    if(isFull_pktring(pRing))return 0;
    if(size > pRing->maxpktSize)return 0;

#ifdef DMA_SUPPORT
    dmaTrans(pRing->pktbuf[pRing->pIn].pPktBuffer, buf, size);
#else
    memcpy(pRing->pktbuf[pRing->pIn].pPktBuffer, buf, size);
#endif
    pRing->pktbuf[pRing->pIn].pktLen=size;

    //change pIn
    if(pRing->pIn == PKTRING_MAX_NUM-1){
        pRing->pIn=0;
    }
    else{
        pRing->pIn++;
    }
    return size;
}

//从pktring中的一个pkt区，取出数据到buf中。若buf大小不够就不会取。
unsigned int get_pktring(PKT_RING* pRing,char* buf,unsigned int recvBufsize)
{
    if(isEmpty_pktring(pRing))return 0;
    if(recvBufsize < pRing->pktbuf[pRing->pOut].pktLen){
        PKTRING_DEBUG("recvBufsize must > pktSize\n");
        return 0;
    }
    unsigned int ret = pRing->pktbuf[pRing->pOut].pktLen;
    memcpy(buf,pRing->pktbuf[pRing->pOut].pPktBuffer, ret);

    pRing->pktbuf[pRing->pOut].pktLen = 0;

    //change pOut
    if(pRing->pOut == PKTRING_MAX_NUM-1){
        pRing->pOut=0;
    }
    else{
        pRing->pOut++;
    }
    return ret;
}

void clear_pktring(PKT_RING* pRing)
{
    pRing->pOut=0;
    pRing->pIn=0;
}




/*-----------------------------pkt test------------------------------------*/


#define CHECKSUM_CONFIG 1
#define CRC_CHECK_CONFIG 2
#define CHECKSUM_MUTIL_CONFIG 3

#define CHECK_CONFIG CHECKSUM_MUTIL_CONFIG   /*配置校验方式*/

#if (CHECK_CONFIG==CHECKSUM_CONFIG)
    #define CHECKCODE_LEN 1
#elif (CHECK_CONFIG==CRC_CHECK_CONFIG)
    #define CHECKCODE_LEN 2
#elif (CHECK_CONFIG==CHECKSUM_MUTIL_CONFIG)
    #define CHECKCODE_LEN 4
    unsigned char checkCode[CHECKCODE_LEN]={233,179,157,163};
#endif


int checkpkt(unsigned char* data,unsigned int size)
{
    //if(data[0]!=0xeb || data[1]!=0x90)return 0;
#if (CHECK_CONFIG==CRC_CHECK_CONFIG)
    unsigned short crcdata = (unsigned short)crc32(data,size-CHECKCODE_LEN);
    if((data[size-2]==((crcdata>>8)&0xff)) && (data[size-1]==((crcdata)&0xff))){
        return 1;
    }
    else{
        return 0;
    }

#else

    unsigned int sum=0;
    int i;
    for(i=0;i<size-CHECKCODE_LEN;i++)
    {
        sum+=data[i];
    }
    unsigned char checkdata[CHECKCODE_LEN];
    for(i=0; i<CHECKCODE_LEN; i++)
    {

#if (CHECK_CONFIG==CHECKSUM_CONFIG)
        checkdata[i]=~(sum&0xff)+1;
#elif (CHECK_CONFIG==CHECKSUM_MUTIL_CONFIG)
        checkdata[i]=sum%checkCode[i];
#endif
    }
    for(i=size-CHECKCODE_LEN; i<size; i++){
        if(data[i]!=checkdata[i-size+CHECKCODE_LEN]){
            return 0;
        }
    }

    return 1;
#endif
}

unsigned int getpktcnt(unsigned char* data)
{
    return *(unsigned int*)(&data[4]);
}

void updatepkt(unsigned char* data,unsigned int size,unsigned int cnt)
{
    data[0]=0xeb;
    data[1]=0x90;
    data[2]=0xeb;
    data[3]=0x90;

    *(unsigned int*)(&data[4])=cnt;

#if (CHECK_CONFIG==CRC_CHECK_CONFIG)
    unsigned short crcdata = (unsigned short)crc32(data,size-CHECKCODE_LEN);
    data[size-2]=(unsigned char)(crcdata>>8)&0xff;
    data[size-1]=(unsigned char)(crcdata)&0xff;
#else
    unsigned int sum=0;
    int i;
    for(i=0;i<size-CHECKCODE_LEN;i++)
    {
        sum+=data[i];
    }
    unsigned char checkdata[CHECKCODE_LEN];
    for(i=0; i<CHECKCODE_LEN; i++)
    {
#if (CHECK_CONFIG==CHECKSUM_CONFIG)
        checkdata[i]=~(sum&0xff)+1;
#elif (CHECK_CONFIG==CHECKSUM_MUTIL_CONFIG)
        checkdata[i]=sum%checkCode[i];
#endif
    }

    for(i=size-CHECKCODE_LEN; i<size; i++){
        data[i]=checkdata[i-size+CHECKCODE_LEN];
    }
#endif
}

