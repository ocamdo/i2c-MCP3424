// Program: wl.cpp
// Author: Daniel Ocampo
// Version: 1.0
// Date: 23-Sep-2018
// Description: Reading data from i2c MCP3424 in Raspberry Pi
// IC: MCP3424 i2c (4 channles)
// Wiring:   Sensor     RPi
//             VDD  -> VDD (GPIO 1 : 3.3v)
//             SDA  -> SDA (GPIO 3)
//             SCL  -> SCL (GPIO 5)
//             GND  -> GND (GPIO 7 or any GND pin)
// Voltage on input pins must be < 2.048V
// Configuration used: R/Channel/O-S-C=0/12 bits=00/PGA-1=00
//                              0x80 = 1/00/0/00/00
//                              0xA0 = 1/01/0/00/00
//                              0xC0 = 1/10/0/00/00
//                              0xE0 = 1/11/0/00/00
// Ccompiling: g++ wl.cpp -o wl -lbcm2835
//

#include <stdio.h>
#include <stdlib.h>
#include <bcm2835.h>
#include <string.h>
#include <math.h>

// Definitions (variables & buffer)
char buf[3];                                            // Store 4 bytes from i2c sensor
uint16_t snsr_addr = 0x68;                              // i2c sensor address
char conf[4] = {0x80, 0xA0, 0xC0, 0xE0};                // Channel 1, 2, 3 & 4
long gain[4] = {1000000, 2000000, 4000000, 8000000};    // PGA/gain 1, 2, 4 & 8

/// MAIN Prog
int main(int argc, char** argv)
{
    if (!bcm2835_init())
        return 1;

    // Clock divider (however not used yet)
    bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_626);
    // i2c set-up start
    bcm2835_i2c_begin();
    // Set sensor address
    bcm2835_i2c_setSlaveAddress(snsr_addr);

    // Initializing the IC and write a command configuration as per conf array
    for (int c = 0; c < 4; c ++)
    {
        char chan[1] = {conf[c]};
        bcm2835_i2c_write(chan, 1);
        delay(100);                                  // 100ms delay

        unsigned int status = bcm2835_i2c_read(buf, 3) != BCM2835_I2C_REASON_OK; // Check that status is 0 and read the conversion
        if( status )
        {
                printf("Status: 1 error! \n");      // Exit if status is not 0
                bcm2835_i2c_end();
        }
        else
        {
                // Check MSB & LSB and calculate the voltage and use different gains (PGA)
                for (int g = 0; g < 4; g ++)
                {
                        delay(100);
                        long res = (((long)buf[0] & 0x0F) << 8) | ((long)buf[1] & 0xFF);
                        res |= long(buf[0] & 0x80) << 24;
                        //float volts = res*1000.0/1000000;
                        float volts = res*1000.0/gain[g];
                        printf("OK! Status: %d - Channel: %X \n", status, chan[0]);
                        printf("Gain: [%d] \n", gain[g]);
                        printf("Bytes: [%X, %X, %X]\n",buf[0],buf[1],buf[2]);
                        printf("Volts: [%.4f]\n",volts);
                }
        }
    }
    // Closing reading from sensor
    bcm2835_i2c_end();

    // Closing BCM2835 library
    bcm2835_close();
    return (EXIT_SUCCESS);
}
