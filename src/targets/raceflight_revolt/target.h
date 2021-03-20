#include "config.h"
#include "config_helper.h"

#define Raceflight_Revolt

#define F4
#define F405

//PORTS
#define SPI_PORTS   \
  SPI1_PA5PA6PA7    \
  SPI2_PB13PB14PB15 \
  SPI3_PB3PB4PB5

#define USART_PORTS \
  USART1_PA10PA9    \
  USART3_PB11PB10   \
  USART4_PA1PA0     \
  USART6_PC7PC6

#define USB_DETECT_PIN GPIO_Pin_5
#define USB_DETECT_PORT GPIOC

//LEDS & BUZZER
#define LED_NUMBER 1
#define LED1_INVERT
#define LED1PIN PIN_B5
#define BUZZER_PIN PIN_B4
//#define BUZZER_INVERT
#define FPV_PIN PIN_A13

//GYRO
#define ICM20602_SPI_PORT SPI_PORT1
#define ICM20602_NSS PIN_A4
#define ICM20602_INT PIN_C4
#define USE_DUMMY_I2C
#define GYRO_ID_1 0x68
#define GYRO_ID_2 0x12
#define GYRO_ID_3 0x69
#define GYRO_ID_4 0x72
#define SENSOR_ROTATE_90_CW
//#define DISABLE_GYRO_CHECK

//RADIO
#define USART1_INVERTER_PIN PIN_C0

#ifdef SERIAL_RX
#define RX_USART USART_PORT1
#define SOFTSPI_NONE
#endif
#ifndef SOFTSPI_NONE
#define RADIO_CHECK
#define SPI_MISO_PIN GPIO_Pin_10
#define SPI_MISO_PORT GPIOA
#define SPI_MOSI_PIN GPIO_Pin_9
#define SPI_MOSI_PORT GPIOA
#define SPI_CLK_PIN GPIO_Pin_11
#define SPI_CLK_PORT GPIOB
#define SPI_SS_PIN GPIO_Pin_10
#define SPI_SS_PORT GPIOB
#endif

//VOLTAGE DIVIDER
#define BATTERYPIN PIN_C2
#define BATTERY_ADC_CHANNEL ADC_Channel_12
#ifndef VOLTAGE_DIVIDER_R1
#define VOLTAGE_DIVIDER_R1 10000
#endif
#ifndef VOLTAGE_DIVIDER_R2
#define VOLTAGE_DIVIDER_R2 1000
#endif
#ifndef ADC_REF_VOLTAGE
#define ADC_REF_VOLTAGE 3.3
#endif

// MOTOR PINS
#define MOTOR_PIN0 MOTOR_PIN_PA3
#define MOTOR_PIN1 MOTOR_PIN_PA2
#define MOTOR_PIN2 MOTOR_PIN_PB0
#define MOTOR_PIN3 MOTOR_PIN_PB1
