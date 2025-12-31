#include "hardware-test.h"
#include "hardware.h"
#include "esp_log.h"
#include "../Device_Drivers/AD7450_ADC.h"
#include "test_data_gen.h"
#include "freertos/FreeRTOS.h"

#define TARGET_BUCKET 7

static const char *TAG = "HARDWARE_TEST";

int test_adc(void) {
    int16_t buf[70];
    for (int i = 0; i < 64; i++) {
        buf[i] = i;
    }

    if ( AD7450_Read(buf, 64) != 0) {
        ////ESP_LOGE(TAG, "test_adc failed");
        return -1;
    }
    ESP_LOGI(TAG, "test_adc passed");
    return 0;
}

int test_signal_gen(void) {
    // Dummy test for signal generator
    float freq = 1000.0f;
    if (signal_gen_start(freq) != 0) {
        //ESP_LOGE(TAG, "test_signal_gen failed");
        return -1;
    }
    ESP_LOGI(TAG, "test_signal_gen passed");
    return 0;
}

int test_inamp_pots(void) {
    uint16_t in_amp_scr = 400;
    uint16_t in_amp_sense = 200;

    // Dummy test for inamp pots
    if (init_inamp_pots() != 0) {
        //ESP_LOGE(TAG, "test_inamp_pots init failed");
        return -1;
    }
    
    for (int i = 512; i > 0; i--) {
        vTaskDelay(pdMS_TO_TICKS(100));
        ESP_LOGI(TAG, "Setting src/sns gain to %d", i);
        if (set_src_inamp_gain(i) != 0) {
            ESP_LOGE(TAG, "test_inamp_pots set src gain failed");
            return -1;
        }
        
        if (set_sense_inamp_gain(i) != 0) {
            ESP_LOGE(TAG, "test_inamp_pots set sense gain failed");
            return -1;
        }
    }

    ESP_LOGI(TAG, "test_inamp_pots passed");
    return 0;
}

int test_mux(void) {
    // Dummy test for mux
    if (init_mux() != 0) {
        //ESP_LOGE(TAG, "test_mux init failed");
        return -1;
    }

    int ch1 = 5, ch2 = 5, ch3 = 5, ch4 = 5;
    for (int i = 0; i < 10000; i++) {
        if (set_mux(ch1, ch2, ch3, ch4) != 0) {
            //ESP_LOGE(TAG, "test_mux set failed");
            return -1;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
        ESP_LOGI(TAG, "test_mux passed");
    }
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

void test_function(void) {
    //test_adc();
    //test_signal_gen();
    // test_inamp_pots();
    test_mux();
      

    // test_dsp(false, 0.0f);
}
