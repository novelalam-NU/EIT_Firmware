#include <stdio.h>
#include <string.h>
#include "measurement.h"
#include "calibration.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ESP_OK 0

void measurement_task(void* args) {

    /* Wait for calibration task to end */
    if ( xSemaphoreTake( sem_cal_to_meas, portMAX_DELAY ) != pdPASS ) {
        printf("Semaphore failed\n");
    }

    uint16_t amps[NUM_ELECTRODE_PAIRS * NUM_SENSE_PAIRS];
    uint8_t buffer[256];
    size_t buffer_len = sizeof(buffer);

    while (1) {
        int idx = 0;
        
        // Ensure gains are set
        set_src_inamp_gain(src_rdata);
        set_sense_inamp_gain(max_calibrated_sense_rdata);

        for (uint8_t src_elec_pair = 0; src_elec_pair < NUM_ELECTRODE_PAIRS; src_elec_pair++) {
            for (uint8_t sense_elec_pair = 0; sense_elec_pair < NUM_SENSE_PAIRS; sense_elec_pair++) {
                Calibration_t* curr_config = &calibration_table[src_elec_pair][sense_elec_pair];

                if (set_mux(curr_config->src_pos, curr_config->src_neg, 
                            curr_config->sense_pos, curr_config->sense_neg) != ESP_OK) {
                    printf("Failed to set mux\n");
                    continue;
                }

                // Small delay for settling
                vTaskDelay(pdMS_TO_TICKS(1));

                if (adcRead(buffer, buffer_len) != ESP_OK) {
                    printf("Failed to read ADC\n");
                    continue;
                }

                uint16_t amplitude = dsp_freq_amp(buffer, buffer_len);
                
                // Calculate difference as requested: calibration_table->reference_amp - amplitude
                if (idx < (NUM_ELECTRODE_PAIRS * NUM_SENSE_PAIRS)) {
                    amps[idx] = curr_config->reference_amp - amplitude;
                    idx++;
                }
            }
        }

        // Print results
        printf("Measurement Cycle Complete. Amps:\n");
        for (int i = 0; i < (NUM_ELECTRODE_PAIRS * NUM_SENSE_PAIRS); i++) {
            int src_pair = i / NUM_SENSE_PAIRS;
            int sense_pair = i % NUM_SENSE_PAIRS;
            printf("Src:%u Sense:%u -> %u ", src_pair, sense_pair, amps[i]);
            if ((i + 1) % NUM_SENSE_PAIRS == 0) printf("\n");
        }
        printf("\n");

        vTaskDelay(pdMS_TO_TICKS(1000)); // Run every second
    }
}
