/*
 * 
 */

#include <msp430.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define CHILD_ADDR 0x38
#define THS_MEASURE_CMD 0xAC
#define THS_STATUS_CMD 0x71
#define SENSE_WAIT 100

int8_t temp_corr;
uint8_t rh_corr;
uint32_t temp_uncorr;
uint32_t rh_uncorr;

uint8_t i;
char tx_buffer[] = {0xAC, 0x33, 0x00};
char rx_buffer[] = {0,0,0,0,0,0};


int8_t ths_calc_temp(uint32_t temp_uncorr);
uint8_t ths_calc_rh(uint32_t rh_uncorr);

void main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer

  /*
   *------INITIALIZE I2C
   */

  UCB0CTLW0 = UCSWRST;                  //RESET
  UCB0CTLW0 |= UCMODE_3;                //SET I2C BIT
  UCB0CTLW0 |= UCMST;                   //SET MASTER
 
  UCB0CTLW0 |= UCSYNC;                  //SET SYNCHRONOUS MODE

  UCB0CTLW0 |= UCSSEL_3;                //SEL SECONDARY CLOCK
  UCB0BRW = 10;                         //SET SCALER DIVIDE BY 10 FOR 100KHZ

  UCB0I2CSA = CHILD_ADDR;               //SET SLAVE ADDRESS

  P3SEL |= 0x03;                        //P3SEL |= BIT0;                        //SET P3.0 SDA??
                                        //P3SEL |= BIT1;                        //SET P3.1 SCL??

                                        //ENABLE PULLUP RESISTORS????????????
                                        //TURN ON IO?? PM5CTL0 &= ~LOCKLPM5
                                        //P3REN |=      enable pullup resistor?? 
                                        //P3OUT = 0x01  enable pullup resistor high??

  UCB0CTLW0 &= ~UCSWRST;                //EXIT RESET

  /*
   *------ENABLE IRQ
   */

  UCB0IE |= UCTXIE;                     //SET TX IRQ
  UCB0IE |= UCRXIE;                     //SET RX IRQ                       
  __enable_interrupt();                 //SET GLOBAL INTERRUPT GIE
  
  /*
   *------CONFIGURE LED
   */

  P1DIR |= BIT0;
  P1OUT&= ~BIT0;
  
  while(1){
    UCB0CTLW0 |= UCTR;                  //SET TO WRITE
    UCB0CTLW0 |= UCTXSTT;               //START TRANSMISSION

    

    while( UCB0IFG & UCSTPIFG == 0){};  //WAIT TILL STOP FLAG IS SET
    UCB0IFG &= ~UCSTPIFG;               //CLEAR STOP FLAG

    __delay_cycles(1000000);     //delay for reading - is this required???
                                   
    UCB0CTLW0 &= ~UCTR;                 //CHANGE TO READ
    UCB0CTLW0 |= UCTXSTT;               //START TRANSMISSION

                                        //verify status bit???

    while( UCB0IFG & UCSTPIFG == 0){};  //WAIT TILL STOP FLAG IS SET
    UCB0IFG &= ~UCSTPIFG;               //CLEAR STOP FLAG

    P1OUT &= ~BIT0;

    temp_uncorr = (uint32_t)((rx_buffer[1] << 12) | (rx_buffer[2] << 4) | (rx_buffer[3] >> 4));     //extract raw temp data

    rh_uncorr = (uint32_t)(((rx_buffer[3] & 0x0F) << 16) | (rx_buffer[4] << 8) | (rx_buffer[5]));   //extract raw rh data

    temp_corr = ths_calc_temp(temp_uncorr);                                                         //convert temp data
    rh_corr = ths_calc_rh(rh_uncorr);                                                               //convert rh data
    
    printf("Current temperature: %d fahrenheit./n", temp_corr);                             //print temp to console
    printf("Current relative humidity: %d %%/n", rh_corr);                                  //print rh to console
  }
  
}


/*
 *  ISRs
 */
uint8_t i = 0;
 #pragma vector = USCI_B0_VECTOR  
 __interrupt void USCI_B0_I2C_ISR (void){
  
  switch(UCB0IV){
        case 0x0A:                                //RXIFG (Receive Interrupt Flag)
            i = 0;                                //WILL RECEIVE 6 BITS FROM SENSOR
            if (i == 4){                          //UCTXSTP MUST BE SENT DURING THE NEXT TO LAST BYTE
              UCB0CTLW0 |= UCTXSTP;
              rx_buffer[i] = UCB0RXBUF;
              i++;
            }
            if(i == 5){
              rx_buffer[i] = UCB0RXBUF;
              i = 0;
            } else{
              rx_buffer[i] = UCB0RXBUF;
              i++;
            }
            break;
        case 0x0C:                                //TXIFG (Transmit Interrupt Flag)
            i = 0;
            if(i == (sizeof(tx_buffer) - 1)){     
              UCB0TXBUF = tx_buffer[i];
              i = 0;
              UCB0CTLW0 |= UCTXSTP;
              P1OUT |= BIT0;
            } else{
              UCB0TXBUF = tx_buffer[i];
              i++;
            }
            break;
        default: break;
  }
}



 /*
  * Functions
  */

int8_t ths_calc_temp(uint32_t temp_uncorr){
    return (int8_t)((temp_uncorr/pow(2,20))*200-50); 
}

uint8_t ths_calc_rh(uint32_t rh_uncorr){
    return (uint8_t)((rh_uncorr/pow(2,20))*100);
}
