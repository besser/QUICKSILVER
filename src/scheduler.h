#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <cbor.h>

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
  uint32_t period;

  uint32_t runtime_current;
  uint32_t runtime_min;
  uint32_t runtime_avg;
  uint32_t runtime_worst;
  uint32_t runtime_max;

  uint32_t runtime_avg_sum;
} task_t;

#define CREATE_TASK(p_name, p_subname, p_mask, p_priority, p_period, p_poll_func, p_func) \
  {                                                                                       \
    .name = p_name,                                                                       \
    .subname = p_subname,                                                                 \
    .mask = p_mask,                                                                       \
    .priority = p_priority,                                                               \
    .poll_func = p_poll_func,                                                             \
    .func = p_func,                                                                       \
    .period = p_period,                                                                   \
    .last_run_time = 0,                                                                   \
    .runtime_current = 0,                                                                 \
    .runtime_min = UINT32_MAX,                                                            \
    .runtime_avg = 0,                                                                     \
    .runtime_worst = 0,                                                                   \
    .runtime_max = 0,                                                                     \
    .runtime_avg_sum = 0,                                                                 \
  }

void scheduler_init();
void scheduler_update();

cbor_result_t cbor_encode_task_stats(cbor_value_t *enc);