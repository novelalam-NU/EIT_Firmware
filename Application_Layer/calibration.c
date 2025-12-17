#include <stdio.h>
#include "calibration.h"
#include "esp_err.h"
#include <stdlib.h>
#include <stdint.h>
#include <time.h> //for debug purposes


const uint16_t src_rdata = 200; //Fixed source gain value
uint16_t max_calibrated_sense_rdata = (uint16_t)MAX_GAIN;


/* 2d array to hold calibration values and electrode mappings */
Calibration_t calibration_table [NUM_ELECTRODE_PAIRS][NUM_SENSE_PAIRS] = {0};



static int calibrate_gain_pair(Calibration_t *cal)
{
    /* Used for ADC and DSP */
    uint8_t buffer[256];
    size_t buffer_len = sizeof(buffer);

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
            if (adcRead(buffer, buffer_len) != ESP_OK) {
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
        
        uint8_t buffer[256];
        if (adcRead(buffer, sizeof(buffer)) != ESP_OK) {
            return ESP_FAIL;
        }
        
        cal->reference_amp = dsp_freq_amp(buffer, sizeof(buffer)); // Get amplitude at max gain
    }
    

    return ESP_OK;
}


void calibration_task(void* args) {
    printf("Calibration done\n");

    srand(time(NULL));

    
    /* Intizlized Mux and set to 0,0 */
    
    if ( init_mux() != ESP_OK) {
        
    }

    vTaskDelay(pdMS_TO_TICKS(1000));

    if ( set_src_inamp_gain(src_rdata) != ESP_OK) {

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


            printf("Calibration[%u][%u]: reference_amp=%u\n",
                src_elec_pair, sense_elec_pair,
                curr_config->reference_amp);

             vTaskDelay(100);

        }
    }

    

    printf("\n=== Calibration Table Summary ===\n");
    for (uint8_t i = 0; i < NUM_ELECTRODE_PAIRS; i++) {
        for (uint8_t j = 0; j < NUM_SENSE_PAIRS; j++) {
            printf("[%u][%u] ref_amp=%u\n",
                   i, j,
                   calibration_table[i][j].reference_amp);
        }
    }


    printf("Final calibrated sense gain: %u\n", max_calibrated_sense_rdata);





    /* Signal to the measurement task */
    if ( xSemaphoreGive(sem_cal_to_meas) != pdTRUE ) {
        printf("Failed to give semaphore\n");
    }


    vTaskDelete(NULL);

}


int set_src_inamp_gain(uint16_t src_gain) {
    printf("set_src_inamp_gain called with src_gain=%u\n", src_gain);
    return ESP_OK;
}

int set_sense_inamp_gain(uint16_t sense_gain) {
    printf("set_sense_inamp_gain called with sense_gain=%u\n", sense_gain);
    return ESP_OK;
}


int adcRead(uint8_t *buf, size_t len) {
    printf("adcRead called with buffer length=%zu\n", len);
    return ESP_OK;
}

uint16_t dsp_freq_amp(uint16_t *buf, size_t len) {
    printf("dsp_freq_amp called with buffer length=%zu\n", len);
    return (uint16_t)(rand() % 1000);
}

int init_mux(void) {
    printf("init_mux called\n");
    return ESP_OK;
}

int set_mux(uint8_t src_pos, uint8_t src_neg, uint8_t sense_pos, uint8_t sense_neg) {
    printf("set_mux called with src_pos=%u, src_neg=%u, sense_pos=%u, sense_neg=%u\n",
           src_pos, src_neg, sense_pos, sense_neg);
    return ESP_OK;
}