#ifndef _PKTRING_H_
#define _PKTRING_H_

/*dma的时候请注意，如果pktring满了，将不会执行dma操作*/
/*
	文件：pktring.h
	说明：基于数据包的环形缓冲区。（支持dma）
	作者：zouzhiqiang
*/
#include "stdio.h"
#include "string.h"


//#define DMA_SUPPORT		//支持DMA技术

#define PKTRING_DEBUG(ch) printf(ch)

#ifdef DMA_SUPPORT
	#define PKT_MALLOC(size) cacheDmaMalloc(size)
	#define PKT_FREE(ptr) free(ptr)
#else
	#define PKT_MALLOC(size) malloc(size)
	#define PKT_FREE(ptr) free(ptr)
#endif



//最大缓存包
#define PKTRING_MAX_NUM 256

typedef struct
{
    unsigned int pktLen;		//包大小，同时也确定了pPktBuffer的界限，pRead不能>=pktlen，当pktlen为0时，为空pkt
	unsigned int pRead;			//为后续设计，可以支持单字节读取
    volatile char* pPktBuffer;
}R_PKT;

typedef struct
{
    unsigned int maxpktSize;	//每个包的最大size
    volatile unsigned int pIn; //环形缓冲的头指针，指向一个正准备写的空pkt
    volatile unsigned int pOut; //环形缓冲的尾指针，指向一个最后一个有数据的pkt
    R_PKT pktbuf[PKTRING_MAX_NUM];
}PKT_RING;


PKT_RING* creat_pktring(unsigned int maxpktLen);

void delate_pktring(PKT_RING* pRing);

//将数据拷贝到pktring中的一个pkt区。pkt+1
unsigned int put_pktring(PKT_RING* pRing,char* buf,unsigned int size);

//从pktring中的一个pkt区，取出数据到buf中。若buf大小不够就不会取。
unsigned int get_pktring(PKT_RING* pRing,char* buf,unsigned int recvBufsize);

//当前包长度
unsigned int size_pktring(PKT_RING* pRing);
//判断
int isEmpty_pktring(PKT_RING* pRing);
int isFull_pktring(PKT_RING* pRing);

//清空
void clear_pktring(PKT_RING* pRing);

#endif
