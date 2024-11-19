/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|

*/

#include "gpio.h"
#include "EG91.h"
#include "ble_spp_server_demo.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "core.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "pcf85063.h"
#include "rele.h"
#include "routines.h"
#include "system.h"
#include "users.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int debounce_Time_Input0;
int debounce_Time_Input1;
int debounce_Time_Input2;
int debounce_Time_Input3;
int debounce_Time_Input_RTC_ALARM;
int debounce_Time_Input_SIMPRE;

uint8_t Input0_state = 0;
uint8_t Input1_state = 0;
uint8_t Input_RTC_Alarm_state = 0;
int SDCard_Value = 0;

int last_Input_SIMPRE;
int last_Input_SDCARD;

extern SemaphoreHandle_t rdySem_Feedback_Call;

static QueueHandle_t gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void *arg) {
  uint32_t gpio_num = (uint32_t)arg;
  // ////printf("GPIO[%d]\n", gpio_num);
  xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task_example(void *arg) {
  uint32_t io_num;
  // ////printf("gpio_task_example\n");

  last_Input_SIMPRE = gpio_get_level(GPIO_INPUT_IO_SIMPRE);

  for (;;) {
    if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
      // ////printf("GPIO[%d] \n", io_num);
      Input_GPIO_Debounce(io_num);
    }
  }
}

uint8_t Input_GPIO_Debounce(uint32_t io_num) {
  char input_Data_IO[50] = {};
  uint64_t now = esp_timer_get_time();

  uint64_t diff = 0;

  uint8_t input_nowState;

  switch (io_num) {
    ////////printf("GPIO[%d] intr, val: %d\n", io_num, input_nowState);
  case CONFIG_GPIO_INPUT_0:

    break;

  case CONFIG_GPIO_INPUT_1:

    break;

  case GPIO_INPUT_IO_EG91_STATUS:

    // ////printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
    if (((now - debounce_Time_Input2) / 1000) > INPUT_GPIO_TIME_DEBOUNCE) {
      debounce_Time_Input2 = now;
      // ////printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
    }

    break;

  case GPIO_INPUT_IO_CD_SDCARD:

    SDCard_Value = gpio_get_level(GPIO_INPUT_IO_CD_SDCARD);

    if (((now - debounce_Time_Input3) / 1000) > INPUT_GPIO_TIME_DEBOUNCE) {

      // ////printf("\n\nGPIO_INPUT_IO_CD_SDCARD\n\n");

      last_Input_SDCARD = SDCard_Value;
    }

    break;

  case GPIO_INPUT_RTC_ALARM:

    // ////printf("fafasfasf GPIO[%d] intr, val: %d\n", GPIO_INPUT_RTC_ALARM,
    // gpio_get_level(GPIO_INPUT_RTC_ALARM));
    if (((now - debounce_Time_Input_RTC_ALARM) / 1000) >
        INPUT_GPIO_TIME_DEBOUNCE) {
      debounce_Time_Input_RTC_ALARM = now;
      // ////printf("GPIO[%d] intr, val: %d\n", GPIO_INPUT_RTC_ALARM,
      // gpio_get_level(GPIO_INPUT_RTC_ALARM));
      if (!gpio_get_level(GPIO_INPUT_RTC_ALARM)) {

        // ////printf("\n\nGPIO_INPUT_RTC_ALARM 11\n\n");
        xSemaphoreGive(rdySem_Control_refreshSystemTime_task);
        // uint8_t data[7];
        // PCF85_Readnow(data);
        //  //////printf("\nRESET ALARM\n");
        //  EG91_send_AT_Command("AT+QLTS=1", "QLTS", 3000);

        // EG91_send_AT_Command("AT+CCLK?", "OK", 1000);

        // time = data[0] + 3;
        /* if (time > 59)
        {
            time = time - 60;
        } */

        // enableAlarm();  // setAlarm(time, 99, 99, 99, 99);
      }
      // ////printf("\n\nGPIO_INPUT_RTC_ALARM 22\n\n");
      break;
    }

  case GPIO_INPUT_IO_SIMPRE:

    if (((now - debounce_Time_Input_SIMPRE) / 1000) >
        INPUT_GPIO_TIME_DEBOUNCE) {
      if (label_initSystem_SIMPRE == 1) {
        // system_stack_high_water_mark("simpre0");
        // printf("\n\nGPIO_INPUT_IO_SIMPRE = %d\n\n",
        // gpio_get_level(GPIO_INPUT_IO_EG91_STATUS));
        debounce_Time_Input_SIMPRE = now;
        if (gpio_get_level(GPIO_INPUT_IO_SIMPRE) != last_Input_SIMPRE) {
          last_Input_SIMPRE = gpio_get_level(GPIO_INPUT_IO_SIMPRE);

          // ////printf("\n\n ENTER SIM CARD INSERT123\n\n");
          if (gpio_get_level(GPIO_INPUT_IO_SIMPRE)) {
            timer_pause(TIMER_GROUP_1, TIMER_0);
            ////////printf("\nsms or call %s\n", atcmd);
            // vTaskSuspend(xHandle_Timer_VerSystem);
            give_rdySem_Control_Send_AT_Command();
            update_ACT_TimerVAlue((double)RSSI_NOT_DETECT);
            gpio_set_level(GPIO_OUTPUT_ACT, 1);

            // ////printf("\n\n ENTER SIM CARD INSERT\n\n");

            if (!gpio_get_level(GPIO_INPUT_IO_EG91_STATUS)) {
              EG91_PowerOn();
            } else {
              EG91_send_AT_Command("AT+CFUN=0", "OK", 10000);
              EG91_send_AT_Command("AT+CFUN=1", "OK", 10000);
            }

            vTaskDelay(pdMS_TO_TICKS(10000));

            if (!EG91_initNetwork()) {
              update_ACT_TimerVAlue((double)RSSI_NOT_DETECT);
            }

            EG91_send_AT_Command("AT+CSDH=1", "OK", 1000);

            EG91_send_AT_Command("AT+QCFG=\"urc/ri/other\",\"off\"", "OK",
                                 1000);

            // vTaskResume(xHandle_Timer_VerSystem);
            timer_start(TIMER_GROUP_1, TIMER_0);
          } else {
            RSSI_LED_TOOGLE = RSSI_NOT_DETECT;
            update_ACT_TimerVAlue((double)RSSI_NOT_DETECT);
            save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL, 0, nvs_System_handle);
            // ////printf("\n\n ENTER SIM CARD INSERT4444\n\n");

            last_Input_SIMPRE = gpio_get_level(GPIO_INPUT_IO_SIMPRE);
            EG91_Power_OFF();
          }
        }
      }
      /*  else
       {
           label_initSystem_SIMPRE = 1;
       } */
      // diff = (now - debounce_Time_Input_RTC_ALARM) / 1000;
      // input_nowState = gpio_get_level(io_num);

      // if (Input_RTC_Alarm_state != input_nowState)
      // {

      //     if (!gpio_get_level(io_num))
      //     {
      //         uint8_t data[7];
      //         PCF85_Readnow(data);
      //         //////printf("\nRESET ALARM\n");
      //         time = data[0] + 3;
      //         if (time > 59)
      //         {
      //             time = time - 60;
      //         }

      //         /* enableAlarm(); */ setAlarm(time, 99, 99, 99, 99);
      //     }
      //     if (input_nowState)
      //     {
      //         if (diff > INPUT_GPIO_TIME_DEBOUNCE)
      //         {
      //         }
      //     }
      //     else
      //     {
      //     }
      // }

      break;
    }
  default:
    break;
  }
  return 0;
}

void task_refresh_SystemTime(void *pvParameter) {
  data_EG91_Send_SMS sms_Data;
  int minutesCount = 0;
  int time_To_refresh = 0200;
  uint8_t label_Aux_TimeToRefresh = 0;
  uint32_t restartSystem_time = 0;
  uint8_t pingTimeout = 1;

  for (;;) {
    xSemaphoreTake(rdySem_Control_refreshSystemTime_task, portMAX_DELAY);
    // ////printf("\n\ntask_refresh_SystemTime 01
    // %d\n\n",label_Reset_Password_OR_System);

    // ////printf("\n\n pingTimeout aa %d\n\n",pingTimeout); //55

    /*  if (label_Reset_Password_OR_System == 2)
     {
          pingTimeout++;
         //////printf("\n\n pingTimeout ff %d\n\n",pingTimeout); //55
         if (pingTimeout == 2)
         {
            // EG91_UDP_Ping();
            // pingTimeout = 0;
         //}


     } */

    get_RTC_System_Time();
    // ////printf("\n\ntask_refresh_SystemTime 0 - %d\n\n", nowTime.time);

    if (nvs_get_u32(nvs_System_handle, NVS_KEY_RESTART_SYSTEM,
                    &restartSystem_time) != ESP_OK) {
      nvs_set_u32(nvs_System_handle, NVS_KEY_RESTART_SYSTEM, 303);
      restartSystem_time = 303;
    }

    // ////printf("\n\ntask_refresh_SystemTime 0 - %d - %d\n\n", nowTime.time,
    // restartSystem_time);

    // ////printf("\n\nR1 - %d R2 - %d I1 - %d I2 -
    // %d\n\n",gpio_get_level(GPIO_OUTPUT_IO_0),gpio_get_level(GPIO_OUTPUT_IO_1),!gpio_get_level(CONFIG_GPIO_INPUT_0),!gpio_get_level(CONFIG_GPIO_INPUT_1));

    // TODO: DESCOMENTAR SE NÃO FUNCIONAR NAS NOVAS PCBS
    if (nowTime.time == restartSystem_time) {
      if (label_Routine2_ON == 0 && label_Routine1_ON == 0 &&
          gpio_get_level(GPIO_OUTPUT_IO_0) == 0 &&
          gpio_get_level(GPIO_OUTPUT_IO_1) == 0 &&
          !gpio_get_level(CONFIG_GPIO_INPUT_0) == 0 &&
          !gpio_get_level(CONFIG_GPIO_INPUT_1) == 0) {
        esp_restart();
      } else {
        restartSystem_time = restartSystem_time + 100;

        if (restartSystem_time > 2400) {
          restartSystem_time = 3;
        }

        // ////printf("\n\nrestart system2 - %d\n\n",restartSystem_time);
        nvs_set_u32(nvs_System_handle, NVS_KEY_RESTART_SYSTEM,
                    restartSystem_time);
      }
    }

    if (nowTime.time == 201) {
      if (gpio_get_level(GPIO_INPUT_IO_SIMPRE)) {
        // ////printf("\n\ntask_refresh_SystemTime 2\n\n");
        EG91_send_AT_Command("AT+QLTS=1", "QLTS", 3000);

        /*  sprintf(sms_Data.phoneNumber, "%s", "917269448");

         sprintf(sms_Data.payload, "%s%d", "ATUALIZAÇÃO DA HORA - ",
         nowTime.time);

         xQueueSendToBack(queue_EG91_SendSMS, (void *)&sms_Data,
         pdMS_TO_TICKS(100)); label_Aux_TimeToRefresh = 0; */
      } else {
        // ////printf("\n\ntask_refresh_SystemTime 3\n\n");
        refresh_ESP_RTC();
      }

      cron_stop();
      cron_job_clear_all();
      routines_ID = 0;
      save_INT8_Data_In_Storage(NVS_ROUTINES_ID, routines_ID,
                                nvs_System_handle);
      initRoutines();
    }

    if (nowTime.time == 1) {
      uint32_t value = 0;
      erase_Users_With_LastTime(nowTime.date);
      value = MyUser_List_AllUsers();
      save_User_Counter_In_Storage(value);
      if (gpio_get_level(GPIO_INPUT_IO_SIMPRE)) {
        uint8_t label_SMS_Periodic = get_INT8_Data_From_Storage(
            NVS_KEY_LABEL_PERIODIC_SMS, nvs_System_handle);

        if (label_To_Send_Periodic_SMS == 255 ||
            label_To_Send_Periodic_SMS == 0) {
          label_To_Send_Periodic_SMS = 0;
          EG91_send_AT_Command("AT+QLTS=1", "QLTS", 3000);
        }

        // ////printf("\n\nlabel_SMS_Periodic %d\n\n", label_SMS_Periodic);

        if (label_SMS_Periodic == 1) {

          size_t required_size;
          char aux_own_Number[30] = {};
          memset(aux_own_Number, 0, sizeof(aux_own_Number));

          ////////printf("\nerror 1 %d\n", err);
          ////////printf("\nkey 1 %s\n", key);
          // //////printf("strlen key %d", strlen(key));

          if (nvs_get_str(nvs_System_handle, NVS_KEY_OWN_NUMBER, NULL,
                          &required_size) == ESP_OK) {
            // ////printf("\nrequire size %d\n", required_size);
            // ////printf("\nGET USERS NAMESPACE\n");
            if (nvs_get_str(nvs_System_handle, NVS_KEY_OWN_NUMBER,
                            aux_own_Number, &required_size) == ESP_OK) {
              // ////printf("\n\nsms_Data.phoneNumber 00\n\n");

              esp_err_t ack =
                  nvs_get_u32(nvs_System_handle, NVS_KEY_DATE_PERIODIC_SMS,
                              &date_To_Send_Periodic_SMS);

              // ////printf("\n\ndate_To_Send_Periodic_SMS %d\n\n",
              // date_To_Send_Periodic_SMS);

              if (nowTime.date >= date_To_Send_Periodic_SMS) {

                sprintf(sms_Data.phoneNumber, "%s", aux_own_Number);

                sprintf(sms_Data.payload, "%s%d", "MOTORLINE", nowTime.time);

                // ////printf("\n\nsms_Data.phoneNumber 11\n\n");

                xQueueSendToBack(queue_EG91_SendSMS, (void *)&sms_Data,
                                 pdMS_TO_TICKS(100));

                memset(aux_own_Number, 0, sizeof(aux_own_Number));

                // sprintf(sms_Data.phoneNumber, "%s", "917269448");

                // sprintf(sms_Data.payload, "%s%d", "SMS PERIODICA - ",
                // nowTime.time);

                // ////printf("\n\nsms_Data.phoneNumber 11\n\n");

                // xQueueSendToBack(queue_EG91_SendSMS, (void *)&sms_Data,
                // pdMS_TO_TICKS(100));

                label_To_Send_Periodic_SMS = 1;
                save_INT8_Data_In_Storage(NVS_KEY_LABEL_PERIODIC_SMS,
                                          label_To_Send_Periodic_SMS,
                                          nvs_System_handle);

                date_To_Send_Periodic_SMS = parseDatetoInt(nowTime.date, 60);
                nvs_set_u32(nvs_System_handle, NVS_KEY_DATE_PERIODIC_SMS,
                            date_To_Send_Periodic_SMS);
              }
            }
          }
        }
      }

      memset(&sms_Data, 0, sizeof(sms_Data));

      // ////printf("\n\ntask_refresh_SystemTime 2\n\n");

      /* sprintf(sms_Data.phoneNumber, "%s", "917269448");

      sprintf(sms_Data.payload, "%s%d", "ELIMINAR USERS - ", nowTime.time); */

      // xQueueSendToBack(queue_EG91_SendSMS, (void *)&sms_Data,
      // pdMS_TO_TICKS(100));
      label_Aux_TimeToRefresh = 0;
    }

    enableAlarm();
  }

  vTaskDelete(NULL);
}

void gpio_init() {

  // zero-initialize the config structure.
  gpio_config_t io_conf = {};

  // gpio_set_level(GPIO_OUTPUT_ACT, 1);
  memset(&io_conf, 0, sizeof(io_conf));
  // interrupt of rising edge
  io_conf.intr_type =
      GPIO_INTR_ANYEDGE; // GPIO_INTR_POSEDGE; // GPIO_INTR_ANYEDGE;
  // bit mask of the pins, use GPIO4/5 here
  io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
  // set as input mode
  io_conf.mode = GPIO_MODE_INPUT;
  // enable pull-up mode
  io_conf.pull_up_en = 0;
  io_conf.pull_down_en = 0;
  gpio_config(&io_conf);

  // disable interrupt
  io_conf.intr_type = GPIO_INTR_DISABLE;
  // set as output mode
  io_conf.mode = GPIO_MODE_INPUT_OUTPUT;
  // bit mask of the pins that you want to set,e.g.GPIO18/19
  io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
  // disable pull-down mode
  io_conf.pull_down_en = 0;
  // disable pull-up mode
  io_conf.pull_up_en = 0;
  // configure GPIO with the given settings
  gpio_config(&io_conf);

  rdySem_Control_refreshSystemTime_task = xSemaphoreCreateBinary();
  xSemaphoreTake(rdySem_Control_refreshSystemTime_task, 0);

  resetProcess();
  // change gpio interrupt type for one pin
  // gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_ANYEDGE);

  // create a queue to handle gpio event from isr
  gpio_evt_queue = xQueueCreate(4, sizeof(uint32_t));
  // start gpio task
  xTaskCreate(gpio_task_example, "gpio_task_example", 8000, NULL, 21, NULL);
  xTaskCreate(task_readInputs, "task_readInputs", 8048, NULL, 32, NULL);
  xTaskCreate(task_refresh_SystemTime, "task_refresh_SystemTime", 9000, NULL,
              15, NULL);

  // // install gpio isr service
  gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
  // // hook isr handler for specific gpio pin
  // // gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void
  // *)GPIO_INPUT_IO_0);
  // // hook isr handler for specific gpio pin
  // // gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void
  // *)GPIO_INPUT_IO_1);

  gpio_isr_handler_add(GPIO_INPUT_IO_EG91_STATUS, gpio_isr_handler,
                       (void *)GPIO_INPUT_IO_EG91_STATUS);

  gpio_isr_handler_add(GPIO_INPUT_IO_SIMPRE, gpio_isr_handler,
                       (void *)GPIO_INPUT_IO_SIMPRE);

  gpio_isr_handler_add(GPIO_INPUT_IO_CD_SDCARD, gpio_isr_handler,
                       (void *)GPIO_INPUT_IO_CD_SDCARD);

  // gpio_isr_handler_add(GPIO_INPUT_EG91_RI, gpio_isr_handler, (void
  // *)GPIO_INPUT_EG91_RI);

  gpio_isr_handler_add(GPIO_INPUT_RTC_ALARM, gpio_isr_handler,
                       (void *)GPIO_INPUT_RTC_ALARM);

  // ////printf("Minimum free heap size: %d bytes\n",
  // esp_get_minimum_free_heap_size());
}

#define TAG "BUTTON"

typedef struct {
  uint8_t pin;
  bool inverted;
  uint16_t history;
  uint32_t down_time;
  uint32_t next_long_time;
} debounce_t;

int pin_count = -1;
debounce_t *debounce;
QueueHandle_t queue;

static void update_button(debounce_t *d) {
  d->history = (d->history << 1) | gpio_get_level(d->pin);
}

#define MASK 0b1111000000111111
static bool button_rose(debounce_t *d) {
  if ((d->history & MASK) == 0b0000000000111111) {
    d->history = 0xffff;
    return 1;
  }
  return 0;
}
static bool button_fell(debounce_t *d) {
  if ((d->history & MASK) == 0b1111000000000000) {
    d->history = 0x0000;
    return 1;
  }
  return 0;
}
static bool button_down(debounce_t *d) {
  if (d->inverted)
    return button_fell(d);
  return button_rose(d);
}
static bool button_up(debounce_t *d) {
  if (d->inverted)
    return button_rose(d);
  return button_fell(d);
}

static uint32_t millis() { return esp_timer_get_time() / 1000; }

static void send_event(debounce_t db, int ev) {
  /* button_event_t event = {
      .pin = db.pin,
      .event = ev,
  };
  xQueueSend(queue, &event, portMAX_DELAY); */
}

static void button_task(void *pvParameter) {
  for (;;) {
    for (int idx = 0; idx < pin_count; idx++) {

      update_button(&debounce[idx]);
      if (button_up(&debounce[idx])) {
        debounce[idx].down_time = 0;
        // ESP_LOGI(TAG, "%d UP", debounce[idx].pin);
        //  send_event(debounce[idx], BUTTON_UP);
      } /*else if (debounce[idx].down_time && millis() >=
      debounce[idx].next_long_time) {
          //ESP_LOGI(TAG, "%d LONG", debounce[idx].pin);
          debounce[idx].next_long_time = debounce[idx].next_long_time +
      CONFIG_ESP32_BUTTON_LONG_PRESS_REPEAT_MS; send_event(debounce[idx],
      BUTTON_HELD);
      }*/
      else if (button_down(&debounce[idx]) && debounce[idx].down_time == 0) {
        debounce[idx].down_time = millis();
        // ESP_LOGI(TAG, "%d DOWN", debounce[idx].pin);
        debounce[idx].next_long_time =
            debounce[idx].down_time +
            CONFIG_ESP32_BUTTON_LONG_PRESS_DURATION_MS;
        // send_event(debounce[idx], BUTTON_DOWN);
      }
    }
    // ////printf("gpio input\n");
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

QueueHandle_t button_init(unsigned long long pin_select) {
  return pulled_button_init(pin_select, GPIO_FLOATING);
}

QueueHandle_t pulled_button_init(unsigned long long pin_select,
                                 gpio_pull_mode_t pull_mode) {
  if (pin_count != -1) {
    // ESP_LOGI(TAG, "Already initialized");
    return NULL;
  }

  // Configure the pins
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_POSEDGE;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_up_en =
      (pull_mode == GPIO_PULLUP_ONLY || pull_mode == GPIO_PULLUP_PULLDOWN);
  io_conf.pull_down_en =
      (pull_mode == GPIO_PULLDOWN_ONLY || pull_mode == GPIO_PULLUP_PULLDOWN);
  ;
  io_conf.pin_bit_mask = pin_select;
  gpio_config(&io_conf);

  // Scan the pin map to determine number of pins
  pin_count = 0;
  for (int pin = 0; pin <= 39; pin++) {
    if ((1ULL << pin) & pin_select) {
      pin_count++;
    }
  }

  // Initialize global state and queue
  debounce = calloc(pin_count, sizeof(debounce_t));
  // queue = xQueueCreate(CONFIG_ESP32_BUTTON_QUEUE_SIZE, sizeof(int));

  // Scan the pin map to determine each pin number, populate the state
  uint32_t idx = 0;
  for (int pin = 0; pin <= 39; pin++) {
    if ((1ULL << pin) & pin_select) {
      // ESP_LOGI(TAG, "Registering button input: %d", pin);
      debounce[idx].pin = pin;
      debounce[idx].down_time = 0;
      debounce[idx].inverted = true;
      if (debounce[idx].inverted)
        debounce[idx].history = 0xffff;
      idx++;
    }
  }

  // Spawn a task to monitor the pins
  xTaskCreate(&button_task, "button_task", CONFIG_ESP32_BUTTON_TASK_STACK_SIZE,
              NULL, 10, NULL);

  return queue;
}

/*************  ✨ Codeium Command 🌟  *************/
void task_readInputs(void *pvParameter) {

  uint8_t Input1_value = 0;
  uint8_t Input2_value = 0;
  uint8_t aux_Input1_value = 0;
  uint8_t aux_Input2_value = 0;
  uint8_t last_Input1_value = 0;
  uint8_t last_Input2_value = 0;
  uint8_t count_I1 = 0;
  uint8_t count_I2 = 0;
  uint8_t debounceCount = 0;
  data_EG91_Send_SMS sms_Data;
  memset(&sms_Data, 0, sizeof(sms_Data));

  MyUser rexMyUser;
  memset(&rexMyUser, 0, sizeof(rexMyUser));
  sprintf(rexMyUser.firstName, "%s", "S/N");
  // sprintf(rexMyUser.phone, "%s", number);
  sprintf(rexMyUser.start.date, "%s","220101");
  sprintf(rexMyUser.start.hour, "%s", "0000");
  sprintf(rexMyUser.end.days, "%c", '*');
  sprintf(rexMyUser.end.hour, "%s", "2359");
  sprintf(rexMyUser.key, "%s", "REX");
  sprintf(rexMyUser.week, "%s", "1111111");
  rexMyUser.permition = '1';
  rexMyUser.relayPermition = '0';
  rexMyUser.ble_security = '0';
  rexMyUser.erase_User_After_Date = '0';
  rexMyUser.wiegand_code[0] = ':';
  rexMyUser.wiegand_code[1] = 0;
  rexMyUser.wiegand_rele_permition = ':';
  rexMyUser.rf_serial[0] = ':';
  rexMyUser.rf1_relay = ':';
  rexMyUser.rf2_relay = ':';
  // dsgaga
  for (;;) {

    Input1_value = !gpio_get_level(CONFIG_GPIO_INPUT_0);
    Input2_value = !gpio_get_level(CONFIG_GPIO_INPUT_1);
    debounceCount = 0;

    while (debounceCount < 5) {
      vTaskDelay(pdMS_TO_TICKS(10));
      aux_Input1_value = !gpio_get_level(CONFIG_GPIO_INPUT_0);
      aux_Input2_value = !gpio_get_level(CONFIG_GPIO_INPUT_1);

      // printf("\n\n aux_input1 %d, input1 %d - aux_input2 %d, input2 %d\n\n",
      // aux_Input1_value, Input1_value, aux_Input2_value, Input2_value);

      if (aux_Input1_value == Input1_value) {
        count_I1++;
      }

      if (aux_Input2_value == Input2_value) {
        count_I2++;
      }

      debounceCount++;
    }

    if (count_I1 == 5) {
      Input1_value = aux_Input1_value;
    } else {
      Input1_value = last_Input1_value;
      count_I1 = 0;
      aux_Input1_value = last_Input1_value;
    }

    if (count_I2 == 5) {
      Input2_value = aux_Input2_value;
    } else {
      aux_Input2_value = last_Input2_value;
      Input2_value = last_Input2_value;
      count_I2 = 0;
    }

    // printf("\n\n last input1 %d, input 1 %d - last input2 %d, input 2
    // %d\n\n", last_Input1_value, Input1_value, last_Input2_value,
    // Input2_value);
    //  ////printf("\n\n last input1 %d, input 1 %d - last input2 %d, input 2
    //  %d\n\n", last_Input1_value, Input1_value, last_Input2_value,
    //  Input2_value);
    /*  Input1_value = !gpio_get_level(CONFIG_GPIO_INPUT_0);
         Input2_value = !gpio_get_level(CONFIG_GPIO_INPUT_1); */

    if (Input1_value != last_Input1_value ||
        Input2_value != last_Input2_value) {
      debounceCount = 0;
      char input_Data_IO[50] = {};
      sprintf(input_Data_IO, "%s %d", "I1 G I", Input1_value);
      BLE_Broadcast_Notify(input_Data_IO);

      printf("\n\nSent BLE Notify: %s\n", input_Data_IO);

      memset(input_Data_IO, 0, 50);
      sprintf(input_Data_IO, "%s %d", "I2 G I", Input2_value);
      BLE_Broadcast_Notify(input_Data_IO);

      printf("\n\nSent BLE Notify: %s\n", input_Data_IO);

      /*  if (label_ResetSystem == 0)
       {
           if (Input1_value != last_Input1_value && Input2_value !=
       last_Input2_value)
           {

               if (Input1_value == 0 && Input2_value == 0)
               {
                   printf("\n\ncount_ResetSystem\n\n");
                   // ////printf("\n\ncount_ResetSystem\n\n");
                   count_ResetSystem++;
               }

               if (count_ResetSystem == 2)
               {
                   label_Reset_Password_OR_System = 1;
                   label_ResetSystem = 1;
                   printf("\n\ncount_ResetSystem11\n\n");
                   // ////printf("\n\ncount_ResetSystem11\n\n");
                   example_tg_timer_deinit(TIMER_GROUP_0, TIMER_0);
                   printf("\n\ncount_ResetSystem22\n\n");
                   // ////printf("\n\ncount_ResetSystem22\n\n");
                   example_tg_timer_init(TIMER_GROUP_0, TIMER_0, false, 60);
                   printf("\n\ncount_ResetSystem33\n\n");
                   // ////printf("\n\ncount_ResetSystem33\n\n");
                   xTimerChangePeriod(xTimers, pdMS_TO_TICKS(1000), 100);
                   printf("\n\ncount_ResetSystem44\n\n");
                   // ////printf("\n\ncount_ResetSystem44\n\n");
               }
           }

           printf("\n\n input 1 - %d / input 2 - %d\n\n", Input1_value,
       Input2_value);
           // ////printf("\n\n input 1 - %d / input 2 - %d\n\n", Input1_value,
       Input2_value);
       }
       else if (label_ResetSystem == 1 && label_Reset_Password_OR_System == 2)
       { */
      uint8_t rex =
          get_INT8_Data_From_Storage(NVS_INPUT_REX_VALUE, nvs_System_handle);
      printf("\n\n aux_input1 123 %d\n\n", rex);
      if (Input1_value != last_Input1_value) {

        printf("\n\n aux_input1 124\n\n");
        if (Input1_value == 1 && (rex == 1 || rex == 3)) {
          printf("\n\n aux_input1 125\n\n");

          /* sprintf(rexMyUser.phone, "%s", "REX");
          sprintf(rexMyUser.key, "%s", "REX"); */
          parse_ReleData(REX_INDICATION, RELE1_NUMBER, 'S', 'R', "REX", NULL,
                         &rexMyUser, NULL, NULL, NULL, NULL, NULL);
          // setReles(1, BLE_INDICATION, NULL, NULL, NULL, NULL, NULL);
        }

        if (label_network_portalRegister == 1) {
          printf("\n\n aux_input1 126\n\n");
          char input_Data_IO[50] = {};
          sprintf(input_Data_IO, "%s %d", "& I1", Input1_value);
          // TODO: DESCOMENTAR LINHA A BAIXO
          //  send_UDP_Send(input_Data_IO,"");
        }
      }

      if (Input2_value != last_Input2_value) {
        // uint8_t rex =
        // get_INT8_Data_From_Storage(NVS_INPUT_REX_VALUE, nvs_System_handle);
        printf("\n\n aux_input1 124 \n\n");
        if (Input2_value == 1 && (rex == 2 || rex == 3)) {
          printf("\n\n aux_input1 125\n\n");
          parse_ReleData(REX_INDICATION, RELE2_NUMBER, 'S', 'R', "REX", NULL,
                         &rexMyUser, NULL, NULL, NULL, NULL, NULL);
          // setReles(2, BLE_INDICATION, NULL, NULL, NULL, NULL, NULL);
        }

        if (label_network_portalRegister == 1) {
          printf("\n\n aux_input1 126\n\n");
          char input_Data_IO[50] = {};
          sprintf(input_Data_IO, "%s %d", "& I2", Input2_value);
          // send_UDP_Send(input_Data_IO,"");
        }
      }

      printf("\n\n input 1 - %d / input 2 - %d\n\n", Input1_value,
             Input2_value);
      // ////printf("\n\n input 1 - %d / input 2 - %d\n\n", Input1_value,
      // Input2_value);
      // }
    }

    last_Input2_value = Input2_value;
    last_Input1_value = Input1_value;

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
/******  4afe1a66-2936-4aba-8bd1-f871c51979c8  *******/