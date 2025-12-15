#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"


#include "../Device_Drivers/AD5270.h"
#include "../Device_Drivers/AD5930.h"

#include "../Application_Layer/calibration.h"

#define SIG_GEN_FREQ (50000.0f) 

/* Task Details for Calibration Task*/
static TaskHandle_t cal_task;
static const char* cal_task_name = "CalibrationTask";
static const configSTACK_DEPTH_TYPE cal_task_stack_depth = 2048;
static const UBaseType_t cal_task_priority = 5;




void app_main(void)
{
    /* Start the Signal Generator at the correct frequency */
    if ( AD5930_init( SIG_GEN_FREQ ) != ESP_OK) {
        printf("Failed to intialize Signal Generator\n");
    }

    /* Create a Task to find all the calibration values */
    void* cal_arg = NULL;
    if ( xTaskCreate( &calibration_task, cal_task_name, cal_task_stack_depth, cal_arg, cal_task_priority, &cal_task ) != pdPASS ) {
        printf("Failed to create thread\n");
    } 
 

}
