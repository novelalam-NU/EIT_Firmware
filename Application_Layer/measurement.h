#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void measurement_task(void* args);


/* Tasks and Primative Handles*/
extern TaskHandle_t cal_task;
extern TaskHandle_t meas_task;
extern SemaphoreHandle_t sem_cal_to_meas;


