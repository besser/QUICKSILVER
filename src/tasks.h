#pragma once

#include "project.h"
#include "scheduler.h"

typedef enum {
  TASK_MAIN,
  TASK_RX,
  TASK_USB,
#ifdef ENABLE_BLACKBOX
  TASK_BLACKBOX,
#endif
#ifdef ENABLE_OSD
  TASK_OSD,
#endif
  TASK_VTX,

  TASK_MAX

} task_id_t;

extern task_t tasks[TASK_MAX];