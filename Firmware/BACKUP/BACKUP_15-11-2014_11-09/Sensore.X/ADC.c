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
            break;
//        case 2:
//            //Inizializzazione applicazione.
//            break;
    }
}

void ADCConv(void)
{
    unsigned short sumBatt=0,sumTemp=0;
    unsigned char cntSum=0;

    while(1)
    {
        //Controlla quale canale è attivo.
        switch(adc_ch)
        {
            case ADC_CH_BATT:
                //Attiva l'uscita per il controllo della tensione della batteria.
                OUTSetTESTBATTOn();
                //Controlla se sono state raggiunte il numero massimo di somme per il calcolo della media.
                if(cntSum==MAX_CNTSUM)
                {
                    //Calcola la media.
                    batt=sumBatt/MAX_CNTSUM;
                    //Calcola la tensione in volt.
                    /*"*4" perchè la conversione è a 10bit anche se vengono
                     * considerati solo gli 8 più significativi; "*1.024/1024"
                     * per ottenere i mV corrispondenti; "*11" perchè il
                     partitore utilizzato ha un rapporto di 1/11.*/
                    battV=batt*4*1.024/1024*11;
                    //Azzera la somma.
                    sumBatt=0;
                    //Azzera il contatore.
                    cntSum=0;
                    //Setta la prossima conversione AD.
                    ADCON1=ADCON1_TEMP;
                    ADCON0=ADCON0_TEMP;
                    adc_ch=ADC_CH_TEMP;
                }
                else
                {
                    //Legge e somma il valore (approssimato a 8 bit).
                    sumBatt+=ADRESH;
                    //Incrementa il contatore.
                    cntSum++;
                }
                //Ritardo sospensivo per l'attivazione della conversione.
                __delay_us(10);
                //Attivazione della conversione.
                ADCON0bits.GO=1;
                break;
            case ADC_CH_TEMP:
                //Disattiva l'uscita per il controllo della tensione della batteria.
                OUTSetTESTBATTOff();
                //Controlla se sono state raggiunte il numero massimo di somme per il calcolo della media.
                if(cntSum==MAX_CNTSUM)
                {
                    //Calcola la media.
                    temp=sumTemp/MAX_CNTSUM;
                    //Calcola il valore in celsius.
                    /*"*4" perchè la conversione è a 10bit mentre vengono
                     * considerati solo gli 8 più significativi; "*(battV/1024)*1000"
                     * per ottenere i mV corrispondenti; "-747" per riferimento
                     * allo 0°C (valore da datasheet); "/2.47" per ricavare
                     * il valore della temperatura in °C.*/
                    tempC=((temp*4)*(battV/1024)*1000-747)/2.47;
                    //Azzera la somma.
                    sumTemp=0;
                    //Azzera il contatore.
                    cntSum=0;
                    //Setta la prossima conversione AD.
                    ADCON1=ADCON1_BATT;
                    ADCON0=ADCON0_BATT;
                    adc_ch=ADC_CH_BATT;
                    //Conversioni completate.
                    return;
                }
                else
                {
                    //Legge e somma il valore (approssimato a 8 bit).
                    sumTemp+=ADRESH;
                    //Incrementa il contatore.
                    cntSum++;
                }
                //Ritardo sospensivo per l'attivazione della conversione.
                __delay_us(10);
                //Attivazione della conversione.
                ADCON0bits.GO=1;
                break;
        }
        //Attende il completamento della conversione.
        while(ADCON0bits.GO)
            CLRWDT();
    }
}

float ADCGetTempCCal(void)
{
    if(battV>=3)
        return (tempC+VAL_CALTEMP_SUP3V);
    else
        return (tempC+VAL_CALTEMP_INF3V);
}