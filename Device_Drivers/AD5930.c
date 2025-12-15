/**
 * @file ad5930.c - ad5930 signal generator (HAL-agnostic).
 * Author: Novel Alam, Date: Nov 12 2025.
 */

#include "AD5930.h"

int AD5930_init(float init_freq) {

    printf("AD5930 Set to %f \n", init_freq);
    // uint16_t packet; //spi packet

    // if (init_freq > MAX_FREQ) {
    //     init_freq = 123;
    // }

    // /* Configure the control register */
    // uint8_t B24 = 0b1;  // freq value transfered after both high and low byte done 
    // uint8_t dac_enable = 0b0; //intially dont turn on dac
    // uint8_t SINE_TRIG = 1; //output sin wave
    // uint8_t MSBOUT_EN = 1; //enable msbout wire
    // uint8_t Continouse = 1; 



    // packet = (C_REG<<12) | (B24<<11) | (dac_enable<< 10) | (SINE_TRIG<<9)  | (MSBOUT_EN<<8) | (Continouse<<7) | 0b00000011;

    // //spi send (packet, )



    // /* Set Low and High byte of Fstart register */
    // uint32_t F_start = 0xFFFFFF & ( freq_to_reg( init_freq ) );

    // packet = (START_FREQ_LOW<<12) | F_start & 0xFFF;

    // //spi send

    // packet = (START_FREQ_HIGH<<12) | (F_start>>12) & 0xFFF;

    // //spi send

    // /* Set increment freq to 0*/
    // packet = (DELTA_FREQ_LOW<<12) | 0x000;

    // packet = (DELTA_FREQ_HIGH<<12) | 0x000;

    // // spi send


    // /* Set Number of incriments to 0*/
    // packet = (INC_INTERVAL<<12) | 0x000;


    // // spi send
    // dac_enable = 1; //start 
    // packet = (C_REG<<12) | (B24<<11) | (dac_enable<< 10) | (SINE_TRIG<<9)  | (MSBOUT_EN<<8) | 0b000000011;


    return 0;



}


static inline uint32_t freq_to_reg(float freq) {
    return (uint32_t) (((freq * CONSTANT_1) / MCLK_FREQ) + 0.5f);
}