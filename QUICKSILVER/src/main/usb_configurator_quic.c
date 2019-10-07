#include "usb_configurator.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "blackbox.h"
#include "drv_time.h"
#include "drv_usb.h"
#include "flash.h"
#include "profile.h"
#include "project.h"
#include "sixaxis.h"

#if defined(F405)

#define QUIC_HEADER_LEN 4
#define QUIC_PROTOCOL_VERSION 1
#define quic_errorf(cmd, args...) send_quic_strf(cmd, QUIC_FLAG_ERROR, args)

extern profile_t profile;
extern profile_t default_profile;
extern float rx[4];
extern float rxcopy[4];
extern uint8_t aux[AUX_CHANNEL_MAX];
extern float vbattfilt;
extern float vbatt_comp;
extern unsigned long lastlooptime;

extern uint8_t blackbox_enabled;
extern uint32_t blackbox_rate;

extern uint8_t encode_buffer[1024];
extern uint8_t decode_buffer[1024];

void send_quic(quic_command cmd, quic_flag flag, uint8_t *data, uint16_t len) {
  static uint8_t frame[1024 + QUIC_HEADER_LEN];
  if (len > 1024) {
    return;
  }

  const uint16_t size = len + QUIC_HEADER_LEN;

  frame[0] = USB_MAGIC_QUIC;
  frame[1] = (cmd & (0xff >> 3)) | (flag & (0xff >> 5)) << 5;
  frame[2] = (len >> 8) & 0xFF;
  frame[3] = len & 0xFF;

  for (uint32_t i = 0; i < len; i++) {
    frame[i + QUIC_HEADER_LEN] = data[i];
  }

  usb_serial_write(frame, size);
}

cbor_result_t send_quic_str(quic_command cmd, quic_flag flag, const char *str) {
  const uint32_t size = strlen(str) + 128;
  uint8_t buffer[size];

  cbor_value_t enc;
  cbor_encoder_init(&enc, buffer, size);

  cbor_result_t res = cbor_encode_str(&enc, str);
  if (res < CBOR_OK) {
    return res;
  }
  send_quic(cmd, flag, buffer, cbor_encoder_len(&enc));
  return res;
}

cbor_result_t send_quic_strf(quic_command cmd, quic_flag flag, const char *fmt, ...) {
  const uint32_t size = strlen(fmt) + 128;
  char str[size];

  memset(str, 0, size);

  va_list args;
  va_start(args, fmt);
  vsnprintf(str, size, fmt, args);
  va_end(args);
  return send_quic_str(cmd, flag, str);
}

#define check_cbor_error(cmd)               \
  if (res < CBOR_OK) {                      \
    quic_errorf(cmd, "CBOR ERROR %d", res); \
    return;                                 \
  }

cbor_result_t quic_blackbox(const blackbox_t *blackbox) {
  cbor_result_t res = CBOR_OK;

  cbor_value_t enc;
  cbor_encoder_init(&enc, encode_buffer, 1024);

  res = cbor_encode_blackbox_t(&enc, blackbox);
  if (res < CBOR_OK) {
    quic_errorf(QUIC_CMD_BLACKBOX, "CBOR ERROR %d", res);
    return res;
  }

  send_quic(QUIC_CMD_BLACKBOX, QUIC_FLAG_NONE, encode_buffer, cbor_encoder_len(&enc));

  // reset loop time
  lastlooptime = gettime();

  return res;
}

void get_quic(uint8_t *data, uint32_t len) {
  cbor_result_t res = CBOR_OK;

  cbor_value_t dec;
  cbor_decoder_init(&dec, data, len);

  cbor_value_t enc;
  cbor_encoder_init(&enc, encode_buffer, 1024);

  quic_values value = QUIC_CMD_INVALID;
  res = cbor_decode_uint8(&dec, &value);
  check_cbor_error(QUIC_CMD_GET);

  res = cbor_encode_uint8(&enc, &value);
  check_cbor_error(QUIC_CMD_GET);

  switch (value) {
  case QUIC_VAL_PROFILE: {
    res = cbor_encode_profile_t(&enc, &profile);
    check_cbor_error(QUIC_CMD_GET);

    send_quic(QUIC_CMD_GET, QUIC_FLAG_NONE, encode_buffer, cbor_encoder_len(&enc));
    break;
  }
  case QUIC_VAL_DEFAULT_PROFILE: {
    res = cbor_encode_profile_t(&enc, &default_profile);
    check_cbor_error(QUIC_CMD_GET);

    send_quic(QUIC_CMD_GET, QUIC_FLAG_NONE, encode_buffer, cbor_encoder_len(&enc));
    break;
  }
  case QUIC_VAL_INFO:
    cbor_encode_map(&enc, 1);

    cbor_encode_str(&enc, "protocol");

    static const uint32_t protocol = QUIC_PROTOCOL_VERSION;
    cbor_encode_uint32(&enc, &protocol);

    blackbox_enabled = 1;

    send_quic(QUIC_CMD_GET, QUIC_FLAG_NONE, encode_buffer, cbor_encoder_len(&enc));
    break;
  case QUIC_VAL_BLACKBOX_RATE:
    cbor_encode_uint32(&enc, &blackbox_rate);
    send_quic(QUIC_CMD_GET, QUIC_FLAG_NONE, encode_buffer, cbor_encoder_len(&enc));
    break;
  default:
    quic_errorf(QUIC_CMD_GET, "INVALID VALUE %d", value);
    break;
  }
}

void set_quic(uint8_t *data, uint32_t len) {
  cbor_result_t res = CBOR_OK;

  cbor_value_t dec;
  cbor_decoder_init(&dec, data, len);

  cbor_value_t enc;
  cbor_encoder_init(&enc, encode_buffer, 1024);

  quic_values value;
  res = cbor_decode_uint8(&dec, &value);
  check_cbor_error(QUIC_CMD_SET);

  res = cbor_encode_uint8(&enc, &value);
  check_cbor_error(QUIC_CMD_SET);

  switch (value) {
  case QUIC_VAL_PROFILE: {
    res = cbor_decode_profile_t(&dec, &profile);
    check_cbor_error(QUIC_CMD_SET);

    flash_save();

    res = cbor_encode_profile_t(&enc, &profile);
    check_cbor_error(QUIC_CMD_SET);

    send_quic(QUIC_CMD_SET, QUIC_FLAG_NONE, encode_buffer, cbor_encoder_len(&enc));
    break;
  }
  case QUIC_VAL_BLACKBOX_RATE: {
    res = cbor_decode_uint32(&dec, &blackbox_rate);
    check_cbor_error(QUIC_CMD_SET);

    res = cbor_encode_uint32(&enc, &blackbox_rate);
    check_cbor_error(QUIC_CMD_SET);

    send_quic(QUIC_CMD_SET, QUIC_FLAG_NONE, encode_buffer, cbor_encoder_len(&enc));
    break;
  }
  default:
    quic_errorf(QUIC_CMD_GET, "INVALID VALUE %d", value);
    break;
  }
}

void usb_process_quic() {
  const uint8_t cmd = usb_serial_read_byte();
  if (cmd == QUIC_CMD_INVALID) {
    quic_errorf(QUIC_CMD_INVALID, "INVALID CMD %d", cmd);
    return;
  }

  const uint16_t size = (uint16_t)usb_serial_read_byte() << 8 | usb_serial_read_byte();
  if (size != 0) {
    uint32_t len = usb_serial_read(decode_buffer, size);
    for (uint32_t timeout = 1000; len < size && timeout > 0; --timeout) {
      len += usb_serial_read(decode_buffer + len, size - len);
      delay(10);
    }
    if (len != size) {
      quic_errorf(cmd, "INVALID SIZE %d", size);
      return;
    }
  }

  switch (cmd) {
  case QUIC_CMD_GET:
    get_quic(decode_buffer, size);
    break;
  case QUIC_CMD_SET:
    set_quic(decode_buffer, size);
    break;
  case QUIC_CMD_CAL_IMU:
    gyro_cal(); // for flashing lights
    acc_cal();

#ifdef FLASH_SAVE2
    extern float accelcal[3];
    flash2_fmc_write(accelcal[0] + 127, accelcal[1] + 127);
#endif

#ifdef FLASH_SAVE1
    flash_save();
    flash_load();
#endif
    break;
  default:
    quic_errorf(QUIC_CMD_INVALID, "INVALID CMD %d", cmd);
    break;
  }

  // reset loop time
  lastlooptime = gettime();
}
#endif