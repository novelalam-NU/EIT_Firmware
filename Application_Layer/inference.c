#include "inference.h"
#include "calibration.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "gesture_model.h"
#ifndef WINDOW_SIZE
#define WINDOW_SIZE 40
#endif
#define NUM_FEATURES 5
#define NUM_CHANNELS EWMA_AMP_COUNT // 35 channels

#define MEASURE_INFERENCE
#define OPTIMIZATIONS

#if defined(MEASURE_INFERENCE) || defined(MEASURE_INFERECE)
#define MEASURE_INFERENCE_ENABLED
#endif

#if defined(OPTIMIZATIONS) || defined(OPTIMISATIONS)
#define OPTIMIZATIONS_ENABLED
#endif

#ifdef OPTIMIZATIONS_ENABLED
#include "esp_dsp.h"

// Precomputed values for Linear Regression Slope derived from WINDOW_SIZE:
#define TIME_MEAN (((float)WINDOW_SIZE - 1.0f) / 2.0f)
#define SLOPE_DENOMINATOR (((float)WINDOW_SIZE * ((float)WINDOW_SIZE * (float)WINDOW_SIZE - 1.0f)) / 12.0f)

// Array holding [t - TIME_MEAN] for t in 0..(WINDOW_SIZE - 1). Used for dsps_dotprod_f32.
// Aligned to 16-byte boundary for vector instructions.
__attribute__((aligned(16))) static float time_offsets[WINDOW_SIZE];

static void init_dsp_features(void) {
  for (int w = 0; w < WINDOW_SIZE; w++) {
    time_offsets[w] = (float)w - TIME_MEAN;
  }
}

static inline void dsps_stat_f32(const float *input, int len, float *mean,
                                 float *var) {
  float sum = 0;
  for (int i = 0; i < len; i++) {
    sum += input[i];
  }
  float m = sum / (float)len;
  *mean = m;

  float sum_sq = 0;
  dsps_dotprod_f32(input, input, &sum_sq, len);

  float v = (sum_sq / (float)len) - (m * m);
  if (v < 0.0f)
    v = 0.0f;
  *var = v;
}
#endif

static const char *TAG = "INFERENCE";

TaskHandle_t inference_task_handle = NULL;

static float window_buffer[WINDOW_SIZE][NUM_CHANNELS];
static int window_head = 0;
static bool buffer_full = false;

static float feature_vector[NUM_CHANNELS * NUM_FEATURES];

/**
 * Extracts 5 features per channel over the sliding window:
 * 1. Mean
 * 2. Standard Deviation (SD)
 * 3. Slope (Linear Regression)
 * 4. Range (Max - Min)
 * 5. Net Displacement (End - Start)
 */
static void extract_features(void) {
#ifdef OPTIMIZATIONS_ENABLED
  ESP_LOGD(TAG, "Extracting features using ESP-DSP...");
  int feat_idx = 0;

  // Local buffer to hold contiguous chronological data for ESP-DSP
  __attribute__((aligned(16))) float temp_array[WINDOW_SIZE];

  for (int ch = 0; ch < NUM_CHANNELS; ch++) {
    // 1. Copy data to a flat, chronological array & find Min/Max at the same
    // time
    int circular_idx = window_head;
    float min_val = window_buffer[circular_idx][ch];
    float max_val = window_buffer[circular_idx][ch];

    for (int w = 0; w < WINDOW_SIZE; w++) {
      float val = window_buffer[circular_idx][ch];
      temp_array[w] =
          val; // Store chronologically (oldest at index 0, newest at WINDOW_SIZE - 1)

      if (val < min_val)
        min_val = val;
      if (val > max_val)
        max_val = val;

      circular_idx++;
      if (circular_idx >= WINDOW_SIZE)
        circular_idx = 0;
    }

    // 2. Use ESP-DSP to calculate Mean and Variance/SD
    float mean = 0.0f;
    float variance = 0.0f;

    // This calculates mean and variance using ESP32-S3 SIMD hardware
    dsps_stat_f32(temp_array, WINDOW_SIZE, &mean, &variance);

    if (variance < 0.0f)
      variance = 0.0f; // Guard against precision errors
    float sd = sqrtf(variance);

    // 3. Use ESP-DSP to calculate Slope Numerator
    float slope_num = 0.0f;

    // dsps_dotprod_f32 calculates sum(time_offsets[i] * temp_array[i]) using
    // SIMD
    dsps_dotprod_f32(time_offsets, temp_array, &slope_num, WINDOW_SIZE);

    float slope = slope_num / SLOPE_DENOMINATOR;
    float range = max_val - min_val;

    // 4. Net displacement (newest value minus oldest value)
    float net_displacement = temp_array[WINDOW_SIZE - 1] - temp_array[0];

    // Store sequentially in feature vector
    feature_vector[feat_idx++] = mean;
    feature_vector[feat_idx++] = sd;
    feature_vector[feat_idx++] = slope;
    feature_vector[feat_idx++] = range;
    feature_vector[feat_idx++] = net_displacement;
  }

  // Log the features of channel 0 as a debug reference
  ESP_LOGI(TAG,
           "Ch 0 Feats -> Mean:%.1f SD:%.1f Slope:%.2f Range:%.1f NetDisp:%.1f",
           feature_vector[0], feature_vector[1], feature_vector[2],
           feature_vector[3], feature_vector[4]);
#else
  ESP_LOGD(TAG, "Extracting features from sliding window...");
  int feat_idx = 0;
  for (int ch = 0; ch < NUM_CHANNELS; ch++) {
    float sum = 0.0f;
    float min_val = window_buffer[0][ch];
    float max_val = window_buffer[0][ch];

    // 1. Calculate Mean, Min, Max
    for (int w = 0; w < WINDOW_SIZE; w++) {
      float val = window_buffer[w][ch];
      sum += val;
      if (val < min_val)
        min_val = val;
      if (val > max_val)
        max_val = val;
    }
    float mean = sum / (float)WINDOW_SIZE;

    // 2. Calculate Standard Deviation & Slope
    float variance_sum = 0.0f;
    float num_sum = 0.0f;
    float den_sum = 0.0f;
    float mean_t = (float)(WINDOW_SIZE - 1) / 2.0f; // Mean of indices 0..WINDOW_SIZE-1

    for (int w = 0; w < WINDOW_SIZE; w++) {
      // Reconstruct chronological sequence from the circular buffer
      int circular_idx = (window_head + w) % WINDOW_SIZE;
      float val = window_buffer[circular_idx][ch];

      variance_sum += (val - mean) * (val - mean);

      float t = (float)w;
      num_sum += (t - mean_t) * (val - mean);
      den_sum += (t - mean_t) * (t - mean_t);
    }

    float sd = sqrtf(variance_sum / (float)WINDOW_SIZE);
    float slope = (den_sum > 0.0f) ? (num_sum / den_sum) : 0.0f;
    float range = max_val - min_val;

    // Net displacement = chronological last value - chronological first value
    int first_idx = window_head;
    int last_idx = (window_head + WINDOW_SIZE - 1) % WINDOW_SIZE;
    float net_displacement =
        window_buffer[last_idx][ch] - window_buffer[first_idx][ch];

    // Store sequentially in feature vector
    feature_vector[feat_idx++] = mean;
    feature_vector[feat_idx++] = sd;
    feature_vector[feat_idx++] = slope;
    feature_vector[feat_idx++] = range;
    feature_vector[feat_idx++] = net_displacement;
  }

  // Log the features of channel 0 as a debug reference
  ESP_LOGI(TAG,
           "Ch 0 Feats -> Mean:%.1f SD:%.1f Slope:%.2f Range:%.1f NetDisp:%.1f",
           feature_vector[0], feature_vector[1], feature_vector[2],
           feature_vector[3], feature_vector[4]);
#endif
}

/**
 * Placeholder for the classifier prediction (e.g. ExtraTrees).
 * Replace this prediction logic with your compiled model.
 */
static int run_classifier(const float *features) {
  return gesture_model_predict(features, 175);
}

void inference_task(void *arg) {
  uint16_t local_ewma[NUM_CHANNELS];
  uint32_t frame_count = 0;
  ESP_LOGI(TAG, "Inference task starting");

  for (;;) {
    // Wait blockingly for a notification from the measurement task (frame
    // ready)
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    frame_count++;
    if (frame_count % 48 == 0) {
      ESP_LOGI(TAG,
               "Received 48 EIT frames (current rate is ~1Hz window shifts)");
    }

    // 1. Thread-safe copy of current EWMA values from calibration.c
    memcpy(local_ewma, ewma_amp, sizeof(local_ewma));

    // 2. Add current frame to sliding circular window buffer
    for (int ch = 0; ch < NUM_CHANNELS; ch++) {
      window_buffer[window_head][ch] = (float)local_ewma[ch];
    }

    window_head = (window_head + 1) % WINDOW_SIZE;
    if (window_head == 0) {
      if (!buffer_full) {
        ESP_LOGI(TAG, "Sliding window buffer is now FULL (%d frames). Starting "
                      "inference!", WINDOW_SIZE);
      }
      buffer_full = true;
    }

    if (!buffer_full) {
      ESP_LOGI(TAG, "Buffering EIT window: %d/%d frames completed", window_head,
               WINDOW_SIZE);
    }

    // 3. Compute features & run classifier once the buffer is full
    if (buffer_full) {
#ifdef MEASURE_INFERENCE_ENABLED
      int64_t start_extract = esp_timer_get_time();
#endif
      extract_features();
#ifdef MEASURE_INFERENCE_ENABLED
      int64_t end_extract = esp_timer_get_time();
#endif

      ESP_LOGD(TAG, "Running ExtraTrees classifier model...");
#ifdef MEASURE_INFERENCE_ENABLED
      int64_t start_infer = esp_timer_get_time();
#endif
      int gesture = run_classifier(feature_vector);
#ifdef MEASURE_INFERENCE_ENABLED
      int64_t end_infer = esp_timer_get_time();
#endif

      ESP_LOGI(TAG, "Inference prediction: Gesture Class %d", gesture);
#ifdef MEASURE_INFERENCE_ENABLED
      int64_t extract_time = end_extract - start_extract;
      int64_t infer_time = end_infer - start_infer;
      ESP_LOGI(TAG,
               "Feature extraction took: %lld us, Inference took: %lld us, "
               "Total: %lld us",
               extract_time, infer_time, extract_time + infer_time);
#endif
    }
  }
}

void start_inference_task(void) {
#ifdef OPTIMIZATIONS_ENABLED
  init_dsp_features();
#endif
  // Created on core 0 to keep measurement task running with low jitter on core
  // 1
  if (xTaskCreatePinnedToCore(inference_task, "InferenceTask", 4000, NULL, 1,
                              &inference_task_handle, 0) != pdPASS) {
    ESP_LOGE(TAG, "Failed to create Inference task");
  }
}
