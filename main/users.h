/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______ 
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__   
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|  
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____ 
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|
                                                                                                                          
*/

#ifndef _USERS_H
#define _USERS_H

#include <stdint.h>
#include <stdio.h>

#include <ctype.h>
#include "stdbool.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "iconv.h"
#include "nvs.h"

#include "EG91.h"
#include "keeloqDecrypt.h"



#define MYUSER_LIST_SIZE MYSTORAGE_MAX_SLOTS
/******************************************************************************/
#define MYUSER_PHONE_SIZE ((uint8_t)(20))

extern SemaphoreHandle_t rdySem;

#define SUN 1
#define MON 2
#define TUE 4
#define WED 8
#define THU 16
#define FRI 32
#define SAT 64

#define SUN_STR "SUN"
#define MON_STR "MON"
#define TUE_STR "TUE"
#define WED_STR "WED"
#define THU_STR "THU"
#define FRI_STR "FRI"
#define SAT_STR "SAT"

#define DEFAULT_PASSWORD "999999"
#define DEFAULT_USER_PASSWORD "888888"
#define DEFAULT_ADMIN_PASSWORD "999999"

#define LIMIT_USERS_REGISTER_NUMBER 2000

extern uint8_t readAllUser_Label;
extern uint16_t readAllUser_ConnID;

extern int UsersCountNumbers;
extern uint32_t GuestCountNumbers;
/* extern mqtt_information mqttInformation_user; */

static QueueHandle_t allUsers_msg_queue;


typedef struct Lost_SMS_ADD_USER
{
    char phone[100];
    char payload[200];
    uint8_t point_Numbers;
    uint8_t flag_Lost_SMS_Add_User;
    mqtt_information mqttInfo;
} lost_SMS_Add_User;

lost_SMS_Add_User lost_SMS_addUser;

typedef struct StructUser
{
    char phone[MYUSER_PHONE_SIZE];
    //char Name[25];
    char firstName[25];
   

    struct
    {
        char date[7];
        char hour[5];

    } start;

    struct
    {
        char days[3];
        char hour[5];
    } end;
    
    char key[7];
    char permition;
    char week[8];
    char relayPermition;
    char ble_security;
    char erase_User_After_Date;

    #if CONFIG_WIEGAND_CODE == 1
 
    char wiegand_code[MYUSER_PHONE_SIZE];
    char wiegand_rele_permition;
    
    #endif // DEBUG

    char rf_serial[MYUSER_PHONE_SIZE];
    char rf1_relay;
    //char rf2[MYUSER_PHONE_SIZE];
    char rf2_relay;
    //char rf3_relay;    

} MyUser;

//MyUser user;

void printUser(MyUser user);
bool isLeapYear(int year);
int parseDatetoInt(int date, int days);
uint8_t check_DateisValid(char *date);
uint8_t validate_Hour(char *str);
uint16_t MyUser_Add(MyUser *user);
uint32_t MyUser_List_AllUsers();
uint8_t MyUser_Search_User(char *phoneNumber, char *file_contents);
uint8_t MyUser_Search_User_AUX_Call(char *phoneNumber, char *file_contents);
void parse_ValidateData_User(char *file_contents, MyUser *user_validateData);
uint8_t validate_DataUser(MyUser *user_validateData, char *password);
uint8_t Myuser_deleteUser(MyUser *user);
uint8_t Myuser_delete_ALLUser();
uint8_t MyUser_newPassword(MyUser *user_validateData, char *password, int line);
uint8_t check_NewUser_Data(MyUser *user_validateData);
uint8_t add_Default_Number(char *number);
uint8_t verify_WeekAcess(MyUser *user_validateData);
uint8_t verify_TimeAcess(MyUser *user_validateData);

uint8_t rf_auto_save(char rfButon, uint64_t rfSerial, uint8_t relay,char *outputData);

char *MyUser_ReadUser(uint8_t BLE_SMS_Indication, char *payload,char usrPermition,char* phNumber_Get);
char *MyUser_ReadAllUsers(uint8_t gattsIF, uint16_t connID, uint16_t handle_table,char userPermition, uint8_t BLE_UDP_Indication);
uint8_t MyUser_newUSER(char *payload,char *newUserData,char* phNumber,char *password);
char *MyUser_new_MultiUSERS(char *payload,char *mrsp);
char *MyUser_new_hour(uint8_t BLE_SMS_Indication,char *payload,mqtt_information *mqttInfo);
char *MyUser_new_WeekDays(uint8_t BLE_SMS_Indication,char *payload,mqtt_information *mqttInfo);
char *MyUser_new_LimitTime(uint8_t BLE_SMS_Indication,char *payload, mqtt_information *mqttInfo);
char *MyUser_get_LimitTime(uint8_t BLE_SMS_Indication,char *payload,char *mrsp);
char *MyUser_reset_LimitTime(uint8_t BLE_SMS_Indication,char *payload, mqtt_information *mqttInfo);

char *import_mqtt_users(char* URL,char *ownerNumber, char *ownerPassword);

uint8_t sendUDP_all_User_funtion();

char *MyUser_get_ReleRestrition(char* payload,uint8_t BLE_SMS);

char *parse_UserData(uint8_t BLE_SMS_Indication, int line, char cmd, char param, char *phPassword, char *payload, MyUser *user_validateData, uint8_t gattsIF, uint16_t connID, uint16_t handle_table,mqtt_information *mqttInfo);

char *reset_Label_BLE_Security(char *payload,uint8_t BLE_SMS);

uint8_t add_onlyNewWiegand_number(char *payload);

uint8_t replaceUser(MyUser *user_validateData);
char *MyUser_get_startDate(uint8_t BLE_SMS_Indication,char *payload);
char *MyUser_new_StartDate(uint8_t BLE_SMS_Indication,char *payload,char *mrsp,mqtt_information *mqttInfo);
char *MyUser_new_UserName(uint8_t BLE_SMS_Indication, char *payload, char *mrsp,char permition,mqtt_information *mqttInfo);
char *MyUser_getFromName(char* payload,uint8_t gattsIF, uint16_t connID, uint16_t handle_table);

uint8_t checkIf_rfSerialExist(char* payload, char* user_contents);

char *MyUser_Restore_Password(char* payload, char permition,uint8_t BLE_SMS);

uint8_t checkIf_wiegandExist(char* payload, char *user_contents);
uint8_t check_IF_Exist_IN_Other_NVS(char *key);

uint8_t MyUser_add_wiegand(char *wiegandCode,char *payload, char permition);
uint8_t MyUser_add_rfSerial(char *rfSerial, char *payload, char permition);

char *MyUser_Set_Relay_Restrition(MyUser *user_validateData,uint8_t BLE_SMS_Indication,char *payload, mqtt_information *mqttInfo);

uint8_t erase_Users_With_LastTime(int nowDate);

char *return_ERROR_New_Add_User(char *output,uint8_t error_ID);
char *MyUser_add_Owner(char* payload,char *SMS_phoneNumber,uint8_t BLE_SMS_Indication);

static inline char *stristr (const char *haystack, const char *needle);

void inserirSubstringNoInicio(char destino[], const char origem[], int posicao);

uint8_t add_defaultUser_wiegand(uint64_t wiegandCode);
//void ReadALLUsers_task(allUsers_parameters_Message *pvParameter);
void copiar_a_partir_do_segundo_caractere(const char* original, char* destino);

#endif