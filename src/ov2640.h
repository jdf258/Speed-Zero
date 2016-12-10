#ifndef __OV2640_H__
#define __OV2640_H__

#include <stdint.h>

typedef struct Sensor_Reg {
  uint8_t addr;
  uint8_t value;
} Sensor_Reg;

typedef enum  {
  OV2640_160x120_JPEG = 0,
  OV2640_176x144_JPEG,
  OV2640_320x240_JPEG,
  OV2640_352x288_JPEG,
  OV2640_640x480_JPEG,
  OV2640_800x600_JPEG,
  OV2640_1024x768_JPEG,
  OV2640_1280x1024_JPEG,
  OV2640_1600x1200_JPEG
} OV2640_JPEG_Size;

#define OV2640_DEFAULT_SIZE OV2640_160x120_JPEG

// max i2c badrate 400k
#define I2C_BAUDRATE 200000


// I2C helpers
#define I2C_OPEN(config, brg) OpenI2C1(config, brg)
#define STARTI2C() StartI2C1()
#define STOPI2C() StopI2C1()
#define RESTARTI2C() RestartI2C1()
#define IDLEI2C() IdleI2C1()
#define WRITEI2C(data) MasterWriteI2C1(data)
#define READI2C() MasterReadI2C1()
#define I2CACKSTAT I2C1STATbits.ACKSTAT


void ov2640_init();

void ov2640_set_jpeg_size(OV2640_JPEG_Size size);

#endif
