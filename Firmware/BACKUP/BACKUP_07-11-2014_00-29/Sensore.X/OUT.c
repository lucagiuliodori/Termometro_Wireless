#include <xc.h>
#include "OUT.h"

#define mInitOut() TRISA&=0b00000000; TRISB&=0b00010111; TRISC&=0b00010000; ANSELA&=0b00000000; ANSELB&=0b00010010; ANSELC&=0b00000000; PORTB&=0b11011111; PORTC|=0b10000000

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
