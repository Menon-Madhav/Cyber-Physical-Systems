#include "lab06.h"

#include <xc.h>
//do not change the order of the following 2 definitions
#define FCY 12800000UL
#include <libpic30.h>
#include <stdint.h>

#include <math.h>

#include "types.h"
#include "lcd.h"
#include "led.h"
#include <stdbool.h>


/*
 * Parameter
 */


/*
 * Common Definitions
 */
#define TCKPS_1   0x00
#define TCKPS_8   0x01
#define TCKPS_64  0x02
#define TCKPS_256 0x03

#define TICK_US 5  // it takes 5us for each tick  1/clock_frequency 


#define US_to_ticks(us) ((uint16_t)((us)/ TICK_US))
#define us_frequency 10000 
#define A1 1
#define A2 -0.6796
#define B1 0.1602
#define B2 0.1602
#define Ts_control 0.02f //50 Hz control
#define SERVO_CENTER_X 1579.0f
#define SERVO_CENTER_Y 1535.0f 
#define PWM_MIN_US 1000
#define PWM_MAX_US 2000
#define PWM_CYC_US 20000
#define CIRCLE_CENTER_X 370.0f
#define CIRCLE_CENTER_Y 380.0f
#define CIRCLE_RADIUS 0.0f
#define CIRCLE_HZ   0.2f //revolution per second 



/*
 * Global Variables
 */
int count=0;
volatile bool flagReadtouch= false ;
volatile bool flagControl= false ;
volatile bool flagPrint= false ;
volatile bool xyReady=false;

volatile uint32_t deadlinemisses=0;
volatile bool execBusy =false ;

void timer_initialize();
void init_servo(char servo);
void set_servo(char servo, uint16_t pulse);
void init_touch();
void set_touch(char dim);
uint16_t read_touch ();
void read_position();

static uint8_t primed=0;
static uint8_t nextIsX=1;

volatile uint16_t rawX=0;
volatile uint16_t rawY=0;
double xfilt=0, yfilt=0;
double x1X=0;
double y1X=0;
double x1Y=0;
double y1Y=0;

//gain and previous errors 

float KpX=0.001f, KdX=0.01f;
float KpY=0.001f, KdY=0.01f;

static double ePrevX=0.0f;
static double ePrevY=0.0f;

double xRef=0.0f;
double yRef=0.0f;

static float t=0.0f;
float emag=0.0f;
float pulseX=0.0f;
float pulseY=0.0f;
double eX=0.0;
double eY=0.0;


/*
 * Timer Code
 */

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

// ---- Timer2: PWM base 20 ms, NO interrupt ----
    CLEARBIT(T2CONbits.TON);
    CLEARBIT(T2CONbits.TCS);
    CLEARBIT(T2CONbits.TGATE);
    T2CONbits.TCKPS = 0b10; // 1:64 => 5us tick
    TMR2 = 0;
    PR2 = US_to_ticks(PWM_CYC_US) - 1u;  // 20ms -> 3999
    CLEARBIT(IFS0bits.T2IF);
    CLEARBIT(IEC0bits.T2IE); // no T2 interrupt
    SETBIT(T2CONbits.TON);

    // ---- Timer3: scheduler 10 ms interrupt ----
    CLEARBIT(T3CONbits.TON);
    CLEARBIT(T3CONbits.TCS);
    CLEARBIT(T3CONbits.TGATE);
    T3CONbits.TCKPS = 0b10;// 1:64 => 5us tick
    TMR3 = 0;
    PR3 = 2000; // 10ms -> 1999
    CLEARBIT(IFS0bits.T3IF);
    SETBIT(IEC0bits.T3IE); // enable T3 interrupt
    SETBIT(T3CONbits.TON);

    __builtin_enable_interrupts();
}


/*
 * Servo Code
 */

void init_servo(char servo){
    
    
    
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


/*
 * Touch screen code
 */
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
        Nop();
        SETBIT(PORTEbits.RE2);
        Nop();
        SETBIT(PORTEbits.RE3);
        
    
    }



    else if (dim == 'Y'){
        SETBIT(PORTEbits.RE1);
        Nop();
        CLEARBIT(PORTEbits.RE2);
        Nop();
        CLEARBIT(PORTEbits.RE3);
        
    
    }
    else{
           
        SETBIT(PORTEbits.RE1);
        Nop();
        SETBIT(PORTEbits.RE2);
        Nop();
        CLEARBIT(PORTEbits.RE3);
    
    
    }

}


uint16_t read_touch (){
    
    AD1CON1bits.SAMP = 1;
    __delay_us(50);
    AD1CON1bits.SAMP = 0;
    while(!AD1CON1bits.DONE);
    
    AD1CON1bits.DONE=0;
    
    return (uint16_t) ADC1BUF0;

}



/*
 * PD Controller
 */
static inline float clamp_f(float v, float lo, float hi){

    if (v<lo) return lo;
    if (v>hi) return hi; 
    return v;
}

void Setpoint_50Hz(void){

    t+= Ts_control;
    float w =2.0f *3.1415926f* CIRCLE_HZ;
    xRef=CIRCLE_CENTER_X+CIRCLE_RADIUS*cosf(w*t);
    yRef=CIRCLE_CENTER_Y+CIRCLE_RADIUS*sinf(w*t);

}

void Control(){
    
     eX=xRef-xfilt;
     eY=yRef-yfilt;
    
    double dEX=(filter(eX, 'X')-ePrevX)/Ts_control;
    double dEY=(filter(eY, 'Y')-ePrevY)/Ts_control;
    
    ePrevX=eX;
    ePrevY=eY;
    
    double uX=KpX*eX+KdX*dEX;
    double uY=KpY*eY+KdY*dEY;
    
    uX=clamp_f(uX, -200.0f, 200.0f);
    uY=clamp_f(uY, -200.0f, 200.0f);
    
    pulseX=SERVO_CENTER_X + uX;
    pulseY=SERVO_CENTER_Y + uY;
    

    
    
    set_servo('X',(uint16_t)pulseX);
    set_servo('Y', (uint16_t)pulseY);
    emag=sqrt(eX*eX+eY*eY);
    
}




/*
 * Butterworth Filter N=1, Cutoff 3 Hz, sampling @ 50 Hz
 */

double filter (double x, char channel){

    double y;
    
    if (channel == 'X'){
    
        y=B1*x+ B2*x1X-A2*y1X;
        x1X=x;
        y1X= y;
    }
    else if (channel=='Y'){
    
        y=B1*x+ B2*x1Y-A2*y1Y;
        x1Y=x;
        y1Y= y;
    }
    
    return y;

}


/*
 * main loop
 */
void main_loop()
{
    // print assignment information
    lcd_printf("Lab06: Amazing Ball");
    lcd_locate(0, 1);
    lcd_printf("Group: Group 4");
    
    timer_initialize();
    // initialize touchscreen
    init_touch();
    // initialize servos
    init_servo('X');
    init_servo('Y');
   
    while(TRUE) {
        
      
        
     
        
        if (flagReadtouch==TRUE){
            flagReadtouch=false;
            read_position();
        
        }
        
        
        if (xyReady){
        
            flagControl=false;
            xyReady=false;
            Setpoint_50Hz();
            Control();
            //set_servo('X',SERVO_CENTER_X);
            //set_servo('Y',SERVO_CENTER_Y);
        }
        if (flagPrint){
            flagPrint= false ;
            Print_5Hz();
       
        }
        

       
        execBusy=false;
       
       
        
        
        
    }
}


void __attribute__((__interrupt__, __shadow__, __auto_psv__)) _T3Interrupt(void)
{
    
   CLEARBIT(IFS0bits.T3IF);
  
  
   //deadline miss check
  
  
   
   count++;
   flagReadtouch=true;
   
   if (count%2==0){
       flagControl=true;
   
   
   }
   
   if (count%20==0){
       flagPrint=true;
   
       
   }
    if(execBusy){
       deadlinemisses++;
   }
   execBusy=true;
 
    
    
}


void read_position(){
    
    
    // Read X
    if (primed==0){
    
        set_touch('X');
        AD1CHS0bits.CH0SA= 15;
        primed =1;
        nextIsX=1;
        return;
    }
    
    if (nextIsX==1){
        
        
        rawX=read_touch();
        xfilt=filter(rawX,'X');
        
        
        set_touch('Y');
        AD1CHS0bits.CH0SA=9;
        nextIsX=0;
        
        return;
  
    }
    
    else {
        rawY=read_touch();
        yfilt=filter(rawY, 'Y');
        
        xyReady=true;
        
        set_touch('X');
        AD1CHS0bits.CH0SA= 15;
       
        nextIsX=1;
        return;
    
    
    }

   

}







void Print_5Hz(){
  lcd_locate(0,0);
    

   
    lcd_locate(0,3);
    lcd_printf("Miss:%lu      ", (unsigned long)deadlinemisses);
    lcd_locate(0,4);
    
    lcd_printf("xr:%4u      ", rawX);
    lcd_printf("xf:%4d", (int)xfilt);
    
    lcd_locate(0,6);
    
    lcd_printf("eX:%5d     ", (int)eX);
    lcd_printf("eY:%5d      ", (int)eY);
    



}