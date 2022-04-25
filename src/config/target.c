#include "target.h"

#include <string.h>

#include "io/quic.h"
#include "profile.h"
#include "rx.h"
#include "util/cbor_helper.h"

target_t target = {
    .name = "unknown",

    .led_pins = {},
    .led_pin_count = 0,

    .motor_pins = {
        PIN_NONE,
        PIN_NONE,
        PIN_NONE,
        PIN_NONE,
    },

    .gyro_spi_port = SPI_PORT_INVALID,
    .gyro_nss = PIN_NONE,
};

#define _MACRO_STR(arg) #arg
#define MACRO_STR(name) _MACRO_STR(name)

target_info_t target_info = {
    .mcu = MACRO_STR(MCU_NAME),
    .git_version = MACRO_STR(GIT_VERSION),

    .features = 0
#ifdef BRUSHLESS_TARGET
                | FEATURE_BRUSHLESS
#endif
#ifdef ENABLE_OSD
                | FEATURE_OSD
#endif
#ifdef ENABLE_BLACKBOX
                | FEATURE_BLACKBOX
#endif
#ifdef DEBUG
                | FEATURE_DEBUG
#endif
    ,
    .rx_protocol = RX_PROTOCOL,
    .quic_protocol_version = QUIC_PROTOCOL_VERSION,

    .gyro_id = 0x0,
};

#define MEMBER CBOR_ENCODE_MEMBER
#define STR_MEMBER CBOR_ENCODE_STR_MEMBER
#define TSTR_MEMBER CBOR_ENCODE_TSTR_MEMBER
#define ARRAY_MEMBER CBOR_ENCODE_ARRAY_MEMBER
#define STR_ARRAY_MEMBER CBOR_ENCODE_STR_ARRAY_MEMBER

CBOR_START_STRUCT_ENCODER(target_info_t)
TARGET_INFO_MEMBERS
CBOR_END_STRUCT_ENCODER()

CBOR_START_STRUCT_ENCODER(led_pin_t)
LED_PIN_MEMBERS
CBOR_END_STRUCT_ENCODER()

CBOR_START_STRUCT_ENCODER(serial_port_t)
SERIAL_PORT_MEMBERS
CBOR_END_STRUCT_ENCODER()

CBOR_START_STRUCT_ENCODER(spi_port_t)
SPI_PORT_MEMBERS
CBOR_END_STRUCT_ENCODER()

CBOR_START_STRUCT_ENCODER(target_t)
TARGET_MEMBERS
CBOR_END_STRUCT_ENCODER()

#undef MEMBER
#undef STR_MEMBER
#undef TSTR_MEMBER
#undef ARRAY_MEMBER
#undef STR_ARRAY_MEMBER

#define MEMBER CBOR_DECODE_MEMBER
#define STR_MEMBER CBOR_DECODE_STR_MEMBER
#define TSTR_MEMBER CBOR_DECODE_TSTR_MEMBER
#define ARRAY_MEMBER CBOR_DECODE_ARRAY_MEMBER
#define STR_ARRAY_MEMBER CBOR_DECODE_STR_ARRAY_MEMBER

CBOR_START_STRUCT_DECODER(led_pin_t)
LED_PIN_MEMBERS
CBOR_END_STRUCT_DECODER()

CBOR_START_STRUCT_DECODER(serial_port_t)
SERIAL_PORT_MEMBERS
CBOR_END_STRUCT_DECODER()

CBOR_START_STRUCT_DECODER(spi_port_t)
SPI_PORT_MEMBERS
CBOR_END_STRUCT_DECODER()

CBOR_START_STRUCT_DECODER(target_t)
TARGET_MEMBERS
CBOR_END_STRUCT_DECODER()

#undef MEMBER
#undef STR_MEMBER
#undef TSTR_MEMBER
#undef ARRAY_MEMBER
#undef STR_ARRAY_MEMBER

const uint8_t pin_none_str[] = "NONE";

cbor_result_t cbor_encode_gpio_pins_t(cbor_value_t *enc, const gpio_pins_t *t) {
  cbor_result_t res = CBOR_OK;

  uint8_t buf[4];
  uint8_t size = 3;

  if (*t != PIN_NONE) {
    const uint8_t port = (*t - 1) / 16;
    const uint8_t pin = (*t - 1) % 16;

    buf[0] = 'P';
    buf[1] = 'A' + port;

    if (pin >= 10) {
      buf[2] = '1';
      buf[3] = '0' + pin % 10;
      size = 4;
    } else {
      buf[2] = '0' + pin;
    }
  } else {
    memcpy(buf, pin_none_str, 4);
    size = 4;
  }

  CBOR_CHECK_ERROR(res = cbor_encode_tstr(enc, buf, size));

  return res;
}

cbor_result_t cbor_decode_gpio_pins_t(cbor_value_t *dec, gpio_pins_t *t) {
  cbor_result_t res = CBOR_OK;

  const uint8_t *buf;
  uint32_t size;
  CBOR_CHECK_ERROR(res = cbor_decode_tstr(dec, &buf, &size));

  if (size > 4 && size < 3) {
    return CBOR_ERR_INVALID_TYPE;
  }

  if (size == 4 && memcmp(buf, pin_none_str, 4) == 0) {
    *t = PIN_NONE;
    return res;
  }

  if (buf[0] != 'P') {
    return CBOR_ERR_INVALID_TYPE;
  }

  gpio_pins_t val = 1;
  val += (buf[1] - 'A') * 16;
  if (size == 4) {
    val += 10;
    val += buf[3] - '0';
  } else {
    val += buf[2] - '0';
  }

  *t = val;

  return res;
}

cbor_result_t cbor_encode_spi_port_index_t(cbor_value_t *enc, const spi_port_index_t *t) {
  cbor_result_t res = CBOR_OK;

  if (*t <= SPI_PORT_INVALID || *t >= SPI_PORT_MAX) {
    uint8_t buf[16] = "SPI_PORT_INVALID";
    CBOR_CHECK_ERROR(res = cbor_encode_tstr(enc, buf, 16));
  } else {
    uint8_t buf[9] = "SPI_PORT0";
    buf[8] += *t;

    CBOR_CHECK_ERROR(res = cbor_encode_tstr(enc, buf, 9));
  }

  return res;
}

cbor_result_t cbor_decode_spi_port_index_t(cbor_value_t *dec, spi_port_index_t *t) {
  cbor_result_t res = CBOR_OK;

  const uint8_t *buf;
  uint32_t size;
  CBOR_CHECK_ERROR(res = cbor_decode_tstr(dec, &buf, &size));

  if (size == 16 && memcmp(buf, "SPI_PORT_INVALID", 16) == 0) {
    *t = SPI_PORT_INVALID;
  } else if (size == 9 || memcmp(buf, "SPI_PORT", 8) == 0) {
    const int8_t index = buf[8] - '0';
    if (index <= SPI_PORT_INVALID || index >= SPI_PORT_MAX) {
      return CBOR_ERR_INVALID_TYPE;
    }

    *t = index;
  } else {
    return CBOR_ERR_INVALID_TYPE;
  }

  return res;
}