/**
 * @file ad5930.c - ad5930 signal generator (HAL-agnostic).
 * Author: Novel Alam, Date: Nov 12 2025.
 */

#include "AD5930_SigGen.h"
#include "../Middle_Ware/hardware.h"
#include "driver/spi_master.h"
#include "driver/spi_common.h"
#include "esp_log.h"

static const char *TAG = "AD5930";

/* Spi Config stuff*/
static const spi_device_interface_config_t ad5930_spi_config = {
    .command_bits = 0,
    .address_bits = 0,
    .dummy_bits = 0,
    .mode = 2,
    .duty_cycle_pos = 128,
    .spics_io_num = PIN_CS_AD5930,
    .queue_size = 1,
    .clock_speed_hz = SPI_MASTER_FREQ_8M

};

/* Spi handle*/
static spi_device_handle_t ad5930_handle = NULL;

/* Used for setting frequency*/
static spi_transaction_t msg_set_freq = {
    .flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA,
    .length = 16,
    .rx_data = {0}
};





/*Add Signal generator to spi bus (assuming intialized)*/

int AD5930_init(float init_freq) {
    esp_err_t ret = spi_bus_add_device(SPI2_HOST, &ad5930_spi_config, &ad5930_handle);


    if ( ret != ESP_OK ) {
        ESP_LOGE(TAG, "Failed to add AD5930 to spi bus: %s", esp_err_to_name(ret));
        return ret;
    } else {
        ESP_LOGI(TAG, "Added AD5930 to bus");
    }

    
    /* Scale according to MCLK_FREQ */
    uint32_t converted_freq = ( ( (float) init_freq ) / ( (float) MCLK_FREQ ) ) * UINT24_MAX;
    
    
    uint16_t freq_low = converted_freq & 0xFFF;
    uint16_t freq_high = (converted_freq >> 12U) & 0xFFF;
    
    
    uint16_t packet1 = AD5930_PACKET(START_FREQ_LOW, freq_low);
    uint16_t packet2 = AD5930_PACKET(START_FREQ_HIGH, freq_high);
    
    /* Send first packet */
    msg_set_freq.tx_data[0] = (packet1 >> 8) & 0xff;
    msg_set_freq.tx_data[1] = (packet1) & 0xff;


    ret = spi_device_transmit(ad5930_handle, &msg_set_freq);
    
    if ( ret != ESP_OK ) {
        ESP_LOGE(TAG, "Failed to set freq for ad5930: %s", esp_err_to_name(ret));
        return ret;
    } 

       


    /* Send second packet */
    msg_set_freq.tx_data[0] = (packet2 >> 8) & 0xff;
    msg_set_freq.tx_data[1] = (packet2) & 0xff;

    ret = spi_device_transmit(ad5930_handle, &msg_set_freq);

    if ( ret != ESP_OK ) {
        ESP_LOGE(TAG, "Failed to set freq for ad5930: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "AD5930 Set to %f", init_freq);
    return 0;



}
