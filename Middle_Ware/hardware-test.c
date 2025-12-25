#include "hardware-test.h"
#include "hardware.h"
#include "esp_log.h"
#include "../Device_Drivers/AD7450_ADC.h"
#include "test_data_gen.h"

#define TARGET_BUCKET 7

static const char *TAG = "HARDWARE_TEST";

int test_adc(void) {
    uint16_t buf[70];
    for (int i = 0; i < 64; i++) {
        buf[i] = i;
    }

    if ( AD7450_Read(buf, 64) != 0) {
        ESP_LOGE(TAG, "test_adc failed");
        return -1;
    }
    ESP_LOGI(TAG, "test_adc passed");
    return 0;
}

int test_signal_gen(void) {
    // Dummy test for signal generator
    float freq = 1000.0f;
    if (signal_gen_start(freq) != 0) {
        ESP_LOGE(TAG, "test_signal_gen failed");
        return -1;
    }
    ESP_LOGI(TAG, "test_signal_gen passed");
    return 0;
}

int test_inamp_pots(void) {
    // Dummy test for inamp pots
    if (init_inamp_pots() != 0) {
        ESP_LOGE(TAG, "test_inamp_pots init failed");
        return -1;
    }
    
    if (set_src_inamp_gain(512) != 0) {
        ESP_LOGE(TAG, "test_inamp_pots set src gain failed");
        return -1;
    }

    if (set_sense_inamp_gain(512) != 0) {
        ESP_LOGE(TAG, "test_inamp_pots set sense gain failed");
        return -1;
    }

    ESP_LOGI(TAG, "test_inamp_pots passed");
    return 0;
}

int test_mux(void) {
    // Dummy test for mux
    if (init_mux() != 0) {
        ESP_LOGE(TAG, "test_mux init failed");
        return -1;
    }

    if (set_mux(1, 2, 3, 4) != 0) {
        ESP_LOGE(TAG, "test_mux set failed");
        return -1;
    }
    ESP_LOGI(TAG, "test_mux passed");
    return 0;
}



int test_dsp(bool clipped, float clip_percent) {
    int16_t samples[TEST_GEN_N];
    float test_freq = 54000.0f; // 20 kHz test signal

    // ESP_LOGI(TAG, "test_dsp: clipped=%d, clip_percent=%0.2f", clipped, clip_percent);
    
    //generate_sine_int16(samples, test_freq);
    generate_sine_int16_multi_random_amp_clipped(samples, test_freq, 3.0f, 5.0f, clipped, clip_percent);
    
    uint16_t amplitude = (uint16_t)dsp_freq_amp(samples, TEST_GEN_N, TARGET_BUCKET, TARGET_BUCKET);
    ESP_LOGI(TAG, "test_dsp: Generated %0.1f Hz, Amplitude: %u", test_freq, amplitude);
    

    bool clip_detected = detect_opamp_clipping(samples, TEST_GEN_N, 100, 10, 31);

    ESP_LOGI(TAG, "test_dsp: clip_detected=%d, clipped=%d, clip_percent=%0.2f", clip_detected, clipped, clip_percent);


    return 0;
}
