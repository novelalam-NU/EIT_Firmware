#ifndef AD7450_ADC_H
#define AD7450_ADC_H

#define AD7450_QUEUE_SIZE 1000

#include <stdint.h>

/**
 * @brief Initialize the AD7450 ADC driver.
 * 
 * This function adds the AD7450 device to the SPI bus and performs a dummy read
 * to ensure the device is powered up.
 * 
 * @return 0 on success, or an error code.
 */
int AD7450_init();

/**
 * @brief Read samples from the AD7450 ADC.
 * 
 * This function performs the specified number of reads and stores the values in the provided buffer.
 * 
 * @param buf Pointer to buffer where ADC values will be stored.
 * @param len Number of samples to read.
 * @return 0 on success, or an error code.
 */
int AD7450_Read(int16_t *buf, uint32_t len);

#endif // AD7450_ADC_H
