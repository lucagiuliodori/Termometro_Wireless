#include <xc.h>
#include "OUT.h"

#define mInitOut() TRISA&=0b11011111; TRISC&=0b01010111; ANSEL&=0b11011111; PORTC|=0b10000000

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
