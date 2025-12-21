/*
 * ADG73.c
 *
 * Short description:
 *   Minimal source file for an ADG73 device driver.
 *   Provides control interfaces for 4 daisy chained muxes
 *   Assuming Source + is first, next Source -, Sense +, Sense -
 *
 * Author: Novel Alam
 * Date: 2025-12-19
 */

#include "ADG73.h"
#include "esp_log.h"

static const char *TAG = "ADG73";

/* Device Handle*/
static spi_device_handle_t ADG73_handle = NULL;

/* Device config*/
static const spi_device_interface_config_t ADG73_config = {
    .mode = 3, //(CPOL=1, CPHA=1)
    .clock_source = SPI_CLK_SRC_DEFAULT,
    .duty_cycle_pos = 128,
    .clock_speed_hz = SPI_MASTER_FREQ_8M,
    .spics_io_num = PIN_CS_MUX,
    .queue_size = 1
};

uint32_t rx_buf;

/* Message config */ 
static uint32_t packet;
spi_transaction_t message = {
    .length = 32,
    .tx_buffer = &packet,
    .rx_buffer = &rx_buf

};


int init_src_sense_ADG73() {
    if ( spi_bus_add_device( SPI2_HOST, &ADG73_config, &ADG73_handle ) != ESP_OK ) {
        return -1;
    }
    return 0;
}



int set_src_sense_ADG73(uint8_t src_pos, uint8_t src_neg, uint8_t sense_pos, uint8_t sense_neg) {

    ESP_LOGI(TAG, "Sending packet");
    /* Data packet to send */
    packet =  (0xff & 1<<src_pos)<<24 | (0xff & 1<<src_neg)<<16 |  (0xff & 1<<sense_pos)<<8 | (0xff & 1<<sense_neg);
    if ( spi_device_polling_transmit(&ADG73_handle, &message) != ESP_OK ) {
        return -1;
    }

    return 0;

}