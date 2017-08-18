#include <xc.h>
#include "Sensore.h"
#include "DIN.h"
#include "NVM.h"
#include "LED.h"
#include "OUT.h"
#include "SLP.h"
#include "PGM.h"

#define VAL_EEP_CHECK1      0xC5	//Valore per l'indirizzo di controllo eeprom.
#define VAL_EEP_CHECK2      0x5C	//Valore per l'indirizzo di controllo eeprom.
#define RST_EEP_MENU        0x00	//Valore di reset per il men�.

#define TICK_TMRPGM         100         //Tick per il timer di fine programmazione.

#define mPgmInit() T1CON=0b00000001     //Attiva il timer 1 per ricavare il valore casuale per l'id del sense.

unsigned char tmrPgm;

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
            tmrPgm=0;
            pgm_flg.val=0;
            PGMReadNvm();
            PGMCheckNvm();
            break;
//        case 2:
//            //Inizializzazione applicazione.
//            break;
    }
}	

void PGMTick(void)
{
    //Timer di termine programmazione.
    if(tmrPgm)
    {
        tmrPgm--;
        if(!(tmrPgm))
        {
            //Cancella il flag.
            pgm_flg.pgmonoff=0;
            //Spegne il led.
            OUTSetLEDOff();
            //Manda il dispositivo in sleep.
            flg_slp.goSleep=1;
        }
    }
}

static void PGMReadNvm(void)
{
    NVMRead((unsigned short)(&(nvm_pgm_cfg)),(char *)(&(pgm_cfg)),sizeof(ts_PGM_CFG));
}	

void PGMWriteNvm(void)
{
    NVMWrite((unsigned short)(&(nvm_pgm_cfg)),(char *)(&(pgm_cfg)),sizeof(ts_PGM_CFG));
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
    RESET();
}

void PGMPgmOff(void)
{
    //Setta il timer.
    tmrPgm=0;
    //Setta il flag.
    pgm_flg.pgmonoff=0;
}

void PGMSelDown(void)
{
    //Segnala che � stata richiesta la programmazione dall'utente.
    pgm_flg.pgmonoff=1;
    //Attiva il led per segnalare che la programmazione � attiva.
    OUTSetLEDOn();
    //Genera un nuovo id.
    pgm_cfg.sensId=TMR1L^TMR1H;
    //Esegue la scrittura della NVM.
    PGMWriteNvm();
    //Attiva il timer di programmazione.
    tmrPgm=TICK_TMRPGM;
}

void PGMSelLong(void)
{
    //Modifica il valore del men� per segnala che il senspre non � programmato.
    pgm_cfg.menu=0;
    //Esegue la scrittura della NVM.
    PGMWriteNvm();
    //Esegue alcuni lampeggi (sospensivi) per segnalare che la procedura � andata a buon fine.
    LEDNormalFlash(3);
}
