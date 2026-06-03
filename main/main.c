#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "../Application_Layer/wireless.h"
#include "../Application_Layer/calibration.h"
#include "../Application_Layer/measurement.h"
#include "../Application_Layer/tasks.h"
#include "../Middle_Ware/hardware.h"

#include "../Application_Layer/inference.h"

#define SIG_GEN_FREQ (50000.0f)
#define WIFI_CONNECT_TIMEOUT_MS 15000

static bool eit_hardware_init(void);
static const char *TAG = "MAIN";

// #define TEST

extern void test_hardware(void);

void app_main(void)
{
    ESP_LOGI(TAG, "app_main start");

    esp_task_wdt_deinit();

    if (!eit_hardware_init()) {
        ESP_LOGE(TAG, "Hardware initialization failed");
        return;
    }

#ifdef TEST
    test_hardware();
#else
    start_measurement_task();
    // start_udp_task();
    start_inference_task();
#endif
}

/* Initializes all EIT hardware components. Returns true on success, false on failure. */
static bool eit_hardware_init(void)
{
    if (init_spi() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init SPI");
        return false;
    }

    if (adc_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init ADC");
        return false;
    }

    if (init_mux() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init Mux");
        return false;
    }

    if (init_inamp_pots() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init inamp pots");
        return false;
    }

    if (signal_gen_start(SIG_GEN_FREQ) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Signal Generator");
        return false;
    }

    return true;
}
