#include <xc.h>
#include "OUT.h"

#define mInitOut() TRISA=0b00000000; TRISC&=0b01010111; ANSEL&=0b11011111; PORTA&=0b00100000; PORTC|=0b10000000

void OUTInit(unsigned char sta)
{
    switch(sta)
    {
        case 0:
            //Inizializzazione hardware.
            mInitOut();
            break;
//        case 1:
//            //Inizializzazione variabili.
//            break;
//        case 2:
//            //Inizializzazione applicazione.
//            break;
    }
}

void OUTSetHTCDataNibble(unsigned char val)
{
    unsigned char dataToSet=0;
    //Vengono considerati solo i primi 4 bit del valore passato.
    if(val&0b00000001)
        dataToSet|=0b00001000;
    if(val&0b00000010)
        dataToSet|=0b00000100;
    if(val&0b00000100)
        dataToSet|=0b00000010;
    if(val&0b00000001)
        dataToSet|=0b00000001;

    dataToSet&=0b00001111;
    port_DATA|=dataToSet;
    dataToSet|=0b11110000;
    port_DATA&=dataToSet;
}
