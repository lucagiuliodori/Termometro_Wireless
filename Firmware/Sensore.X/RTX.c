#include <xc.h>
#include "Sensore.h"
#include "smartrf_CC1101.h"
#include "CC1.h"
#include "PGM.h"
#include "NVM.h"
#include "ADC.h"
#include "OUT.h"
#include "IN.h"
#include "LED.h"
#include "DLY.h"
#include "SLP.h"
#include "RTX.h"

//Definizioni.
#define ADDR_CC1_FIFO		0x3F	//Indirizzo del CC1101 per la lettura del FIFO.
#define ADDR_CC1_STA		0x35	//Indirizzo del CC1101 per la lettura del byte di stato.
#define ADDR_CC1_BYTX		0x3A	//Indirizzo del CC1101 per la lettura del byte contenente il nuemro di byte in trasmissione e presenti nel FIFO.
#define ADDR_CC1_BYRX		0x3B	//Indirizzo del CC1101 per la lettura del byte contenente il nuemro di byte ricevuti e presenti nel FIFO.

#define TICK_TMRRCVPKT	100                         //Base tempi 10ms, 1s di timeout.
#define MAX_SNDDATA     5                           //Numero massimo di invii in caso di non ricezione della risposta.
#define PKT_LEN         SMARTRF_SETTING_PKTLEN      //Numero di byte per pacchetto.

typedef enum _RTX_OP
{
    DAT=0x00,
    PGM=0x01,

    rDAT=0x80,
    rPGM=0x81
}te_RTX_OP;

typedef union _RTX_DATA
{
    float val;
    unsigned char val_byte[3];  //un dato di tipo "float" per xc8 è di 3 byte.
}tu_RTX_DATA;

static unsigned char pktRx[PKT_LEN],pktTx[PKT_LEN],cntSndData,tmrRcvPkt;

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
            cntSndData=0;
            rtx_flg.val=0;
            break;
        case 2:
            //Avvia la calibrazione e la ricezione.
            CC1Strobe(SIDLE);
            CC1Strobe(SCAL);
            CC1Strobe(SRX);
            RTXStartSendData();
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
}

void RTXTask(void)
{
    //Controlla se è stato ricevuto un pacchetto e quindi esegue la lettura dello stesso.
    if(!(INGetGDO2()))
        RTXReadPkt();
}

void RTXAwake(void)
{
    //Risveglia il CC1101.
    OUTSetSSOff();
    DLYDelay_ms(1);
    OUTSetSSOn();
    DLYDelay_ms(1);
    
    //Resetta il CC1101 e carica i registri.
    CC1Init(2);

    //Avvia la trasmissione.
    RTXInit(2);
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
        //Controlla se è attiva la programmazione.
        if(pgm_flg.pgmonoff)
        {
            if(pktRx[0]==0x00)      //Controlla se la comunicazione è in broadcast.
            {
                //Controlla se il pacchetto ricevuto è di programmazione.
                if(pktRx[2]==PGM)
                {
                    //Memorizza il nuovo ID della base.
                    pgm_cfg.baseId=pktRx[1];
                    //setta la variabile per indicare che la memorizzazione è stata eseguita.
                    pgm_cfg.menu=1;
                    PGMWriteNvm();
                    //Invia la risposta.
                    RTXSendRPgm();
                    //Cancella la richiesta di programmazione.
                    rtx_flg.pgmDone=1;
                    //Arresta il timer per la ricezione della risposta.
                    tmrRcvPkt=0;
                    //Avvia l'invio dei dati.
                    RTXStartSendData();
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
                    flg_slp.goSleep=1;
                }
           }
        }
        //Legge il numero di byte rimanenti.
        CC1Read(ADDR_CC1_BYRX,(&(numRxByte)),1);
    }
}

void RTXStartSendData(void)
{
    //Tempo di attesa per la misurazione del sensore.
    DLYDelay_ms(100);
    //Attesa ed esecuzioni di tutte le conversioni.
    ADCConv();

    cntSndData=0;
    RTXSendData();
}

static void RTXSendData(void)
{
    tu_RTX_DATA valTemp,valBatt;

    if(pgm_flg.pgmonoff)
    {
        //Cancella il contatore per il numero di invii.
        cntSndData=0;
        //Attiva il timer di attesa per la riprova dell'invio.
        tmrRcvPkt=TICK_TMRRCVPKT;
    }
    else
    {
        //Controlla il contatore del numero di invii.
        if(cntSndData<MAX_SNDDATA)
        {
            //Controlla se almeno una conversione è stata eseguita.
            if(pgm_cfg.menu)
            {
                //Crea il pacchetto da inviare.
                pktTx[0]=pgm_cfg.baseId;
                pktTx[1]=pgm_cfg.sensId;
                pktTx[2]=DAT;
                valTemp.val=ADCGetTempCCal();
                pktTx[3]=valTemp.val_byte[0];
                pktTx[4]=valTemp.val_byte[1];
                pktTx[5]=valTemp.val_byte[2];
                valBatt.val=ADCGetBattV();
                pktTx[6]=valBatt.val_byte[0];
                pktTx[7]=valBatt.val_byte[1];
                pktTx[8]=valBatt.val_byte[2];

                //Invia il pacchetto.
                RTXSndPkt(pktTx,PKT_LEN);

                //Incrementa il contatore del numero di invii.
                cntSndData++;

                //Esegue una breve accensione del led.
                LEDFastFlash(1);
            }

            //Attiva il timer di attesa per la riprova dell'invio.
            tmrRcvPkt=TICK_TMRRCVPKT;
        }
        else
            flg_slp.goSleep=1;        //Segnala che il dispositivo deve andare in sleep.
    }
}

static void RTXSendRPgm(void)
{
    //Crea il pacchetto da inviare.
    pktTx[0]=pgm_cfg.baseId;
    pktTx[1]=pgm_cfg.sensId;
    pktTx[2]=rPGM;
    pktTx[3]=0x00;
    pktTx[4]=0x00;
    pktTx[5]=0x00;
    pktTx[6]=0x00;
    pktTx[7]=0x00;
    pktTx[8]=0x00;

    //Invia il pacchetto.
    RTXSndPkt(pktTx,PKT_LEN);
}
