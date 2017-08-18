#include <xc.h>
#include "SLP.h"
#include "CC1.h"
#include "OUT.h"

void SLPSleep(void)
{
    unsigned char IOCBTmp,INTCONTmp,WPUBTmp,WDTCONTmp,OPTION_REGTmp;
    //Setta il CC1101 in sleep.
    CC1Sleep();
    //Attiva solo l'interrupt sulla PORTB.
    IOCBTmp=IOCB;
    IOCB=0b00000100;
    INTCONTmp=INTCON;
    INTCON=0b10001000;
    //Attiva solo il pull-up del tasto sel.
    WPUBTmp=WPUB;
    WPUB=0b00000100;
    //Setta il postscaler del watchdog-timer.
    WDTCONTmp=WDTCON;
    WDTCON=0b00010111;
    //Setta il prescaler per il watchdog-timer.
    OPTION_REGTmp=OPTION_REG;
    OPTION_REG=0b00001010;
    //Va in sleep.
    CLRWDT();
    SLEEP();
    //Ripristina i regitri.
    IOCB=IOCBTmp;
    INTCON=INTCONTmp;
    WPUB=WPUBTmp;
    WDTCON=WDTCONTmp;
    OPTION_REG=OPTION_REGTmp;
    //Setta il flag di uscita dallo sleep.
    flg_slp.exitSleep=1;
}

