
#define bitMask_LED 0b10000000
#define bitMask_SS 0b00100000
#define bitMask_SCK 0b00001000
#define bitMask_SDO 0b00100000
#define bitMask_TESTBATT 0b00100000

#define port_LED PORTC
#define port_SS PORTA
#define port_SCK PORTC
#define port_SDO PORTC
#define port_TESTBATT PORTB

#define pin_LED PORTCbits.RC7
#define pin_SS PORTAbits.RA5
#define pin_SCK PORTCbits.RC3
#define pin_SDO PORTCbits.RC5
#define pin_TESTBATT PORTBbits.RB5

#define OUTSetLEDOff() pin_LED=1
#define OUTSetSSOff() pin_SS=0
#define OUTSetSCKOff() pin_SCK=0
#define OUTSetSDOOff() pin_SDO=0
#define OUTSetTESTBATTOff() pin_TESTBATT=0

#define OUTSetLEDOn() pin_LED=0
#define OUTSetSSOn() pin_SS=1
#define OUTSetSCKOn() pin_SCK=1
#define OUTSetSDOOn() pin_SDO=1
#define OUTSetTESTBATTOn() pin_TESTBATT=1

#define OUTSetLEDToggle() port_LED^=bitMask_LED
#define OUTSetSSToggle() port_SS^=bitMask_SS
#define OUTSetSCKToggle() port_SCK^=bitMask_SCK
#define OUTSetSDOToggle() port_SDO^=bitMask_SDO
#define OUTSetTESTBATTToggle() port_TESTBATT^=bitMask_TESTBATT

void OUTInit(unsigned char);
