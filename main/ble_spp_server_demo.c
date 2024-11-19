/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|

*/

#include "driver/uart.h"
#include "esp_bt.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "stdlib.h"
#include "string.h"
#include <sys/time.h>
#include <time.h>

#include "ble_spp_server_demo.h"
#include "core.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_spiffs.h"
#include "rele.h"
#include "sdCard.h"
#include <gpio.h>
#include <stdio.h>
#include <users.h>

#include "timer.h"

#include "AT_CMD_List.h"
#include "EG91.h"
#include "esp_heap_trace.h"

#include "sdCard.h"
#include <stdio.h>

#include "UDP_Codes.h"
#include "crc32.h"
#include "cron.h"
#include "jobs.h"
#include "pcf85063.h"
#include "timer.h"
#include "unity.h"
#include <sys/time.h>
#include <time.h>

#include "cJSON.h"

#include "errno.h"
#include "esp_flash_partitions.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "routines.h"

#include "esp_bt.h"
#include "osi/allocator.h"
#include "osi/future.h"

#include "esp_heap_trace.h"
#include "inputs.h"
#include <rf.h>
#include "mbedtls/aes.h"
#include "mbedtls/base64.h"
#include "wiegand.h"

// #include "esp_clk.h"

#define GATTS_TABLE_TAG "GATTS_SPP_DEMO"

#define SPP_PROFILE_NUM 1
#define SPP_PROFILE_APP_IDX 0
#define ESP_SPP_APP_ID 0x56
#define SAMPLE_DEVICE_NAME                                                     \
  "MOTORLINE M200" // The Device Name Characteristics in GAP
#define SPP_SVC_INST_ID 0

/// SPP Service
static const uint16_t spp_service_uuid = 0xABF0;
static const uint16_t spp_UserList_service_uuid = 0xBCF0;
static const uint16_t spp_Files_service_uuid = 0xCCF0;
static const uint16_t spp_Routines_service_uuid = 0xEDF0;
/// Characteristic UUID
#define ESP_GATT_UUID_SPP_DATA_RECEIVE 0xABF1
#define ESP_GATT_UUID_SPP_DATA_NOTIFY 0xABF2
#define ESP_GATT_UUID_SPP_COMMAND_RECEIVE 0xABF3
#define ESP_GATT_UUID_SPP_COMMAND_NOTIFY 0xABF4

#define ESP_GATT_UUID_SPP_USERLIST_DATA_RECEIVE 0xBCF1
#define ESP_GATT_UUID_SPP_USERLIST_DATA_NOTIFY 0xBCF2
#define ESP_GATT_UUID_SPP_USERLIST_COMMAND_RECEIVE 0xBCF3
#define ESP_GATT_UUID_SPP_USERLIST_COMMAND_NOTIFY 0xBCF4

#define ESP_GATT_UUID_SPP_FILES_DATA_RECEIVE 0xCCF1
#define ESP_GATT_UUID_SPP_FILES_DATA_NOTIFY 0xCCF2
#define ESP_GATT_UUID_SPP_FILES_COMMAND_RECEIVE 0xCCF3
#define ESP_GATT_UUID_SPP_FILES_COMMAND_NOTIFY 0xCCF4

#define ESP_GATT_UUID_SPP_ROUTINES_DATA_RECEIVE 0xEDF1
#define ESP_GATT_UUID_SPP_ROUTINES_DATA_NOTIFY 0xEDF2
#define ESP_GATT_UUID_SPP_ROUTINES_COMMAND_RECEIVE 0xEDF3
#define ESP_GATT_UUID_SPP_ROUTINES_COMMAND_NOTIFY 0xEDF4

#ifdef SUPPORT_HEARTBEAT
#define ESP_GATT_UUID_SPP_HEARTBEAT 0xABF5
#endif

#define GATTS_DEMO_CHAR_VAL_LEN_MAX 500
#define PREPARE_BUF_MAX_SIZE 1024
#define CHAR_DECLARATION_SIZE (sizeof(uint8_t))

#define ADV_CONFIG_FLAG (1 << 0)
#define SCAN_RSP_CONFIG_FLAG (1 << 1)

uint16_t active_BLE_conn[8];

static uint16_t spp_handle_table[SPP_IDX_NB];
static uint16_t spp_UserList_handle_table[SPP_USERLIST_NB];
static uint16_t spp_Files_handle_table[SPP_FILES_NB];
static uint16_t spp_Routines_handle_table[SPP_ROUTINES_NB];
uint16_t heart_rate_handle_table[HRS_IDX_NB];

static uint8_t sec_service_uuid[16] = {
    /* LSB
       <-------------------------------------------------------------------------------->
       MSB */
    // first uuid, 16bit, [12],[13] is the value
    0x2f, 0x5a, 0x39, 0x2f, 0xc1, 0x04, 0x94, 0xa6,
    0xdb, 0x41, 0x81, 0xae, 0xc3, 0x44, 0xc9, 0xf7,
};

static uint8_t adv_config_done = 0;
static uint8_t test_manufacturer[16] = "MOTORLINE_M200";
static uint8_t test_MANUTEST[10] = "TESTETESTE";
static SemaphoreHandle_t rdySem_BLE_SPP;
// extern SemaphoreHandle_t rdySem_RelayMonoInicial;

typedef struct {
  uint8_t *prepare_buf;
  int prepare_len;
} prepare_type_env_t;

static prepare_type_env_t prepare_write_env;

// /* static const uint8_t spp_adv_data[31] = {
//     /* Flags */
//     0x02, 0x01, 0x06,
//     /* Complete List of 16-bit Service Class UUIDs */
//     0x03, 0x03, 0xF0, 0xFE,
//     /* Complete Local Name in advertising */
//     0x16, 0x09, 'M', 'O', 'T', 'O', 'R', 'L', 'I', 'N', 'E', ' ', 'M', '2',
//     '0', '0'}; */

/* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */

// #ifdef SUPPORT_HEARTBEAT
// static QueueHandle_t cmd_heartbeat_queue = NULL;
// static uint8_t heartbeat_s[9] = {'E', 's', 'p', 'r', 'e', 's', 's', 'i',
// 'f'}; static bool enable_heart_ntf = false; static uint8_t
// heartbeat_count_num = 0; #endif

static esp_ble_adv_data_t spp_adv_config = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x20 /* 500 / 0.625 */, // slave connection min interval,
                                            // Time = min_interval * 1.25 msec
    .max_interval = 0x40 /* 500 / 0.625 */, // slave connection max interval,
                                            // Time = max_interval * 1.25 msec
    .appearance = 0x00,
    .manufacturer_len =
        sizeof(test_manufacturer), // TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = /* test_MANUTEST,  NULL,*/ &test_manufacturer[0],
    .service_data_len = sizeof(sec_service_uuid),
    .p_service_data = &sec_service_uuid,
    .service_uuid_len = 2,
    .p_service_uuid = {0xAE, 0xF0},
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),

};

static esp_ble_adv_params_t spp_adv_params = {
    .adv_int_min = 0x20 /* 500/1.6 */,
    .adv_int_max = 0x40 /* 500/1.6 */,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = /* BLE_ADDR_TYPE_RPA_RANDOM, */ BLE_ADDR_TYPE_PUBLIC,
    .peer_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .peer_addr = {0xE4, 0x05, 0xF8, 0x0D, 0xB3, 0x9C},
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static esp_ble_adv_data_t heart_rate_scan_rsp_config = {
    .set_scan_rsp = true,
    .include_name = true,
    .manufacturer_len = sizeof(test_manufacturer),
    .p_manufacturer_data = test_manufacturer,
};

/*
Type: com.silabs.service.ota
UUID: 1D14D6EE-FD63-4FA1-BFA4-8F47B42119F0
*/
static uint8_t service_uuid[16] = {
    /* LSB
       <-------------------------------------------------------------------------------->
       MSB */
    // first uuid, 16bit, [12],[13] is the value
    // 0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00,
    // 0xFF, 0x00, 0x00, 0x00,
    0xf0, 0x19, 0x21, 0xb4, 0x47, 0x8f, 0xa4, 0xbf,
    0xa1, 0x4f, 0x63, 0xfd, 0xee, 0xd6, 0x14, 0x1d,
};

static uint8_t service_UserList_uuid[16] = {
    /* LSB
       <-------------------------------------------------------------------------------->
       MSB */
    // first uuid, 16bit, [12],[13] is the value
    // 0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00,
    // 0xFF, 0x00, 0x00, 0x00,
    0x3d, 0xf0, 0x6d, 0xe8, 0xab, 0x86, 0xa3, 0x8f,
    0x38, 0x45, 0x95, 0xcd, 0x45, 0xee, 0x3d, 0xe8,
};

static uint8_t service_Files_uuid[16] = {
    /* LSB
       <-------------------------------------------------------------------------------->
       MSB */
    // first uuid, 16bit, [12],[13] is the value
    // 0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00,
    // 0xFF, 0x00, 0x00, 0x00,
    0x7a, 0x55, 0x7e, 0x70, 0x11, 0xd7, 0xfc, 0xa9,
    0x46, 0x40, 0x39, 0x69, 0x8b, 0xed, 0xfb, 0xf0,
};

static uint8_t service_Routines_uuid[16] = {
    0x32, 0xb2, 0xfa, 0xdc, 0xd0, 0xc8, 0x9e, 0xa0,
    0x7b, 0x4a, 0xa0, 0x2b, 0xf8, 0x9c, 0x81, 0x00,
};

/*
Type: com.silabs.characteristic.ota_control
UUID: F7BF3564-FB6D-4E53-88A4-5E37E0326063
Silicon Labs OTA Control.
Property requirements:
    Notify - Excluded
    Read - Excluded
    Write Without Response - Excluded
    Write - Mandatory
    Reliable write - Excluded
    Indicate - Excluded
*/
static uint8_t char_ota_control_uuid[16] = {
    /* LSB
       <-------------------------------------------------------------------------------->
       MSB */
    // first uuid, 16bit, [12],[13] is the value
    // 0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00,
    // 0xFF, 0x00, 0x00, 0x00,
    0x63, 0x60, 0x32, 0xe0, 0x37, 0x5e, 0xa4, 0x88,
    0x53, 0x4e, 0x6d, 0xfb, 0x64, 0x35, 0xbf, 0xf7,
};

/*
Type: com.silabs.characteristic.ota_data
UUID: 984227F3-34FC-4045-A5D0-2C581F81A153
Silicon Labs OTA Data.
Property requirements:
    Notify - Excluded
    Read - Excluded
    Write Without Response - Mandatory
    Write - Mandatory
    Reliable write - Excluded
    Indicate - Excluded
*/
static uint8_t char_ota_data_uuid[16] = {
    /* LSB
       <-------------------------------------------------------------------------------->
       MSB */
    // first uuid, 16bit, [12],[13] is the value
    // 0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00,
    // 0xFF, 0x00, 0x00, 0x00,
    0x53, 0xa1, 0x81, 0x1f, 0x58, 0x2c, 0xd0, 0xa5,
    0x45, 0x40, 0xfc, 0x34, 0xf3, 0x27, 0x42, 0x98,
};

// static uint16_t spp_handle_table[SPP_IDX_NB];

/* static esp_ble_adv_params_t spp_adv_params = {
    .adv_int_min = 0x40,
    .adv_int_max = 0x80,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
}; */

struct gatts_profile_inst {
  esp_gatts_cb_t gatts_cb;
  uint16_t gatts_if;
  uint16_t app_id;
  uint16_t conn_id;
  uint16_t service_handle;
  esp_gatt_srvc_id_t service_id;
  uint16_t char_handle;
  esp_bt_uuid_t char_uuid;
  esp_gatt_perm_t perm;
  esp_gatt_char_prop_t property;
  uint16_t descr_handle;
  esp_bt_uuid_t descr_uuid;
};

typedef struct spp_receive_data_node {
  int32_t len;
  uint8_t *node_buff;
  struct spp_receive_data_node *next_node;
} spp_receive_data_node_t;

static spp_receive_data_node_t *temp_spp_recv_data_node_p1 = NULL;
static spp_receive_data_node_t *temp_spp_recv_data_node_p2 = NULL;

typedef struct spp_receive_data_buff {
  int32_t node_num;
  int32_t buff_size;
  spp_receive_data_node_t *first_node;
} spp_receive_data_buff_t;

static spp_receive_data_buff_t SppRecvDataBuff = {
    .node_num = 0, .buff_size = 0, .first_node = NULL};

static void gatts_profile_event_handler(esp_gatts_cb_event_t event,
                                        esp_gatt_if_t gatts_if,
                                        esp_ble_gatts_cb_param_t *param);

/* One gatt-based profile one app_id and one gatts_if, this array will store the
 * gatts_if returned by ESP_GATTS_REG_EVT */
static struct gatts_profile_inst spp_profile_tab[SPP_PROFILE_NUM] = {
    [SPP_PROFILE_APP_IDX] =
        {
            .gatts_cb = gatts_profile_event_handler,
            .gatts_if = ESP_GATT_IF_NONE, /* Not get the gatt_if, so initial is
                                             ESP_GATT_IF_NONE */
        },
};

/*
 *  SPP PROFILE ATTRIBUTES
 ****************************************************************************************
 */

#define CHAR_DECLARATION_SIZE (sizeof(uint8_t))
static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_client_config_uuid =
    ESP_GATT_UUID_CHAR_CLIENT_CONFIG;

static const uint8_t char_prop_read_notify =
    ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
// static const uint8_t char_prop_read_write = ESP_GATT_CHAR_PROP_BIT_WRITE |
// ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;

#ifdef SUPPORT_HEARTBEAT
static const uint8_t char_prop_read_write_notify =
    ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE_NR |
    ESP_GATT_CHAR_PROP_BIT_NOTIFY;
#endif

static esp_bd_addr_t spp_remote_bda = {
    0x0,
};

/// SPP Service - data receive characteristic, read&write without response
static const uint16_t spp_data_receive_uuid = ESP_GATT_UUID_SPP_DATA_RECEIVE;
static const uint8_t spp_data_receive_val[20] = {0x00};

/// SPP Service - data notify characteristic, notify&read
static const uint16_t spp_data_notify_uuid = ESP_GATT_UUID_SPP_DATA_NOTIFY;
// static const uint16_t spp_data_notify_userList_uuid =
// ESP_GATT_UUID_SPP_COMMAND_NOTIFY_USERLIST;
static const uint8_t spp_data_notify_val[200] = {0x00};
// static const uint8_t spp_data_notifyUserList_val[200] = {0x00};
static const uint8_t spp_data_notify_ccc[2] = {0x00, 0x00};

static const uint8_t char_value[4] = {0x11, 0x22, 0x33, 0x44};

/// SPP  USER LIST Service - data receive characteristic, read&write without
/// response
static const uint16_t spp_USERLIST_data_receive_uuid =
    ESP_GATT_UUID_SPP_USERLIST_DATA_RECEIVE;
static const uint8_t spp_USERLIST_data_receive_val[20] = {0x00};
/// SPP Service - data notify characteristic, notify&read
static const uint16_t spp_USERLIST_data_notify_uuid =
    ESP_GATT_UUID_SPP_USERLIST_DATA_NOTIFY;
// static const uint16_t spp_data_notify_userList_uuid =
// ESP_GATT_UUID_SPP_COMMAND_NOTIFY_USERLIST;
static const uint8_t spp_USERLIST_data_notify_val[200] = {0x00};
// static const uint8_t spp_data_notifyUserList_val[200] = {0x00};
static const uint8_t spp_USERLIST_data_notify_ccc[2] = {0x00, 0x00};

static const uint16_t spp_FILES_data_receive_uuid =
    ESP_GATT_UUID_SPP_FILES_DATA_RECEIVE;
static const uint8_t spp_FILES_data_receive_val[20] = {0x00};
/// SPP Service - data notify characteristic, notify&read
static const uint16_t spp_FILES_data_notify_uuid =
    ESP_GATT_UUID_SPP_FILES_DATA_NOTIFY;
// static const uint16_t spp_data_notify_userList_uuid =
// ESP_GATT_UUID_SPP_COMMAND_NOTIFY_USERLIST;
static const uint8_t spp_FILES_data_notify_val[200] = {0x00};
// static const uint8_t spp_data_notifyUserList_val[200] = {0x00};
static const uint8_t spp_FILES_data_notify_ccc[2] = {0x00, 0x00};

static const uint16_t spp_ROUTINES_data_receive_uuid =
    ESP_GATT_UUID_SPP_ROUTINES_DATA_RECEIVE;
static const uint8_t spp_ROUTINES_data_receive_val[20] = {0x00};
/// SPP Service - data notify characteristic, notify&read
static const uint16_t spp_ROUTINES_data_notify_uuid =
    ESP_GATT_UUID_SPP_ROUTINES_DATA_NOTIFY;
// static const uint16_t spp_data_notify_userList_uuid =
// ESP_GATT_UUID_SPP_COMMAND_NOTIFY_USERLIST;
static const uint8_t spp_ROUTINES_data_notify_val[200] = {0x00};
// static const uint8_t spp_data_notifyUserList_val[200] = {0x00};
static const uint8_t spp_ROUTINES_data_notify_ccc[2] = {0x00, 0x00};

static const uint8_t char_prop_write_writenorsp =
    ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_WRITE_NR;

static bool enable_data_ntf = false;
static bool is_connected = false;

static uint16_t spp_mtu_size = 512;
static uint16_t spp_conn_id = 0xffff;
static esp_gatt_if_t spp_gatts_if = 0xff;
QueueHandle_t spp_uart_queue = NULL;
static QueueHandle_t cmd_cmd_queue = NULL;

/*///SPP Service - command characteristic, read&write without response
static const uint16_t spp_command_uuid = ESP_GATT_UUID_SPP_COMMAND_RECEIVE;
static const uint8_t  spp_command_val[10] = {0x00};*/

/*///SPP Service - status characteristic, notify&read
static const uint16_t spp_status_uuid = ESP_GATT_UUID_SPP_COMMAND_NOTIFY;
static const uint8_t  spp_status_val[10] = {0x00};
static const uint8_t  spp_status_ccc[2] = {0x00, 0x00};*/

#ifdef SUPPORT_HEARTBEAT
/// SPP Server - Heart beat characteristic, notify&write&read
static const uint16_t spp_heart_beat_uuid = ESP_GATT_UUID_SPP_HEARTBEAT;
static const uint8_t spp_heart_beat_val[2] = {0x00, 0x00};
static const uint8_t spp_heart_beat_ccc[2] = {0x00, 0x00};
#endif

/// Full HRS Database Description - Used to add attributes into the database
static const esp_gatts_attr_db_t spp_gatt_db[SPP_IDX_NB] = {
    // SPP -  Service Declaration
    [SPP_IDX_SVC] = {{ESP_GATT_AUTO_RSP},
                     {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid,
                      ESP_GATT_PERM_READ, sizeof(spp_service_uuid),
                      sizeof(spp_service_uuid), (uint8_t *)&spp_service_uuid}},

    // SPP -  data receive characteristic Declaration
    [SPP_IDX_SPP_DATA_RECV_CHAR] =
        {{ESP_GATT_AUTO_RSP},
         {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid,
          ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE,
          (uint8_t *)&char_prop_write_writenorsp /* &char_prop_read_write */}},

    // SPP -  data receive characteristic Value
    [SPP_IDX_SPP_DATA_RECV_VAL] =
        {{ESP_GATT_AUTO_RSP},
         {ESP_UUID_LEN_16, (uint8_t *)&spp_data_receive_uuid,
          ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, SPP_DATA_MAX_LEN,
          sizeof(spp_data_receive_val), (uint8_t *)spp_data_receive_val}},

    // SPP -  data notify characteristic Declaration
    [SPP_IDX_SPP_DATA_NOTIFY_CHAR] =
        {{ESP_GATT_AUTO_RSP},
         {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid,
          ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE,
          (uint8_t *)&char_prop_read_notify}},

    // SPP -  data notify characteristic Value
    [SPP_IDX_SPP_DATA_NTY_VAL] = {{ESP_GATT_AUTO_RSP},
                                  {ESP_UUID_LEN_16,
                                   (uint8_t *)&spp_data_notify_uuid,
                                   ESP_GATT_PERM_READ, SPP_DATA_MAX_LEN,
                                   sizeof(spp_data_notify_val),
                                   (uint8_t *)spp_data_notify_val}},

    // SPP -  data notify characteristic - Client Characteristic Configuration
    // Descriptor
    [SPP_IDX_SPP_DATA_NTF_CFG] =
        {{ESP_GATT_AUTO_RSP},
         {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid,
          ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, sizeof(uint16_t),
          sizeof(spp_data_notify_ccc), (uint8_t *)spp_data_notify_ccc}},

};

static const esp_gatts_attr_db_t spp_userList_gatt_db[SPP_USERLIST_NB] = {
    // SPP -  Service Declaration
    [SPP_USERLIST_SVC] = {{ESP_GATT_AUTO_RSP},
                          {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid,
                           ESP_GATT_PERM_READ,
                           sizeof(spp_UserList_service_uuid),
                           sizeof(spp_UserList_service_uuid),
                           (uint8_t *)&spp_UserList_service_uuid}},

    // SPP -  data receive characteristic Declaration
    [SPP_USERLIST_SPP_DATA_RECV_CHAR] =
        {{ESP_GATT_AUTO_RSP},
         {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid,
          ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE,
          (uint8_t *)&char_prop_write_writenorsp}},

    // SPP -  data receive characteristic Value
    [SPP_USERLIST_SPP_DATA_RECV_VAL] =
        {{ESP_GATT_AUTO_RSP},
         {ESP_UUID_LEN_16, (uint8_t *)&spp_USERLIST_data_receive_uuid,
          ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, SPP_DATA_MAX_LEN,
          sizeof(spp_USERLIST_data_receive_val),
          (uint8_t *)spp_USERLIST_data_receive_val}},

    // SPP -  data notify characteristic Declaration
    [SPP_USERLIST_SPP_DATA_NOTIFY_CHAR] =
        {{ESP_GATT_AUTO_RSP},
         {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid,
          ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE,
          (uint8_t *)&char_prop_read_notify}},

    // SPP -  data notify characteristic Value
    [SPP_USERLIST_SPP_DATA_NTY_VAL] =
        {{ESP_GATT_AUTO_RSP},
         {ESP_UUID_LEN_16, (uint8_t *)&spp_USERLIST_data_notify_uuid,
          ESP_GATT_PERM_READ, SPP_DATA_MAX_LEN,
          sizeof(spp_USERLIST_data_notify_val),
          (uint8_t *)spp_USERLIST_data_notify_val}},

    // SPP -  data notify characteristic - Client Characteristic Configuration
    // Descriptor
    [SPP_USERLIST_SPP_DATA_NTF_CFG] =
        {{ESP_GATT_AUTO_RSP},
         {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid,
          ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, sizeof(uint16_t),
          sizeof(spp_USERLIST_data_notify_ccc),
          (uint8_t *)spp_USERLIST_data_notify_ccc}},

};

static const esp_gatts_attr_db_t spp_routines_gatt_db[SPP_ROUTINES_NB] = {
    // SPP -  Service Declaration
    [SPP_ROUTINES_SVC] = {{ESP_GATT_AUTO_RSP},
                          {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid,
                           ESP_GATT_PERM_READ, sizeof(service_Routines_uuid),
                           sizeof(spp_Routines_service_uuid),
                           (uint8_t *)&spp_Routines_service_uuid}},

    // SPP -  data receive characteristic Declaration
    [SPP_ROUTINES_SPP_DATA_RECV_CHAR] =
        {{ESP_GATT_AUTO_RSP},
         {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid,
          ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE,
          (uint8_t *)&char_prop_write_writenorsp}},

    // SPP -  data receive characteristic Value
    [SPP_ROUTINES_SPP_DATA_RECV_VAL] =
        {{ESP_GATT_AUTO_RSP},
         {ESP_UUID_LEN_16, (uint8_t *)&spp_ROUTINES_data_receive_uuid,
          ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, SPP_DATA_MAX_LEN,
          sizeof(spp_ROUTINES_data_receive_val),
          (uint8_t *)spp_ROUTINES_data_receive_val}},

    // SPP -  data notify characteristic Declaration
    [SPP_ROUTINES_SPP_DATA_NOTIFY_CHAR] =
        {{ESP_GATT_AUTO_RSP},
         {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid,
          ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE,
          (uint8_t *)&char_prop_read_notify}},

    // SPP -  data notify characteristic Value
    [SPP_ROUTINES_SPP_DATA_NTY_VAL] =
        {{ESP_GATT_AUTO_RSP},
         {ESP_UUID_LEN_16, (uint8_t *)&spp_ROUTINES_data_notify_uuid,
          ESP_GATT_PERM_READ, SPP_DATA_MAX_LEN,
          sizeof(spp_ROUTINES_data_notify_val),
          (uint8_t *)spp_ROUTINES_data_notify_val}},

    // SPP -  data notify characteristic - Client Characteristic Configuration
    // Descriptor
    [SPP_ROUTINES_SPP_DATA_NTF_CFG] =
        {{ESP_GATT_AUTO_RSP},
         {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid,
          ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, sizeof(uint16_t),
          sizeof(spp_ROUTINES_data_notify_ccc),
          (uint8_t *)spp_ROUTINES_data_notify_ccc}},

};

static const esp_gatts_attr_db_t spp_files_gatt_db[SPP_FILES_NB] = {
    // SPP -  Service Declaration
    [SPP_FILES_SVC] = {{ESP_GATT_AUTO_RSP},
                       {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid,
                        ESP_GATT_PERM_READ, sizeof(spp_Files_service_uuid),
                        sizeof(spp_Files_service_uuid),
                        (uint8_t *)&spp_Files_service_uuid}},

    // SPP -  data receive characteristic Declaration
    [SPP_FILES_SPP_DATA_RECV_CHAR] =
        {{ESP_GATT_AUTO_RSP},
         {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid,
          ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE,
          (uint8_t *)&char_prop_write_writenorsp}},

    // SPP -  data receive characteristic Value
    [SPP_FILES_SPP_DATA_RECV_VAL] = {{ESP_GATT_AUTO_RSP},
                                     {ESP_UUID_LEN_16,
                                      (uint8_t *)&spp_FILES_data_receive_uuid,
                                      ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                      SPP_DATA_MAX_LEN,
                                      sizeof(spp_FILES_data_receive_val),
                                      (uint8_t *)spp_FILES_data_receive_val}},

    // SPP -  data notify characteristic Declaration
    [SPP_FILES_SPP_DATA_NOTIFY_CHAR] =
        {{ESP_GATT_AUTO_RSP},
         {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid,
          ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE,
          (uint8_t *)&char_prop_read_notify}},

    // SPP -  data notify characteristic Value
    [SPP_FILES_SPP_DATA_NTY_VAL] = {{ESP_GATT_AUTO_RSP},
                                    {ESP_UUID_LEN_16,
                                     (uint8_t *)&spp_FILES_data_notify_uuid,
                                     ESP_GATT_PERM_READ, SPP_DATA_MAX_LEN,
                                     sizeof(spp_FILES_data_notify_val),
                                     (uint8_t *)spp_FILES_data_notify_val}},

    // SPP -  data notify characteristic - Client Characteristic Configuration
    // Descriptor
    [SPP_FILES_SPP_DATA_NTF_CFG] = {{ESP_GATT_AUTO_RSP},
                                    {ESP_UUID_LEN_16,
                                     (uint8_t *)&character_client_config_uuid,
                                     ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                     sizeof(uint16_t),
                                     sizeof(spp_FILES_data_notify_ccc),
                                     (uint8_t *)spp_FILES_data_notify_ccc}},

};

static const esp_gatts_attr_db_t gatt_db[HRS_IDX_NB] = {
    // Service Declaration
    [IDX_SVC] = {{ESP_GATT_AUTO_RSP},
                 {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid,
                  ESP_GATT_PERM_READ, sizeof(service_uuid),
                  sizeof(service_uuid), (uint8_t *)&service_uuid}},

    /* Characteristic Declaration */
    [IDX_CHAR_A] = {{ESP_GATT_AUTO_RSP},
                    {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid,
                     ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE,
                     CHAR_DECLARATION_SIZE,
                     (uint8_t *)&char_prop_write_writenorsp}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_A] = {{ESP_GATT_AUTO_RSP},
                        {ESP_UUID_LEN_128, (uint8_t *)&char_ota_control_uuid,
                         ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                         GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_value),
                         (uint8_t *)char_value}},

    /* Characteristic Declaration */
    [IDX_CHAR_B] = {{ESP_GATT_AUTO_RSP},
                    {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid,
                     ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE,
                     CHAR_DECLARATION_SIZE,
                     (uint8_t *)&char_prop_write_writenorsp}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_B] = {{ESP_GATT_AUTO_RSP},
                        {ESP_UUID_LEN_128, (uint8_t *)&char_ota_data_uuid,
                         ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                         GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_value),
                         (uint8_t *)char_value}},

};

static uint8_t find_char_and_desr_index(uint16_t handle) {
  uint8_t error = 0xff;

  for (int i = 0; i < SPP_IDX_NB; i++) {
    if (handle == spp_handle_table[i]) {
      return i;
    }
  }

  return error;
}

void adv() { esp_ble_gap_start_advertising(&spp_adv_params); }

void example_prepare_write_event_env(esp_gatt_if_t gatts_if,
                                     prepare_type_env_t *prepare_write_env,
                                     esp_ble_gatts_cb_param_t *param) {
  // ESP_LOGI(GATTS_TABLE_TAG, "prepare write, handle = %d, value len = %d",
  // param->write.handle, param->write.len);
  esp_gatt_status_t status = ESP_GATT_OK;
  if (prepare_write_env->prepare_buf == NULL) {
    prepare_write_env->prepare_buf =
        (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE * sizeof(uint8_t));
    prepare_write_env->prepare_len = 0;
    if (prepare_write_env->prepare_buf == NULL) {
      // ESP_LOGE(GATTS_TABLE_TAG, "%s, Gatt_server prep no mem", __func__);
      status = ESP_GATT_NO_RESOURCES;
    }
  } else {
    if (param->write.offset > PREPARE_BUF_MAX_SIZE) {
      status = ESP_GATT_INVALID_OFFSET;
    } else if ((param->write.offset + param->write.len) >
               PREPARE_BUF_MAX_SIZE) {
      status = ESP_GATT_INVALID_ATTR_LEN;
    }
  }
  /*send response when param->write.need_rsp is true */
  if (param->write.need_rsp) {
    esp_gatt_rsp_t *gatt_rsp = (esp_gatt_rsp_t *)malloc(sizeof(esp_gatt_rsp_t));
    if (gatt_rsp != NULL) {
      gatt_rsp->attr_value.len = param->write.len;
      gatt_rsp->attr_value.handle = param->write.handle;
      gatt_rsp->attr_value.offset = param->write.offset;
      gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
      memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);
      esp_err_t response_err =
          esp_ble_gatts_send_response(gatts_if, param->write.conn_id,
                                      param->write.trans_id, status, gatt_rsp);
      if (response_err != ESP_OK) {
        // ESP_LOGE(GATTS_TABLE_TAG, "Send response error");
      }
      free(gatt_rsp);
    } else {
      // ESP_LOGE(GATTS_TABLE_TAG, "%s, malloc failed", __func__);
    }
  }
  if (status != ESP_GATT_OK) {
    return;
  }
  memcpy(prepare_write_env->prepare_buf + param->write.offset,
         param->write.value, param->write.len);
  prepare_write_env->prepare_len += param->write.len;
}

void example_exec_write_event_env(prepare_type_env_t *prepare_write_env,
                                  esp_ble_gatts_cb_param_t *param) {
  if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC &&
      prepare_write_env->prepare_buf) {
    esp_log_buffer_hex(GATTS_TABLE_TAG, prepare_write_env->prepare_buf,
                       prepare_write_env->prepare_len);
  } else {
    // ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATT_PREP_WRITE_CANCEL");
  }
  if (prepare_write_env->prepare_buf) {
    free(prepare_write_env->prepare_buf);
    prepare_write_env->prepare_buf = NULL;
  }
  prepare_write_env->prepare_len = 0;
}

static bool store_wr_buffer(esp_ble_gatts_cb_param_t *p_data) {
  temp_spp_recv_data_node_p1 =
      (spp_receive_data_node_t *)malloc(sizeof(spp_receive_data_node_t));

  if (temp_spp_recv_data_node_p1 == NULL) {
    // ESP_LOGI(GATTS_TABLE_TAG, "malloc error %s %d\n", __func__, __LINE__);
    return false;
  }
  if (temp_spp_recv_data_node_p2 != NULL) {
    temp_spp_recv_data_node_p2->next_node = temp_spp_recv_data_node_p1;
  }
  temp_spp_recv_data_node_p1->len = p_data->write.len;
  SppRecvDataBuff.buff_size += p_data->write.len;
  temp_spp_recv_data_node_p1->next_node = NULL;
  temp_spp_recv_data_node_p1->node_buff = (uint8_t *)malloc(p_data->write.len);
  temp_spp_recv_data_node_p2 = temp_spp_recv_data_node_p1;
  memcpy(temp_spp_recv_data_node_p1->node_buff, p_data->write.value,
         p_data->write.len);
  if (SppRecvDataBuff.node_num == 0) {
    SppRecvDataBuff.first_node = temp_spp_recv_data_node_p1;
    SppRecvDataBuff.node_num++;
  } else {
    SppRecvDataBuff.node_num++;
  }

  return true;
}

static void free_write_buffer(void) {
  temp_spp_recv_data_node_p1 = SppRecvDataBuff.first_node;

  while (temp_spp_recv_data_node_p1 != NULL) {
    temp_spp_recv_data_node_p2 = temp_spp_recv_data_node_p1->next_node;
    free(temp_spp_recv_data_node_p1->node_buff);
    free(temp_spp_recv_data_node_p1);
    temp_spp_recv_data_node_p1 = temp_spp_recv_data_node_p2;
  }

  SppRecvDataBuff.node_num = 0;
  SppRecvDataBuff.buff_size = 0;
  SppRecvDataBuff.first_node = NULL;
}

static void print_write_buffer(void) {
  temp_spp_recv_data_node_p1 = SppRecvDataBuff.first_node;

  while (temp_spp_recv_data_node_p1 != NULL) {
    uart_write_bytes(UART_NUM_0,
                     (char *)(temp_spp_recv_data_node_p1->node_buff),
                     temp_spp_recv_data_node_p1->len);
    temp_spp_recv_data_node_p1 = temp_spp_recv_data_node_p1->next_node;
  }
}

static void gap_event_handler(esp_gap_ble_cb_event_t event,
                              esp_ble_gap_cb_param_t *param) {
  esp_err_t err;
  // ESP_LOGE(GATTS_TABLE_TAG, "GAP_EVT, event %d\n", event);

  switch (event) {
  case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
    adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
    if (adv_config_done == 0) {
      esp_ble_gap_start_advertising(&spp_adv_params);
    }
    break;
  case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
    adv_config_done &= (~ADV_CONFIG_FLAG);
    if (adv_config_done == 0) {
      esp_ble_gap_start_advertising(&spp_adv_params);
    }
    break;
  case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
    esp_ble_gap_start_advertising(&spp_adv_params);
    break;
  case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
    // advertising start complete event to indicate advertising start
    // successfully or failed
    if ((err = param->adv_start_cmpl.status) != ESP_BT_STATUS_SUCCESS) {
      // ESP_LOGE(GATTS_TABLE_TAG, "Advertising start failed: %s\n",
      // esp_err_to_name(err));
    }
    break;
  case ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT:
    if (param->local_privacy_cmpl.status != ESP_BT_STATUS_SUCCESS) {
      // ESP_LOGE(GATTS_TABLE_TAG, "config local privacy failed, error status =
      // %x", param->local_privacy_cmpl.status);
      break;
    }

    esp_err_t ret = esp_ble_gap_config_adv_data(&spp_adv_config);
    if (ret) {
      // ESP_LOGE(GATTS_TABLE_TAG, "config adv data failed, error code = %x",
      // ret);
    } else {
      adv_config_done |= ADV_CONFIG_FLAG;
    }

    ret = esp_ble_gap_config_adv_data(&heart_rate_scan_rsp_config);
    if (ret) {
      // ESP_LOGE(GATTS_TABLE_TAG, "config adv data failed, error code = %x",
      // ret);
    } else {
      adv_config_done |= SCAN_RSP_CONFIG_FLAG;
    }

    break;
  default:
    break;
  }
}

char ble_Name[20] = {};
int CountBLEWrite = 0;
esp_ota_handle_t update_handle = 0;
const esp_partition_t *update_partition = NULL;

static void gatts_profile_event_handler(esp_gatts_cb_event_t event,
                                        esp_gatt_if_t gatts_if,
                                        esp_ble_gatts_cb_param_t *param) {
  esp_ble_gatts_cb_param_t *p_data = (esp_ble_gatts_cb_param_t *)param;
  uint8_t res = 0xff;

  // ESP_LOGI(GATTS_TABLE_TAG, "event = %x\n", event);
  switch (event) {
  case ESP_GATTS_REG_EVT:
    ESP_LOGI(GATTS_TABLE_TAG, "%s %d\n", __func__, __LINE__);

    // size_t required_size = 0;
    // char BLE_Device_Name[50];

    /* memset(BLE_Device_Name, 0, sizeof(BLE_Device_Name));

    if (nvs_get_str(nvs_System_handle, key, NULL, &required_size) == ESP_OK)
    {
        //////printf("\nrequire size %d\n", required_size);
        //////printf("\nGET USERS NAMESPACE\n");
        if (nvs_get_str(nvs_System_handle, NVS_KEY_BLE_NAME, BLE_Device_Name,
    &required_size) == ESP_OK)
        {
            //////printf("\naux get str feedback %s\n",
    aux_Get_Feedback_User_str);
        }
        else
        {
            sprintf(BLE_Device_Name, "%s", "MOTORLINE M200");
        }
    }
    else
    {
        sprintf(BLE_Device_Name, "%s", "MOTORLINE M200");
    } */

    // ////printf("\n\n device name - %s, %d\n\n", ble_Name, gatts_if);

    // esp_ble_gap_set_device_name("MOTORLINE");
    char data[100] = {};

    if (get_STR_Data_In_Storage(NVS_KEY_BLE_NAME, nvs_System_handle, &data) ==
        ESP_OK) {
      char rsp_BLE_Name[31] = {};
      change_BLE_Name(data, &rsp_BLE_Name);

      if (save_STR_Data_In_Storage(NVS_KEY_BLE_NAME, data, nvs_System_handle) !=
          ESP_OK) {
        change_BLE_Name(DEFAULT_BLE_NAME, &rsp_BLE_Name);
        save_STR_Data_In_Storage(NVS_KEY_BLE_NAME, DEFAULT_BLE_NAME,
                                 nvs_System_handle);
      }
    } else {
      char rsp_BLE_Name[31] = {};
      change_BLE_Name(DEFAULT_BLE_NAME, &rsp_BLE_Name);
      save_STR_Data_In_Storage(NVS_KEY_BLE_NAME, DEFAULT_BLE_NAME,
                               nvs_System_handle);
    }

    // esp_ble_gap_config_adv_data_raw((uint8_t *)spp_adv_data,
    // sizeof(spp_adv_data));
    esp_ble_gap_config_local_privacy(true);
    // esp_ble_gap_config_adv_data(&spp_adv_config);

    // ESP_LOGI(GATTS_TABLE_TAG, "%s %d\n", __func__, __LINE__);
    esp_ble_gatts_create_attr_tab(spp_gatt_db, gatts_if, SPP_IDX_NB,
                                  SPP_SVC_INST_ID);

    // ESP_LOGI(GATTS_TABLE_TAG, "%s %d\n", __func__, __LINE__);
    esp_ble_gatts_create_attr_tab(spp_userList_gatt_db, gatts_if,
                                  SPP_USERLIST_NB, SPP_SVC_INST_ID);

    esp_ble_gatts_create_attr_tab(spp_files_gatt_db, gatts_if, SPP_FILES_NB,
                                  SPP_SVC_INST_ID);

    // ESP_LOGI(GATTS_TABLE_TAG, "%s %d\n", __func__, __LINE__);
    esp_ble_gatts_create_attr_tab(spp_routines_gatt_db, gatts_if,
                                  SPP_ROUTINES_NB, SPP_SVC_INST_ID);

    esp_err_t create_attr_ret = esp_ble_gatts_create_attr_tab(
        gatt_db, gatts_if, HRS_IDX_NB, SPP_SVC_INST_ID);
    if (create_attr_ret) {
      // ESP_LOGE(GATTS_TABLE_TAG, "create attr table failed, error code = %x",
      // create_attr_ret);
    }
    break;
  case ESP_GATTS_READ_EVT:
    res = find_char_and_desr_index(p_data->read.handle);
    if (res == SPP_IDX_SPP_STATUS_VAL) {
    }
    break;
  case ESP_GATTS_WRITE_EVT: {
    // if (label_timerVerifySystem != 1)
    // {
    //
    esp_err_t err;
    CountBLEWrite++;
    // ////printf("\n\nCountBLEWrite %d\n\n", CountBLEWrite);
    //   //////printf("aram->write.handle %d\n", param->write.handle);
    //   //////printf("spp_handle_table[SPP_IDX_SVC] %d\n",
    //   spp_handle_table[SPP_IDX_SVC]);
    //   //////printf("spp_handle_table[SPP_IDX_NB - 1] %d\n",
    //   spp_handle_table[SPP_IDX_NB - 1]);
    // ESP_LOGI(GATTS_TABLE_TAG, "GATT_WRITE_EVT, handle = %d, value len = %d,
    // value :", param->write.handle, param->write.len);
    // //ESP_LOGI(GATTS_TABLE_TAG, "spp_handle_table[SPP_USERLIST_SVC] %d,
    // spp_handle_table[SPP_USERLIST_NB] = %d, value :",
    // spp_handle_table[SPP_IDX_SVC], spp_handle_table[SPP_IDX_NB]);
    // //ESP_LOGI(GATTS_TABLE_TAG, "spp_UserList_handle_table[SPP_ROUTINES_SVC]
    // %d, spp_UserList_handle_table[SPP_ROUTINES_NB] = %d, value :",
    // spp_Routines_handle_table[SPP_ROUTINES_SVC],
    // spp_Routines_handle_table[SPP_ROUTINES_NB]);

    if (param->write.handle >= heart_rate_handle_table[IDX_SVC] &&
        param->write.handle <= heart_rate_handle_table[HRS_IDX_NB - 1]) {
      // ////printf("after output data999 %s \n", (char *)p_data->write.value);
      if (!param->write.is_prep) {
        // the data length of gattc write  must be less than
        // GATTS_DEMO_CHAR_VAL_LEN_MAX.
        // ESP_LOGI(GATTS_TABLE_TAG, "GATT_WRITE_EVT, handle = %d, value len =
        // %d, value :", param->write.handle, param->write.len);
        // esp_log_buffer_hex(GATTS_TABLE_TAG, param->write.value,
        // param->write.len);
        // esp_reset();
        if (heart_rate_handle_table[IDX_CHAR_VAL_A] == param->write.handle &&
            param->write.len == 1) {
          uint8_t value = param->write.value[0];
          // ESP_LOGI(GATTS_TABLE_TAG, "ota-control = %d", value);
          if (0x00 == value) {
            // ESP_LOGI(GATTS_TABLE_TAG, "======beginota======");
            // esp_vfs_spiffs_unregister(NULL);
            update_partition = esp_ota_get_next_update_partition(NULL);
            // ESP_LOGI(GATTS_TABLE_TAG, "Writing to partition subtype %d at
            // offset 0x%x",                        update_partition->subtype,
            // update_partition->address);
            assert(update_partition != NULL);
            err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES,
                                &update_handle);
            if (err != ESP_OK) {

              esp_ble_gatts_send_indicate(
                  gatts_if, param->write.conn_id,
                  spp_UserList_handle_table[SPP_USERLIST_SPP_DATA_NTY_VAL],
                  strlen("FOTA ERROR"), (uint8_t *)"FOTA ERROR", true);
              // ESP_LOGE(GATTS_TABLE_TAG, "esp_ota_begin failed (%s)",
              // esp_err_to_name(err));
              esp_ota_abort(update_handle);
            }
          } else if (0x03 == value) {
            // ESP_LOGI(GATTS_TABLE_TAG, "======endota======");
            err = esp_ota_end(update_handle);
            if (err != ESP_OK) {
              if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
                // ESP_LOGE(GATTS_TABLE_TAG, "Image validation failed, image is
                // corrupted");
              }

              // ESP_LOGE(GATTS_TABLE_TAG, "esp_ota_end failed (%s)!",
              // esp_err_to_name(err));
              esp_ble_gatts_send_indicate(
                  gatts_if, param->write.conn_id,
                  spp_UserList_handle_table[SPP_USERLIST_SPP_DATA_NTY_VAL],
                  strlen("FOTA ERROR"), (uint8_t *)"FOTA ERROR", true);
            }

            err = esp_ota_set_boot_partition(update_partition);
            if (err != ESP_OK) {

              esp_ble_gatts_send_indicate(
                  gatts_if, param->write.conn_id,
                  spp_UserList_handle_table[SPP_USERLIST_SPP_DATA_NTY_VAL],
                  strlen("FOTA ERROR"), (uint8_t *)"FOTA ERROR", true);
              // ESP_LOGE(GATTS_TABLE_TAG, "esp_ota_set_boot_partition failed
              // (%s)!", esp_err_to_name(err));
            }

            // ESP_LOGI(GATTS_TABLE_TAG, "Prepare to restart system!");

            vTaskDelay(pdMS_TO_TICKS(1500));
            esp_restart();
            return;
          }
        }

        if (heart_rate_handle_table[IDX_CHAR_VAL_B] == param->write.handle) {
          uint16_t length =
              param->write
                  .len; // modify uint8_t to uint16_t when mtu larger than 255
          // ESP_LOGI(GATTS_TABLE_TAG, "ota-data = %d", length);
          err = esp_ota_write(update_handle, (const void *)param->write.value,
                              length);
          if (err != ESP_OK) {
            esp_ble_gatts_send_indicate(
                gatts_if, p_data->write.conn_id,
                spp_UserList_handle_table[SPP_IDX_SPP_DATA_NTY_VAL],
                strlen("FOTA ERROR"), (uint8_t *)"FOTA ERROR", true);
            esp_ota_abort(update_handle);
            // ESP_LOGI(GATTS_TABLE_TAG, "esp_ota_write error!");
          }
        }

        /* send response when param->write.need_rsp is true*/
        if (param->write.need_rsp) {
          // ////printf("\nota send rsp 1\n");
          esp_ble_gatts_send_response(gatts_if, param->write.conn_id,
                                      param->write.trans_id, ESP_GATT_OK, NULL);
        }
      } else {
        /* handle prepare write */
        example_prepare_write_event_env(gatts_if, &prepare_write_env, param);
      }
    } else if (param->write.handle >=
                   spp_UserList_handle_table[SPP_USERLIST_SVC] &&
               param->write.handle <=
                   (spp_UserList_handle_table[SPP_USERLIST_SVC] +
                    (SPP_USERLIST_NB - 1))) {

      // ////printf("p_data->write.value user list %s len=%d\n",
      // p_data->write.value, p_data->write.len);
      // ////printf("spp_gatts_if %d - p_data->write.conn_id %d\n",
      // spp_gatts_if, p_data->write.conn_id);
      //   char *x1;

      // ////printf("after output data %s \n", (char *)p_data->write.value);
      //   asprintf(&x1, "%s", p_data->write.value);
      // ////printf("after output data22 %s \n", (char *)p_data->write.value);
      //  //  parseInputData(x1,BLE_INDICATION);
      //  //  readAllUser_Label = 0;
      //   if (strlen((char *)p_data->write.value) > 2)
      //  {
      /* code */

      if (strlen((char *)p_data->write.value) > 5) {
        char *output_Data;
        output_Data = parseInputData(
            p_data->write.value, BLE_INDICATION, spp_gatts_if,
            p_data->write.conn_id,
            spp_UserList_handle_table[SPP_USERLIST_SPP_DATA_NTY_VAL], NULL,
            NULL);

        // free(x1);
        // ////printf("after output data33x \n");

        if (strstr(output_Data, "NTRSP") == NULL && strlen(output_Data) > 5) {
          // ////printf("\nmemcpy SPP_USERLIST_SPP_DATA_NTY_VAL 11212 %s\n",
          // output_Data);
          esp_ble_gatts_send_indicate(
              spp_gatts_if, p_data->write.conn_id,
              spp_handle_table[SPP_USERLIST_SPP_DATA_NTY_VAL],
              strlen(output_Data), (uint8_t *)output_Data, false);
          // ////printf("\nmemcpy SPP_USERLIST_SPP_DATA_NTY_VAL 32332323 %s\n",
          // output_Data);
        }
        free(output_Data);
      }
      break;
    } else if (param->write.handle >= spp_Files_handle_table[SPP_FILES_SVC] &&
               param->write.handle <= (spp_Files_handle_table[SPP_FILES_SVC] +
                                       (SPP_FILES_NB - 1))) {

      char *output_Data;
      // ////printf("after output data %s \n", (char *)p_data->write.value);
      //   asprintf(&x1, "%s", p_data->write.value);
      // ////printf("after output data22 %s \n", (char *)p_data->write.value);
      //   //  parseInputData(x1,BLE_INDICATION);
      //   //  readAllUser_Label = 0;
      // ////printf("after output BLE FILES %s \n", (char
      // *)p_data->write.value);
      if (strlen((char *)p_data->write.value) > 2) {
        /* code */

        output_Data = parseInputData(
            p_data->write.value, BLE_INDICATION, spp_gatts_if,
            p_data->write.conn_id,
            spp_Files_handle_table[SPP_FILES_SPP_DATA_NTY_VAL], NULL, NULL);

        res = find_char_and_desr_index(p_data->write.handle);

        free(output_Data);
        // free(p_data->write.value);
      }
      break;
    } else if (param->write.handle >=
                   spp_Routines_handle_table[SPP_ROUTINES_SVC] &&
               param->write.handle <=
                   (spp_Routines_handle_table[SPP_ROUTINES_SVC] +
                    (SPP_ROUTINES_NB - 1))) {

      char *output_Data;
      // ////printf("after output data %s \n", (char *)p_data->write.value);
      //   asprintf(&x1, "%s", p_data->write.value);
      // ////printf("after output data22 %s \n", (char *)p_data->write.value);
      //   //  parseInputData(x1,BLE_INDICATION);
      //   //  readAllUser_Label = 0;
      if (strlen((char *)p_data->write.value) > 2) {
        /* code */

        output_Data = parseInputData(
            p_data->write.value, BLE_INDICATION, spp_gatts_if,
            p_data->write.conn_id,
            spp_Routines_handle_table[SPP_ROUTINES_SPP_DATA_NTY_VAL], NULL,
            NULL);

        if (strstr(output_Data, "NTRSP") == NULL) {
          esp_ble_gatts_send_indicate(
              spp_gatts_if, p_data->write.conn_id,
              spp_Routines_handle_table[SPP_ROUTINES_SPP_DATA_NTY_VAL],
              strlen(output_Data), (uint8_t *)output_Data, false);
        }

        /* esp_ble_gatts_send_indicate(spp_gatts_if, p_data->write.conn_id,
         * spp_Routines_handle_table[SPP_ROUTINES_SPP_DATA_NTY_VAL],
         * strlen(output_Data), (uint8_t *)output_Data, false); */

        res = find_char_and_desr_index(p_data->write.handle);

        // ////printf("esp 00 nn\n");
        //   free(x1);
        // ////printf("esp 11 nn\n");
        //   free(p_data->write.value);
        free(output_Data);
      }
      break;
    } else {
      //     if (param->write.handle >= spp_handle_table[SPP_IDX_SVC] &&
      //     param->write.handle <= spp_handle_table[SPP_IDX_NB - 1])
      // {
      // ////printf("p_data->write NORMAL\n");
      if (!p_data->write.is_prep) {

        char *output_Data;

        if (strlen((char *)p_data->write.value) > 6) {

          printf("\n write len11 %s - %d\n", (char *)p_data->write.value,
          strlen((char *)p_data->write.value));
          //   ESP_ERROR_CHECK(heap_trace_start(HEAP_TRACE_LEAKS));
          output_Data = parseInputData(
              p_data->write.value, BLE_INDICATION, spp_gatts_if,
              p_data->write.conn_id, spp_handle_table[SPP_IDX_SPP_DATA_NTY_VAL],
              NULL, NULL);

          if (strstr(output_Data, "NTRSP") == NULL) {
            printf("\nmemcpy output 11212 %s\n", output_Data);
            esp_ble_gatts_send_indicate(
                spp_gatts_if, p_data->write.conn_id,
                spp_handle_table[SPP_IDX_SPP_DATA_NTY_VAL], strlen(output_Data),
                (uint8_t *)output_Data, false);

            if (output_Data[3] != 'G' && label_BLE_UDP_send) {
              mqtt_information mqttInfo;
              sprintf(mqttInfo.data, "%s", output_Data);
              sprintf(mqttInfo.topic, "%s", "");
              send_UDP_queue(&mqttInfo);
              //////printf("\nmemcpy output 32332323 %s\n", output_Data);
            }

            label_BLE_UDP_send = 1;
            // ////printf("\nmemcpy output 32332323 %s\n", output_Data);
          } else {
            CountBLEWrite--;
          }

          // }

          res = find_char_and_desr_index(p_data->write.handle);

          // example_prepare_write_event_env(gatts_if, &prepare_write_env,
          // p_data); xSemaphoreTake(rdySem_BLE_SPP,pdMS_TO_TICKS(30000));
          // free(x1);
          // ////printf("esp 14555 nn\n");
          free(output_Data);

          //    heap_trace_stop();
          //     heap_trace_dump();

          /* ESP_ERROR_CHECK(heap_trace_stop());
          heap_trace_dump(); */

          ESP_LOGI("TAG", "uxTaskGetStackHighWaterMark(NULL) %d",
                   uxTaskGetStackHighWaterMark(NULL));
          ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  : %d",
                   esp_get_minimum_free_heap_size());
          ESP_LOGI("TAG", "heap_caps_get_largest_free_block: %d",
                   heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
          ESP_LOGI("TAG", "free heap memory                : %d",
                   heap_caps_get_free_size(MALLOC_CAP_8BIT));

          // free(p_data->write.value);

          // ////printf("esp 12 nn\n");
        }

        // ////printf("esp HHHH nn\n");
      } else {

        /* handle prepare write */
        example_prepare_write_event_env(gatts_if, &prepare_write_env, param);

        // ESP_LOGI("TAG", "xPortGetFreeHeapSize            : %d",
        // xPortGetFreeHeapSize()); ESP_LOGI("TAG",
        // "esp_get_minimum_free_heap_size  : %d",
        // esp_get_minimum_free_heap_size()); ESP_LOGI("TAG",
        // "heap_caps_get_largest_free_block: %d",
        // heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
        // ESP_LOGI("TAG", "free heap memory                : %d",
        // heap_caps_get_free_size(MALLOC_CAP_8BIT));
        CountBLEWrite--;
      }

      // ESP_LOGI("TAG", "xPortGetFreeHeapSize  parseInputData2222: %d",
      // xPortGetFreeHeapSize()); ESP_LOGI("TAG",
      // "esp_get_minimum_free_heap_size  : %d",
      // esp_get_minimum_free_heap_size()); ESP_LOGI("TAG",
      // "heap_caps_get_largest_free_block: %d",
      // heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT)); ESP_LOGI("TAG",
      // "free heap memory                : %d",
      // heap_caps_get_free_size(MALLOC_CAP_8BIT));

      // ////printf("esp YYYY\n");

      break;
    }

    // ////printf("esp NNNN\n");
    break;
    // }
    // break;
  }
  case ESP_GATTS_EXEC_WRITE_EVT: {

    // ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_EXEC_WRITE_EVT\n");
    if (p_data->exec_write.exec_write_flag) {
      print_write_buffer();
      free_write_buffer();
      example_exec_write_event_env(&prepare_write_env, param);
    }
    break;
  }
  case ESP_GATTS_MTU_EVT:
    spp_mtu_size = p_data->mtu.mtu;
    break;
  case ESP_GATTS_CONF_EVT:

    // ////printf("\np_data->conf.handle:%d - %d\n",
    // spp_Files_handle_table[SPP_FILES_SPP_DATA_NTY_VAL], p_data->conf.handle);
    if (spp_UserList_handle_table[SPP_USERLIST_SPP_DATA_NTY_VAL] ==
        p_data->conf.handle) {
      // ////printf("\n relay sem 111 nn\n");
      xSemaphoreGive(rdySem);
      // ////printf("\n relay sem 112 nn\n");
    } else if (spp_handle_table[SPP_IDX_SPP_DATA_NTY_VAL] ==
                   p_data->conf.handle &&
               label_MonoStableRelay1 == 1) {
      // ////printf("\n relay sem 2222 nn\n");

      giveMonoSt_semaphore();
      // xSemaphoreGive(rdySem_RelayMonoInicial);
      // ////printf("\n relay sem 2223 nn\n");
      // label_MonoStableRelay1 = 0;
    } else if (spp_Files_handle_table[SPP_FILES_SPP_DATA_NTY_VAL] ==
               p_data->conf.handle) {
      // ////printf("\n relay sem 55 nn\n");
      // give_LOG_Files_Semph();
      // xSemaphoreGive(rdySem_Send_LOGS_Files);
      // ////printf("\n relay sem 56 nn\n");
    } else if (spp_Routines_handle_table[SPP_ROUTINES_SPP_DATA_NTY_VAL] ==
               p_data->conf.handle) {
      // ////printf("\n spp_routines_handle_table\n");
      xSemaphoreGive(rdySem_Send_Routines);
      // ////printf("\n spp_routines_handle_table\n");
    } else if (spp_handle_table[SPP_IDX_SPP_DATA_NTY_VAL] ==
                   p_data->conf.handle ||
               label_MonoStableRelay1 == 0) {
      // ////printf("\n relay sem 57 nn\n");
      xSemaphoreGive(rdySem_BLE_SPP);
      // ////printf("\n relay sem 58 nn\n");
    }

    break;
  case ESP_GATTS_UNREG_EVT:
    // ////printf("\n ESP_GATTS_UNREG_EVT \n");
    break;
  case ESP_GATTS_DELETE_EVT:
    // ////printf("\n ESP_GATTS_DELETE_EVT \n");
    break;
  case ESP_GATTS_START_EVT:
    // ////printf("\n ESP_GATTS_START_EVT \n");
    break;
  case ESP_GATTS_STOP_EVT:
    // ////printf("\n ESP_GATTS_STOP_EVT \n");
    break;
  case ESP_GATTS_CONNECT_EVT:
    spp_conn_id = p_data->connect.conn_id;

    if (spp_conn_id < 7) {
      // ////printf("\n conect conn id %d\n", spp_conn_id);
      active_BLE_conn[spp_conn_id] = 1;
      spp_gatts_if = gatts_if;
      is_connected = true;
      memcpy(&spp_remote_bda, &p_data->connect.remote_bda,
             sizeof(esp_bd_addr_t));
      // ESP_LOGE(GATTS_TABLE_TAG, "bd addr 111
      // %02x:%02x:%02x:%02x:%02x:%02x:",spp_remote_bda[0], spp_remote_bda[1],
      // spp_remote_bda[2],spp_remote_bda[3], spp_remote_bda[4],
      // spp_remote_bda[5]);

      // TODO: DESCOMENTAR LINHA A BAIXO
      // esp_ble_gap_config_adv_data(&spp_adv_config);

      // ////printf("\n\nstart_advertising 4\n\n");
      esp_ble_gap_start_advertising(&spp_adv_params);
      // ////printf("\n\nstart_advertising 5\n\n");

      // esp_ble_gap_start_advertising(&spp_adv_params);
    }
    break;
#ifdef SUPPORT_HEARTBEAT
    uint16_t cmd = 0;
    xQueueSend(cmd_heartbeat_queue, &cmd, 10 / portTICK_PERIOD_MS);
#endif
    break;
  case ESP_GATTS_DISCONNECT_EVT:
    active_BLE_conn[p_data->disconnect.conn_id] = 0;
    // ////printf("\n disconect conn id %d\n", p_data->disconnect.conn_id);
    is_connected = false;
    enable_data_ntf = false;
    // ////printf("\n\nstart_advertising 3\n\n");
    // esp_bt_controller_get_status()
    // TODO: DESCOMENTAR LINHA A BAIXO
    // esp_ble_gap_config_adv_data(&spp_adv_config);

    // ////printf("\n\nstart_advertising 4\n\n");
    esp_ble_gap_start_advertising(&spp_adv_params);
    // ////printf("\n\nstart_advertising 5\n\n");
    //   esp_ble_gap_start_advertising(&spp_adv_params);
    break;
  case ESP_GATTS_OPEN_EVT:
    // ////printf("\n\nESP_GATTS_OPEN_EVT \n\n");
    break;
  case ESP_GATTS_CANCEL_OPEN_EVT:
    // ////printf("\n\n ESP_GATTS_CANCEL_OPEN_EVT \n\n");
    break;
  case ESP_GATTS_CLOSE_EVT:
    // ////printf("\n\n ESP_GATTS_CLOSE_EVT \n\n");
    break;
  case ESP_GATTS_LISTEN_EVT:
    // ////printf("\n\nESP_GATTS_LISTEN_EVT\n\n");
    break;
  case ESP_GATTS_CONGEST_EVT:
    // ////printf("\n\nCONGEST EVENT\n\n");
    break;
  case ESP_GATTS_CREAT_ATTR_TAB_EVT: {
    // ESP_LOGI(GATTS_TABLE_TAG, "The number handle =%x\n",
    // param->add_attr_tab.num_handle);
    if (param->add_attr_tab.status != ESP_GATT_OK) {
      // ESP_LOGE(GATTS_TABLE_TAG, "Create attribute table failed, error
      // code=0x%x", param->add_attr_tab.status);
    } else {
      // ////printf("param->add_attr_tab.svc_uuid.uuid.uuid16 %d\n",
      // param->add_attr_tab.svc_uuid.uuid.uuid16);
      switch (param->add_attr_tab.svc_uuid.uuid.uuid16) {

      case (spp_service_uuid): {
        if (param->add_attr_tab.num_handle != SPP_IDX_NB) {
          // ESP_LOGE(GATTS_TABLE_TAG, "Create attribute table abnormally,
          // num_handle (%d) doesn't equal to HRS_IDX_NB(%d)",
          // param->add_attr_tab.num_handle, SPP_IDX_NB);
        } else {
          memcpy(spp_handle_table, param->add_attr_tab.handles,
                 sizeof(spp_handle_table));
          esp_ble_gatts_start_service(spp_handle_table[SPP_IDX_SVC]);
        }
        break;
      }
      case (0x19F0): {
        if (param->add_attr_tab.num_handle != HRS_IDX_NB) {
          // ESP_LOGE(GATTS_TABLE_TAG, "[B] create attribute table abnormally,
          // num_handle (%d) - doesnt equal to SVC_A_IDX_NB(%d)",
          // param->add_attr_tab.num_handle, HRS_IDX_NB);
        } else {
          // ESP_LOGI(GATTS_TABLE_TAG, "[B] create attribute table successfully,
          // the number handle = %d\n", param->add_attr_tab.num_handle);
          memcpy(heart_rate_handle_table, param->add_attr_tab.handles,
                 sizeof(heart_rate_handle_table));
          esp_ble_gatts_start_service(heart_rate_handle_table[IDX_SVC]);
        }
        break;
      }
      case (spp_UserList_service_uuid): {
        if (param->add_attr_tab.num_handle != SPP_USERLIST_NB) {
          // ESP_LOGE(GATTS_TABLE_TAG, "[B] create attribute table abnormally,
          // num_handle (%d) - doesnt equal to SVC_A_IDX_NB(%d)",
          // param->add_attr_tab.num_handle, SPP_USERLIST_NB);
        } else {
          // ESP_LOGI(GATTS_TABLE_TAG, "[B] create attribute table successfully,
          // the number handle = %d\n", param->add_attr_tab.num_handle);
          memcpy(spp_UserList_handle_table, param->add_attr_tab.handles,
                 sizeof(spp_UserList_handle_table));
          esp_ble_gatts_start_service(
              spp_UserList_handle_table[SPP_USERLIST_SVC]);
        }
        break;
      }
      case (spp_Files_service_uuid): {
        if (param->add_attr_tab.num_handle != SPP_FILES_NB) {
          // ESP_LOGE(GATTS_TABLE_TAG, "[B] create attribute table abnormally,
          // num_handle (%d) - doesnt equal to SVC_A_IDX_NB(%d)",
          // param->add_attr_tab.num_handle, SPP_FILES_NB);
        } else {
          // ESP_LOGI(GATTS_TABLE_TAG, "[B] create attribute table successfully,
          // the number handle = %d\n", param->add_attr_tab.num_handle);
          memcpy(spp_Files_handle_table, param->add_attr_tab.handles,
                 sizeof(spp_Files_handle_table));
          esp_ble_gatts_start_service(spp_Files_handle_table[SPP_FILES_SVC]);
        }
        break;
      }
      case (spp_Routines_service_uuid): {
        if (param->add_attr_tab.num_handle != SPP_ROUTINES_NB) {
          // ESP_LOGE(GATTS_TABLE_TAG, "[B] create attribute table abnormally,
          // num_handle (%d) - doesnt equal to SVC_A_IDX_NB(%d)",
          // param->add_attr_tab.num_handle, SPP_ROUTINES_NB);
        } else {

          // ESP_LOGI(GATTS_TABLE_TAG, "[B] create attribute table successfully,
          // the number handle = %d\n", param->add_attr_tab.num_handle);
          memcpy(spp_Routines_handle_table, param->add_attr_tab.handles,
                 sizeof(spp_Routines_handle_table));
          esp_ble_gatts_start_service(
              spp_Routines_handle_table[SPP_ROUTINES_SVC]);
        }
        break;
      }
      default: {
        // ESP_LOGE(GATTS_TABLE_TAG, "attribute table does not match any known
        // cases, the number handle = %d\n", param->add_attr_tab.num_handle);
        break;
      }
      }
    }

    break;

  default:
    break;
  }
  }

  // ////printf("esp END 1124567\n");
}

static void gatts_event_handler(esp_gatts_cb_event_t event,
                                esp_gatt_if_t gatts_if,
                                esp_ble_gatts_cb_param_t *param) {
  // ESP_LOGI(GATTS_TABLE_TAG, "EVT %d, gatts if %d\n", event, gatts_if);

  /* If event is register event, store the gatts_if for each profile */
  if (event == ESP_GATTS_REG_EVT) {
    if (param->reg.status == ESP_GATT_OK) {
      spp_profile_tab[SPP_PROFILE_APP_IDX].gatts_if = gatts_if;
    } else {
      // ESP_LOGI(GATTS_TABLE_TAG, "Reg app failed, app_id %04x, status %d\n",
      // param->reg.app_id, param->reg.status);
      return;
    }
  }

  do {
    int idx;
    for (idx = 0; idx < SPP_PROFILE_NUM; idx++) {
      if (gatts_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a
                                             certain gatt_if, need to call every
                                             profile cb function */
          gatts_if == spp_profile_tab[idx].gatts_if) {
        if (spp_profile_tab[idx].gatts_cb) {
          spp_profile_tab[idx].gatts_cb(event, gatts_if, param);
        }
      }
    }
  } while (0);
}

/* #define TIMER_BASE_CLK (esp_clk_apb_freq())
#define TIMER_DIVIDER 16
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER)
#define TIMER_INTERVAL0_SEC (3.4179)
#define TIMER_INTERVAL1_SEC (5.78)
#define TEST_WITHOUT_RELOAD 0
#define TEST_WITH_RELOAD 1 */

char text[] = "char* data";

// void vTimer_ADV_Callback(TimerHandle_t xTimer)
// {

//     ////printf("\n\nTIMER ADV00\n\n");

//     esp_bt_controller_status_t status = esp_bt_controller_get_status();
//     // xQueueSendToBack(UDP_Send_queue, (void *)&text, pdMS_TO_TICKS(1000));
//     // send_UDP_Package("char* data",strlen("char* data"));
//     //  if (status != ESP_BT_CONTROLLER_STATUS_ENABLED)
//     //  {
//     //      esp_restart();
//     //      // O controlador Bluetooth está ativado
//     //  }
//     //  ////printf("\n\nstart_advertising 4\n\n");

//     // ////printf("\n\nTIMER ADV11\n\n");
// }

void print_hex(const unsigned char *data, size_t len) {
  for (size_t i = 0; i < len; i++) {
    //////printf("%02x ", data[i]);
  }
  //////printf("\n");
}

void encrypt_aes_cbc1(const unsigned char *input, size_t input_len,
                      unsigned char *output, const unsigned char *key,
                      const unsigned char *iv) {
  mbedtls_aes_context aes_ctx;
  mbedtls_aes_init(&aes_ctx);

  mbedtls_aes_setkey_enc(&aes_ctx, key, AES_KEY_SIZE * 8);

  size_t block_count = (input_len + AES_KEY_SIZE - 1) / AES_KEY_SIZE;

  unsigned char prev_block[AES_KEY_SIZE];
  memcpy(prev_block, iv, AES_KEY_SIZE);

  for (size_t i = 0; i < block_count; i++) {
    unsigned char current_block[AES_KEY_SIZE];
    memcpy(current_block, &input[i * AES_KEY_SIZE], AES_KEY_SIZE);

    for (size_t j = 0; j < AES_KEY_SIZE; j++) {
      current_block[j] ^= prev_block[j];
    }

    mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_ENCRYPT, current_block, output);

    memcpy(prev_block, output, AES_KEY_SIZE);
    output += AES_KEY_SIZE;
  }

  mbedtls_aes_free(&aes_ctx);
}

void app_main(void) {
  esp_err_t ret;
  vTaskDelay(pdMS_TO_TICKS(5000));
  // Initialize NVS
  ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  size_t required_size = 0;

  rdySem_BLE_SPP = xSemaphoreCreateBinary();

  xSemaphoreGive(rdySem_BLE_SPP);

  unsigned char content[64] = {0};

  esp_vfs_spiffs_conf_t conf = {.base_path = "/spiffs",
                                .partition_label = NULL,
                                .max_files = 5,
                                .format_if_mount_failed = true};

  /* heap_caps_check_integrity_all(true);
  heap_caps_print_heap_info(MALLOC_CAP_DEFAULT); */
  network_Activate_Flag = 1;
  ret = esp_vfs_spiffs_register(&conf);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      // ESP_LOGE("TAG", "Failed to mount or format filesystem");
    } else if (ret == ESP_ERR_NOT_FOUND) {
      // ESP_LOGE("TAG", "Failed to find SPIFFS partition");
    } else {
      // ESP_LOGE("TAG", "Failed to initialize SPIFFS (%s)",
      // esp_err_to_name(ret));
    }
    return;
  }

  size_t total = 0, used = 0;
  ret = esp_spiffs_info(conf.partition_label, &total, &used);
  if (ret != ESP_OK) {
    // ESP_LOGE("TAG", "Failed to get SPIFFS partition information (%s)",
    // esp_err_to_name(ret));
  } else {
    // ESP_LOGI("TAG", "Partition size: total: %d, used: %d", total, used);
  }

  int line = 0;
  // TODO: APAGAR LINHA A BAIXO
  // heap_trace_init_standalone(trace_record, NUM_RECORDS);
  initSystem();

  // Configuração do pino GPIO como saída

  // init_mipot();
  // import_mqtt_users("917269448", "999999");
  // save_STR_Data_In_Storage(NVS_KEY_HW_VERSION,"1.0.1", nvs_System_handle);

  vTaskDelay(pdMS_TO_TICKS(1000));
  //////printf("\n\n akakak 888-111\n\n");
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

  // esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

  ESP_ERROR_CHECK(esp_bt_controller_mem_release(
      /* ESP_BT_MODE_BLE  */ ESP_BT_MODE_CLASSIC_BT));

  ret = esp_bt_controller_init(&bt_cfg);
  if (ret) {
    // ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed: %s\n", __func__,
    // esp_err_to_name(ret));
    return;
  }

  ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
  if (ret) {
    // ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed: %s\n", __func__,
    // esp_err_to_name(ret));
    return;
  }

  // ESP_LOGI(GATTS_TABLE_TAG, "%s init bluetooth\n", __func__);
  ret = esp_bluedroid_init();
  if (ret) {
    // ESP_LOGE(GATTS_TABLE_TAG, "%s init bluetooth failed: %s\n", __func__,
    // esp_err_to_name(ret));
    return;
  }
  ret = esp_bluedroid_enable();
  if (ret) {
    // ESP_LOGE(GATTS_TABLE_TAG, "%s enable bluetooth failed: %s\n", __func__,
    // esp_err_to_name(ret));
    return;
  }

  esp_ble_gatts_register_callback(gatts_event_handler);
  esp_ble_gap_register_callback(gap_event_handler);
  esp_ble_gatts_app_register(ESP_SPP_APP_ID);

  ESP_LOGI("TAG", "xPortGetFreeHeapSize  parseInputData1111: %d",
           xPortGetFreeHeapSize());
  ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  : %d",
           esp_get_minimum_free_heap_size());
  ESP_LOGI("TAG", "heap_caps_get_largest_free_block: %d",
           heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
  ESP_LOGI("TAG", "free heap memory                : %d",
           heap_caps_get_free_size(MALLOC_CAP_8BIT));

  return;
}

void disableBLE() {
  esp_ble_gatts_app_unregister(ESP_SPP_APP_ID);

  // Desabilita o stack Bluedroid
  esp_bluedroid_disable();

  // Deinitialize o stack Bluedroid
  esp_bluedroid_deinit();

  // Desativa o controlador Bluetooth
  esp_bt_controller_disable();

  // Deinitialize o controlador Bluetooth
  esp_bt_controller_deinit();

  esp_bt_controller_mem_release(/* ESP_BT_MODE_BLE  */ ESP_BT_MODE_CLASSIC_BT);
}

void restartBLE() {
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  esp_err_t ret;

  ESP_ERROR_CHECK(esp_bt_controller_mem_release(
      /* ESP_BT_MODE_BLE  */ ESP_BT_MODE_CLASSIC_BT));

  ret = esp_bt_controller_init(&bt_cfg);
  if (ret) {
    // ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed: %s\n", __func__,
    // esp_err_to_name(ret));
    return;
  }

  ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
  if (ret) {
    // ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed: %s\n", __func__,
    // esp_err_to_name(ret));
    return;
  }

  // ESP_LOGI(GATTS_TABLE_TAG, "%s init bluetooth\n", __func__);
  ret = esp_bluedroid_init();
  if (ret) {
    // ESP_LOGE(GATTS_TABLE_TAG, "%s init bluetooth failed: %s\n", __func__,
    // esp_err_to_name(ret));
    return;
  }
  ret = esp_bluedroid_enable();
  if (ret) {
    // ESP_LOGE(GATTS_TABLE_TAG, "%s enable bluetooth failed: %s\n", __func__,
    // esp_err_to_name(ret));
    return;
  }

  esp_ble_gatts_register_callback(gatts_event_handler);
  esp_ble_gap_register_callback(gap_event_handler);
  esp_ble_gatts_app_register(ESP_SPP_APP_ID);
}

void BLE_Broadcast_Notify(char *data) {

  for (uint16_t i = 0; i < 7; i++) {
    if (active_BLE_conn[i] == 1) {
      //////printf("\nnotify conn id %d\n", i);
      esp_ble_gatts_send_indicate(spp_gatts_if, i,
                                  spp_handle_table[SPP_IDX_SPP_DATA_NTY_VAL],
                                  strlen(data), (uint8_t *)data, false);
    }
  }

  // free(data);
}

char *change_BLE_Name(char *payload, char *rsp_BLE_Name) {

  esp_err_t ret;

  uint8_t spp_adv_data1[31] = {/* Flags */
                               0x02, 0x01, 0x06,
                               /* Complete List of 16-bit Service Class UUIDs */
                               0x03, 0x03, 0xF0, 0xAE,
                               /* Complete Local Name in advertising */
                               0x16, 0x09};

  for (size_t i = 9; i < strlen(payload) + 9; i++) {
    if (i > 30) {
      break;
    }

    spp_adv_data1[i] = (uint8_t)payload[i - 9];
    rsp_BLE_Name[i - 9] = payload[i - 9];
  }

  esp_ble_gap_config_adv_data_raw((uint8_t *)spp_adv_data1,
                                  sizeof(spp_adv_data1));
  esp_ble_gap_set_device_name(rsp_BLE_Name);

  return "OK";
}
