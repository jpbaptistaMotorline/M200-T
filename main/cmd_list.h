/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______ 
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__   
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|  
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____ 
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|
                                                                                                                          
*/


#ifndef _CMD_LIST_H_
#define _CMD_LIST_H_

/**********************************
*          COMMANDS
**********************************/

#define SET_CMD 'S'
#define GET_CMD 'G'
#define RESET_CMD 'R'

/*********************************/

/**********************************
*           ELEMENTS
**********************************/

#define RELE1_ELEMENT "R1"
#define RELE2_ELEMENT "R2"

#define INPUT1_ELEMENT "I1"
#define INPUT2_ELEMENT "I2"

#define FEEDBACK1_ELEMENT "F1"
#define FEEDBACK2_ELEMENT "F2"
#define FEEDBACK3_ELEMENT "F3"
#define FEEDBACK4_ELEMENT "F4"
#define FEEDBACK5_ELEMENT "F5"
#define FEEDBACK6_ELEMENT "F6"
#define FEEDBACK_MODE_ELEMENT "FX"

#define ALARM_ELEMENT "AL"

#define ADMIN_ELEMENT "ME"

#define USER_ELEMENT "UR"

#define ROUTINE_ELEMENT "RT"

#define WIEGAND_ELEMENT "WI"

#define RF_ELEMENT "RF"

/*********************************/

/**********************************
*           PARAMETERS
**********************************/

#define RELE_PARAMETER 'R'
#define RELE_MODE_PARAMETER 'M'
#define MULTI_USERS_PARAMETER 'M'
#define TIME_PARAMETER 'T'
#define PAIRING_PARAMETER 'B'
#define INPUT_PARAMETER 'I'
#define INPUT_REX_PARAMETER 'X'
#define PHONE_PARAMETER 'P'
#define LAST_PARAMETER 'L'
#define SIGNAL_PARAMETER 'Q'
#define ADMIN_PARAMETER 'A'
#define ALARM_PARAMETER 'A'
#define USER_PARAMETER 'U'
#define ALL_PARAMETER '*'
#define DATE_PARAMETER 'D'
#define HOUR_PARAMETER 'H'
#define WEEK_PARAMETER 'W'
#define NAME_PARAMETER 'N'
#define OWNER_PARAMETER 'O'
#define SOUND_PARAMETER 'S'
#define SYSTEM_BLE_START_PARAMETER 'I'
#define CLOCK_PARAMETER 'C'
#define ROUTINE_PARAMETER 'R'
#define ROUTINE_RANGE_PARAMETER 'T'
#define ROUTINE_DAY_EXCEPTION_PARAMETER 'D'
#define ROUTINE_ACTIVATE_PARAMETER 'A'
#define RELE_RESTRITION_PARAMETER 'R'
#define SIM_CARD_BALANCE_PARAMETER 'B'
#define BLOCK_FEEDBACK_SMS_PARAMETER 'F'
#define ALARM_OTHER_USERS_FEEDBACK_PARAMETER 'O'
#define ALARM_CHANGE_INPUT_STATE_FEEDBACK_PARAMETER 'C'
#define NORMAL_FEEDBACK_MODE_PARAMETER 'M'
#define START_DOWNLOAD_FILE 'K'
#define LANGUAGE_FILE_PARAMETER 'T'
#define IMEI_NUMBER_PARAMETER 'M'
#define SIM_IMEI_ESPMAC_PARAMETER 'P'
#define ESP_MAC_ADDRESS_PARAMETER 'E'
#define OWNER_KEY_PARAMETER 'K'
#define USER_RESET_PASSWORD_PARAMETER 'A'
#define USER_RESET_BLE_SECURITY_PARAMETER 'B'
#define FORMAT_SDCARD_PARAMETER 'M'
#define M200_FIRMWARE_HARDWARE_PARAMETER 'M'
#define M200_RELAYS_CONFIGURATION_PARAMETER 'D'
#define M200_ACTIVATE_NETWORK 'Q'
#define ROUTINES_HOLIDAYS_RANGE_PARAMETER 'F'
#define ROUTINE_HOLIDAYS_MOBILE_DAYS 'H'
#define NETWORK_LABEL_PARAMETER 'W'
#define NETWORK_LOGS_LABEL_PARAMETER 'E'
#define GET_IMEI_PARAMETER 'E'
#define WIEGAND_CHANGE_RELAY_PARAMETER 'R'
#define WIEGAND_START_AUTO_SAVE_PARAMETER 'S'
#define WIEGANG_NUMBER_PARAMETER 'W'
#define WIEGANG_PHONE_NUMBER_PARAMETER 'P'
#define WIEGANG_TURN_ON_OFF_PARAMETER 'C'
#define REDITECT_SMS_PARAMETER 'R'
#define SMS_CALL_VERIFICATION_PARAMETER 'V'
#define SMS_TRANSLATE_PARAMETER 'I'
#define EG91_FOTA_PARAMETER 'P'
#define IMPORT_USERS_HTTPS_PARAMETER 'I'
#define ACTIVATE_ANTIPASSBACK_PARAMETER 'A'


#define RF_CHANGE_RELAY_PARAMETER 'R'

#define RF_GET_PARAMETER 'F'
#define RF_DELETE_RF_PARAMETER 'F'
#define RF_SAVE_AUTO_PARAMETER 'S'
#define RF_SAVE_COMMAND_PARAMETER 'C'
#define RF_SAVE_BUTTON_PARAMETER 'B'
#define RF_CELL_PHONE_NUMBER_PARAMETER 'P'
#define RF_ROLLING_CODE_PARAMETER 'R'



/*********************************/


#endif