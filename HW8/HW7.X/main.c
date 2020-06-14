#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include<math.h>
#include "font.h"
#include "ssd1306.h"
#include "ws2812b.h"
#include "i2c_master_noint.h"
#include "imu.h"
#include "rtcc.h"

// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = ON // disable secondary oscillator
#pragma config IESO = OFF // disable switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // disable clock output
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // disable clock switch and FSCM
#pragma config WDTPS = PS1048576 // use largest wdt
#pragma config WINDIS = OFF // use non-window mode wdt
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations

int main() {

    __builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    TRISBbits.TRISB5 = 0;
    LATBbits.LATB5 = 0;
    //TRISBbits.TRISB4 = 1;

    // initialize
    i2c_master_setup();
    ssd1306_setup();
    adc_setup();
    ws2812b_setup();
    ctmu_setup();

     __builtin_enable_interrupts();

    // color variables
    wsColor color[4];
    color[2] = HSBtoRGB(0,0,0);
    
    // setup RTCC
    unsigned long time_init = 0x12000000;
    unsigned long date_init = 0x20061400;
    rtcc_setup(time_init, date_init);
    
    // time & date struct for RTCC data
    rtccTime time_date;
    
    
    
    // for counting
     int display_count = 0;

    // display pointers
     char msg[50];
     char day_msg[50];
    while(1){
        // change LED for heartbeat
        LATBbits.LATB5 = !LATBbits.LATB5;
        
        // clear previous ssd
        ssd1306_clear();
        
        // read from RTCC
        time_date = readRTCC();
        dayOfTheWeek(time_date.wk, day_msg);
        
        sprintf(msg, "Hi! %d", display_count);
        ssd1306_drawMessage(0,0,msg);
        sprintf(msg, "%d%d:%d%d:%d%d", time_date.hr10, time_date.hr01, //...
           time_date.min10, time_date.min01, time_date.sec10, time_date.sec01);
        ssd1306_drawMessage(10,12,msg);
        sprintf(msg, "%s, %d%d/%d%d/20%d%d", day_msg, time_date.mn10, //...
           time_date.mn01, time_date.dy10, time_date.dy01, time_date.yr10, time_date.yr01);
        ssd1306_drawMessage(10,24,msg);
        
        // show on ssd
        ssd1306_update();
       
        //delay half a second
        _CP0_SET_COUNT(0);
        while (_CP0_GET_COUNT() < 24000000/2) {}
        display_count++;
    }
}
