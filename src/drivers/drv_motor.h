#pragma once

#include <stdint.h>

#include "hardware.h"

#define FORWARD 0
#define REVERSE 1

#define MOTOR_BL 0
#define MOTOR_FL 1
#define MOTOR_BR 2
#define MOTOR_FR 3

typedef struct {
  gpio_pins_t pin;
} motor_pin_def_t;

#define MAKE_MOTOR_PIN_DEF(port, _pin, pin_af, timer, timer_channel) \
  {                                                                  \
    .pin = PIN_IDENT(port, _pin),                                    \
  }

extern const volatile motor_pin_def_t motor_pin_defs[MOTOR_PIN_MAX];

void motor_init();
void motor_set(uint8_t number, float pwm);
void motor_wait_for_ready();
void motor_set_all(float pwm);
void motor_beep();