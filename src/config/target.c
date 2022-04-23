#include "target.h"

#include "io/quic.h"
#include "profile.h"
#include "rx.h"
#include "util/cbor_helper.h"

target_t target = {
    .name = "unknown",

    .led_pins = {},
    .led_pin_count = 0,

    .motor_pins = {
        MOTOR_PIN_INVALID,
        MOTOR_PIN_INVALID,
        MOTOR_PIN_INVALID,
        MOTOR_PIN_INVALID,
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

CBOR_START_STRUCT_DECODER(target_t)
TARGET_MEMBERS
CBOR_END_STRUCT_DECODER()

#undef MEMBER
#undef STR_MEMBER
#undef TSTR_MEMBER
#undef ARRAY_MEMBER
#undef STR_ARRAY_MEMBER