#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern TaskHandle_t inference_task_handle;

void start_inference_task(void);
