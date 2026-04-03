#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void start_measurement_task(void);
void start_udp_task(void);

extern TaskHandle_t meas_task;
extern TaskHandle_t udp_task;
