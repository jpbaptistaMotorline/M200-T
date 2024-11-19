/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|

*/

#ifndef _CORE_H_
#define _CORE_H_

#include <stdio.h>
#include <stdlib.h>
#include "users.h"
#include "gpio.h"
#include "inputs.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "EG91.h"
#include "timer.h"
#include "esp_heap_trace.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "mbedtls/base64.h"
#include "rele.h"
#include "rf.h"

extern nvs_handle_t nvs_System_handle;
extern nvs_handle_t nvs_Owner_handle;
extern nvs_handle_t nvs_Admin_handle;
extern nvs_handle_t nvs_Users_handle;
extern nvs_handle_t nvs_Feedback_handle;
extern nvs_handle_t nvs_beep_handle;
extern nvs_handle_t nvs_Routines_handle;
extern nvs_handle_t nvs_Exeption_Days_handle;
nvs_handle_t nvs_Mobile_Holydays_handle;
extern nvs_handle_t nvs_rf_codes_users_handle;
extern nvs_handle_t nvs_rf_codes_admin_handle;
extern nvs_handle_t nvs_rf_codes_owner_handle;

#define FW_VERSION "V3.REV001"
#define HW_VERSION_PROD "0.4.4"

#define BLE_INDICATION 1
#define SMS_INDICATION 2
#define UDP_INDICATION 3
#define WIEGAND_INDICATION 4
#define RF_INDICATION 5
#define REX_INDICATION 6

#define RELE1_NUMBER 1
#define RELE2_NUMBER 2
#define BIESTABLE_MODE_INDEX 0   // 0
#define MONOESTABLE_MODE_INDEX 1 // 1

#define FEEDBACK1_NUMBER 1
#define FEEDBACK2_NUMBER 2
#define FEEDBACK3_NUMBER 3
#define FEEDBACK4_NUMBER 4
#define FEEDBACK5_NUMBER 5
#define FEEDBACK6_NUMBER 6
#define FEEDBACK_MODE_NUMBER 10

#define INPUT1_NUMBER 1
#define INPUT2_NUMBER 2

#define KEY_SIZE 16   // Tamanho da chave AES-128 em bytes
#define BLOCK_SIZE 16 // Tamanho do bloco AES em bytes

#define RELE_BISTATE_MAX_TIME 18000

#define IMSI_NUMBER_REF_1 24007
#define IMSI_NUMBER_REF_2 24803

extern void system_stack_high_water_mark(char *tag);
nvs_handle_t nvs_System_handle;
nvs_handle_t nvs_Owner_handle;
nvs_handle_t nvs_Admin_handle;
nvs_handle_t nvs_Users_handle;
nvs_handle_t nvs_Feedback_handle;
nvs_handle_t nvs_beep_handle;
nvs_handle_t nvs_Routines_handle;
nvs_handle_t nvs_Exeption_Days_handle;
nvs_handle_t nvs_wiegand_codes_users_handle;
nvs_handle_t nvs_wiegand_codes_admin_handle;
nvs_handle_t nvs_wiegand_codes_owner_handle;
nvs_handle_t nvs_rf_codes_users_handle;
nvs_handle_t nvs_rf_codes_admin_handle;
nvs_handle_t nvs_rf_codes_owner_handle;
nvs_handle_t nvs_wiegand_antipassback_USER_handle;
nvs_handle_t nvs_wiegand_antipassback_ADMIN_handle;
nvs_handle_t nvs_wiegand_antipassback_OWNER_handle;

/* static */ SemaphoreHandle_t rdySem_RelayMonoStart;
/* static  */SemaphoreHandle_t rdySem_RelayMonoInicial;
static SemaphoreHandle_t rdySem_Control_Timer_EG91_task;

#define AES_KEY_SIZE 16
#define AES_KEY1 "Ad5Otj0nOpI6VDxw"

unsigned char iv[17];
// #define AES_IV "1BzUeGptJ3SnvnIv"
//  unsigned char aes_key[16] = /*  {0x00, 0x01, 0x02,0x03 ,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03} */;
//  unsigned char iv[16] = "0123456789012345";

uint8_t label_BLE_UDP_send;
uint8_t label_network_portalRegister;

extern uint8_t label_MonoStableRelay1;
extern uint8_t label_MonoStableRelay2;

/* static */ QueueHandle_t queue_BLE_Parameters1;
/* static */ QueueHandle_t queue_BLE_Parameters2;
extern QueueHandle_t queue_EG91_SendSMS;

extern cJSON *sms_Rsp_Json;

extern uint8_t label_Reset_Password_OR_System;
extern uint8_t label_initSystem_SIMPRE;
extern uint8_t label_initSystem_CALL;

extern TimerHandle_t xTimers;

typedef struct
{
    int time;

    int date;
    int8_t year;
    uint8_t month;
    uint8_t day;

    uint8_t weekDay;

    char strTime[30];

} NOW_TIME;

inline uint64_t get_nowTime_in_seconds(void)
{
    time_t now;
    time(&now);
    return now;
}
#define NUM_RECORDS 100
static heap_trace_record_t trace_record[NUM_RECORDS];
void decrypt_aes_cfb_padding(const unsigned char *input, size_t input_len, unsigned char *output);

typedef struct
{
    uint8_t gattsIF;
    uint16_t connID;
    uint16_t handle_table;
    uint8_t BLE_UDP_Indication;
    char payload[200];
    char usr_perm;

} allUsers_parameters_Message;

extern NOW_TIME nowTime;

extern uint32_t date_To_Send_Periodic_SMS;
extern uint8_t label_To_Send_Periodic_SMS;

extern uint8_t network_Activate_Flag;

uint8_t UDP_logs_label;

void init_Storage();
void initNVS_relay1();
void initNVS_relay2();

void vTimerCallback(TimerHandle_t xTimer);
int strpos(const char *haystack, const char *needle);

char *replace_Char_in_String(char *str, char c_to_replace, char c_new);

int add_time(int old_time, int addition);

uint8_t refresh_ESP_RTC();

/// @brief
/// @param inputData
/// @param BLE_SMS_Indication
/// @param gattsIF
/// @param connID
/// @param handle_table
/// @param data_SMS
/// @param mqttInfo
/// @return
char *parseInputData(uint8_t *inputData, uint8_t BLE_SMS_Indication, uint8_t gattsIF, uint16_t connID, uint16_t handle_table, data_EG91_Send_SMS *data_SMS, mqtt_information *mqttInfo);

uint8_t get_RTC_System_Time();
int base64_decode(const char *input, size_t input_len, unsigned char **output);
void add_padding_pkcs7(unsigned char *input, size_t input_len, size_t block_size);
int remove_padding(unsigned char *plaintext, size_t plaintext_length);

void decrypt_aes_cbc_padding(const unsigned char *input, size_t input_len, unsigned char *output, const unsigned char *key, const unsigned char *iv);
void encrypt_aes_cbc(const unsigned char *input, size_t input_len, unsigned char *output, const unsigned char *key, const unsigned char *iv);
char *encrypt_and_base64_encode(const unsigned char *key, const unsigned char *plaintext, size_t plaintext_size);

// void add_padding_pkcs7(unsigned char *input, size_t input_len, size_t block_size);
void modifyJSONValue(const char *key, const char *newValue);

uint8_t parse_CountryPhoneNumber(char *data);
char *check_IF_haveCountryCode(char *phoneNumber, uint8_t label_addUser);
char *check_IF_haveCountryCode_AUX_Call(char *phoneNumber);

uint8_t cmd_process();

char *compress_UDP_Data(uint8_t code, char *payload);

uint8_t restore_FileContacts();
int calculate_weekDay(int year, int month, int day);

void give_LOG_Files_Semph();
void take_LOG_Files_Semph();

// void initSystem();

uint8_t save_STR_Data_In_Storage(char *key, char *payload, nvs_handle_t my_handle);

uint8_t get_STR_Data_In_Storage(char *key, nvs_handle_t my_handle, char *strRSP);

int8_t get_Data_Users_From_Storage(char *key, char *rsp);

uint8_t get_Data_STR_Feedback_From_Storage(char *key, char *rsp);

uint8_t save_INT8_Data_In_Storage(char *key, uint8_t value, nvs_handle_t my_handle);

uint8_t get_INT8_Data_From_Storage(char *key, nvs_handle_t my_handle);

uint8_t save_User_Counter_In_Storage(uint32_t value);

uint32_t get_User_Counter_From_Storage();

void initSystem();

uint8_t save_NVS_Last_Month(uint8_t month);
uint8_t get_NVS_Last_Month();
uint8_t get_Data_STR_LastCALL_From_Storage(char *rsp);

void insert_SubString(char *mainString, char *subString, int position);

char *return_ERROR_Codes(char *input_array, char *ERROR_Code_Message);

char *return_Json_SMS_Data(char *json_Key);

char *hexToAscii(char hex[]);

int valueOf(char symbol);

void string2hexString(char *input, char *output);
char *removeSpacesFromStr(char *string);

char *erase_Password_For_Rsp(char *payload, char *rsp_payload);

void get_NVS_Parameters();

char *remove_non_printable(char *str, size_t len);

void resetProcess();

#endif
