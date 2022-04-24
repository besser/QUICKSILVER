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

#define USART_IDENT(channel) USART_PORT##channel
#define SOFT_SERIAL_IDENT(channel) SOFT_SERIAL_PORT##channel

typedef enum {
  USART_PORT_INVALID,
  USART_PORTS_MAX,
  SOFT_SERIAL_PORTS_MAX = USART_PORTS_MAX,
} usart_ports_t;

#define SPI_IDENT(spi_prt, dma_prt, chan, rx, tx) SPI_PORT##spi_prt
#define SPI_PORT(spi_prt, dma_prt, chan, rx, tx) SPI_IDENT(spi_prt, dma_prt, chan, rx, tx),

typedef enum {
  SPI_PORT_INVALID,
  SPI_PORT1,
  SPI_PORT2,
  SPI_PORT3,
#if defined(STM32F7) || defined(STM32H7)
  SPI_PORT4,
#endif
  SPI_PORT_MAX,
} spi_port_index_t;

#undef SPI_PORT