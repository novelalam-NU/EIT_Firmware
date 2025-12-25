#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Test the ADC by reading dummy data.
 * 
 * @return 0 on success, or an error code.
 */
int test_adc(void);

/**
 * @brief Test the signal generator.
 * 
 * @return 0 on success, or an error code.
 */
int test_signal_gen(void);

/**
 * @brief Test the instrumentation amplifier potentiometers.
 * 
 * @return 0 on success, or an error code.
 */
int test_inamp_pots(void);

/**
 * @brief Test the multiplexer.
 * 
 * @return 0 on success, or an error code.
 */
int test_mux(void);

/**
 * @brief Tests the DSP (Digital Signal Processor) module.
 * 
 * @param clipped If true, applies clipping to the generated test signal.
 * @param clip_percent The percentage of the amplitude to clip at (0.0 to 1.0).
 * @return int Returns 0 on success, non-zero on failure.
 */
int test_dsp(bool clipped, float clip_percent);

