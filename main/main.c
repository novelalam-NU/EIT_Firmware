#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "wireless.h"
#include "../Application_Layer/calibration.h"
#include "../Application_Layer/measurement.h"
#include "../Middle_Ware/hardware.h"

#define SIG_GEN_FREQ (50000.0f)
#define WIFI_CONNECT_TIMEOUT_MS 15000

static bool eit_hardware_init(void);

static const char *TAG = "MAIN";

/* Task details for measurement task */
TaskHandle_t meas_task;
static const char *meas_task_name = "MeasurementTask";
static const configSTACK_DEPTH_TYPE meas_task_stack_depth = 4000;
static const UBaseType_t meas_task_priority = 5;

void app_main(void)
{
    ESP_LOGI(TAG, "app_main start");

    esp_task_wdt_deinit();

    if (!eit_hardware_init()) {
        ESP_LOGE(TAG, "Hardware initialization failed");
        return;
    }

    if (!wireless_hardware_init()) {
        ESP_LOGE(TAG, "Wireless initialization failed");
        return;
    }

    if (!wireless_wait_for_ip(WIFI_CONNECT_TIMEOUT_MS)) {
        ESP_LOGW(TAG, "Wi-Fi did not get IP within %d ms", WIFI_CONNECT_TIMEOUT_MS);
    } else {
        ESP_LOGI(TAG, "Wi-Fi ready, starting measurement task");
    }

    /* Create a task for measurement */
    if (xTaskCreatePinnedToCore(&measurement_task, meas_task_name, meas_task_stack_depth, NULL, meas_task_priority, &meas_task, 0) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create measurement task");
    }
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
