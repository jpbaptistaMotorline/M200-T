/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______ 
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__   
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|  
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____ 
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|
                                                                                                                          
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system.h"



/*
 * DEFINES
 ****************************************************************************************
 */
//#define SUPPORT_HEARTBEAT
//#define SPP_DEBUG_MODE

#define spp_sprintf(s,...)         sprintf((char*)(s), ##__VA_ARGS__)
#define SPP_DATA_MAX_LEN           (512)
#define SPP_CMD_MAX_LEN            (20)
#define SPP_STATUS_MAX_LEN         (20)
#define SPP_DATA_BUFF_MAX_LEN      (2*1024)
#define DEFAULT_BLE_NAME            "MOTORLINE M270"

//TimerHandle_t xTimer_ADV;

void adv();

///Attributes State Machine
enum{
    SPP_IDX_SVC,

    SPP_IDX_SPP_DATA_RECV_CHAR,
    SPP_IDX_SPP_DATA_RECV_VAL,

    SPP_IDX_SPP_DATA_NOTIFY_CHAR,
    SPP_IDX_SPP_DATA_NTY_VAL,
    SPP_IDX_SPP_DATA_NTF_CFG,

    SPP_IDX_SPP_COMMAND_CHAR,
    SPP_IDX_SPP_COMMAND_VAL,

    SPP_IDX_SPP_STATUS_CHAR,
    SPP_IDX_SPP_STATUS_VAL,  
    SPP_IDX_SPP_STATUS_CFG,

#ifdef SUPPORT_HEARTBEAT
    SPP_IDX_SPP_HEARTBEAT_CHAR,
    SPP_IDX_SPP_HEARTBEAT_VAL,
    SPP_IDX_SPP_HEARTBEAT_CFG,
#endif

    SPP_IDX_NB,
};



/* Attributes State Machine */
enum
{
    IDX_SVC,
    IDX_CHAR_A,
    IDX_CHAR_VAL_A,

    IDX_CHAR_B,
    IDX_CHAR_VAL_B,

    HRS_IDX_NB,
};

enum
{
    SPP_USERLIST_SVC,

    SPP_USERLIST_SPP_DATA_RECV_CHAR,
    SPP_USERLIST_SPP_DATA_RECV_VAL,

    SPP_USERLIST_SPP_DATA_NOTIFY_CHAR,
    SPP_USERLIST_SPP_DATA_NTY_VAL,
    SPP_USERLIST_SPP_DATA_NTF_CFG,

    SPP_USERLIST_SPP_COMMAND_CHAR,
    SPP_USERLIST_SPP_COMMAND_VAL,

    SPP_USERLIST_SPP_STATUS_CHAR,
    SPP_USERLIST_SPP_STATUS_VAL,
    SPP_USERLIST_SPP_STATUS_CFG,
    SPP_USERLIST_NB,
  
};

enum
{
    SPP_FILES_SVC,

    SPP_FILES_SPP_DATA_RECV_CHAR,
    SPP_FILES_SPP_DATA_RECV_VAL,

    SPP_FILES_SPP_DATA_NOTIFY_CHAR,
    SPP_FILES_SPP_DATA_NTY_VAL,
    SPP_FILES_SPP_DATA_NTF_CFG,

    SPP_FILES_SPP_COMMAND_CHAR,
    SPP_FILES_SPP_COMMAND_VAL,

    SPP_FILES_SPP_STATUS_CHAR,
    SPP_FILES_SPP_STATUS_VAL,
    SPP_FILES_SPP_STATUS_CFG,
    SPP_FILES_NB,
  
};

enum
{
    SPP_ROUTINES_SVC,

    SPP_ROUTINES_SPP_DATA_RECV_CHAR,
    SPP_ROUTINES_SPP_DATA_RECV_VAL,

    SPP_ROUTINES_SPP_DATA_NOTIFY_CHAR,
    SPP_ROUTINES_SPP_DATA_NTY_VAL,
    SPP_ROUTINES_SPP_DATA_NTF_CFG,

    SPP_ROUTINES_SPP_COMMAND_CHAR,
    SPP_ROUTINES_SPP_COMMAND_VAL,

    SPP_ROUTINES_SPP_STATUS_CHAR,
    SPP_ROUTINES_SPP_STATUS_VAL,
    SPP_ROUTINES_SPP_STATUS_CFG,
    SPP_ROUTINES_NB,
  
};



char *change_BLE_Name(char* payload,char* rsp_BLE_Name);

void BLE_Broadcast_Notify(char *data);


void disableBLE();
void restartBLE();

