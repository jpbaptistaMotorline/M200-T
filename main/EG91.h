#ifndef _EG91_H
#define _EG91_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "utf8.h"
#include <locale.h>
#include <iconv.h>
#include <errno.h>
#include "ctype.h"

//#include "esp_aes.h"
//#include "core.h"
//#include "gpio.h"
#define BUFF_SIZE 512

#define RSSI_EXCELLENT_LED_TIME 1
#define RSSI_GOOD_LED_TIME 2
#define RSSI_WEAK_LED_TIME 3
#define RSSI_VERY_WEAK_LED_TIME 4
#define RSSI_NOT_DETECT 5
#define MQTT_NOT_CONECT_LED_TIME 8

#define EX_UART_NUM UART_NUM_0
#define PATTERN_CHR_NUM (2) /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/

#define BUF_SIZE (1100)


#define TXD_PIN 17 // 3
#define RXD_PIN 18 // 2

#define INCOMING_CALL_STATE 1
#define CALL_STATE 2

#define RECORD_SOUND_STATE 1
#define PLAY_RECORD_SOUND_STATE 2
#define EMERGENCY_CALL_STATE 3

#define SMS_UNREAD "REC UNREAD"

#define AT_QUEUE_SIZE 2

///* extern  */lost_SMS_Add_User lost_SMS_addUser;



typedef uint32_t codepoint_t;
typedef uint8_t utf8_t; // The type of a single UTF-8 character
typedef uint16_t utf16_t; // The type of a single UTF-16 character

// The last codepoint of the Basic Multilingual Plane, which is the part of Unicode that
// UTF-16 can encode without surrogates
#define BMP_END 0xFFFF

// The highest valid Unicode codepoint
#define UNICODE_MAX 0x10FFFF

// The codepoint that is used to replace invalid encodings
#define INVALID_CODEPOINT 0xFFFD

// If a character, masked with GENERIC_SURROGATE_MASK, matches this value, it is a surrogate.
#define GENERIC_SURROGATE_VALUE 0xD800
// The mask to apply to a character before testing it against GENERIC_SURROGATE_VALUE
#define GENERIC_SURROGATE_MASK 0xF800

// If a character, masked with SURROGATE_MASK, matches this value, it is a high surrogate.
#define HIGH_SURROGATE_VALUE 0xD800
// If a character, masked with SURROGATE_MASK, matches this value, it is a low surrogate.
#define LOW_SURROGATE_VALUE 0xDC00
// The mask to apply to a character before testing it against HIGH_SURROGATE_VALUE or LOW_SURROGATE_VALUE
#define SURROGATE_MASK 0xFC00

// The value that is subtracted from a codepoint before encoding it in a surrogate pair
#define SURROGATE_CODEPOINT_OFFSET 0x10000
// A mask that can be applied to a surrogate to extract the codepoint value contained in it
#define SURROGATE_CODEPOINT_MASK 0x03FF
// The number of bits of SURROGATE_CODEPOINT_MASK
#define SURROGATE_CODEPOINT_BITS 10


// The highest codepoint that can be encoded with 1 byte in UTF-8
#define UTF8_1_MAX 0x7F
// The highest codepoint that can be encoded with 2 bytes in UTF-8
#define UTF8_2_MAX 0x7FF
// The highest codepoint that can be encoded with 3 bytes in UTF-8
#define UTF8_3_MAX 0xFFFF
// The highest codepoint that can be encoded with 4 bytes in UTF-8
#define UTF8_4_MAX 0x10FFFF

// If a character, masked with UTF8_CONTINUATION_MASK, matches this value, it is a UTF-8 continuation byte
#define UTF8_CONTINUATION_VALUE 0x80
// The mask to a apply to a character before testing it against UTF8_CONTINUATION_VALUE
#define UTF8_CONTINUATION_MASK 0xC0
// The number of bits of a codepoint that are contained in a UTF-8 continuation byte
#define UTF8_CONTINUATION_CODEPOINT_BITS 6

#define EG91_FILE_NORMAL_MODE 0
#define EG91_FILE_FOTA_MODE 1
#define EG91_FILE_USERS_MODE 2

typedef struct
{
    char phoneNumber[100];
    char payload[550];
    uint8_t labelRsp;
    uint8_t labelIncomingCall;

} data_EG91_Send_SMS;

typedef struct
{
    uint8_t SMS_ID;
    char phNumber[100];
    char receiveData[400];
    char SMS_Status[20];
    char strHour[20];
    char strDate[50];

} data_EG91_Receive_SMS;

typedef struct mqttINFORMATION 
{
  char topic[70];
  char data[200];
  
}mqtt_information;




typedef struct
{
    char payload[200];
    char rsp[200];

} EG91_SendCommand_Message;

uint8_t disconect_mqtt();

// SemaphoreHandle_t rdySem_EG91_UART;


static QueueHandle_t EG91_CALL_SMS_UART_queue;
static QueueHandle_t EG91_CALL_CLCC_UART_queue;
static QueueHandle_t EG91_CALL_CHUP_UART_queue;
static QueueHandle_t EG91_WRITE_FILE_queue;
//static QueueHandle_t EG91_GET_SIM_BALANCE_queue;

static QueueHandle_t uart0_queue;
static QueueHandle_t AT_Command_Feedback_queue;
static QueueHandle_t UDP_Send_queue;

static QueueHandle_t NO_CARRIER_Call_queue;
static QueueHandle_t Type_Call_queue;
static QueueHandle_t Lost_SMS_queue;
static QueueHandle_t receive_mqtt_queue;
static QueueHandle_t Receive_UDP_OK_queue;
static QueueHandle_t HTTPS_data_queue;

static SemaphoreHandle_t rdySem_Control_SMS_Task;
static SemaphoreHandle_t rdySem_Control_SEND_SMS_Task;
static SemaphoreHandle_t rdySem_Control_Send_AT_Command;
static SemaphoreHandle_t rdySem_Control_IncomingCALL;
static SemaphoreHandle_t rdySem_Feedback_Call;
static SemaphoreHandle_t rdySem_No_Carrie_Call;
static SemaphoreHandle_t rdySem_QPSND;
static SemaphoreHandle_t rdySem_UART_CTR;
static SemaphoreHandle_t rdySem_Lost_SMS;
static SemaphoreHandle_t rdySem_Control_SMS_UDP;
static SemaphoreHandle_t rdySem_Control_pubx;



typedef struct EG91_ReadData_File
{
    uint8_t mode;
    int status;
    int nowFileSize;
    int fileSize;
    uint32_t packetFile_size; 
    uint8_t readOK;
    uint32_t ckm;
    size_t decryptSize;
     char receiveData[BUF_SIZE];

}eg91_read_dataFile_struct;

eg91_read_dataFile_struct EG915_readDataFile_struct;

extern int RSSI_VALUE;
//extern char SIM_Balance[500];

uint8_t mqtt_openLabel;
uint8_t mqtt_connectLabel;


void receive_NoCarrie_queue();
void send_UDP_queue(mqtt_information *mqttInfo);

char fileID[5];

//char data_ReceiveAT_Serial[1024];

void EG915_fota(mqtt_information *mqttInfo);

extern uint8_t SIM_CARD_PIN_status;

void verify_SMS_List();
void give_rdySem_Feedback_Call();
void give_rdySem_Control_Send_AT_Command();
void send_Type_Call_queue(uint8_t state);
void give_rdySem_Control_SMS_Task();

uint8_t EG91_PowerOn();
uint8_t EG91_Power_Reset();
uint8_t EG91_Power_OFF();

/* uint8_t open_mqtt_and_conection(); */

uint8_t parse_OpenFile(char *payload);
uint8_t parse_Confirmation_Send_SMS(char *payload);
uint8_t parseCHUP(char *payload);
int8_t  EG91_Parse_ReceiveData(char *receiveData, char *rsp);

void    EG91_writeFile(char *fileName, char *file,int filesize);
void    record_Feedback_Sound();
void    play_Feedback_Sound();

uint8_t EG91_Send_SMS(char *phNumber, char *text);
uint8_t EG91_Call_PHONE(char *phNumber);

uint8_t EG91_send_AT_Command(char *data, char *rsp, int time);
uint8_t parse_NetworkStatus(char *payload);
uint8_t EG91_parse_QFREAD(char * payload);

uint8_t parse_SMS_data(data_EG91_Receive_SMS *receive_SMS_data);
uint8_t parse_SMS(char *payload);
uint8_t parse_SMS_Payload(char *payload);
uint8_t parse_https_get(char *payload);

uint8_t EG91_parseGet_imsi(char *data);



uint8_t reopen_and_connection();
uint8_t has_letters(const char *str);

uint8_t checkIF_operator_sms(data_EG91_Receive_SMS receive_SMS_data);
int8_t  parse_Call(uint8_t state);
int8_t  parse_IncomingCall_Payload(char *payload);
uint8_t parse_NowTime(char *payload);
uint8_t parse_RSSI(char *payload);
uint8_t parse_SMS_List(char *payload);
uint8_t EG91_parse_CPIN(char *payload);
uint8_t parse_IMEI(char *payload);
uint8_t EG91_parse_CPAS(char *receiveData);
char    *EG91_Send_Parse_CUSD(char *payload,char *output);
uint8_t EG91_parse_CUSD(char *receiveData);
uint8_t EG91_parse_CNUM(char *receiveData);
uint8_t EG91_parse_ICCID(char * receiveData);
uint8_t EG91_parse_IMEI(char * receiveData);

uint8_t get_EG91_IMEI();
uint8_t check_SIM_Card_PIN();

uint8_t getFile_importUsers(char *DNS);

time_t epoch_Calculator(struct tm *tm);

uint8_t init_UDP_socket();

uint8_t send_UDP_Package(char* data, int size, char *topic);
uint8_t parse_Incoming_UDP_data(char *mqtt_data);

uint8_t EG91_UDP_Ping();

uint8_t send_UDP_Send(char *data,char *topic);

void task_EG91_Run_SMS(void *pvParameter);
void task_EG91_Run_IncomingCall(void *pvParameter);
void task_EG91_SendSMS(void *pvParameter);
void task_EG91_Feedback_Call(void *pvParameter);
void task_EG91_Record_Feedback_Call(void *pvParameter);
void task_EG91_Verify_Unread_SMS(void *pvParameter);
void task_EG91_Receive_UDP(void *pvParameter);
void task_EG91_Send_UDP(void *pvParameter);

void give_rdySem_Control_SMS_UDP();
void take_rdySem_Control_SMS_UDP();

void giveSem_CtrIncomingCall();
static void uart_event_task(void *pvParameters);
void uartInit(void);

void init_EG91(void);

void checkIf_mqttSend_fail(char *data);

char *activateUDP_network();
uint8_t desactivateUDP_network();

int u8_wc_toutf8(char *dest, u_int32_t ch);

uint8_t save_NVS_Last_Call(char);

uint8_t EG91_initNetwork();

uint8_t check_NetworkState();

int conv_utf8_to_ucs2(const char* src, size_t len);

void convUTF16(const uint8_t *utf8_buff, uint16_t utf8_buff_len, uint16_t *utf16_buff);

void register_UDP_Device();

uint8_t EG91_get_IMEI(/* char *IMEI */);

uint8_t EG91_Check_IF_Have_PIN();

uint8_t EG91_Unlock_SIM_PIN(char *lastPIN);

uint8_t parse_QMTOPEN(char *receiveData);

uint8_t parseQMTPUBEX(char *receiveData);
uint8_t parse_qmtstat(char *stat_receiveData);
uint8_t parse_at_qmtopen_(char *receiveData);
uint8_t parse_qmtconn(char *receiveData);
uint8_t parse_qmtsub(char *receiveData);

uint8_t parse_verify_mqtt_conection(char *receiveData);
uint8_t parse_verify_mqtt_open(char *receiveData);


int encontrarSubstring(const char *str, const char *subStr);

#endif