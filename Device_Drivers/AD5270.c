/**
 * @file ad5720.c - AD5720 digital potentiometer driver (HAL-agnostic).
 * Author: Novel Alam, Date: Nov 12 2025.
 */

#include "AD5270.h"


//extern bool spi_write(uint8_t *buf, uint8_t len);

int ad5270_init(void) {

    printf("ad5270_init Successful\n");
//     uint8_t tx_buf[2]; 
//     uint16_t data;

//     /* send command 7 to with control register values 0b11 to enable write to RDAC*/
//     data = 0x1C03; 

//     tx_buf[0] = data & 0xff;
//     tx_buf[1] = (data >> 8) & 0xff;

//    // spi_write(tx_buf, sizeof(tx_buf));

    return 0;

}

int ad5270_set_wiper(uint16_t r_code) {

    printf("Wiper set to %d\n", r_code);
    return -1;

//     if (resistance > MAX_R)
//         resistance = MAX_R;


//     uint8_t tx_buf[2]; 
//     uint16_t data;


//     data = (r_code & 0b1111111111) | 0b0001 << 10; //or the data with command number 1


//     tx_buf[0] = data & 0xff;
//     tx_buf[1] = (data >> 8) & 0xff;

//    // spi_write(tx_buf, sizeof(tx_buf));

//     return 0;

}

