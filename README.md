# I2C_AHT21_TX_RX

Description: I wrote this program with the intention of exlporing and learning about embedded systems
software design. The goal is to use the MSP430F5529 to communicate with the AHT21 combined temperature
and relative humidity sensor. I would like to output the temperature and humidity data to some sort of
display that has yet been undetermined. Depending on the level of succes and my ambition I would like
to potentially write a full driver for the AHT21 for use on the MSP430.

## STATUS
Currently the the MSP430 can transmit the required commands to the AHT21 and receive the required bytes
from the AHT21. I have verified in the CCS debugger the RxData array is receiving data from the sensor and that when looping through the program the connects will update. However, the code for extracting the
data from the bytes and separating it into their respective uncorrected variables is not functioning as expected and therefore the temperature and relative humidity cannot be calculated. 

## COMPONENTS
- MSP430F5529 Launchpad
- AHT21 Combined Temperature and Relative Humidty Sensor

### MSP430F5529
ENTER DISCRIPTION HERE

### AHT21
ENTER DISCRIPTION HERE

## TOOLCHAIN

### Code Composer Studio
- ENTER VERSION HERE

### COMPILER
- ENTER VERSION HERE

### LINKER
- ENTER VERSION HERE


