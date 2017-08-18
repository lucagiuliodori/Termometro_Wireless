#include <xc.h>
#include "Sensore.h"
#include "ADC.h"

#define ADCON0_BATT 0b10000001
#define ADCON0_TEMP 0b10101001

#define MAX_CNTSUM 10   //Numero di somme per il calcolo della media.

typedef enum _ADC_CH
{
    ADC_CH_BATT,
    ADC_CH_TEMP
}te_ADC_CH;

#define mADCInit() ADCON0=ADCON0_BATT; ADCON1=0b00000000

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
        //Controlla quale canale è attivo.
        switch(adc_ch)
        {
            case ADC_CH_BATT:
                //Controlla se sono state raggiunte il numero massimo di somme per il calcolo della media.
                if(cntSum==MAX_CNTSUM)
                {
                    //Calcola la media.
                    batt=sumBatt/MAX_CNTSUM;
                    //Azzera la somma.
                    sumBatt=0;
                }
                else
                    sumBatt+=ADRESH;    //Legge e somma il valore (approssimato a 8 bit).
                //Setta la prossima conversione AD.
                ADCON0=ADCON0_TEMP;
                adc_ch=ADC_CH_TEMP;
                break;
            case ADC_CH_TEMP:
                //Controlla se sono state raggiunte il numero massimo di somme per il calcolo della media.
                if(cntSum==MAX_CNTSUM)
                {
                    //Calcola la media.
                    temp=sumTemp/MAX_CNTSUM;
                    //Azzera la somma.
                    sumTemp=0;
                    //Azzera il contatore.
                    cntSum=0;
                }
                else
                {
                    //Legge e somma il valore (approssimato a 8 bit).
                    sumTemp+=ADRESH;
                    //Incrementa il contatore.
                    cntSum++;
                }
                //Setta la prossima conversione AD.
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
