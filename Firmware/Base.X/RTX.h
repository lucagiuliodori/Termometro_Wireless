
// Struttura pacchetto (lunghezza 9 byte; il tipo flot in xc8 � rappresentato con 3 byte):
//
// Lunghezza 9 byte pacchetto invio dati.
// - 1� byte ID destinazione (0x00 comunicazione in broadcast).
// - 2� byte ID sorgente.
// - 3� byte OPCODE.
// - 4�->6� byte valore letto del sensore di temperatura (LSB->MSB).
// - 7�->9� byte valore letto dalla tensione della batteria (LSB->MSB).

// Lunghezza 3 byte pacchetto programmazione.
// - 1� byte ID destinazione (0x00 comunicazione in broadcast).
// - 2� byte ID sorgente.
// - 3� byte OPCODE.
// - 4�->6� 0x000000 (LSB->MSB).
// - 7�->9� 0x000000 (LSB->MSB).

// OPCODE:
//
// - DAT: dati.
// - PGM: programmazione.
// - rDAT: risposta per conferma ricezione dati.
// - rPGM: risposta per conferma programmazione.

typedef enum _RTX_UPD_DPY
{
    UPD_DPY_RST,
    UPD_DPY_TEMP,
    UPD_DPY_BATT,
    UPD_DPY_PGM,
    UPD_DPY_PGMDONE,
    UPD_DPY_PGMRST
}te_RTX_UPD_DPY;

typedef union _RTX_FLG
{
    unsigned char val;
    struct
    {
        unsigned int sendDone:1;    //Segnala che il dispositivo ha terminato l'invio dei pacchetti.
        unsigned int pgmDone:1;     //Segnala che la programmazione � stata eseguita.
        unsigned int pgmonoff:1;    //Flag per indicare lo stato della programmazione.
    };
}tu_RTX_FLG;

tu_RTX_FLG rtx_flg;
float temp,batt;

void RTXInit(unsigned char);
void RTXTick(void);
void RTXReadPkt(void);
void RTXTask(void);
void RTXStartSendData(void);
#define RTXGetTemp() temp
#define RTXGetBatt() batt
void RTXPgmOff(void);
void RTXPgmOn(void);
void RTXUpdateDisplay(te_RTX_UPD_DPY);
void RTXSelDown(void);
