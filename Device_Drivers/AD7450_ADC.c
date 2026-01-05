#include "AD7450_ADC.h"
#include "../Middle_Ware/hardware.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <string.h>

static const char *TAG = "AD7450";
static spi_device_handle_t ad7450_handle = NULL;

int AD7450_init() {
    if (ad7450_handle != NULL) {
        ESP_LOGW(TAG, "AD7450 already initialized");
        return ESP_OK;
    }

    spi_device_interface_config_t devcfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 3, // CPOL=1, CPHA=1 (SCLK idle high, sample on rising edge)
        .clock_speed_hz = 100, // 8 MHz
        .spics_io_num = PIN_CS_ADC,
        .queue_size = AD7450_QUEUE_SIZE,
    };

    esp_err_t ret = spi_bus_add_device(SPI2_HOST, &devcfg, &ad7450_handle);
    if (ret != ESP_OK) {
        #if DEBUG
        ESP_LOGE(TAG, "Failed to add device to SPI bus: %s", esp_err_to_name(ret));
        #endif
        return ret;
    }

    // Perform a dummy read to ensure the device is powered up and in normal mode
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 16;
    t.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA; // Use TX data to ensure MOSI is driven (though ignored by ADC)
    
    ret = spi_device_transmit(ad7450_handle, &t);
    if (ret != ESP_OK) {
        #if DEBUG
        ESP_LOGE(TAG, "Dummy read failed: %s", esp_err_to_name(ret));
        #endif
        return ret;
    }

    ESP_LOGI(TAG, "AD7450 initialized successfully");
    return ESP_OK;
}

int AD7450_Read(int16_t *buf, uint32_t len) {
    if (ad7450_handle == NULL) {
        #if DEBUG
        ESP_LOGE(TAG, "AD7450 not initialized");
        #endif
        return -1;
    }

    if (len > AD7450_QUEUE_SIZE) {
        #if DEBUG
        ESP_LOGE(TAG, "Read length %lu exceeds queue size %d", len, AD7450_QUEUE_SIZE);
        #endif
        return -1;
    }

    /* Clear the buffer */
    //memset(buf, 0, (size_t)len * sizeof(uint16_t));

    /* Acquire the bus*/
    if (spi_device_acquire_bus(ad7450_handle, portMAX_DELAY) != ESP_OK) {
        #if DEBUG
        ESP_LOGE(TAG, "Failed to acquire SPI bus");
        #endif
        return -1;
    }

    /* Hold all the transation reference in flight */
    static spi_transaction_t trans_in_flight[AD7450_QUEUE_SIZE];

    //#define QUEUE
    #ifdef QUEUE
    /* Queue "len" reads */
    for (uint32_t curr_queue_trans = 0; curr_queue_trans < len; curr_queue_trans++) {
        trans_in_flight[curr_queue_trans].length = 16;
        trans_in_flight[curr_queue_trans].rx_buffer = &buf[curr_queue_trans];
        trans_in_flight[curr_queue_trans].tx_buffer = NULL;
        
        if (spi_device_queue_trans(ad7450_handle, &trans_in_flight[curr_queue_trans], portMAX_DELAY) != ESP_OK) {
            spi_device_release_bus(ad7450_handle);
            return -1;
        }
        
        
        #if DEBUG
        ESP_LOGE(TAG, "send packet %lu", curr_queue_trans);
        #endif

    }

    /* Collect results */
    for (uint32_t curr_queue_trans = 0; curr_queue_trans < len; curr_queue_trans++) {
        spi_transaction_t* trans_result;
        
        if (spi_device_get_trans_result(ad7450_handle, &trans_result, portMAX_DELAY) != ESP_OK) {
            #if DEBUG
            ESP_LOGE(TAG, "Failed to get SPI transaction result");
            #endif
            spi_device_release_bus(ad7450_handle);
            return -1;
        }
    }
    #else

    spi_transaction_t t = {
        .length = 16,
        .flags = 0,
        .tx_data = {0},
        .rx_buffer = buf
    };


    


    if (spi_device_polling_transmit(ad7450_handle, &t) != ESP_OK) {
        spi_device_release_bus(ad7450_handle);
        return -1;
    }

    
    // for (uint32_t i = 0; i < len; i++) {
        
    //     int64_t start_time = esp_timer_get_time();
    //     if (spi_device_polling_transmit(ad7450_handle, &t) != ESP_OK) {
    //         spi_device_release_bus(ad7450_handle);
    //         return -1;
    //     }
    //     buf[i] = *(int16_t*)t.rx_data;
    //     int64_t end_time = esp_timer_get_time();
    //     printf("ADC Read Time: %lld us\n", (end_time - start_time));
    //     vTaskDelay(1);
    // }

    #endif

    /* See if transaction finsihed and went well*/

    // for (uint32_t curr_queue_trans = 0; curr_queue_trans < len; curr_queue_trans++) {

    //     /* throw away var to hold pointer to pointer to transmit*/
    //     spi_transaction_t* trans_result;

    //     if (spi_device_get_trans_result(ad7450_handle, &trans_result, portMAX_DELAY) != ESP_OK) {
    #if DEBUG
    ESP_LOGE(TAG, "Failed to get SPI transaction result");
    #endif
    //         spi_device_release_bus(ad7450_handle);
    //         return -1;
    //     }

    //     vTaskDelay(300);

    // }


    spi_device_release_bus(ad7450_handle);

    return 0;
}
