#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"


#define NUM_ELECTRODE_PAIRS 8
#define NUM_SENSE_PAIRS (NUM_ELECTRODE_PAIRS - 3)
#define MAX_GAIN 512




void calibration_task(void* args);

int set_src_inamp_gain(uint16_t src_gain);
int set_sense_inamp_gain(uint16_t sense_gain);
int adcRead(uint8_t *buf, size_t len);
uint16_t dsp_freq_amp(uint16_t *buf, size_t len);
int init_mux(void);
int set_mux(uint8_t src_pos, uint8_t src_neg, uint8_t sense_pos, uint8_t sense_neg);


/* Structure that holds calibration values and electrode number mappings*/
typedef struct {
    const uint8_t src_pos; //positive source electrode number
    const uint8_t src_neg; //negative source electrode number
    
    const uint8_t sense_pos; //positive sense electrode number
    const uint8_t sense_neg; //negative sense electrode number
        
    uint16_t reference_amp; //The max amplitude sensed in the calibration 
    
} Calibration_t;

extern const uint16_t src_rdata; //Fixed source gain value
extern uint16_t max_calibrated_sense_rdata;
/* 2d array to hold calibration values and electrode mappings */
extern Calibration_t calibration_table [NUM_ELECTRODE_PAIRS][NUM_SENSE_PAIRS];

/* Tasks and Primative Handles*/
extern TaskHandle_t cal_task;
extern TaskHandle_t meas_task;
extern SemaphoreHandle_t sem_cal_to_meas;




