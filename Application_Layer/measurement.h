#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MAX_ADC_PACKETS 40
#define ADC_READINGS_PER_PACKET 64

void measurement_task(void* args);


/* Tasks and Primative Handles*/
extern TaskHandle_t cal_task;
extern TaskHandle_t meas_task;
extern SemaphoreHandle_t sem_cal_to_meas;

/* Array of buffers holding chunks of ADC readings */
extern int16_t adc_packet_buffers[MAX_ADC_PACKETS][ADC_READINGS_PER_PACKET];


