#include <xc.h>
#include "Sensore.h"
#include "ADC.h"
#include "OUT.h"
#include "DLY.h"

#define ADCON0_BATT 0b00101101
#define ADCON0_TEMP 0b00101001

#define ADCON1_BATT 0b00000011      //Per la misurazione della batteria occorre settare come riferimento di tensione il VR interno.
#define ADCON1_TEMP 0b00000000      //Per la misurazione della batteria occorre settare come riferimento di tensione la tensione di alimentazione.

#define MAX_CNTSUM 10   //Numero di somme per il calcolo della media.

typedef enum _ADC_CH
{
    ADC_CH_BATT,
    ADC_CH_TEMP
}te_ADC_CH;

#define mADCInit() ADCON0=ADCON0_BATT; ADCON1=ADCON1_BATT; FVRCON=0b10000001; ANSELB|=0b00010010;

te_ADC_CH adc_ch;

void ADCInit(unsigned char state)
{
    switch(state)
    {
        case 0:
            //Inizializzazione hardware.
            mADCInit();
            break;
        case 1:
            //Inizializzazione variabili.
            adc_ch=ADC_CH_BATT;
            batt=0;
            temp=0;
            battV=0;
            tempC=0;
            adc_ev.val=0;
            break;
        case 2:
            //Inizializzazione applicazione.
            //Ritardo sospensivo per l'attivazione della conversione.
            __delay_us(10);
            //Attivazione della conversione.
            ADCON0bits.GO=1;
            break;
    }
}

void ADCTask(void)
{
    static unsigned short sumBatt=0,sumTemp=0;
    static unsigned char cntSum=0;

    //Controlla che la conversione AD sia terminata.
    if(!(ADCON0bits.GO))
    {
        //Disattiva l'uscita per il controllo della tensione della batteria.
        OUTSetTESTBATTOff();
        //Controlla quale canale è attivo.
        switch(adc_ch)
        {
            case ADC_CH_BATT:
                //Controlla se sono state raggiunte il numero massimo di somme per il calcolo della media.
                if(cntSum==MAX_CNTSUM)
                {
                    //Calcola la media.
                    batt=sumBatt/MAX_CNTSUM;
                    //Calcola la tensione in volt.
                    /*"*4" perchè la conversione è a 10bit anche se vengono
                     * considerati solo gli 8 più significativi; "*2.024/1024"
                     * per ottenere i mV corrispondenti; "*11" perchè il
                     partitore utilizzato ha un rapporto di 1/11.*/
                    battV=batt*4*1.024/1024*11;
                    //Azzera la somma.
                    sumBatt=0;
                }
                else
                    sumBatt+=ADRESH;    //Legge e somma il valore (approssimato a 8 bit).
                //Setta la prossima conversione AD.
                ADCON1=ADCON1_TEMP;
                ADCON0=ADCON0_TEMP;
                adc_ch=ADC_CH_TEMP;
                break;
            case ADC_CH_TEMP:
                //Controlla se sono state raggiunte il numero massimo di somme per il calcolo della media.
                if(cntSum==MAX_CNTSUM)
                {
                    //Calcola la media.
                    temp=sumTemp/MAX_CNTSUM;
                    //Calcola il valore in celsius.
                    /*"*4" perchè la conversione è a 10bit anche se vengono
                     * considerati solo gli 8 più significativi; "*(battV/1024)*1000"
                     * per ottenere i mV corrispondenti; "-747" per riferimento
                     * allo 0°C (valore da datasheet); "/2.47" per ricavare
                     * il valore della temperatura in °C.*/
                    tempC=((temp*4)*(battV/1024)*1000-968)/2.47;
                    //Azzera la somma.
                    sumTemp=0;
                    //Azzera il contatore.
                    cntSum=0;
                    //Segnala che un ciclo di conversione è stato eseguito.
                    adc_ev.convDone=1;
                }
                else
                {
                    //Legge e somma il valore (approssimato a 8 bit).
                    sumTemp+=ADRESH;
                    //Incrementa il contatore.
                    cntSum++;
                }
                //Setta la prossima conversione AD.
                OUTSetTESTBATTOn();
                ADCON1=ADCON1_BATT;
                ADCON0=ADCON0_BATT;
                adc_ch=ADC_CH_BATT;
                break;
        }
    }
    //Ritardo sospensivo per l'attivazione della conversione.
    __delay_us(10);
    //Attivazione della conversione.
    ADCON0bits.GO=1;
}
