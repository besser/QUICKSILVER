#include "io/led.h"

#include "drv_gpio.h"
#include "drv_time.h"
#include "flight/control.h"
#include "project.h"
#include "util/util.h"

// for led flash on gestures
int ledcommand = 0;
int ledblink = 0;
uint32_t ledcommandtime = 0;

void led_init() {
  LL_GPIO_InitTypeDef init;
  init.Mode = LL_GPIO_MODE_OUTPUT;
  init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  init.Pull = LL_GPIO_PULL_NO;
  init.Speed = LL_GPIO_SPEED_FREQ_HIGH;

  for (uint8_t i = 0; i < target.led_pin_count; i++) {
    const led_pin_t led = target.led_pins[i];
    gpio_pin_init(&init, led.pin);
    if (led.invert) {
      gpio_pin_set(led.pin);
    } else {
      gpio_pin_reset(led.pin);
    }
  }
}

void led_on(uint8_t val) {
  for (uint8_t i = 0; i < target.led_pin_count; i++) {
    if (val & (i + 1)) {
      const led_pin_t led = target.led_pins[i];
      if (led.invert) {
        gpio_pin_reset(led.pin);
      } else {
        gpio_pin_set(led.pin);
      }
    }
  }
}

void led_off(uint8_t val) {
  for (uint8_t i = 0; i < target.led_pin_count; i++) {
    if (val & (i + 1)) {
      const led_pin_t led = target.led_pins[i];
      if (led.invert) {
        gpio_pin_set(led.pin);
      } else {
        gpio_pin_reset(led.pin);
      }
    }
  }
}

void led_flash(uint32_t period, int duty) {
  if (time_micros() % period > (period * duty) >> 4) {
    led_on(LEDALL);
  } else {
    led_off(LEDALL);
  }
}

// delta- sigma first order modulator.
uint8_t led_pwm(uint8_t pwmval) {
  static float lastledbrightness = 0;
  static uint32_t lastledtime = 0;
  static float ds_integrator = 0;

  uint32_t time = time_micros();
  uint32_t ledtime = time - lastledtime;

  lastledtime = time;

  float desiredbrightness = pwmval * (1.0f / 15.0f);
  limitf(&ds_integrator, 2);

  ds_integrator += (desiredbrightness - lastledbrightness) * ledtime * (1.0f / LOOPTIME);

  if (ds_integrator > 0.49f) {
    led_on(255);
    lastledbrightness = 1.0f;
  } else {
    led_off(255);
    lastledbrightness = 0;
  }
  return 0;
}

void led_update() {
  if (target.led_pin_count <= 0) {
    return;
  }

  // led flash logic
  if (flags.lowbatt) {
    led_flash(500000, 8);
    return;
  }

  if (flags.rx_mode == RXMODE_BIND) { // bind mode
    led_flash(100000, 12);
    return;
  }

  if (flags.failsafe) {
    led_flash(500000, 15);
    return;
  }

  if (ledcommand) {
    if (!ledcommandtime)
      ledcommandtime = time_micros();
    if (time_micros() - ledcommandtime > 500000) {
      ledcommand = 0;
      ledcommandtime = 0;
    }
    led_flash(100000, 8);
    return;
  }

  if (ledblink) {
    uint32_t time = time_micros();
    if (!ledcommandtime) {
      ledcommandtime = time;
      led_off(255);
    }
    if (time - ledcommandtime > 500000) {
      ledblink--;
      ledcommandtime = 0;
    }
    if (time - ledcommandtime > 300000) {
      led_on(255);
    }
  } else { // led is normally on
    if (LED_BRIGHTNESS != 15)
      led_pwm(LED_BRIGHTNESS);
    else
      led_on(255);
  }
}
