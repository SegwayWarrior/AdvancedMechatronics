 #include<xc.h>           // processor SFR definitions
 #include<sys/attribs.h>  // __ISR macro
 #include<math.h>
 #include"spi.h"

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
 #pragma config FPLLIDIV = DIV_2// divide input clock to be in range 4-5MHz
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

 ////// do your TRIS and LAT commands here///////////////////////////////////////
     TRISAbits.TRISA1 = 0;
     TRISBbits.TRISB4 = 1;
     //
     LATAbits.LATA1 = 0;
     
     initSPI();

     __builtin_enable_interrupts();



    // Make a sine wave
    int sine_wave[50]; // Will make two cycles at 100 hz (2Hz wave)
    int sine_amp = 1000, sine_avg = 1500;
    int i = 0;
    for (i; i < 50; ++i){
        sine_wave[i] = sine_amp*sin((2*3.14/50)*i) + sine_avg;
    }

    // Make a triangle wave
    int tri_wave[100]; // Will make one cycle at 100 hz (1Hz wave)
        
    int j = 0, tri_min = 1000, tri_max = 3000;
    for (j = 0; j < 100; ++j){
        if (j < 50){
        tri_wave[j] = (((tri_max - tri_min)*j)/50) + tri_min;
        } else {
            tri_wave[j] = tri_max - ((tri_max - tri_min)*(j-50))/50;
        }
    }
    

     int clock_count = 0;
     unsigned short sine_DAC_cmd = 0, tri_DAC_cmd = 0;

     while (1) {
 //         use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
 //         remember the core timer runs at half the sysclk

       _CP0_SET_COUNT(0);

       if (clock_count < 50){
         sine_DAC_cmd = 0b0111<<12 | sine_wave[clock_count]; // constants than voltage
       }
       else{
         sine_DAC_cmd = 0b0111<<12 | sine_wave[clock_count-50];
       }

       tri_DAC_cmd = 0b1111<<12 | tri_wave[clock_count];

       clock_count ++;
       if (clock_count > 100){
         clock_count = 0;
       }

       LATAbits.LATA0 = 0; //set CS low to communicate
       spi_io(sine_DAC_cmd>>8); // send first half of spi signal
       spi_io(sine_DAC_cmd); // self truncating
       LATAbits.LATA0 = 1; //set CS to not communicating

       LATAbits.LATA0 = 0; //set CS to low again for b signal
       spi_io(tri_DAC_cmd>>8);
       spi_io(tri_DAC_cmd);
       LATAbits.LATA0 = 1;

       while (_CP0_GET_COUNT()<(48000000/2)/100){} // wait 1/100 seconds
     }

 }