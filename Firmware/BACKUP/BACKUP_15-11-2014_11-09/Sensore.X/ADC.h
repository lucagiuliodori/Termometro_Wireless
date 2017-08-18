#define VAL_CALTEMP_INF3V -102     //Valore per la calibrazione del sensore di temperatura.
#define VAL_CALTEMP_SUP3V -108     //Valore per la calibrazione del sensore di temperatura.

unsigned char batt,temp;
float battV,tempC;

void ADCInit(unsigned char state);
void ADCConv(void);
#define ADCGetTemp() temp
#define ADCGetBatt() batt
#define ADCGetTempC() tempC
#define ADCGetBattV() battV
float ADCGetTempCCal(void);
