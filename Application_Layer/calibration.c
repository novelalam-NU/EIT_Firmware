#include <stdio.h>
#include "calibration.h"
#include "hardware.h"
#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "CALIBRATION";

const uint16_t SCR_RDATA_CONST = 300; //Fixed source gain value
const uint16_t SNS_RDATA_CONST = 60; //Fixed sense gain value

uint16_t ewma_amp[EWMA_AMP_COUNT] = {0};

/* 2d array to hold electrode mappings */
Calibration_t pair_calibration_map[NUM_ELECTRODE - NUM_ELECTRODE_INJECT][NUM_ELECTRODE - NUM_ELECTRODE_INJECT-NUM_ELECTRODE_SINK-1] = {

    [0][0] = {.src_pos = 1, .src_neg = 2, .sense_pos = 3, .sense_neg = 4},
    [0][1] = {.src_pos = 1, .src_neg = 2, .sense_pos = 4, .sense_neg = 5},
    [0][2] = {.src_pos = 1, .src_neg = 2, .sense_pos = 5, .sense_neg = 6},
    [0][3] = {.src_pos = 1, .src_neg = 2, .sense_pos = 6, .sense_neg = 7},
    [0][4] = {.src_pos = 1, .src_neg = 2, .sense_pos = 7, .sense_neg = 8},

    [1][0] = {.src_pos = 1, .src_neg = 3, .sense_pos = 2, .sense_neg = 4},
    [1][1] = {.src_pos = 1, .src_neg = 3, .sense_pos = 4, .sense_neg = 5},
    [1][2] = {.src_pos = 1, .src_neg = 3, .sense_pos = 5, .sense_neg = 6},
    [1][3] = {.src_pos = 1, .src_neg = 3, .sense_pos = 6, .sense_neg = 7},
    [1][4] = {.src_pos = 1, .src_neg = 3, .sense_pos = 7, .sense_neg = 8},

    [2][0] = {.src_pos = 1, .src_neg = 4, .sense_pos = 2, .sense_neg = 3},
    [2][1] = {.src_pos = 1, .src_neg = 4, .sense_pos = 3, .sense_neg = 5},
    [2][2] = {.src_pos = 1, .src_neg = 4, .sense_pos = 5, .sense_neg = 6},
    [2][3] = {.src_pos = 1, .src_neg = 4, .sense_pos = 6, .sense_neg = 7},
    [2][4] = {.src_pos = 1, .src_neg = 4, .sense_pos = 7, .sense_neg = 8},

    [3][0] = {.src_pos = 1, .src_neg = 5, .sense_pos = 2, .sense_neg = 3},
    [3][1] = {.src_pos = 1, .src_neg = 5, .sense_pos = 3, .sense_neg = 4},
    [3][2] = {.src_pos = 1, .src_neg = 5, .sense_pos = 4, .sense_neg = 5},
    [3][3] = {.src_pos = 1, .src_neg = 5, .sense_pos = 5, .sense_neg = 6},
    [3][4] = {.src_pos = 1, .src_neg = 5, .sense_pos = 6, .sense_neg = 7},

    [4][0] = {.src_pos = 1, .src_neg = 6, .sense_pos = 1, .sense_neg = 2},
    [4][1] = {.src_pos = 1, .src_neg = 6, .sense_pos = 2, .sense_neg = 3},
    [4][2] = {.src_pos = 1, .src_neg = 6, .sense_pos = 3, .sense_neg = 4},
    [4][3] = {.src_pos = 1, .src_neg = 6, .sense_pos = 4, .sense_neg = 5},
    [4][4] = {.src_pos = 1, .src_neg = 6, .sense_pos = 5, .sense_neg = 7},

    [5][0] = {.src_pos = 1, .src_neg = 7, .sense_pos = 1, .sense_neg = 2},
    [5][1] = {.src_pos = 1, .src_neg = 7, .sense_pos = 2, .sense_neg = 3},
    [5][2] = {.src_pos = 1, .src_neg = 7, .sense_pos = 3, .sense_neg = 4},
    [5][3] = {.src_pos = 1, .src_neg = 7, .sense_pos = 4, .sense_neg = 5},
    [5][4] = {.src_pos = 1, .src_neg = 7, .sense_pos = 5, .sense_neg = 6},

    [6][0] = {.src_pos = 1, .src_neg = 8, .sense_pos = 2, .sense_neg = 3},
    [6][1] = {.src_pos = 1, .src_neg = 8, .sense_pos = 3, .sense_neg = 4},
    [6][2] = {.src_pos = 1, .src_neg = 8, .sense_pos = 4, .sense_neg = 5},
    [6][3] = {.src_pos = 1, .src_neg = 8, .sense_pos = 5, .sense_neg = 6},
    [6][4] = {.src_pos = 1, .src_neg = 8, .sense_pos = 6, .sense_neg = 7},
   
};




void calibrate(void) {

    if (set_src_inamp_gain(SCR_RDATA_CONST) != ESP_OK) {
        #if DEBUG
        ESP_LOGE(TAG, "Failed to set source inamp gain");
        #endif
        return;
    }

    if (set_sense_inamp_gain(SNS_RDATA_CONST) != ESP_OK) {
        #if DEBUG
        ESP_LOGE(TAG, "Failed to set fixed sense inamp gain");
        #endif
        return;
    }
}