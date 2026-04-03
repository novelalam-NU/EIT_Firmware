#include "tasks.h"
#include "measurement.h"
#include "wireless.h"
#include "esp_log.h"

static const char *TAG = "TASKS";

static const char *meas_task_name = "MeasurementTask";
static const configSTACK_DEPTH_TYPE meas_task_stack_depth = 4000;
static const UBaseType_t meas_task_priority = 0;

static const char *udp_task_name = "UDPTask";
static const configSTACK_DEPTH_TYPE udp_task_stack_depth = 4000;
static const UBaseType_t udp_task_priority = 0;

TaskHandle_t meas_task;
TaskHandle_t udp_task;

void start_measurement_task(void)
{
    if (xTaskCreatePinnedToCore(measurement_task, meas_task_name, meas_task_stack_depth, NULL,
                                meas_task_priority, &meas_task, 1) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create measurement task");
    }
}

void start_udp_task(void)
{
    if (xTaskCreatePinnedToCore(UDP_task, udp_task_name, udp_task_stack_depth, NULL,
                                udp_task_priority, &udp_task, 0) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create UDP task");
    }
}
