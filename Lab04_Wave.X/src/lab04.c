#include "lab04.h"

#include <xc.h>
//do not change the order of the following 2 definitions
#define FCY 12800000UL
#include <libpic30.h>

#include "math.h"

#include "types.h"
#include "lcd.h"
#include "led.h"
#include "dac.h"



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






// signal parameter
#define frequency 10.0f   
#define sample_rate 250.0f
#define Vmin 0.0f
#define Vmax 2.0f
#define ticks(frequency) ((uint16_t)(12800000/frequency) )
#define pi 3.14159f





float voff = (Vmax/2.0f);
float vamp=(Vmax/2.0f);

float wsig=2.0f * pi *(frequency/sample_rate);
int i=0;
float Vout1=0.0f;
float phase=0.0f;
uint16_t cmd=0;
volatile uint16_t milliseconds = 0;
/*
 * Timer code
 */

#define TCKPS_1   0x00
#define TCKPS_8   0x01
#define TCKPS_64  0x02
#define TCKPS_256 0x03

void timer_initialize()
{
    
    // Timer 3
    T3CONbits.TON = 0;//Disable Timer
    T3CONbits.TCKPS = 0b00; // Prescaler
    CLEARBIT(T3CONbits.TCS); // Internal Clock
    CLEARBIT(T3CONbits.TGATE); // Disable Gate
    PR3 = ticks(frequency);
    TMR3 = 0x00;
    
    T3CONbits.TON = 1; // Start Timer
    
}

  


/*
 * main loop
 */

void main_loop()
{
    // print assignment information
    lcd_printf("Lab04: Wave");
    lcd_locate(0, 1);
    lcd_printf("Group: 4");
 
    
    while(TRUE) {  
        
        
  
    
    
    
    }
}



void __attribute__((__interrupt__, __shadow__, __auto_psv__)) _T3Interrupt(void)
{
    milliseconds++;
          
    phase+=wsig;
    if(phase>=(2.0f*pi)){
        phase-= (2.0f*pi);
    }
    Vout1=vamp * sinf(phase)+ voff;
    lcd_locate(0, 2);
    
    cmd=makeDACCommand(Vout1);
    sendSDI(cmd);
    CLEARBIT(IFS0bits.T2IF);
    
   
    
    
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