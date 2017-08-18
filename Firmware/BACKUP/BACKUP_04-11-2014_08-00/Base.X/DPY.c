#include <xc.h>
#include "DLY.h"
#include "OUT.h"
#include "DPY.h"
#include "IN.h"

#define ADDR_PICBATT 0x00

typedef enum _DPY_RS
{
    RS_ISTRUCTION,
    RS_DATA
}te_DPY_RS;

typedef enum _DPY_BF
{
    BF_NOTBUSY=0,
    BF_BUSY=1
}te_DPY_BF;

unsigned char picBatt[9]=
{
    0b00001110,
    0b00011111,
    0b00010001,
    0b00010001,
    0b00010001,
    0b00010001,
    0b00010001,
    0b00011111,
};

static void DPYInitDisplay(void);
static void DPYWriteNibble(te_DPY_RS,unsigned char);
static void DPYWriteByte(te_DPY_RS,unsigned char);
static te_DPY_BF DPYReadByte(te_DPY_RS,unsigned char *);
static void DPYSendChar(unsigned char);
static void DPYSetCur(unsigned char,unsigned char);
static void DPYDrawChar(unsigned char,char *);
static te_DPY_BF DPYIsBusy(void);

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
//    DPYWriteNibble(RS_ISTRUCTION,0b00000011);
//
//    DLYDelay_ms(5);
//    DPYWriteNibble(RS_ISTRUCTION,0b00000011);
//
//    DLYDelay_us(100);
//    DPYWriteNibble(RS_ISTRUCTION,0b00000011);

    //Imosta la modalità a 4 bit.
    DPYWriteNibble(RS_ISTRUCTION,0b00000010);

    //Imposta la modalità a 4 bit, il numero di linee e la grandezza del font.
    tmp=0b00100000;
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
    DPYWriteByte(RS_ISTRUCTION,tmp);

    //Accende il display e il cursore.
    DPYWriteByte(RS_ISTRUCTION,0b00001110);

    //Setta la modalità di incremento.
    DPYWriteByte(RS_ISTRUCTION,0b00000110);

    //Cancella il display.
    DPYWriteByte(RS_ISTRUCTION,0b00000001);

    //Disegna il carattere per la batteria.
    DPYDrawChar(ADDR_PICBATT,picBatt);
}

static void DPYDrawChar(unsigned char addr,char *p)
{
    unsigned char i,tmpAddr;
    //Setta l'indirizzo e il dato.
    for(i=0;i<9;i++)
    {
        tmpAddr=(((i+addr)&0b00111111)|0b01000000);
        DPYWriteByte(RS_ISTRUCTION,tmpAddr);
        DPYWriteByte(RS_DATA,(*(p+i)));
    }
}

static void DPYSetCur(unsigned char row,unsigned char col)
{
    unsigned char addr;

    //Ricava l'indirizzo per la scrittura del carattere.
    addr=col;
    //Controlla se è la seconda riga.
    if(row==1)
        addr+=0x40;

    //Setta l'indirizzo.
    DPYWriteByte(RS_ISTRUCTION,(addr|0b10000000));
}

static void DPYSendChar(unsigned char c)
{
    //Scrive il carattere.
    DPYWriteByte(RS_DATA,c);
}

void DPYSendStr(unsigned char row,unsigned char col,char* data,unsigned char len)
{
    unsigned char i;

    //Setta la posizione iniziale del cursore.
    DPYSetCur(row,col);

    for(i=0;i<len;i++)
    {
        //Invia i caratteri.
        DPYSendChar((*(data+i)));
        while(DPYIsBusy())
            NOP();

        //Setta il cursore per il prossimo carattere.
        col++;
        if(col==DPY_COL_NUM)
        {
            col=0;
            row++;
            DPYSetCur(row,col);
        }
        else if(col==32)
        {
            col=0;
            row=0;
            DPYSetCur(row,col);
        }
    }
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
    DLYDelay_us(100);

    //Settaggio la linea E.
    OUTSetEOn();
    //Attesa.
    DLYDelay_us(100);
    //Settaggio linee dati (MSB).
    OUTSetHTCDataNibble(val&0x0F);
    //Attesa.
    DLYDelay_us(100);
    //Settaggio la linea E.
    OUTSetEOff();
    //Attesa.
    DLYDelay_us(100);
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
    DLYDelay_us(100);

    //Settaggio la linea E.
    OUTSetEOn();
    //Attesa.
    DLYDelay_us(100);
    //Settaggio linee dati (MSB).
    OUTSetHTCDataNibble((val>>4)&0b00001111);
    //Attesa.
    DLYDelay_us(100);
    //Settaggio la linea E.
    OUTSetEOff();
    //Attesa.
    DLYDelay_us(100);
    //Settaggio la linea E.
    OUTSetEOn();
    //Attesa.
    DLYDelay_us(100);
    //Settaggio linee dati (LSB).
    OUTSetHTCDataNibble(val&0b00001111);
    //Attesa.
    DLYDelay_us(100);
    //Settaggio la linea E.
    OUTSetEOff();
    //Attesa.
    DLYDelay_us(100);
}

static te_DPY_BF DPYReadByte(te_DPY_RS rs,unsigned char *val)
{
    //Azzeramento valore.
    (*(val))=0;

    //Settaggio direzione pin.
    tris_DPY|=0b00001111;

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
    DLYDelay_us(100);

    //Settaggio la linea E.
    OUTSetEOn();
    //Settaggio linee dati (MSB).
    if(INGetD7())
        (*(val))|=0b10000000;
    if(INGetD6())
        (*(val))|=0b01000000;
    if(INGetD5())
        (*(val))|=0b00100000;
    if(INGetD4())
        (*(val))|=0b00010000;
    //Attesa.
    DLYDelay_us(100);
    //Settaggio la linea E.
    OUTSetEOff();
    //Attesa.
    DLYDelay_us(100);
    //Settaggio la linea E.
    OUTSetEOn();
    //Settaggio linee dati (LSB).
    if(INGetD7())
        (*(val))|=0b00001000;
    if(INGetD6())
        (*(val))|=0b00000100;
    if(INGetD5())
        (*(val))|=0b00000010;
    if(INGetD4())
        (*(val))|=0b00000001;
    //Attesa.
    DLYDelay_us(100);
    //Settaggio la linea E.
    OUTSetEOff();
    //Attesa.
    DLYDelay_us(100);

    //Settaggio busy-flag.
    //TODO

    //Settaggio direzione pin.
    tris_DPY&=0b11110000;

    if((*(val))&0b10000000)
        return BF_BUSY;
    else
        return BF_NOTBUSY;
}

static te_DPY_BF DPYIsBusy(void)
{
    te_DPY_BF bf;
    
    //Settaggio direzione pin.
    tris_DPY|=0b00001111;

    //Settaggio la linea E.
    OUTSetEOff();

    //Settaggio la linea RS.
    OUTSetRSOff();

    //Settaggio linea RW.
    OUTSetRWOn();

    //Attesa.
    DLYDelay_us(100);

    //Settaggio la linea E.
    OUTSetEOn();
    //Settaggio busy flag.
    bf=BF_NOTBUSY;
    if(INGetD7())
        bf=BF_BUSY;
    //Attesa.
    DLYDelay_us(100);
    //Settaggio la linea E.
    OUTSetEOff();
    //Attesa.
    DLYDelay_us(100);
    //Settaggio la linea E.
    OUTSetEOn();
    //Attesa.
    DLYDelay_us(100);
    //Settaggio la linea E.
    OUTSetEOff();
    //Attesa.
    DLYDelay_us(100);

    //Settaggio direzione pin.
    tris_DPY&=0b11110000;

    return bf;
}

////----------------------------------------------------------------------------------------------------
////Function LcdInit
////Descrizione           : inizializza il display
////Parametri passati     : nessuno
////Parametri restituiti  : nessuno
//
////----------------------------------------------------------------------------------------------------
////Caratteristiche del display LCD
//#define ROWS 2      //numero di righe
//#define COLS 16     //numero di colonne
////----------------------------------------------------------------------------------------------------
//
////----------------------------------------------------------------------------------------------------
////Cablaggio pin LCD
//#define	RS    pin_RS
//#define	RW    pin_RW
//#define	EN    pin_E
//
//#define LCD_D4    pin_D4
//#define LCD_D5    pin_D5
//#define LCD_D6    pin_D6
//#define LCD_D7    pin_D7
////#define LCDPORT   PORTA      // Porta di I/O display LCD
////----------------------------------------------------------------------------------------------------
//
////----------------------------------------------------------------------------------------------------
//#define Read            1
//#define Write           0
//#define LCDOUTPUT   0b00000000 	 	  // Direzione port con bus dati da uC  a LCD
//#define LCDINPUT    0x00011111 		  // Direzione port con bus dati da LCD a uC
////----------------------------------------------------------------------------------------------------
//
//void LcdInit(void);
//void LcdStrobe(void);
//void putch (unsigned char);
//
//void LcdInit(void)
//{
//  TRISA = LCDOUTPUT;                               // Inizializzazione direzione PORTA
//  DLYDelay_us(10000);                                    // Attendo Power On LCD Controller
//  RS=0; RW=0; LCD_D7=0; LCD_D6=0; LCD_D5=1; LCD_D4=0;    // Function set 4 bit
//  DLYDelay_us(15000);
//  LcdStrobe();        // Clock su EN
//  DLYDelay_us(10000);
//  LcdStrobe();        // Clock per Display
//  DLYDelay_us(10000);
//  LcdStrobe();        // Clock per Display
//  DLYDelay_us(10000);
//  putch(0b00100000);                             // 4 bit
//  putch(0b00101000);                             // 4 bit, 2 lines, 5*7 dots
//  putch(0b00001000);                             // Display OFF
//  putch(0b00001100);                             // Display ON
//  putch(0b00000001);                             // Display clear
//  putch(0b00000110);                             // Auto increment ON + no shift
//  DLYDelay_us(10000);
// }
////----------------------------------------------------------------------------------------------------
//
////----------------------------------------------------------------------------------------------------
////Function LcdStrobe
////Descrizione           : da uno strobe (1-->0) su enable (EN)
////Parametri passati     : nessuno
////Parametri restituiti  : nessuno
//
//void LcdStrobe(void)
//{
// EN = 1;
// DLYDelay_us(10);
// EN = 0;
//}
////----------------------------------------------------------------------------------------------------
//
////----------------------------------------------------------------------------------------------------
////Function LcdBusy
////Descrizione          : interroga il display per vedere se è libero
////Parametri passati    : nessuno
////Parametri restituiti : unsigned char (0 --> Lcd libero, ~0 --> Lcd occupato)
//
//unsigned char LcdBusy (void)
//{
// TRISA = LCDINPUT;               //Lcd --> uC
// RW = Read;                      //In lettura
// return(LCD_D7);                 //LcdBusy = 1 --> LCD occupato
//                                 //LcdBusy = 0 --> LCD libero
//}
////----------------------------------------------------------------------------------------------------
//
////----------------------------------------------------------------------------------------------------
////Function LcdGotoXY
////Descrizione           : posizione il cursore sul display
////Parametri passati     : numero di riga (unsigned char) e numero di colonna (unsigned char)
////Parametri restituiti  : nessuno
//
//void LcdGotoXY (unsigned char Riga, unsigned char Colonna)
//{
//  static unsigned char Data;
//  static bit Temp;
//  Temp = RS;                                        // BackUp del bit CONTROLLO/DATO
//  TRISA = LCDOUTPUT;                                // Inizializzazione direzione PORTA
//  RS = 0;                                           // Parole di controllo ...
//  if (Riga    <  1)    Riga    =  1;                // Saturazione Riga min
//  if (Riga    >  ROWS) Riga    =  2;                // Saturazione Riga max
//  if (Colonna <  1)    Colonna =  1;                // Saturazione Colonna min
//  if (Colonna > COLS)  Colonna = 16;                // Saturazione Colonna max
//
//  switch (Riga)
//  {
//   case 1:
//   Data = 0x80;
//   break;
//
//   case 2:
//   Data = 0xC0;
//   break;
//
//   case 3:
//   Data = 0x90;
//   break;
//
//   case 4:
//   Data = 0xD0;
//   break;
//  }
//
//  Data  += Colonna - 1;
//  putch(Data);                                   // Esegui comando
//  RS = Temp;
//}
////----------------------------------------------------------------------------------------------------
//
////----------------------------------------------------------------------------------------------------
////Function LcdDelLine
////Descrizione           : cancella una linea sul display
////Parametri passati     : numero di riga (unsigned char)
////Parametri restituiti  : nessuno
//
//void LcdDelLine (unsigned char Riga)
//{
//  unsigned char i;
//  for (i=1; i<COLS+1; i++)
//  {
//   LcdGotoXY (Riga,i);
//   RS = 1;
//   putch(' ');
//  }
//}
////----------------------------------------------------------------------------------------------------
//
////----------------------------------------------------------------------------------------------------
////Function LcdClrScr
////Descrizione           : pulisce il display
////Parametri passati     : nessuno
////Parametri restituiti  : nessuno
//
//void LcdClrScr (void)
//{
// unsigned char m;
// for (m=1; m<ROWS+1; m++)
// {
//  LcdDelLine(m);
// }
//}
////----------------------------------------------------------------------------------------------------
//
////----------------------------------------------------------------------------------------------------
////Function putch
////Descrizione           : stampa un carattere sul display, overloading di putch <stdio.h>
////Parametri passati     : carattere da stampare
////Parametri restituiti  : nessuno
//
// void putch (unsigned char c)
// {
//  static bit Temp;
//
//  Temp = RS;                                 // BackUp del bit CONTROLLO/DATO
//
//  while(!LcdBusy());                               // Attesa da LCD Busy ...
//
//  // Direzione dati: da PIC a LCD
//  TRISA = LCDOUTPUT;                               // Nibble dati in uscita
//
//  // Trasmissione H-Nibble
//  OUTSetHTCDataNibble((c & 0xF0));
//  RS = Temp;
//  RW = Write;                                // Scrittura
//  EN = 1; DLYDelay_us(100); EN = 0; DLYDelay_us(100);    // Clock per Display   Clock su EN
//
//  // Trasmissione L-Nibble
//  OUTSetHTCDataNibble((c & 0x0F)<<4);
//  RS = Temp;
//  RW = Write;                                // Scrittura
//  EN = 1; DLYDelay_us(100); EN = 0; DLYDelay_us(100);    // Clock per Display  Clock su EN
//}
////----------------------------------------------------------------------------------------------------
