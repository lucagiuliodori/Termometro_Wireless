#include "DLY.h"
#include "OUT.h"
#include "DPY.h"

typedef enum _DPY_RS
{
    RS_ISTRUCTION,
    RS_DATA
}te_DPY_RS;

typedef enum _DPY_BF
{
    BF_NOTBUSY,
    BF_BUSY
}te_DPY_BF;

static void DPYInitDisplay(void);
static void DPYSendNibble(te_DPY_RS,unsigned char);
static void DPYSendByte(te_DPY_RS,unsigned char);

void DPYInit(unsigned char sta)
{
   switch(sta)
    {
        case 0:
            //Inizializzazione hardware.
            break;
        case 1:
            //Inizializzazione variabili.
            break;
        case 2:
            //Inizializzazione applicazione.
            DPYInitDisplay();
            break;
    }
}

static void DPYInitDisplay(void)
{
    unsigned char tmp;

    DLYDelay_ms(15);
    OUTSetRSOff();
    OUTSetRWOff();
    DPYSendNibble(RS_ISTRUCTION,0b00000011);

    DLYDelay_ms(5);
    DPYSendNibble(RS_ISTRUCTION,0b00000011);

    DLYDelay_us(100);
    DPYSendNibble(RS_ISTRUCTION,0b00000011);

    DPYSendNibble(RS_ISTRUCTION,0b00000010);
    DPYSendNibble(RS_ISTRUCTION,0b00000010);

    tmp=0;
    #ifdef DPY_N_ONE
    //Nessuna istruzione.
    #endif
    #ifdef DPY_N_TWO
    tmp|=0b00001000;
    #endif
    #ifdef DPY_F_5X8
    //Nessuna istruzione.
    #endif
    #ifdef DPY_F_5X10
    tmp|=0b00000100;
    #endif
    DPYSendNibble(RS_ISTRUCTION,tmp);

    DPYSendNibble(RS_ISTRUCTION,0b00000000);
    DPYSendNibble(RS_ISTRUCTION,0b00001000);
    DPYSendNibble(RS_ISTRUCTION,0b00000000);
    DPYSendNibble(RS_ISTRUCTION,0b00000001);
    DPYSendNibble(RS_ISTRUCTION,0b00000000);

    tmp=0;
    #ifdef DPY_ID_DEC
    //Nessuna istruzione.
    #endif
    #ifdef DPY_ID_INC
    tmp|=0b00000010;
    #endif
    #ifdef DPY_S_NO
    //Nessuna istruzione.
    #endif
    #ifdef DPY_S_YES
    tmp|=0b00000001;
    #endif
    DPYSendNibble(RS_ISTRUCTION,tmp);
}

static void DPYWriteNibble(te_DPY_RS rs,unsigned char val)
{
    //N.B.: vengono considerati solo i 4 bit meno significativi.

    //Settaggio la linea E.
    OUTSetEOff();

    //Settaggio la linea RS.
    if(rs==RS_ISTRUCTION)
        OUTSetRSOff();
    else if(rs==RS_DATA)
        OUTSetRSOn();

    //Settaggio la linea RW.
    OUTSetRWOff();

    //Attesa.
    DLYDelay_us(1);

    //Settaggio la linea E.
    OUTSetEOn();
    //Attesa.
    DLYDelay_us(1);
    //Settaggio linee dati (MSB).
    OUTSetHTCDataNibble(val&0b00001111);
    //Attesa.
    DLYDelay_us(1);
    //Settaggio la linea E.
    OUTSetEOff();
    //Attesa.
    DLYDelay_us(1);
}

static void DPYWriteByte(te_DPY_RS rs,unsigned char val)
{
    //Settaggio la linea E.
    OUTSetEOff();

    //Settaggio la linea RS.
    if(rs==RS_ISTRUCTION)
        OUTSetRSOff();
    else if(rs==RS_DATA)
        OUTSetRSOn();

    //Settaggio la linea RW.
    OUTSetRWOff();

    //Attesa.
    DLYDelay_us(1);

    //Settaggio la linea E.
    OUTSetEOn();
    //Attesa.
    DLYDelay_us(1);
    //Settaggio linee dati (MSB).
    OUTSetHTCDataNibble((val>>4)&0b00001111);
    //Attesa.
    DLYDelay_us(1);
    //Settaggio la linea E.
    OUTSetEOff();
    //Attesa.
    DLYDelay_us(1);
    //Settaggio la linea E.
    OUTSetEOn();
    //Attesa.
    DLYDelay_us(1);
    //Settaggio linee dati (LSB).
    OUTSetHTCDataNibble(val&0b00001111);
    //Attesa.
    DLYDelay_us(1);
    //Settaggio la linea E.
    OUTSetEOff();
    //Attesa.
    DLYDelay_us(1);
}

static te_DPY_BF DPYReadByte(te_DPY_RS rs,unsigned char *val)
{
    //Settaggio la linea E.
    OUTSetEOff();

    //Settaggio la linea RS.
    if(rs==RS_ISTRUCTION)
        OUTSetRSOff();
    else if(rs==RS_DATA)
        OUTSetRSOn();

    //Settaggio la linea RW.
    OUTSetRWOn();

    //Attesa.
    DLYDelay_us(1);

    //Settaggio la linea E.
    OUTSetEOn();
    //Settaggio linee dati (MSB).
    //TODO
    //Attesa.
    DLYDelay_us(1);
    //Settaggio la linea E.
    OUTSetEOff();
    //Attesa.
    DLYDelay_us(1);
    //Settaggio la linea E.
    OUTSetEOn();
    //Settaggio linee dati (LSB).
    //TODO
    //Attesa.
    DLYDelay_us(1);
    //Settaggio la linea E.
    OUTSetEOff();
    //Attesa.
    DLYDelay_us(1);

    //Settaggio busy-flag.
    //TODO
}

void DPYSendStr(unsigned char* data,unsigned char len,unsigned char rowNum)
{

}
