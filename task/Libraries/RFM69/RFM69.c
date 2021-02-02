#include "RFM69.h"



volatile uint8_t mode = RF69_MODE_STANDBY; // should be protected?
uint8_t address;                           //nodeID




void spi_start()
{
  CLK_PeripheralClockConfig(CLK_Peripheral_SPI1, ENABLE);
  SPI_DeInit(SPI1);
  SPI_Init(SPI1, SPI_FirstBit_MSB, SPI_BaudRatePrescaler_4, SPI_Mode_Master, SPI_CPOL_Low, SPI_CPHA_1Edge, SPI_Direction_2Lines_FullDuplex, SPI_NSS_Soft, 0x00);//assert_param(0u)???
  SPI_Cmd(SPI1, ENABLE);
  GPIO_SetBits(GPIOB, SPI1_NSS);
}

void rfm69cw_write(uint8_t addr, uint8_t dat)
{
    uint8_t read_data;
    GPIO_ResetBits(GPIOB, SPI1_NSS);
    addr = addr | 0x80;
    SPI_SendData(SPI1, addr);
    while( SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) ==RESET)  
    read_data = 0;//wait till Tx buffer is transferred to shift register
    read_data = (uint8_t)SPI1->DR;//dummy read previously saved data in Rx shift register
 
    
    SPI_SendData(SPI1, dat);
    while( SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) ==RESET)
    read_data = (uint8_t)SPI1->DR;
    read_data = 0;//dummy read previously saved data in Rx shift register
    read_data = (uint8_t)SPI1->DR;//dummy read previously saved data in Rx shift register
 
    GPIO_SetBits(GPIOB, SPI1_NSS);
}

void rfm69cw_multiwrite(uint8_t addr, uint8_t *dat, uint8_t length)
{
    uint8_t read_data;
    GPIO_ResetBits(GPIOB, SPI1_NSS);
    addr = addr | 0x80;
    SPI_SendData(SPI1, addr);
    while( SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) ==RESET)  
    read_data = 0;// dummy action
    read_data = (uint8_t)SPI1->DR;//dummy read previously saved data in Rx shift register
    
    uint8_t i=0;
    while(i<length)
    {
    SPI_SendData(SPI1, dat[i]);
    while( SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) ==RESET)
    read_data = 0;//dummy action 
    read_data = (uint8_t)SPI1->DR;//dummy read previously saved data in Rx shift register
    i++;
    }
    GPIO_SetBits(GPIOB, SPI1_NSS);
}


uint8_t rfm69cw_read(uint8_t addr)
{
    SPI1->SR = (SPI1->SR & (!SPI_FLAG_RXNE));//erase RXNE  flag at the beginning
    uint8_t read_data;
    GPIO_ResetBits(GPIOB, SPI1_NSS);
    SPI_SendData(SPI1, addr);
    while( SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) ==RESET)  
      read_data = 0;//wait till data is transferred from Tx buffer to shift register
    read_data = (uint8_t)SPI1->DR;//dummy read previously saved data in Rx shift register
    
    SPI_SendData(SPI1, 0xff);//dummy data
    while( SPI_GetFlagStatus(SPI1, SPI_FLAG_RXNE) ==RESET)
      read_data=0;//wait untill data is transferred from shift register to Rx buffer
    GPIO_SetBits(GPIOB, SPI1_NSS); //disable spi
    read_data = SPI_ReceiveData(SPI1);//read Rx buffer
    SPI1->SR = (SPI1->SR & (!SPI_FLAG_RXNE));//erase RXNE  flag at the end
    return read_data;
}

void rfm69cw_multiread(uint8_t addr, uint8_t *byte, uint8_t length)
{
    SPI1->SR = (SPI1->SR & (!SPI_FLAG_RXNE));//erase RXNE  flag at the beginning
    uint8_t read_data;
    GPIO_ResetBits(GPIOB, SPI1_NSS);
    SPI_SendData(SPI1, addr);
    while( SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) ==RESET)  
      read_data = 0;
    read_data = (uint8_t)SPI1->DR;//dummy read previously saved data in Rx shift register
    
    uint8_t i=0;
    while(i<length)
    {
      SPI_SendData(SPI1, 0xff);//dummy data
      while( SPI_GetFlagStatus(SPI1, SPI_FLAG_RXNE) ==RESET)
        read_data=0;
       byte[i] = SPI_ReceiveData(SPI1);
       SPI1->SR = (SPI1->SR & (!SPI_FLAG_RXNE));//erase RXNE  flag at the end
       i++;
    }
    GPIO_SetBits(GPIOB, SPI1_NSS);
}




// freqBand must be selected from 315, 433, 868, 915
void rfm69_init(uint16_t freqBand, uint8_t nodeID, uint8_t networkID)
{
    const uint8_t CONFIG[][2] =
    {
        /* 0x01 */ { REG_OPMODE, RF_OPMODE_SEQUENCER_ON | RF_OPMODE_LISTEN_OFF | RF_OPMODE_STANDBY },
     // {REG_OPMODE, RF_OPMODE_SYNTHESIZER | RF_OPMODE_TRANSMITTER },
      
        /* 0x02 */ { REG_DATAMODUL, RF_DATAMODUL_DATAMODE_PACKET | RF_DATAMODUL_MODULATIONTYPE_FSK | RF_DATAMODUL_MODULATIONSHAPING_00 }, // no shaping
        /* 0x03 */ { REG_BITRATEMSB, RF_BITRATEMSB_55555}, // default: 4.8 KBPS
        /* 0x04 */ { REG_BITRATELSB, RF_BITRATELSB_55555},
        /* 0x05 */ { REG_FDEVMSB, RF_FDEVMSB_50000}, // default: 5KHz, (FDEV + BitRate / 2 <= 500KHz)
        /* 0x06 */ { REG_FDEVLSB, RF_FDEVLSB_50000},

        /* 0x07 */ { REG_FRFMSB, RF_FRFMSB_868},
        /* 0x08 */ { REG_FRFMID, RF_FRFMID_868},
        /* 0x09 */ { REG_FRFLSB, RF_FRFLSB_868},
        
        ///* 0x07 */ { REG_FRFMSB, (uint8_t) (freqBand==RF_315MHZ ? RF_FRFMSB_315 : (freqBand==RF_433MHZ ? RF_FRFMSB_433 : (freqBand==RF_868MHZ ? RF_FRFMSB_868 : RF_FRFMSB_915))) },
       ///* 0x08 */ { REG_FRFMID, (uint8_t) (freqBand==RF_315MHZ ? RF_FRFMID_315 : (freqBand==RF_433MHZ ? RF_FRFMID_433 : (freqBand==RF_868MHZ ? RF_FRFMID_868 : RF_FRFMID_915))) },
        ///* 0x09 */ { REG_FRFLSB, (uint8_t) (freqBand==RF_315MHZ ? RF_FRFLSB_315 : (freqBand==RF_433MHZ ? RF_FRFLSB_433 : (freqBand==RF_868MHZ ? RF_FRFLSB_868 : RF_FRFLSB_915))) },


        // looks like PA1 and PA2 are not implemented on RFM69W, hence the max output power is 13dBm
        // +17dBm and +20dBm are possible on RFM69HW
        // +13dBm formula: Pout = -18 + OutputPower (with PA0 or PA1**)
        // +17dBm formula: Pout = -14 + OutputPower (with PA1 and PA2)**
        // +20dBm formula: Pout = -11 + OutputPower (with PA1 and PA2)** and high power PA settings (section 3.3.7 in datasheet)
        /* 0x11 */ { REG_PALEVEL, RF_PALEVEL_PA0_ON | RF_PALEVEL_PA1_OFF | RF_PALEVEL_PA2_OFF | RF_PALEVEL_OUTPUTPOWER_11111},
        /* 0x13 */ { REG_OCP, RF_OCP_ON | RF_OCP_TRIM_95 }, // over current protection (default is 95mA)

        // RXBW defaults are { REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_24 | RF_RXBW_EXP_5} (RxBw: 10.4KHz)
        /* 0x19 */ { REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_16 | RF_RXBW_EXP_2 }, // (BitRate < 2 * RxBw)
        //for BR-19200: /* 0x19 */ { REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_24 | RF_RXBW_EXP_3 },
        /* 0x25 */ { REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_01 }, // DIO0 is the only IRQ we're using
        /* 0x26 */ { REG_DIOMAPPING2, RF_DIOMAPPING2_CLKOUT_OFF }, // DIO5 ClkOut disable for power saving
        /* 0x28 */ { REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOOVERRUN }, // writing to this bit ensures that the FIFO & status flags are reset
        /* 0x29 */ { REG_RSSITHRESH, 220 }, // must be set to dBm = (-Sensitivity / 2), default is 0xE4 = 228 so -114dBm
        ///* 0x2D */ { REG_PREAMBLELSB, RF_PREAMBLESIZE_LSB_VALUE } // default 3 preamble bytes 0xAAAAAA
        /* 0x2E */ { REG_SYNCCONFIG, RF_SYNC_ON | RF_SYNC_FIFOFILL_AUTO | RF_SYNC_SIZE_2 | RF_SYNC_TOL_0 },
        /* 0x2F */ { REG_SYNCVALUE1, 0x2D },      // attempt to make this compatible with sync1 byte of RFM12B lib
        /* 0x30 */ { REG_SYNCVALUE2, networkID }, // NETWORK ID
        /* 0x37 */ { REG_PACKETCONFIG1, RF_PACKET1_FORMAT_VARIABLE | RF_PACKET1_DCFREE_OFF | RF_PACKET1_ADRSFILTERING_OFF },
        /* 0x38 */ { REG_PAYLOADLENGTH, 64 }, // in variable length mode: the max frame size, not used in TX
        ///* 0x39 */ { REG_NODEADRS, nodeID }, // turned off because we're not using address filtering
        /* 0x3C */ { REG_FIFOTHRESH, RF_FIFOTHRESH_TXSTART_FIFONOTEMPTY | RF_FIFOTHRESH_VALUE }, // TX on FIFO not empty
        /* 0x3D */ { REG_PACKETCONFIG2, RF_PACKET2_RXRESTARTDELAY_2BITS | RF_PACKET2_AUTORXRESTART_ON | RF_PACKET2_AES_OFF }, // RXRESTARTDELAY must match transmitter PA ramp-down time (bitrate dependent)
        //for BR-19200: /* 0x3D */ { REG_PACKETCONFIG2, RF_PACKET2_RXRESTARTDELAY_NONE | RF_PACKET2_AUTORXRESTART_ON | RF_PACKET2_AES_OFF }, // RXRESTARTDELAY must match transmitter PA ramp-down time (bitrate dependent)
        /* 0x6F */ { REG_TESTDAGC, RF_DAGC_IMPROVED_LOWBETA0 }, // run DAGC continuously in RX mode for Fading Margin Improvement, recommended default for AfcLowBetaOn=0
        {255, 0}
    };

   
    rfm69cw_write(REG_SYNCVALUE1,  0xaa);

    for (uint8_t i = 0; CONFIG[i][0] != 255; i++)
        rfm69cw_write(CONFIG[i][0], CONFIG[i][1]);

    setMode(RF69_MODE_STANDBY);


    address = nodeID;
    setAddress(address);            // setting this node id
    setNetwork(networkID);
}

// set this node's address
void setAddress(uint8_t addr)
{
    rfm69cw_write(REG_NODEADRS, addr);
}

// set network address
void setNetwork(uint8_t networkID)
{
    rfm69cw_write(REG_SYNCVALUE2, networkID);
}


// set *transmit/TX* output power: 0=min, 31=max
// this results in a "weaker" transmitted signal, and directly results in a lower RSSI at the receiver
// the power configurations are explained in the SX1231H datasheet (Table 10 on p21; RegPaLevel p66): http://www.semtech.com/images/datasheet/sx1231h.pdf
// valid powerLevel parameter values are 0-31 and result in a directly proportional effect on the output/transmission power
// this function implements 2 modes as follows:
//       - for RFM69W the range is from 0-31 [-18dBm to 13dBm] (PA0 only on RFIO pin)
//       - for RFM69HW the range is from 0-31 [5dBm to 20dBm]  (PA1 & PA2 on PA_BOOST pin & high Power PA settings - see section 3.3.7 in datasheet, p22)
void setPowerLevel(uint8_t powerLevel)
{
    rfm69cw_write(REG_PALEVEL, (rfm69cw_read(REG_PALEVEL) & 0xE0) | powerLevel);
}

//put transceiver in sleep mode to save battery - to wake or resume receiving just call receiveDone()
void sleep() 
{
    setMode(RF69_MODE_SLEEP);
}

// return the frequency (in Hz)
uint32_t getFrequency()
{
    return RF69_FSTEP * (((uint32_t) rfm69cw_read(REG_FRFMSB) << 16) + ((uint16_t) rfm69cw_read(REG_FRFMID) << 8) + rfm69cw_read(REG_FRFLSB));
}

// set the frequency (in Hz)
void setFrequency(uint32_t freqHz)
{
    uint8_t oldMode = mode;
    if (oldMode == RF69_MODE_TX) {
        setMode(RF69_MODE_RX);
    }
    freqHz /= RF69_FSTEP; // divide down by FSTEP to get FRF
    rfm69cw_write(REG_FRFMSB, freqHz >> 16);
    rfm69cw_write(REG_FRFMID, freqHz >> 8);
    rfm69cw_write(REG_FRFLSB, freqHz);
    if (oldMode == RF69_MODE_RX) {
        setMode(RF69_MODE_SYNTH);
    }
    setMode(oldMode);
}


void setMode(uint8_t newMode)
{
    if (newMode == mode)
    return;

    switch (newMode)
    {
        case RF69_MODE_TX:
            rfm69cw_write(REG_OPMODE, (rfm69cw_read(REG_OPMODE) & 0xE3) | RF_OPMODE_TRANSMITTER);
            break;
        case RF69_MODE_RX:
            rfm69cw_write(REG_OPMODE, (rfm69cw_read(REG_OPMODE) & 0xE3) | RF_OPMODE_RECEIVER);
            break;
        case RF69_MODE_SYNTH:
            rfm69cw_write(REG_OPMODE, (rfm69cw_read(REG_OPMODE) & 0xE3) | RF_OPMODE_SYNTHESIZER);
            break;
        case RF69_MODE_STANDBY:
            rfm69cw_write(REG_OPMODE, (rfm69cw_read(REG_OPMODE) & 0xE3) | RF_OPMODE_STANDBY);
            break;
        case RF69_MODE_SLEEP:
            rfm69cw_write(REG_OPMODE, (rfm69cw_read(REG_OPMODE) & 0xE3) | RF_OPMODE_SLEEP);
            break;
        default:
        return;
    }
    // we are using packet mode, so this check is not really needed
    // but waiting for mode ready is necessary when going from sleep because the FIFO may not be immediately available from previous mode
    while (mode == RF69_MODE_SLEEP && (rfm69cw_read(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00); // wait for ModeReady
    mode = newMode;
}
    

 


