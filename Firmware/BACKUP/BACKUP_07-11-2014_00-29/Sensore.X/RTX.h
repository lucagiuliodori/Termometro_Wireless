
// Struttura pacchetto (lunghezza 9 byte; il tipo flot in xc8 è rappresentato con 3 byte):
//
// Lunghezza 9 byte pacchetto invio dati.
// - 1° byte ID destinazione (0x00 comunicazione in broadcast).
// - 2° byte ID sorgente.
// - 3° byte OPCODE.
// - 4°->6° byte valore letto del sensore di temperatura (LSB->MSB).
// - 7°->9° byte valore letto dalla tensione della batteria (LSB->MSB).

// Lunghezza 3 byte pacchetto programmazione.
// - 1° byte ID destinazione (0x00 comunicazione in broadcast).
// - 2° byte ID sorgente.
// - 3° byte OPCODE.
// - 4°->6° 0x000000 (LSB->MSB).
// - 7°->9° 0x000000 (LSB->MSB).

// OPCODE:
//
// - DAT: dati.
// - PGM: programmazione.
// - rDAT: risposta per conferma ricezione dati.
// - rPGM: risposta per conferma programmazione.

typedef union _RTX_FLG
{
    unsigned char val;
    struct
    {
        unsigned int sendDone:1;    //Segnala che il dispositivo ha terminato l'invio dei pacchetti.
        unsigned int pgmDone:1;     //Segnala che la programmazione è stata eseguita.
    };
}tu_RTX_FLG;

tu_RTX_FLG rtx_flg;

void RTXInit(unsigned char);
void RTXTick(void);
void RTXReadPkt(void);
void RTXTask(void);
void RTXStartSendData(void);
