#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include<math.h>
#include "font.h"
#include "ssd1306.h"
#include "ws2812b.h"
#include "i2c_master_noint.h"
#include "imu.h"

// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // disable secondary oscillator
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
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 0;
    TRISBbits.TRISB4 = 1;

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
    
    // for counting
     int i;

    while(1){
        // change LED for heartbeat
        _CP0_SET_COUNT(0);
        LATAbits.LATA4 = !LATAbits.LATA4;
        
        // right capacitor
        int capR[10];
        int capR_sum = 0;
        char capR_message[50];
        for(i=0;i<10;i++){
            capR[i] = ctmu_read(5,50);
            capR_sum = capR_sum+capR[i];
        }
        sprintf(capR_message,"%d",capR_sum);
        ssd1306_drawMessage(0,8,capR_message);

        // left capacitor
        int capL[10];
        int capL_sum = 0;
        char capL_message[50];
        for(i=0;i<10;i++){
            capL[i] = ctmu_read(4,50);
            capL_sum = capL_sum+capL[i];
        }
        sprintf(capL_message,"%d",capL_sum);
        ssd1306_drawMessage(0,0,capL_message);

        // show on ssd
        ssd1306_update();
        ssd1306_clear();

        // turn on light if capacitance drops from touch
        if(capL_sum > 5000){
            color[0] = HSBtoRGB(120,0,0);
        }
        else{
            color[0] = HSBtoRGB(120,1,1);
        }

        if(capR_sum > 5000){
            color[1] = HSBtoRGB(120,0,0);
        }
        else{
            color[1] = HSBtoRGB(120,1,1);
        }

        // capacitance drops even more when touching middle ground
        if(capR_sum < 1000 || capL_sum < 1000){
          color[3] = HSBtoRGB(50,1,1);
        }
        else{
            color[3] = HSBtoRGB(0,0,0);
        }

        ws2812b_setColor(color,4);

        //delay
        _CP0_SET_COUNT(0);
        while (_CP0_GET_COUNT() < 24000000/100) {}

    }
}
