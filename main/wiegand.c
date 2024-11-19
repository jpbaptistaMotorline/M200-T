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
 * @file wiegand.c
 *
 * ESP-IDF Wiegand protocol receiver
 *
 * Copyright (c) 2021 Ruslan V. Uss <unclerus@gmail.com>
 *
 * BSD Licensed as described in the file LICENSE
 */
#include <esp_log.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <esp_idf_lib_helpers.h>
#include "cmd_list.h"
#include "core.h"
#include "erro_list.h"
#include "esp_err.h"
#include "esp_gatts_api.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "nvs.h"
#include "rele.h"
#include "users.h"
#include "wiegand.h"

#define TIMER_INTERVAL_US 50000 // 50000 // 50ms

static const char *TAG = "wiegand";
char rsp_pointer[200] = {};

#define CHECK(x)                                                               \
  do {                                                                         \
    esp_err_t __;                                                              \
    if ((__ = x) != ESP_OK)                                                    \
      return __;                                                               \
  } while (0)
#define CHECK_ARG(VAL)                                                         \
  do {                                                                         \
    if (!(VAL))                                                                \
      return ESP_ERR_INVALID_ARG;                                              \
  } while (0)

static void isr_disable(wiegand_reader_t *reader) {
  gpio_set_intr_type(reader->gpio_d0, GPIO_INTR_DISABLE);
  gpio_set_intr_type(reader->gpio_d1, GPIO_INTR_DISABLE);
}

static void isr_enable(wiegand_reader_t *reader) {
  gpio_set_intr_type(reader->gpio_d0, GPIO_INTR_NEGEDGE);
  gpio_set_intr_type(reader->gpio_d1, GPIO_INTR_NEGEDGE);
}

#if HELPER_TARGET_IS_ESP32
static void IRAM_ATTR isr_handler(void *arg)
#else
static void isr_handler(void *arg)
#endif
{
  wiegand_reader_t *reader = (wiegand_reader_t *)arg;
  if (!reader->enabled)
    return;

  int d0 = !gpio_get_level(reader->gpio_d0);
  int d1 = !gpio_get_level(reader->gpio_d1);

  // ////printf("\nd0 - %d / d1 - %d\n",d0,d1);
  //  ignore equal
  if (d0 == d1)
    return;
  // overflow
  if (reader->bits >= reader->size * 8)
    return;

  esp_timer_stop(reader->timer);

  uint8_t value;
  if (reader->bit_order == WIEGAND_MSB_FIRST) {
    value = (d0 ? 0x80 : 0) >> (reader->bits % 8);
    // ////printf("\n\n value1 %d\n\n",value);
  } else {
    value = (d0 ? 1 : 0) << (reader->bits % 8);
    // ////printf("\n\n value2 %d\n\n",value);
  }

  if (reader->byte_order == WIEGAND_MSB_FIRST)
    reader->buf[reader->size - reader->bits / 8 - 1] |= value;
  else
    reader->buf[reader->bits / 8] |= value;

  reader->bits++;

  esp_timer_start_once(reader->timer, TIMER_INTERVAL_US);
}

static void timer_handler2(void *arg) {
  wiegand_reader_t *reader = (wiegand_reader_t *)arg;

  ESP_LOGD(TAG, "Got %d bits of data", reader->bits);

  wiegand_reader_disable(reader);

  if (reader->callback)
    reader->callback(reader);

  wiegand_reader_enable(reader);

  isr_enable(reader);
}

static void timer_handler(void *arg) {
  wiegand_reader_t *reader = (wiegand_reader_t *)arg;

  ESP_LOGD(TAG, "Got %d bits of data", reader->bits);

  wiegand_reader_disable(reader);

  if (reader->callback)
    reader->callback(reader);

  wiegand_reader_enable(reader);

  isr_enable(reader);
}

////////////////////////////////////////////////////////////////////////////////
esp_err_t wiegand_reader_init2(wiegand_reader_t *reader, gpio_num_t gpio_d0,
                               gpio_num_t gpio_d1, bool internal_pullups,
                               size_t buf_size, wiegand_callback_t callback,
                               wiegand_order_t bit_order,
                               wiegand_order_t byte_order) {
  CHECK_ARG(reader && buf_size && callback);

  wiegandMode = 0;

  esp_err_t res = gpio_install_isr_service(0);
  if (res != ESP_OK && res != ESP_ERR_INVALID_STATE)
    return res;

  memset(reader, 0, sizeof(wiegand_reader_t));
  reader->gpio_d0 = gpio_d0;
  reader->gpio_d1 = gpio_d1;
  reader->size = buf_size;
  reader->buf = calloc(buf_size, 1);
  reader->bit_order = bit_order;
  reader->byte_order = byte_order;
  reader->callback = callback;

  esp_timer_create_args_t timer_args = {.name = TAG,
                                        .arg = reader,
                                        .callback = timer_handler2,
                                        .dispatch_method = ESP_TIMER_TASK};
  CHECK(esp_timer_create(&timer_args, &reader->timer));

  CHECK(gpio_set_direction(gpio_d0, GPIO_MODE_INPUT));
  CHECK(gpio_set_direction(gpio_d1, GPIO_MODE_INPUT));
  CHECK(gpio_set_pull_mode(gpio_d0, internal_pullups ? GPIO_PULLUP_ONLY
                                                     : GPIO_FLOATING));
  CHECK(gpio_set_pull_mode(gpio_d1, internal_pullups ? GPIO_PULLUP_ONLY
                                                     : GPIO_FLOATING));
  isr_disable(reader);
  CHECK(gpio_isr_handler_add(gpio_d0, isr_handler, reader));
  CHECK(gpio_isr_handler_add(gpio_d1, isr_handler, reader));
  isr_enable(reader);
  reader->enabled = true;
  ESP_LOGD(TAG, "Reader initialized on D0=%d, D1=%d", gpio_d0, gpio_d1);
  return ESP_OK;
}

esp_err_t wiegand_reader_init1(wiegand_reader_t *reader, gpio_num_t gpio_d0,
                               gpio_num_t gpio_d1, bool internal_pullups,
                               size_t buf_size, wiegand_callback_t callback,
                               wiegand_order_t bit_order,
                               wiegand_order_t byte_order) {
  CHECK_ARG(reader && buf_size && callback);

  wiegandMode = get_INT8_Data_From_Storage(NVS_ANTIPASSBACK_MODE_LABEL,
                                           nvs_System_handle);

  // antipassback_peopleCounter = nvs_get_u32(nvs_System_handle,nvs_ant,
  // uint32_t *out_value);
  //  anti_passback_activation = 1;
  //  wiegand_antipassback_mode = 2;
  BLE_SMS_Indication_wiegand_autoSave = 0;
  gattsIF_wiegand_autoSave = 0;
  connID_wiegand_autoSave = 0;
  handle_table_wiegand_autoSave = 0;

  esp_err_t res = gpio_install_isr_service(0);
  if (res != ESP_OK && res != ESP_ERR_INVALID_STATE)
    return res;

  memset(reader, 0, sizeof(wiegand_reader_t));
  reader->gpio_d0 = gpio_d0;
  reader->gpio_d1 = gpio_d1;
  reader->size = buf_size;
  reader->buf = calloc(buf_size, 1);
  reader->bit_order = bit_order;
  reader->byte_order = byte_order;
  reader->callback = callback;

  esp_timer_create_args_t timer_args = {.name = TAG,
                                        .arg = reader,
                                        .callback = timer_handler,
                                        .dispatch_method = ESP_TIMER_TASK};
  CHECK(esp_timer_create(&timer_args, &reader->timer));

  CHECK(gpio_set_direction(gpio_d0, GPIO_MODE_INPUT));
  CHECK(gpio_set_direction(gpio_d1, GPIO_MODE_INPUT));
  CHECK(gpio_set_pull_mode(gpio_d0, internal_pullups ? GPIO_PULLUP_ONLY
                                                     : GPIO_FLOATING));
  CHECK(gpio_set_pull_mode(gpio_d1, internal_pullups ? GPIO_PULLUP_ONLY
                                                     : GPIO_FLOATING));
  isr_disable(reader);
  CHECK(gpio_isr_handler_add(gpio_d0, isr_handler, reader));
  CHECK(gpio_isr_handler_add(gpio_d1, isr_handler, reader));
  isr_enable(reader);
  reader->enabled = true;
  ESP_LOGD(TAG, "Reader initialized on D0=%d, D1=%d", gpio_d0, gpio_d1);
  return ESP_OK;
}

esp_err_t wiegand_reader_disable(wiegand_reader_t *reader) {
  CHECK_ARG(reader);

  isr_disable(reader);
  esp_timer_stop(reader->timer);
  reader->enabled = false;

  ESP_LOGD(TAG, "Reader on D0=%d, D1=%d disabled", reader->gpio_d0,
           reader->gpio_d1);
  return ESP_OK;
}

esp_err_t wiegand_reader_enable(wiegand_reader_t *reader) {
  CHECK_ARG(reader);

  reader->bits = 0;
  memset(reader->buf, 0, reader->size);

  isr_enable(reader);
  reader->enabled = true;

  ESP_LOGD(TAG, "Reader on D0=%d, D1=%d enabled", reader->gpio_d0,
           reader->gpio_d1);

  return ESP_OK;
}

esp_err_t wiegand_reader_done(wiegand_reader_t *reader) {
  CHECK_ARG(reader && reader->buf);

  isr_disable(reader);
  CHECK(gpio_isr_handler_remove(reader->gpio_d0));
  CHECK(gpio_isr_handler_remove(reader->gpio_d1));
  esp_timer_stop(reader->timer);
  CHECK(esp_timer_delete(reader->timer));
  free(reader->buf);

  ESP_LOGD(TAG, "Reader removed");

  return ESP_OK;
}

// callback on new data in reader
static void reader_callback(wiegand_reader_t *r) {
  // you can decode raw data from reader buffer here, but remember:
  // reader will ignore any new incoming data while executing callback

  // create simple undecoded data packet
  data_packet_t p;
  p.bits = r->bits;
  memcpy(p.data, r->buf, CONFIG_EXAMPLE_BUF_SIZE);

  // Send it to the queue
  xQueueSendToBack(queue, &p, 0);
}

// callback on new data in reader
static void reader_callback2(wiegand_reader_t *r) {
  // you can decode raw data from reader buffer here, but remember:
  // reader will ignore any new incoming data while executing callback

  // create simple undecoded data packet
  data_packet_t p;
  p.bits = r->bits;
  memcpy(p.data, r->buf, CONFIG_EXAMPLE_BUF_SIZE);

  // Send it to the queue
  xQueueSendToBack(queue2, &p, 0);
}

void wiegand2_task(void *arg) {
  // Create queue
  queue2 = xQueueCreate(5, sizeof(data_packet_t));
  // ESP_ERROR_CHECK( heap_trace_init_standalone(trace_record, NUM_RECORDS) );
  if (!queue2) {
    // ESP_LOGE(TAG, "Error creating queue");
    ESP_ERROR_CHECK(ESP_ERR_NO_MEM);
  }

  // Initialize reader
  ESP_ERROR_CHECK(wiegand_reader_init2(
      &reader2, 40, 39, true, CONFIG_EXAMPLE_BUF_SIZE, reader_callback2,
      WIEGAND_MSB_FIRST, WIEGAND_LSB_FIRST));

  xTimer_autoadd_wiegand2 =
      xTimerCreate("xTimer_autoadd2_wiegand", // Nome do timer
                   pdMS_TO_TICKS(10000),      // Período em milissegundos
                   false,                     // Modo de repetição automática
                   3,                         // ID do timer (0 neste exemplo)
                   timer_autoAdd2_Callback    // Função de callback
      );

  uint8_t keypadCode[7];
  keypadCount = 0;
  uint8_t keyPadIndex = 0;

  xTimerStop(xTimer_autoadd_wiegand2, 0);

  data_packet_t p;
  while (1) {
    // ESP_LOGI("TAG", "Waiting for Wiegand data...");
    xQueueReceive(queue2, &p, portMAX_DELAY);

    // dump received data
    // //printf("==========================================\n");
    // //printf("Bits received: %d\n", p.bits);
    // //printf("Received data: - %d", p.bits);

    uint8_t bits_to_shift = 0;
    uint64_t wiegandResult = 0;

    wiegandResult =
        (~wiegandResult + 1); // * -1;
                              // wiegandResult = (~wiegandResult) * -1;

    if (p.bits == 26) {
      bits_to_shift = 32 - p.bits;
    } else if (p.bits == 34) {
      bits_to_shift = 40 - p.bits;
    }

    // wiegandResult &= 16777215;
    // wiegandResult = wiegandResult >> (bits_to_shift);

    int bytes = p.bits / 8;
    int tail = p.bits % 8;

    for (size_t i = 0; i < bytes + (tail ? 1 : 0); i++) {
      // printf(" 0x%02x", p.data[i]);
      wiegandResult = (wiegandResult << 8) | p.data[i];
    }
    printf("\nwiegand 21- %lld\n", wiegandResult);
    // wiegandResult = wiegandResult & 0x03FFFFFF;

    if (p.bits == 26) {
      wiegandResult = wiegandResult >> (bits_to_shift);
    } else if (p.bits == 34) {
      wiegandResult = wiegandResult >> (bits_to_shift /* + 1 */);
    }

    printf("\nwiegand 22- %lld\n", wiegandResult);

    if (wiegandResult == 160 && keypadCount == 0) {
      keypadCount = 1;
      wiegandMode2 = WIEGAND_KEYPAD_MODE_LABEL;
      xTimerChangePeriod(xTimer_autoadd_wiegand2, pdMS_TO_TICKS(10000), 0);
      // printf("\nwiegand keypadCount 55- %d\n", keypadCount);
      memset(keypadCode, 0, sizeof(keypadCode));
    } else if (wiegandResult == 176 && keypadCount == 1) {

      ESP_LOG_BUFFER_HEX("keypadCode ", keypadCode, 6);
      uint64_t keyPadValue =
          ((uint64_t)keypadCode[5] << 40) | ((uint64_t)keypadCode[4] << 32) |
          ((uint64_t)keypadCode[3] << 24) | ((uint64_t)keypadCode[2] << 16) |
          ((uint64_t)keypadCode[1] << 8) | keypadCode[0];
      keypadCode[keyPadIndex++] = 34;

      wiegand2_action(keyPadValue);
      // wiegand_parse_getData(keyPadValue, &keypadCode,
      // WIEGAND_KEYPAD_MODE_LABEL,
      //  2);
      keyPadIndex = 0;
      keypadCount = 0;
      wiegandMode2 = WIEGAND_NORMAL_MODE_LABEL;
    }

    // //printf("\n==========================================\n");
    if (wiegandMode2 == WIEGAND_KEYPAD_MODE_LABEL) {
      if (keyPadIndex < 6) {
        if (wiegandResult != 160) {
          // printf("\nwiegand keyPadIndex- %lld\n", wiegandResult);
          keypadCode[keyPadIndex++] = (wiegandResult >> 4) + 48;
          // printf("\nwiegand keyPadIndex-aaaa %lld\n", wiegandResult);
        }
      } else {
        // printf("\nwiegand xTimerStop- %lld\n", wiegandResult);
        xTimerStop(xTimer_autoadd_wiegand2, 0);
        memset(keypadCode, 0, sizeof(keypadCode));
        keyPadIndex = 0;
        keypadCount = 0;
        wiegandMode2 = WIEGAND_NORMAL_MODE_LABEL;
      }
    } else if (wiegandMode2 == WIEGAND_NORMAL_MODE_LABEL) {
      wiegand2_action(wiegandResult);
    }
  }
}

void wiegand2_action(uint64_t wiegandResult) {
  ESP_LOGI("WIEGAND", "wiegand2_action: %lld", wiegandResult);
  uint8_t relay_wiegand2 = 0;
  uint8_t antipassback_autorization = 0;

  if (get_INT8_Data_From_Storage(NVS_ANTIPASSBACK_ACTIVATE_LABEL,
                                 nvs_System_handle) == 1) {
    anti_passback_activation = 1;
  } else {
    anti_passback_activation = 0;
  }

  if (anti_passback_activation == 1) {
    relay_wiegand2 = 1;
    char auxWiegan_number[50] = {0};
    char auxWiegan_number2[50] = {0};
    char auxGetUserList[200] = {0};
    char *wiegandData_str;

    asprintf(&wiegandData_str, "%lld", wiegandResult);
    sprintf(auxWiegan_number, "$%lld", wiegandResult);

    ESP_LOGI("WIEGAND", "wiegand2_action: checking if %s exists",
             wiegandData_str);

    if (checkIf_wiegandExist(auxWiegan_number, auxWiegan_number2)) {
      MyUser myUser_antipassback;
      memset(&myUser_antipassback, 0, sizeof(myUser_antipassback));
      ESP_LOGI("WIEGAND", "wiegand2_action: %s exists, checking antipassback",
               wiegandData_str);

      MyUser_Search_User(auxWiegan_number2, auxGetUserList);

      parse_ValidateData_User(auxGetUserList, &myUser_antipassback);

      if (get_antipassback_user(myUser_antipassback.wiegand_code) == ESP_OK) {

        nvs_get_u32(nvs_System_handle, NVS_ANTIPASSBACK_PEOPLE_NUMBER,
                    &anti_passback_people_number);
        nvs_get_u32(nvs_System_handle, NVS_ANTIPASSBACK_PEOPLE_COUNTER,
                    &antipassback_peopleCounter);

        if (anti_passback_people_number == 0) {
          return;
        } else {
          anti_passback_people_number--;
          nvs_set_u32(nvs_System_handle, NVS_ANTIPASSBACK_PEOPLE_NUMBER,
                      anti_passback_people_number);
        }

        wiegand_antipassback_mode = get_INT8_Data_From_Storage(
            NVS_ANTIPASSBACK_MODE_LABEL, nvs_System_handle);
        if (wiegand_antipassback_mode == 2) {
          relay_wiegand2 = 2;
        } else {
          relay_wiegand2 = 1;
        }

        antipassback_autorization = 1;
        erase_antipassback_user(wiegandData_str);
        ESP_LOGI("WIEGAND",
                 "wiegand2_action: antipassback authorization GRANTED for %s",
                 wiegandData_str);
      } else {
        ESP_LOGI("WIEGAND",
                 "wiegand2_action: antipassback authorization DENIED for %s",
                 wiegandData_str);
        antipassback_autorization = 0;
      }
    } else {
      ESP_LOGI("WIEGAND", "wiegand2_action: %s does not exist",
               wiegandData_str);
      antipassback_autorization = 0;
      erase_antipassback_user(wiegandData_str);
    }
    free(wiegandData_str);
  } else {
    relay_wiegand2 = 2;
    antipassback_autorization = 1;
    ESP_LOGI("WIEGAND",
             "wiegand2_action: antipassback disabled, authorization GRANTED");
  }

  if (antipassback_autorization == 1) {
    antipassback_autorization = 0;
    ESP_LOGI("WIEGAND",
             "wiegand2_action: calling wiegand_parse_getData with "
             "relay_wiegand2 = %d",
             relay_wiegand2);
    wiegand_parse_getData(wiegandResult, NULL, WIEGAND_NORMAL_MODE_LABEL,
                          relay_wiegand2,2);
  }
}

void wiegand1_action(uint64_t wiegandResult) {

  uint8_t antipassback_autorization = 0;
  char auxWiegan_number[50] = {0};
  char auxWiegan_number2[50] = {0};
  char auxGetUserList[200] = {0};
  ESP_LOGI("WIEGAND", "wiegand1_action: starting with wiegandResult %lld",
           wiegandResult);

  if (get_INT8_Data_From_Storage(NVS_ANTIPASSBACK_ACTIVATE_LABEL,
                                 nvs_System_handle) == 1) {
    anti_passback_activation = 1;
  } else {
    anti_passback_activation = 0;
  }

  ESP_LOGI("WIEGAND", "wiegand1_action: antipassback activation %d",
           anti_passback_activation);

  if (anti_passback_activation == 1) {
    ESP_LOGI("WIEGAND", "wiegand1_action: checking antipassback for %lld",
             wiegandResult);

    char *wiegandData_str;

    asprintf(&wiegandData_str, "%lld", wiegandResult);
    sprintf(auxWiegan_number, "$%lld", wiegandResult);

    if (checkIf_wiegandExist(auxWiegan_number, auxWiegan_number2)) {

      MyUser myUser_antipassback;
      memset(&myUser_antipassback, 0, sizeof(myUser_antipassback));
      ESP_LOGI("WIEGAND", "wiegand2_action: %s exists, checking antipassback",
               wiegandData_str);

      MyUser_Search_User(auxWiegan_number2, auxGetUserList);

      parse_ValidateData_User(auxGetUserList, &myUser_antipassback);

      ESP_LOGI("WIEGAND", "wiegand1_action: checking antipassback user %s",
               wiegandData_str);

      if (get_antipassback_user(wiegandData_str) != ESP_OK) {

        ESP_LOGI("WIEGAND", "wiegand1_action: getting antipassback people "
                            "number and counter");
        nvs_get_u32(nvs_System_handle, NVS_ANTIPASSBACK_PEOPLE_NUMBER,
                    &anti_passback_people_number);
        nvs_get_u32(nvs_System_handle, NVS_ANTIPASSBACK_PEOPLE_COUNTER,
                    &antipassback_peopleCounter);

        ESP_LOGI("WIEGAND", "wiegand1_action: antipassback people number %d",
                 anti_passback_people_number);
        ESP_LOGI("WIEGAND", "wiegand1_action: antipassback people counter %d",
                 antipassback_peopleCounter);

        if (anti_passback_people_number >= antipassback_peopleCounter) {
          ESP_LOGI("WIEGAND",
                   "wiegand1_action: antipassback people number is greater "
                   "or equal to the antipassback people counter");
          return;
        } else {
          ESP_LOGI("WIEGAND",
                   "wiegand1_action: incrementing antipassback people number");
          anti_passback_people_number++;
          ESP_LOGI("WIEGAND",
                   "wiegand1_action: setting antipassback people number %d",
                   anti_passback_people_number);
          nvs_set_u32(nvs_System_handle, NVS_ANTIPASSBACK_PEOPLE_NUMBER,
                      anti_passback_people_number);

          if (set_antipassback_user(wiegandData_str,
                                    myUser_antipassback.permition) == ESP_OK) {
            antipassback_autorization = 1;
            ESP_LOGI(
                "WIEGAND",
                "wiegand1_action: antipassback authorization GRANTED for %s",
                wiegandData_str);

          } else {
            ESP_LOGI("WIEGAND",
                     "wiegand1_action: error setting antipassback user %s",
                     wiegandData_str);
          }
        }
        ESP_LOGI("WIEGAND", "wiegand1_action: setting antipassback user %s",
                 wiegandData_str);

      } else {
        ESP_LOGI("WIEGAND",
                 "wiegand1_action: antipassback authorization DENIED for %s",
                 wiegandData_str);
        antipassback_autorization = 0;
      }
    } else {
      ESP_LOGI("WIEGAND", "wiegand1_action: user %s not found in wiegand",
               wiegandData_str);
      nvs_erase_key(nvs_wiegand_antipassback_USER_handle, wiegandData_str);
      antipassback_autorization = 0;
    }

    free(wiegandData_str);
  } else {
    ESP_LOGI("WIEGAND",
             "wiegand1_action: antipassback disabled, authorization GRANTED");
    // relay_wiegand2 = 2;
    antipassback_autorization = 1;
  }

  if (antipassback_autorization == 1) {
    antipassback_autorization = 0;
    ESP_LOGI("WIEGAND", "wiegand1_action: calling wiegand_parse_getData with "
                        "relay_wiegand1 = 1");
    wiegand_parse_getData(wiegandResult, NULL, WIEGAND_NORMAL_MODE_LABEL, 1,1);
  }
}

uint8_t get_antipassback_user(char *wiegandNumber) {

  size_t required_size;
  uint8_t ACK = 1;

  ESP_LOGI("WIEGAND", "get_antipassback_user: checking user %s", wiegandNumber);

  if (nvs_get_str(nvs_wiegand_antipassback_USER_handle, wiegandNumber, NULL,
                  &required_size) == ESP_OK) {
    ESP_LOGI("WIEGAND", "get_antipassback_user: user %s found in user list",
             wiegandNumber);
    ACK = 0;
    goto final;
  }

  if (nvs_get_str(nvs_wiegand_antipassback_ADMIN_handle, wiegandNumber, NULL,
                  &required_size) == ESP_OK) {
    ESP_LOGI("WIEGAND", "get_antipassback_user: user %s found in admin list",
             wiegandNumber);
    ACK = 0;
    goto final;
  }

  if (nvs_get_str(nvs_wiegand_antipassback_OWNER_handle, wiegandNumber, NULL,
                  &required_size) == ESP_OK) {
    ESP_LOGI("WIEGAND", "get_antipassback_user: user %s found in owner list",
             wiegandNumber);
    ACK = 0;
    goto final;
  }

  ESP_LOGI("WIEGAND", "get_antipassback_user: user %s not found",
           wiegandNumber);

final:
  ESP_LOGI("WIEGAND", "goto final %d not found", ACK);
  return ACK;
}
uint8_t set_antipassback_user(char *wiegandNumber, char permition) {

  if (permition == '0') {
    return nvs_set_str(nvs_wiegand_antipassback_USER_handle, wiegandNumber,
                       "1");
  } else if (permition == '1') {
    return nvs_set_str(nvs_wiegand_antipassback_ADMIN_handle, wiegandNumber,
                       "1");
  } else if (permition == '2') {
    return nvs_set_str(nvs_wiegand_antipassback_OWNER_handle, wiegandNumber,
                       "1");
  }

  return 1;
}

uint8_t erase_antipassback_user(char *wiegandNumber) {

  nvs_erase_key(nvs_wiegand_antipassback_USER_handle, wiegandNumber);
  nvs_erase_key(nvs_wiegand_antipassback_ADMIN_handle, wiegandNumber);
  nvs_erase_key(nvs_wiegand_antipassback_OWNER_handle, wiegandNumber);
  return ESP_OK;
}
// #include "esp_heap_trace.h"

// #define NUM_RECORDS 100
// static heap_trace_record_t trace_record[NUM_RECORDS]; // This buffer must be
// in internal RAM

void wiegand1_task(void *arg) {
  // Create queue
  queue = xQueueCreate(5, sizeof(data_packet_t));
  // ESP_ERROR_CHECK( heap_trace_init_standalone(trace_record, NUM_RECORDS) );
  if (!queue) {
    // ESP_LOGE(TAG, "Error creating queue");
    ESP_ERROR_CHECK(ESP_ERR_NO_MEM);
  }

  // Initialize reader
  ESP_ERROR_CHECK(wiegand_reader_init1(&reader, 7, 6, true,
                                       CONFIG_EXAMPLE_BUF_SIZE, reader_callback,
                                       WIEGAND_MSB_FIRST, WIEGAND_LSB_FIRST));

  xTimer_autoadd_wiegand1 =
      xTimerCreate("xTimer_autoadd_wiegand1", // Nome do timer
                   pdMS_TO_TICKS(10000),      // Período em milissegundos
                   false,                     // Modo de repetição automática
                   1,                         // ID do timer (0 neste exemplo)
                   timer_autoAdd_Callback     // Função de callback
      );

  uint8_t keypadCode[7];
  keypadCount = 0;
  uint8_t keyPadIndex = 0;

  xTimerStop(xTimer_autoadd_wiegand1, 0);

  data_packet_t p;
  while (1) {
    // ESP_LOGI("TAG", "Waiting for Wiegand data...");
    xQueueReceive(queue, &p, portMAX_DELAY);

    // dump received data
    // //printf("==========================================\n");
    // //printf("Bits received: %d\n", p.bits);
    // //printf("Received data: - %d", p.bits);

    uint8_t bits_to_shift = 0;
    uint64_t wiegandResult = 0;

    wiegandResult =
        (~wiegandResult + 1); // * -1;
                              // wiegandResult = (~wiegandResult) * -1;

    if (p.bits == 26) {
      bits_to_shift = 32 - p.bits;
    } else if (p.bits == 34) {
      bits_to_shift = 40 - p.bits;
    }

    // wiegandResult &= 16777215;
    // wiegandResult = wiegandResult >> (bits_to_shift);

    int bytes = p.bits / 8;
    int tail = p.bits % 8;

    for (size_t i = 0; i < bytes + (tail ? 1 : 0); i++) {
      // printf(" 0x%02x", p.data[i]);
      wiegandResult = (wiegandResult << 8) | p.data[i];
    }
    printf("\nwiegand 1- %lld\n", wiegandResult);
    // wiegandResult = wiegandResult & 0x03FFFFFF;

    if (p.bits == 26) {
      wiegandResult = wiegandResult >> (bits_to_shift);
    } else if (p.bits == 34) {
      wiegandResult = wiegandResult >> (bits_to_shift /* + 1 */);
    }

    printf("\nwiegand 1- %lld\n", wiegandResult);

    if (wiegandResult == 160 && keypadCount == 0) {

      keypadCount = 1;
      wiegandMode = WIEGAND_KEYPAD_MODE_LABEL;
      xTimerChangePeriod(xTimer_autoadd_wiegand1, pdMS_TO_TICKS(10000), 0);
      // printf("\nwiegand keypadCount 55- %d\n", keypadCount);
      memset(keypadCode, 0, sizeof(keypadCode));

    } else if (wiegandResult == 176 && keypadCount == 1) {

      ESP_LOG_BUFFER_HEX("keypadCode ", keypadCode, 6);
      uint64_t keyPadValue =
          ((uint64_t)keypadCode[5] << 40) | ((uint64_t)keypadCode[4] << 32) |
          ((uint64_t)keypadCode[3] << 24) | ((uint64_t)keypadCode[2] << 16) |
          ((uint64_t)keypadCode[1] << 8) | keypadCode[0];
      keypadCode[keyPadIndex++] = 34;
      // printf("\n\nhexhex 0x%llx \n\n", keyPadValue);
      //  ESP_LOG_BUFFER_HEX("keyPadValue ", keyPadValue, 8);
      // printf("\nwiegand 89898- %lld\n", keyPadValue);
      // wiegand_parse_getData(keyPadValue, &keypadCode,
      // WIEGAND_KEYPAD_MODE_LABEL, 1);
      wiegand1_action(keypadCode);
      keyPadIndex = 0;
      keypadCount = 0;
      wiegandMode = WIEGAND_NORMAL_MODE_LABEL;
    }

    // //printf("\n==========================================\n");
    if (wiegandMode == WIEGAND_KEYPAD_MODE_LABEL) {
      if (keyPadIndex < 6) {
        if (wiegandResult != 160) {
          // printf("\nwiegand keyPadIndex- %lld\n", wiegandResult);
          keypadCode[keyPadIndex++] = (wiegandResult >> 4) + 48;
          // printf("\nwiegand keyPadIndex-aaaa %lld\n", wiegandResult);
        }
      } else {
        // printf("\nwiegand xTimerStop- %lld\n", wiegandResult);
        xTimerStop(xTimer_autoadd_wiegand1, 0);
        memset(keypadCode, 0, sizeof(keypadCode));
        keyPadIndex = 0;
        keypadCount = 0;
        wiegandMode = WIEGAND_NORMAL_MODE_LABEL;
      }
    } else if (wiegandMode == WIEGAND_AUTO_SAVE_MODE_LABEL) {
      uint8_t addDefault_user_result = add_defaultUser_wiegand(wiegandResult);

      if (addDefault_user_result == 1) {
        if (xTimerIsTimerActive(xTimer_autoadd_wiegand1) == pdFALSE) {
          gattsIF_wiegand_autoSave = 0;
          connID_wiegand_autoSave = 0;
          handle_table_wiegand_autoSave = 0;
          BLE_SMS_Indication_wiegand_autoSave = 0;
          wiegandMode = WIEGAND_NORMAL_MODE_LABEL;
          // TODO: ENVIAR ERRO QUE NÃO IRA GRAVAR MAIS
        } else {
          if (xTimerChangePeriod(xTimer_autoadd_wiegand1, pdMS_TO_TICKS(10000),
                                 0) != pdPASS) {
            xTimerStop(xTimer_autoadd_wiegand1, 0);

            wiegandMode = WIEGAND_NORMAL_MODE_LABEL;
            gattsIF_wiegand_autoSave = 0;
            connID_wiegand_autoSave = 0;
            handle_table_wiegand_autoSave = 0;
            BLE_SMS_Indication_wiegand_autoSave = 0;

            // xTimerDelete(xTimer_autoadd_wiegand1, 0);

            // TODO: ENVIAR ERRO QUE NÃO IRA GRAVAR MAIS
          } else {
          }
        }
      } else if (addDefault_user_result == ERROR_NEW_USER_ALREADY_EXISTS) {
        // //printf("\n\nWIEGAND JA EXISTE - %lld\n\n", wiegandResult);
      } else {
        // //printf("\n\nWIEGAND ERROR - %lld\n\n", wiegandResult);
      }
    } else if (wiegandMode == WIEGAND_NORMAL_MODE_LABEL) {
      wiegand1_action(wiegandResult);
      // wiegand_parse_getData(wiegandResult, NULL, WIEGAND_NORMAL_MODE_LABEL,
      // 1);
    } else if (wiegandMode == WIEGAND_READ_MODE_LABEL) {
      char *wiegandNumber;

      char auxWiegand_str[200] = {};

      asprintf(&wiegandNumber, "WI G W %lld", wiegandResult);
      esp_ble_gatts_send_indicate(
          gattsIF_wiegand_autoSave, connID_wiegand_autoSave,
          handle_table_wiegand_autoSave, strlen(wiegandNumber),
          (uint8_t *)wiegandNumber, false);

      gattsIF_wiegand_autoSave = 0;
      connID_wiegand_autoSave = 0;
      handle_table_wiegand_autoSave = 0;
      BLE_SMS_Indication_wiegand_autoSave = 0;

      if (xTimerChangePeriod(xTimer_autoadd_wiegand1, pdMS_TO_TICKS(10000),
                             0) == pdPASS) {
        xTimerStop(xTimer_autoadd_wiegand1, 0);
      }

      send_UDP_Send(wiegandNumber, "");

      wiegandMode = WIEGAND_NORMAL_MODE_LABEL;
    }
  }
}

uint8_t antipassback_activate(char *payload) {
  anti_passback_activation = 1;
  uint8_t strIndex = 0;
  uint8_t aux = 0;
  uint8_t ACK = 0;
  char peopleCounter[5] = {};
  ESP_LOGI(TAG, "antipassback_activate payload %s", payload);
  for (int i = 0; i < strlen(payload); i++) {
    if (payload[i] == '.') {
      aux++;
      strIndex = 0;
    } else {
      if (aux == 0) {
        wiegand_antipassback_mode = payload[i] - '0';
        ESP_LOGI(TAG, "antipassback_activate wiegand_antipassback_mode %c",
                 wiegand_antipassback_mode);
      } else if (aux == 1) {
        peopleCounter[strIndex] = payload[i];
        strIndex++;
        ESP_LOGI(TAG, "antipassback_activate peopleCounter %s", peopleCounter);
      } else {
        return ESP_FAIL + 1;
      }
    }
  }

  antipassback_peopleCounter = atoi(peopleCounter);

  if (antipassback_peopleCounter > 2000 &&
      wiegand_antipassback_mode - '0' > 2) {
    return ESP_FAIL + 1;
  }

  ESP_LOGI(TAG, "antipassback_activate anti_pass_back_peopleCounter %d",
           antipassback_peopleCounter);

  if (nvs_set_u32(nvs_System_handle, NVS_ANTIPASSBACK_PEOPLE_COUNTER,
                  antipassback_peopleCounter) == ESP_OK) {
    ESP_LOGI(TAG, "antipassback_activate NVS_ANTIPASSBACK_PEOPLE_NUMBER OK");
    if (nvs_set_u8(nvs_System_handle, NVS_ANTIPASSBACK_ACTIVATE_LABEL, 1) ==
        ESP_OK) {
      anti_passback_activation = 1;
      ESP_LOGI(TAG, "antipassback_activate NVS_ANTIPASSBACK_ACTIVATE_LABEL OK");
      if (nvs_set_u8(nvs_System_handle, NVS_ANTIPASSBACK_MODE_LABEL,
                     wiegand_antipassback_mode) == ESP_OK) {
        ESP_LOGI(TAG, "antipassback_activate NVS_ANTIPASSBACK_MODE_LABEL OK");
        return ESP_OK + 1;
      } else {
        anti_passback_activation = 0;
        ESP_LOGE(TAG, "antipassback_activate NVS_ANTIPASSBACK_MODE_LABEL FAIL");
        return ESP_FAIL + 1;
      }
    } else {
      anti_passback_activation = 0;
      ESP_LOGE(TAG,
               "antipassback_activate NVS_ANTIPASSBACK_ACTIVATE_LABEL FAIL");
      return ESP_FAIL + 1;
    }
  } else {
    anti_passback_activation = 0;
    ESP_LOGE(TAG, "antipassback_activate NVS_ANTIPASSBACK_PEOPLE_NUMBER FAIL");
    return ESP_FAIL + 1;
  }

  return ESP_FAIL + 1;
}
uint8_t antipassback_deactivate() {
  anti_passback_activation = 0;
  return save_INT8_Data_In_Storage(NVS_ANTIPASSBACK_ACTIVATE_LABEL, 0,
                                   nvs_System_handle) +
         1;
}
uint8_t wiegand_parse_getData(uint64_t wiegand_data, char *keypadValue,
                              uint8_t mode, uint8_t wiegand_relay,uint8_t readerNumber) {

  MyUser validateData_user;

  memset(&validateData_user, 0, sizeof(validateData_user));

  char *wiegandData_str;
  char wiegandDataNumber[20] = {};
  char wiegandDataUser[200] = {};

  if (mode == WIEGAND_KEYPAD_MODE_LABEL) {
    asprintf(&wiegandData_str, "$%s", keypadValue);
  } else {
    asprintf(&wiegandData_str, "$%lld", wiegand_data);
  }

    mqtt_information mqttInfo;
  memset(&mqttInfo, 0, sizeof(mqttInfo));

  if (readerNumber == 1) {
    sprintf(mqttInfo.data, "%s", "READER 1");
  }
  else {
    sprintf(mqttInfo.data, "%s", "READER 2");
  }

   // vsnprintf(, const char *restrict format, ...)
  // //printf("\n\nwiegand Number ph1 %s\n\n", wiegandData_str);

  if (checkIf_wiegandExist(wiegandData_str, &wiegandDataNumber)) {
    // printf("\n\nwiegand Number ph2 %s\n\n", wiegandDataNumber);

    if (MyUser_Search_User(wiegandDataNumber, wiegandDataUser) == ESP_OK) {

      parse_ValidateData_User(wiegandDataUser, &validateData_user);
      free(wiegandData_str);
      wiegandData_str = parse_ReleData(
          WIEGAND_INDICATION, wiegand_relay, 'S', 'R', validateData_user.key,
          NULL, &validateData_user, NULL, NULL, NULL, NULL, &mqttInfo);
    }
  } else {
    // //printf("\n\nWIEGAND NOT EXIST\n\n");
  }
  free(wiegandData_str);

  return 0;
}

void timer_autoAdd_Callback(TimerHandle_t xTimer) {
  xTimerStopFromISR(xTimer, NULL);

  wiegandMode = WIEGAND_NORMAL_MODE_LABEL;
  gattsIF_wiegand_autoSave = 0;
  connID_wiegand_autoSave = 0;
  handle_table_wiegand_autoSave = 0;
  BLE_SMS_Indication_wiegand_autoSave = 0;

  keypadCount = 0;
  // TODO:
  // IMPLEMENTAR ENVIO QUE PAROU A GRAVAÇÃO AUTOMATICA OU O MODO READ
  // printf("\n\n parou gravação\n\n");
}

void timer_autoAdd2_Callback(TimerHandle_t xTimer) {
  xTimerStopFromISR(xTimer, NULL);

  wiegandMode = WIEGAND_NORMAL_MODE_LABEL;
  gattsIF_wiegand_autoSave = 0;
  connID_wiegand_autoSave = 0;
  handle_table_wiegand_autoSave = 0;
  BLE_SMS_Indication_wiegand_autoSave = 0;

  keypadCount = 0;
  // TODO:
  // IMPLEMENTAR ENVIO QUE PAROU A GRAVAÇÃO AUTOMATICA OU O MODO READ
  // printf("\n\n parou gravação\n\n");
}

int countBits(int n) {
  int count = 0;
  while (n) {
    count++;
    n >>= 1;
  }
  return count;
}

void wiegandToFacilityCard(int wiegandDecimal, int wiegandBits,
                           int *facilityCode, int *cardCode) {
  /*
   * This code is responsible for decoding a Wiegand decimal value into its
   * corresponding facility code and card code components based on the number
   * of bits specified.
   *
   * The Wiegand format is a widely used protocol for encoding access control
   * data, typically consisting of a facility code and a card code. The facility
   * code identifies the facility or site, while the card code represents the
   * unique identifier for the access card or credential.
   *
   * The code supports two common Wiegand bit lengths: 26 bits and 34 bits.
   */

  if (wiegandBits == 26) {
    /*
     * For 26-bit Wiegand values:
     * - The facility code is represented by the first 8 bits.
     * - The card code is represented by the last 16 bits.
     */
    *facilityCode = (wiegandDecimal >> 16) &
                    0xFF; // Extract the first 8 bits for the Facility Code
    *cardCode =
        wiegandDecimal & 0xFFFF; // Extract the last 16 bits for the Card Code
  } else if (wiegandBits == 34) {
    /*
     * For 34-bit Wiegand values:
     * - The facility code is represented by the first 17 bits.
     * - The card code is represented by the last 17 bits.
     */
    *facilityCode = (wiegandDecimal >> 17) &
                    0x1FFFF; // Extract the first 17 bits for the Facility Code
    *cardCode =
        wiegandDecimal & 0x1FFFF; // Extract the last 17 bits for the Card Code
  } else {
    /*
     * If the number of Wiegand bits is neither 26 nor 34, it is considered
     * an invalid input. In this case, the facility code and card code are
     * set to -1 as an error indicator.
     */
    // //printf("Erro: O número de bits de Wiegand deve ser 26 ou 34.\n");
    *facilityCode = -1; // Indicador de erro
    *cardCode = -1;     // Indicador de erro
  }
}

uint8_t activate_wiegand(uint8_t mode) {
  if (save_INT8_Data_In_Storage(NVS_AL_CONF_AL, mode, nvs_System_handle) ==
      ESP_OK) {
    // fd_configurations.alarmMode.A = mode;
    return 1;
  } else {
    return 0;
  }
  return 0;
}

uint8_t deactivate_wiegand() { return 1; /* alarm_Turn_OFF(); */ }

char *parseWiegand_data(uint8_t BLE_SMS_Indication, uint8_t gattsIF,
                        uint16_t connID, uint16_t handle_table, char cmd,
                        char param, char *phPassword, char *payload,
                        MyUser *user_validateData, mqtt_information *mqttInfo) {

  if (BLE_SMS_Indication == BLE_INDICATION) {

    BLE_SMS_Indication_wiegand_autoSave = BLE_SMS_Indication;
    gattsIF_wiegand_autoSave = gattsIF;
    connID_wiegand_autoSave = connID;
    handle_table_wiegand_autoSave = handle_table;
  }

  if (cmd == SET_CMD) {
    if (param == WIEGAND_START_AUTO_SAVE_PARAMETER) {
      char *rsp;

      if (xTimer_autoadd_wiegand1 != NULL) {
        if (xTimerIsTimerActive(xTimer_autoadd_wiegand1) == pdFALSE) {
          if (xTimerStart(xTimer_autoadd_wiegand1, 0) == pdPASS) {
            wiegandMode = WIEGAND_AUTO_SAVE_MODE_LABEL;
            // //printf("\n\n wiegand mode payload - %s\n\n", payload);
            parse_ValidateData_User(payload, &user_auto_controlAcess);
            sprintf(user_auto_controlAcess.key, "%s", "888888");
            asprintf(&rsp, "%s", "WI S S OK");
          } else {
            asprintf(&rsp, "%s", "WI S S ERROR");
            // xTimerDelete(xTimer_autoadd_wiegand1, 0);
          }
        } else {
          xTimerChangePeriod(xTimer_autoadd_wiegand1, pdMS_TO_TICKS(15000), 0);
          // Inicie o timer

          // Após terminar de usar o timer, exclua-o
        }
      } else {
        asprintf(&rsp, "%s", "WI S S ERROR");
      }

      return rsp;
    } else if (param == ACTIVATE_ANTIPASSBACK_PARAMETER) {
      char *rsp;
      if (antipassback_activate(payload)) {
         asprintf(&rsp, "WI S A %s",payload );
      }
      else {
        asprintf(&rsp, "WI S A %s","ERROR");
      }
     
      return rsp;
    } else if (param == WIEGANG_TURN_ON_OFF_PARAMETER) {
      char *rsp;
      if (save_INT8_Data_In_Storage(NVS_AL_CONF_AL, atoi(payload),
                                    nvs_System_handle) == ESP_OK) {

        wiegand_reader_enable(&reader);
        // fd_configurations.alarmMode.A = atoi(payload);
        asprintf(&rsp, "WI S C %d", atoi(payload));
      } else {
        asprintf(&rsp, "WI S C %d", 4);
      }

      return rsp;
    } else if (param == WIEGANG_PHONE_NUMBER_PARAMETER) {
      char wiegand_number[20] = {};
      char phone_number[20] = {};
      char *rsp;
      parse_put_phoneNumber_to_wiegand(payload, wiegand_number, phone_number);
      asprintf(&rsp, "WI S P %s",
               put_phoneNumber_to_wiegand(wiegand_number, phone_number));
      return rsp;
    } else if (param == WIEGANG_NUMBER_PARAMETER) {
      char wiegand_number[20] = {};
      char phone_number[20] = {};
      char relay_wiegandPermition = 0;
      char *rsp;

      parse_put_wiegand_to_phoneNumber(payload, wiegand_number, phone_number,
                                       &relay_wiegandPermition);
      asprintf(&rsp, "WI S W %s",
               put_wiegand_to_phoneNumber(wiegand_number, phone_number,
                                          &relay_wiegandPermition));
      return rsp;
    } else if (param == WIEGAND_CHANGE_RELAY_PARAMETER) {
      char wiegandNumber[20] = {};
      char wiegand_relayPermition = 0;
      char *rsp;

      parse_edit_wiegand_relay(payload, wiegandNumber, &wiegand_relayPermition);
      // printf("\n\n WIEGAND RELAY - %s \n\n", rsp);
      asprintf(
          &rsp, "WI S R %s",
          edit_wiegand_relay(wiegandNumber,
                             wiegand_relayPermition)); /* "%s", "NTRSP"); */
      // printf("\n\n WIEGAND RELAY11 - %s \n\n", rsp);
      // printf("\n\n WIEGAND RELAY222 - %s \n\n", rsp);

      // sprintf(mqttInfo->data, "WI S R %s",rsp);
      // send_UDP_queue(mqttInfo);
      // send_UDP_Package(mqttInfo->data,strlen(mqttInfo->data),mqttInfo->topic);
      // send_UDP_Send(rsp,mqttInfo->topic);

      // printf("\n\n WIEGAND RELAY 555- %s \n\n", rsp);
      return rsp;
    }
  } else if (cmd == GET_CMD) {
    if (param == WIEGANG_NUMBER_PARAMETER) {
      if (xTimer_autoadd_wiegand1 != NULL) {
        // //printf("\n\n gw1\n\n");
        if (xTimerIsTimerActive(xTimer_autoadd_wiegand1) == pdFALSE) {
          // //printf("\n\n gw2\n\n");
          if (xTimerStart(xTimer_autoadd_wiegand1, 0) == pdPASS) {
            // //printf("\n\n gw3\n\n");
            wiegandMode = WIEGAND_READ_MODE_LABEL;
          } else {
            // //printf("\n\n gw4\n\n");
            //  asprintf(&rsp, "%s", "WI G W ERROR");
            //   xTimerDelete(xTimer_autoadd_wiegand1, 0);
          }
        } else {
          // //printf("\n\n gw5\n\n");
          xTimerChangePeriod(xTimer_autoadd_wiegand1, pdMS_TO_TICKS(15000), 0);
          // Inicie o timer

          // Após terminar de usar o timer, exclua-o
        }
        // //printf("\n\n gw6\n\n");
        char *rsp;
        asprintf(&rsp, "%s", "NTRSP");
        return rsp;
      }
    }
  } else if (cmd == RESET_CMD) {
    if (param == WIEGANG_NUMBER_PARAMETER) {
      char *rsp;
      asprintf(&rsp, "%s", erase_wiegand_number(payload));
      return rsp;
    } else if (param == ACTIVATE_ANTIPASSBACK_PARAMETER) {
      char *rsp;
      asprintf(&rsp, "WI R A %d", antipassback_deactivate());
      return rsp;
    } else if (param == WIEGANG_TURN_ON_OFF_PARAMETER) {
      char *rsp;

      if (save_INT8_Data_In_Storage(NVS_AL_CONF_AL, 0, nvs_System_handle) ==
          ESP_OK) {
        // fd_configurations.alarmMode.A = 0;
        wiegand_reader_disable(&reader);
      }

      asprintf(&rsp, "WI R C %d", 0);
      return rsp;
    } else if (param == WIEGAND_START_AUTO_SAVE_PARAMETER) {
      char *rsp;
      if (xTimerChangePeriod(xTimer_autoadd_wiegand1, pdMS_TO_TICKS(10000),
                             0) == pdPASS) {
        xTimerStop(xTimer_autoadd_wiegand1, 0);
        asprintf(&rsp, "%s", "WI R S OK");
      } else {
        asprintf(&rsp, "%s", "WI R S ERROR");
      }

      wiegandMode = WIEGAND_NORMAL_MODE_LABEL;
      gattsIF_wiegand_autoSave = 0;
      connID_wiegand_autoSave = 0;
      handle_table_wiegand_autoSave = 0;
      BLE_SMS_Indication_wiegand_autoSave = 0;

      return rsp;
    }
  } else {
    char *rsp;
    asprintf(&rsp, "%s", "ERROR");
    return rsp;
  }

  char *rsp;
  asprintf(&rsp, "%s", "ERROR");
  return rsp;
}

void parse_edit_wiegand_relay(char *payload, char *wiegandNumber,
                              char *permition) {
  uint8_t count = 0;
  uint8_t index = 0;

  for (size_t i = 0; i < strlen(payload); i++) {
    if (payload[i] == '.') {
      index = 0;
      count++;
    } else {
      if (count == 0) {
        *(wiegandNumber + index) = payload[i];
        index++;
      } else if (count == 1) {
        *permition = payload[i];
        break;
      }
    }
  }

  // //printf("\n\n wiegand - %s / phone - %s / permition %c\n\n", payload,
  // wiegandNumber, *permition);
}

char *edit_wiegand_relay(char *wiegandNumber, char relay_wiegandPermition) {
  char wiegandContent[20] = {};
  char aux_wiegandContent[20] = {};
  char userContent[200] = {};

  MyUser wiegand_myUser_data;
  memset(&wiegand_myUser_data, 0, sizeof(wiegand_myUser_data));

  if (checkIf_wiegandExist(wiegandNumber, wiegandContent)) {
    // printf("\n\nwiegand cont %s \n\n", wiegandContent);
    if (MyUser_Search_User(wiegandContent, userContent) == ESP_OK) {
      // printf("\n\nwiegand user cont %s - %c \n\n", userContent,
      // relay_wiegandPermition);
      parse_ValidateData_User(userContent, &wiegand_myUser_data);

      wiegand_myUser_data.wiegand_rele_permition = relay_wiegandPermition;
      // printf("\n\nwiegand user cont 33 %s - %c \n\n", userContent,
      // relay_wiegandPermition);

      if (replaceUser(&wiegand_myUser_data) == ESP_OK) {
        // printf("\n\nwiegand user cont 44 %s - %c \n\n", wiegandNumber,
        // relay_wiegandPermition);
        memset(rsp_pointer, 0, sizeof(rsp_pointer));
        // printf("\n\nwiegand user cont 55 %s - %c \n\n", wiegandNumber,
        // relay_wiegandPermition);
        sprintf(rsp_pointer, "%s.%c", wiegandNumber, relay_wiegandPermition);
        // printf("\n\nwiegand user cont 66 %s - %c \n\n", wiegandNumber,
        // relay_wiegandPermition);
        return rsp_pointer;
      } else {
        return "ERROR";
      }
    } else {
      return "ERROR MEM";
    }
  }

  return "WIEGAND NOT EXIST";
}

char *erase_wiegand_number(char *payload) {
  char file_contents_Users[200] = {};
  char aux_wiegandNumber[30] = {};

  MyUser wiegand_myUser_data;
  memset(&wiegand_myUser_data, 0, sizeof(wiegand_myUser_data));

  memset(rsp_pointer, 0, sizeof(rsp_pointer));

  printf("\n\nerase_wiegand_number - %s \n\n", payload);

  if (MyUser_Search_User(payload, file_contents_Users) == ESP_OK) {
    printf("\n\nfile_contents_Users - %s \n\n", file_contents_Users);
    parse_ValidateData_User(file_contents_Users, &wiegand_myUser_data);
    sprintf(aux_wiegandNumber, "$%s", wiegand_myUser_data.wiegand_code);

    printf("\n\naux_wiegandNumber - %s \n\n", aux_wiegandNumber);

    printf("\n\nwiegand_myUser_data.permition - %c \n\n",
           wiegand_myUser_data.permition);
    char aux_wiegandPermission = wiegand_myUser_data.permition;
    if (erase_only_wiegand(aux_wiegandNumber, aux_wiegandPermission) ==
        ESP_OK) {
      if (Myuser_deleteUser(&wiegand_myUser_data) == ESP_OK) {
        for (size_t i = 0; i < 20; i++) {
          wiegand_myUser_data.wiegand_code[i] = 0;
        }

        wiegand_myUser_data.wiegand_code[0] = ':';
        wiegand_myUser_data.wiegand_rele_permition = ':';

        if (MyUser_Add(&wiegand_myUser_data) == ESP_OK) {
          sprintf(rsp_pointer, "WI R W %s", aux_wiegandNumber);
          printf("\n\nrsp_pointer - %s \n\n", rsp_pointer);
          return rsp_pointer;
        }
      } else {
        MyUser_add_wiegand(aux_wiegandNumber, payload, aux_wiegandPermission);
        return "WI R W ERROR";
      }
      printf("\n\nwiegand_myUser_data.permition 22- %c \n\n",
             wiegand_myUser_data.permition);
      // wiegand_myUser_data.permition = 0;
      /*  if (replaceUser(&wiegand_myUser_data) == ESP_OK) {

         printf("\n\nwiegand_myUser_data.permition 33- %c \n\n",
                wiegand_myUser_data.permition);
         sprintf(rsp_pointer, "WI R W %s", aux_wiegandNumber);
         printf("\n\nrsp_pointer - %s \n\n", rsp_pointer);
         return rsp_pointer;
       } else {
         printf("\n\nwiegand_myUser_data.permition 332- %c \n\n",
                wiegand_myUser_data.permition);
         return "WI R W ERROR";
       } */
    } else {

      return "WI R W WIEGAND NOT EXIST";
    }
  }

  return "WI R W USER DONT EXIST";
}

uint8_t erase_only_wiegand(char *wiegandNumber, char permition) {

  printf("\n\nerase_only_wiegand - %s - %c", wiegandNumber, permition);
  if (permition == '0') {

    if (nvs_erase_key(nvs_wiegand_codes_users_handle, wiegandNumber) ==
        ESP_OK) {

      return ESP_OK;
    } else {
      return ESP_FAIL;
    }
  } else if (permition == '1') {

    if (nvs_erase_key(nvs_wiegand_codes_admin_handle, wiegandNumber) ==
        ESP_OK) {
      return ESP_OK;
    } else {
      return ESP_FAIL;
    }
  } else if (permition == '2') {

    if (nvs_erase_key(nvs_wiegand_codes_owner_handle, wiegandNumber) ==
        ESP_OK) {

      return ESP_OK;
    } else {
      return ESP_FAIL;
    }
  } else {
    return ESP_FAIL;
  }

  return ESP_FAIL;
}

void parse_put_phoneNumber_to_wiegand(char *payload, char *wiegand,
                                      char *phoneNumber) {
  uint8_t count = 0;
  uint8_t index = 0;

  for (size_t i = 0; i < strlen(payload); i++) {
    if (payload[i] == '.') {
      index = 0;
      count++;
    } else {
      if (count == 0) {
        *(wiegand + index) = payload[i];
        index++;
      } else if (count == 1) {
        *(phoneNumber + index) = payload[i];
        index++;
      }
    }
  }

  // //printf("\n\n wiegand - %s / phone - %s\n\n", wiegand, phoneNumber);
}

char *put_phoneNumber_to_wiegand(char *wiegandNumber, char *phoneNumber) {
  char payload[200] = {};
  char aux_payload[200] = {};

  MyUser wiegand_myUser_data;
  memset(&wiegand_myUser_data, 0, sizeof(wiegand_myUser_data));

  if (MyUser_Search_User(phoneNumber, payload) != ESP_OK) {
    memset(payload, 0, sizeof(payload));
    char wiegandKey[20] = {};
    sprintf(wiegandKey, "$%s", wiegandNumber);

    if (checkIf_wiegandExist(wiegandKey, payload)) {
      // //printf("\n\n 333wiegand -  / phone - %s\n\n", payload);
      if (MyUser_Search_User(payload, aux_payload) == ESP_OK) {

        parse_ValidateData_User(aux_payload, &wiegand_myUser_data);
        // //printf("\n\n 444wiegand -  / phone - %s\n\n", aux_payload);

        if (Myuser_deleteUser(&wiegand_myUser_data) == ESP_OK) {
          sprintf(wiegand_myUser_data.phone, "%s", phoneNumber);
          // //printf("\n\n 555wiegand -  / phone - %s\n\n",
          // wiegand_myUser_data.phone);
          if (MyUser_Add(&wiegand_myUser_data) == ESP_OK) {
            memset(rsp_pointer, 0, sizeof(rsp_pointer));
            memset(aux_payload, 0, sizeof(aux_payload));
            // //printf("\n\n 6666wiegand -  / phone - %s\n\n",
            // wiegand_myUser_data.phone);
            if (MyUser_Search_User(wiegand_myUser_data.phone, aux_payload) ==
                ESP_OK) {

              erase_Password_For_Rsp(aux_payload, rsp_pointer);
              return rsp_pointer;
            }
          } else {
            return "ERROR CHANGE USER";
          }
        } else {
          return "ERROR CHANGE USER";
        }
      }
    } else {
      return "ERROR WI NOT EXIST";
    }
  } else {
    return "ERROR PH NUMBER EXIST";
  }

  return "ERROR";
}

void parse_put_wiegand_to_phoneNumber(char *payload, char *wiegandNumber,
                                      char *phoneNumber,
                                      char *relay_wiegandPermition) {
  uint8_t count = 0;
  uint8_t index = 0;

  for (size_t i = 0; i < strlen(payload); i++) {
    if (payload[i] == '.') {
      index = 0;
      count++;
    } else {
      if (count == 0) {
        *(wiegandNumber + index) = payload[i];
        index++;
      } else if (count == 1) {
        *(phoneNumber + index) = payload[i];
        index++;
      } else if (count == 2) {
        *relay_wiegandPermition = payload[i];
      }
    }
  }

  // //printf("\n\n wiegand - %s / phone - %s / permition %c\n\n",
  // wiegandNumber, phoneNumber, *relay_wiegandPermition);
}

char *put_wiegand_to_phoneNumber(char *wiegandNumber, char *phoneNumber,
                                 char *relay_wieganPermition) {

  char payload[200] = {};
  char aux_payload[200] = {};
  MyUser wiegand_myUser_data;
  memset(&wiegand_myUser_data, 0, sizeof(wiegand_myUser_data));
  char wiegandKey[20] = {};
  sprintf(wiegandKey, "$%s", wiegandNumber);
  if (MyUser_Search_User(phoneNumber, payload) == ESP_OK) {
    parse_ValidateData_User(payload, &wiegand_myUser_data);
    printf("\n\nSearch User Success - %s\n\n", phoneNumber);
    if (!checkIf_wiegandExist(wiegandKey, aux_payload)) {
      printf("\n\nWiegand dont exist - %s\n\n", wiegandKey);
      char insertPayload[50] = {};

      if (Myuser_deleteUser(&wiegand_myUser_data) == ESP_OK) {
        sprintf(wiegand_myUser_data.wiegand_code, "%s", wiegandNumber);
        wiegand_myUser_data.wiegand_rele_permition = *relay_wieganPermition;

        if (MyUser_Add(&wiegand_myUser_data) == ESP_OK) {
          memset(aux_payload, 0,
                 sizeof(aux_payload)); // memset(payload, int c, size_t n)

          if (strlen(wiegand_myUser_data.phone) < 1) {
            if (wiegand_myUser_data.wiegand_code[0] == ':' &&
                wiegand_myUser_data.rf_serial[0] != ':') {

              sprintf(phoneNumber, "$%s", wiegand_myUser_data.rf_serial);

            } else {
              sprintf(phoneNumber, "$%s", wiegand_myUser_data.wiegand_code);
            }
          }

          if (MyUser_Search_User(phoneNumber, aux_payload) == ESP_OK) {

            erase_Password_For_Rsp(aux_payload, rsp_pointer);
            return rsp_pointer;
          }
        }
      } else {
        return "ERROR";
      }

      char newSTR_payload[50] = {};

      // sprintf(newSTR_payload, "%c;%s;", *relay_wieganPermition,
      // wiegandNumber); strcat(payload, newSTR_payload);

      // parse_ValidateData_User(payload, &wiegand_myUser_data);
      /*
            printf("\n\n WIGAND EXIST 233 - %s - %c\n\n",
                   wiegand_myUser_data.wiegand_code,
                   wiegand_myUser_data.relayPermition);

            replaceUser(&wiegand_myUser_data);

            if (MyUser_add_wiegand(wiegandKey, phoneNumber,
                                   wiegand_myUser_data.permition) != ESP_OK) {
              // //printf("\n\n WIGAND EXIST 555\n\n");
              return "ERROR ADD";
            } else {
              memset(rsp_pointer, 0, sizeof(rsp_pointer));
              memset(aux_payload, 0, sizeof(aux_payload));
            } */
    } else {
      return "ERROR WIEGAND EXIST";
    }
  } else {
    return "ERROR USER DONT EXIST";
  }

  return "ERROR";
} // *phoneNumber,char  char permition)
// {
//     char payload[200] = {};

//     if (permition == '0')
//     {
//         if (save_STR_Data_In_Storage(wiegandNumber, phoneNumber,
//         nvs_wiegand_codes_users_handle) == ESP_OK)
//         {
//             if (get_STR_Data_In_Storage(wiegandNumber, nvs_Users_handle,
//             &payload) == ESP_OK)
//             {
//                 if (save_STR_Data_In_Storage(phoneNumber, payload,
//                 nvs_Users_handle) == ESP_OK)
//                 {
//                     if (nvs_erase_key(nvs_Users_handle, wiegandNumber) ==
//                     ESP_OK)
//                     {
//                         return 1;
//                     }
//                 }
//                 else
//                 {
//                     return 0;
//                 }
//             }
//             else
//             {
//                 return 0;
//             }
//         }
//         return 0;
//     }
//     else if (permition == '1')
//     {
//         if (save_STR_Data_In_Storage(wiegandNumber, phoneNumber,
//         nvs_wiegand_codes_admin_handle) == ESP_OK)
//         {
//             if (get_STR_Data_In_Storage(wiegandNumber, nvs_Admin_handle,
//             &payload == ESP_OK))
//             {
//                 if (save_STR_Data_In_Storage(phoneNumber, payload,
//                 nvs_Admin_handle) == ESP_OK)
//                 {
//                     if (nvs_erase_key(nvs_Admin_handle, wiegandNumber) ==
//                     ESP_OK)
//                     {
//                         return 1;
//                     }
//                     else
//                     {
//                         return 0;
//                     }
//                 }
//                 else
//                 {
//                     return 0;
//                 }
//             }
//             else
//             {
//                 return 0;
//             }
//         }
//     }

//     return 0;
// }