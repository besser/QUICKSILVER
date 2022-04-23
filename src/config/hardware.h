#pragma once

#define PIN_IDENT(port, num) PIN_##port##num
#define GPIO_PIN(port, num) PIN_IDENT(port, num),

typedef enum {
  PIN_NONE,
#include "gpio_pins.in"
  PINS_MAX,
} gpio_pins_t;

#undef GPIO_PIN

typedef enum {
  MOTOR_PIN0,
  MOTOR_PIN1,
  MOTOR_PIN2,
  MOTOR_PIN3,
  MOTOR_MAX
} motor_index_t;

#define MOTOR_PIN_IDENT(port, pin) MOTOR_PIN_P##port##pin
#define MOTOR_PIN(port, pin, pin_af, timer, timer_channel) MOTOR_PIN_IDENT(port, pin),

typedef enum {
  MOTOR_PIN_INVALID,
#include "motor_pins.in"
  MOTOR_PIN_MAX
} motor_pins_t;

#undef MOTOR_PIN

#define USART_IDENT(channel) USART_PORT##channel
#define SOFT_SERIAL_IDENT(channel) SOFT_SERIAL_PORT##channel

typedef enum {
  USART_PORT_INVALID,
  USART_PORTS_MAX,
  SOFT_SERIAL_PORTS_MAX = USART_PORTS_MAX,
} usart_ports_t;

#define SPI_IDENT(chan, sck_pin, miso_pin, mosi_pin) SPI##chan##_##sck_pin##_##miso_pin##_##mosi_pin
#define SPI_PORT(chan, sck_pin, miso_pin, mosi_pin) SPI_IDENT(chan, sck_pin, miso_pin, mosi_pin),

typedef enum {
  SPI_PORT_INVALID,
#include "spi_ports.in"
  SPI_PORTS_MAX,
} spi_ports_t;

#undef SPI_PORT