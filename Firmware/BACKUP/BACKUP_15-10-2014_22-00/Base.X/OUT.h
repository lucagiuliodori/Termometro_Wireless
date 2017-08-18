
#define bitMask_LED 0b10000000
#define bitMask_SS 0b00100000
#define bitMask_SCK 0b00001000
#define bitMask_SDO 0b00100000
#define bitMask_RS 0b01000000
#define bitMask_RW 0b10000000
#define bitMask_E 0b00010000
#define bitMask_D4 0b00001000
#define bitMask_D5 0b00000100
#define bitMask_D6 0b00000010
#define bitMask_D7 0b00000001

#define port_LED PORTC
#define port_SS PORTA
#define port_SCK PORTC
#define port_SDO PORTC
#define port_RS PORTA
#define port_RW PORTA
#define port_E PORTA
#define port_DATA PORTA

#define pin_LED PORTCbits.RC7
#define pin_SS PORTAbits.RA5
#define pin_SCK PORTCbits.RC3
#define pin_SDO PORTCbits.RC5
#define pin_RS PORTAbits.RA6
#define pin_RW PORTAbits.RA7
#define pin_E PORTAbits.RA4
#define pin_D4 PORTAbits.RA3
#define pin_D5 PORTAbits.RA2
#define pin_D6 PORTAbits.RA1
#define pin_D7 PORTAbits.RA0


#define OUTSetLEDOff() pin_LED=1
#define OUTSetSSOff() pin_SS=0
#define OUTSetSCKOff() pin_SCK=0
#define OUTSetSDOOff() pin_SDO=0
#define OUTSetRSOff() pin_RS=0
#define OUTSetRWOff() pin_RW=0
#define OUTSetEOff() pin_E=0
#define OUTSetD4Off() pin_D4=0
#define OUTSetD5Off() pin_D5=0
#define OUTSetD6Off() pin_D6=0
#define OUTSetD7Off() pin_D7=0

#define OUTSetLEDOn() pin_LED=0
#define OUTSetSSOn() pin_SS=1
#define OUTSetSCKOn() pin_SCK=1
#define OUTSetSDOOn() pin_SDO=1
#define OUTSetRSOn() pin_RS=1
#define OUTSetRWOn() pin_RW=1
#define OUTSetEOn() pin_E=1
#define OUTSetD4On() pin_D4=1
#define OUTSetD5On() pin_D5=1
#define OUTSetD6On() pin_D6=1
#define OUTSetD7On() pin_D7=1

#define OUTSetLEDToggle() port_LED^=bitMask_LED
#define OUTSetSSToggle() port_SS^=bitMask_SS
#define OUTSetSCKToggle() port_SCK^=bitMask_SCK
#define OUTSetSDOToggle() port_SDO^=bitMask_SDO
#define OUTSetRSToggle() port_RS^=bitMask_RS
#define OUTSetRWToggle() port_RW^=bitMask_RW
#define OUTSetEToggle() port_E^=bitMask_E
#define OUTSetD4Toggle() port_DATA^=bitMask_D4
#define OUTSetD5Toggle() port_DATA^=bitMask_D5
#define OUTSetD6Toggle() port_DATA^=bitMask_D6
#define OUTSetD7Toggle() port_DATA^=bitMask_D7

void OUTInit(unsigned char);
void OUTSetHTCDataNibble(unsigned char);
