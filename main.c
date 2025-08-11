/*
 * 
 */

#include <msp430.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define CHILD_ADDR 0x38

int8_t temp_corr;
uint8_t rh_corr;
uint32_t temp_uncorr;
uint32_t rh_uncorr;

unsigned char *PTxData;                     //POINTER TO TxData
const unsigned char TxData[] = {0xAC, 0x33, 0x00};
unsigned char *PRxData;
unsigned char RxData[6];
unsigned char TxByteCtr;
unsigned char RxByteCtr;
unsigned char i;


int8_t ths_calc_temp(uint32_t temp_uncorr);
uint8_t ths_calc_rh(uint32_t rh_uncorr);

void main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer

  /*
   *------INITIALIZE I2C
   */

  UCB0CTL1 = UCSWRST;                       //RESET
  P3SEL |= 0X03;                            //SET PORT 3 BIT 0,1 TO PERIPHERAL (I2C)
  UCB0CTL0 = UCMODE_3 + UCMST + UCSYNC;     //SET I2C BIT, MASTER BIT, SYCHRONOUS MODE BIT
  UCB0CTL1 |= UCSSEL_3;                     //SEL SECONDARY CLOCK
  UCB0BRW = 12;                             //SET SCALER DIVIDE BY 12 FOR approx 100KHZ
  UCB0BR1 = 0;
  UCB0I2CSA = CHILD_ADDR;                   //SET SLAVE ADDRESS
  UCB0CTL1 &= ~UCSWRST;                     //EXIT RESET
   
  /*
   *------ENABLE IRQ
   */

  UCB0IE |= UCTXIE;                     //SET TX IRQ
  UCB0IE |= UCRXIE;                     //SET RX IRQ                       

  /*
   *------SUPER LOOP
   */

  while(1){
    RxByteCtr = sizeof RxData;                     //SET COUNTER TO RECEIVE ARRAY SIZE
    TxByteCtr = sizeof TxData;                     //SET COUNTER TO TRANSMIT ARRAY SIZE

    /*
     *------WRITE
     */
    for(i=0;i<10;i++);                             //DELAY BEFORE TX
    PTxData = (unsigned char *)TxData;             //TX array start address

    UCB0CTLW0 |= UCTR + UCTXSTT;                   //SET TO WRITE MODE AND TRANSMIT

    __bis_SR_register(LPM0_bits + GIE);      //ENTER LPMN0 AND ENABLE GLOBAL INTERRUPTS
    __no_operation();                              //REMAIN IN LPM0 UNTIL ALL DATA IS TX'D

    while (UCB0CTL1 & UCTXSTP);                    //ENSURE STOP CONDITION GOT SENT
    UCB0IFG &= ~UCSTPIFG;                          //CLEAR STOP FLAG

    /*
     *------READ
     */
    for(i=0; i<100; i--);                          //DELAY FOR AHT21 TO TAKE MEASUREMENT
    PRxData = (unsigned char *)RxData;             //RX ARRAY START ADDRESS

    UCB0CTLW0 &= ~UCTR;                            //CHANGE TO READ
    UCB0CTLW0 |= UCTXSTT;                          //START TRANSMISSION
    while(UCB0CTL1 & UCTXSTT);                     //START CONDITION SENT

    __bis_SR_register(LPM0_bits + GIE);      //ENTER LPM0 AND ENABLE GLOBAL INTERRUPTS
    __no_operation();                              //REMAIN IN LPM0 UNTIL ALL DATA IS RX'D

    while(UCB0CTL1 & UCTXSTP);                     //WAIT TILL STOP FLAG IS SET
    UCB0IFG &= ~UCSTPIFG;                          //CLEAR STOP FLAG

    /*
     *------CALCULATE CORRECTED TEMPERATURE AND RELATIVE HUMIDITY
     */

    temp_uncorr = (uint32_t)((RxData[1] << 12) | (RxData[2] << 4) | (RxData[3] >> 4));     //extract raw temp data
    rh_uncorr = (uint32_t)(((RxData[3] & 0x0F) << 16) | (RxData[4] << 8) | (RxData[5]));   //extract raw rh data

    temp_corr = ths_calc_temp(temp_uncorr);                                                         //convert temp data
    rh_corr = ths_calc_rh(rh_uncorr);                                                               //convert rh data
    
    /*
     *-------PRINT TEMP AND RH DATA
     */
    printf("Current temperature: %d fahrenheit./n", temp_corr);                             //print temp to console
    printf("Current relative humidity: %d %%/n", rh_corr);                                  //print rh to console
  }
}

/*
 *------Interrupt Service Routines
 */

 #pragma vector = USCI_B0_VECTOR  
 __interrupt void USCI_B0_I2C_ISR (void){
  switch(__even_in_range(UCB0IV,12))
  {
  case  0: break;                           // Vector  0: No interrupts
  case  2: break;                           // Vector  2: ALIFG
  case  4: break;                           // Vector  4: NACKIFG
  case  6: break;                           // Vector  6: STTIFG
  case  8: break;                           // Vector  8: STPIFG
  case 10:                                  // Vector 10: RXIFG
    RxByteCtr--;                                  // Decrement RX byte counter
    if (RxByteCtr)
    {
      *PRxData++ = UCB0RXBUF;                     // Move RX data to address PRxData
      if (RxByteCtr == 1)                         // Only one byte left?
        UCB0CTL1 |= UCTXSTP;                      // Generate I2C stop condition
    }
    else
    {
      *PRxData = UCB0RXBUF;                       // Move final RX data to PRxData
      __bic_SR_register_on_exit(LPM0_bits); // Exit active CPU
    }
    break;                           
  case 12:                                        // Vector 12: TXIFG  
    if (TxByteCtr)                                // Check TX byte counter
    {
      UCB0TXBUF = *PTxData++;                     // Load TX buffer
      TxByteCtr--;                                // Decrement TX byte counter
    }
    else
    {
      UCB0CTL1 |= UCTXSTP;                        // I2C stop condition
      UCB0IFG &= ~UCTXIFG;                        // Clear USCI_B0 TX int flag
      __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
    }  
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




/*
switch(UCB0IV){
        case 0x0A:                                //RXIFG (Receive Interrupt Flag)                             
            if(RxByteCounter == (sizeof(rx_buffer) - 2)){
              rx_buffer[i] = UCB0RXBUF;
              UCB0CTL1 |= UCTXSTP;
              UCB0IFG &= ~UCTXIFG;
              } else{
              rx_buffer[i] = UCB0RXBUF;
              RxByteCounter++;
              __bic_SR_register_on_exit(LPM0_bits);
            }
            break;
        case 0x0C:                                //TXIFG (Transmit Interrupt Flag)
            if(TxByteCounter == (sizeof(tx_buffer) - 1)){     
              UCB0TXBUF = tx_buffer[i];
              UCB0CTL1 |= UCTXSTP;
              P1OUT |= BIT0;
            } else{
              UCB0TXBUF = tx_buffer[i];
              TxByteCounter++;
              __bic_SR_register_on_exit(LPM0_bits);
            }
            break;
        default: break;
  }
*/
