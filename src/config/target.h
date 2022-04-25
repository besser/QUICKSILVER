#pragma once

#include <cbor.h>

#include "hardware.h"

#define LED_PINS_MAX 4

typedef enum {
  FEATURE_BRUSHLESS = (1 << 1),
  FEATURE_OSD = (1 << 2),
  FEATURE_BLACKBOX = (1 << 3),
  FEATURE_DEBUG = (1 << 4),
} target_feature_t;

typedef struct {
  const char *mcu;
  const char *git_version;

  uint32_t features;
  uint32_t rx_protocol;
  uint32_t quic_protocol_version;

  uint8_t gyro_id;
} target_info_t;

#define TARGET_INFO_MEMBERS             \
  STR_MEMBER(mcu)                       \
  STR_MEMBER(git_version)               \
  MEMBER(features, uint32)              \
  MEMBER(rx_protocol, uint32)           \
  MEMBER(quic_protocol_version, uint32) \
  MEMBER(gyro_id, uint8)

typedef struct {
  gpio_pins_t pin;
  uint8_t invert;
} led_pin_t;

#define LED_PIN_MEMBERS    \
  MEMBER(pin, gpio_pins_t) \
  MEMBER(invert, uint8)

typedef struct {
  uint8_t index;
  gpio_pins_t rx_pin;
  gpio_pins_t tx_pin;
} serial_port_t;

#define SERIAL_PORT_MEMBERS   \
  MEMBER(index, uint8)        \
  MEMBER(rx_pin, gpio_pins_t) \
  MEMBER(tx_pin, gpio_pins_t)

typedef struct {
  uint8_t index;
  gpio_pins_t sck;
  gpio_pins_t miso;
  gpio_pins_t mosi;
} spi_port_t;

#define SPI_PORT_MEMBERS    \
  MEMBER(index, uint8)      \
  MEMBER(sck, gpio_pins_t)  \
  MEMBER(miso, gpio_pins_t) \
  MEMBER(mosi, gpio_pins_t)

typedef struct {
  uint8_t name[32];

  led_pin_t led_pins[LED_PINS_MAX];
  uint8_t led_pin_count;

  gpio_pins_t motor_pins[MOTOR_MAX];
  serial_port_t serial_ports[SERIAL_PORT_MAX - 1];
  spi_port_t spi_ports[SPI_PORT_MAX - 1];

  gpio_pins_t vbat;
  gpio_pins_t ibat;

  spi_port_index_t gyro_spi_port;
  gpio_pins_t gyro_nss;
  gpio_pins_t gyro_int;
} target_t;

#define TARGET_MEMBERS                                  \
  TSTR_MEMBER(name, 32)                                 \
  ARRAY_MEMBER(led_pins, LED_PINS_MAX, led_pin_t)       \
  MEMBER(led_pin_count, uint8)                          \
  ARRAY_MEMBER(motor_pins, MOTOR_MAX, gpio_pins_t)      \
  ARRAY_MEMBER(spi_ports, SPI_PORT_MAX - 1, spi_port_t) \
  MEMBER(vbat, gpio_pins_t)                             \
  MEMBER(ibat, gpio_pins_t)                             \
  MEMBER(gyro_spi_port, spi_port_index_t)               \
  MEMBER(gyro_nss, gpio_pins_t)                         \
  MEMBER(gyro_int, gpio_pins_t)

extern target_info_t target_info;
extern target_t target;

cbor_result_t cbor_encode_gpio_pins_t(cbor_value_t *enc, const gpio_pins_t *t);
cbor_result_t cbor_decode_gpio_pins_t(cbor_value_t *dec, gpio_pins_t *t);

cbor_result_t cbor_encode_spi_port_index_t(cbor_value_t *enc, const spi_port_index_t *t);
cbor_result_t cbor_decode_spi_port_index_t(cbor_value_t *dec, spi_port_index_t *t);

cbor_result_t cbor_encode_target_t(cbor_value_t *enc, const target_t *t);
cbor_result_t cbor_decode_target_t(cbor_value_t *dec, target_t *t);

cbor_result_t cbor_encode_target_info_t(cbor_value_t *enc, const target_info_t *i);
