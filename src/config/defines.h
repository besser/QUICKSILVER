#pragma once

#include "config.h"
#include "config_helper.h"

#include "target.h"

typedef enum {
  LOOPTIME_2K = 500,
  LOOPTIME_4K = 250,
  LOOPTIME_8K = 125,
} looptime_autodetect_t;

#ifdef STM32F411
#define SYS_CLOCK_FREQ_HZ 108000000
#define PWM_CLOCK_FREQ_HZ 108000000
#define SPI_CLOCK_FREQ_HZ (SYS_CLOCK_FREQ_HZ / 2)

#define LOOPTIME LOOPTIME_4K
#endif

#ifdef STM32F405
#define SYS_CLOCK_FREQ_HZ 168000000
#define PWM_CLOCK_FREQ_HZ 84000000
#define SPI_CLOCK_FREQ_HZ (SYS_CLOCK_FREQ_HZ / 4)

#define LOOPTIME LOOPTIME_8K
#endif

#ifdef STM32F7
#define SYS_CLOCK_FREQ_HZ 216000000
#define PWM_CLOCK_FREQ_HZ 216000000
#define SPI_CLOCK_FREQ_HZ (SYS_CLOCK_FREQ_HZ / 4)

#define LOOPTIME LOOPTIME_8K

#define WITHIN_DTCM_RAM(p) (((uint32_t)p & 0xffff0000) == 0x20000000)
#define WITHIN_DMA_RAM(p) (false)
#endif

#ifdef STM32H7
#define SYS_CLOCK_FREQ_HZ 480000000
#define PWM_CLOCK_FREQ_HZ (SYS_CLOCK_FREQ_HZ / 2)
#define SPI_CLOCK_FREQ_HZ (SYS_CLOCK_FREQ_HZ / 4)

#define LOOPTIME LOOPTIME_8K

#define WITHIN_DTCM_RAM(p) (((uint32_t)p & 0xfffe0000) == 0x20000000)
#define WITHIN_DMA_RAM(p) (((uint32_t)p & 0xfffe0000) == 0x30000000)
#endif

#ifdef USE_FAST_RAM
#define FAST_RAM __attribute__((section(".fast_ram"), aligned(4)))
#else
#define FAST_RAM
#endif

#ifdef USE_DMA_RAM
#define DMA_RAM __attribute__((section(".dma_ram"), aligned(32)))
#else
#define DMA_RAM
#endif

// this should be precalculated by the compiler when parameters are constant
//(1 - alpha. filtertime = 1 / filter-cutoff-frequency) as long as filtertime > sampleperiod
#define FILTERCALC(sampleperiod, filtertime) (1.0f - (6.0f * (float)(sampleperiod)) / (3.0f * (float)(sampleperiod) + (float)(filtertime)))
#define MHZ_TO_HZ(mhz) (mhz * 1000000)

#define MAKE_SEMVER(major, minor, patch) ((major << 16) | (minor << 8) | patch)

// Throttle must drop below this value if arming feature is enabled for arming to take place.  MIX_INCREASE_THROTTLE_3 if enabled
// will also not activate on the ground untill this threshold is passed during takeoff for safety and better staging behavior.
#define THROTTLE_SAFETY .10f

// x (micro)seconds after loss of tx or low bat before buzzer starts
#define BUZZER_DELAY 30e6

#ifndef MOTOR_BEEPS_TIMEOUT
#define MOTOR_BEEPS_TIMEOUT 30e3
#endif

#define ANGLE_PID_SIZE 2

#define ENABLE_SMART_AUDIO
#define ENABLE_TRAMP

#ifdef BRUSHLESS_TARGET
// dshot pin initialization & usb interface to esc
#define USE_DSHOT_DMA_DRIVER
#define USE_SERIAL_4WAY_BLHELI_INTERFACE
#endif

#ifdef BRUSHED_TARGET
// pwm pin initialization
#define USE_PWM_DRIVER
#endif

#if defined(USE_M25P16) || defined(USE_SDCARD)
#define ENABLE_BLACKBOX
#endif

#if defined(BUZZER_ENABLE) && !defined(BUZZER_PIN)
#undef BUZZER_ENABLE
#endif

#ifdef RX_SBUS
#define RX_UNIFIED_SERIAL
#endif
#ifdef RX_CRSF
#define RX_UNIFIED_SERIAL
#endif
#ifdef RX_IBUS
#define RX_UNIFIED_SERIAL
#endif
#ifdef RX_FPORT
#define RX_UNIFIED_SERIAL
#endif
#ifdef RX_DSMX
#define RX_UNIFIED_SERIAL
#endif
#ifdef RX_DSM2
#define RX_UNIFIED_SERIAL
#endif

//*************************************Features that still need to be moved into targets and checked for compatability************************************************
// RGB led type ws2812 - ws2813
// numbers over 8 could decrease performance
//#define RGB_LED_NUMBER 0
//#define RGB_LED_DMA

// pin / port for the RGB led ( programming port ok )
//#define RGB_PIN LL_GPIO_PIN_11
//#define RGB_PORT GPIOA
