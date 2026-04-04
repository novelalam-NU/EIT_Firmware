#include <inttypes.h>
#include <stdio.h>
#include <math.h>

#include "measurement.h"
#include "tasks.h"
#include "calibration.h"
#include "hardware.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"

static const char *TAG = "MEASUREMENT";

#define TOTAL_MEASUREMENTS (NUM_ELECTRODE_PAIRS * NUM_SENSE_PAIRS)

/** Alpha for per-channel EWMA smoothing (0..1). Smaller = smoother. */
#define AMP_FILTER_ALPHA  0.8f

// #define PROFILE_SAMPLE_RATE
// #define PRINT_MEASUREMENTS

uint16_t adc_packet_buffers[MAX_ADC_PACKETS][ADC_READINGS_PER_PACKET] = {0};




void measurement_task(void* args) {
    const uint8_t total_measurements = NUM_ELECTRODE_PAIRS * NUM_SENSE_PAIRS;

    ESP_LOGI(TAG, "Measurement task starting");

#ifdef PROFILE_SAMPLE_RATE
    uint32_t prev_us = 0;
#endif

    set_src_inamp_gain(SCR_RDATA_CONST);
    set_sense_inamp_gain(SNS_RDATA_CONST);

    while (1) {
        int idx = 0;

        for (uint8_t src_elec_pair = 0; src_elec_pair < NUM_ELECTRODE_PAIRS; src_elec_pair++) {
            for (uint8_t sense_elec_pair = 0; sense_elec_pair < NUM_SENSE_PAIRS; sense_elec_pair++) {
                Calibration_t* curr_config = &pair_calibration_map[src_elec_pair][sense_elec_pair];
                const size_t ch = (size_t)src_elec_pair * NUM_SENSE_PAIRS + sense_elec_pair;

                if (set_mux(curr_config->src_pos, curr_config->src_neg,
                            curr_config->sense_pos, curr_config->sense_neg) != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to set mux");
                    continue;
                }
                uint16_t amplitude = calc_peak_to_peak();

                float prev = (float)ewma_amp[ch];
                float smoothed = (1.0f - AMP_FILTER_ALPHA) * prev + AMP_FILTER_ALPHA * (float)amplitude;
                ewma_amp[ch] = (uint16_t)smoothed;

                idx++;

            }

        }

        /* Send task notification to UDP task */
        xTaskNotifyGive( udp_task );
        

#ifdef PRINT_MEASUREMENTS
        for (uint8_t i = 0; i < total_measurements; i++) {
            printf("%u ", (unsigned)ewma_amp[i]);
        }
        printf("\n");
#endif

#ifdef PROFILE_SAMPLE_RATE
        uint32_t timestamp_us = (uint32_t)esp_timer_get_time();
        uint32_t delta_us = (prev_us > 0) ? (timestamp_us - prev_us) : 0;
        prev_us = timestamp_us;
        float freq_hz = (delta_us > 0) ? (1000000.0f / (float)delta_us) : 0.0f;
        printf("delta_us: %" PRIu32 ", freq: %.2f Hz\n", delta_us, freq_hz);


#endif

        



    }
}
