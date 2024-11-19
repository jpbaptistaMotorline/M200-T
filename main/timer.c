/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|

*/

#include "timer.h"

#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "gpio.h"
#include "sdkconfig.h"
#include <string.h>
#include <unistd.h>

#include "AT_CMD_List.h"
#include "core.h"
#include "driver/timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "system.h"

uint8_t NetworkFail_Counter = 0;
uint8_t label_timerVerifySystem;

SemaphoreHandle_t rdySem_Timer_Input2_feedback_Timeout;
// SemaphoreHandle_t rdySem_Reset_Password_System_Timeout;
SemaphoreHandle_t semaphore_ACT;

TaskHandle_t xHandle_Timer_VerSystem;
TaskHandle_t xHandle_Timer_Reset_PSW_SYS;

uint8_t RSSI_LED_TOOGLE;

const esp_timer_create_args_t periodic_timer_args = {
    .callback = &periodic_timer_callback,
    /* name is optional, but may help identify the timer when debugging */
    .name = "periodic"};

uint32_t TIMER_BASE_CLK_CALC() {
  timer_config_t config;
  timer_get_config(TIMER_GROUP_0, TIMER_0, &config);
  uint64_t timer_freq = APB_CLK_FREQ;
  if (config.divider > 0) {
    timer_freq /= (config.divider + 1);
  }
  return (uint32_t)timer_freq;
}

void initTimers() {
  label_timerVerifySystem = 0;
  vSemaphoreCreateBinary(semaphore_ACT);
  vSemaphoreCreateBinary(rdySem_Timer_Input2_feedback_Timeout);
  xSemaphoreTake(rdySem_Timer_Input2_feedback_Timeout, 0);
  s_timer_queue = xQueueCreate(2, sizeof(example_timer_event_t));
  alarm_I1_Timeout_queue = xQueueCreate(2, sizeof(example_timer_event_t));

  example_tg_timer_init(TIMER_GROUP_1, TIMER_0, true, 5);
  s_timer_queue = xQueueCreate(2, sizeof(example_timer_event_t));

  // example_tg_timer_init(TIMER_GROUP_0, TIMER_0, false, 10);
  // timer_pause(TIMER_GROUP_0, TIMER_0);

  // example_tg_timer_init(TIMER_GROUP_1, TIMER_1, true, 2);
  // xTaskCreate(&task_VerifySystem, "task_VerifySystem", 4 * 2048, NULL, 2
  // /*32*/, &xHandle_Timer_VerSystem);
  

  // example_tg_timer_init(TIMER_GROUP_0, TIMER_0, false, 10000);
  // timer_pause(TIMER_GROUP_0, TIMER_0);
  //  xTaskCreate(timer0_main_ctrl, "timer0_main_ctrl", 2048, NULL, 3, NULL);

  // ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
  // ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 100000000));
}

void updateSystemTimer(int seconds) {
  // seconds = seconds / 2;
  if (label_ResetSystem == 1) {
    timer_set_alarm_value(TIMER_GROUP_1, TIMER_0, seconds * TIMER_SCALE);
  }
}

void timer0_main_ctrl() {
  while (true) {
    xSemaphoreTake(semaphore_ACT, portMAX_DELAY);
    if (RSSI_VALUE == 99 || RSSI_VALUE == 199) {
      gpio_set_level(GPIO_OUTPUT_ACT, 1);
    } else {
      //////printf("\nact toogle\n");
      //////printf("\nCD CARD PIN %d\n",
      ///gpio_get_level(GPIO_INPUT_IO_CD_SDCARD));
      toggle21 ^= 0x01;
      gpio_set_level(GPIO_OUTPUT_ACT, toggle21);
      //  gpio_set_level(GPIO_NUM_21, toggle21);
    }
  }
}

void interrupt_timer_group0_isr(void *para) {
  toggle04 ^= 0x01;
  // TIMERG0.hw_timer[0].update.tn_update = 1;
  TIMERG0.int_clr_timers.t0_int_clr = 1;

  xSemaphoreGiveFromISR(semaphore_ACT, NULL);

  // TIMERG0.hw_timer[0].config.tn_alarm_en = 1;
}

void example_tg_timer_deinit(int group, int timer) {
  ESP_ERROR_CHECK(timer_isr_callback_remove(group, timer));
  ESP_ERROR_CHECK(timer_deinit(group, timer));
}

static void init_tg0_timer0() {
  int timer_group = TIMER_GROUP_0;
  int timer_idx = TIMER_0;

  // Configure timer
  timer_config_t config;
  config.alarm_en = 1;
  config.auto_reload = 1;
  config.counter_dir = TIMER_COUNT_UP;
  config.divider = TIMER_DIVIDER;
  config.intr_type = TIMER_INTR_SEL;
  config.counter_en = TIMER_PAUSE;
  config.clk_src = TIMER_SRC_CLK_APB;

  timer_init(timer_group, timer_idx, &config);

  timer_pause(timer_group, timer_idx);

  update_ACT_TimerVAlue(RSSI_NOT_DETECT);
  timer_enable_intr(timer_group, timer_idx);
  timer_isr_register(timer_group, timer_idx, interrupt_timer_group0_isr,
                     (void *)timer_idx, ESP_INTR_FLAG_LOWMED, NULL);
  // timer_pause(TIMER_GROUP_0, TIMER_0);
  timer_start(TIMER_GROUP_0, TIMER_0);
}

static bool IRAM_ATTR timer_group_I1_Timeout_isr_callback(void *args) {
  BaseType_t high_task_awoken = pdFALSE;
  example_timer_info_t *info = (example_timer_info_t *)args;
  //////printf("\n\n timer_group_I1_Timeout_isr_callback 1\n\n");
  uint64_t timer_counter_value =
      timer_group_get_counter_value_in_isr(info->timer_group, info->timer_idx);
  //////printf("\n\n timer_group_I1_Timeout_isr_callback 2\n\n");

  /* Prepare basic event data that will be then sent back to task */
  example_timer_event_t evt = {.info.timer_group = info->timer_group,
                               .info.timer_idx = info->timer_idx,
                               .info.auto_reload = info->auto_reload,
                               .info.alarm_interval = info->alarm_interval,
                               .timer_counter_value = timer_counter_value};

  if (!info->auto_reload) {
    timer_counter_value += info->alarm_interval * TIMER_SCALE;
    timer_group_set_alarm_value_in_isr(info->timer_group, info->timer_idx,
                                       timer_counter_value);
  }

  /* Now just send the event data back to the main program task */
  //////printf("\n\n timeout I1 %lld\n\n",timer_counter_value);
  xQueueSendFromISR(alarm_I1_Timeout_queue, &evt, &high_task_awoken);
  return high_task_awoken ==
         pdTRUE; // return whether we need to yield at the end of ISR
}

static bool IRAM_ATTR
timer_group_I2_Normal_Feedback_Timeout_isr_callback(void *args) {
  BaseType_t high_task_awoken = pdFALSE;
  example_timer_info_t *info = (example_timer_info_t *)args;
  //////printf("\n\n timer_group_I1_Timeout_isr_callback 1\n\n");
  uint64_t timer_counter_value =
      timer_group_get_counter_value_in_isr(info->timer_group, info->timer_idx);
  //////printf("\n\n timer_group_I1_Timeout_isr_callback 2\n\n");

  /* Prepare basic event data that will be then sent back to task */
  example_timer_event_t evt = {.info.timer_group = info->timer_group,
                               .info.timer_idx = info->timer_idx,
                               .info.auto_reload = info->auto_reload,
                               .info.alarm_interval = info->alarm_interval,
                               .timer_counter_value = timer_counter_value};

  if (!info->auto_reload) {
    timer_counter_value += info->alarm_interval * TIMER_SCALE;
    timer_group_set_alarm_value_in_isr(info->timer_group, info->timer_idx,
                                       timer_counter_value);
  }

  /* Now just send the event data back to the main program task */
  //////printf("\n\n timeout I1 %lld\n\n",timer_counter_value);
  xSemaphoreGive(rdySem_Timer_Input2_feedback_Timeout);
  return high_task_awoken ==
         pdTRUE; // return whether we need to yield at the end of ISR
}

void init_I1_Timeout_timer0() {
  int group = TIMER_GROUP_0;
  int timer = TIMER_0;

  /* Select and initialize basic parameters of the timer */
  timer_config_t config = {
      .divider = TIMER_DIVIDER,
      .counter_dir = TIMER_COUNT_UP,
      .counter_en = TIMER_PAUSE,
      .alarm_en = TIMER_ALARM_EN,
      .auto_reload = 1,
  }; // default clock source is APB
  //////printf("\n\n timer_init I1 1\n\n");
  timer_init(group, timer, &config);
  //////printf("\n\n timer_init I1 2\n\n");
  timer_set_counter_value(group, timer, 0);
  //////printf("\n\n timer_init I1 3\n\n");

  /* Configure the alarm value and the interrupt on alarm. */
  timer_set_alarm_value(group, timer, 2 * TIMER_SCALE);
  //////printf("\n\n timer_init I1 4\n\n");
  timer_enable_intr(group, timer);
  //////printf("\n\n timer_init I1 5\n\n");

  example_timer_info_t *timer_info = calloc(1, sizeof(example_timer_info_t));
  timer_info->timer_group = group;
  timer_info->timer_idx = timer;
  timer_info->auto_reload = true;
  timer_info->alarm_interval = 2;
  timer_isr_callback_add(group, timer, timer_group_I1_Timeout_isr_callback,
                         timer_info, 0);

  //////printf("\n\n timer_init I1 6\n\n");
  //  timer_pause(TIMER_GROUP_0, TIMER_0);
  timer_start(group, timer);

  //////printf("\n\n timer_init I1 7\n\n");
}

void update_ACT_TimerVAlue(double seconds) {
  seconds = seconds * 1000;
  // seconds = 2000;

  if (label_ResetSystem == 1) {
    if (seconds == (RSSI_NOT_DETECT * 1000)) {
      gpio_set_level(GPIO_OUTPUT_ACT, 1);
    } else {
      xTimerChangePeriod(xTimers, pdMS_TO_TICKS(seconds), 100);
    }
  }

  // seconds = seconds / 2;
  // int timer_group = TIMER_GROUP_0;
  // int timer_idx = TIMER_0;
  // timer_set_alarm_value(timer_group, timer_idx, 1000 /* seconds */ *
  // TIMER_SCALE);
}

static bool IRAM_ATTR timer_group_isr_callback(void *args) {
  BaseType_t high_task_awoken = pdFALSE;
  example_timer_info_t *info = (example_timer_info_t *)args;

  uint64_t timer_counter_value =
      timer_group_get_counter_value_in_isr(info->timer_group, info->timer_idx);

  /* Prepare basic event data that will be then sent back to task */
  example_timer_event_t evt = {.info.timer_group = info->timer_group,
                               .info.timer_idx = info->timer_idx,
                               .info.auto_reload = info->auto_reload,
                               .info.alarm_interval = info->alarm_interval,
                               .timer_counter_value = timer_counter_value};

  if (!info->auto_reload) {
    timer_counter_value += info->alarm_interval * TIMER_SCALE;
    timer_group_set_alarm_value_in_isr(info->timer_group, info->timer_idx,
                                       timer_counter_value);
  }

  /* Now just send the event data back to the main program task */
  xQueueSendFromISR(s_timer_queue, &evt, &high_task_awoken);

  return high_task_awoken ==
         pdTRUE; // return whether we need to yield at the end of ISR
}

static bool IRAM_ATTR
timer_group_Reset_System_OR_Password_isr_callback(void *args) {
  BaseType_t high_task_awoken = pdFALSE;
  example_timer_info_t *info = (example_timer_info_t *)args;

  uint64_t timer_counter_value =
      timer_group_get_counter_value_in_isr(info->timer_group, info->timer_idx);

  /* Prepare basic event data that will be then sent back to task */
  example_timer_event_t evt = {.info.timer_group = info->timer_group,
                               .info.timer_idx = info->timer_idx,
                               .info.auto_reload = info->auto_reload,
                               .info.alarm_interval = info->alarm_interval,
                               .timer_counter_value = timer_counter_value};

  if (!info->auto_reload) {
    timer_counter_value += info->alarm_interval * TIMER_SCALE;
    timer_group_set_alarm_value_in_isr(info->timer_group, info->timer_idx,
                                       timer_counter_value);
  }

  /* Now just send the event data back to the main program task */
  //xTaskCreate(&task_Reset_Password_System_Timeout,
             // "task_Reset_Password_System_Timeout", 8000, NULL, 20,
              //&xHandle_Timer_Reset_PSW_SYS);

  return high_task_awoken ==
         pdTRUE; // return whether we need to yield at the end of ISR
}

/*
 * A simple helper function to print the raw timer counter value
 * and the counter value converted to seconds
 */
static void inline print_timer_counter(uint64_t counter_value) {
  //////printf("Counter: 0x%08x%08x\r\n", (uint32_t)(counter_value >>
  ///32),(uint32_t)(counter_value));

  //////printf("Time   : %.8f s\r\n", (double)counter_value / TIMER_SCALE);
}

/**
 * @brief Initialize selected timer of timer group
 *
 * @param group Timer Group number, index from 0
 * @param timer timer ID, index from 0
 * @param auto_reload whether auto-reload on alarm event
 * @param timer_interval_sec interval of alarm
 */
void example_tg_timer_init(int group, int timer, bool auto_reload,
                           int timer_interval_sec) {
  /* uint32_t clk_src_hz = 0;
 esp_clk_tree_src_get_freq_hz((soc_module_clk_t)TIMER_SRC_CLK_DEFAULT,
 ESP_CLK_TREE_SRC_FREQ_PRECISION_CACHED, &clk_src_hz); */

  timer_config_t config = {
      .divider = TIMER_DIVIDER,
      .counter_dir = TIMER_COUNT_UP,
      .counter_en = TIMER_PAUSE,
      .alarm_en = TIMER_ALARM_EN,
      .auto_reload = auto_reload,
      .clk_src = TIMER_SRC_CLK_APB,
  }; // default clock source is APB
  timer_init(group, timer, &config);

  /* Timer's counter will initially start from value below.
     Also, if auto_reload is set, this value will be automatically reload on
     alarm */
  timer_set_counter_value(group, timer, 0);

  /* Configure the alarm value and the interrupt on alarm. */
  /* timer_set_alarm_value(group, timer, timer_interval_sec * TIMER_SCALE);
  timer_enable_intr(group, timer); */

  example_timer_info_t *timer_info = calloc(1, sizeof(example_timer_info_t));
  timer_info->timer_group = group;
  timer_info->timer_idx = timer;
  timer_info->auto_reload = auto_reload;
  timer_info->alarm_interval = timer_interval_sec;

  if (group == TIMER_GROUP_0) {
    if (timer == TIMER_0) {
      timer_set_alarm_value(group, timer, timer_interval_sec * TIMER_SCALE);
      timer_enable_intr(group, timer);
      timer_isr_callback_add(group, timer,
                             timer_group_Reset_System_OR_Password_isr_callback,
                             timer_info, 0);
      timer_start(group, timer);
    } else if (timer == TIMER_1) {
      timer_set_alarm_value(group, timer, timer_interval_sec * TIMER_SCALE);
      timer_enable_intr(group, timer);
      timer_isr_callback_add(
          group, timer, timer_group_I2_Normal_Feedback_Timeout_isr_callback,
          timer_info, 0);
      timer_start(group, timer);
    }
  } else if (group == TIMER_GROUP_1) {
    if (timer == TIMER_0) {
      timer_set_alarm_value(group, timer, timer_interval_sec * TIMER_SCALE);
      timer_enable_intr(group, timer);
      timer_isr_callback_add(group, timer, timer_group_isr_callback, timer_info,
                             0);
      timer_start(group, timer);
    } else if (timer == TIMER_1) {
      timer_set_alarm_value(group, timer, timer_interval_sec * TIMER_SCALE);
      timer_enable_intr(group, timer);
      timer_isr_callback_add(group, timer, timer_group_I1_Timeout_isr_callback,
                             timer_info, 0);
      timer_start(group, timer);
    }
  }
}

void periodic_timer_callback(void *arg) {
  // uint8_t ACK = 0;

  // ACK = check_NetworkState();

  // if (ACK == 2 || ACK == 3)
  // {
  //     if (NetworkFail_Counter == 3)
  //     {
  //         NetworkFail_Counter = 0;
  //         EG91_Power_Reset();
  //         EG91_initNetwork();
  //     }
  //     else
  //     {
  //         NetworkFail_Counter++;
  //         update_ACT_TimerVAlue((double)RSSI_VERY_WEAK_LED_TIME);
  //         // updateSystemTimer(SYSTEM_TIMER_URGENT_STATE);
  //     }
  // }

  // EG91_send_AT_Command("AT+CSQ", "CSQ", 500);
  return;
}

void task_Reset_Password_System_Timeout(void *pvParameter) {
  if (label_ResetSystem == 0) {
    label_ResetSystem = 1;
    printf("\n\nlabel_ResetSystem == 012345\n\n");
    // ////printf("\n\nlabel_ResetSystem == asdfg\n\n");
    example_tg_timer_deinit(TIMER_GROUP_0, TIMER_0);
    // ////printf("\n\nlabel_ResetSystem == 54321\n\n");

    if (gpio_get_level(GPIO_INPUT_IO_SIMPRE)) {
      if (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS)) {
        printf("\n\nlabel_ResetSystem == qwerty\n\n");
        EG91_send_AT_Command(AT_CSQ, "CSQ", 1000);
      }
    }

    xTaskCreate(&task_VerifySystem, "task_VerifySystem", 4 * 2048 + 2048, NULL,
                /* 5 */ 31, &xHandle_Timer_VerSystem);
    update_ACT_TimerVAlue(3);
    label_Reset_Password_OR_System = 2;
    label_initSystem_CALL = 1;
  } else if (label_ResetSystem == 1) {
    printf("\n\nlabel_ResetSystem == 1\n\n");
    // label_ResetSystem = 2;

    example_tg_timer_deinit(TIMER_GROUP_0, TIMER_0);

    if (gpio_get_level(GPIO_INPUT_IO_SIMPRE)) {
      EG91_send_AT_Command(AT_CSQ, "CSQ", 1000);
    }

    xTaskCreate(&task_VerifySystem, "task_VerifySystem", 4 * 2048 + 2048, NULL,
                /* 5  */ 31, &xHandle_Timer_VerSystem);
    update_ACT_TimerVAlue(3);
    label_Reset_Password_OR_System = 2;
    label_initSystem_CALL = 1;
    // example_tg_timer_init(TIMER_GROUP_0, TIMER_0, false, 60);
  }
  // else if (label_ResetSystem == 2)
  // {
  //     //////printf("\n\nlabel_ResetSystem == 2\n\n");
  //     /* example_tg_timer_deinit(TIMER_GROUP_0, TIMER_0);
  //     label_ResetSystem = 3; */
  // }

  //////printf("\n\ntask_Reset_Password_System_Timeout\n\n");
  // xTimerStop(xTimers, 100);
  // gpio_set_level(GPIO_OUTPUT_ACT, 0);
  // label_Reset_Password_OR_System = 0;
  // vTaskDelay(pdMS_TO_TICKS(2000));
  // uint8_t ledCount = 0;
  // while (ledCount < 5)
  // {
  //     gpio_set_level(GPIO_OUTPUT_ACT, 1);
  //     vTaskDelay(pdMS_TO_TICKS(300));
  //     gpio_set_level(GPIO_OUTPUT_ACT, 0);
  //     vTaskDelay(pdMS_TO_TICKS(300));
  //     ledCount++;
  // }
  // gpio_set_level(GPIO_OUTPUT_ACT, 0);
  // vTaskDelay(pdMS_TO_TICKS(3000));

  // // timer_deinit(group, timer);
  // //timer_pause(TIMER_GROUP_0, TIMER_0);
  // //timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
  // xTimerStart(xTimers, 100);

  // example_tg_timer_deinit(TIMER_GROUP_0, TIMER_0);
  // // timer_pause(TIMER_GROUP_0, TIMER_0);
  vTaskDelete(xHandle_Timer_Reset_PSW_SYS);
}


void task_VerifySystem(void *pvParameter) {
  example_timer_event_t evt;
  while (1) {

    xQueueReceive(s_timer_queue, &evt, portMAX_DELAY);
    // take_rdySem_Control_SMS_UDP();

    printf("Task  State   Prio    Stack    Num\n");

    if (get_INT8_Data_From_Storage(NVS_QMT_LARGE_DATA_TIMER_LABEL,
                                   nvs_System_handle) != 1) {
      /* code */

      if (gpio_get_level(GPIO_INPUT_IO_SIMPRE)) {
        if (!gpio_get_level(GPIO_INPUT_IO_EG91_STATUS)) {
          gpio_set_level(GPIO_OUTPUT_ACT, 1);
          uint8_t pwrFD = 0;
          uint8_t pwrFD_count = 0;

          while (pwrFD != 1) {
            if (pwrFD_count == 3 || pwrFD == 1) {
              break;
            }
            pwrFD_count++;

            pwrFD = EG91_PowerOn();
          }
          uint8_t InitNetworkCount = 0;

          // TODO: TROCAR ESTE IF e retirar o delay
          vTaskDelay(pdMS_TO_TICKS(750));

          //////printf("\n\nSIMPRE INIT NETWORK\n\n");
          while (InitNetworkCount < 3) {

            if (!EG91_initNetwork()) {
              // update_ACT_TimerVAlue((double)RSSI_NOT_DETECT);
              InitNetworkCount++;
              EG91_send_AT_Command("AT+CSDH=1", "OK", 1000);

              EG91_send_AT_Command("AT+QCFG=\"urc/ri/other\",\"off\"", "OK",
                                   1000);
            } else {
              break;
            }
          }
        } else {
          if (check_IF_System_Have_SIM_and_PIN() == 2) {

            // send_UDP_Package("char* data",strlen("char* data"));

            label_timerVerifySystem = 1;
            //////printf("\n------- EVENT TIME --------\n");
            print_timer_counter(evt.timer_counter_value);

            /* Print the timer values as visible by this task */
            // ////printf("-------- TASK TIME --------\n");
            uint64_t task_counter_value;
            timer_get_counter_value(evt.info.timer_group, evt.info.timer_idx,
                                    &task_counter_value);
            print_timer_counter(task_counter_value);
            ESP_LOGI("TAG", "xPortGetFreeHeapSize            : %d",
                     xPortGetFreeHeapSize());
            ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  : %d",
                     esp_get_minimum_free_heap_size());
            ESP_LOGI("TAG", "heap_caps_get_largest_free_block: %zu",
                     heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
            ESP_LOGI("TAG", "free heap memory                : %zu",
                     heap_caps_get_free_size(MALLOC_CAP_8BIT));
            // TODO: DESCOMENTAR LINHA A BAIXO
            verify_System();
          } else {
            // ////printf("-------- TASK TIME 000111--------\n");
            RSSI_LED_TOOGLE = RSSI_NOT_DETECT;

            gpio_set_level(GPIO_OUTPUT_ACT, 1);
            update_ACT_TimerVAlue((double)RSSI_NOT_DETECT);
          }
        }
      } else {

        if (check_IF_System_Have_SIM_and_PIN() == 2) {

          label_timerVerifySystem = 1;
          //////printf("\n------- EVENT TIME --------\n");
          print_timer_counter(evt.timer_counter_value);

          /* Print the timer values as visible by this task */
          //////printf("-------- TASK TIME --------\n");
          uint64_t task_counter_value;
          timer_get_counter_value(evt.info.timer_group, evt.info.timer_idx,
                                  &task_counter_value);
          print_timer_counter(task_counter_value);
          // ESP_LOGI("TAG", "xPortGetFreeHeapSize            : %d",
          // xPortGetFreeHeapSize()); ESP_LOGI("TAG",
          // "esp_get_minimum_free_heap_size  : %d",
          // esp_get_minimum_free_heap_size()); ESP_LOGI("TAG",
          // "heap_caps_get_largest_free_block: %d",
          // heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
          // ESP_LOGI("TAG", "free heap memory                : %d",
          // heap_caps_get_free_size(MALLOC_CAP_8BIT));
          //  TODO: DESCOMENTAR LINHA A BAIXO
          //  verify_System();
        } else {
          RSSI_LED_TOOGLE = RSSI_NOT_DETECT;

          gpio_set_level(GPIO_OUTPUT_ACT, 1);
          update_ACT_TimerVAlue((double)RSSI_NOT_DETECT);
        }
      }

      ESP_LOGI("TAG", "xPortGetFreeHeapSize            : %d",
               xPortGetFreeHeapSize());
      ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  : %d",
               esp_get_minimum_free_heap_size());
      ESP_LOGI("TAG", "heap_caps_get_largest_free_block: %zu",
               heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
      ESP_LOGI("TAG", "free heap memory                : %zu",
               heap_caps_get_free_size(MALLOC_CAP_8BIT));
      //}

      //////printf("\n\ntask_VerifySystem\n\n");
      label_timerVerifySystem = 0;
      // give_rdySem_Control_SMS_UDP();
      //   timer_start(TIMER_GROUP_1, TIMER_0);
    }
  }
}


uint8_t qmtstat_verifyCounter = 0;

uint8_t verify_System() {
  uint8_t ACK = 0;
  uint8_t verifyCount = 0;
  uint8_t qmtstatValue =
      get_INT8_Data_From_Storage(NVS_QMTSTAT_LABEL, nvs_System_handle);
  //////printf("\n\n\n\n verify_System - %d\n\n\n\n", qmtstat_verifyCounter);

  if (qmtstat_verifyCounter == 3) {
    /* code */

    switch (qmtstatValue) {

    case 0:
      qmtstat_verifyCounter = 0;
      break;

    case 1:

      if (init_UDP_socket() == 2) {
        mqtt_openLabel = 1;
        mqtt_connectLabel = 1;
        register_UDP_Device();
        save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL, 0, nvs_System_handle);
        qmtstat_verifyCounter = 0;
      } else {
        mqtt_openLabel = 0;
        mqtt_connectLabel = 0;
        save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL, 1, nvs_System_handle);
        qmtstat_verifyCounter = 0;
      }
      break;

    case 2:

      EG91_Power_Reset();
      EG91_initNetwork();
      qmtstat_verifyCounter = 0;
      /* save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL, 0, nvs_System_handle);
      qmtstat_verifyCounter = 0; */
      break;

    case 6:

      if (init_UDP_socket() == 2) {
        mqtt_openLabel = 1;
        mqtt_connectLabel = 1;
        register_UDP_Device();
        save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL, 0, nvs_System_handle);
        qmtstat_verifyCounter = 0;
      } else {
        mqtt_openLabel = 0;
        mqtt_connectLabel = 0;
        save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL, 6, nvs_System_handle);
        qmtstat_verifyCounter = 0;
      }
      break;

    case 4:

      if (reopen_and_connection()) {
        mqtt_openLabel = 1;
        mqtt_connectLabel = 1;
        register_UDP_Device();
        save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL, 0, nvs_System_handle);
        qmtstat_verifyCounter = 0;
      } else {
        mqtt_openLabel = 0;
        mqtt_connectLabel = 0;
        save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL, 4, nvs_System_handle);
        qmtstat_verifyCounter = 0;
      }
      break;
    case 8:

      if (init_UDP_socket() == 2) {
        mqtt_openLabel = 1;
        mqtt_connectLabel = 1;
        register_UDP_Device();
        save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL, 0, nvs_System_handle);
        qmtstat_verifyCounter = 0;
      } else {
        mqtt_openLabel = 0;
        mqtt_connectLabel = 0;
        save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL, 8, nvs_System_handle);
        qmtstat_verifyCounter = 0;
      }

      break;

    default:
      qmtstat_verifyCounter = 0;
      break;
    }
  } else {
    qmtstat_verifyCounter++;
  }

  while (verifyCount < 3) {
    ACK = check_NetworkState();
    // ////printf("\n\n ACK check_NetworkState - %d\n\n",ACK);

    if (ACK != 2 || ACK != 3) {
      break;
    }

    verifyCount++;
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  // ACK = check_NetworkState();
  //////printf("\n\ncheck_NetworkState - %d\n\n", ACK);
  //////printf("\n\n network check = %d\n\n", ACK);
  if (ACK == 2 || ACK == 3) {

    // ////printf("\n\n ACK check_NetworkState fail - %d\n\n",ACK);
    if (NetworkFail_Counter == 3) {
      NetworkFail_Counter = 0;
      EG91_Power_Reset();
      EG91_initNetwork();
    } else {
      NetworkFail_Counter++;
      update_ACT_TimerVAlue((double)RSSI_VERY_WEAK_LED_TIME);
      // updateSystemTimer(SYSTEM_TIMER_URGENT_STATE);
    }
  } else if (qmtstatValue != 0) {
    RSSI_LED_TOOGLE = MQTT_NOT_CONECT_LED_TIME;
    // update_ACT_TimerVAlue((double)RSSI_EXCELLENT_LED_TIME);
    updateSystemTimer(SYSTEM_TIMER_ALARM_STATE);
  }

  // ESP_LOGI("TAG", "xPortGetFreeHeapSize            : %d",
  // xPortGetFreeHeapSize()); ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  :
  // %d", esp_get_minimum_free_heap_size()); ESP_LOGI("TAG",
  // "heap_caps_get_largest_free_block: %d",
  // heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT)); ESP_LOGI("TAG",
  // "free heap memory                : %d",
  // heap_caps_get_free_size(MALLOC_CAP_8BIT));
  return 1;
}

void List_Backup_File_Contacts() {
  // Open renamed file for reading
  //////printf("Reading file");
  FILE *f1 = fopen("/spiffs/backup.txt", "r");

  if (f1 == NULL) {
    //////printf("Failed to open file for reading");
  }
  char ch1[200];

  while (fgets(ch1, 256, f1) != NULL) {
    // ch = fgetc(f);
    //////printf("backup - %s", ch1);
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  fclose(f1);
}

void backup_ContactsFormInternalFlash() {}