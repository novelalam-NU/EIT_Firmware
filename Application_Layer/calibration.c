#include <stdio.h>
#include "calibration.h"
#include "hardware.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdlib.h>
#include <stdint.h>
#include <time.h> //for debug purposes

static const char *TAG = "CALIBRATION";

const uint16_t SCR_RDATA_CONST = 200; //Fixed source gain value
uint16_t max_calibrated_sense_rdata = (uint16_t)MAX_GAIN;


/* 2d array to hold calibration values and electrode mappings */
Calibration_t calibration_table[NUM_ELECTRODE_PAIRS][NUM_SENSE_PAIRS] = {
    [0][0] = {.src_pos = 0, .src_neg = 1, .sense_pos = 2, .sense_neg = 3},
    [0][1] = {.src_pos = 0, .src_neg = 1, .sense_pos = 3, .sense_neg = 4},
    [0][2] = {.src_pos = 0, .src_neg = 1, .sense_pos = 4, .sense_neg = 5},
    [0][3] = {.src_pos = 0, .src_neg = 1, .sense_pos = 5, .sense_neg = 6},
    [0][4] = {.src_pos = 0, .src_neg = 1, .sense_pos = 6, .sense_neg = 7},
    [1][0] = {.src_pos = 1, .src_neg = 2, .sense_pos = 3, .sense_neg = 4},
    [1][1] = {.src_pos = 1, .src_neg = 2, .sense_pos = 4, .sense_neg = 5},
    [1][2] = {.src_pos = 1, .src_neg = 2, .sense_pos = 5, .sense_neg = 6},
    [1][3] = {.src_pos = 1, .src_neg = 2, .sense_pos = 6, .sense_neg = 7},
    [1][4] = {.src_pos = 1, .src_neg = 2, .sense_pos = 7, .sense_neg = 0},
    [2][0] = {.src_pos = 2, .src_neg = 3, .sense_pos = 4, .sense_neg = 5},
    [2][1] = {.src_pos = 2, .src_neg = 3, .sense_pos = 5, .sense_neg = 6},
    [2][2] = {.src_pos = 2, .src_neg = 3, .sense_pos = 6, .sense_neg = 7},
    [2][3] = {.src_pos = 2, .src_neg = 3, .sense_pos = 7, .sense_neg = 0},
    [2][4] = {.src_pos = 2, .src_neg = 3, .sense_pos = 0, .sense_neg = 1},
    [3][0] = {.src_pos = 3, .src_neg = 4, .sense_pos = 5, .sense_neg = 6},
    [3][1] = {.src_pos = 3, .src_neg = 4, .sense_pos = 6, .sense_neg = 7},
    [3][2] = {.src_pos = 3, .src_neg = 4, .sense_pos = 7, .sense_neg = 0},
    [3][3] = {.src_pos = 3, .src_neg = 4, .sense_pos = 0, .sense_neg = 1},
    [3][4] = {.src_pos = 3, .src_neg = 4, .sense_pos = 1, .sense_neg = 2},
    [4][0] = {.src_pos = 4, .src_neg = 5, .sense_pos = 6, .sense_neg = 7},
    [4][1] = {.src_pos = 4, .src_neg = 5, .sense_pos = 7, .sense_neg = 0},
    [4][2] = {.src_pos = 4, .src_neg = 5, .sense_pos = 0, .sense_neg = 1},
    [4][3] = {.src_pos = 4, .src_neg = 5, .sense_pos = 1, .sense_neg = 2},
    [4][4] = {.src_pos = 4, .src_neg = 5, .sense_pos = 2, .sense_neg = 3},
    [5][0] = {.src_pos = 5, .src_neg = 6, .sense_pos = 7, .sense_neg = 0},
    [5][1] = {.src_pos = 5, .src_neg = 6, .sense_pos = 0, .sense_neg = 1},
    [5][2] = {.src_pos = 5, .src_neg = 6, .sense_pos = 1, .sense_neg = 2},
    [5][3] = {.src_pos = 5, .src_neg = 6, .sense_pos = 2, .sense_neg = 3},
    [5][4] = {.src_pos = 5, .src_neg = 6, .sense_pos = 3, .sense_neg = 4},
    [6][0] = {.src_pos = 6, .src_neg = 7, .sense_pos = 0, .sense_neg = 1},
    [6][1] = {.src_pos = 6, .src_neg = 7, .sense_pos = 1, .sense_neg = 2},
    [6][2] = {.src_pos = 6, .src_neg = 7, .sense_pos = 2, .sense_neg = 3},
    [6][3] = {.src_pos = 6, .src_neg = 7, .sense_pos = 3, .sense_neg = 4},
    [6][4] = {.src_pos = 6, .src_neg = 7, .sense_pos = 4, .sense_neg = 5},
    [7][0] = {.src_pos = 7, .src_neg = 0, .sense_pos = 1, .sense_neg = 2},
    [7][1] = {.src_pos = 7, .src_neg = 0, .sense_pos = 2, .sense_neg = 3},
    [7][2] = {.src_pos = 7, .src_neg = 0, .sense_pos = 3, .sense_neg = 4},
    [7][3] = {.src_pos = 7, .src_neg = 0, .sense_pos = 4, .sense_neg = 5},
    [7][4] = {.src_pos = 7, .src_neg = 0, .sense_pos = 5, .sense_neg = 6},
};






static int calibrate_gain_pair(Calibration_t *cal)
{
    /* Used for ADC and DSP */
    int16_t buffer[128];
    size_t buffer_len = sizeof(buffer) / sizeof(buffer[0]);

    if (cal == NULL) {
        return ESP_FAIL;
    }

    uint16_t max_amp = 0;
    uint16_t best_sense_gain = 0;

    /* Iterate through sense gain values from zero to current highest gain value before clipping */
        for (uint16_t inAmp_sense_gain = 0; inAmp_sense_gain < max_calibrated_sense_rdata; inAmp_sense_gain+=10) {

            if (set_sense_inamp_gain(inAmp_sense_gain) != ESP_OK) {
                continue;
            }
            
            /* Some delay if required */
            vTaskDelay(1); //so watch dog task not starved

            /* ADC read into a buffer */
            if (adcRead((uint16_t*)buffer, buffer_len) != ESP_OK) {
                continue;
            }

            /* Calculate amplitude at 50kHz */
            uint16_t amplitude = dsp_freq_amp(buffer, buffer_len);

            if (amplitude > max_amp) { //can replace with a threshold function
                max_amp = amplitude;
                best_sense_gain = inAmp_sense_gain;
            }
        }

    /* Replace if and only if new smaller gain requried */
    max_calibrated_sense_rdata =
        ( best_sense_gain < max_calibrated_sense_rdata )  ? 
        best_sense_gain : max_calibrated_sense_rdata;

    
    /* Set the tared voltage for electrode pair*/
    if ( best_sense_gain < max_calibrated_sense_rdata ) {
        cal->reference_amp = max_amp; //max_amp is amp at max_calibrated_sense_rdata
    } else { // Recalculate amp at max gain
        if (set_sense_inamp_gain(max_calibrated_sense_rdata) != ESP_OK) {
            return ESP_FAIL;
        }
        
        vTaskDelay(1); // Wait for stabilization
        
        int16_t buffer[128];
        size_t buffer_len = sizeof(buffer) / sizeof(buffer[0]);
        if (adcRead((uint16_t*)buffer, buffer_len) != ESP_OK) {
            return ESP_FAIL;
        }
        
        cal->reference_amp = dsp_freq_amp(buffer, buffer_len); // Get amplitude at max gain
    }
    

    return ESP_OK;
}


void calibration_task(void* args) {
    ESP_LOGI(TAG, "Calibration done");

    srand(time(NULL));

    
    /* Intizlized Mux and set to 0,0 */
    //do this 

    vTaskDelay(pdMS_TO_TICKS(1000));

    if ( set_src_inamp_gain(SCR_RDATA_CONST) != ESP_OK) {

    }

    

    for (uint8_t src_elec_pair = 0; src_elec_pair < NUM_ELECTRODE_PAIRS; src_elec_pair++ ) {
        for (uint8_t sense_elec_pair = 0; sense_elec_pair < NUM_SENSE_PAIRS; sense_elec_pair++ ) {
            /* Temp object */
            Calibration_t* curr_config = &calibration_table[src_elec_pair][sense_elec_pair];

            /* Set mux to src_elec_pair, sense_elec_pair */
            if ( set_mux( curr_config->src_pos,
                          curr_config->src_neg,
                          curr_config->sense_pos,
                          curr_config->sense_neg ) != ESP_OK ) {
             
            }

            if (calibrate_gain_pair(curr_config) != ESP_OK) {
                // handle calibration failure
            }


            ESP_LOGI(TAG, "Calibration[%u][%u]: reference_amp=%u",
                src_elec_pair, sense_elec_pair,
                curr_config->reference_amp);

             vTaskDelay(100);

        }
    }

    

    ESP_LOGI(TAG, "\n=== Calibration Table Summary ===");
    for (uint8_t i = 0; i < NUM_ELECTRODE_PAIRS; i++) {
        for (uint8_t j = 0; j < NUM_SENSE_PAIRS; j++) {
            ESP_LOGI(TAG, "[%u][%u] ref_amp=%u",
                   i, j,
                   calibration_table[i][j].reference_amp);
        }
    }


    ESP_LOGI(TAG, "Final calibrated sense gain: %u", max_calibrated_sense_rdata);





    /* Signal to the measurement task */
    if ( xSemaphoreGive(sem_cal_to_meas) != pdTRUE ) {
        ESP_LOGE(TAG, "Failed to give semaphore");
    }


    vTaskDelete(NULL);

}