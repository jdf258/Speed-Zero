#include "arducam.h"
#include "ov2640.h"
#include "wifi.h"
#include "wifi_interface.h"
#include "debug.h"
#include "delay.h"

// TODO application level (see below arducam_set_fifo_burst in .h)



#define ARDUCAM_WIFI_BUFFER_LENGTH 200
uint8_t arducam_wifi_buffer[ARDUCAM_WIFI_BUFFER_LENGTH] = {0};
Wifi_Packet arducam_wifi_packet;

uint8_t arducam_spi_transfer(uint8_t data) {
  SPI_WRITE(data);
  while(SPI_BUSY_FLAG);
  return SPI_READ();
}

void arducam_spi_write(uint8_t addr, uint8_t data) {
  PORTClearBits(CS_PORT, CS_PIN);
  SPI_WRITE(WRITE_CMD(addr));
  while(SPI_BUSY_FLAG);
  SPI_READ();
  SPI_WRITE(data);
  while(SPI_BUSY_FLAG);
  SPI_READ();
  PORTSetBits(CS_PORT, CS_PIN);
}


uint8_t arducam_spi_read(uint8_t addr) {
  uint8_t temp = 0xEE;
  PORTClearBits(CS_PORT, CS_PIN);
  SPI_WRITE(READ_CMD(addr));
  while(SPI_BUSY_FLAG);
  SPI_READ();
  SPI_WRITE(temp); // dummy data
  while(SPI_BUSY_FLAG);
  temp = SPI_READ();
  PORTSetBits(CS_PORT, CS_PIN);

  return temp;
}

void arducam_spi_set_bit(uint8_t addr, uint8_t bit) {
  uint8_t temp;
  temp = arducam_spi_read(addr);
  arducam_spi_write(addr, temp | bit);
}


void arducam_spi_clear_bit(uint8_t addr, uint8_t bit) {
  uint8_t temp;
  temp = arducam_spi_read(addr);
  arducam_spi_write(addr, temp ^ bit);}


uint8_t arducam_spi_get_bit(uint8_t addr, uint8_t bit) {
  uint8_t temp;
  temp = arducam_spi_read(addr);
  return temp & bit;
}

void arducam_flush_fifo(void) {
  arducam_spi_write(ARDUCHIP_FIFO, FIFO_CLEAR_MASK);
}

void arducam_set_capture(void) {
  arducam_spi_write(ARDUCHIP_FIFO, FIFO_START_MASK);
}

void arducam_clear_fifo_flag(void) {
  arducam_spi_write(ARDUCHIP_FIFO, FIFO_CLEAR_MASK);
}

uint8_t arducam_read_fifo(void) {
  return arducam_spi_read(SINGLE_FIFO_READ);
}

uint32_t arducam_read_fifo_length(void) {
  uint32_t length = 0;
  length = arducam_spi_read(FIFO_SIZE1);
  length |= (arducam_spi_read(FIFO_SIZE2) << 8);
  length |= (arducam_spi_read(FIFO_SIZE3) << 16);
  length &= 0x07fffff;
  return length;
}

void arducam_set_fifo_burst() {
  arducam_spi_transfer(BURST_FIFO_READ);
}


uint8_t arducam_verify() {
  uint8_t test;
  arducam_spi_write(ARDUCHIP_TEST_ADDR, ARDUCHIP_TEST_VALUE);
  test = arducam_spi_read(ARDUCHIP_TEST_ADDR);
  if(test != ARDUCHIP_TEST_VALUE) {
    sprintf(wifi_debug_buffer, "FAILED:Arducam:Setup:TEST_VALUE[%X,%X]",
            test, ARDUCHIP_TEST_VALUE);
    wifi_send_debug(wifi_debug_buffer);
    return 0;
  } else {
    wifi_send_debug("PASSED:Arducam:Setup:TEST_VALUE");
    return 1;
  }
}

void arducam_init() {
  ov2640_init();

  arducam_wifi_packet.capacity = ARDUCAM_WIFI_BUFFER_LENGTH;
  arducam_wifi_packet.payload = arducam_wifi_buffer;

  // SPI setup

  PPSInput(3, SDI2, RPA4);
  PPSOutput(3, RPB2, SDO2);
  PORTSetPinsDigitalOut(CS_PORT, CS_PIN);
  
  SPI_OPEN(FRAME_ENABLE_OFF | ENABLE_SDO_PIN | SPI_MODE8_ON | SLAVE_ENABLE_OFF
           | MASTER_ENABLE_ON | SPI_SMP_ON | SPI_CKE_ON | CLK_POL_ACTIVE_HIGH
           | SPI_PRESCALER, SPI_ENABLE);

  if(!arducam_verify())
    debug_error_handler();
  
  arducam_clear_fifo_flag();
}

void arducam_start_capture() {
  arducam_flush_fifo();
  arducam_clear_fifo_flag();
  arducam_set_capture();
}

uint8_t arducam_capture_done() {
  return arducam_spi_get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK);
}

void arducam_wifi_send() {
  uint32_t length = 0;
  uint8_t index = 0;
  uint8_t is_header = 0;
  uint8_t temp = 0;
  uint8_t temp_last = 0;

  arducam_wifi_packet.header = WIFI_ARDUCAM_DATA;

  length = arducam_read_fifo_length();

  PORTClearBits(CS_PORT, CS_PIN);
  arducam_set_fifo_burst();

  temp = arducam_spi_transfer(0x00);
  length--;
  while(length--) {
    temp_last = temp;
    temp = arducam_spi_transfer(0x00);
    if(is_header) {
      arducam_wifi_buffer[index++] = temp;
      if(index == ARDUCAM_WIFI_BUFFER_LENGTH) {
        arducam_wifi_packet.length = ARDUCAM_WIFI_BUFFER_LENGTH;
        wifi_send(&arducam_wifi_packet);
        index = 0;
      }
    } else if ((temp == 0xD8) & (temp_last == 0xFF)) {
      is_header = 1;
      arducam_wifi_buffer[index++] = temp_last;
      if(index == ARDUCAM_WIFI_BUFFER_LENGTH) {
        arducam_wifi_packet.length = ARDUCAM_WIFI_BUFFER_LENGTH;
        wifi_send(&arducam_wifi_packet);
        index = 0;
      }
      arducam_wifi_buffer[index++] = temp;
      if(index == ARDUCAM_WIFI_BUFFER_LENGTH) {
        arducam_wifi_packet.length = ARDUCAM_WIFI_BUFFER_LENGTH;
        wifi_send(&arducam_wifi_packet);
        index = 0;
      }
    }
    if((temp == 0xD9) && (temp_last == 0xFF))
      break;
  }
  PORTClearBits(CS_PORT, CS_PIN);
  is_header = 0;

  if(index) {
    arducam_wifi_packet.length = index;
    wifi_send(&arducam_wifi_packet);
  }

  arducam_wifi_packet.header = WIFI_ARDUCAM_END;
  arducam_wifi_packet.length = 0;
  wifi_send(&arducam_wifi_packet);

  arducam_clear_fifo_flag();
}
