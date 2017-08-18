#include <xc.h>
#include "Sensore.h"
#include "DIN.h"
#include "NVM.h"
#include "LED.h"
#include "OUT.h"
#include "PGM.h"

#define ADDR_STARTNVM       0x00    //Indirizzo iniziale della NVM.
#define VAL_EEP_CHECK1      0xC5	//Valore per l'indirizzo di controllo eeprom.
#define VAL_EEP_CHECK2      0x5C	//Valore per l'indirizzo di controllo eeprom.
#define RST_EEP_MENU        0x00	//Valore di reset per il menù.

#define mPgmInit() T1CON=0b00000001     //Attiva il timer 1 per ricavare il valore casuale per l'id del sense.

static void PGMReadNvm(void);
static void PGMNvmReset(void);
static void PGMCheckNvm(void);

void PGMInit(unsigned char state)
{
    switch(state)
    {
        case 0:
            //Inizializzazione hardware.
            mPgmInit();
            break;
        case 1:
            //Inizializzazione variabili.
            PGMReadNvm();
            PGMCheckNvm();
            break;
//        case 2:
//            //Inizializzazione applicazione.
//            break;
    }
}	

static void PGMReadNvm(void)
{
    NVMRead(ADDR_STARTNVM,(char *)(&(pgm_cfg)),sizeof(ts_PGM_CFG));
}	

void PGMWriteNvm(void)
{
    NVMWrite(ADDR_STARTNVM,(char *)(&(pgm_cfg)),sizeof(ts_PGM_CFG));
}

static void PGMCheckNvm(void)
{
    if(pgm_cfg.valChk1!=VAL_EEP_CHECK1 || pgm_cfg.valChk2!=VAL_EEP_CHECK2)
        PGMNvmReset();        //Esegue il reset dei valori.
}	

static void PGMNvmReset(void)
{
    //Setta i valori.
    pgm_cfg.valChk1=VAL_EEP_CHECK1;
    pgm_cfg.valChk2=VAL_EEP_CHECK2;
    pgm_cfg.menu=RST_EEP_MENU;
    pgm_cfg.sensId=0x00;
    pgm_cfg.baseId=0x00;
    //Esegue la scrittura della NVM.
    PGMWriteNvm();
    //Esegue un reset del microcontrollore.
    while(1)
        NOP();
}

void PGMSelLong(void)
{
    //Genera il numero casuale.
    pgm_cfg.sensId=TMR1L^TMR1H;
    //Modifica il valore del menù per segnala che il sense è programmato.
    pgm_cfg.menu=0x01;
    //Esegue la scrittura della NVM.
    PGMWriteNvm();
    //Esegue alcuni lampeggi (sospensivi) per segnalare che la procedura è andata a buon fine.
    LEDNormalFlash(3);
}
