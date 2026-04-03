#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"

#define NUM_ELECTRODE (8)
#define NUM_ELECTRODE_INJECT (1)
#define NUM_ELECTRODE_SINK (1)

#define NUM_ELECTRODE_PAIRS (NUM_ELECTRODE - NUM_ELECTRODE_INJECT)
#define NUM_SENSE_PAIRS (NUM_ELECTRODE - NUM_ELECTRODE_INJECT - NUM_ELECTRODE_SINK - 1)

#define EWMA_AMP_COUNT (NUM_ELECTRODE_PAIRS * NUM_SENSE_PAIRS)

void calibrate(void);

/* Structure that holds calibration values and electrode number mappings*/
// Structure to hold calibration values and electrode number mappings
typedef struct {
    const uint8_t src_pos;   // positive source electrode number
    const uint8_t src_neg;   // negative source electrode number

    const uint8_t sense_pos; // positive sense electrode number
    const uint8_t sense_neg; // negative sense electrode number
} Calibration_t;

extern const uint16_t SCR_RDATA_CONST; //Fixed source gain value
extern const uint16_t SNS_RDATA_CONST; //Fixed sense gain value
/* 2d array to hold calibration values and electrode mappings */
extern Calibration_t pair_calibration_map [NUM_ELECTRODE_PAIRS][NUM_SENSE_PAIRS];

/** Per-channel EWMA amplitude; index = src_elec_pair * NUM_SENSE_PAIRS + sense_elec_pair. */
extern uint16_t ewma_amp[EWMA_AMP_COUNT];




