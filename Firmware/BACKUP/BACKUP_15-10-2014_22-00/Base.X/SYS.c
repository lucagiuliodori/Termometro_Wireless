
#include <xc.h>
#include "IN.h"
#include "DIN.h"
#include "LED.h"
#include "CC1.h"
#include "RTX.h"
#include "PGM.h"
#include "SLP.h"
#include "SYS.h"

//Definizione macro.
#define mTMR0Reset() TMR0=255-156
#define mSysInit() OPTION_REG=0b00000110; OSCCON=0b01110001; WDTCON=0b00001001; IOCB=0b00000000; mTMR0Reset(); INTCON|=0b00100000; INTCON&=0b11111011; PIR1=0b00000000; PIR2=0b00000000; PIE1=0b00000000; PIE2=0b00000000	//prescaler timer0 1:128, frequenza 8MHz

void SYSInit(unsigned char state)
{
    switch(state)
    {
        case 0:
            //Inizializzazione hardware.
            mSysInit();
            break;
        case 1:
            //Inizializzazione variabili.
            u_SYS_STA.VAL=0;
            break;
//        case 2:
//            //Inizializzazione applicazione.
//            break;
    }
}

void SYSInt(void)
{
	//Setta il flag per il tick di sistema.
	u_SYS_STA.TICK=1;

	//Ricarica il timer 0.
	mTMR0Reset();
}	

void SYSTask(void)
{
    unsigned char i;

    //Gestisce il tick di sistema.
    if(u_SYS_STA.TICK)
    {
        //Cancella il flag.
        u_SYS_STA.TICK=0;

        //Chiama le funzioni interessate.
        RTXTick();
        DINTick();
    }

    if(u_in_ev_long.sel)
    {
        //Cancella il flag.
        u_in_ev_long.sel=0;
        PGMSelLong();
    }

    if(rtx_ev.sleep && (!(INGetSEL())))
    {
        //Azzera il flag.
        rtx_ev.sleep=0;
        //Manda il sistema in sleep.
        SLPSleep();
    }

    if(flg_slp.exitSleep)
    {
        //Cancella il flag.
        flg_slp.exitSleep=0;
        //Reinizializza alcuni blocchi.
        for(i=0;i<3;i++)
            CC1Init(i);
    }
}