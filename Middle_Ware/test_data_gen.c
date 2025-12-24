#include "test_data_gen.h"
#include <math.h>
#include <stdlib.h>
#include "esp_random.h"
#include <stdio.h>

void generate_sine_int16(int16_t *out, float freq_hz)
{
    const float amplitude = 10000.0f;
    
    // Random start phase (0 to 2*PI)
    float start_phase = ((float)esp_random() / UINT32_MAX) * 2.0f * (float)M_PI;

    // Generate ~7 cycles worth of samples
    int num_samples = (int)(7 * TEST_GEN_FS / freq_hz);
    if (num_samples > TEST_GEN_N) num_samples = TEST_GEN_N;

    for (int n = 0; n < num_samples; n++) {
        out[n] = (int16_t)(
            amplitude * sinf((2.0f * (float)M_PI * freq_hz * n / TEST_GEN_FS) + start_phase)
        );
    }

    // Print the starting amplitude phase in degreee
    printf("Start phase: %f degrees\n", start_phase * 180.0f / (float)M_PI);

    
    // Pad remaining samples with zero
    for (int n = num_samples; n < TEST_GEN_N; n++) {
        out[n] = 0;
    }
}

void generate_sine_int16_multi(int16_t *out, float freq_hz1, float freq_hz2, float freq_hz3)
{
    const float amplitude = 10000.0f / 3.0f;
    
    float start_phase = ((float)esp_random() / UINT32_MAX) * 2.0f * (float)M_PI;
    int num_samples = TEST_GEN_N;

    for (int n = 0; n < num_samples; n++) {
        float sample = amplitude * sinf((2.0f * (float)M_PI * freq_hz1 * n / TEST_GEN_FS) + start_phase);
        sample += amplitude * sinf((2.0f * (float)M_PI * freq_hz2 * n / TEST_GEN_FS) + start_phase);
        sample += amplitude * sinf((2.0f * (float)M_PI * freq_hz3 * n / TEST_GEN_FS) + start_phase);
        out[n] = (int16_t)sample;
    }

    printf("Start phase: %f degrees\n", start_phase * 180.0f / (float)M_PI);
}
