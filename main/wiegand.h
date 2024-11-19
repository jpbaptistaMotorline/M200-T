/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|

*/

/*
 * Copyright (c) 2021 Ruslan V. Uss <unclerus@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of itscontributors
 *    may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file wiegand.h
 * @defgroup wiegand wiegand//
 * @{
 *
 * ESP-IDF Wiegand protocol receiver
 *
 * Copyright (c) 2021 Ruslan V. Uss <unclerus@gmail.com>
 *
 * BSD Licensed as described in the file LICENSE
 */
#ifndef __WIEGAND_H__
#define __WIEGAND_H__

#include "cmd_list.h"
#include "timer.h"
#include "users.h"
#include <driver/gpio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define WIEGAND_NORMAL_MODE_LABEL 0
#define WIEGAND_AUTO_SAVE_MODE_LABEL 1
#define WIEGAND_READ_MODE_LABEL 2
#define WIEGAND_KEYPAD_MODE_LABEL 3

#define CONFIG_EXAMPLE_BUF_SIZE 5

uint8_t BLE_SMS_Indication_wiegand_autoSave;
uint8_t gattsIF_wiegand_autoSave;
uint16_t connID_wiegand_autoSave;
uint16_t handle_table_wiegand_autoSave;

typedef struct wiegand_reader wiegand_reader_t;
TimerHandle_t xTimer_autoadd_wiegand1;
TimerHandle_t xTimer_autoadd_wiegand2;

typedef void (*wiegand_callback_t)(wiegand_reader_t *reader);

// static const char *TAG = "wiegand_reader";

static wiegand_reader_t reader;
static wiegand_reader_t reader2;
static QueueHandle_t queue = NULL;
static QueueHandle_t queue2 = NULL;

uint8_t keypadCount;

// Single data packet
typedef struct {
  uint8_t data[CONFIG_EXAMPLE_BUF_SIZE];
  size_t bits;
} data_packet_t;

/**
 * Bit and byte order of data
 */
typedef enum { WIEGAND_MSB_FIRST = 0, WIEGAND_LSB_FIRST } wiegand_order_t;

uint8_t wiegandMode;
uint8_t wiegandMode2;
uint8_t anti_passback_activation;
uint8_t wiegand_antipassback_mode;
uint32_t anti_passback_people_number;
uint32_t antipassback_peopleCounter;

/**
 * Wiegand reader descriptor
 */
struct wiegand_reader {
  gpio_num_t gpio_d0, gpio_d1;
  wiegand_callback_t callback;
  wiegand_order_t bit_order;
  wiegand_order_t byte_order;

  uint8_t *buf;
  size_t size;
  size_t bits;
  esp_timer_handle_t timer;
  bool start_parity;
  bool enabled;
};

MyUser user_auto_controlAcess;

/**
 * @brief Create and initialize reader instance.
 *
 * @param reader           Reader descriptor
 * @param gpio_d0          GPIO pin for D0
 * @param gpio_d1          GPIO pin for D0
 * @param internal_pullups Enable internal pull-up resistors for D0 and D1 GPIO
 * @param buf_size         Reader buffer size in bytes, must be large enough to
 *                         contain entire Wiegand key
 * @param callback         Callback function for processing received codes
 * @param bit_order        Bit order of data
 * @param byte_order       Byte order of data
 * @return `ESP_OK` on success
 */
esp_err_t wiegand_reader_init1(wiegand_reader_t *reader, gpio_num_t gpio_d0,
                               gpio_num_t gpio_d1, bool internal_pullups,
                               size_t buf_size, wiegand_callback_t callback,
                               wiegand_order_t bit_order,
                               wiegand_order_t byte_order);

esp_err_t wiegand_reader_init2(wiegand_reader_t *reader, gpio_num_t gpio_d0,
                               gpio_num_t gpio_d1, bool internal_pullups,
                               size_t buf_size, wiegand_callback_t callback,
                               wiegand_order_t bit_order,
                               wiegand_order_t byte_order);

/**
 * @brief Disable reader
 *
 * While reader is disabled, it will not receive new data
 *
 * @param reader Reader descriptor
 * @return `ESP_OK` on success
 */
esp_err_t wiegand_reader_disable(wiegand_reader_t *reader);

/**
 * @brief Enable reader
 *
 * @param reader Reader descriptor
 * @return `ESP_OK` on success
 */
esp_err_t wiegand_reader_enable(wiegand_reader_t *reader);

/**
 * @brief Delete reader instance.
 *
 * @param reader Reader descriptor
 * @return `ESP_OK` on success
 */
esp_err_t wiegand_reader_done(wiegand_reader_t *reader);
static void reader_callback(wiegand_reader_t *r);
static void reader_callback2(wiegand_reader_t *r);

int countBits(int n);
void wiegandToFacilityCard(int wiegandDecimal, int wiegandBits,
                           int *facilityCode, int *cardCode);

void wiegand2_action(uint64_t wiegandResult);
void wiegand1_action(uint64_t wiegandResult);
char *parseWiegand_data(uint8_t BLE_SMS_Indication,uint8_t gattsIF, uint16_t connID, uint16_t handle_table,char cmd, char param, char *phPassword, char *payload,
                        MyUser *user_validateData, mqtt_information *mqttInfo);
void timer_autoAdd_Callback(TimerHandle_t xTimer);
void timer_autoAdd2_Callback(TimerHandle_t xTimer);

uint8_t activate_wiegand(uint8_t mode);
uint8_t deactivate_wiegand();

uint8_t wiegand_parse_getData(uint64_t wiegand_data, char *keypadValue,
                              uint8_t mode, uint8_t wiegand_relay,uint8_t readerNumber);

uint8_t get_antipassback_user(char *wiegandNumber);
uint8_t set_antipassback_user(char *wiegandNumber, char permition);
uint8_t erase_antipassback_user(char *wiegandNumber);

void wiegand1_task(void *arg);
void wiegand2_task(void *arg);

void parse_put_phoneNumber_to_wiegand(char *payload, char *wiegand,
                                      char *phoneNumber);
char *put_phoneNumber_to_wiegand(char *wiegandNumber, char *phoneNumber);

char *put_wiegand_to_phoneNumber(char *wiegandNumber, char *phoneNumber,
                                 char *relay_wieganPermition);

void parse_put_wiegand_to_phoneNumber(char *payload, char *wiegandNumber,
                                      char *phoneNumber,
                                      char *relay_wiegandPermition);

char *erase_wiegand_number(char *payload);

void parse_edit_wiegand_relay(char *payload, char *wiegandNumber,
                              char *permition);
char *edit_wiegand_relay(char *wiegandNumber, char relay_wiegandPermition);
/**@}*/

uint8_t erase_only_wiegand(char *wiegandNumber, char permition);

uint8_t antipassback_activate(char *payload);
uint8_t antipassback_deactivate();

#endif /* __WIEGAND_H__ */