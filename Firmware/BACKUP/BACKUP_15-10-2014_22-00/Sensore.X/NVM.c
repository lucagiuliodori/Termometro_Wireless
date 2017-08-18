#include <xc.h>
#include "NVM.h"

void NVMRead(unsigned char addr,char *dest,unsigned char len)
{
    unsigned char i;

    for(i=0;i<len;i++)
    {
        CLRWDT();
        (*(dest+i))=eeprom_read(addr+i);
    }
}

void NVMWrite(unsigned char addr,char *source,unsigned char len)
{
    unsigned char i;

    for(i=0;i<len;i++)
    {
        CLRWDT();
	eeprom_write((addr+i),(*(source+i)));
    }
}	
