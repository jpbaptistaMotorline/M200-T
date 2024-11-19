/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|

*/

#ifndef _GPIO_H_
#define _GPIO_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "sdCard.h"

/**
 * Brief:
 * This test code shows how to configure gpio and how to use gpio interrupt.
 *
 * GPIO status:
 * GPIO18: output (ESP32C2/ESP32H2 uses GPIO8 as the second output pin)
 * GPIO19: output (ESP32C2/ESP32H2 uses GPIO9 as the second output pin)
 * GPIO4:  input, pulled up, interrupt from rising edge and falling edge
 * GPIO5:  input, pulled up, interrupt from rising edge.
 *
 * Note. These are the default GPIO pins to be used in the example. You can
 * change IO pins in menuconfig.
 *
 * Test:
 * Connect GPIO18(8) with GPIO4
 * Connect GPIO19(9) with GPIO5
 * Generate pulses on GPIO18(8)/19(9), that triggers interrupt on GPIO4/5
 *
 */



#define CONFIG_GPIO_OUTPUT_0 15     // 15//18
#define CONFIG_GPIO_OUTPUT_1 16     // 19
#define CONFIG_GPIO_OUTPUT_PWRKEY 4 // 19
#define CONFIG_GPIO_OUTPUT_ACT 3    // 19
#define CONFIG_GPIO_OUTPUT_EG91_RESET 2

#define CONFIG_GPIO_INPUT_1 /* 8 */ 19 // 0
#define CONFIG_GPIO_INPUT_0 /*8 13 */ 21 // 1
#define CONFIG_GPIO_INPUT_2 1  // 0
#define CONFIG_GPIO_INPUT_3 14 // 1
#define CONFIG_GPIO_INPUT_4 12 // 1
#define CONFIG_GPIO_INPUT_5 42// 1
#define CONFIG_GPIO_INPUT_6 38// 1

#define CONFIG_GPIO_INPUT_46 46 // 1

#define GPIO_OUTPUT_IO_0 CONFIG_GPIO_OUTPUT_0
#define GPIO_OUTPUT_IO_1 CONFIG_GPIO_OUTPUT_1
#define GPIO_OUTPUT_IO_PWRKEY CONFIG_GPIO_OUTPUT_PWRKEY
#define GPIO_OUTPUT_ACT CONFIG_GPIO_OUTPUT_ACT
#define GPIO_OUTPUT_EG91_RESET CONFIG_GPIO_OUTPUT_EG91_RESET


#define GPIO_OUTPUT_PIN_SEL ((1ULL << GPIO_OUTPUT_IO_0) | (1ULL << GPIO_OUTPUT_IO_1) | (1ULL << GPIO_OUTPUT_IO_PWRKEY) | (1ULL << GPIO_OUTPUT_ACT) /*|  (1ULL << GPIO_OUTPUT_EG91_RESET) */)

#define GPIO_INPUT_IO_0 CONFIG_GPIO_INPUT_0
#define GPIO_INPUT_IO_1 CONFIG_GPIO_INPUT_1
#define GPIO_INPUT_IO_EG91_STATUS CONFIG_GPIO_INPUT_2
#define GPIO_INPUT_IO_CD_SDCARD CONFIG_GPIO_INPUT_3 
#define GPIO_INPUT_IO_SIMPRE CONFIG_GPIO_INPUT_4
#define GPIO_INPUT_EG91_RI CONFIG_GPIO_INPUT_5  
#define GPIO_INPUT_RTC_ALARM CONFIG_GPIO_INPUT_6
#define OUTPUT_GPIO  GPIO_NUM_9 // Troque para o pino GPIO desejado
#define GPIO_INPUT_IO_RESET CONFIG_GPIO_INPUT_46

#define GPIO_INPUT_PIN_SEL ((1ULL << GPIO_INPUT_IO_0) | (1ULL << GPIO_INPUT_IO_1) | (1ULL << GPIO_INPUT_IO_EG91_STATUS) | (1ULL << GPIO_INPUT_IO_CD_SDCARD) | (1ULL << GPIO_INPUT_IO_SIMPRE) | (1ULL << GPIO_INPUT_EG91_RI) | (1ULL << GPIO_INPUT_RTC_ALARM) | (1ULL << GPIO_NUM_9) )

#define ESP_INTR_FLAG_DEFAULT 0

#define INPUT_GPIO_TIME_DEBOUNCE 100

#ifndef CONFIG_ESP32_BUTTON_LONG_PRESS_DURATION_MS
#define CONFIG_ESP32_BUTTON_LONG_PRESS_DURATION_MS (2000)
#endif

#ifndef CONFIG_ESP32_BUTTON_LONG_PRESS_REPEAT_MS
#define CONFIG_ESP32_BUTTON_LONG_PRESS_REPEAT_MS (50)
#endif

#ifndef CONFIG_ESP32_BUTTON_QUEUE_SIZE
#define CONFIG_ESP32_BUTTON_QUEUE_SIZE (4)
#endif

#ifndef CONFIG_ESP32_BUTTON_TASK_STACK_SIZE
#define CONFIG_ESP32_BUTTON_TASK_STACK_SIZE 3072
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define PIN_BIT(x) (1ULL << x)

#define BUTTON_DOWN (1)
#define BUTTON_UP (2)
#define BUTTON_HELD (3)

#define NBIT                65 // number of bit to receive -1
#define NBIT_12BIT          13 // number of bit to receive -1
#define TRFreset            0
#define TRFSYNC             1
#define TRFUNO              2
#define TRFZERO             3
#define TRFUNO_12BIT        4
#define TRFZERO_12BIT       5

#define HIGH_TO             -10 // longest high Te
#define LOW_TO              10 // longest low Te
#define SHORT_HEAD          20 // shortest Thead accepted 2,7ms
#define LONG_HEAD           45 // longest Thead accepted 6,2ms

#define LONG_HEAD_12BIT     95 // longest Thead accepted 13,0ms
#define SHORT_HEAD_12BIT    80 // longest Thead accepted 11,0ms


//#endif


/**
* \typedef    	KEYS_STRUCT
*
* \brief    Estrutura com timer para validação da tecla premida.
*/

/**
* \typedef    	KEYS_STRUCT
*
* \brief    Estrutura com timer para validação da tecla premida.
*/







/*   typedef struct
  {
    uint8_t pin;
    uint8_t event;
  } button_event_t; */

  QueueHandle_t button_init(unsigned long long pin_select);
  QueueHandle_t pulled_button_init(unsigned long long pin_select, gpio_pull_mode_t pull_mode);
  static SemaphoreHandle_t rdySem_Control_refreshSystemTime_task;

  uint8_t Input_GPIO_Debounce(uint32_t io_num);

  // static QueueHandle_t gpio_evt_queue;

  void gpio_init(void);

  void task_refresh_SystemTime(void *pvParameter);

   void task_readInputs(void *pvParameter);


void input1_Alarme_Feedbacks_processing(uint8_t inputLevel);
void input2_Alarme_Feedbacks_processing(uint8_t inputLevel);
#endif