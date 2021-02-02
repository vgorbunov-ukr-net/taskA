#ifndef RFM69_h
#define RFM69_h

#include "stm8l15x.h"
#include "RFM69registers.h"


#define SPI1_MISO GPIO_Pin_7
#define SPI1_MOSI GPIO_Pin_6
#define SPI1_SCK  GPIO_Pin_5
#define SPI1_NSS  GPIO_Pin_3


#define RF69_MAX_DATA_LEN       61  // to take advantage of the built in AES/CRC we want to limit the frame size to the internal FIFO size (66 bytes - 3 bytes overhead - 2 bytes crc)
#define CSMA_LIMIT             -90 // upper RX signal sensitivity threshold in dBm for carrier sense access
#define RF69_MODE_SLEEP         0   // XTAL OFF
#define RF69_MODE_STANDBY       1   // XTAL ON
#define RF69_MODE_SYNTH         2   // PLL ON
#define RF69_MODE_RX            3   // RX MODE
#define RF69_MODE_TX            4   // TX MODE
#define null                  0
#define COURSE_TEMP_COEF    -90     // puts the temperature reading in the ballpark, user can fine tune the returned value
#define RF69_BROADCAST_ADDR   0
#define RF69_CSMA_LIMIT_MS 1000
#define RF69_TX_LIMIT_MS   1000
#define RF69_FSTEP    61.03515625   // == FXOSC / 2^19 = 32MHz / 2^19 (p13 in datasheet) / FXOSC = module crystal oscillator frequency 
// TWS: define CTLbyte bits
#define RFM69_CTL_SENDACK   0x80
#define RFM69_CTL_REQACK    0x40




void spi_start();
void rfm69cw_write(uint8_t addr, uint8_t dat);
uint8_t rfm69cw_read(uint8_t addr);
void rfm69cw_multiread(uint8_t addr, uint8_t *byte, uint8_t length);
void rfm69cw_multiwrite(uint8_t addr, uint8_t *byte, uint8_t length);


void rfm69_init(uint16_t freqBand, uint8_t nodeID, uint8_t networkID);
void setAddress(uint8_t addr);
void setNetwork(uint8_t networkID);
uint32_t getFrequency();
void setFrequency(uint32_t freqHz);
void setPowerLevel(uint8_t level);            // reduce/increase transmit power level
void sleep();
void setMode(uint8_t mode);


#endif

