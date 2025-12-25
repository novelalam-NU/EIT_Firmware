#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"

// #include "../Device_Drivers/AD5270_DigiPot.h"
// #include "../Device_Drivers/AD5930_SigGen.h"

#include "../Application_Layer/calibration.h"
#include "../Application_Layer/measurement.h"
#include "../Middle_Ware/hardware.h"
#include "../Middle_Ware/hardware-test.h"

#define SIG_GEN_FREQ (50000.0f) 

static const char *TAG = "MAIN";

/* Task Details for Calibration Task*/
TaskHandle_t cal_task;
static const char* cal_task_name = "CalibrationTask";
static const configSTACK_DEPTH_TYPE cal_task_stack_depth = 4000;
static const UBaseType_t cal_task_priority = 5;

/* Task Details for Measurement Task */
TaskHandle_t meas_task;
static const char* meas_task_name = "MeasurementTask";
static const configSTACK_DEPTH_TYPE meas_task_stack_depth = 4000;
static const UBaseType_t meas_task_priority = 5;

/* Semaphore for syncing of calibration and measurement task*/
SemaphoreHandle_t sem_cal_to_meas = NULL;

/* Used to init digital pots*/
extern int init_inamp_pots();

void app_main(void)
{
    /* Create Semaphore */
    sem_cal_to_meas = xSemaphoreCreateBinary();

    if (sem_cal_to_meas == NULL ) {
        ESP_LOGE(TAG, "Failed to create semaphore");
    }

    /* Initialize SPI Bus */
    if (init_spi() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init SPI");
        return;
    }

    /* Initialize ADC */
    if (adc_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init ADC");
        return;
    }


    if (init_mux() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init Mux");
    }


    if (init_inamp_pots() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init inamp pots");
        return;
    }



    if (signal_gen_start(SIG_GEN_FREQ) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Signal Generator");
        return;
    }


    /* Start the Signal Generator at the correct frequency */
    if ( signal_gen_start( SIG_GEN_FREQ ) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to intialize Signal Generator");
    }

    /* Create a Task to find all the calibration values */
    void* cal_arg = NULL;
    if ( xTaskCreate( &calibration_task, cal_task_name, cal_task_stack_depth, cal_arg, cal_task_priority, &cal_task ) != pdPASS ) {
        ESP_LOGE(TAG, "Failed to create thread");
    } 
 
 
    vTaskDelay(pdTICKS_TO_MS(100000000));

    /* Create a Task for Measurement */
    if ( xTaskCreate( &measurement_task, meas_task_name, meas_task_stack_depth, NULL, meas_task_priority, &meas_task ) != pdPASS ) {
        ESP_LOGE(TAG, "Failed to create measurement task");
    }

}
