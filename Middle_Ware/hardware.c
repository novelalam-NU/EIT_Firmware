#include "hardware.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_err.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "../Device_Drivers/AD5270_DigiPot.h"
#include "../Device_Drivers/AD5930_SigGen.h"
#include "../Device_Drivers/ADG73_MUX.h"
#include "../Device_Drivers/AD7450_ADC.h"
#include "../Middle_Ware/test_data_gen.h"
#include "esp_dsp.h"
#include <math.h>



#define ESP_OK 0

static const char *TAG = "HARDWARE";



/* Bus Configuration*/
static const spi_bus_config_t buscfg = {
    .mosi_io_num = PIN_SPI_MOSI,
    .miso_io_num = PIN_SPI_MISO,
    .sclk_io_num = PIN_SPI_SCLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 16,
};

int init_spi(void) {
    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "SPI bus initialized successfully");
    } else if (ret == ESP_ERR_INVALID_STATE) {
        ESP_LOGD(TAG, "SPI bus already initialized");
    } else {
        ESP_LOGE(TAG, "Failed to init SPI bus: %s", esp_err_to_name(ret));
        return ret;
    }
    return ESP_OK;
}

int signal_gen_start(float freq) {
    return AD5930_init(freq);
}

int set_src_inamp_gain(uint16_t src_gain) {
    if ( ad5270_set_wiper(src_gain, SRC_INAMP_HANDLE) != 0 ) {
        ESP_LOGE(TAG, "Failed to set wiper");
    } 
    ESP_LOGI(TAG, "set_src_inamp_gain called with src_gain=%u", src_gain);
    return ESP_OK;
}

int set_sense_inamp_gain(uint16_t sense_gain) {
    if ( ad5270_set_wiper(sense_gain, SENSE_INAMP_HANDLE) != 0 ) {
        ESP_LOGE(TAG, "Failed to set wiper");
    } 
    ESP_LOGI(TAG, "set_sense_inamp_gain called with sense_gain=%u", sense_gain);
    return ESP_OK;
}

#define ADC_MOCK
int adcRead(int16_t *buf, size_t len, uint16_t gain) {
    ESP_LOGI(TAG, "adcRead called with buffer length=%zu", len);

    #ifdef ADC_MOCK
    if (gain >= 250) {
        bool clipped = (rand() % 100) < 5;
        generate_sine_int16_multi_random_amp_clipped(buf, 50000.0f, 0, 0, clipped, 0.98f);

    } else {
        generate_sine_int16_multi_random_amp_clipped(buf, 50000.0f, 0, 0, false, 1.0f);

    }
    #else
        if ( AD7450_Read(buf, len) != 0) {
            ESP_LOGI(TAG, "test_adc failed");
            return -1;
        }
    #endif

    return ESP_OK;
}

static int16_t w[64];
uint32_t dsp_freq_amp(int16_t *buf, size_t len, uint8_t begin, uint8_t end) {
    ESP_LOGI(TAG, "dsp_freq_amp called with buffer length=%zu, begin=%u, end=%u", len, begin, end);

    /* Add in the imagninary component and apply Hanning window */
    int16_t real_and_imagine[128] = {0};

    for (int i = 0; i < len; i++) {
        float window = 0.5f - 0.5f * cosf(2.0f * M_PI * i / (len - 1));
        real_and_imagine[2*i] = (int16_t)(buf[i] * window);
        real_and_imagine[2*i+1] = 0;
    }
    
    /* Initialize the FFT sine/cos tables once */
    static bool fft_initialized = false;
    if (!fft_initialized) {
        if (dsps_fft2r_init_sc16(w, 64) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to init sine/cos tables");
            return 0;
        }
        fft_initialized = true;
    }
    ESP_LOGI(TAG, "FFT sine/cos tables initialized");

    /* Run the actual FFT */
    
    if ( dsps_fft2r_sc16_ansi_(real_and_imagine, len, w) != ESP_OK ) {
        ESP_LOGE(TAG, "Failed to run FFT");
        return 0;
    }
    ESP_LOGI(TAG, "FFT computation completed");

    if ( dsps_bit_rev_sc16_ansi(real_and_imagine, len) != ESP_OK ) {
         ESP_LOGE(TAG, "Failed to reverse FFT");
        return 0;
    }
    ESP_LOGI(TAG, "Bit reversal completed");

    uint32_t accumulated_mag = 0;
    uint32_t max_mag = 0;
    uint32_t mag6 = 0, mag7 = 0, mag8 = 0;
    uint32_t high_freq_sum = 0;

    for (size_t k = 0; k < len / 2; k++) {
        int32_t re = real_and_imagine[2 * k];
        int32_t im = real_and_imagine[2 * k + 1];

        uint32_t mag = (uint32_t)sqrtf((float)(re * re + im * im));
        
        if (k >= begin && k <= end) {
            accumulated_mag += mag;
        }

        if (mag > max_mag) {
            max_mag = mag;
        }

        if (k == 6) mag6 = mag;
        if (k == 7) mag7 = mag;
        if (k == 8) mag8 = mag;

        if (k >= 12) {
            high_freq_sum += mag;
        }

       // printf("bin[%zu] magnitude = %lu\n", k, mag);
    }

    printf("Sum of bins 12-31: %lu\n", high_freq_sum);

    // if (mag7 > 0) {
    //     printf("Ratio Bin 6/7: %f\n", (float)mag6 / mag7);
    //     printf("Ratio Bin 8/7: %f\n", (float)mag8 / mag7);
    // }
    // if (mag6 > 0) {
    //      printf("Ratio Bin 8/6: %f\n", (float)mag8 / mag6);
    // }

    ESP_LOGI(TAG, "DSP frequency amplitude calculation finished");

    //printf("Max magnitude: %lu at bin 7\n", max_mag);
    return accumulated_mag;
}




int init_inamp_pots() {
    if ( ad5270_init( SRC_INAMP_HANDLE ) != 0) {
        ESP_LOGE(TAG, "Failed to init SRC_INAMP_HANDLE");
        return -1;
    }

    if ( ad5270_init( SENSE_INAMP_HANDLE ) != 0) {
        ESP_LOGE(TAG, "Failed to init SENSE_INAMP_HANDLE");
        return -1;
    }
    
    ESP_LOGI(TAG, " Pots initialized");
    return ESP_OK;
}

int set_mux(uint8_t src_pos, uint8_t src_neg, uint8_t sense_pos, uint8_t sense_neg) {

    ESP_LOGI(TAG, "set_mux called with src_pos=%u, src_neg=%u, sense_pos=%u, sense_neg=%u",
             src_pos, src_neg, sense_pos, sense_neg);

           esp_err_t ret = set_src_sense_ADG73(src_pos, src_neg, sense_pos, sense_neg);


            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to set mux: %s", esp_err_to_name(ret));
                return ret;
            }
    return ESP_OK;
}
int init_mux(void) {
    esp_err_t ret = init_src_sense_ADG73();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize MUX: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "MUX initialized successfully");
    return ESP_OK;
}

int adc_init(void) {
    return AD7450_init();
}


bool detect_opamp_clipping(int16_t *buf, size_t len, uint32_t threshold, uint8_t begin, uint8_t end) {
    uint32_t accumulated_mag = dsp_freq_amp(buf, len, begin, end);
    if (accumulated_mag > threshold) {
        return true;
    }
    return false;
}
