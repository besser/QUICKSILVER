#include "drv_motor.h"

void motor_set_all(float pwm) {
  motor_set(MOTOR_BL, pwm);
  motor_set(MOTOR_FL, pwm);
  motor_set(MOTOR_FR, pwm);
  motor_set(MOTOR_BR, pwm);
}

#define MOTOR_PIN(port, pin, pin_af, timer, timer_channel) MAKE_MOTOR_PIN_DEF(port, pin, pin_af, timer, timer_channel),

const volatile motor_pin_def_t motor_pin_defs[MOTOR_PIN_MAX] = {
    {},
#include "motor_pins.in"
};

#undef MOTOR_PIN