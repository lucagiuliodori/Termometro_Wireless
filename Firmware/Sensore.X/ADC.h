#define VAL_CALTEMP 2     //Valore per la calibrazione del sensore di temperatura.

unsigned char batt,temp;
float battV,tempC;

void ADCInit(unsigned char state);
void ADCConv(void);
#define ADCGetTemp() temp
#define ADCGetBatt() batt
#define ADCGetTempC() tempC
#define ADCGetBattV() battV
#define ADCGetTempCCal() tempC+VAL_CALTEMP
