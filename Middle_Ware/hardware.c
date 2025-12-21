#include "hardware.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_err.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "../Device_Drivers/AD5270.h"
#include "../Device_Drivers/AD5930.h"
#include "../Device_Drivers/ADG73.h"

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

int adcRead(uint8_t *buf, size_t len) {
    ESP_LOGI(TAG, "adcRead called with buffer length=%zu", len);
    return ESP_OK;
}

uint16_t dsp_freq_amp(uint16_t *buf, size_t len) {
    ESP_LOGI(TAG, "dsp_freq_amp called with buffer length=%zu", len);
    return (uint16_t)(rand() % 1000);
}

int init_inamp_pots() {
    if ( ad5270_init( SRC_INAMP_HANDLE ) != 0) {
        ESP_LOGE(TAG, "Failed to init SRC_INAMP_HANDLE");
        return -1;
    }
    vTaskDelay(300);

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
            esp_err_t ret = ADG73_set_mux(src_pos, src_neg, sense_pos, sense_neg);
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
