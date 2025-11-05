#include <xc.h>
#include <p33Fxxxx.h>
//do not change the order of the following 2 definitions
#define FCY 12800000UL 
#include <libpic30.h>

#include "lcd.h"
#include "led.h"

/* Configuration of the Chip */
// Initial Oscillator Source Selection = Primary (XT, HS, EC) Oscillator with PLL
#pragma config FNOSC = PRIPLL
// Primary Oscillator Mode Select = XT Crystal Oscillator mode
#pragma config POSCMD = XT
// Watchdog Timer Enable = Watchdog Timer enabled/disabled by user software
// (LPRC can be disabled by clearing the SWDTEN bit in the RCON register)
#pragma config FWDTEN = OFF

void delay(){
    uint32_t a = 1;
    while(a++){
       if(a>500000) break;       
    }
};

int main(){
    //Init LCD and LEDs
    lcd_initialize();
    led_init();
    
    //Task 1.1 
    lcd_clear(); 
    lcd_locate(0, 0);
   
    lcd_printf("Maryam Mirabdollahyani");
    lcd_locate(0, 1);
    lcd_printf("Madhav Menon");
    lcd_locate(0, 2);
    lcd_printf("Yue Jiang");
    lcd_locate(0, 3);
    //task1.2 & task 1.3 
    int t = 0;
    int x1, x2, x3, x4, x5;
    
    while(1){
        CLEARLED(LED1_PORT);
        CLEARLED(LED2_PORT);
        CLEARLED(LED3_PORT);
        CLEARLED(LED4_PORT);
        CLEARLED(LED5_PORT);
      // Deci to Binary 
        x1 = t & 1;
        x2 = t & 2;
        x3 = t & 4;
        x4 = t & 8;
        x5 = t & 16;
        
        delay();

        if(x1){SETLED(LED5_PORT);
        }
        if(x2){SETLED(LED4_PORT);
        }
        if(x3){SETLED(LED3_PORT);
        }
        if(x4){SETLED(LED2_PORT);
        }
        if(x5){SETLED(LED1_PORT);
        }
        
        //Counter print 
        lcd_locate(0, 3);
        lcd_printf("%d", t++);
        
        delay();

    };
}