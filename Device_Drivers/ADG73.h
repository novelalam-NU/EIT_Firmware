#pragma once

#include <stdint.h>

#include "../Middle_Ware/hardware.h"
#include "driver/spi_master.h"
#include "driver/spi_common.h"
#include "esp_log.h"





int init_src_sense_ADG73();


int set_src_sense_ADG73(uint8_t src_pos, uint8_t src_neg, uint8_t sense_pos, uint8_t sense_neg);