#include "dac.h"
#include "types.h"

// tristate register
#define DAC_CS_TRIS TRISDbits.TRISD8
#define DAC_SDI_TRIS TRISBbits.TRISB10
#define DAC_SCK_TRIS TRISBbits.TRISB11
#define DAC_LDAC_TRIS TRISBbits.TRISB13

// port register
#define DAC_CS_PORT PORTDbits.RD8
#define DAC_SDI_PORT PORTBbits.RB10
#define DAC_SCK_PORT PORTBbits.RB11
#define DAC_LDAC_PORT PORTBbits.RB13

// analog to digital converter 1 port configuration register
#define DAC_SDI_AD1CFG AD1PCFGLbits.PCFG10
#define DAC_SCK_AD1CFG AD1PCFGLbits.PCFG11
#define DAC_LDAC_AD1CFG AD1PCFGLbits.PCFG13

// analog to digital converter 2 port configuration register
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

void dac_convert_milli_volt(uint16_t milliVolt)
{
    
}
