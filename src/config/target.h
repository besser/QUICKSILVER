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

#define LED_PIN_MEMBERS \
  MEMBER(pin, uint8)    \
  MEMBER(invert, uint8)

typedef struct {
  uint8_t name[32];

  led_pin_t led_pins[LED_PINS_MAX];
  uint8_t led_pin_count;

  motor_pins_t motor_pins[MOTOR_MAX];

  spi_ports_t gyro_spi_port;
  gpio_pins_t gyro_nss;
} target_t;

#define TARGET_MEMBERS                            \
  TSTR_MEMBER(name, 32)                           \
  ARRAY_MEMBER(led_pins, LED_PINS_MAX, led_pin_t) \
  MEMBER(led_pin_count, uint8)                    \
  ARRAY_MEMBER(motor_pins, MOTOR_MAX, uint8)      \
  MEMBER(gyro_spi_port, uint8)                    \
  MEMBER(gyro_nss, uint8)

extern target_info_t target_info;
extern target_t target;

cbor_result_t cbor_encode_target_t(cbor_value_t *enc, const target_t *t);
cbor_result_t cbor_decode_target_t(cbor_value_t *dec, target_t *t);

cbor_result_t cbor_encode_target_info_t(cbor_value_t *enc, const target_info_t *i);
