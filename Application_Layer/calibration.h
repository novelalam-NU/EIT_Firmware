#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"


#define NUM_ELECTRODE_PAIRS 8
#define NUM_SENSE_PAIRS (NUM_ELECTRODE_PAIRS - 3)
#define MAX_GAIN 512
#define BUFFER_LEN 64

#define CLIP_THRESHOLD 100
#define CLIP_MIN_SAMPLES 12
#define CLIP_MAX_INDEX 31
#define CALIBRATION_DSP_BUCKET 7



void calibration_task(void* args);

/* Structure that holds calibration values and electrode number mappings*/
typedef struct {
    const uint8_t src_pos; //positive source electrode number
    const uint8_t src_neg; //negative source electrode number
    
    const uint8_t sense_pos; //positive sense electrode number
    const uint8_t sense_neg; //negative sense electrode number
        
    uint16_t reference_amp; //The max amplitude sensed in the calibration 
    
} Calibration_t;

extern const uint16_t SCR_RDATA_CONST; //Fixed source gain value
extern uint16_t max_calibrated_sense_rdata;
/* 2d array to hold calibration values and electrode mappings */
extern Calibration_t calibration_table [NUM_ELECTRODE_PAIRS][NUM_SENSE_PAIRS];

/* Tasks and Primative Handles*/
extern TaskHandle_t cal_task;
extern TaskHandle_t meas_task;
extern SemaphoreHandle_t sem_cal_to_meas;




