/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|

*/

#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdio.h>
#include "core.h"
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "driver/adc.h"
#include "driver/mcpwm.h"
//#include "soc/mcpwm_reg.h"
//#include "soc/mcpwm_struct.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "soc/syscon_periph.h"
#include "EG91.h"

#define TIMER_RESOLUTION_HZ   1000000 // 1MHz resolution
#define TIMER_ALARM_PERIOD_S  0.5     // Alarm period 0.5s

#define TIMER_BASE_CLK APB_CLK_FREQ //(esp_timer_get_counter_hz())



#define TIMER_DIVIDER (16)                               //  Hardware timer clock divider
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER)     // convert counter value to seconds
#define TIMER_INTR_SEL TIMER_INTR_LEVEL                  // Timer level interrupt
#define TIMER_GROUP TIMER_GROUP_0                        // Test on timer group 0
#define TIMER_INTERVAL0_SEC (1.001)                      // test interval for timer 0 [1000usec/1.0msec(0.001),100usec/0.1msec(0.0001),8.5usec/0.0085msec(0.00001)]
#define TIMER_CPU_CLOCK 160000000L                       // CPU Clock 160Mhz
#define TIMER_SCALE160 (TIMER_CPU_CLOCK / TIMER_DIVIDER) // used to calculate counter value

#define SYSTEM_TIMER_NORMAL_STATE 30
#define SYSTEM_TIMER_ALARM_STATE 15
#define SYSTEM_TIMER_URGENT_STATE 5 

// SemaphoreHandle_t rdySem_Timer_Input2_feedback_Timeout;
// //SemaphoreHandle_t rdySem_Reset_Password_System_Timeout;
// SemaphoreHandle_t semaphore_ACT;

static QueueHandle_t s_timer_queue;
static QueueHandle_t alarm_I1_Timeout_queue;

extern TaskHandle_t xHandle_Timer_VerSystem;


extern uint8_t RSSI_LED_TOOGLE;

static bool toggle21 = 0x01;
static bool toggle04 = 0x01;

typedef struct
{
    int timer_group;
    int timer_idx;
    int alarm_interval;
    bool auto_reload;
} example_timer_info_t;

/**
 * @brief A sample structure to pass events from the timer ISR to task
 *
 */
typedef struct
{
    example_timer_info_t info;
    uint64_t timer_counter_value;
} example_timer_event_t;

void periodic_timer_callback(void *arg);
void updateSystemTimer(int seconds);


//esp_timer_handle_t periodic_timer;




void initTimers();

void task_VerifySystem(void *pvParameter);

void task_Reset_Password_System_Timeout(void *pvParameter);

static void init_tg0_timer0();
void interrupt_timer_group0_isr(void *para);
//void timer0_main_ctrl();
void update_ACT_TimerVAlue(double seconds);
static void inline print_timer_counter(uint64_t counter_value);
void example_tg_timer_init(int group, int timer, bool auto_reload, int timer_interval_sec);
uint8_t verify_System();
void backup_ContactsFormInternalFlash();
void List_Backup_File_Contacts();
void init_I1_Timeout_timer0();
void example_tg_timer_deinit(int group, int timer);



#endif