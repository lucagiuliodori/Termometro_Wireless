
unsigned char batt,temp;

void ADCInit(unsigned char state);
void ADCTask(void);
#define ADCGetTemp() temp;
#define ADCGetBatt() batt;
