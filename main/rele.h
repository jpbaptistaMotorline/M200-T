/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______ 
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__   
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|  
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____ 
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|
                                                                                                                          
*/

#ifndef _RELE_H
#define _RELE_H

#include "users.h"
#include "stdio.h"
#include "core.h"
#include "EG91.h"

#define NVS_R1_MODE   "NVS_R1_MODE"
#define NVS_R2_MODE   "NVS_R2_MODE"
#define NVS_R1_TIME   "NVS_R1_TIME"
#define NVS_R2_TIME   "NVS_R2_TIME"
#define NVS_R1_RESTR  "NVS_R1_RESTR"

extern TaskHandle_t rele1_Bistate_Task_Handle;
extern TaskHandle_t rele2_Bistate_Task_Handle;

extern uint8_t rele1_Mode_Label;
extern int rele1_Bistate_Time;
extern uint8_t rele2_Mode_Label;
extern int rele2_Bistate_Time;


extern QueueHandle_t queue_BLE_Parameters1;
extern QueueHandle_t queue_BLE_Parameters2;
extern QueueHandle_t receive_mqtt_queue;

extern SemaphoreHandle_t rdySem_RelayMonoInicial;
extern SemaphoreHandle_t rdySem_RelayMonoStart;
/* SemaphoreHandle_t rdySem_Control_Send_RelayState1;
SemaphoreHandle_t rdySem_Control_Send_RelayState2; */




typedef struct
{
    uint8_t gattsIF;
    uint16_t connID;
    uint16_t handle_table;
    uint8_t relaynumber;
    uint8_t BLE_SMS_INDICATION;
    mqtt_information mqttInfo_ble;
    data_EG91_Send_SMS EG91_data;
    char payload[200];

}data_BLE_Send_RelayState;

extern uint8_t rele1_Restriction;

void giveMonoSt_semaphore();

void setRele1();

void toogleRele1();
void toogleRele2();

void setRele2();

void resetRele1();

void resetRele2();

uint8_t getRele1();

uint8_t getRele2();

uint8_t setReles(int releNumber, uint8_t BLE_SMS_Indication, uint8_t gattsIF, uint16_t connID, uint16_t handle_table, data_EG91_Send_SMS *data_SMS, mqtt_information *mqttInfo);

char* parse_ReleData(uint8_t BLE_SMS_Indication,uint8_t releNumber, char cmd,char param,char* phPassword,char* payload, MyUser *user_validateData,uint8_t gattsIF, uint16_t connID, uint16_t handle_table,data_EG91_Send_SMS *data_SMS, mqtt_information *mqttInfo);

char* configuration_Relay_Parameter(char* payload,uint8_t BLE_SMS, mqtt_information *mqttInfo);

void task_BiState_Send_RelayState1(void *pvParameter);
void task_BiState_Send_RelayState2(void *pvParameter);

char *getUserApp_id(char *topic);

#endif