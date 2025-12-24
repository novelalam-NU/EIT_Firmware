#pragma once
#include <stdint.h>

#define TEST_GEN_N   64
#define TEST_GEN_FS  500000.0f   // 500 kHz sampling rate

/**
 * @brief Generate a sine wave in an int16_t buffer.
 * 
 * @param out Pointer to the output buffer (must be at least TEST_GEN_N in size).
 * @param freq_hz Frequency of the sine wave in Hz.
 */
void generate_sine_int16(int16_t *out, float freq_hz);
