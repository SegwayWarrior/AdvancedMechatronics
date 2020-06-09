#include "imu.h"
#include "i2c_master_noint.h"
#include "ssd1306.h"

void imu_setup(){
    unsigned char who = 0;
    // read from IMU_WHOAMI
    who = i2c_read_pin(IMU_ADDR,IMU_WHOAMI);
    
    // init IMU_CTRL1_XL
    i2c_set_pin(IMU_ADDR,IMU_CTRL1_XL,0b10000010);
    // init IMU_CTRL2_G
    i2c_set_pin(IMU_ADDR,IMU_CTRL2_G,0b10001000);
    // init IMU_CTRL3_C
    i2c_set_pin(IMU_ADDR,IMU_CTRL3_C,0b00000100);
}

void imu_read(unsigned char reg, signed short * data, int len){
    unsigned char i2c_data[2*len];
    int i,j;
    // read multiple from the imu, each data takes 2 reads so you need len*2 chars
    i2c_read_multiple(IMU_ADDR,reg,i2c_data,2*len);
    // turn the chars into the shorts
    for(i=2*len-1,j=len-1;i>0;i=i-2,j--){
        data[j] = (i2c_data[i]<<8)|i2c_data[i-1];
    }
}

void imu_bar_x(signed short x, int val){
    int i;
    x = 64*x/16383;
    if(x >= 0){
        for(i=0;i<x;i++){
            ssd1306_drawPixel(64+i,16,val);
        }
    }
    else{
        for(i=0;i>x;i--){
            ssd1306_drawPixel(64+i,16,val);
        }
    }
}

void imu_bar_y(signed short y, int val){
    int i;
    y = 16*y/16383;
    if(y >= 0){
        for(i=0;i<y;i++){
            ssd1306_drawPixel(64,16+i,val);
        }
    }
    else{
        for(i=0;i>y;i--){
            ssd1306_drawPixel(64,16+i,val);
        }
    }
}