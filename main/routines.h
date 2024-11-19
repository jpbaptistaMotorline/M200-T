/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______ 
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__   
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|  
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____ 
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|
                                                                                                                          
*/

#ifndef _ROUTINES_H
#define _ROUTINES_H

#include "stdio.h"
#include "stdint.h"
#include "core.h"
#include "unity.h"
#include "cron.h"
#include "jobs.h"
#include "list.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#define CARNIVAL_HOLYDAY_DAY 1
#define EASTER_FRIDAY_HOLYDAY_DAY 2
#define EASTER_HOLYDAY_DAY 4
#define CORPUS_CHRISTI_HOLYDAY_DAY 8
#define EASTER_MONDAY_HOLYDAY_DAY 16

extern TaskHandle_t task_Routine_BiState_RelayState1;
extern TaskHandle_t task_Routine_BiState_RelayState2;

extern allUsers_parameters_Message Routines_message;

extern SemaphoreHandle_t rdySem_Send_Routines;
extern SemaphoreHandle_t rdySem_Routine_BiState_Send_RelayState1;
extern SemaphoreHandle_t rdySem_Routine_BiState_Send_RelayState2;

static QueueHandle_t send_Routines_queue;

extern uint8_t label_Routine1_ON;
extern uint8_t label_Routine2_ON;


extern uint8_t routines_ID;
extern struct list_node *routine_list;

void initRoutines();

char *setRoutine(char *payload);

char *getRoutines(uint8_t gattsIF, uint16_t connID, uint16_t handle_table,uint8_t BLE_UDP_Indication);

char *eraseRoutines();

char *set_ExeptionDay(char* payload);

char *set_Routine_Range(char* payload);

char *erase_Routine_Range();

uint8_t get_Routine_Range(char *inicalTime,char *finalTime);

char *erase_ExeptionDay();

char *get_ExeptionDay();

char *activate_Routines(uint8_t BLE_indication,uint8_t gattsIF, uint16_t connID, uint16_t handle_table);

void test_cron_job_sample_callback(cron_job *job);

char *parse_RoutineData(uint8_t BLE_SMS_Indication,uint8_t gattsIF, uint16_t connID,uint16_t handle_table, char cmd, char param, char *payload);

char *set_holidaysDays(uint8_t holidaysDays);
char *get_holidaysDays();
char *reset_holidaysDays();

char *set_rangeHolidaysDays();
uint8_t get_rangeHolidaysDays(char *inicalTime, char *finalTime);
char *reset_rangeHolidaysDays();

void refresh_holidaysDays();

void task_Routine_BiState_Send_RelayState1(void *pvParameter);
void task_Routine_BiState_Send_RelayState2(void *pvParameter);
void task_Send_Routines(void *pvParameter);

uint8_t send_udp_routines_funtion();

void calcularPascoa(int ano, int *dia, int *mes);

int subtrairDias(int date, int days);

uint8_t set_easter_day();
uint8_t set_carnival();
uint8_t set_corpusChristi();
uint8_t set_easter_friday();
uint8_t set_easter_monday();





#endif