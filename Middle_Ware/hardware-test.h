#pragma once

#include <stdint.h>

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
 * @return int Returns 0 on success, non-zero on failure.
 */
int test_dsp(void);


