#include <stdio.h>
#include <string.h>
#include "measurement.h"
#include "calibration.h"
#include "hardware.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"


static const char *TAG = "MEASUREMENT";

/* Initially cleared out*/
int16_t adc_packet_buffers[MAX_ADC_PACKETS][ADC_READINGS_PER_PACKET] = {0};

void measurement_task(void* args) {

    /* Wait for calibration task to end */
    if ( xSemaphoreTake( sem_cal_to_meas, portMAX_DELAY ) != pdPASS ) {
        ESP_LOGE(TAG, "Semaphore failed");
    }
    /* Holds the magnitude of fft for target frequncy for that electrode configuration*/
    int16_t amps[NUM_ELECTRODE_PAIRS * NUM_SENSE_PAIRS];

    while (1) {
        int idx = 0; //reset index
        
        // Ensure gains are set
        set_src_inamp_gain(SCR_RDATA_CONST);
        set_sense_inamp_gain(max_calibrated_sense_rdata);

        for (uint8_t src_elec_pair = 0; src_elec_pair < NUM_ELECTRODE_PAIRS; src_elec_pair++) {
            for (uint8_t sense_elec_pair = 0; sense_elec_pair < NUM_SENSE_PAIRS; sense_elec_pair++) {
                Calibration_t* curr_config = &calibration_table[src_elec_pair][sense_elec_pair];

                if (set_mux(curr_config->src_pos, curr_config->src_neg, 
                            curr_config->sense_pos, curr_config->sense_neg) != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to set mux");
                    continue;
                }

                // Small delay for settling
                vTaskDelay(pdMS_TO_TICKS(1));

                if (adcRead((uint16_t*)adc_packet_buffers[idx], ADC_READINGS_PER_PACKET) != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to read ADC");
                    continue;
                }

            //will run on core 1
                uint16_t amplitude = dsp_freq_amp(adc_packet_buffers[idx], ADC_READINGS_PER_PACKET);
                
               // Calculate difference: calibration_table->reference_amp - amplitude
                if (idx < (NUM_ELECTRODE_PAIRS * NUM_SENSE_PAIRS)) {
                    amps[idx] = (int16_t)curr_config->reference_amp - (int16_t)amplitude;
                    idx++;
                } 
            }
        }

        // Print results
        ESP_LOGI(TAG, "Measurement Cycle Complete. Amps:");
        for (int i = 0; i < (NUM_ELECTRODE_PAIRS * NUM_SENSE_PAIRS); i++) {
            int src_pair = i / NUM_SENSE_PAIRS;
            int sense_pair = i % NUM_SENSE_PAIRS;
            ESP_LOGI(TAG, "Src:%u Sense:%u -> %d ", src_pair, sense_pair, amps[i]);
        }
        ESP_LOGI(TAG, "End of Cycle");

        vTaskDelay(pdMS_TO_TICKS(1000)); // Run every second
    }
}
