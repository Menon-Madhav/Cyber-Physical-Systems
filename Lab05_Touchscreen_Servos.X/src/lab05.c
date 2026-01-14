#include "lab05.h"

#include <xc.h>
//do not change the order of the following 2 definitions
#define FCY 12800000UL
#include <libpic30.h>

#include "types.h"
#include "lcd.h"
#include "led.h"



void timer_initialize();
void init_servo(char servo);
void set_servo(char servo, uint16_t pulse);
void init_touch();
void set_touch(char dim);
uint16_t read_touch ();
void print_position();






//for servo
char servo;

/*
 * PWM code
 */

#define TCKPS_1   0x00
#define TCKPS_8   0x01
#define TCKPS_64  0x02
#define TCKPS_256 0x03

#define PWM_MIN_US 1000
#define PWM_MID_US 1500
#define PWM_MAX_US 2000
#define PWM_CYC_US 20000

#define TICK_US 5  // it takes 5us for each tick  1/timer_frequency 


#define US_to_ticks(us) ((uint16_t)((us)/ TICK_US))


#define corner_delay 5000

#define X_Left 1100
#define X_Right 1900
#define Y_front 1100
#define Y_back 1900

/*
 * touch screen code
 */


/*
 * main loop
 */

void main_loop()
{
    // print assignment information
    lcd_printf("Lab05: Touchscreen &\r\n");
    lcd_printf("       Servos");
    lcd_locate(0, 2);
    lcd_printf("Group: Group 4");
    
    // initialize touchscreen
     init_touch();
    // initialize servos
    init_servo('X');
    init_servo('Y');
    //set_servo('X',X_Left);
    //set_servo('Y',Y_front);
    
    uint8_t step=0;
     
    while(TRUE) {
       
        switch(step){
            case 0:
                set_servo('X',X_Left);
                set_servo('Y',Y_front);
                break;
                
             case 1:
                set_servo('X',X_Left);
                set_servo('Y',Y_back);
                break;
                
             case 2:
                set_servo('X',X_Right);
                set_servo('Y',Y_back);
                break;
             case 3:
                set_servo('X',X_Right);
                set_servo('Y',Y_front);
                break;
        
        
        }
        print_position();
        step ++;
        if (step==4){
            step=0;
        }
        __delay_ms(5000);
        
    }
     
}


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
    CLEARBIT(T2CONbits.TON);//Disable Timer

    CLEARBIT(T2CONbits.TCS); // Internal Clock
    CLEARBIT(T2CONbits.TGATE); // Disable Gate
    T2CONbits.TCKPS = 0b10; // Prescaler
   
    TMR2 = 0x00;
    CLEARBIT(IFS0bits.T2IF);
    CLEARBIT(IEC0bits.T2IE);
    PR2 = US_to_ticks(PWM_CYC_US);
    // Enable the Timers
    SETBIT(T2CONbits.TON); 
    
   
    
}

void init_servo(char servo){
    
    static uint8_t timer_init=0;
    
    if (!timer_init){
        
        timer_initialize();
        timer_init=1;
    
    }
    
    uint16_t init_ticks= PR2- US_to_ticks(1500);

    //Setup OC8
    if (servo == 'X'){
        CLEARBIT(TRISDbits.TRISD7); //Set OC8 as output
        OC8R=init_ticks;                  //set initial duty cycle to 5ms 
        OC8RS=init_ticks;                 //Load OCRS: next pwm duty cycle 
        OC8CON= 0x0006;            //set OC8: PW no fault checl Timer 2 
        SETBIT(T2CONbits.TON);      //Turn timer 3 on 
    }
    else if (servo == 'Y'){
    
        CLEARBIT(TRISDbits.TRISD6); //Set OC7 as output
        OC7R=init_ticks;                  //set initial duty cycle to 5ms 
        OC7RS=init_ticks;                 //Load OCRS: next pwm duty cycle 
        OC7CON= 0x0006;              //set OC8: PW no fault check Timer 2 
        SETBIT(T2CONbits.TON);      //Turn timer 3 on 
    
    
    
    }
   



}


void set_servo(char servo, uint16_t pulse){
   
    
    if (pulse <= PWM_MIN_US){
        pulse=PWM_MIN_US;
    }
    else if (pulse>= PWM_MAX_US){
    
        pulse=PWM_MAX_US;
    }
    
    uint16_t ticks=  pulse/(TICK_US); 
    uint16_t  period_ticks= PR2;
    uint16_t oc_ticks= period_ticks-ticks;
  
    
      //Setup OC8
    if (servo == 'X'){
       
                          //set initial duty cycle to 5ms 
        OC8RS=oc_ticks;                 //Load OCRS: next pwm duty cycle 
    }
    else if (servo == 'Y'){
    
        
                         //set initial duty cycle to 5ms 
        OC7RS=oc_ticks;                 //Load OCRS: next pwm duty cycle 
        

    
    }



}


void init_touch(){

// Set up the I/O pins E1, E2, E3 to be output pins
CLEARBIT(TRISEbits.TRISE1); // I/O pin set to output
CLEARBIT(TRISEbits.TRISE2); // I/O pin set to output
CLEARBIT(TRISEbits.TRISE3); // I/O pin set to output
//floating
SETBIT(PORTEbits.RE1);
SETBIT(PORTEbits.RE2);
CLEARBIT(PORTEbits.RE3);


// Disable ADC
CLEARBIT(AD1CON1bits.ADON);

//initialize PIN AN15
SETBIT(TRISBbits.TRISB15); // Set TRISE RB15 to input
CLEARBIT(AD1PCFGLbits.PCFG15); // Set AD1 AN15 input pin as analog

//initialize PIN AN9
SETBIT(TRISBbits.TRISB9); // Set TRISE RB9 to input
CLEARBIT(AD1PCFGLbits.PCFG9); // Set AD1 AN9 input pin as analog


//Configure AD1CON1
CLEARBIT(AD1CON1bits.AD12B); // Set 10b Operation Mode
AD1CON1bits.FORM = 0; // Set integer output
AD1CON1bits.SSRC = 0x7; // Set automatic conversion

// Configure AD1CON2
AD1CON2 = 0; // Not using scanning sampling

//Configure AD1CON3
CLEARBIT(AD1CON3bits.ADRC); // Internal clock source
AD1CON3bits.SAMC = 0x1F; // Sample-to-conversion clock = 31Tad
AD1CON3bits.ADCS = 0x2; // Tad = 3Tcy (Time cycles)

// Leave AD1CON4 at its default value
// Enable ADC
SETBIT(AD1CON1bits.ADON);



}


void set_touch(char dim){



    
    if (dim =='X'){
        CLEARBIT(PORTEbits.RE1);
        SETBIT(PORTEbits.RE2);
        SETBIT(PORTEbits.RE3);
        
    
    }



    else if (dim == 'Y'){
        SETBIT(PORTEbits.RE1);
        CLEARBIT(PORTEbits.RE2);
        CLEARBIT(PORTEbits.RE3);
        
    
    }
    else{
           
        SETBIT(PORTEbits.RE1);
        SETBIT(PORTEbits.RE2);
        CLEARBIT(PORTEbits.RE3);
    
    
    }

}


uint16_t read_touch (){
    
    __delay_ms(10);
    AD1CON1bits.SAMP = 1;
    __delay_ms(10);
    AD1CON1bits.SAMP = 0;
    while(!AD1CON1bits.DONE);
    
    AD1CON1bits.DONE=0;
    
    return (uint16_t) ADC1BUF0;
    
    


}


void print_position(){
  // Read X
    
    set_touch('X');
    __delay_ms(10);
    AD1CHS0bits.CH0SA= 15;
    uint16_t x = read_touch();   // ensure your read_touch selects AN15 for X
   
    // Read Y
    set_touch('Y');
    __delay_ms(10);
    AD1CHS0bits.CH0SA=9;
    uint16_t y = read_touch();   // ensure your read_touch selects AN9 for Y

    // Print fixed locations so the LCD updates in place (no scrolling)
    lcd_locate(0, 4);               // choose rows that do not overwrite your header
    lcd_printf("X=%4u    ", x);     // horizontal
    lcd_locate(0, 5);
    lcd_printf("Y=%4u    ", y);     // vertical
}






