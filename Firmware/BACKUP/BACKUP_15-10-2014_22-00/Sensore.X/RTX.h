
// Struttura pacchetto:
//
// Lunghezza 5 byte.
// - 1° byte ID destinazione (0x00 comunicazione in broadcast).
// - 2° byte ID sorgente.
// - 3° byte OPCODE.
// - 3° byte valore letto del sensore di temperatura.
// - 4° byte valore letto dalla tensione della batteria.

// OPCODE:
//
// - DAT: dati.
// - PGM: programmazione.
// - rDAT: risposta per conferma ricezione dati.
// - rPGM: risposta per conferma programmazione.

typedef union _RTX_EV
{
    unsigned char val;
    struct
    {
        unsigned int sleep;     //Segnala che il dispositivo può andare in sleep.
    };
}tu_RTX_EV;

tu_RTX_EV rtx_ev;

void RTXInit(unsigned char);
void RTXTick(void);
void RTXReadPkt(void);
void RTXSelDown(void);
void RTXTask(void);
