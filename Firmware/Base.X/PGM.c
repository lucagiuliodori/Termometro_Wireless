#include <xc.h>
#include "Base.h"
#include "DIN.h"
#include "NVM.h"
#include "OUT.h"
#include "DPY.h"
#include "DLY.h"
#include "RTX.h"
#include "PGM.h"

#define VAL_EEP_CHECK1      0xC5	//Valore per l'indirizzo di controllo eeprom.
#define VAL_EEP_CHECK2      0x5C	//Valore per l'indirizzo di controllo eeprom.
#define RST_EEP_MENU        0x00	//Valore di reset per il men�.

#define TICK_TMRPGM         1000        //Tick per il timer di fine programmazione.

#define mPgmInit() T1CON=0b00000001     //Attiva il timer 1 per ricavare il valore casuale per l'id del sense.

unsigned short tmrPgm;

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
            PGMPgmOff();
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
    pgm_flg.evPgmOff=1;
    //Cambia la visualizzazione del display.
    RTXUpdateDisplay(UPD_DPY_RST);
}

void PGMSelDown(void)
{
    if(!(pgm_cfg.menu))
    {
        if(pgm_flg.pgmonoff)
            PGMPgmOff();        //Arresta la programmazione.
        else
        {
            //Segnala che � stata richiesta la programmazione dall'utente.
            pgm_flg.pgmonoff=1;
            pgm_flg.evPgmOn=1;
            //Attiva il timer di programmazione.
            tmrPgm=TICK_TMRPGM;
            //Cambia la visualizzazione del display.
            RTXUpdateDisplay(UPD_DPY_PGM);
        }
    }
}

void PGMSelLong(void)
{
    //Genera il numero casuale.
    pgm_cfg.sensId=TMR1L^TMR1H;
    //Modifica il valore del men� per segnala che il sense � programmato.
    pgm_cfg.menu=RST_EEP_MENU;
    //Esegue la scrittura della NVM.
    PGMWriteNvm();
    //Aggiorna il display.
    RTXUpdateDisplay(UPD_DPY_PGMRST);
    //Ritardo di visualizzazione.
    DLYDelay_ms(2000);
    //Cambia la visualizzazione del display.
    RTXUpdateDisplay(UPD_DPY_RST);
    //Arresta la programmazione.
    PGMPgmOff();
    RTXPgmOff();
}
