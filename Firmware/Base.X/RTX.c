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
#include "DLY.h"
#include "smartrf_CC1101.h"
#include "RTX.h"

//Definizioni.
#define ADDR_CC1_FIFO		0x3F	//Indirizzo del CC1101 per la lettura del FIFO.
#define ADDR_CC1_STA		0x35	//Indirizzo del CC1101 per la lettura del byte di stato.
#define ADDR_CC1_BYTX		0x3A	//Indirizzo del CC1101 per la lettura del byte contenente il nuemro di byte in trasmissione e presenti nel FIFO.
#define ADDR_CC1_BYRX		0x3B	//Indirizzo del CC1101 per la lettura del byte contenente il nuemro di byte ricevuti e presenti nel FIFO.

#define TICK_TMRRCVPKT	10                         //Base tempi 10ms, 1s di timeout.
#define MAX_SNDDATA     5                           //Numero massimo di invii in caso di non ricezione della risposta.
#define PKT_LEN         SMARTRF_SETTING_PKTLEN      //Numero di byte per pacchetto.
#define TICK_VISBATT    500                         //Timeout per la visualizzazione della batteria.

#define MAX_DECNUM 2    //Numero massimo di decimali.

#define MAX_TMRCALIBRATION 1000          //Massimo valore per il timer di calibrazione.

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

static char sta,numRxByte,numTxByte;
static unsigned char pktRx[PKT_LEN],pktTx[PKT_LEN],tmrRcvPkt;
static unsigned short tmrCalibration,tmrVisBatt;

static void RTXSndPkt(unsigned char *,unsigned char);
static void RTXRcvPkt(unsigned char *,unsigned char);
static void RTXSetSensSta(unsigned char,unsigned char);
static void RTXSendPgm(void);
static void RTXSendRData(void);
static void RTXSup(void);
static unsigned char RTXVisDec(float,char *,unsigned char);

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
            tmrVisBatt=0;
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
    //Chiama la funzione del supervisore.
    RTXSup();
    //Timer per la calibrazione.
    tmrCalibration++;
    if(tmrCalibration>MAX_TMRCALIBRATION)
    {
        tmrCalibration=0;
        CC1Strobe(SIDLE);
        CC1Strobe(SCAL);
        CC1Strobe(SFRX);
    }
		
    //Timer di attesa per la ricezione della risposta.
    if(tmrRcvPkt)
    {
        tmrRcvPkt--;
        if(!(tmrRcvPkt))
            RTXSendPgm();  //Se il timeout è scaduto, trasmette di nuovo il pacchetto di programmazione.
    }

    //Timer visualizzazione batteria.
    if(tmrVisBatt)
    {
        tmrVisBatt--;
        if(!(tmrVisBatt))
            RTXUpdateDisplay(UPD_DPY_TEMP);
    }
}

void RTXTask(void)
{
    //Controlla se è stato ricevuto un pacchetto e quindi esegue la lettura dello stesso.
    if(!(INGetGDO2()))
        RTXReadPkt();
}

void RTXUpdateDisplay(te_RTX_UPD_DPY val)
{
    char charIdSens[4],strTitle[DPY_COL_NUM],strText[DPY_COL_NUM];
    unsigned char i,maxDigit,curPos;

    //Inizializza i vettori.
    for(i=0;i<sizeof(strTitle)/sizeof(char);i++)
        strTitle[i]=' ';
    for(i=0;i<sizeof(strText)/sizeof(char);i++)
        strText[i]=' ';

    if(val==UPD_DPY_PGM || val==UPD_DPY_PGMDONE || val==UPD_DPY_PGMRST)
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
        if(val==UPD_DPY_PGMRST)
        {
            strText[5]='R';
            strText[6]='E';
            strText[7]='S';
            strText[8]='E';
            strText[9]='T';
        }
        else
        {
            strText[5]='I';
            strText[6]='D';
            strText[7]=':';
            if(val==UPD_DPY_PGM)
            {
                strText[8]='?';
                strText[9]='?';
                strText[10]='?';
            }
            else
            {
                itoa(charIdSens,pgm_cfg.sensId,10);
                for(i=0;i<4;i++)
                {
                    if(charIdSens[i]==0x00)
                        break;
                    strText[8+i]=charIdSens[i];
                }
            }
        }
    }
    else if(val==UPD_DPY_BATT)
    {
        strTitle[3]='B';
        strTitle[4]='a';
        strTitle[5]='t';
        strTitle[6]='t';
        strTitle[7]='e';
        strTitle[8]='r';
        strTitle[9]='i';
        strTitle[10]='a';
        strTitle[11]=':';

        //Calcola il massimo numero di cifre scrivibili.
        maxDigit=(DPY_COL_NUM-1);    //I caratteri in meno è riservato per "V".
        curPos=RTXVisDec(batt,strText,maxDigit);
        //Setta gli altri caratteri.
        strText[curPos]='V';
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

        if(val==UPD_DPY_TEMP)
        {
            //Calcola il massimo numero di cifre scrivibili.
            maxDigit=(DPY_COL_NUM-2);    //I 2 caratteri sono riservati per "°C".
            if(batt<2.5)
                maxDigit=(DPY_COL_NUM-3);    //I 3 caratteri sono riservati per "°C B".

            curPos=RTXVisDec(temp,strText,maxDigit);

            //Setta gli altri caratteri.
            strText[curPos]=0b11011111;     //Simbolo "°"
            strText[curPos+1]='C';
            if(batt<2.5)
                strText[sizeof(strText)/sizeof(char)-1]=0x00;
        }
        else if(val==UPD_DPY_RST)
        {
            strText[5]='?';
            strText[6]='?';
            strText[7]='.';
            strText[8]='?';
            strText[9]='?';
            strText[10]=0b11011111;     //Simbolo "°"
            strText[11]='C';
        }
    }
    DPYSendStr(0,0,strTitle,sizeof(strTitle)/sizeof(char));
    DPYSendStr(1,0,strText,sizeof(strText)/sizeof(char));
}

static unsigned char RTXVisDec(float val,char *buff,unsigned char maxDigit)
{
    char *p;
    unsigned char strTmp[DPY_COL_NUM],isDec,posDec,i,j,offset;
    int status;

    p=ftoa(val,(&(status)));
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
        //Controlla se è arrivato al carattere di fine stringa.
        if((*(p+i))==0x00)
            break;
        //Copia i caratteri.
        strTmp[i]=(*(p+i));
    }
    
    //Controlla il massimo numero di cifre scrivibili.
    if(i>=maxDigit)
        i=maxDigit-1;

    //Copia i valori in modo che siano centrati sul display.
    offset=(maxDigit-i)/2;
    for(j=0;j<i;j++)
        (*(buff+j+offset))=strTmp[j];

    return (j+offset);  //Ritorna la posizione del cursore.
}

static void RTXSup(void)
{
    //Legge lo stato e attiva l'operazione appropriata.
    CC1Read(ADDR_CC1_STA,(&(sta)),1);

    //Operazione da eseguire in seguito alla lettura dello stato.
    switch(sta)
    {
        case IDLE:
            CC1Strobe(SRX);
            break;
        case RXFIFO_OVERFLOW:
            CC1Strobe(SFRX);
            CC1Strobe(SRX);
            break;
        case TXFIFO_UNDERFLOW:
            CC1Strobe(SFTX);
            CC1Strobe(SRX);
            break;
    }
}

void RTXSelDown(void)
{
    if(pgm_cfg.menu)
    {
        if(tmrVisBatt)
        {
            //Visualizza la temperatura.
            RTXUpdateDisplay(UPD_DPY_TEMP);
            //Arresta il timer.
            tmrVisBatt=0;
        }
        else
        {
            //Visualizza la tensione della batteria.
            RTXUpdateDisplay(UPD_DPY_BATT);
            //Avvia il timer.
            tmrVisBatt=TICK_VISBATT;
        }
    }
}

static void RTXSndPkt(unsigned char *src,unsigned char len)
{
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
    tu_RTX_DATA valTemp,valBatt;
    
    //Legge il numero di byte presenti nel FIFO.
    CC1Read(ADDR_CC1_BYRX,(&(numRxByte)),1);
    while(numRxByte>=PKT_LEN)
    {
        CLRWDT();
        //Legge un pacchetto.
        RTXRcvPkt(pktRx,PKT_LEN);
        //Controlla se è attiva la programmazione.
        if(rtx_flg.pgmonoff)
        {
            if((pktRx[0]==pgm_cfg.baseId))  //Controlla se l'id della base corrisponde.
            {
                //Controlla se il pacchetto ricevuto è di programmazione.
                if(pktRx[2]==rPGM)
                {
                    //Memorizza il nuovo ID del sensore.
                    pgm_cfg.sensId=pktRx[1];
                    //Segnala che è stata eseguita la programmazione.
                    pgm_cfg.menu=1;
                    PGMWriteNvm();
                    //Segnala che la programmazione è stata eseguita.
                    rtx_flg.pgmDone=1;
                    //Arresta l'invio dei pacchetti di programmazione (è sufficiente arresta il time-out).
                    tmrRcvPkt=0;
                    //Cambia la visualizzazione del display.
                    RTXUpdateDisplay(UPD_DPY_PGMDONE);
                    //Ritardo di visualizzazione.
                    DLYDelay_ms(2000);
                    //Cambia la visualizzazione del display.
                    RTXUpdateDisplay(UPD_DPY_RST);
                    //Chiama la funzione per l'arresto della programmazione.
                    RTXPgmOff();
                }
            }
        }
        else
        {
            if((pktRx[0]==pgm_cfg.baseId) && (pktRx[1]==pgm_cfg.sensId))
            {
                //Controlla se il pacchetto ricevuto è di risposta al dato.
                if(pktRx[2]==DAT)
                {
                    //Memorizza la temperatura.
                    valTemp.val_byte[0]=pktRx[3];
                    valTemp.val_byte[1]=pktRx[4];
                    valTemp.val_byte[2]=pktRx[5];
                    temp=valTemp.val;
                    //Memorizza la batteria.
                    valBatt.val_byte[0]=pktRx[6];
                    valBatt.val_byte[1]=pktRx[7];
                    valBatt.val_byte[2]=pktRx[8];
                    batt=valBatt.val;
                    //Invia la risposta ai dati ricevuti.
                    RTXSendRData();
                    //Aggiorna il valore sul display.
                    if(tmrVisBatt)
                        RTXUpdateDisplay(UPD_DPY_BATT);
                    else
                        RTXUpdateDisplay(UPD_DPY_TEMP);
                }
            }
        }
        //Legge il numero di byte rimanenti.
        CC1Read(ADDR_CC1_BYRX,(&(numRxByte)),1);
        //Esegue un controllo sullo stato.
        RTXSup();
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
    pktTx[0]=0;//pgm_cfg.sensId;
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
}

void RTXPgmOff(void)
{
    //Setta il flag.
    rtx_flg.pgmonoff=0;
    //Arresta l'invio dei pacchetti di programmazione (è sufficiente arresta il time-out).
    tmrRcvPkt=0;
    //Arresta il timer per la visualizzazione della batteria (in caso di attivazione dopo reset).
    tmrVisBatt=0;
}
