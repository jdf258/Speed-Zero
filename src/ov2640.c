#include "ov2640.h"
#include "delay.h"
#include "main.h"
#include "wifi.h"
#include "debug.h"

// Minor Todo
// + BMP
// + concerns in ov2640_setup

// baudrate generator
const unsigned int BRG = (PB_CLOCK / 2 / I2C_BAUDRATE) - 2;

#define OV2640_I2C_ADDR 0x60

#define OV2640_CHIPID_ADDR 0xFF
#define OV2640_CHIPID_VAL 0x01
#define OV2640_CHIPID_HIGH 0x0A
#define OV2640_CHIPID_LOW 0x0B
#define OV2640_CHIPID_VID 0x26
#define OV2640_CHIPID_PID 0x42


extern const Sensor_Reg OV2640_QVGA_REGS[];
extern const Sensor_Reg OV2640_JPEG_INIT_REGS[];
extern const Sensor_Reg OV2640_YUV422_REGS[];
extern const Sensor_Reg OV2640_JPEG_REGS[];
extern const Sensor_Reg OV2640_160x120_JPEG_REGS[];
extern const Sensor_Reg OV2640_176x144_JPEG_REGS[];
extern const Sensor_Reg OV2640_320x240_JPEG_REGS[];
extern const Sensor_Reg OV2640_352x288_JPEG_REGS[];
extern const Sensor_Reg OV2640_640x480_JPEG_REGS[];
extern const Sensor_Reg OV2640_800x600_JPEG_REGS[];
extern const Sensor_Reg OV2640_1024x768_JPEG_REGS[];
extern const Sensor_Reg OV2640_1280x1024_JPEG_REGS[];
extern const Sensor_Reg OV2640_1600x1200_JPEG_REGS[];

const Sensor_Reg *ov2640_jpeg_sizes[] = {
  OV2640_160x120_JPEG_REGS,
  OV2640_176x144_JPEG_REGS,
  OV2640_320x240_JPEG_REGS,
  OV2640_352x288_JPEG_REGS,
  OV2640_640x480_JPEG_REGS,
  OV2640_800x600_JPEG_REGS,
  OV2640_1024x768_JPEG_REGS,
  OV2640_1280x1024_JPEG_REGS,
  OV2640_1600x1200_JPEG_REGS
};


void ov2640_i2c_write(uint8_t addr, uint8_t value) {
  STARTI2C();
  IDLEI2C();

  WRITEI2C(OV2640_I2C_ADDR);
  IDLEI2C();
  if(I2CACKSTAT)
    wifi_send_debug("FAILED:ov2640:i2c_write:1st ack");

  WRITEI2C(addr);
  IDLEI2C();
  if(I2CACKSTAT)
    wifi_send_debug("FAILED:ov2640:i2c_write:2nd ack");

	WRITEI2C(value);
  IDLEI2C();
  if(I2CACKSTAT)
    wifi_send_debug("FAILED:ov2640:i2c_write:3rd ack");

  STOPI2C();
  IDLEI2C();
}


uint8_t ov2640_i2c_read(uint8_t addr) {
  uint8_t value;

  STARTI2C();
  IDLEI2C();

  WRITEI2C(OV2640_I2C_ADDR);
  IDLEI2C();
  if(I2CACKSTAT)
    wifi_send_debug("FAILED:ov2640:i2c_read:1st ack");

  WRITEI2C(addr);
  IDLEI2C();
  if(I2CACKSTAT)
    wifi_send_debug("FAILED:ov2640:i2c_read:2nd ack");

  RESTARTI2C();
  IDLEI2C();

  WRITEI2C(OV2640_I2C_ADDR | 1);
  IDLEI2C();
  if(I2CACKSTAT)
    wifi_send_debug("FAILED:ov2640:i2c_read:3rd ack");

  value = READI2C();
  IDLEI2C();
  NotAckI2C1();
  //if(!I2CACKSTAT)
  //  wifi_send_debug("FAILED:ov2640:i2c_read:4th ack");

  STOPI2C();
  IDLEI2C();

  return value;
}


uint8_t ov2640_i2c_writes(const Sensor_Reg *regs) {
  uint16_t index = 0;
  uint8_t addr = 0;
  uint8_t value = 0;

  while((addr != 0xFF) || (value != 0xFF)) {
    addr = regs[index].addr;
    value = regs[index++].value;
    ov2640_i2c_write(addr, value);
  }
}


void ov2640_set_jpeg_size(OV2640_JPEG_Size size) {
  ov2640_i2c_writes(ov2640_jpeg_sizes[size]);
}

uint8_t ov2640_verify() {
  uint8_t vid = 0;
  uint8_t pid = 0;

  ov2640_i2c_write(OV2640_CHIPID_ADDR, OV2640_CHIPID_VAL);
  vid = ov2640_i2c_read(OV2640_CHIPID_HIGH);
  pid = ov2640_i2c_read(OV2640_CHIPID_LOW);
  if((vid != OV2640_CHIPID_VID) || (pid != OV2640_CHIPID_PID)) {
    sprintf(wifi_debug_buffer, "FAILED:ov2640:verify:vid[%X,%X],pid[%X,%X]",vid,OV2640_CHIPID_VID,pid,OV2640_CHIPID_PID);
    wifi_send_debug(wifi_debug_buffer);
    return 0;
  } else {
    wifi_send_debug("PASSED:ov2640:verify:sensor id confirmed");
    return 1;
  }
}


void ov2640_init() {
  I2C_OPEN(I2C_EN, BRG);  

  if(!ov2640_verify())
    debug_error_handler();

  ov2640_i2c_write(0xFF, 0x01);
  ov2640_i2c_write(0x12, 0x80);
  delay(100);
  // JPEG
  ov2640_i2c_writes(OV2640_JPEG_INIT_REGS);
  ov2640_i2c_writes(OV2640_YUV422_REGS);
  ov2640_i2c_writes(OV2640_JPEG_REGS);
  ov2640_i2c_write(0xFF, 0x01);
  ov2640_i2c_write(0x15, 0x00);
  ov2640_set_jpeg_size(OV2640_DEFAULT_SIZE);
  // TODO if not jpeg
  // ov2640_i2c_writes(OV2640_QVGA);

  delay(1000);
}
