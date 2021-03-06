#ifndef IMU_H
#define	IMU_H

#include <xc.h>     // processor SFR definitions

#define IMU_WHOAMI      0x0F
#define IMU_ADDR        0b11010110
#define IMU_CTRL1_XL    0x10
#define IMU_CTRL2_G     0x11
#define IMU_CTRL3_C     0x12
#define IMU_OUT_TEMP_L  0x20

void imu_setup();
void imu_read(unsigned char address, unsigned char reg, signed short * dataShort, int len);

#endif