
static unsigned long table[256];

//位逆转
static unsigned long bitrev(unsigned long input, int bw)
{
    int i;
    unsigned long var;
    var = 0;
    for(i=0;i<bw;i++)
    {
        if(input & 0x01)
        {
            var |= 1<<(bw-1-i);
        }
        input>>=1;
    }
    return var;
}

//码表生成
//如:X32+X26+...X1+1,poly=(1<<26)|...|(1<<1)|(1<<0)
void crc32_init()
{
    int i;
    int j;
    unsigned long c;
    unsigned long poly = 0x4C11DB7;
    poly=bitrev(poly,32);
    for(i=0; i<256; i++)
    {
        c = i;
        for (j=0; j<8; j++)
        {
            if(c&1)
            {
                c=poly^(c>>1);
            }
            else
            {
                c=c>>1;
            }
        }
        table[i] = c;
    }
}

unsigned long crc32(void* input, int len)
{
    int i;
    unsigned char* pch;
    unsigned long crc = 0xFFFFFFFF;
    pch = (unsigned char*)input;
    for(i=0;i<len;i++)
    {
        crc = (crc>>8)^table[(unsigned char)(crc^*pch)];
        pch++;
    }
    crc ^= 0xFFFFFFFF;

    return crc;
}
