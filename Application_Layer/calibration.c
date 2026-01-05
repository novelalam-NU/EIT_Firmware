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
uint16_t max_calibrated_sense_rdata = 512; //holds the max gain intially than best gain  


//have better logic to detect clippimng besides just seeing amplitude went down 



/* 2d array to hold calibration values and electrode mappings */
Calibration_t calibration_table[NUM_ELECTRODE_PAIRS][NUM_SENSE_PAIRS] = {
    [0][0] = {.src_pos = 1, .src_neg = 2, .sense_pos = 3, .sense_neg = 4},
    [0][1] = {.src_pos = 1, .src_neg = 2, .sense_pos = 4, .sense_neg = 5},
    [0][2] = {.src_pos = 1, .src_neg = 2, .sense_pos = 5, .sense_neg = 6},
    [0][3] = {.src_pos = 1, .src_neg = 2, .sense_pos = 6, .sense_neg = 7},
    [0][4] = {.src_pos = 1, .src_neg = 2, .sense_pos = 7, .sense_neg = 8},
    [1][0] = {.src_pos = 2, .src_neg = 3, .sense_pos = 4, .sense_neg = 5},
    [1][1] = {.src_pos = 2, .src_neg = 3, .sense_pos = 5, .sense_neg = 6},
    [1][2] = {.src_pos = 2, .src_neg = 3, .sense_pos = 6, .sense_neg = 7},
    [1][3] = {.src_pos = 2, .src_neg = 3, .sense_pos = 7, .sense_neg = 8},
    [1][4] = {.src_pos = 2, .src_neg = 3, .sense_pos = 8, .sense_neg = 1},
    [2][0] = {.src_pos = 3, .src_neg = 4, .sense_pos = 5, .sense_neg = 6},
    [2][1] = {.src_pos = 3, .src_neg = 4, .sense_pos = 6, .sense_neg = 7},
    [2][2] = {.src_pos = 3, .src_neg = 4, .sense_pos = 7, .sense_neg = 8},
    [2][3] = {.src_pos = 3, .src_neg = 4, .sense_pos = 8, .sense_neg = 1},
    [2][4] = {.src_pos = 3, .src_neg = 4, .sense_pos = 1, .sense_neg = 2},
    [3][0] = {.src_pos = 4, .src_neg = 5, .sense_pos = 6, .sense_neg = 7},
    [3][1] = {.src_pos = 4, .src_neg = 5, .sense_pos = 7, .sense_neg = 8},
    [3][2] = {.src_pos = 4, .src_neg = 5, .sense_pos = 8, .sense_neg = 1},
    [3][3] = {.src_pos = 4, .src_neg = 5, .sense_pos = 1, .sense_neg = 2},
    [3][4] = {.src_pos = 4, .src_neg = 5, .sense_pos = 2, .sense_neg = 3},
    [4][0] = {.src_pos = 5, .src_neg = 6, .sense_pos = 7, .sense_neg = 8},
    [4][1] = {.src_pos = 5, .src_neg = 6, .sense_pos = 8, .sense_neg = 1},
    [4][2] = {.src_pos = 5, .src_neg = 6, .sense_pos = 1, .sense_neg = 2},
    [4][3] = {.src_pos = 5, .src_neg = 6, .sense_pos = 2, .sense_neg = 3},
    [4][4] = {.src_pos = 5, .src_neg = 6, .sense_pos = 3, .sense_neg = 4},
    [5][0] = {.src_pos = 6, .src_neg = 7, .sense_pos = 8, .sense_neg = 1},
    [5][1] = {.src_pos = 6, .src_neg = 7, .sense_pos = 1, .sense_neg = 2},
    [5][2] = {.src_pos = 6, .src_neg = 7, .sense_pos = 2, .sense_neg = 3},
    [5][3] = {.src_pos = 6, .src_neg = 7, .sense_pos = 3, .sense_neg = 4},
    [5][4] = {.src_pos = 6, .src_neg = 7, .sense_pos = 4, .sense_neg = 5},
    [6][0] = {.src_pos = 7, .src_neg = 8, .sense_pos = 1, .sense_neg = 2},
    [6][1] = {.src_pos = 7, .src_neg = 8, .sense_pos = 2, .sense_neg = 3},
    [6][2] = {.src_pos = 7, .src_neg = 8, .sense_pos = 3, .sense_neg = 4},
    [6][3] = {.src_pos = 7, .src_neg = 8, .sense_pos = 4, .sense_neg = 5},
    [6][4] = {.src_pos = 7, .src_neg = 8, .sense_pos = 5, .sense_neg = 6},
    [7][0] = {.src_pos = 8, .src_neg = 1, .sense_pos = 2, .sense_neg = 3},
    [7][1] = {.src_pos = 8, .src_neg = 1, .sense_pos = 3, .sense_neg = 4},
    [7][2] = {.src_pos = 8, .src_neg = 1, .sense_pos = 4, .sense_neg = 5},
    [7][3] = {.src_pos = 8, .src_neg = 1, .sense_pos = 5, .sense_neg = 6},
    [7][4] = {.src_pos = 8, .src_neg = 1, .sense_pos = 6, .sense_neg = 7},
};




static int calibrate_gain_pair(Calibration_t *cal)
{
    

    /* Used for ADC and DSP */
    int16_t buffer[BUFFER_LEN];
    size_t buffer_len = sizeof(buffer) / sizeof(buffer[0]);

    
    if (cal == NULL) {
        
        return ESP_FAIL;
    }


    /* Iterate through sense gain values from zero to current highest gain value before clipping */
        for (uint16_t inAmp_sense_gain = 100; inAmp_sense_gain < max_calibrated_sense_rdata; inAmp_sense_gain+=10) {

            if (set_sense_inamp_gain(inAmp_sense_gain) != ESP_OK) {
                
                continue;
            }
            
            /* Some delay if required */
            vTaskDelay(1); //so watch dog task not starved

            /* ADC read into a buffer */
            if (adcRead((int16_t*)buffer, buffer_len, inAmp_sense_gain) != ESP_OK) {
                
                continue;
            }

            /* If clipping detected break out of the loop and set max_calibrated_sense_rdata to inAmp_sense_gain*/
            bool clip_detected = detect_opamp_clipping(buffer, buffer_len, CLIP_THRESHOLD, CLIP_MIN_SAMPLES, CLIP_MAX_INDEX);
            
            // Check if clipping was detected in the current buffer
            if (clip_detected) {
                // Set the maximum calibrated sense gain to the current value before clipping occurred
                max_calibrated_sense_rdata = inAmp_sense_gain;
                // Store the amplitude at this gain level as the reference amplitude
                // Exit the loop since we've found the maximum safe gain

                ////printf("\n\nClip detected at gain level: %u\n", inAmp_sense_gain);
                vTaskDelay(100);
                
                break;
            } 
            // Handle the last iteration: ensure reference amplitude is saved at maximum gain
            else  {
                continue;
            }

           
        }

    return ESP_OK;
}


void calibration_task(void* args) {
    

    srand(time(NULL));
    

    
    /* Intizlized Mux and set to 0,0 */
    //do this 
    

    vTaskDelay(pdMS_TO_TICKS(1000));
    

    if ( set_src_inamp_gain(SCR_RDATA_CONST) != ESP_OK) {
        #if DEBUG
        ESP_LOGE(TAG, "Failed to set source inamp gain");
        #endif
        vTaskDelete(NULL);
        return;
    }
    

    

    for (uint8_t src_elec_pair = 0; src_elec_pair < NUM_ELECTRODE_PAIRS; src_elec_pair++ ) {
        
        for (uint8_t sense_elec_pair = 0; sense_elec_pair < NUM_SENSE_PAIRS; sense_elec_pair++ ) {
            

            /* Temp pointer to elment in 2d config array */
            Calibration_t* curr_config = &calibration_table[src_elec_pair][sense_elec_pair];

            /* Set mux to src_elec_pair, sense_elec_pair */
            if ( set_mux( curr_config->src_pos,
                          curr_config->src_neg,
                          curr_config->sense_pos,
                          curr_config->sense_neg ) != ESP_OK ) {
                #if DEBUG
                ESP_LOGE(TAG, "Failed to set mux for pair [%u][%u]", src_elec_pair, sense_elec_pair);
                #endif
                continue;
            }
            

            if (calibrate_gain_pair(curr_config) != ESP_OK) {
                #if DEBUG
                ESP_LOGE(TAG, "Failed to calibrate gain for pair [%u][%u]", src_elec_pair, sense_elec_pair);
                #endif
                continue;
            }
                // handle calibration failure
            
            ////printf("Current max_calibrated_sense_rdata: %u\n", max_calibrated_sense_rdata);
            


            ESP_LOGI(TAG, "Calibration[%u][%u]: reference_amp=%u",
                src_elec_pair, sense_elec_pair,
                curr_config->reference_amp);
            

             

        }
    }



    /* Set the calibrated sense gain once for all pairs */
    if (set_sense_inamp_gain(max_calibrated_sense_rdata) != ESP_OK) {
        #if DEBUG
        ESP_LOGE(TAG, "Failed to set sense gain for tare measurement");
        #endif
    }
    
    vTaskDelay(1);
    
    /* Now that we have the best sense gain, find the tared value for each pair when hand is at rest */
    for (uint8_t src_elec_pair = 0; src_elec_pair < NUM_ELECTRODE_PAIRS; src_elec_pair++) {
        for (uint8_t sense_elec_pair = 0; sense_elec_pair < NUM_SENSE_PAIRS; sense_elec_pair++) {
            Calibration_t* curr_config = &calibration_table[src_elec_pair][sense_elec_pair];
            
            /* Set mux to src_elec_pair, sense_elec_pair */
            if (set_mux(curr_config->src_pos,
                        curr_config->src_neg,
                        curr_config->sense_pos,
                        curr_config->sense_neg) != ESP_OK) {
                #if DEBUG
                ESP_LOGE(TAG, "Failed to set mux for tare pair [%u][%u]", src_elec_pair, sense_elec_pair);
                #endif
                continue;
            }
            
            vTaskDelay(1);
            
            /* Read tare value */
            int16_t tare_buffer[BUFFER_LEN];
            size_t buffer_len = sizeof(tare_buffer) / sizeof(tare_buffer[0]);
            if (adcRead(tare_buffer, buffer_len, max_calibrated_sense_rdata) != ESP_OK) {
                #if DEBUG
                ESP_LOGE(TAG, "Failed to read tare for pair [%u][%u]", src_elec_pair, sense_elec_pair);
                #endif
                continue;
            }
            
            /* Store tare value in calibration table reference_amp */
            curr_config->reference_amp = dsp_freq_amp(tare_buffer, buffer_len, CALIBRATION_DSP_BUCKET, CALIBRATION_DSP_BUCKET);
            ESP_LOGI(TAG, "Tare[%u][%u] complete", src_elec_pair, sense_elec_pair);
        }
    }



    
    /* Signal to the measurement task */
    if ( xSemaphoreGive(sem_cal_to_meas) != pdTRUE ) {
        
        #if DEBUG
        ESP_LOGE(TAG, "Failed to give semaphore");
        #endif
    }
    

    vTaskDelete(NULL);
    

}