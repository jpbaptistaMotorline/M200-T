/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______ 
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__   
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|  
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____ 
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|
                                                                                                                          
*/

#ifndef _NVS_H_
#define _NVS_H_

#include "nvs.h"
#include "sdkconfig.h"
  extern uint8_t other_Users_Feedback;
    extern uint8_t change_Input_State_Feedback;

#define NVS_OWNER_NAMESPACE                 "OW_NAMESPACE"
#define NVS_SYSTEM_NAMESPACE                "SY_NAMESPACE"
#define NVS_ADMIN_NAMESPACE                 "AD_NAMESPACE"
#define NVS_USERS_NAMESPACE                 "UR_NAMESPACE"
#define NVS_FEEDBACK_NAMESPACE              "FB_NAMESPACE"
#define NVS_ROUTINES_NAMESPACE              "RT_NAMESPACE"
#define NVS_EXEPTION_DAYS_NAMESPACE         "ED_NAMESPACE"
#define NVS_MOBILE_HOLYDAYS_NAMESPACE       "MH_NAMESPACE"
#define NVS_WIEGAND_CODES_GUEST_NAMESPACE   "W_G_NAMESPACE"
#define NVS_WIEGAND_CODES_ADMIN_NAMESPACE   "W_A_NAMESPACE"
#define NVS_WIEGAND_CODES_OWNER_NAMESPACE   "W_O_NAMESPACE"
#define NVS_RF_CODES_GUEST_NAMESPACE        "RF_G_NAMESPACE"
#define NVS_RF_CODES_ADMIN_NAMESPACE        "RF_A_NAMESPACE"
#define NVS_RF_CODES_OWNER_NAMESPACE        "RF_O_NAMESPACE"

#define NVS_WIEGAND_ANTIPASSBACK_NAMESPACE  "W_AP_NAMESPACE"



#define NVS_LAST_MONTH                      "NVS_LAST_MONTH"
#define NVS_LAST_CALL                       "NVS_LAST_CALL"

#define NVS_AES_KEY                         "NVS_AES_KEY"
#define NVS_AES_IV                          "NVS_AES_IV"

#define NVS_USERS_NB                        "NVS_USERS_NB"
#define NVS_FW_VERSION                      "NVS_FW_VERSION"

#define NVS_FB_CONF_F1                      "NVS_FB_CONF_F1"
#define NVS_FB_CONF_F2                      "NVS_FB_CONF_F2"
#define NVS_FB_CONF_F3                      "NVS_FB_CONF_F3"
#define NVS_FB_CONF_F4                      "NVS_FB_CONF_F4"
#define NVS_FB_CONF_F5                      "NVS_FB_CONF_F5"
#define NVS_FB_CONF_F6                      "NVS_FB_CONF_F6"

#define NVS_FB_CONF_P1                      "NVS_FB_CONF_P1"
#define NVS_FB_CONF_P2                      "NVS_FB_CONF_P2"
#define NVS_FB_CONF_P3                      "NVS_FB_CONF_P3"
#define NVS_FB_CONF_P4                      "NVS_FB_CONF_P4"
#define NVS_FB_CONF_P5                      "NVS_FB_CONF_P5"
#define NVS_FB_CONF_P6                      "NVS_FB_CONF_P6"

#define NVS_FB_CONF_I1                      "NVS_FB_CONF_I1"
#define NVS_FB_CONF_I2                      "NVS_FB_CONF_I2"
#define NVS_FB_CONF_I3                      "NVS_FB_CONF_I3"
#define NVS_FB_CONF_I4                      "NVS_FB_CONF_I4"
#define NVS_FB_CONF_I5                      "NVS_FB_CONF_I5"
#define NVS_FB_CONF_I6                      "NVS_FB_CONF_I6"

#define NVS_FB_MODE                         "NVS_FB_MODE"
#define NVS_FB_TIM1                         "NVS_FB_TIM1"
#define NVS_FB_TIM2                         "NVS_FB_TIM2"

#define NVS_AL_CONF_AL                      "NVS_AL_CONF_AL"

#define NVS_AL_CONF_IN                      "NVS_AL_CONF_IN"
#define NVS_AL_CONF_F1                      "NVS_AL_CONF_F1"
#define NVS_AL_CONF_F2                      "NVS_AL_CONF_F2"
#define NVS_AL_CONF_F3                      "NVS_AL_CONF_F3"
#define NVS_AL_CONF_F4                      "NVS_AL_CONF_F4"
#define NVS_AL_CONF_F5                      "NVS_AL_CONF_F5"
#define NVS_AL_CONF_F6                      "NVS_AL_CONF_F6"
#define NVS_AL_OTHER_USERS_FEEDBACK         "NVS_AL_O_U_FB"
#define NVS_AL_CHANGE_INPUT_STATE_FEEDBACK  "NVS_AL_I_S_FB"

#define NVS_ROUTINES_ID                     "NVS_RT_ID"
#define NVS_ROUTINES_RANGE_T0               "NVS_RT_RG_T0"
#define NVS_ROUTINES_RANGE_T1               "NVS_RT_RG_T1"
#define NVS_KEY_ROUTINE1_LABEL              "NVS_RT_1_LB"
#define NVS_KEY_ROUTINE2_LABEL              "NVS_RT_2_LB"
#define NVS_KEY_ROUTINE1_TIME_ON            "NVS_RT_1_T_ON"
#define NVS_KEY_ROUTINE2_TIME_ON            "NVS_RT_2_T_ON"

#define NVS_TIMEZONE                        "NVS_TIMEZONE"

#define NVS_KEY_BLOCK_FEEDBACK_SMS          "NVS_BK_FD_SMS"
#define NVS_KEY_FEEDBACK_RELES_MODE         "NVS_FB_R_MODE"

#define NVS_KEY_FEEDBACK_SOUND_INDEX        "NVS_SOUND_IDX"

#define NVS_KEY_OWNER_INFORMATION           "NVS_OW_INFO"
#define NVS_KEY_OWNER_LABEL                 "NVS_OW_LABEL"

#define NVS_KEY_BLE_NAME                    "NVS_BLE_NAME"

#define NVS_KEY_LABEL_PERIODIC_SMS          "NVS_L_P_SMS"
#define NVS_KEY_DATE_PERIODIC_SMS           "NVS_D_P_SMS"

#define NVS_KEY_OWN_NUMBER                  "NVS_OWN_NUMBER"

#define NVS_KEY_SDCARD_RESET                "NVS_SD_RESET"

#define NVS_KEY_RELAY_PAIRING_VALUE         "NVS_R_P_VALUE"

#define NVS_RELAY1_BISTATE_TIME             "NVS_R1_BI_TIM"
#define NVS_RELAY2_BISTATE_TIME             "NVS_R2_BI_TIM"

#define NVS_RELAY1_MODE                     "NVS_R1_MODE"
#define NVS_RELAY2_MODE                     "NVS_R2_MODE"

#define NVS_KEY_HW_VERSION                  "NVS_HW_VER"

#define NVS_KEY_USER_COUNTER                "NVS_USER_CNT"

#define NVS_KEY_GUEST_COUNTER               "NVS_GUEST_CNT"

#define NVS_KEY_BLE_NAME                    "NVS_BLE_NAME"

#define NVS_KEY_EG91_ICCID                  "NVS_ICCID"

#define NVS_KEY_EG91_IMEI                   "NVS_EG91_IMEI"

#define NVS_KEY_ACTIVATE_NETWORK_LABEL      "NVS_NW_LB"

#define NVS_KEY_RESTART_SYSTEM              "NVS_RST_SYS"

#define NVS_KEY_LABEL_CARNIVAL              "NVS_LB_CAR" 
#define NVS_KEY_LABEL_EASTER_FRIDAY         "NVS_LB_E_FR" 
#define NVS_KEY_LABEL_EASTER                "NVS_LB_EAS" 
#define NVS_KEY_LABEL_CORPUS_CHRISTI        "NVS_LB_C_CH" 
#define NVS_KEY_LABEL_EASTER_MONDAY         "NVS_LB_E_MN" 

#define NVS_KEY_OLD_YEAR                    "NVS_OLD_YEAR"

#define NVS_KEY_RELAY1_LAST_VALUE           "NVS_R1_L_V"
#define NVS_KEY_RELAY2_LAST_VALUE           "NVS_R2_L_V"

#define NVS_ROUTINES_HOLIDAYS_RANGE_T1      "NVS_RT_R_H_T1"
#define NVS_ROUTINES_HOLIDAYS_RANGE_T2      "NVS_RT_R_H_T2"

#define NVS_NETWORK_PORTAL_REGISTER         "NVS_NT_P_REG"
#define NVS_NETWORK_LABEL_SEND_LOGS         "NVS_NT_S_LOG"

#define NVS_NETWORK_LOCAL_CHANGED           "NVS_NT_L_C"

#define NVS_OWNER_PASSWORD_KEY_CHANGED      "NVS_O_P_C"

#define NVS_QMTSTAT_LABEL                   "NVS_QMTSTAT_L"

#define NVS_QMT_LARGE_DATA_TIMER_LABEL      "NVS_QMT_LDTL"

#define NVS_LIMIT_USERS                     "NVS_LIM_USR"

#define NVS_WIEGAND_ACTIVATE_LABEL          "NVS_WI_L_E"
#define NVS_ANTIPASSBACK_ACTIVATE_LABEL     "NVS_AP_L_E"
#define NVS_ANTIPASSBACK_MODE_LABEL         "NVS_AP_M_E"
#define NVS_ANTIPASSBACK_PEOPLE_NUMBER      "NVS_AP_PC_N"
#define NVS_ANTIPASSBACK_PEOPLE_COUNTER      "NVS_AP_PC_C"

#define NVS_RF_ROLLING_CODE   "NVS_RF_R_C"

#define NVS_REDIRECT_SMS                    "NVS_RED_SMS"

#define NVS_SMS_CALL_VERIFICATION           "NVS_S_C_VER"

#define NVS_SMS_INPUT1_MESSAGE              "NVS_SMS_I1"

#define NVS_SMS_INPUT2_MESSAGE              "NVS_SMS_I2"

#define NVS_EG91_ICCID_VALUE                 "NVS_ICCID_EG"

#define NVS_INPUT_REX_VALUE                   "NVS_I_R_VAL"







#endif