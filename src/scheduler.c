#include "scheduler.h"

#include <stdbool.h>
#include <string.h>

#include "debug.h"
#include "drv_time.h"
#include "flight/control.h"
#include "tasks.h"
#include "util/cbor_helper.h"

#define TASK_AVERAGE_SAMPLES 32
#define TASK_RUNTIME_REDUCTION 0.99
#define TASK_RUNTIME_BUFFER 15

#define US_TO_CYCLES(us) ((us)*TICKS_PER_US)
#define CYCLES_TO_US(cycles) ((cycles) / TICKS_PER_US)

uint8_t looptime_warning;
uint8_t blown_loop_counter;

static uint32_t lastlooptime;

static task_t *task_queue[TASK_MAX];
static uint32_t task_queue_size = 0;

extern int liberror;

static bool task_queue_contains(task_t *task) {
  for (uint32_t i = 0; i < task_queue_size; i++) {
    if (task_queue[i] == task) {
      return true;
    }
  }
  return false;
}

static bool task_queue_push(task_t *task) {
  if (task_queue_size >= TASK_MAX || task_queue_contains(task)) {
    return false;
  }
  for (uint32_t i = 0; i < TASK_MAX; i++) {
    if (task_queue[i] != NULL && task_queue[i]->priority <= task->priority) {
      continue;
    }

    memcpy(task_queue + i + 1, task_queue + i, (task_queue_size - i) * sizeof(task_t *));
    task_queue[i] = task;
    task_queue_size++;
    return true;
  }
  return false;
}

static bool should_run_task(const uint32_t start_cycles, uint8_t task_mask, task_t *task) {
  if ((task_mask & task->mask) == 0) {
    // task shall not run in this firmware state
    return false;
  }

  if ((int32_t)(task->last_run_time - start_cycles) > 0) {
    // task was already run this loop
    return false;
  }

  const bool poll_result = task->poll_func == NULL || task->poll_func();
  if (!poll_result && task->period == 0) {
    // task has polled false and does not need updating due too a period
    return false;
  }

  if (!poll_result && task->period != 0 && (time_cycles() - task->last_run_time) < US_TO_CYCLES(task->period)) {
    // task has a period, but its not up yet
    return false;
  }

  const int32_t time_left = US_TO_CYCLES(state.looptime_autodetect - TASK_RUNTIME_BUFFER) - (time_cycles() - start_cycles);
  if (task->priority != TASK_PRIORITY_REALTIME && task->runtime_avg > time_left) {
    // we dont have any time left this loop and task is not realtime
    task->runtime_avg *= TASK_RUNTIME_REDUCTION;
    return false;
  }

  return true;
}

static void do_run_task(task_t *task) {
  const uint32_t start = time_cycles();
  task->last_run_time = start;
  task->func();
  const int32_t time_taken = time_cycles() - start;

  if (time_taken < task->runtime_min) {
    task->runtime_min = time_taken;
  }

  task->runtime_avg_sum -= task->runtime_avg;
  task->runtime_avg_sum += time_taken;
  task->runtime_avg = task->runtime_avg_sum / TASK_AVERAGE_SAMPLES;

  if (time_taken > task->runtime_max) {
    task->runtime_max = time_taken;
  }
}

static void run_tasks(const uint32_t start_cycles) {
  uint8_t task_mask = 0;

  if (flags.on_ground && !flags.arm_state) {
    task_mask |= TASK_MASK_ON_GROUND;
  }
  if (flags.in_air || flags.arm_state) {
    task_mask |= TASK_MASK_IN_AIR;
  }

  while ((time_cycles() - start_cycles) < US_TO_CYCLES(state.looptime_autodetect - TASK_RUNTIME_BUFFER)) {
    for (uint32_t i = 0; i < task_queue_size; i++) {
      task_t *task = task_queue[i];

      if (!should_run_task(start_cycles, task_mask, task)) {
        continue;
      }

      do_run_task(task);
    }
  }
}

void looptime_update() {
  // looptime_autodetect sequence
  static uint8_t loop_delay = 0;
  if (loop_delay < 200) {
    loop_delay++;
  }

  static float loop_avg = 0;
  static uint8_t loop_counter = 0;

  if (loop_delay >= 200 && loop_counter < 200) {
    loop_avg += state.looptime_us;
    loop_counter++;
  }

  if (loop_counter == 200) {
    loop_avg /= 200;

    if (loop_avg < 130.f) {
      state.looptime_autodetect = LOOPTIME_8K;
    } else if (loop_avg < 255.f) {
      state.looptime_autodetect = LOOPTIME_4K;
    } else {
      state.looptime_autodetect = LOOPTIME_2K;
    }

    loop_counter++;
  }

  if (loop_counter == 201) {
    if (state.cpu_load > state.looptime_autodetect + 5) {
      blown_loop_counter++;
    }

    if (blown_loop_counter > 100) {
      blown_loop_counter = 0;
      loop_counter = 0;
      loop_avg = 0;
      looptime_warning++;
    }
  }
}

void scheduler_init() {
  // attempt 8k looptime for f405 or 4k looptime for f411
  state.looptime = LOOPTIME * 1e-6;
  state.looptime_autodetect = LOOPTIME;

  lastlooptime = time_micros();

  for (uint32_t i = 0; i < TASK_MAX; i++) {
    task_queue_push(&tasks[i]);
  }
}

void scheduler_update() {
  const uint32_t cycles = time_cycles();
  const uint32_t time = time_micros();
  state.looptime_us = ((uint32_t)(time - lastlooptime));
  lastlooptime = time;

  if (state.looptime_us <= 0) {
    state.looptime_us = 1;
  }

  // max loop 20ms
  if (state.looptime_us > 20000) {
    failloop(FAILLOOP_LOOPTIME);
  }

  looptime_update();

  state.looptime = state.looptime_us * 1e-6f;

  state.uptime += state.looptime;
  if (flags.arm_state) {
    state.armtime += state.looptime;
  }

  if (liberror > 20) {
    failloop(FAILLOOP_SPI_MAIN);
  }

  run_tasks(cycles);

  state.cpu_load = (time_micros() - lastlooptime);
  while ((time_micros() - time) < state.looptime_autodetect)
    __NOP();
}

void reset_looptime() {
  extern uint32_t lastlooptime;
  lastlooptime = time_micros();
}

#ifdef DEBUG

#define ENCODE_CYCLES(val)                                \
  {                                                       \
    const uint32_t us = CYCLES_TO_US(val);                \
    CBOR_CHECK_ERROR(res = cbor_encode_uint32(enc, &us)); \
  }

cbor_result_t cbor_encode_task_stats(cbor_value_t *enc) {
  CBOR_CHECK_ERROR(cbor_result_t res = cbor_encode_array_indefinite(enc));

  for (uint32_t i = 0; i < TASK_MAX; i++) {
    CBOR_CHECK_ERROR(res = cbor_encode_map_indefinite(enc));

    CBOR_CHECK_ERROR(res = cbor_encode_str(enc, "name"));
    CBOR_CHECK_ERROR(res = cbor_encode_str(enc, tasks[i].name));

    CBOR_CHECK_ERROR(res = cbor_encode_str(enc, "last_run_time"));
    ENCODE_CYCLES(tasks[i].last_run_time)

    CBOR_CHECK_ERROR(res = cbor_encode_str(enc, "min"));
    ENCODE_CYCLES(tasks[i].runtime_min)

    CBOR_CHECK_ERROR(res = cbor_encode_str(enc, "avg"));
    ENCODE_CYCLES(tasks[i].runtime_avg)

    CBOR_CHECK_ERROR(res = cbor_encode_str(enc, "max"));
    ENCODE_CYCLES(tasks[i].runtime_max)

    CBOR_CHECK_ERROR(res = cbor_encode_end_indefinite(enc));
  }

  CBOR_CHECK_ERROR(res = cbor_encode_end_indefinite(enc));

  return res;
}

#endif