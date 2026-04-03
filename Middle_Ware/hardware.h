#pragma once

#define ADC_BUF_LEN   64

// #define DEBUG 1

#include <stdint.h>
#include <stddef.h>
#include <driver/gpio.h>

#define SRC_INAMP_HANDLE 0
#define SENSE_INAMP_HANDLE 1

#define ESP32C3

#ifdef ESP32C3
// SPI bus pins (XIAO ESP32-C3)
#define PIN_SPI_MOSI      GPIO_NUM_9    
#define PIN_SPI_MISO      GPIO_NUM_10   
#define PIN_SPI_SCLK      GPIO_NUM_8   

// Chip Selects
#define PIN_CS_ADC        GPIO_NUM_2    
#define PIN_CS_DRIVE      GPIO_NUM_5    
#define PIN_CS_MEAS       GPIO_NUM_6    
#define PIN_CS_MUX        GPIO_NUM_7    
#define PIN_CS_AD5930     GPIO_NUM_44   

#define PIN_CTRL          GPIO_NUM_4    
#define PIN_MSB           GPIO_NUM_3  
#endif

/**
 * @brief Start the signal generator at a specific frequency.
 * 
 * @param freq The frequency to generate in Hz.
 * @return 0 on success, or an error code.
 */
int signal_gen_start(float freq);

/**
 * @brief Initialize the SPI bus.
 * 
 * @return 0 on success, or an error code.
 */
int init_spi(void);

/**
 * @brief Set the gain of the source instrumentation amplifier.
 * 
 * @param src_gain The gain value to set.
 * @return 0 on success, or an error code.
 */
int set_src_inamp_gain(uint16_t src_gain);

/**
 * @brief Set the gain of the sense instrumentation amplifier.
 * 
 * @param sense_gain The gain value to set.
 * @return 0 on success, or an error code.
 */
int set_sense_inamp_gain(uint16_t sense_gain);

/**
 * @brief Read raw data from the ADC.
 *
 * @param buf Buffer to store the read data.
 * @param len Length of the buffer.
 * @return 0 on success, or an error code.
 */
int adcRead(uint16_t *buf, size_t len);

/**
 * @brief Set the multiplexer channels for source and sense.
 * 
 * @param src_pos The positive source channel.
 * @param src_neg The negative source channel.
 * @param sense_pos The positive sense channel.
 * @param sense_neg The negative sense channel.
 * @return 0 on success, or an error code.
 */
int set_mux(uint8_t src_pos, uint8_t src_neg, uint8_t sense_pos, uint8_t sense_neg);

/**
 * @brief Initialize the instrumentation amplifier potentiometers.
 * 
 * @return 0 on success, or an error code.
 */
int init_inamp_pots();

/**
 * @brief Initialize the multiplexers.
 * 
 * @return 0 on success, or an error code.
 */
int init_mux(void);

/**
 * @brief Initialize the ADC.
 *
 * @return 0 on success, or an error code.
 */
int adc_init(void);

/**
 * @brief Reads ADC samples and computes the peak-to-peak amplitude.
 *
 * This function reads a buffer of ADC samples of length ADC_BUF_LEN,
 * checks for read errors, and computes the amplitude using the
 * standard deviation magnitude function. Returns 0 on ADC read error.
 *
 * @return Peak-to-peak amplitude (uint16_t), or 0 on error.
 */
uint16_t calc_peak_to_peak(void);




/**
 * @brief this function takes in a buffer of size Nfloat and returns a mag of that std dev range of value
 */
uint16_t calc_std_dev_mag(int16_t* buf, uint16_t buf_len, float std_multiplier);

