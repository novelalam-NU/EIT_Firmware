#pragma once

#include <stdint.h>
#include <stddef.h>
#include <driver/gpio.h>

#define SRC_INAMP_HANDLE 0
#define SENSE_INAMP_HANDLE 1

#define ESP32C3

#ifdef ESP32C3
// SPI bus pins (XIAO ESP32-C3)
#define PIN_SPI_MOSI      GPIO_NUM_10    // D8  / SCK
#define PIN_SPI_MISO      GPIO_NUM_9    // D9  / MISO
#define PIN_SPI_SCLK      GPIO_NUM_8   // D10 / MOSI

// Chip Selects
#define PIN_CS_ADC        GPIO_NUM_2    // P2-1
#define PIN_CS_DRIVE      GPIO_NUM_5    // P2-2
#define PIN_CS_MEAS       GPIO_NUM_6    // P2-3
#define PIN_CS_MUX        GPIO_NUM_7    // P2-4
#define PIN_CS_AD5930     GPIO_NUM_20    // P2-5

#define PIN_CTRL          GPIO_NUM_4    // P2-6
#define PIN_MSB           GPIO_NUM_3   // P3-7
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
int adcRead(int16_t *buf, size_t len, uint16_t gain);

/**
 * @brief Calculate the accumulated amplitude of frequency components from the buffer within a range of bins.
 * 
 * @param buf Buffer containing the data.
 * @param len Length of the buffer.
 * @param begin Start bin index (inclusive).
 * @param end End bin index (inclusive).
 * @return The accumulated amplitude.
 */
uint32_t dsp_freq_amp(int16_t *buf, size_t len, uint8_t begin, uint8_t end);



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
 * @brief Detect if the operational amplifier output is clipping.
 * 
 * @param buf Buffer containing the sampled data values.
 * @param len Length of the buffer.
 * @param threshold Threshold for the accumulated magnitude.
 * @param begin Start bin index (inclusive).
 * @param end End bin index (inclusive).
 * @return true if clipping is detected, false otherwise.
 */
bool detect_opamp_clipping(int16_t *buf, size_t len, uint32_t threshold, uint8_t begin, uint8_t end);



