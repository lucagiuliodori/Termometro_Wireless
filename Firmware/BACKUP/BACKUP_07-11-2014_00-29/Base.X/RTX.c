#include <xc.h>
#include <stdlib.h>
#include <stdio.h>
#include "Base.h"
#include "CC1.h"
#include "PGM.h"
#include "NVM.h"
#include "OUT.h"
#include "IN.h"
#include "DPY.h"
#include "smartrf_CC1101.h"
#include "RTX.h"
#include "DLY.h"

//Definizioni.
#define ADDR_CC1_FIFO		0x3F	//Indirizzo del CC1101 per la lettura del FIFO.
#define ADDR_CC1_STA		0x35	//Indirizzo del CC1101 per la lettura del byte di stato.
#define ADDR_CC1_BYTX		0x3A	//Indirizzo del CC1101 per la lettura del byte contenente il nuemro di byte in trasmissione e presenti nel FIFO.
#define ADDR_CC1_BYRX		0x3B	//Indirizzo del CC1101 per la lettura del byte contenente il nuemro di byte ricevuti e presenti nel FIFO.

#define TICK_TMRRCVPKT	100                         //Base tempi 10ms, 1s di timeout.
#define MAX_SNDDATA     5                           //Numero massimo di invii in caso di non ricezione della risposta.
#define PKT_LEN         9//SMARTRF_SETTING_PKTLEN      //Numero di byte per pacchetto.

#define MAX_DECNUM 2    //Numero massimo di decimali.

#define MAX_TMRCALIBRATION 1000          //Massimo valore per il timer di calibrazione.

typedef enum _RTX_OP
{
    DAT=0x00,
    PGM=0x01,

    rDAT=0x80,
    rPGM=0x81
}te_RTX_OP;

typedef enum _RTX_UPD_DPY
{
    UPD_DPY_RST,
    UPD_DPY_TEMP,
    UPD_DPY_PGM,
    UPD_DPY_PGMDONE
}te_RTX_UPD_DPY;


static char strTitle[DPY_COL_NUM],strTemp[DPY_COL_NUM];
static unsigned char pktRx[PKT_LEN],pktTx[PKT_LEN],tmrRcvPkt;
static unsigned short tmrCalibration;

static void RTXSndPkt(unsigned char *,unsigned char);
static void RTXRcvPkt(unsigned char *,unsigned char);
static void RTXSetSensSta(unsigned char,unsigned char);
static void RTXSendPgm(void);
static void RTXSendRData(void);
static void RTXUpdateDisplay(te_RTX_UPD_DPY);

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
            rtx_flg.val=0;
            tmrCalibration=0;
            break;
        case 2:
            //Avvia la calibrazione e la ricezione.
            CC1Strobe(SIDLE);
            CC1Strobe(SCAL);
            CC1Strobe(SRX);
            //Scrittua display.
            RTXUpdateDisplay(UPD_DPY_RST);
            break;
	}
}

void RTXTick(void)
{
    //Timer per la calibrazione.
    tmrCalibration++;
    if(tmrCalibration>MAX_TMRCALIBRATION)
    {
        tmrCalibration=0;
        CC1Strobe(SIDLE);
        CC1Strobe(SCAL);
        CC1Strobe(SFRX);
        CC1Strobe(SRX);
    }
		
    //Timer di attesa per la ricezione della risposta.
    if(tmrRcvPkt)
    {
        tmrRcvPkt--;
        if(!(tmrRcvPkt))
            RTXSendPgm();  //Se il timeout � scaduto, trasmette di nuovo il pacchetto di programmazione.
    }
}

void RTXTask(void)
{
    //Controlla se � stato ricevuto un pacchetto e quindi esegue la lettura dello stesso.
    if(!(INGetGDO2()))
        RTXReadPkt();
}

static void RTXUpdateDisplay(te_RTX_UPD_DPY val)
{
    char *p,strTmp[DPY_COL_NUM];
    unsigned char i,j,isDec,posDec,maxDigit,offset;
    int status;

    //Inizializza i vettori.
    for(i=0;i<sizeof(strTitle)/sizeof(char);i++)
        strTitle[i]=' ';
    for(i=0;i<sizeof(strTemp)/sizeof(char);i++)
        strTemp[i]=' ';

    if(val==UPD_DPY_PGM || val==UPD_DPY_PGMDONE)
    {
        strTitle[1]='P';
        strTitle[2]='r';
        strTitle[3]='o';
        strTitle[4]='g';
        strTitle[5]='r';
        strTitle[6]='a';
        strTitle[7]='m';
        strTitle[8]='m';
        strTitle[9]='a';
        strTitle[10]='z';
        strTitle[11]='i';
        strTitle[12]='o';
        strTitle[13]='n';
        strTitle[14]='e';
        DPYSendStr(0,(DPY_COL_NUM-sizeof(strTitle))/2,strTitle,sizeof(strTitle)/sizeof(char));
        strTemp[5]='I';
        strTemp[6]='D';
        strTemp[7]=':';
        if(val==UPD_DPY_PGM)
        {
            strTemp[9]='?';
            strTemp[10]='?';
            strTemp[11]='?';
        }
        else
        {
            itoa(p,pgm_cfg.sensId,10);
            strTemp[9]=(*(p));
            strTemp[10]=(*(p+1));
            strTemp[11]=(*(p+2));
        }
        DPYSendStr(1,0,strTemp,sizeof(strTemp)/sizeof(char));
    }
    else
    {
        strTitle[2]='T';
        strTitle[3]='e';
        strTitle[4]='m';
        strTitle[5]='p';
        strTitle[6]='e';
        strTitle[7]='r';
        strTitle[8]='a';
        strTitle[9]='t';
        strTitle[10]='u';
        strTitle[11]='r';
        strTitle[12]='a';
        strTitle[13]=':';
        DPYSendStr(0,(DPY_COL_NUM-sizeof(strTitle))/2,strTitle,sizeof(strTitle)/sizeof(char));
        if(val==UPD_DPY_TEMP)
        {
            p=ftoa(temp,(&(status)));
            //Ricava il vettore e la sua lunghezza.
            isDec=0;
            for(i=0;i<sizeof(strTmp)/sizeof(char);i++)
            {
                //Controlla la posizione della virgola.
                if((*(p+i))=='.')
                {
                    isDec=1;
                    posDec=i;
                }
                //Calcola il numero di cifre decimali.
                if(isDec)
                {
                    if((i-posDec)>MAX_DECNUM)
                        break;
                }
                //Controlla se � arrivato al carattere di fine stringa.
                if((*(p+i))==0x00)
                    break;
                //Copia i caratteri.
                strTmp[i]=(*(p+i));
            }

            //Calcola il massimo numero di cifre scrivibili.
            maxDigit=(DPY_COL_NUM-2);    //I 2 caratteri sono riservati per "�C".
            if(batt<2.5)
                maxDigit=(DPY_COL_NUM-3);    //I 3 caratteri sono riservati per "�C B".

            //Controlla il massimo numero di cifre scrivibili.
            if(i>=maxDigit)
                i=maxDigit-1;

            //Copia i valori in modo che siano centrati sul display.
            offset=(maxDigit-i)/2;
            for(j=0;j<i;j++)
                strTemp[j+offset]=strTmp[j];

            //Setta gli altri caratteri.
            strTemp[j+offset]=0b11011111;     //Simbolo "�"
            strTemp[j+offset+1]='C';
            if(batt<2.5)
                strTemp[sizeof(strTemp)/sizeof(char)-1]=0x00;
        }
        else if(val==UPD_DPY_RST)
        {
            strTemp[5]='?';
            strTemp[6]='?';
            strTemp[7]='.';
            strTemp[8]='?';
            strTemp[9]='?';
            strTemp[10]=0b11011111;     //Simbolo "�"
            strTemp[11]='C';
        }
        DPYSendStr(1,0,strTemp,sizeof(strTemp)/sizeof(char));
    }
}

static void RTXSndPkt(unsigned char *src,unsigned char len)
{
    unsigned char sta,numRxByte,numTxByte;

    //Legge lo stato e attende che il CC1101 � in idle o in ricezione.
    while(1)
    {
        CLRWDT();
        CC1Read(ADDR_CC1_STA,(&(sta)),1);
        sta&=0b00011111;
        if(sta==0x0D)
            break;
        if(sta==0x01)
            break;
    }
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
    unsigned char numRxByte,tmp,sta;

    //Legge il numero di byte presenti nel FIFO.
    CC1Read(0x35,(&(sta)),1);
    CC1Read(0x38,(&(tmp)),1);
    CC1Read(ADDR_CC1_BYRX,(&(numRxByte)),1);
    if(tmp&0b10000000)
        NOP();
    while(numRxByte>=PKT_LEN)
    {
        CLRWDT();
        //Legge un pacchetto.
        RTXRcvPkt(pktRx,PKT_LEN);
        //Controlla se � attiva la programmazione.
        if(rtx_flg.pgmonoff)
        {
            if((pktRx[0]==pgm_cfg.baseId))  //Controlla se l'id della base corrisponde.
            {
                //Controlla se il pacchetto ricevuto � di programmazione.
                if(pktRx[2]==rPGM)
                {
                    //Memorizza il nuovo ID del sensore.
                    pgm_cfg.sensId=pktRx[1];
                    PGMWriteNvm();
                    //Segnala che la programmazione � stata eseguita.
                    rtx_flg.pgmDone=1;
                    //Arresta l'invio dei pacchetti di programmazione (� sufficiente arresta il time-out).
                    tmrRcvPkt=0;
                    //Cambia la visualizzazione del display.
                    RTXUpdateDisplay(UPD_DPY_PGMDONE);
                    //Ritardo di visualizzazione.
                    DLYDelay_ms(2000);
                    //Chiama la funzione per l'arresto della programmazione.
                    RTXPgmOff();
                }
            }
        }
        else
        {
            if((pktRx[0]==pgm_cfg.baseId) && (pktRx[1]==pgm_cfg.sensId))
            {
                //Controlla se il pacchetto ricevuto � di risposta al dato.
                if(pktRx[2]==DAT)
                {
                    //Memorizza la temperatura.
                    temp=pktRx[3];
                    //Memorizza la batteria.
                    batt=pktRx[4];
                    //Invia la risposta ai dati ricevuti.
                    RTXSendRData();
                    //Aggiorna il valore sul display.
                    RTXUpdateDisplay(UPD_DPY_TEMP);
                }
            }
        }
        //Legge il numero di byte rimanenti.
        CC1Read(ADDR_CC1_BYRX,(&(numRxByte)),1);
    }
}

static void RTXSendRData(void)
{
    //Crea il pacchetto da inviare.
    pktTx[0]=pgm_cfg.sensId;
    pktTx[1]=pgm_cfg.baseId;
    pktTx[2]=rDAT;
    pktTx[3]=0x00;
    pktTx[4]=0x00;
    pktTx[5]=0x00;
    pktTx[6]=0x00;
    pktTx[7]=0x00;
    pktTx[8]=0x00;

    //Invia il pacchetto.
    RTXSndPkt(pktTx,PKT_LEN);
}

static void RTXSendPgm(void)
{
    //Crea il pacchetto da inviare.
    pktTx[0]=pgm_cfg.sensId;
    pktTx[1]=pgm_cfg.baseId;
    pktTx[2]=PGM;
    pktTx[3]=0x00;
    pktTx[4]=0x00;
    pktTx[5]=0x00;
    pktTx[6]=0x00;
    pktTx[7]=0x00;
    pktTx[8]=0x00;

    //Invia il pacchetto.
    RTXSndPkt(pktTx,PKT_LEN);

    //Avvia il timer per il rinvio del pacchetto.
    tmrRcvPkt=TICK_TMRRCVPKT;
}

void RTXPgmOn(void)
{
    //Setta il flag.
    rtx_flg.pgmonoff=1;
    //Avvia la trasmissione di un pacchetto di programmazione.
    RTXSendPgm();
    //Cambia la visualizzazione del display.
    RTXUpdateDisplay(UPD_DPY_PGM);
}

void RTXPgmOff(void)
{
    //Setta il flag.
    rtx_flg.pgmonoff=0;
    //Arresta l'invio dei pacchetti di programmazione (� sufficiente arresta il time-out).
    tmrRcvPkt=0;
    //Cambia la visualizzazione del display.
    RTXUpdateDisplay(UPD_DPY_RST);
}
