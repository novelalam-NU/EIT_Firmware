#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "hardware.h"

static const char *TAG = "HARDWARE_TEST";

static void test_mux(void)
{
    ESP_LOGI(TAG, "Starting MUX test... Switching channels indefinitely.");
    uint8_t ch = 1;
    while (1) {
        // Toggle through electrode positions (1 to 8)
        uint8_t src_pos = ch;
        uint8_t src_neg = (ch % 8) + 1;
        uint8_t sense_pos = ((ch + 1) % 8) + 1;
        uint8_t sense_neg = ((ch + 2) % 8) + 1;
        
        ESP_LOGI(TAG, "Switching MUX: src_pos=%d, src_neg=%d, sense_pos=%d, sense_neg=%d", 
                 src_pos, src_neg, sense_pos, sense_neg);
                 
        if (set_mux(src_pos, src_neg, sense_pos, sense_neg) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to set MUX");
        }
        
        ch = (ch % 8) + 1;
        vTaskDelay(pdMS_TO_TICKS(1000)); // Switch every 1 second
    }
}

void test_hardware(void)
{
    ESP_LOGI(TAG, "Entering hardware test mode...");
    ESP_LOGI(TAG, "Active Pin Configuration:");
    ESP_LOGI(TAG, "  SPI MOSI:       GPIO %d", PIN_SPI_MOSI);
    ESP_LOGI(TAG, "  SPI MISO:       GPIO %d", PIN_SPI_MISO);
    ESP_LOGI(TAG, "  SPI SCLK:       GPIO %d", PIN_SPI_SCLK);
    ESP_LOGI(TAG, "  CS ADC:         GPIO %d", PIN_CS_ADC);
    ESP_LOGI(TAG, "  CS DRIVE (POT): GPIO %d", PIN_CS_DRIVE);
    ESP_LOGI(TAG, "  CS MEAS (POT):  GPIO %d", PIN_CS_MEAS);
    ESP_LOGI(TAG, "  CS MUX:         GPIO %d", PIN_CS_MUX);
    ESP_LOGI(TAG, "  CS AD5930:      GPIO %d", PIN_CS_AD5930);
    ESP_LOGI(TAG, "  CTRL Pin:       GPIO %d", PIN_CTRL);
    ESP_LOGI(TAG, "  MSB/Mode Pin:   GPIO %d", PIN_MSB);

    test_mux();
}
