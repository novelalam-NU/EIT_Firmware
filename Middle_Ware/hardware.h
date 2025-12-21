#pragma once

#include <stdint.h>
#include <stddef.h>
#include <driver/gpio.h>

#define SRC_INAMP_HANDLE 0
#define SENSE_INAMP_HANDLE 1

#define ESP32C3

#ifdef ESP32C3
// SPI bus pins (XIAO ESP32-C3)
#define PIN_SPI_MOSI      GPIO_NUM_8    // D8  / SCK
#define PIN_SPI_MISO      GPIO_NUM_9    // D9  / MISO
#define PIN_SPI_SCLK      GPIO_NUM_10   // D10 / MOSI

// Chip Selects
#define PIN_CS_ADC        GPIO_NUM_2    // P2-1
#define PIN_CS_DRIVE      GPIO_NUM_3    // P2-2
#define PIN_CS_MEAS       GPIO_NUM_4    // P2-3
#define PIN_CS_MUX        GPIO_NUM_5    // P2-4
#define PIN_CS_AD5930     GPIO_NUM_6    // P2-5

#define PIN_CTRL          GPIO_NUM_7    // P2-6
#define PIN_MSB           GPIO_NUM_20   // P3-7
#endif


int signal_gen_start(float freq);

int init_spi(void);

int set_src_inamp_gain(uint16_t src_gain);
int set_sense_inamp_gain(uint16_t sense_gain);

int adcRead(uint8_t *buf, size_t len);

uint16_t dsp_freq_amp(uint16_t *buf, size_t len);



int set_mux(uint8_t src_pos, uint8_t src_neg, uint8_t sense_pos, uint8_t sense_neg);

int init_inamp_pots();

int init_mux(void);
