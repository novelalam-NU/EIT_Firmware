#include <stdio.h>
#include "calibration.h"

void calibration_task(void* args) {
    printf("Calibration done\n");

    vTaskDelete(NULL);

    return NULL;
}
