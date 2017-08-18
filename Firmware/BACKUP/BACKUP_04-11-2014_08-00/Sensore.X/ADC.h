
typedef union _ADC_EV
{
    unsigned char val;
    struct
    {
        unsigned int convDone:1;
    };
}tu_ADC_EV;

tu_ADC_EV adc_ev;
unsigned char batt,temp;
float battV,tempC;

void ADCInit(unsigned char state);
void ADCTask(void);
#define ADCGetTemp() temp
#define ADCGetBatt() batt
#define ADCGetTempV() tempC
#define ADCGetBattV() battV
