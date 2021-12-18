#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <cbor.h>

#define TASK_AVERAGE_SAMPLES 32
#define TASK_RUNTIME_REDUCTION 0.9

typedef enum {
  TASK_PRIORITY_REALTIME,
  TASK_PRIORITY_HIGH,
  TASK_PRIORITY_MEDIUM,
  TASK_PRIORITY_LOW,
} task_priority_t;

typedef enum {
  TASK_MASK_ON_GROUND = (0x1 << 0),
  TASK_MASK_IN_AIR = (0x1 << 1),

  TASK_MASK_ALWAYS = 0xFF,
} task_mask_t;

typedef void (*task_function_t)();
typedef bool (*task_poll_function_t)();

typedef struct {
  const char *name;
  const char *subname;

  uint8_t mask;

  task_priority_t priority;

  task_poll_function_t poll_func;
  task_function_t func;

  uint32_t last_run_time;

  int32_t runtime_min;
  int32_t runtime_avg;
  int32_t runtime_max;

  int32_t runtime_avg_sum;
} task_t;

#define CREATE_TASK(p_name, p_subname, p_mask, p_priority, p_poll_func, p_func) \
  {                                                                             \
    .name = p_name,                                                             \
    .subname = p_subname,                                                       \
    .mask = p_mask,                                                             \
    .priority = p_priority,                                                     \
    .poll_func = p_poll_func,                                                   \
    .func = p_func,                                                             \
    .last_run_time = 0,                                                         \
    .runtime_min = UINT32_MAX,                                                  \
    .runtime_avg = 0,                                                           \
    .runtime_max = 0,                                                           \
    .runtime_avg_sum = 0,                                                       \
  }

void scheduler_init();
void scheduler_update();

cbor_result_t cbor_encode_task_stats(cbor_value_t *enc);