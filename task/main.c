#include "stm8l15x_conf.h" 

#define Tx 1
//#define Rx 1

//#define Debug

volatile uint8_t data;

/* global variable stored the value of pressed buttons A2,A3,A4,A5 */
/*filled during external interrupts*/
volatile uint8_t button;

#define NETWORKID 33
#define NODEID     4



int main( void )
{
//CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);//By default it is running with /8 divider

  button=0u;
  
  /* Init LEDs I/O ports */
  GPIO_Init( GPIOC, GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6, GPIO_Mode_Out_PP_Low_Slow);
  GPIO_Init( GPIOD, GPIO_Pin_1, GPIO_Mode_Out_PP_Low_Slow);
  
  /* Init Buttons I/O ports */  
  GPIO_Init( GPIOA, GPIO_Pin_2 | GPIO_Pin_3| GPIO_Pin_4| GPIO_Pin_5, GPIO_Mode_In_PU_IT);
  
  
  /* Init SPI pins*/
  GPIO_Init( GPIOB, SPI1_NSS | SPI1_MOSI, GPIO_Mode_Out_PP_High_Fast);
  GPIO_Init( GPIOB, SPI1_SCK, GPIO_Mode_Out_PP_Low_Fast);
  GPIO_Init( GPIOB, SPI1_MISO , GPIO_Mode_In_PU_No_IT);  
  
  spi_start();
  
  /*load data in rfm69*/
  rfm69_init(868, NODEID, NETWORKID);//315, 433, 868, 915

 
  
  while(1)
  {
      delay_10us(5);
      
#ifdef Tx   
      setMode(RF69_MODE_STANDBY);
      //dattx[0]=10u is Length of Packet
      uint8_t dattx10[11]={10u};
      uint8_t dattx20[21]={20u}; 
      uint8_t dattx30[31]={30u};
      uint8_t dattx40[41]={40u}; 
      
      disableInterrupts();
      
      if(button==2)//(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2)==RESET)
      {
        rfm69cw_multiwrite(0x00, dattx10, 11);
        setMode(RF69_MODE_TX);
        button=0;
      }
       if(button==3)//(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3)==RESET)
      {
        rfm69cw_multiwrite(0x00, dattx20, 21);
        setMode(RF69_MODE_TX);
        button=0;
      }
       if(button==4)//(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4)==RESET)
      {
        rfm69cw_multiwrite(0x00, dattx30, 31);
        setMode(RF69_MODE_TX);
        button=0;
      }
       if(button==5)//(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5)==RESET)
      {
        rfm69cw_multiwrite(0x00, dattx40, 41);
        setMode(RF69_MODE_TX);
        button=0;
      }      
      enableInterrupts(); 
      halt();
      delay_10us(100);
#ifdef Debug      
      data=rfm69cw_read(0x28); //expected status = bx1?0_1000
      data=rfm69cw_read(0x27); //expected status = b1011_0000, 0xB0
#endif      
        
#endif
      
      
#ifdef Rx
      setPowerLevel(0);//decrease current consumption for Rx, do not need out amplifier
      uint8_t datrx[6]={0u};//need to read only first byte - Packet Length
      setMode(RF69_MODE_STANDBY);        
      setMode(RF69_MODE_RX);      
#ifdef Debug      
      data = rfm69cw_read(REG_OPMODE);
#endif      
      delay_10us(100);
      rfm69cw_multiread(0x00, datrx, 6);
      
      switch(datrx[0]){//first byte of message - packet Length
      case 10u:
         GPIO_ResetBits(GPIOC, GPIO_Pin_2 | GPIO_Pin_3| GPIO_Pin_4| GPIO_Pin_5| GPIO_Pin_6); 
         GPIO_ResetBits(GPIOD, GPIO_Pin_1);
         GPIO_SetBits(GPIOC, GPIO_Pin_2 | GPIO_Pin_3);
         break;
      case 20u:
         GPIO_ResetBits(GPIOC, GPIO_Pin_2 | GPIO_Pin_3| GPIO_Pin_4| GPIO_Pin_5| GPIO_Pin_6);
         GPIO_ResetBits(GPIOD, GPIO_Pin_1);
         GPIO_SetBits(GPIOC, GPIO_Pin_3 | GPIO_Pin_5);
         break;
      case 30u:
         GPIO_ResetBits(GPIOC, GPIO_Pin_2 | GPIO_Pin_3| GPIO_Pin_4| GPIO_Pin_5| GPIO_Pin_6); 
         GPIO_ResetBits(GPIOD, GPIO_Pin_1);
         GPIO_SetBits(GPIOC, GPIO_Pin_2 | GPIO_Pin_6);
         break;
      case 40u:
         GPIO_ResetBits(GPIOC, GPIO_Pin_2 | GPIO_Pin_3| GPIO_Pin_4| GPIO_Pin_5| GPIO_Pin_6); 
         GPIO_ResetBits(GPIOD, GPIO_Pin_1);
         GPIO_SetBits(GPIOC, GPIO_Pin_5 | GPIO_Pin_6);
         break;  
      default:
        break;
      }
#endif    
     
  }

  SPI_Cmd(SPI1, DISABLE);
  SPI_DeInit(SPI1);
  
  return 0;
}

