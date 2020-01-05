#include  "hc05.h"
#include "wk2124.h"


void BlueTooth_Sendbuff(u8 *buff, u32 length)
{
    Wk2xxxSendBuf(1, buff, length);
}

u32 bluetooth_getbuff(u8 *get_buff)
{
    u32 length = 0;

    length = Wk2xxxGetBuf(1, get_buff);
    return length;
}
