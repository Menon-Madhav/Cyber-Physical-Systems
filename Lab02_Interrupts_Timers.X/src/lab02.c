#include "lab02.h"

#include <xc.h>
#include <p33Fxxxx.h>
//do not change the order of the following 2 definitions
#define FCY 12800000UL
#include <libpic30.h>

#include "types.h"
#include "lcd.h"
#include "led.h"

#define FCY_EXT 32768

volatile uint16_t seconds = 0;
volatile uint16_t milliseconds = 0;
volatile uint16_t minutes = 0;
void initialize_timer()
{
    // Enable RTC Oscillator -> this effectively does OSCCONbits.LPOSCEN = 1
    // but the OSCCON register is lock protected. That means you would have to 
    // write a specific sequence of numbers to the register OSCCONL. After that 
    // the write access to OSCCONL will be enabled for one instruction cycle.
    // The function __builtin_write_OSCCONL(val) does the unlocking sequence and
    // afterwards writes the value val to that register. (OSCCONL represents the
    // lower 8 bits of the register OSCCON)
    __builtin_write_OSCCONL(OSCCONL | 2);
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
    PR2 = 99;
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
    
    
    // Timer 1
    T1CONbits.TON = 0;   // Disable Timer
    T1CONbits.TCKPS = 0b11; // Prescaler
    T1CONbits.TCS = 1; // External Clock
    CLEARBIT(T1CONbits.TGATE); // Disable Gated
    T1CONbits.TSYNC = 0; // // Disable Sync
    PR1 = 127;
    TMR1 = 0x00;
    IPC0bits.T1IP = 0x03; // Priority
    IFS0bits.T1IF = 0; // Clear Timer1 Interrupt Flag
    IEC0bits.T1IE = 1; // Enable Timer1 interrupt
    T1CONbits.TON = 1; // Start Timer
    
    
    // Timer 3
    T3CONbits.TON = 0;//Disable Timer
    T3CONbits.TCKPS = 0b00; // Prescaler
    CLEARBIT(T3CONbits.TCS); // Internal Clock
    CLEARBIT(T3CONbits.TGATE); // Disable Gate
    PR3 = 0xFFFF;
    TMR3 = 0x00;
    
    T3CONbits.TON = 1; // Start Timer
}

void timer_loop()
{
    // print assignment information
    lcd_printf("Lab02: Ints & Timer");
    lcd_locate(0, 1);
    lcd_printf("Group: 04");   
    uint16_t i = 0;
    while(TRUE)
    {   if(i == 2000){
            lcd_locate(0, 2);
            lcd_printf("%02u : %02u : %03u ", minutes, seconds, milliseconds);
            TOGGLELED(LED3_PORT);
            i = 0;
            lcd_locate(0, 3);
            lcd_printf("c = %lu ", TMR3);
            T3CONbits.TON = 0;
            TMR3 = 0x00;
            T3CONbits.TON = 1;
        }
        i++;
        
    }
}

void __attribute__((__interrupt__, __shadow__, __auto_psv__)) _T1Interrupt(void)
{ // invoked every ??
    seconds++;
    if(seconds == 60){
        seconds = 0;
        minutes++;
    }    
    TOGGLELED(LED2_PORT);
    CLEARBIT(IFS0bits.T1IF);
}

void __attribute__((__interrupt__, __shadow__, __auto_psv__)) _T2Interrupt(void)
{ // invoked every ??
    milliseconds+=2;
    if(milliseconds == 1000){
        milliseconds = 0;
    }    
    TOGGLELED(LED1_PORT);
    CLEARBIT(IFS0bits.T2IF);
    
}
