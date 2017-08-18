
#include <xc.h>
#include "SYS.h"
#include "IN.h"
#include "DIN.h"
#include "OUT.h"
#include "LED.h"
#include "NVM.h"
#include "SPI.h"
#include "CC1.h"
#include "RTX.h"
#include "PGM.h"
#include "ADC.h"
#include "Sensore.h"

//Prima configuration word (l'ordine con cui vengono invocate le macro indica al compilatore l'indice della configuration word).
__CONFIG(FOSC_INTRC_NOCLKOUT & WDTE_ON & PWRTE_ON & MCLRE_ON & CP_ON & CPD_ON & BOREN_ON & IESO_ON & FCMEN_ON & LVP_OFF);
//Seconda configuration word (l'ordine con cui vengono invocate le macro indica al compilatore l'indice della configuration word).
__CONFIG(BOR4V_BOR21V & WRT_OFF);

/*NOTE:

- timer0: tick di sistema con base tempi 10ms.
*/

void interrupt INTIsr(void)
{
    /*Controlla se si è verificato l'interrupt del timer 0.*/
    if(INTCONbits.T0IF==1)
    {
        //Cancella il flag dell'interrupt.
        INTCONbits.T0IF=0;

        //Chiama le funzioni interessate.
        SYSInt();
    }
}

void main(void)
{
    unsigned char i;

    //Inizializzazione.
    for(i=0;i<3;i++)
    {
        CLRWDT();

        SYSInit(i);
        INInit(i);
        OUTInit(i);
        DINInit(i);
        LEDInit(i);
        SPIInit(i);
        CC1Init(i);
        RTXInit(i);
        PGMInit(i);
        ADCInit(i);
    }

    //Abilita gli interrupt.
    INTCONbits.GIE=1;

    //MainLoop.
    while(1)
    {
        CLRWDT();
        //Task.
        SYSTask();
        DINTask();
        ADCTask();
        RTXTask();
    }
	
}
