#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MAX_ADC_PACKETS 40
#define ADC_READINGS_PER_PACKET 64

void measurement_task(void* args);

/* Array of buffers holding chunks of ADC readings */
extern uint16_t adc_packet_buffers[MAX_ADC_PACKETS][ADC_READINGS_PER_PACKET];




