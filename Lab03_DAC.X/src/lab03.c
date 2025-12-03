#include "lab03.h"

#include <xc.h>
//do not change the order of the following 2 definitions
#define FCY 12800000UL
#include <libpic30.h>

#include "types.h"
#include "lcd.h"
#include "led.h"
uint16_t makeDACCommand(float Vout);
void sendSDI(uint16_t cmd); 
float V1=1.0;
float V2=2.5;
float V3=3.5;
uint16_t cmd1=0;
uint16_t cmd2=0;
uint16_t cmd3=0;

uint16_t time=500;
uint16_t F=12800000;
uint16_t prescale=256;

volatile uint16_t milliseconds = 0;
int i=0;

/*
 * DAC code
 */

#define DAC_CS_TRIS TRISDbits.TRISD8
#define DAC_SDI_TRIS TRISBbits.TRISB10
#define DAC_SCK_TRIS TRISBbits.TRISB11
#define DAC_LDAC_TRIS TRISBbits.TRISB13
    
#define DAC_CS_PORT PORTDbits.RD8
#define DAC_SDI_PORT PORTBbits.RB10
#define DAC_SCK_PORT PORTBbits.RB11
#define DAC_LDAC_PORT PORTBbits.RB13

#define DAC_SDI_AD1CFG AD1PCFGLbits.PCFG10
#define DAC_SCK_AD1CFG AD1PCFGLbits.PCFG11
#define DAC_LDAC_AD1CFG AD1PCFGLbits.PCFG13

#define DAC_SDI_AD2CFG AD2PCFGLbits.PCFG10
#define DAC_SCK_AD2CFG AD2PCFGLbits.PCFG11
#define DAC_LDAC_AD2CFG AD2PCFGLbits.PCFG13

void dac_initialize()
{
    // set AN10, AN11 AN13 to digital mode
    //SETBIT(DAC_CS_AD1CFG);
    //SETBIT(DAC_CS_AD2CFG);
    SETBIT(DAC_SCK_AD1CFG);
    SETBIT(DAC_SCK_AD2CFG);
    
    SETBIT(DAC_SDI_AD1CFG);
    SETBIT(DAC_SDI_AD2CFG);
    
    SETBIT(DAC_LDAC_AD1CFG);
    SETBIT(DAC_LDAC_AD2CFG);
    
    // this means AN10 will become RB10, AN11->RB11, AN13->RB13
    // see datasheet 11.3
    
    // set RD8, RB10, RB11, RB13 as output pins
    CLEARBIT(DAC_CS_TRIS);
    CLEARBIT(DAC_SCK_TRIS);
    CLEARBIT(DAC_SDI_TRIS);
    CLEARBIT(DAC_LDAC_TRIS);
    
    // set default state: CS=??, SCK=??, SDI=??, LDAC=??
    DAC_CS_PORT = 1;
    DAC_SCK_PORT = 0;
    DAC_SDI_PORT = 0;
    DAC_LDAC_PORT = 1;
}

/*
 * Timer code
 */
#define FCY_EXT   32768UL

#define TCKPS_1   0x00
#define TCKPS_8   0x01
#define TCKPS_64  0x02
#define TCKPS_256 0x03

void timer_initialize()
{
    // Enable RTC Oscillator -> this effectively does OSCCONbits.LPOSCEN = 1
    // but the OSCCON register is lock protected. That means you would have to 
    // write a specific sequence of numbers to the register OSCCONL. After that 
    // the write access to OSCCONL will be enabled for one instruction cycle.
    // The function __builtin_write_OSCCONL(val) does the unlocking sequence and
    // afterwards writes the value val to that register. (OSCCONL represents the
    // lower 8 bits of the register OSCCON)
    __builtin_write_OSCCONL(OSCCONL | 2);
    // configure timer
     // Disable the Timers
    CLEARBIT(T2CONbits.TON);
    // Set Prescaler
    T2CONbits.TCKPS = 0b11; 
    // Set Clock Source
    CLEARBIT(T2CONbits.TCS);
    // Set Gated Timer Mode -> don't use gating
    CLEARBIT(T2CONbits.TGATE);
    // T1: Set External Clock Input Synchronization -> no sync
    // Load Timer Periods
    PR2 = (time *F)/prescale;
    // Reset Timer Values
    TMR2 = 0x00;
    // Set Interrupt Priority
    IPC1bits.T2IP = 0x06; 
    // Clear Interrupt Flags
    CLEARBIT(IFS0bits.T2IF); 
    // Enable Interrupts
    SETBIT(IEC0bits.T2IE); 
    // Enable the Timers
    SETBIT(T2CONbits.TON); 
}

// interrupt service routine?

/*
 * main loop
 */

void main_loop()
{
    // print assignment information
    lcd_printf("Lab03: DAC");
    lcd_locate(0, 1);
    lcd_printf("Group: Group4");

    //lcd_locate(0, 2);
     cmd1=makeDACCommand(V1);
     cmd2=makeDACCommand(V2);
     cmd3=makeDACCommand(V3);

    lcd_locate(0, 2);
    lcd_printf("0x%04X", cmd2);
    
    while(TRUE){
        TOGGLELED(LED1_PORT);
        sendSDI(cmd1);
        delay (380); // 500ms delay calibrated with the system delay 
        sendSDI(cmd2);
        delay (1500); //2000ms
        sendSDI(cmd3);
        delay (750);//1000ms
        
    }
}


uint16_t makeDACCommand(float Vout){
    float Vref= 4.096;
    float Resolution= 4096.0;
    
    uint16_t DAC_input_code=(uint16_t)((Vout/Vref)*Resolution); //Found in datasheet section 4, equation 4-1 (e.g: 0000 0011 1110 1000)
    uint16_t control_bits=0x1000; // bits 12-15 in binary 0001 0000 0000 0000
    
    return  control_bits | DAC_input_code ; //example for vout 1 return = 0001 0011 1110 1000
}

void sendSDI(uint16_t cmd){
    CLEARBIT(DAC_CS_PORT);
    
    for (i=15; i>=0; i--){
        if (cmd &(1<< i)){
            SETBIT(DAC_SDI_PORT);
        } else {
            CLEARBIT(DAC_SDI_PORT) ;
        }
        Nop();
        SETBIT(DAC_SCK_PORT);
        Nop();
        CLEARBIT(DAC_SCK_PORT);
    }
    SETBIT(DAC_CS_PORT);
    CLEARBIT(DAC_SDI_PORT);
    CLEARBIT(DAC_LDAC_PORT);
    Nop();
    SETBIT(DAC_LDAC_PORT);
    return;
}



        
void __attribute__((__interrupt__, __shadow__, __auto_psv__)) _T2Interrupt(void)
{
    milliseconds++;
    CLEARBIT(IFS0bits.T2IF);
}

void delay (int ms){
    milliseconds=0;
    while(milliseconds<ms);
}
 
 
 

