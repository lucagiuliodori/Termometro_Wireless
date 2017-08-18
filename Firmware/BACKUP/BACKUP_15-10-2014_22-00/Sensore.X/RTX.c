#include <xc.h>
#include "Sensore.h"
#include "CC1.h"
#include "PGM.h"
#include "NVM.h"
#include "ADC.h"
#include "OUT.h"
#include "IN.h"
#include "RTX.h"
#include "LED.h"

//Definizioni.
#define ADDR_EEP_BASECODE	0x11	//Indirizzo eeprom codice base.
#define ADDR_EEP_SENSCODE	0x12	//Indirizzo eeprom codice sensi.
#define ADDR_CC1_CHN		0x0A	//Indirizzo del CC1101 per la selezione del canale.
#define ADDR_CC1_FIFO		0x3F	//Indirizzo del CC1101 per la lettura del FIFO.
#define ADDR_CC1_STA		0x35	//Indirizzo del CC1101 per la lettura del byte di stato.
#define ADDR_CC1_BYTX		0x3A	//Indirizzo del CC1101 per la lettura del byte contenente il nuemro di byte in trasmissione e presenti nel FIFO.
#define ADDR_CC1_BYRX		0x3B	//Indirizzo del CC1101 per la lettura del byte contenente il nuemro di byte ricevuti e presenti nel FIFO.

#define TICK_TMRRCVPKT	100		//Base tempi 10ms, 1s di timeout.
#define PKT_LEN		5		//Numero di byte per pacchetto.
#define MAX_SNDDATA     5               //Numero massimo di invii in caso di non ricezione della risposta.
#define TICK_TMRPGM     100             //Tick per il timer di fine programmazione.

typedef enum _RTX_OP
{
    DAT=0x00,
    PGM=0x01,

    rDAT=0x80,
    rPGM=0x81
}te_RTX_OP;

typedef union _RTX_FLG
{
    unsigned char val;
    struct
    {
        unsigned char pgmRq:1;          //Flag per segnalare che è stata richiesta la programmazione dall'utente.
    };
}tu_RTX_FLG;

tu_RTX_FLG rtx_flg;
static unsigned char pktRx[PKT_LEN],pktTx[PKT_LEN],cntSndData,tmrPgm,tmrRcvPkt;

static void RTXSndPkt(unsigned char *,unsigned char);
static void RTXRcvPkt(unsigned char *,unsigned char);
static void RTXSetSensSta(unsigned char,unsigned char);
static void RTXSendRPgm(void);
static void RTXSendData(void);

void RTXInit(unsigned char state)
{
    switch(state)
    {
        case 0:
            //Inizializzazione hardware.
            break;
        case 1:
            //Inizializzazione variabili.
            tmrRcvPkt=0;
            tmrPgm=0;
            cntSndData=0;
            rtx_ev.val=0;
            rtx_flg.val=0;
            break;
        case 2:
            //Avvia la calibrazione e la ricezione.
            CC1Strobe(SIDLE);
            CC1Strobe(SCAL);
            CC1Strobe(SRX);
            RTXSendData();
            break;
	}
}

void RTXTick(void)
{
    //Timer di attesa per la ricezione della risposta.
    if(tmrRcvPkt)
    {
        tmrRcvPkt--;
        if(!(tmrRcvPkt))
            RTXSendData();  //Se il timeout è scaduto, trasmette di nuovo l'informazione.
    }

    //Timer di termine programmazione.
    if(tmrPgm)
    {
        tmrPgm--;
        if(!(tmrPgm))
        {
            //Cancella il flag di richiesta di programmazione.
            rtx_flg.pgmRq=0;
            //Azzera il contatore per il numero di invii.
            cntSndData=0;
            //Avvia l'invio dei dati.
            RTXSendData();
        }
    }
}

void RTXTask(void)
{
    //Controlla se è stato ricevuto un pacchetto e quindi esegue la lettura dello stesso.
    if(!(INGetGDO2()))
        RTXReadPkt();
}

static void RTXSndPkt(unsigned char *src,unsigned char len)
{
    unsigned char sta,numRxByte,numTxByte;

    //Legge lo stato e attende che il CC1101 è in idle o in ricezione.
    do
    {
        CLRWDT();
        CC1Read(ADDR_CC1_STA,(&(sta)),1);
    }
    while(!(((sta&0b00011111)==0x0D) || ((sta&0b00011111)==0x01)));
    //Setta lo stato di idle.
    CC1Strobe(SIDLE);
    CC1Read(ADDR_CC1_BYRX,(&(numRxByte)),1);
    CC1Read(ADDR_CC1_BYTX,(&(numTxByte)),1);
    //Esegue il flush del TX.
    CC1Strobe(SFTX);
    CC1Read(ADDR_CC1_BYTX,(&(numTxByte)),1);
    //Carica il buffer di trasmissione.
    CC1Write(ADDR_CC1_FIFO,src,len);
    //Avvia la trasmissione (terminata la trasmissione il CC1101 passa automaticamente allo stato di ricezione).
    CC1Strobe(STX);
}

static void RTXRcvPkt(unsigned char *dst,unsigned char len)
{
    //Legge il buffer di ricezione.
    CC1Read(ADDR_CC1_FIFO,dst,len);
}

void RTXReadPkt(void)
{
    unsigned char numRxByte;

    //Legge il numero di byte presenti nel FIFO.
    CC1Read(ADDR_CC1_BYRX,(&(numRxByte)),1);
    while(numRxByte>=PKT_LEN)
    {
        CLRWDT();
        //Legge un pacchetto.
        RTXRcvPkt(pktRx,PKT_LEN);
        //Controlla se è attiva la richiesta di programmazione da parte dell'utente.
        if(rtx_flg.pgmRq)
        {
            if(pktRx[0]==0x00)      //Controlla se la comunicazione è in broadcast.
            {
                //Controlla se il pacchetto ricevuto è di programmazione.
                if(pktRx[2]==PGM)
                {
                    //Memorizza il nuovo ID della base.
                    pgm_cfg.baseId=pktRx[1];
                    PGMWriteNvm();
                    //Invia la risposta.
                    RTXSendRPgm();
                    //Cancella la richiesta di programmazione.
                    rtx_flg.pgmRq=0;
                    //Arresta il timer per la ricezione della risposta.
                    tmrRcvPkt=0;
                    //Azzera il contatore per il numero di invii.
                    cntSndData=0;
                    //Avvia l'invio dei dati.
                    RTXSendData();
                    //Esegue tre lampeggi veloci per segnalare che la programmazione è stata eseguita con successo.
                    LEDNormalFlash(3);
                }
            }
        }
        else
        {
            if((pktRx[0]==pgm_cfg.sensId) && (pktRx[1]==pgm_cfg.baseId))
            {
                //Controlla se il pacchetto ricevuto è di risposta al dato.
                if(pktRx[2]==rDAT)
                {
                    //Setta il CC1101 in idle.
                    CC1Strobe(SIDLE);
                    //Arresta il timer per la ricezione della risposta.
                    tmrRcvPkt=0;
                    //Azzera il contatore per il numero di invii.
                    cntSndData=0;
                    //Segnala che il dispositivo deve andare in sleep.
                    rtx_ev.sleep=1;
                }
           }
        }
        //Legge il numero di byte rimanenti.
        CC1Read(ADDR_CC1_BYRX,(&(numRxByte)),1);
    }
}

static void RTXSendData(void)
{
    //Controlla il contatore del numero di invii.
    if(cntSndData<MAX_SNDDATA)
    {
        //Crea il pacchetto da inviare.
        pktTx[0]=pgm_cfg.baseId;
        pktTx[1]=pgm_cfg.sensId;
        pktTx[2]=DAT;
        pktTx[3]=ADCGetTemp();
        pktTx[4]=ADCGetBatt();

        //Invia il pacchetto.
        RTXSndPkt(pktTx,PKT_LEN);

        //Attiva il timer di attesa per la ricezione della risposta.
        tmrRcvPkt=TICK_TMRRCVPKT;

        //Incrementa il contatore del numero di invii.
        cntSndData++;

        //Esegue una breve accensione del led.
        LEDFastFlash(1);
    }
    else
        rtx_ev.sleep=1;        //Segnala che il dispositivo deve andare in sleep.
}

static void RTXSendRPgm(void)
{
    //Crea il pacchetto da inviare.
    pktTx[0]=pgm_cfg.baseId;
    pktTx[1]=pgm_cfg.sensId;
    pktTx[2]=rPGM;
    pktTx[3]=0x00;
    pktTx[4]=0x00;

    //Invia il pacchetto.
    RTXSndPkt(pktTx,PKT_LEN);
}

void RTXSelDown(void)
{
    //Arresta il timer per la ricezione della risposta.
    tmrRcvPkt=0;
    //Azzera il contatore per il numero di invii.
    cntSndData=0;
    //Cancella il flag di sleep.
    rtx_ev.sleep=0;
    //Segnala che è stata richiesta la programmazione dall'utente.
    rtx_flg.pgmRq=1;
    //Attiva il led per segnalare che la programmazione è attiva.
    OUTSetLEDOn();
    //Attiva il timer di programmazione.
    tmrPgm=TICK_TMRPGM;
}
