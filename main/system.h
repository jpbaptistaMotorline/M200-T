/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|

*/

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "EG91.h"
#include "users.h"

#define FEEDBACK_AUDIO_RECORD 1
#define FEEDBACK_AUDIO_PLAY 2
#define FEEDBACK_AUDIO_TIMEOUT 30000

// uint8_t feedback_Audio_timeout;
extern uint8_t label_Block_Feedback_SMS;
// uint8_t label_Translate_Files;

extern SemaphoreHandle_t rdySem_Send_LOGS_Files;
extern SemaphoreHandle_t rdySem_Reset_System;

extern uint8_t label_Semaphore_Reset_System;

extern uint8_t label_ResetSystem;
extern uint8_t count_ResetSystem;

char *parse_SystemData(uint8_t BLE_SMS_Indication, char cmd, char param,
                       char *phPassword, char *payload,
                       MyUser *user_validateData, data_EG91_Send_SMS *data_SMS,
                       mqtt_information *mqttInfo);
char *MyUser_add_Admin(uint8_t BLE_SMS_INDICATION, char *payload,
                       mqtt_information *mqttInfo);
uint8_t MyUser_add_Admin_multi(uint8_t BLE_SMS_INDICATION, char *payload);
char *MyUser_erase_Admin(uint8_t BLE_SMS_INDICATION, char *payload,
                         mqtt_information *mqttInfo);

char *MyUser_change_USER_To_ADMIN(uint8_t BLE_SMS_INDICATION, char *payload,
                                  mqtt_information *mqttInfo);
char *MyUser_change_ADMIN_To_USER(uint8_t BLE_SMS_INDICATION, char *payload,
                                  mqtt_information *mqttInfo);

char *set_Alarm_Sound(char *payload);
char *play_Alert_Sound();
char *return_Start_BLE_Data();
char *get_LogFiles_profiles();
char *send_LogFile(uint8_t gattsIF, uint16_t connID, uint16_t handle_table,
                   char userPermition, char *payload);
char *set_System_Clock(char *payload);
char *get_System_TimeZone();

char *set_Block_Feedback_SMS(uint8_t BLE_SMS_INDICATION);
char *get_Block_Feedback_SMS(uint8_t BLE_SMS_INDICATION);
char *reset_Block_Feedback_SMS(uint8_t BLE_SMS_INDICATION);

uint8_t check_And_Erase_Admin_In_Feedback_List(char *phoneNumber);
void setTimeZone(long offset, int daylight);

char *parse_Start_Download_File(char *payload);
uint8_t start_Download_Sound_File();

char *save_Language_TranslateFile(char *payload);

uint8_t system_Reset();

uint8_t reset_Owner_password(char *payload, data_EG91_Send_SMS *data_SMS);

char *change_Owner(char *payload, uint8_t BLE_SMS);

uint8_t check_IF_System_Have_SIM_and_PIN();

uint8_t block_Feedback_SMS();
uint8_t unlock_Feedback_SMS();
uint8_t get_Feedback_SMS();

char *UDP_activate_desativate_logs(uint8_t log_action);

void task_Send_LOG_File(void *pvParameter);

void give_rdySem_Reset_System();

uint8_t changeInputTranslators(char *payload);

#endif