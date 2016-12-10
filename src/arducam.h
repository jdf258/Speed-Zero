#ifndef __ARDUCAM_H__
#define __ARDUCAM_H__

#include <plib.h>
#include <stdint.h>


// Arducam module-specifi interface
#define ARDUCHIP_TEST_ADDR 0x00
#define ARDUCHIP_TEST_VALUE 0x55

#define ARDUCHIP_FIFO 0x04
#define ARDUCHIP_TRIG 0x41

#define FIFO_CLEAR_MASK 0x01
#define FIFO_START_MASK 0x02
#define SINGLE_FIFO_READ 0x3D
#define FIFO_SIZE1 0x42
#define FIFO_SIZE2 0x43
#define FIFO_SIZE3 0x44
#define BURST_FIFO_READ 0x3C
#define CAP_DONE_MASK 0x08


// SPI peripheral config
#define SPI_PRESCALER PRI_PRESCAL_64_1
#define CS_PORT IOPORT_A
#define CS_PIN BIT_3
#define SPI_OPEN(config1, config2) OpenSPI2(config1, config2)
#define SPI_WRITE(value) WriteSPI2(value)
#define SPI_READ() ReadSPI2();
#define SPI_BUSY_FLAG SPI2STATbits.SPIBUSY


// SPI protocol config
#define READ_CMD(addr) addr & 0x7F
#define WRITE_CMD(addr) addr | 0x80


// Application Interface
void arducam_init();
void arducam_start_capture();
uint8_t arducam_capture_done();
void arducam_wifi_send();



#endif
