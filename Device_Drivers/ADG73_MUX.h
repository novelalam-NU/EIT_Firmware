#pragma once

#include <stdint.h>

#include "../Middle_Ware/hardware.h"
#include "driver/spi_master.h"
#include "driver/spi_common.h"
#include "esp_log.h"

/**
 * @brief Initialize the ADG73 multiplexers.
 * 
 * @return 0 on success, or an error code.
 */
int init_src_sense_ADG73();

/**
 * @brief Set the source and sense channels on the ADG73 multiplexers.
 * 
 * @param src_pos The positive source channel.
 * @param src_neg The negative source channel.
 * @param sense_pos The positive sense channel.
 * @param sense_neg The negative sense channel.
 * @return 0 on success, or an error code.
 */
int set_src_sense_ADG73(uint8_t src_pos, uint8_t src_neg, uint8_t sense_pos, uint8_t sense_neg);
