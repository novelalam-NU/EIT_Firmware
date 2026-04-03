#include "hardware.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/spi_master.h"
#include "../Device_Drivers/AD5270_DigiPot.h"
#include "../Device_Drivers/AD5930_SigGen.h"
#include "../Device_Drivers/ADG73_MUX.h"
#include "../Device_Drivers/AD7450_ADC.h"
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
    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_DISABLED);
    if (ret == ESP_OK) {
        #if DEBUG
        ESP_LOGI(TAG, "SPI bus initialized successfully");
        #endif
    } else if (ret == ESP_ERR_INVALID_STATE) {
        ESP_LOGD(TAG, "SPI bus already initialized");
    } else {
        #if DEBUG
        ESP_LOGE(TAG, "Failed to init SPI bus: %s", esp_err_to_name(ret));
        #endif
        return ret;
    }
    return ESP_OK;
}

int signal_gen_start(float freq) {
    return AD5930_init(freq);
}

int set_src_inamp_gain(uint16_t src_gain) {
    if ( ad5270_set_wiper(src_gain, SRC_INAMP_HANDLE) != 0 ) {
        #if DEBUG
        ESP_LOGE(TAG, "Failed to set wiper");
        #endif
    } 
    #if DEBUG
    ESP_LOGI(TAG, "set_src_inamp_gain called with src_gain=%u", src_gain);
    #endif
    return ESP_OK;
}

int set_sense_inamp_gain(uint16_t sense_gain) {
    if ( ad5270_set_wiper(sense_gain, SENSE_INAMP_HANDLE) != 0 ) {
        #if DEBUG
        ESP_LOGE(TAG, "Failed to set wiper");
        #endif
    } 
    #if DEBUG
    ESP_LOGI(TAG, "set_sense_inamp_gain called with sense_gain=%u", sense_gain);
    #endif
    return ESP_OK;
}

int adcRead(uint16_t *buf, size_t len) {
    #if DEBUG
    ESP_LOGI(TAG, "adcRead called with buffer length=%zu", len);
    #endif

    if (AD7450_Read(buf, len) != 0) {
        ESP_LOGI(TAG, "AD7450 read failed");
        return -1;
    }

    return ESP_OK;
}

uint16_t calc_std_dev_mag(int16_t* buf, uint16_t buf_len, float multiplier) {
    if (buf_len == 0) return 0;

    // 1. Calculate Mean using 32-bit integer for speed
    int32_t total_sum = 0;
    for (uint16_t i = 0; i < buf_len; i++) {
        total_sum += buf[i];
        }
        float mean = (float)total_sum / (float)buf_len;

        // 2. Calculate Mean Absolute Deviation
        float deviation_sum = 0;
        for (uint16_t i = 0; i < buf_len; i++) {
            float diff = (float)buf[i] - mean;
            deviation_sum += fabsf(diff);
        }
        float mad = deviation_sum / (float)buf_len;

        // 3. Return MAD as amplitude measure
        float peak_to_peak = mad;

        // 4. Return as rounded integer
        return (uint16_t)roundf(peak_to_peak);
    }

int init_inamp_pots() {
    if ( ad5270_init( SRC_INAMP_HANDLE ) != 0) {
        #if DEBUG
        ESP_LOGE(TAG, "Failed to init SRC_INAMP_HANDLE");
        #endif
        return -1;
    }

    if ( ad5270_init( SENSE_INAMP_HANDLE ) != 0) {
        #if DEBUG
        ESP_LOGE(TAG, "Failed to init SENSE_INAMP_HANDLE");
        #endif
        return -1;
    }
    
    #if DEBUG
    ESP_LOGI(TAG, " Pots initialized");
    #endif
    return ESP_OK;
}

int set_mux(uint8_t src_pos, uint8_t src_neg, uint8_t sense_pos, uint8_t sense_neg) {

    // ESP_LOGI(TAG, "set_mux called with src_pos=%u, src_neg=%u, sense_pos=%u, sense_neg=%u",
            // src_pos, src_neg, sense_pos, sense_neg);

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
        #if DEBUG
        ESP_LOGE(TAG, "Failed to initialize MUX: %s", esp_err_to_name(ret));
        #endif
        return ret;
    }
    #if DEBUG
    ESP_LOGI(TAG, "MUX initialized successfully");
    #endif
    return ESP_OK;
}

int adc_init(void) {
    return AD7450_init();
}


uint16_t calc_peak_to_peak(void) {
    uint16_t raw[ADC_BUF_LEN];

    if (adcRead(raw, ADC_BUF_LEN) != 0) {
        ESP_LOGE(TAG, "Failed to read ADC samples");
        return 0;
    }

    return calc_std_dev_mag((int16_t *)raw, ADC_BUF_LEN, 1);
}