/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|

*/

#include "system.h"
#include "users.h"
// #include <gpio.h>
#include "AT_CMD_List.h"
#include "EG91.h"
#include "ble_spp_server_demo.h"
#include "cmd_list.h"
#include "core.h"
#include "crc32.h"
#include "cron.h"
#include "erro_list.h"
#include "esp_gatts_api.h"
#include "esp_timer.h"

#include "inputs.h"
#include "jobs.h"
#include "pcf85063.h"
#include "rele.h"
#include "rf.h"
#include "routines.h"
#include "unity.h"
#include "wiegand.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

uint32_t translate_File_CRC32 = 0;
uint8_t label_Translate_File_CRC32 = 0;
uint8_t label_Semaphore_Reset_System;
uint8_t sendLOG_User_Label;
char SYSTEM_NAME[30];
uint8_t label_SoundFile_Open;
uint8_t feedback_Sound_Index = 0;
uint8_t label_Block_Feedback_SMS;
uint8_t count_ResetSystem;
uint8_t label_ResetSystem;

char aux_FileContens_Admin[300] = {};

extern QueueHandle_t Type_Call_queue;
SemaphoreHandle_t rdySem_Send_LOGS_Files;
SemaphoreHandle_t rdySem_Reset_System;

typedef struct {
  uint8_t gattsIF;
  uint16_t connID;
  uint16_t handle_table;
  char payload[200];
  char usr_perm;

} sendLOGS_parameters_Message;

char file_contents[310];

char *MyUser_change_USER_To_ADMIN(uint8_t BLE_SMS_INDICATION, char *payload,
                                  mqtt_information *mqttInfo) {
  char rsp_Data[300] = {};
  MyUser myUser;
  // ////printf("\nchange user to admin 11 \n");
  memset(&myUser, 0, sizeof(myUser));
  // ////printf("\nchange user to admin 22 \n");
  memset(file_contents, 0, sizeof(file_contents));
  if (MyUser_Search_User(payload, file_contents) == ESP_OK) {
    // ////printf("\nendHour  filecontents11 %s\n", file_contents);
    parse_ValidateData_User(file_contents, &myUser);
    // ////printf("\nendHour  filecontents22 %s\n", file_contents);
    // ////printf("\nchange user to admin 55 \n");
    //   myUser.permition = '0';
    // ////printf("\nchange user to admin 555 \n");
    if (myUser.permition == '0') {

      uint8_t ACK_wi_antipassback = get_antipassback_user(myUser.wiegand_code);
      // ////printf("\nchange user to admin 33 \n");
      if (Myuser_deleteUser(&myUser) == ESP_OK) {
        // //printf("\nchange user to admin 44 - %s \n", myUser.phone);
        myUser.permition = '1';
        uint16_t ACK_Add_User = MyUser_Add(&myUser);
        if (ACK_Add_User == ESP_OK) {
          if (ACK_wi_antipassback == ESP_OK) {
            erase_antipassback_user(myUser.wiegand_code);
            set_antipassback_user(myUser.wiegand_code, '1');
            nvs_get_u32(nvs_System_handle, NVS_ANTIPASSBACK_PEOPLE_NUMBER,
                        &anti_passback_people_number);
            anti_passback_people_number++;
            ESP_LOGI("WIEGAND",
                     "wiegand1_action: setting antipassback people number %d",
                     anti_passback_people_number);
            nvs_set_u32(nvs_System_handle, NVS_ANTIPASSBACK_PEOPLE_NUMBER,
                        anti_passback_people_number);
          }
          // MyUser_add_wiegand(myUser.wiegand_code,, char permition);
          memset(file_contents, 0, sizeof(file_contents));
          // memset(&myUser, 0, sizeof(myUser));
          // parse_ValidateData_User(file_contents, &myUser);

          MyUser_Search_User(myUser.phone, rsp_Data);
          memset(aux_FileContens_Admin, 0, sizeof(aux_FileContens_Admin));
          // ////printf("file contents USER_To_ADMIN %s\n", rsp_Data);
          /*memset(&rsp, 0, sizeof(rsp));*/
          erase_Password_For_Rsp(rsp_Data, aux_FileContens_Admin);

          if (BLE_SMS_INDICATION == BLE_INDICATION ||
              BLE_SMS_INDICATION == UDP_INDICATION) {

            sprintf(&file_contents, "ME S U %s", aux_FileContens_Admin);
            return file_contents;
          } else if (BLE_SMS_INDICATION == SMS_INDICATION) {

            char UDP_Rsp[310] = {};
            sprintf(mqttInfo->data, "ME S U %s", aux_FileContens_Admin);
            // send_UDP_Send(UDP_Rsp);
            send_UDP_queue(mqttInfo);
            memset(file_contents, 0, sizeof(file_contents));
            sprintf(file_contents,
                    return_Json_SMS_Data("USER_HAS_BEEN_CHANGED_TO_ADMIN"),
                    myUser.phone);

            return file_contents;
          } else {
            // ////printf("\nchange user to admin 44 \n");

            return return_Json_SMS_Data("ERROR_INPUT_DATA");
          }
        } else {
          // ////printf("\nchange user to admin 57 \n");

          return return_Json_SMS_Data("ERROR_INPUT_DATA");
        }
      }
    }
  } else {
    if (BLE_SMS_INDICATION == SMS_INDICATION) {
      return return_Json_SMS_Data("ERROR_USER_NOT_FOUND");
    }

    // ////printf("\nchange user to admin 58 \n");
    return ERROR_USER_NOT_FOUND;
  }

  return return_Json_SMS_Data("ERROR_INPUT_DATA");
}
char *MyUser_change_ADMIN_To_USER(uint8_t BLE_SMS_INDICATION, char *payload,
                                  mqtt_information *mqttInfo) {
  char rsp_Data[300] = {};
  MyUser myUser;
  // ////printf("\nchange user to admin 11 \n");
  memset(&myUser, 0, sizeof(myUser));
  // ////printf("\nchange user to admin 22 \n");
  memset(file_contents, 0, sizeof(file_contents));

  if (MyUser_Search_User(payload, file_contents) == ESP_OK) {
    // ////printf("\nendHour  filecontents11 %s\n", file_contents);
    parse_ValidateData_User(file_contents, &myUser);
    // ////printf("\nendHour  filecontents22 %s\n", file_contents);
    // ////printf("\nchange user to admin 55 \n");
    //   myUser.permition = '0';
    // ////
    if (myUser.permition == '1') {

      // ////printf("\nchange user to admin 33 \n");
      uint8_t ACK_wi_antipassback = get_antipassback_user(myUser.wiegand_code);
      if (Myuser_deleteUser(&myUser) == ESP_OK) {
        // ////printf("\nchange user to admin 44 \n");
        myUser.permition = '0';
        if (MyUser_Add(&myUser) == ESP_OK) {

          if (ACK_wi_antipassback == ESP_OK) {
            erase_antipassback_user(myUser.wiegand_code);
            printf("\nchange user to admin 555 \n");
            set_antipassback_user(myUser.wiegand_code, '0');
            nvs_get_u32(nvs_System_handle, NVS_ANTIPASSBACK_PEOPLE_NUMBER,
                        &anti_passback_people_number);
            anti_passback_people_number++;
            ESP_LOGI("WIEGAND",
                     "wiegand1_action: setting antipassback people number %d",
                     anti_passback_people_number);
            nvs_set_u32(nvs_System_handle, NVS_ANTIPASSBACK_PEOPLE_NUMBER,
                        anti_passback_people_number);
          }

          memset(file_contents, 0, sizeof(file_contents));
          // memset(&myUser, 0, sizeof(myUser));
          // parse_ValidateData_User(file_contents, &myUser);

          if (BLE_SMS_INDICATION == BLE_INDICATION ||
              BLE_SMS_INDICATION == UDP_INDICATION) {
            MyUser_Search_User(myUser.phone, rsp_Data);

            memset(aux_FileContens_Admin, 0, sizeof(aux_FileContens_Admin));

            erase_Password_For_Rsp(rsp_Data, aux_FileContens_Admin);
            sprintf(&file_contents, "ME R U %s", aux_FileContens_Admin);
            return file_contents;
          } else if (BLE_SMS_INDICATION == SMS_INDICATION) {

            MyUser_Search_User(myUser.phone, rsp_Data);

            memset(aux_FileContens_Admin, 0, sizeof(aux_FileContens_Admin));

            erase_Password_For_Rsp(rsp_Data, aux_FileContens_Admin);

            char UDP_Rsp[310] = {};
            sprintf(mqttInfo->data, "ME R U %s", aux_FileContens_Admin);
            // send_UDP_Send(UDP_Rsp);
            send_UDP_queue(mqttInfo);
            // send_UDP_queue(UDP_Rsp);

            memset(file_contents, 0, sizeof(file_contents));
            sprintf(file_contents,
                    return_Json_SMS_Data("ADMIN_HAS_BEEN_CHANGED_TO_USER"),
                    myUser.phone);

            return file_contents;
          } else {
            // ////printf("\nchange user to admin 44 \n");

            return return_Json_SMS_Data("ERROR_INPUT_DATA");
          }
        }
      }
    }
  } else {
    if (BLE_SMS_INDICATION == SMS_INDICATION) {
      return return_Json_SMS_Data("ERROR_USER_NOT_FOUND");
    }
    return ERROR_USER_NOT_FOUND;
  }

  return return_Json_SMS_Data("ERROR_INPUT_DATA");
}

char *return_Start_BLE_Data() {

  // TODO: IR BUSCAR À MEMORIA O TEMPO DOS RELES PARA TER MAIS REDUNDANCIA
  nvs_get_u32(nvs_System_handle, NVS_KEY_GUEST_COUNTER, &GuestCountNumbers);

  nvs_get_u32(nvs_System_handle, NVS_ANTIPASSBACK_PEOPLE_NUMBER,
              &anti_passback_people_number);
  nvs_get_u32(nvs_System_handle, NVS_ANTIPASSBACK_PEOPLE_COUNTER,
              &antipassback_peopleCounter);

  memset(file_contents, 0, sizeof(file_contents));
  // GuestCountNumbers =
  sprintf(file_contents,
          "R1-%d R2-%d T1-%d T2-%d M1-%d M2-%d I1-%d I2-%d B-%d C-%d U-%d "
          "WI-%d A-%d WC-%d WU-%d RC-%d RX-%d",
          getRele1(), getRele2(), rele1_Bistate_Time, rele2_Bistate_Time,
          rele1_Mode_Label, rele2_Mode_Label, read_Input1(), read_Input2(),
          rele1_Restriction, get_User_Counter_From_Storage(), GuestCountNumbers,
          get_INT8_Data_From_Storage(NVS_AL_CONF_AL, nvs_System_handle),
          get_INT8_Data_From_Storage(NVS_ANTIPASSBACK_ACTIVATE_LABEL,
                                     nvs_System_handle),
          antipassback_peopleCounter, anti_passback_people_number,
          get_INT8_Data_From_Storage(NVS_RF_ROLLING_CODE, nvs_System_handle),get_INT8_Data_From_Storage(NVS_INPUT_REX_VALUE,nvs_System_handle));
  // ////printf("\nreturn_Start_BLE_Data - %s\n", file_contents);
  return file_contents;
}

char *get_System_TimeZone() {

  time_t now;
  struct tm timeinfo;
  char buffer[256];
  int buffer_len = 256;

  time(&now);
  /* setenv("TZ", "UTC1", 1);
  tzset(); */
  localtime_r(&now, &timeinfo);
  memset(file_contents, 0, sizeof(file_contents));
  strftime(file_contents, buffer_len, "%c", &timeinfo);

  return file_contents;
}

char *set_System_Clock(char *payload) {
  cron_stop();
  cron_job_clear_all();

  uint8_t dotCount = 0;
  uint8_t strIndex = 0;

  char str_Clock[50] = {};
  char timeZone[50] = {};
  printf("\n");
  // gpio_set_level(GPIO_OUTPUT_IO_1, 0);
  // gpio_set_level(GPIO_OUTPUT_IO_0, 0);
  label_MonoStableRelay1 = 0;
  label_MonoStableRelay2 = 0;
  // ////printf("\nset_System_Clock payload - %s\n", payload);

  for (int i = 0; i < strlen(payload); i++) {
    if (payload[i] == ';') {
      dotCount++;
      strIndex = 0;
    } else {
      if (dotCount == 0) {
        str_Clock[strIndex] = payload[i];
        strIndex++;
      } else if (dotCount == 1) {
        timeZone[strIndex] = payload[i];
        strIndex++;
      } else {
        return ERROR_INPUT_DATA;
      }
    }
  }

  struct tm timeinfo;
  struct tm timeinfo1;
  struct timeval epoch = {atoi(str_Clock), 0};
  gmtime_r(&epoch.tv_sec, &timeinfo1);

  PCF85_SetTime(timeinfo1.tm_hour, timeinfo1.tm_min, timeinfo1.tm_sec);
  PCF85_SetDate(timeinfo1.tm_wday, timeinfo1.tm_mday, (timeinfo1.tm_mon + 1),
                (timeinfo1.tm_year + 1900));

  time_t auxEpoch = epoch_Calculator(&timeinfo1);

  // ////printf("\n\n aux epoch %ld\n\n",auxEpoch);
  //  struct timeval *tv = &epoch;

  if (!settimeofday(&epoch, NULL)) {
    // ////printf("\tzset 2\n");
    if (!setenv("TZ", timeZone, 1)) {
      // ////printf("\n\n\n getenv - %s", getenv("TZ"));
      // ////printf("\tzset 3\n");
      save_STR_Data_In_Storage(NVS_TIMEZONE, timeZone, nvs_System_handle);
      tzset();
    } else {
      return "ERROR SET TIMEZONE";
    }
  } else {
    return "ERROR SET NEW TIME";
  }

  /* //////printf("\nERASE 34222\n");
  free(tv);
  //////printf("\nERASE 34333\n"); */
  time_t now;

  char buffer[256];
  int buffer_len = 256;

  time(&now);
  /* setenv("TZ", "UTC1", 1);
  tzset(); */
  localtime_r(&now, &timeinfo);
  memset(file_contents, 0, sizeof(file_contents));
  strftime(file_contents, buffer_len, "%c", &timeinfo);
  // ////printf("\nERASE 1 - %s\n", file_contents);

  // ////printf("\n\ntime time 111\n year - %d, month - %d, day - %d, dayweek -
  // %d, hour - %d, minute - %d, sec - %d\n\n", (timeinfo.tm_year + 1900),
  // timeinfo.tm_mon, timeinfo.tm_mday, timeinfo.tm_wday, timeinfo.tm_hour,
  // timeinfo.tm_min, timeinfo.tm_sec);

  uint8_t data[7];

  get_RTC_System_Time();

  // ////printf("\nERASE 2\n");
  /* list_destroy(&routine_list);
  //////printf("\nERASE 3\n");
  routine_list = (list_node *)malloc(sizeof(list_node));
  memset(&routine_list, 0, sizeof(routine_list));
*/
  routines_ID = 0;
  save_INT8_Data_In_Storage(NVS_ROUTINES_ID, routines_ID, nvs_System_handle);

  /* cron_stop();
  cron_job_clear_all();
*/
  initRoutines();

  return file_contents;
}

uint8_t start_Download_Sound_File() {

  label_SoundFile_Open = 0;

  char atCommand[50] = {};

  // vTaskSuspend(handle_SEND_SMS_TASK);

  // lastMonth = 5;
  remove("/sdcard/som.wav");

  EG91_send_AT_Command("AT+QFDEL=\"UFS:RECORD_SOUND.wav\"", "OK", 1000);

  sprintf(atCommand, " AT+QFOPEN=\"%s\"", "UFS:RECORD_SOUND.wav");

  if (EG91_send_AT_Command(atCommand, "QFOPEN", 1000)) {
    // ////printf("\n\n file ID Download %s\n\n\n", fileID);
    label_SoundFile_Open = 1;
    return 1;
  } else {
    return 0;
  }

  return 0;
}

char *parse_Start_Download_File(char *payload) {

  timer_pause(TIMER_GROUP_1, TIMER_0);
  // ////printf("\nsms\n");
  vTaskSuspend(xHandle_Timer_VerSystem);
  // disableAlarm();

  if (strlen(payload) > 1) {
    return return_Json_SMS_Data("ERROR_INPUT_DATA");
  }

  if (payload[0] == '1') {
    if (start_Download_Sound_File()) {
      return "OK";
    } else {
      return "ERROR";
    }
  } else if (payload[0] == '2') {
  }

  return "ERROR";
}
/// @brief
/// @param offset
/// @param daylight
void setTimeZone(long offset, int daylight) {
  char cst[17] = {0};
  char cdt[17] = "DST";
  char tz[33] = {0};

  if (offset % 3600) {
    sprintf(cst, "UTC%ld:%02u:%02u", offset / 3600, abs((offset % 3600) / 60),
            abs(offset % 60));
  } else {
    sprintf(cst, "UTC%ld", offset / 3600);
  }
  if (daylight != 3600) {
    long tz_dst = offset - daylight;
    if (tz_dst % 3600) {
      sprintf(cdt, "DST%ld:%02u:%02u", tz_dst / 3600, abs((tz_dst % 3600) / 60),
              abs(tz_dst % 60));
    } else {
      sprintf(cdt, "DST%ld", tz_dst / 3600);
    }
  }
  sprintf(tz, "%s%s", cst, cdt);
  setenv("TZ", tz, 1);
  tzset();
}

char *save_Language_TranslateFile(char *payload) {
  FILE *f = NULL;
  FILE *f1 = NULL;
  char fileName[50];

  // ////printf("\n\n\nfile translate 5\n\n\n");
  //  sprintf(fileName, "/sdcard/%s", "lang.txt");
  // ////printf("\n\n\nfile translate 6\n\n\n");

  // ////printf("\n\n\nfile translate 1\n\n\n");
  if (label_Block_Feedback_SMS == 0) {

    // ////printf("\n\n\nfile translate 2\n\n\n");
    f = fopen("/spiffs/aux_language.json", "w");

    crc32_Language_file = 0;

    crc32_Language_file =
        esp_rom_crc32_le(0, (uint8_t *)payload, strlen(payload));
    /* json_Translate_File = (char *)malloc(strlen(payload));
    strcpy(json_Translate_File, payload); */

    if (f == NULL) {
      // ////printf("\n\n\nfile translate 211\n\n\n");
    }

    f1 = fopen(fileName, "w");

    /* if (f1 == NULL)
    {
        //////printf("\n\n\nfile translate 2222\n\n\n");
    } */
    label_Block_Feedback_SMS = 1;
  } else {
    // ////printf("\n\n\nfile translate 3\n\n\n");
    f = fopen("/spiffs/aux_language.json", "a");
    f1 = fopen(fileName, "a");
    crc32_Language_file = esp_rom_crc32_le(crc32_Language_file,
                                           (uint8_t *)payload, strlen(payload));
  }

  // ////printf("\n\n\nfile translate 4\n\n\n");
  fprintf(f, "%s", payload);

  // ////printf("\n\n\nfile translate 7\n\n\n");
  //  fprintf(f1, "%s", payload);
  // ////printf("\n\n\nfile translate 8\n\n\n");

  fclose(f);
  // ////printf("\n\n\nfile translate 9\n\n\n");
  //  fclose(f1);
  // ////printf("\n\n\nfile translate 10\n\n\n");

  /*     for (int i = 0; i < filesize; i++)
     {
         fprintf(f, "%c", file[i]);
         //fwrite(file,1,sizeof(file),f);
    } */

  // fprintf(f, "%d", file);

  return "OK";
}

int jsonCounterTranslate = 0;

uint8_t system_Reset() {
  // ////printf("\n\n\n SYSTEM RESET DOING\n\n");
  //  timer_pause(TIMER_GROUP_0, TIMER_0);
  label_ResetSystem = 1;
  label_Semaphore_Reset_System = 1;

  uint8_t lastMonth = get_NVS_Last_Month();

  /*  if (lastMonth == 0)
   {
       if (get_RTC_System_Time())
       {
           lastMonth = nowTime.month;
       }
   } */

  char HW_Version[10] = {};

  size_t required_size = 0;
  if (nvs_get_str(nvs_System_handle, NVS_KEY_HW_VERSION, NULL,
                  &required_size) == ESP_OK) {
    // ////printf("\nrequire size %d\n", required_size);
    // ////printf("\nGET USERS NAMESPACE\n");
    if (nvs_get_str(nvs_System_handle, NVS_KEY_HW_VERSION, HW_Version,
                    &required_size) == ESP_OK) {
    }
  } else {
    return 0;
  }

  nvs_erase_all(nvs_System_handle);
  nvs_erase_all(nvs_Users_handle);
  nvs_erase_all(nvs_Owner_handle);
  nvs_erase_all(nvs_Admin_handle);
  nvs_erase_all(nvs_Routines_handle);
  nvs_erase_all(nvs_Feedback_handle);
  nvs_erase_all(nvs_Exeption_Days_handle);
  nvs_erase_all(nvs_wiegand_codes_users_handle);
  nvs_erase_all(nvs_wiegand_codes_admin_handle);
  nvs_erase_all(nvs_wiegand_codes_owner_handle);
  save_INT8_Data_In_Storage(NVS_KEY_OWNER_LABEL, 0, nvs_System_handle);

  uint8_t owner_Label1 =
      get_INT8_Data_From_Storage(NVS_KEY_OWNER_LABEL, nvs_System_handle);

  // ////printf("\n\n owner label reset11 %d\n\n", owner_Label1);

  save_NVS_Last_Month(lastMonth);
  // ////printf("\n\n owner label reset22 %d\n\n", owner_Label1);
  save_STR_Data_In_Storage(NVS_KEY_HW_VERSION, HW_Version, nvs_System_handle);

  // save_INT8_Data_In_Storage(NVS_NETWORK_PORTAL_REGISTER, 0,
  // nvs_System_handle); send_UDP_Send("ME R O");
  //desactivateUDP_network();
  // ////printf("\n\n owner label reset33 %d\n\n", owner_Label1);

  // //////printf("\n\n\n IMEI -%s / payload -%s\n\n\n",IMEI,payload);

  FILE *f = fopen("/spiffs/language.json", "r");

  if (f != NULL) {
    remove("/spiffs/language.json");
    fclose(f);
  }

gpio_set_level(GPIO_OUTPUT_ACT, 0);
 vTaskDelay(pdMS_TO_TICKS(1500));
  esp_restart();
  // ////printf("\n\nlabel_ResetSystem == 4\n\n");
  //  example_tg_timer_deinit(TIMER_GROUP_0, TIMER_0);
  //  EG91_send_AT_Command(AT_CSQ, "CSQ", 1000);
  //  timer_start(TIMER_GROUP_1, TIMER_0);
  //  // example_tg_timer_init(TIMER_GROUP_1, TIMER_0, true, 5);
  //  // xTaskCreate(&task_VerifySystem, "task_VerifySystem", 4 * 2048, NULL, 2
  //  /*32*/, &xHandle_Timer_VerSystem); label_Reset_Password_OR_System = 2;
  return 0;
}

void give_rdySem_Reset_System() { xSemaphoreGive(rdySem_Reset_System); }

/* uint8_t reset_Owner_Password(char *payload, data_EG91_Send_SMS *data_SMS)
{


} */

uint8_t check_IF_System_Have_SIM_and_PIN() {
  uint8_t result = 0;

  if (gpio_get_level(GPIO_INPUT_IO_SIMPRE)) {

    result++;
    // ////printf("\n\n cpin 1 \n\n");
    if (EG91_Check_IF_Have_PIN()) {
      // ////printf("\n\n cpin 2 \n\n");
      result++;
    }
  } else {
    return 0;
  }

  return result;
}

uint8_t block_Feedback_SMS() {
  if (save_INT8_Data_In_Storage(NVS_KEY_BLOCK_FEEDBACK_SMS, 1,
                                nvs_System_handle) == ESP_OK) {
    return 1;
  } else {
    return 0;
  }
}

uint8_t unlock_Feedback_SMS() {

  if (save_INT8_Data_In_Storage(NVS_KEY_BLOCK_FEEDBACK_SMS, 0,
                                nvs_System_handle) == ESP_OK) {
    return 1;
  } else {
    return 0;
  }
}

uint8_t get_Feedback_SMS() {

  uint8_t feedback_SMS_Value =
      get_INT8_Data_From_Storage(NVS_KEY_BLOCK_FEEDBACK_SMS, nvs_System_handle);

  if (feedback_SMS_Value == 255) {
    if (save_INT8_Data_In_Storage(NVS_KEY_BLOCK_FEEDBACK_SMS, 1,
                                  nvs_System_handle) == ESP_OK) {
      return 1;
    }
  }

  return feedback_SMS_Value;
}

char *UDP_activate_desativate_logs(uint8_t log_action) {
  if (save_INT8_Data_In_Storage(NVS_NETWORK_LABEL_SEND_LOGS, log_action,
                                nvs_System_handle) == ESP_OK) {
    // ////printf("\n\n LOGS LABEL %d\n\n", log_action);
    UDP_logs_label = log_action;
    return "OK";
  } else {
    return "ERROR";
  }
}

uint8_t MyUser_add_Admin_multi(uint8_t BLE_SMS_INDICATION, char *payload) {
  char phNumber[MYUSER_PHONE_SIZE];
  char frsName[20];
  char surName[4];
  memset(phNumber, 0, sizeof(phNumber));
  memset(frsName, 0, sizeof(frsName));
  memset(surName, 0, sizeof(surName));
  MyUser user_validateData; //= malloc(sizeof(MyUser));
  // memset(&user_validateData, 0, sizeof(user_validateData));

  int aux = 0;
  int strIndex = 0;
  for (int i = 0; i < strlen(payload); i++) {
    if (payload[i] == '.') {
      aux++;
      strIndex = 0;
    } else {
      if (aux == 0) {
        if (strIndex < 20) {
          phNumber[strIndex] = payload[i];
          strIndex++;
        } else {

          return 0;
        }
      } else if (aux == 1) {
        if (strIndex < 20) {
          frsName[strIndex] = payload[i];
          strIndex++;
        } else {

          return 0;
        }
      } else if (aux == 2) {
        surName[strIndex] = payload[i];
        strIndex++;
      }
    }
  }

  // ////printf("phone add admin %s\n", phNumber);
  // ////printf("first name add admin %s\n", frsName);
  // ////printf("surname name add admin %s\n", surName);

  if (strlen(phNumber) < 20 && strlen(phNumber) > 1) {
    if (strlen(frsName) > 2) {
      sprintf(user_validateData.firstName, "%s", frsName);
    } else {
      sprintf(user_validateData.firstName, "%s", "S/N");
    }

    if (get_RTC_System_Time() == 0) {
      return 0;
    }

    char auxPhoneNumber[20] = {};

    sprintf(auxPhoneNumber, "%s", removeSpacesFromStr(phNumber));
    memset(phNumber, 0, sizeof(phNumber));

    sprintf(phNumber, "%s", auxPhoneNumber);

    sprintf(user_validateData.phone, "%s", phNumber);
    sprintf(user_validateData.start.date, "%d", nowTime.date);
    sprintf(user_validateData.start.hour, "%s", "0000");
    sprintf(user_validateData.end.days, "%c", '*');
    sprintf(user_validateData.end.hour, "%s", "2359");
    sprintf(user_validateData.key, "%s", DEFAULT_ADMIN_PASSWORD);
    sprintf(user_validateData.week, "%s", "1111111");
    user_validateData.permition = '1';
    user_validateData.relayPermition = '0';
    user_validateData.ble_security = '0';
    user_validateData.erase_User_After_Date = '0';

    // ////printf("new user bff 1313 %s\n", file_contents);

    char auxfileContents[200];

    if (strlen(user_validateData.start.date) > 6) {
      return 0;
    }

    memset(file_contents, 0, sizeof(file_contents));
    if (MyUser_Search_User(user_validateData.phone, auxfileContents) !=
        ESP_OK) {
      // ////printf("\n\nuser_validateData.phone add admin %s\n\n",
      // user_validateData.phone);
      uint16_t ACK_Add_User = MyUser_Add(&user_validateData);
      if (ACK_Add_User == ESP_OK) {
        if (BLE_SMS_INDICATION == BLE_INDICATION ||
            BLE_SMS_INDICATION == UDP_INDICATION) {

          return 1;
        }
      } else if (ACK_Add_User == 128) {
        return 0;
      } else {
        if (BLE_SMS_INDICATION == BLE_INDICATION ||
            BLE_SMS_INDICATION == UDP_INDICATION) {

          return 0;
        }
      }
    } else {
      if (BLE_SMS_INDICATION == BLE_INDICATION ||
          BLE_SMS_INDICATION == UDP_INDICATION) {
        return 0;
      }
    }
    // MyUser_List_AllUsers();
  } else {
    if (BLE_SMS_INDICATION == BLE_INDICATION ||
        BLE_SMS_INDICATION == UDP_INDICATION) {

      return 0;
    } else if (BLE_SMS_INDICATION == SMS_INDICATION) {

      return 0;
    }
  }

  return 0;
}

char *parse_SystemData(uint8_t BLE_SMS_Indication, char cmd, char param,
                       char *phPassword, char *payload,
                       MyUser *user_validateData, data_EG91_Send_SMS *data_SMS,
                       mqtt_information *mqttInfo) {

  char *rsp;
  /*memset(&rsp, 0, sizeof(rsp));*/

  // ////printf("\n\n cmd system %c\n", cmd);
  // ////printf("\n\n param system %c\n", param);

  if (cmd == SET_CMD) {
    if (param == NAME_PARAMETER) {
      if (user_validateData->permition == '2') {
        if (strlen(payload) < 22) {
          char rsp_BLE_Name[31] = {};
          if (!strcmp(change_BLE_Name(payload, &rsp_BLE_Name), "OK")) {

            save_STR_Data_In_Storage(NVS_KEY_BLE_NAME, payload,
                                     nvs_System_handle);

            if (BLE_SMS_Indication == BLE_INDICATION ||
                BLE_SMS_Indication == UDP_INDICATION) {

              asprintf(&rsp, "%s %c %c %s", ADMIN_ELEMENT, cmd, param,
                       rsp_BLE_Name);
              // ////printf("rsp %s\n", rsp);
              return rsp;
            } else if (BLE_SMS_Indication == SMS_INDICATION) {

              char UDP_Rsp[50] = {};
              sprintf(UDP_Rsp, "%s %c %c %s", ADMIN_ELEMENT, cmd, param,
                      rsp_BLE_Name);
              // send_UDP_Send(UDP_Rsp);

              asprintf(&rsp,
                       return_Json_SMS_Data("SYSTEM_NAME_HAS_BEEN_CHANGED_TO"),
                       rsp_BLE_Name);
              return rsp;
            }
          } else {
            return return_ERROR_Codes(&rsp,
                                      return_Json_SMS_Data("ERROR_INPUT_DATA"));
          }
        } else {
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_INPUT_NAME_BLE_SIZE"));
        }
      } else {

        // ////printf(ERROR_USER_NOT_PERMITION);
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }

      return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
    }
    if (param == EG91_FOTA_PARAMETER) {
      send_UDP_Send("»\0", mqttInfo->topic);
      char *endptr;
      EG915_readDataFile_struct.ckm =
          strtoul(payload, &endptr, 10); // atoi(payload);
      EG915_fota(&mqttInfo);
      asprintf(&rsp, "%s", "NTRSP");
      return rsp;
    } else if (param == ADMIN_PARAMETER) {
      if (user_validateData->permition == '2') {
        /*  if (BLE_SMS_Indication == SMS_INDICATION)
         {
             data_SMS->labelRsp = 1;
         } */

        asprintf(&rsp, "%s",
                 MyUser_add_Admin(BLE_SMS_Indication, payload, mqttInfo));
        return rsp;
      } else {
        // ////printf(ERROR_USER_NOT_PERMITION);
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == SMS_TRANSLATE_PARAMETER) {
      if (BLE_SMS_Indication == BLE_INDICATION ||
          BLE_SMS_Indication == UDP_INDICATION) {

        if (user_validateData->permition == '2') {
          changeInputTranslators(payload);
          asprintf(&rsp, "%s", "ME S I OK");
          return rsp;
        } else {

          // //////printf(ERROR_USER_NOT_PERMITION);
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
        }
      }
    } else if (param == INPUT_REX_PARAMETER) {
      if (BLE_SMS_Indication == BLE_INDICATION ||
          BLE_SMS_Indication == UDP_INDICATION) {

        if (user_validateData->permition == '2' ||
            user_validateData->permition == '1') {
         
          if (atoi(payload) >= 0 && atoi(payload) <= 3) {
            nvs_set_u8(nvs_System_handle, NVS_INPUT_REX_VALUE, atoi(payload));
            asprintf(&rsp, "ME S X %d",atoi(payload));
            return rsp;
          } else {
            return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
          }
        } else {
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
        }
      } else {
        return return_ERROR_Codes(&rsp,
                                  return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
      }
    } else if (param == SMS_CALL_VERIFICATION_PARAMETER) {
      if (user_validateData->permition == '2') {
        // //printf("\n\n sms verify444 %d\n\n", atoi(payload));
        if (atoi(payload) < 0 || atoi(payload) > 4) {
          return return_ERROR_Codes(&rsp,
                                    return_Json_SMS_Data("ERROR_INPUT_DATA"));
        } else {
          save_INT8_Data_In_Storage(NVS_SMS_CALL_VERIFICATION, payload[0] - 48,
                                    nvs_System_handle);
        }
        asprintf(&rsp, "%s %c %c %d", ADMIN_ELEMENT, cmd, param, atoi(payload));
        return rsp;
      } else {

        // //////printf(ERROR_USER_NOT_PERMITION);
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == REDITECT_SMS_PARAMETER) {
      if (user_validateData->permition == '2') {
        save_INT8_Data_In_Storage(NVS_REDIRECT_SMS, 1, nvs_System_handle);

        if (BLE_SMS_Indication == SMS_INDICATION) {
          asprintf(&rsp, "%s", return_Json_SMS_Data("REDIRECT_SMS_ACTIVATED"));
          sprintf(mqttInfo->data, "%s", "ME S R 1");
          send_UDP_queue(mqttInfo);
          return rsp;
        } else {

          asprintf(&rsp, "%s", "ME S R 1");
          return rsp;
        }
      } else {

        // //////printf(ERROR_USER_NOT_PERMITION);
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == BLOCK_FEEDBACK_SMS_PARAMETER) {
      if (user_validateData->permition == '2') {
        uint8_t block_Feedback_Value = block_Feedback_SMS();

        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {
          asprintf(&rsp, "%s %c %c %d", ADMIN_ELEMENT, cmd, param,
                   block_Feedback_Value);
          return rsp;
        } else if (BLE_SMS_Indication == SMS_INDICATION) {
          if (block_Feedback_Value == 1) {

            char UDP_Rsp[50] = {};
            sprintf(mqttInfo->data, "%s %c %c %d", ADMIN_ELEMENT, cmd, param,
                    block_Feedback_Value);
            // send_UDP_Send(UDP_Rsp);
            send_UDP_queue(mqttInfo);

            asprintf(&rsp, "%s",
                     return_Json_SMS_Data("SMS_FEEDBACK_BLOCK_ACTIVATED"));
            return rsp;
          } else {
            return return_ERROR_Codes(
                &rsp, return_Json_SMS_Data("ERROR_IN_ACTIVATING_SMS_FEEDBACK"));
          }
        }
      } else {
        // ////printf(ERROR_USER_NOT_PERMITION);
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == CLOCK_PARAMETER) {
      if (BLE_SMS_Indication == BLE_INDICATION ||
          BLE_SMS_Indication == UDP_INDICATION) {
        if (user_validateData->permition == '2') {
          /*memset(&rsp, 0, sizeof(rsp));*/
          asprintf(&rsp, "%s %c %c %s", ADMIN_ELEMENT, cmd, param,
                   set_System_Clock(payload));
          // ////printf("rsp %s\n", rsp);
          return rsp;
        } else {
          // ////printf(ERROR_USER_NOT_PERMITION);
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
        }
      }
      return return_ERROR_Codes(&rsp,
                                return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
    } else if (param == LANGUAGE_FILE_PARAMETER) {
      if (BLE_SMS_Indication == BLE_INDICATION) {
        if (user_validateData->permition == '2') {
          /*memset(&rsp, 0, sizeof(rsp));*/
          // ////printf("\n\n\n receive translate \n\n\n");
          label_BLE_UDP_send = 0;
          asprintf(&rsp, "%s %c %c %s", ADMIN_ELEMENT, cmd, param,
                   save_Language_TranslateFile(payload));
          // ////printf("rsp %s\n", rsp);
          return rsp;
        } else {
          // ////printf(ERROR_USER_NOT_PERMITION);
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
        }
      }
      return return_ERROR_Codes(&rsp,
                                return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
    } else if (param == USER_PARAMETER) {
      if (user_validateData->permition == '2') {
        asprintf(
            &rsp, "%s",
            MyUser_change_USER_To_ADMIN(BLE_SMS_Indication, payload, mqttInfo));
        return rsp;
      } else {

        // ////printf(ERROR_USER_NOT_PERMITION);
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == M200_RELAYS_CONFIGURATION_PARAMETER) {
      if (user_validateData->permition == '2' ||
          user_validateData->permition == '1') {
        asprintf(&rsp, "%s",
                 configuration_Relay_Parameter(payload, BLE_SMS_Indication,
                                               mqttInfo));
        return rsp;
      } else {

        // ////printf(ERROR_USER_NOT_PERMITION);
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == SOUND_PARAMETER) {
      label_BLE_UDP_send = 0;
      if (user_validateData->permition == '2') {
        asprintf(&rsp, "ME S S %s", set_Alarm_Sound(payload));
        return rsp;
      } else {

        // ////printf(ERROR_USER_NOT_PERMITION);
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == NETWORK_LABEL_PARAMETER) {
      if (BLE_SMS_Indication == BLE_INDICATION ||
          BLE_SMS_Indication == UDP_INDICATION) {
        uint8_t InitNetworkCount = 0;

        if (user_validateData->permition == '2') {
          save_INT8_Data_In_Storage(NVS_QMT_LARGE_DATA_TIMER_LABEL, 1,
                                    nvs_System_handle);
          nvs_erase_key(nvs_System_handle, NVS_NETWORK_LOCAL_CHANGED);
          // save_INT8_Data_In_Storage(NVS_NETWORK_LOCAL_CHANGED, 3,
          // nvs_System_handle);
          asprintf(&rsp, "ME S W %s", activateUDP_network());
          save_INT8_Data_In_Storage(NVS_QMT_LARGE_DATA_TIMER_LABEL, 0,
                                    nvs_System_handle);
          return rsp;
        } else {
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_LOGS_USER_NOT_PERMITION"));
        }
      } else {
        return return_ERROR_Codes(&rsp,
                                  return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
      }
    } else if (param == NETWORK_LOGS_LABEL_PARAMETER) {

      if (BLE_SMS_Indication == UDP_INDICATION) {

        if (user_validateData->permition == '2') {
          asprintf(&rsp, "ME S E %s", UDP_activate_desativate_logs(1));
          return rsp;
        } else {
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_LOGS_USER_NOT_PERMITION"));
        }
      } else {
        return return_ERROR_Codes(&rsp, "ERROR");
      }
    } else {
      return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_PARAMETER"));
    }
  } else if (cmd == GET_CMD) {
    if (param == NAME_PARAMETER) {
      if (user_validateData->permition == '2' ||
          user_validateData->permition == '1') {
        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {
          /*memset(&rsp, 0, sizeof(rsp));*/
          asprintf(&rsp, "%s %c %c %s", ADMIN_ELEMENT, cmd, param,
                   (char *)SYSTEM_NAME);
          return rsp;
        } else if (BLE_SMS_Indication == SMS_INDICATION) {
          /*memset(&rsp, 0, sizeof(rsp));*/
          asprintf(&rsp, return_Json_SMS_Data("GET_NAME_SYSTEM"),
                   (char *)SYSTEM_NAME);
          return rsp;
        }
      } else {

        // ////printf(ERROR_USER_NOT_PERMITION);
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } 
    else if (param == INPUT_REX_PARAMETER) {
      if (BLE_SMS_Indication == BLE_INDICATION ||
          BLE_SMS_Indication == UDP_INDICATION) {

        if (user_validateData->permition == '2' ||
            user_validateData->permition == '1') {
         
          uint8_t rex = get_INT8_Data_From_Storage(NVS_INPUT_REX_VALUE,nvs_System_handle);
          asprintf(&rsp, "%s %c %c %d", ADMIN_ELEMENT, cmd, param,rex);
          return rsp;
        } else {
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
        }
      } else {
        return return_ERROR_Codes(&rsp,
                                  return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
      } 
    }
    else if (param == GET_IMEI_PARAMETER) {
      char imei[20] = {};
      size_t required_size = 0;
      if (EG91_get_IMEI()) {
        if (nvs_get_str(nvs_System_handle, NVS_KEY_EG91_IMEI, NULL,
                        &required_size) == ESP_OK) {
          if (nvs_get_str(nvs_System_handle, NVS_KEY_EG91_IMEI, imei,
                          &required_size) == ESP_OK) {
            asprintf(&rsp, "%s %c %c %s", ADMIN_ELEMENT, cmd, param, imei);
            return rsp;
          }
        }
      }
    } else if (param == SMS_CALL_VERIFICATION_PARAMETER) {
      if (user_validateData->permition == '2') {
        uint8_t verification_SMS_CALL = get_INT8_Data_From_Storage(
            NVS_SMS_CALL_VERIFICATION, nvs_System_handle);

        if (verification_SMS_CALL == 255) {
          save_INT8_Data_In_Storage(NVS_SMS_CALL_VERIFICATION, 0,
                                    nvs_System_handle);
          verification_SMS_CALL = 0;
        }

        asprintf(&rsp, "%s %d", "ME G V", verification_SMS_CALL);
        return rsp;
      } else {

        // //////printf(ERROR_USER_NOT_PERMITION);
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == REDITECT_SMS_PARAMETER) {

      if (user_validateData->permition == '2') {
        uint8_t redirectSMS_value =
            get_INT8_Data_From_Storage(NVS_REDIRECT_SMS, nvs_System_handle);

        if (redirectSMS_value == 255) {
          save_INT8_Data_In_Storage(NVS_REDIRECT_SMS, 0, nvs_System_handle);
        }

        if (BLE_SMS_Indication == SMS_INDICATION) {
          if (redirectSMS_value == 1) {
            asprintf(&rsp, "%s", return_Json_SMS_Data("REDIRECT_SMS_ACTIVE"));
            return rsp;
          } else {
            asprintf(&rsp, "%s",
                     return_Json_SMS_Data("REDIRECT_SMS_DESACTIVE"));
            return rsp;
          }
        } else {
          asprintf(&rsp, "ME G R %d", redirectSMS_value);
          return rsp;
        }
      } else {

        // //////printf(ERROR_USER_NOT_PERMITION);
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == SIM_IMEI_ESPMAC_PARAMETER) {
      if (user_validateData->permition == '2') {
        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {
          /* code */

          if (BLE_SMS_Indication == BLE_INDICATION ||
              BLE_SMS_Indication == UDP_INDICATION) {
            asprintf(&rsp, "%s %c %c %d", ADMIN_ELEMENT, cmd, param,
                     check_IF_System_Have_SIM_and_PIN());
          }

          return rsp;
        }
      } else {
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == SOUND_PARAMETER) {
      if (user_validateData->permition == '2') {
        asprintf(&rsp, "%s", play_Alert_Sound());
        return rsp;
      } else {
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    }

    else if (param == CLOCK_PARAMETER) {
      if (user_validateData->permition == '2') {
        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {
          if (get_RTC_System_Time()) {
            asprintf(&rsp, "ME G C %d;%d", nowTime.date, nowTime.time);
            return rsp;
          } else {
            return return_ERROR_Codes(&rsp, "ERROR_GET_RTC\n<60>");
          }
        } else {
          return return_ERROR_Codes(&rsp,
                                    return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
        }
      } else {
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == M200_FIRMWARE_HARDWARE_PARAMETER) {
      if (user_validateData->permition == '2') {
        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {

          char HW_Version[10] = {};

          size_t required_size = 0;
          if (nvs_get_str(nvs_System_handle, NVS_KEY_HW_VERSION, NULL,
                          &required_size) == ESP_OK) {
            // ////printf("\nrequire size %d\n", required_size);
            // ////printf("\nGET USERS NAMESPACE\n");
            if (nvs_get_str(nvs_System_handle, NVS_KEY_HW_VERSION, HW_Version,
                            &required_size) != ESP_OK) {
              return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_GET"));
            }
          } else {
            return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_GET"));
          }
          // #include <esp_https_ota.h>
          // esp_https_ota()
          uint8_t label_UDP_fail_and_changed = get_INT8_Data_From_Storage(
              NVS_NETWORK_LOCAL_CHANGED, nvs_System_handle);

          /*   if (label_UDP_fail_and_changed == 255)
            {
                label_UDP_fail_and_changed = 0;
                save_INT8_Data_In_Storage(NVS_NETWORK_LOCAL_CHANGED,
            label_UDP_fail_and_changed, nvs_System_handle);
            } */
          // TODO:MUDAR A VERSAO DE HW PARA A VARIAVEL
          asprintf(&rsp, "ME G M %s;%s;%s", FW_VERSION, HW_Version, "d03");

          return rsp;
        } else {
          return return_ERROR_Codes(&rsp,
                                    return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
        }
      } else {
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == SYSTEM_BLE_START_PARAMETER) {
      if (BLE_SMS_Indication == BLE_INDICATION ||
          BLE_SMS_Indication == UDP_INDICATION) {
        /*memset(&rsp, 0, sizeof(rsp));*/
        asprintf(&rsp, "%s %c %c %s", ADMIN_ELEMENT, cmd, param,
                 return_Start_BLE_Data());
        // ////printf("rsp %s\n", rsp);
        return rsp;
      } else {
        return return_ERROR_Codes(&rsp,
                                  return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
      }
    } else if (param == BLOCK_FEEDBACK_SMS_PARAMETER) {
      if (user_validateData->permition == '2') {
        uint8_t block_Feedback_Value = get_Feedback_SMS();

        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {
          asprintf(&rsp, "%s %c %c %d", ADMIN_ELEMENT, cmd, param,
                   block_Feedback_Value);
          return rsp;
        } else if (BLE_SMS_Indication == SMS_INDICATION) {
          if (block_Feedback_Value == 1) {
            asprintf(&rsp, "%s",
                     return_Json_SMS_Data("SMS_FEEDBACK_BLOCK_ARE_ACTIVATED"));
            return rsp;
          } else if (block_Feedback_Value == 0) {
            asprintf(&rsp, "%s",
                     return_Json_SMS_Data("SMS_FEEDBACK_BLOCK_ARE_DISABLED"));
            return rsp;
          } else {
            return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_GET"));
          }
        }
      } else {
        // ////printf(ERROR_USER_NOT_PERMITION);
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    }

    else if (param == SIGNAL_PARAMETER) {
      if (user_validateData->permition == '2' ||
          user_validateData->permition == '1') {

        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {
          if (BLE_SMS_Indication == UDP_INDICATION) {
            asprintf(&rsp, "%s %c %c %d", ADMIN_ELEMENT, cmd, param,
                     RSSI_VALUE);
            return rsp;
          }

          if (check_IF_System_Have_SIM_and_PIN() == 2) {
            EG91_send_AT_Command("AT+CSQ", "CSQ", 500);
            /*memset(&rsp, 0, sizeof(rsp));*/
            asprintf(&rsp, "%s %c %c %d", ADMIN_ELEMENT, cmd, param,
                     RSSI_VALUE);
            return rsp;
          } else {
            asprintf(&rsp, "%s %c %c %s", ADMIN_ELEMENT, cmd, param, "99");
            return rsp;
          }
        } else if (BLE_SMS_Indication == SMS_INDICATION) {
          give_rdySem_Control_Send_AT_Command();
          EG91_send_AT_Command("AT+CSQ", "CSQ", 500);
          /*memset(&rsp, 0, sizeof(rsp));*/
          if ((RSSI_VALUE >= 20 && RSSI_VALUE <= 31) ||
              (RSSI_VALUE >= 120 && RSSI_VALUE <= 131)) {
            asprintf(&rsp, "%s", return_Json_SMS_Data("SYSTEM_RSSI_EXCELLENT"));
          } else if ((RSSI_VALUE >= 15 && RSSI_VALUE <= 19) ||
                     (RSSI_VALUE >= 115 && RSSI_VALUE <= 119)) {
            asprintf(&rsp, "%s", return_Json_SMS_Data("SYSTEM_RSSI_GOOD"));
          } else if ((RSSI_VALUE >= 10 && RSSI_VALUE <= 14) ||
                     (RSSI_VALUE >= 110 && RSSI_VALUE <= 114)) {
            asprintf(&rsp, "%s", return_Json_SMS_Data("SYSTEM_RSSI_WEAK"));
          } else if ((RSSI_VALUE >= 0 && RSSI_VALUE <= 9) ||
                     (RSSI_VALUE >= 100 && RSSI_VALUE <= 109)) {
            asprintf(&rsp, "%s", return_Json_SMS_Data("SYSTEM_RSSI_VERY_WEAK"));
          } else if (RSSI_VALUE == 99 || RSSI_VALUE == 199) {
            asprintf(&rsp, "%s",
                     return_Json_SMS_Data("SYSTEM_RSSI_NOT_DETECT"));
          }

          return rsp;
        } else {
          return return_ERROR_Codes(&rsp,
                                    return_Json_SMS_Data("ERROR_INPUT_DATA"));
        }
      } else {

        // ////printf(ERROR_USER_NOT_PERMITION);
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == LAST_PARAMETER) {
      char phoneNumber[20];
      if (user_validateData->permition == '2') {
        if (get_Data_STR_LastCALL_From_Storage(&phoneNumber) == ESP_OK) {
          // ////printf("\n last phnumber print %s\n", phoneNumber);
          if (BLE_SMS_Indication == BLE_INDICATION ||
              BLE_SMS_Indication == UDP_INDICATION) {
            /*memset(&rsp, 0, sizeof(rsp));*/
            asprintf(&rsp, "%s %c %c %s", ADMIN_ELEMENT, cmd, param,
                     phoneNumber);
            return rsp;
          } else if (BLE_SMS_Indication == SMS_INDICATION) {
            /*memset(&rsp, 0, sizeof(rsp));*/
            asprintf(
                &rsp,
                return_Json_SMS_Data("THE_LAST_CALL_WAS_MADE_BY_THE_NUMBER"),
                phoneNumber);
            return rsp;
          } else {
            return return_ERROR_Codes(&rsp,
                                      return_Json_SMS_Data("ERROR_INPUT_DATA"));
          }
        } else {
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("LAST_CALL_NOT_SAVED"));
        }
      } else {

        // ////printf(ERROR_USER_NOT_PERMITION);
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    }
  } else if (cmd == RESET_CMD) {
    if (param == NAME_PARAMETER) {
      if (user_validateData->permition == '2') {
        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {
          /*memset(&rsp, 0, sizeof(rsp));*/
          asprintf(&rsp, "%s %c %c %s", ADMIN_ELEMENT, cmd, param, "MOTORLINE");
          // ////printf("rsp %s\n", rsp);
          memset(SYSTEM_NAME, 0, sizeof(SYSTEM_NAME));
          sprintf(SYSTEM_NAME, "%s", "MOTORLINE");
          return rsp;
        } else if (BLE_SMS_Indication == SMS_INDICATION) {
          /*memset(&rsp, 0, sizeof(rsp));*/
          memset(SYSTEM_NAME, 0, sizeof(SYSTEM_NAME));
          sprintf(SYSTEM_NAME, "%s", "MOTORLINE");
          asprintf(&rsp,
                   return_Json_SMS_Data("SYSTEM_NAME_HAS_BEEN_CHANGED_TO"),
                   "MOTORLINE");
          return rsp;
        } else {
          return return_ERROR_Codes(&rsp,
                                    return_Json_SMS_Data("ERROR_INPUT_DATA"));
        }
      } else {

        // ////printf(ERROR_USER_NOT_PERMITION);
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == NETWORK_LABEL_PARAMETER) {

      if (BLE_SMS_Indication == BLE_INDICATION ||
          BLE_SMS_Indication == UDP_INDICATION) {
        uint8_t InitNetworkCount = 0;

        if (user_validateData->permition == '2') {
          asprintf(&rsp, "ME R W %d", desactivateUDP_network());
          return rsp;
        } else {
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_LOGS_USER_NOT_PERMITION"));
        }
      } else {
        return return_ERROR_Codes(&rsp,
                                  return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
      }
    } else if (param == NETWORK_LOGS_LABEL_PARAMETER) {

      if (BLE_SMS_Indication == UDP_INDICATION) {

        if (user_validateData->permition == '2') {
          asprintf(&rsp, "ME R E %s", UDP_activate_desativate_logs(0));
          return rsp;
        } else {
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_LOGS_USER_NOT_PERMITION"));
        }
      } else {
        return return_ERROR_Codes(&rsp, "ERROR");
      }
    } else if (param == OWNER_PARAMETER) {
      if (user_validateData->permition == '2') {
        if (BLE_SMS_Indication == SMS_INDICATION) {

          data_SMS->labelRsp = 1;

          asprintf(&rsp, "%s", change_Owner(payload, SMS_INDICATION));
          return rsp;
        } else if (BLE_SMS_Indication == BLE_INDICATION ||
                   BLE_SMS_Indication == UDP_INDICATION) {
          asprintf(&rsp, "%s %c %c %s", ADMIN_ELEMENT, cmd, param,
                   change_Owner(payload, BLE_INDICATION));
        }

        // ////printf("\n\n change owner 1\n\n");

        return rsp;

        /*  else if ()
         {
         } */
        /* else
        {
            return return_ERROR_Codes(&rsp,
        return_Json_SMS_Data("ERROR_INPUT_DATA"));
        } */
      } else {

        // ////printf(ERROR_USER_NOT_PERMITION);
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == REDITECT_SMS_PARAMETER) {
      if (user_validateData->permition == '2') {
        save_INT8_Data_In_Storage(NVS_REDIRECT_SMS, 0, nvs_System_handle);

        if (BLE_SMS_Indication == SMS_INDICATION) {
          asprintf(&rsp, "%s",
                   return_Json_SMS_Data("REDIRECT_SMS_DESACTIVATED"));
          sprintf(mqttInfo->data, "%s", "ME R R 1");
          send_UDP_queue(mqttInfo);
          return rsp;
        } else {
          asprintf(&rsp, "%s", "ME R R 0");
          return rsp;
        }
      } else {

        // //////printf(ERROR_USER_NOT_PERMITION);
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == FORMAT_SDCARD_PARAMETER) {
      if (user_validateData->permition == '2') {
        if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD)) {
          // ////printf("\n\nSDCARD 0\n\n");
          if (format_sdcard() == ESP_OK) {
            nvs_set_u8(nvs_System_handle, NVS_LAST_MONTH, 0);
            if (BLE_SMS_Indication == BLE_INDICATION ||
                BLE_SMS_Indication == UDP_INDICATION) {
              label_BLE_UDP_send = 0;
              asprintf(&rsp, "%s %c %c %s", ADMIN_ELEMENT, cmd, param, "OK");
            } else if (BLE_SMS_Indication == SMS_INDICATION) {
              char UDP_Rsp[50] = {};
              sprintf(UDP_Rsp, "%s %c %c %s", ADMIN_ELEMENT, cmd, param, "OK");
              // send_UDP_Send(UDP_Rsp);

              asprintf(&rsp, return_Json_SMS_Data("SD_CARD_FORMAT"));
            } else {
              asprintf(&rsp, return_Json_SMS_Data("ERROR_SD_CARD_FORMAT"));
            }
          } else {
            asprintf(&rsp, return_Json_SMS_Data("ERROR_SD_CARD_FORMAT"));
          }
        } else {
          asprintf(&rsp, return_Json_SMS_Data("ERROR_SD_CARD_NOT_INSERTED"));
        }

        return rsp;
      } else {
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == SIM_IMEI_ESPMAC_PARAMETER) {
      if (user_validateData->permition == '2') {
        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {
          if (EG91_Unlock_SIM_PIN(payload)) {
            asprintf(&rsp, "%s %c %c %s", ADMIN_ELEMENT, cmd, param, "OK");
          } else {
            asprintf(&rsp, "%s %c %c %s", ADMIN_ELEMENT, cmd, param, "ERROR");
          }
        }

        return rsp;
      } else {

        // ////printf(ERROR_USER_NOT_PERMITION);
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == BLOCK_FEEDBACK_SMS_PARAMETER) {
      if (user_validateData->permition == '2') {
        uint8_t block_Feedback_Value = unlock_Feedback_SMS();

        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {
          asprintf(&rsp, "%s %c %c %d", ADMIN_ELEMENT, cmd, param,
                   block_Feedback_Value);
          return rsp;
        } else if (BLE_SMS_Indication == SMS_INDICATION) {
          if (block_Feedback_Value == 1) {
            char UDP_Rsp[50] = {};
            sprintf(mqttInfo->data, "%s %c %c %d", ADMIN_ELEMENT, cmd, param,
                    block_Feedback_Value);
            // send_UDP_Send(UDP_Rsp);
            send_UDP_queue(mqttInfo);

            asprintf(&rsp, "%s",
                     return_Json_SMS_Data("SMS_FEEDBACK_BLOCK_DISABLED"));
            return rsp;
          } else {
            return return_ERROR_Codes(&rsp,
                                      return_Json_SMS_Data("ERROR_RESET"));
          }
        }
      } else {
        // ////printf(ERROR_USER_NOT_PERMITION);
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == USER_PARAMETER) {
      if (user_validateData->permition == '2') {
        asprintf(
            &rsp, "%s",
            MyUser_change_ADMIN_To_USER(BLE_SMS_Indication, payload, mqttInfo));
        return rsp;
      } else {

        // ////printf(ERROR_USER_NOT_PERMITION);
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == ADMIN_PARAMETER) {
      if (user_validateData->permition == '2') {
        asprintf(&rsp, "%s",
                 MyUser_erase_Admin(BLE_SMS_Indication, payload, mqttInfo));
        return rsp;
      } else {

        // ////printf(ERROR_USER_NOT_PERMITION);
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == START_DOWNLOAD_FILE) {
      int file_Lenght;
      char ch;

      label_BLE_UDP_send = 0;
      if (user_validateData->permition == '2') {
        char *json_Translate_File;
        // printf("\n\ncrc32 lang11\n\n\n\n");
        FILE *f = fopen("/spiffs/aux_language.json", "r");
        label_Block_Feedback_SMS = 0;
        char ckf[9] = {};
        if (f != NULL) {

          // printf("\n\ncrc32 lang22\n\n\n\n");

          sprintf(ckf, "%X", crc32_Language_file);
          // printf("\n\ncrc32 ckf lang\n%s - %s\n\n\n", ckf, payload);

          if (!strcmp(ckf, payload)) {
            FILE *f_aux = fopen("/spiffs/language.json", "r");
            // printf("\n\ncrc32 lang33\n\n\n\n");
            if (f_aux != NULL) {

              fclose(f_aux);
              remove("/spiffs/language.json");
              // printf("\n\n f_aux != NULL 55\n\n");
              fclose(f);
              rename("/spiffs/aux_language.json", "/spiffs/language.json");

              f = fopen("/spiffs/language.json", "r");
            } else {
              // printf("\n\n f_aux == NULL 1212\n\n");
              fclose(f);
              // printf("\n\ncrc32 lang77\n\n\n\n");
              rename("/spiffs/aux_language.json", "/spiffs/language.json");
              f = fopen("/spiffs/language.json", "r");
            }

            // printf("\n\n\n end lang files ERROR\n\n\n");

            // ////printf("\n\n\n end lang files 1\n\n\n");
            fseek(f, 0L, SEEK_END);
            file_Lenght = ftell(f);
            fseek(f, 0L, SEEK_SET);
            // rewind(f);
            // printf("\n\n\n end lang files 3 - %d\n\n\n", file_Lenght);

            // ////printf("\n\n\n ckf OK \n\n\n");

            json_Translate_File = (char *)malloc(file_Lenght * sizeof(char));
            memset(json_Translate_File, 0, file_Lenght);

            // printf("\n\n\n end lang files 4\n\n\n");

            if (json_Translate_File) {
              // printf("\n\n\n end lang files 5\n\n\n");

              fread(json_Translate_File, sizeof(char), file_Lenght, f);
              // printf("\n\n\n end lang\n%s\n\n\n", json_Translate_File);

              cJSON_Delete(sms_Rsp_Json);
              sms_Rsp_Json = cJSON_Parse(json_Translate_File);
              jsonCounterTranslate = 1;
              free(json_Translate_File);

              // printf("\n\n\n end lang files 8\n\n\n");
              if (sms_Rsp_Json == NULL) {

                // printf("\n\n X10 9999\n\n");
                const char *error_ptr = cJSON_GetErrorPtr();
                if (error_ptr != NULL) {
                  // printf("Error before: %s\n", error_ptr);
                }
              }
            }

            fclose(f);
            // printf("\n\n\n end lang files 9\n\n\n");

            asprintf(&rsp, "%s %c %c %s", ADMIN_ELEMENT, cmd, param, "OK");

            return rsp;
          } else {
            label_Block_Feedback_SMS = 0;
            fclose(f);
            remove("/spiffs/aux_language.json");
            asprintf(&rsp, "%s %c %c %s", ADMIN_ELEMENT, cmd, param, "ERROR");

            return rsp;
          }
        }
        //}
        else {
          label_Block_Feedback_SMS = 0;
          asprintf(&rsp, "%s %c %c %s", ADMIN_ELEMENT, cmd, param, "ERROR");

          return rsp;
        }
        /* //////printf("\n\n\n end lang files 7\n\n\n");
        sms_Rsp_Json = cJSON_Parse(json_Translate_File);
        //////printf("\n\n\n end lang files 8\n\n\n"); */
        if (sms_Rsp_Json == NULL) {
          label_Block_Feedback_SMS = 0;
          // ////printf("\n\n X10 9999\n\n");
          const char *error_ptr = cJSON_GetErrorPtr();
          if (error_ptr != NULL) {
            // ////printf("Error before: %s\n", error_ptr);
          }
        }
      } else {
        label_Block_Feedback_SMS = 0;

        // ////printf(ERROR_USER_NOT_PERMITION);
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else {

      return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_PARAMETER"));
    }
  }

  return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_CMD"));
}

uint8_t changeInputTranslators(char *payload) {
  uint8_t count = 0;
  uint8_t strIndex = 0;
  char input1_msg[60] = {};
  char input2_msg[60] = {};

  // //printf("\n\n input change payload %s\n\n", payload);

  for (size_t i = 0; i < strlen(payload); i++) {
    if (payload[i] == ';') {
      count++;
      strIndex = 0;
    } else if (count == 0) {
      input1_msg[strIndex++] = payload[i];
    } else if (count == 1) {
      input2_msg[strIndex++] = payload[i];
    } else {
      break;
    }
  }

  // //printf("\n\n input change 1 %s\n\n", input1_msg);
  // //printf("\n\n input change 2 %s\n\n", input2_msg);
  if (strlen(input1_msg) > 4) {
    modifyJSONValue("INPUT_HAS_BEEN_ACTIVATED1", input1_msg);
    save_STR_Data_In_Storage(NVS_SMS_INPUT1_MESSAGE, input1_msg,
                             nvs_System_handle);
  }

  if (strlen(input2_msg)) {
    modifyJSONValue("INPUT_HAS_BEEN_ACTIVATED2", input2_msg);
    save_STR_Data_In_Storage(NVS_SMS_INPUT2_MESSAGE, input2_msg,
                             nvs_System_handle);
  }

  return 1;
}

char *MyUser_erase_Admin(uint8_t BLE_SMS_INDICATION, char *payload,
                         mqtt_information *mqttInfo) {

  if (payload[0] == '*') {

    char w_key[30] = {};
    char aux_buff[200] = {};
    MyUser wi_search_user;
    int count = 0;
    nvs_iterator_t it;

    memset(&wi_search_user, 0, sizeof(wi_search_user));

    it = nvs_entry_find("keys", NVS_ADMIN_NAMESPACE, NVS_TYPE_STR);
    char value[200];
    // ////printf("Iterate NVS\n");

    while (it != NULL) {
      count++;
      nvs_entry_info_t info;
      nvs_entry_info(it, &info);
      it = nvs_entry_next(it);
      // ////printf("key '%s', type '%d' \n", info.key, info.type);
      get_Data_Users_From_Storage(info.key, &value);
      memset(&wi_search_user, 0, sizeof(wi_search_user));
      memset(aux_buff, 0, sizeof(aux_buff));
      memset(w_key, 0, sizeof(w_key));

      MyUser_Search_User(w_key, aux_buff);
      parse_ValidateData_User(aux_buff, &wi_search_user);

      if (get_antipassback_user(wi_search_user.wiegand_code) == ESP_OK) {
        erase_antipassback_user(wi_search_user.wiegand_code);

        nvs_get_u32(nvs_System_handle, NVS_ANTIPASSBACK_PEOPLE_NUMBER,
                    &anti_passback_people_number);

        if (anti_passback_people_number > 0) {
          anti_passback_people_number--;
          nvs_set_u32(nvs_System_handle, NVS_ANTIPASSBACK_PEOPLE_NUMBER,
                      anti_passback_people_number);
        }
      }

      // ////printf("  * value: %s\n", value);
    }

    // ////printf("Iterate NVS END\n");
    ////////printf("Time: %lld", time);
    // ////printf("\ncount numbers: %d\n", count);

    if (nvs_erase_all(nvs_Admin_handle) == ESP_OK) {
      nvs_erase_all(nvs_wiegand_codes_admin_handle);
      nvs_erase_all(nvs_rf_codes_admin_handle);
      free(it);
      memset(file_contents, 0, sizeof(file_contents));
      if (BLE_SMS_INDICATION == BLE_INDICATION ||
          BLE_SMS_INDICATION == UDP_INDICATION) {
        sprintf(file_contents, "%s", "ME R A * OK");
      } else if (BLE_SMS_INDICATION == SMS_INDICATION) {

        char UDP_Rsp[50] = {};
        sprintf(mqttInfo->data, "%s", "ME R A * OK");
        // send_UDP_Send(UDP_Rsp);
        send_UDP_queue(mqttInfo);

        sprintf(file_contents, "%s", return_Json_SMS_Data("ERASE_ALL_ADMIN"));
      }

      UsersCountNumbers = get_User_Counter_From_Storage();

      UsersCountNumbers = UsersCountNumbers - count;

      save_User_Counter_In_Storage(UsersCountNumbers);

      return file_contents;
    } else {

      free(it);
      memset(file_contents, 0, sizeof(file_contents));
      sprintf(file_contents, "%s", return_Json_SMS_Data("ERROR_RESET"));
      return file_contents;
    }

    memset(file_contents, 0, sizeof(file_contents));
    sprintf(file_contents, "%s", return_Json_SMS_Data("ERROR_RESET"));
    return file_contents;
  }

  if (MyUser_Search_User(payload, file_contents) == ESP_OK) {
    MyUser user_deleteValidateData;
    memset(&user_deleteValidateData, 0, sizeof(user_deleteValidateData));
    parse_ValidateData_User(file_contents, &user_deleteValidateData);

    if (user_deleteValidateData.permition == '1') {
      if (Myuser_deleteUser(&user_deleteValidateData) == ESP_OK) {
        /* UsersCountNumbers = get_User_Counter_From_Storage();
        UsersCountNumbers--;

        save_User_Counter_In_Storage(UsersCountNumbers); */

        check_And_Erase_Admin_In_Feedback_List(user_deleteValidateData.phone);

        if (BLE_SMS_INDICATION == BLE_INDICATION ||
            BLE_SMS_INDICATION == UDP_INDICATION) {
          memset(file_contents, 0, sizeof(file_contents));

          if (strlen(user_deleteValidateData.phone) < 1) {
            sprintf(file_contents, "%s %c %c $%s", ADMIN_ELEMENT, RESET_CMD,
                    ADMIN_PARAMETER, user_deleteValidateData.wiegand_code);
          } else {
            sprintf(file_contents, "%s %c %c %s", ADMIN_ELEMENT, RESET_CMD,
                    ADMIN_PARAMETER, user_deleteValidateData.phone);
          }

          // ////printf("\ndelete admin ble %s\n", file_contents);
          return file_contents;
        } else if (BLE_SMS_INDICATION == SMS_INDICATION) {
          char UDP_Rsp[50] = {};
          sprintf(mqttInfo->data, "%s %s", "ME R A",
                  user_deleteValidateData.phone);
          // send_UDP_Send(UDP_Rsp);
          send_UDP_queue(mqttInfo);
          memset(file_contents, 0, sizeof(file_contents));
          sprintf(file_contents, return_Json_SMS_Data("ADMIN_HAS_BEEN_DELETED"),
                  user_deleteValidateData.phone);
          // ////printf("\ndelete admin sms %s\n", file_contents);
          return file_contents;
        } else {
          memset(file_contents, 0, sizeof(file_contents));
          sprintf(file_contents, "%s", ERROR_INPUT_DATA);
          return file_contents;
        }
      } else {
        // ////printf("delete admin falhou\n");
        memset(file_contents, 0, sizeof(file_contents));
        sprintf(file_contents, "%s", return_Json_SMS_Data("ERROR_RESET"));
        return file_contents;
      }
    } else {
      // ////printf("este user é owner nao eliminado\n");
      memset(file_contents, 0, sizeof(file_contents));
      sprintf(file_contents, "%s",
              return_Json_SMS_Data("ERROR_USER_NOT_IS_ADMIN"));
      return file_contents;
    }

    // ////printf("read user 1123 %s\n", file_contents);
    memset(file_contents, 0, sizeof(file_contents));
    sprintf(file_contents, "%s", return_Json_SMS_Data("ERROR_RESET"));
    return file_contents;
  } else {

    // ////printf(ERROR_USER_NOT_FOUND);
    memset(file_contents, 0, sizeof(file_contents));
    sprintf(file_contents, "%s", return_Json_SMS_Data("ERROR_USER_NOT_FOUND"));
    return file_contents;
  }
}
uint8_t check_And_Erase_Admin_In_Feedback_List(char *phoneNumber) { return 1; }

char *set_Alarm_Sound(char *payload) {
  if (payload[0] >= '1' && payload[0] <= '8') {
    feedback_Sound_Index = payload[0] - 48;
    // ////printf("\n\nset_Alarm_Sound 1 -  %d\n\n", feedback_Sound_Index);
    save_INT8_Data_In_Storage(NVS_KEY_FEEDBACK_SOUND_INDEX,
                              feedback_Sound_Index, nvs_System_handle);
    // ////printf("\n\nset_Alarm_Sound 2\n\n");
    return payload;
  } else {
    return "ERROR";
  }

  return "OK";
}

char *play_Alert_Sound() {
  uint8_t state = PLAY_RECORD_SOUND_STATE;
  send_Type_Call_queue(state);
  return "NTRSP";
}

char *set_Block_Feedback_SMS(uint8_t BLE_SMS_INDICATION) {

  if (save_INT8_Data_In_Storage(NVS_KEY_BLOCK_FEEDBACK_SMS, 1,
                                nvs_System_handle) == ESP_OK) {

    if (BLE_SMS_INDICATION == BLE_INDICATION ||
        BLE_SMS_INDICATION == UDP_INDICATION) {
      return "ME S F ON";
    } else if (BLE_SMS_INDICATION == SMS_INDICATION) {
      return return_Json_SMS_Data("FEEDBACK_SMS_HAVE_BEEN_ENABLED");
    } else {
      return ERROR_INPUT_DATA;
    }
  } else {
    if (BLE_SMS_INDICATION == BLE_INDICATION ||
        BLE_SMS_INDICATION == UDP_INDICATION) {
      return "ME S F ERROR";
    } else if (BLE_SMS_INDICATION == SMS_INDICATION) {
      return return_Json_SMS_Data("ERROR_IN_ACTIVATING_SMS_FEEDBACK");
    } else {
      return ERROR_INPUT_DATA;
    }
  }

  return ERROR_INPUT_DATA;
}

char *get_Block_Feedback_SMS(uint8_t BLE_SMS_INDICATION) {

  uint8_t aux_Label_Block_Feedback_SMS =
      get_INT8_Data_From_Storage(NVS_KEY_BLOCK_FEEDBACK_SMS, nvs_System_handle);
  if (aux_Label_Block_Feedback_SMS) {
    // label_Block_Feedback_SMS = 1;

    if (BLE_SMS_INDICATION == BLE_INDICATION) {
      return "ME S F ON";
    } else if (BLE_SMS_INDICATION == SMS_INDICATION) {
      return return_Json_SMS_Data("FEEDBACK_SMS_ARE_ENABLED");
    } else {
      return ERROR_INPUT_DATA;
    }
  } else {
    if (BLE_SMS_INDICATION == BLE_INDICATION) {
      return "ME S F OFF";
    } else if (BLE_SMS_INDICATION == SMS_INDICATION) {
      return return_Json_SMS_Data("FEEDBACK_SMS_ARE_NOT_ENABLED");
    } else {
      return ERROR_INPUT_DATA;
    }
  }

  return ERROR_INPUT_DATA;
}

char *reset_Block_Feedback_SMS(uint8_t BLE_SMS_INDICATION) {
  // TODO: IMPLEMENTAR GRAVAR NA MEMORIA NVS A LABEL E A FUNÇÃO RESET
  return "ok";
}

uint8_t reset_Owner_password(char *payload, data_EG91_Send_SMS *data_SMS) {
  MyUser myUser_owner_Password_change;
  memset(&myUser_owner_Password_change, 0,
         sizeof(myUser_owner_Password_change));
  char owner_Password_change_data[200] = {};
  char aux_phoneNumber[100];

  sprintf(aux_phoneNumber, "%s",
          check_IF_haveCountryCode(data_SMS->phoneNumber, 0));

  if (MyUser_Search_User(aux_phoneNumber, owner_Password_change_data) ==
      ESP_OK) {
    parse_ValidateData_User(owner_Password_change_data,
                            &myUser_owner_Password_change);

    if (myUser_owner_Password_change.permition == '2') {
      if (strlen(payload) == 6) {
        for (size_t i = 0; i < strlen(payload); i++) {
          if (payload[i] > '9' || payload[i] < '0') {
            sprintf(data_SMS->payload, "%s",
                    return_Json_SMS_Data("ERROR_INPUT_DATA"));
            xQueueSendToBack(queue_EG91_SendSMS, (void *)data_SMS,
                             pdMS_TO_TICKS(100));
            return 0;
          }
        }

        // memset(myUser_owner_Password_change.key, 0, sizeof(key));
        sprintf(myUser_owner_Password_change.key, "%s", payload);

        char newline[200] = {};
        char udp_dataSend[100] = {};
        int count;

        /* Remove extra new line character from stdin */
        // fflush(stdin);
        memset(&newline, 0, 200);
        // user_validateData.week[7] = '\0';

        sprintf(&newline, "%s;%s;%s;%s;%s;%s;%s;%c;%s;%c;%c;",
                myUser_owner_Password_change.phone,
                myUser_owner_Password_change.firstName,
                myUser_owner_Password_change.start.date,
                myUser_owner_Password_change.start.hour,
                myUser_owner_Password_change.end.days,
                myUser_owner_Password_change.end.hour,
                myUser_owner_Password_change.key,
                myUser_owner_Password_change.permition,
                myUser_owner_Password_change.week,
                myUser_owner_Password_change.relayPermition,
                myUser_owner_Password_change.ble_security);
        // ////printf("new line password %s\n", newline);

        sprintf(udp_dataSend, "ME R K %s;%s",
                myUser_owner_Password_change.phone,
                myUser_owner_Password_change.key);

        replaceUser(&myUser_owner_Password_change);

        sprintf(data_SMS->payload, return_Json_SMS_Data("OWNER_RESET_PASSWORD"),
                payload);
        xQueueSendToBack(queue_EG91_SendSMS, (void *)data_SMS,
                         pdMS_TO_TICKS(100));
        // ////printf("new line password %s\n", newline);
        send_UDP_Send(udp_dataSend, "");
        // ////printf("new line password11 %s\n", newline);
        return 1;
      } else {
        sprintf(data_SMS->payload, "%s",
                return_Json_SMS_Data("ERROR_INPUT_DATA"));
        xQueueSendToBack(queue_EG91_SendSMS, (void *)data_SMS,
                         pdMS_TO_TICKS(100));
        return 0;
      }
    } else {
      // ////printf("\n\n NOT OWNER PASSWORD CHANGE\n\n");
      return 0;
    }
  }
  return 1;
}

char *change_Owner(char *payload, uint8_t BLE_SMS) {
  char newOwner_PhoneNumber[20] = {};
  char newOwner_Data[200] = {};

  MyUser newOwner_struct;
  MyUser owner_Struct;

  memset(&newOwner_struct, 0, sizeof(newOwner_struct));
  memset(&owner_Struct, 0, sizeof(owner_Struct));

  for (int i = 0; i < strlen(payload); i++) {
    if (payload[i] > '9' || payload[i] < '0') {
      if (payload[i] != '+') {
        // ////printf("PHONE NUMBER NAO CORRECTA\n");
        return return_Json_SMS_Data("ERROR_INPUT_DATA");
      }
    }
  }

  sprintf(newOwner_PhoneNumber, "%s", check_IF_haveCountryCode(payload, 0));

  if (MyUser_Search_User(newOwner_PhoneNumber, newOwner_Data) == ESP_OK) {
    parse_ValidateData_User(newOwner_Data, &newOwner_struct);

    if (newOwner_struct.permition == '1') {
      size_t required_size = 0;
      char owner_Data[200] = {};

      if (nvs_get_str(nvs_System_handle, NVS_KEY_OWNER_INFORMATION, NULL,
                      &required_size) == ESP_OK) {
        // ////printf("\nrequire size %d\n", required_size);
        // ////printf("\nGET OWNER NAMESPACE\n");
        if (nvs_get_str(nvs_System_handle, NVS_KEY_OWNER_INFORMATION,
                        owner_Data, &required_size) == ESP_OK) {
          UsersCountNumbers = get_User_Counter_From_Storage();
          // ////printf("\nUsersCountNumbers 1 %d\n", UsersCountNumbers);
          parse_ValidateData_User(owner_Data, &owner_Struct);

          if (Myuser_deleteUser(&newOwner_struct) == ESP_OK) {
            UsersCountNumbers = get_User_Counter_From_Storage();
            // ////printf("\nUsersCountNumbers 2 %d\n", UsersCountNumbers);
            //  ////printf("\nowner_Data2 %s - %s\n", owner_Data,
            //  owner_Struct.phone);
            if (Myuser_deleteUser(&owner_Struct) == ESP_OK) {
              UsersCountNumbers = get_User_Counter_From_Storage();
              // ////printf("\nUsersCountNumbers 3 %d\n", UsersCountNumbers);
              //  ////printf("\nowner_Data3 %d\n", owner_Data);
              owner_Struct.permition = '1';
              newOwner_struct.permition = '2';

              MyUser_Add(&owner_Struct);
              UsersCountNumbers = get_User_Counter_From_Storage();
              // ////printf("\nUsersCountNumbers 4 %d\n", UsersCountNumbers);
              //  ////printf("\nowner_Data4 %s\n", owner_Data);
              MyUser_Add(&newOwner_struct);
              UsersCountNumbers = get_User_Counter_From_Storage();
              // ////printf("\nUsersCountNumbers 5 %d\n", UsersCountNumbers);
              //  ////printf("\nowner_Data5 %s\n", owner_Data);

              memset(newOwner_Data, 0, 200 /* sizeof(newUserData) */);
              sprintf(newOwner_Data, "%s;%s;%s;%s;%s;%s;%s;%c;%s;%c;%c;%c;",
                      newOwner_struct.phone, newOwner_struct.firstName,
                      newOwner_struct.start.date, newOwner_struct.start.hour,
                      newOwner_struct.end.days, newOwner_struct.end.hour,
                      newOwner_struct.key, newOwner_struct.permition,
                      newOwner_struct.week, newOwner_struct.relayPermition,
                      newOwner_struct.ble_security,
                      newOwner_struct.erase_User_After_Date);

              save_STR_Data_In_Storage(NVS_KEY_OWNER_INFORMATION, newOwner_Data,
                                       nvs_System_handle);
              char UDP_Rsp[50] = {};
              sprintf(UDP_Rsp, "%s %s %s", "ME R O", newOwner_struct.phone,
                      newOwner_struct.key);
              // send_UDP_Send(UDP_Rsp);
              send_UDP_Send(UDP_Rsp, "");
              // send_UDP_queue(UDP_Rsp);

              desactivateUDP_network();

              if (BLE_SMS == BLE_INDICATION) {
                // ////printf("\nowner_Data5 BLE_INDICATION\n");
                return "OK";
              } else if (BLE_SMS == SMS_INDICATION) {
                memset(file_contents, 0, 200);

                // ////printf("\n\nSEND SMS CHANGE OWNER\n\n");
                if (!strcmp(newOwner_struct.firstName, "S/N")) {
                  sprintf(newOwner_struct.firstName, "%s",
                          return_Json_SMS_Data("NO_NAME"));
                }

                sprintf(file_contents,
                        return_Json_SMS_Data("OWNER_CHANGED_TO_ADMIN"),
                        newOwner_struct.firstName, newOwner_struct.phone);
                // ////printf("\n\nSEND SMS CHANGE OWNER1\n\n");
                return file_contents;
              } else if (BLE_SMS == UDP_INDICATION) {
                return "NTRSP";
              } else {
                // ////printf("\nowner_Data5 BLE_INDICATION 111\n");
                return return_Json_SMS_Data("ERROR_RESET");
              }
            } else {
              newOwner_struct.permition = '1';
              // ////printf("\nowner_Data5 BLE_INDICATION 555\n");
              MyUser_Add(&newOwner_struct);
              return return_Json_SMS_Data("ERROR_RESET");
            }
          } else {
            return return_Json_SMS_Data("ERROR_RESET");
          }
        } else {
          return return_Json_SMS_Data("ERROR_RESET");
        }
      } else {
        return return_Json_SMS_Data("ERROR_RESET");
      }
    } else {
      return return_Json_SMS_Data("NEW_OWNER_IS_NOT_ADMIN");
    }
  } else {
    return_Json_SMS_Data("ERROR_USER_NOT_FOUND");
  }

  return return_Json_SMS_Data("ERROR_RESET");
}

char *MyUser_add_Admin(uint8_t BLE_SMS_INDICATION, char *payload,
                       mqtt_information *mqttInfo) {
  /*  if (UsersCountNumbers < LIMIT_USERS_REGISTER_NUMBER)
   { */
  /* code */
  // ////printf("add default number 1\n");
  // ////printf("number add=%s len=%d", payload, strlen(payload));
  char phNumber[MYUSER_PHONE_SIZE];
  char frsName[20] = {};
  char surName[4] = {};
  memset(phNumber, 0, sizeof(phNumber));
  memset(frsName, 0, sizeof(frsName));
  memset(surName, 0, sizeof(surName));
  memset(file_contents, 0, sizeof(file_contents));

  MyUser user_validateData; //= malloc(sizeof(MyUser));
  memset(&user_validateData, 0, sizeof(user_validateData));

  // memset(&user_validateData, 0, sizeof(user_validateData));

  int aux = 0;
  int strIndex = 0;
  for (int i = 0; i < strlen(payload); i++) {
    if (payload[i] == '.') {
      aux++;
      strIndex = 0;
    } else {
      if (aux == 0) {
        if (strIndex < 20) {
          phNumber[strIndex] = payload[i];
          strIndex++;
        } else {
          memset(file_contents, 0, sizeof(file_contents));
          sprintf(file_contents, "%s",
                  return_Json_SMS_Data("ERROR_WRONG_PHONE_NUMBER"));
          return file_contents;
        }
      } else if (aux == 1) {
        if (strIndex < 20) {
          frsName[strIndex] = payload[i];
          strIndex++;
        } else {
          memset(file_contents, 0, sizeof(file_contents));
          sprintf(file_contents, "%s",
                  return_Json_SMS_Data("ERROR_NAME_LENGTH"));
          return file_contents;
        }
      } else if (aux == 2) {
        user_validateData.wiegand_rele_permition = payload[i];
      } else if (aux == 3) {
        user_validateData.wiegand_code[strIndex] = payload[i];
        strIndex++;
      } else if (aux == 4) {
        user_validateData.rf_serial[strIndex] = payload[i];
        strIndex++;
      } else if (aux == 5) {
        user_validateData.rf1_relay = payload[i];
        strIndex++;
      } else if (aux == 6) {
        user_validateData.rf2_relay = payload[i];
        strIndex++;
      } /*  else if (aux == 7) {
         user_validateData.rf3_relay = payload[i];
         strIndex++;
       } */
    }
  }

  if (user_validateData.wiegand_rele_permition == 0 ||
      strlen(user_validateData.wiegand_code) == 0) {
    user_validateData.wiegand_rele_permition = ':';
    user_validateData.wiegand_code[0] = ':';
  }

  if (user_validateData.rf1_relay == 0) {
    user_validateData.rf1_relay = ':';
  }

  if (user_validateData.rf2_relay == 0) {
    user_validateData.rf2_relay = ':';
  }

  /*   if (user_validateData.rf3_relay == 0) {
      user_validateData.rf3_relay = ':';
    } */

  if (strlen(user_validateData.rf_serial) == 0) {
    user_validateData.rf1_relay = ':';
    user_validateData.rf2_relay = ':';
    // user_validateData.rf3_relay = ':';
    user_validateData.rf_serial[0] = ':';
  }

  // ////printf("phone add admin %s\n", phNumber);
  // ////printf("first name add admin %s\n", frsName);
  // ////printf("surname name add admin %s\n", surName);

  if (strlen(phNumber) < 20 && strlen(phNumber) > 1) {
    if (strlen(frsName) > 0) {
      sprintf(user_validateData.firstName, "%s", frsName);
    } else {
      sprintf(user_validateData.firstName, "%s", "S/N");
    }

    if (get_RTC_System_Time() == 0) {
      memset(file_contents, 0, sizeof(file_contents));
      sprintf(file_contents, "%s", return_Json_SMS_Data("ERROR_ADMIN_ADD"));
      return file_contents;
    }

    char auxPhoneNumber[20] = {};

    sprintf(auxPhoneNumber, "%s", removeSpacesFromStr(phNumber));
    memset(phNumber, 0, sizeof(phNumber));

    sprintf(phNumber, "%s", auxPhoneNumber);

    sprintf(user_validateData.phone, "%s", phNumber);
    sprintf(user_validateData.start.date, "%d", nowTime.date);
    sprintf(user_validateData.start.hour, "%s", "0000");
    sprintf(user_validateData.end.days, "%c", '*');
    sprintf(user_validateData.end.hour, "%s", "2359");
    sprintf(user_validateData.key, "%s", DEFAULT_ADMIN_PASSWORD);
    sprintf(user_validateData.week, "%s", "1111111");
    user_validateData.permition = '1';
    user_validateData.relayPermition = '0';
    user_validateData.ble_security = '0';
    user_validateData.erase_User_After_Date = '0';

    // ////printf("new user bff 1313 %s\n", file_contents);

    char auxfileContents[200];

    if (strlen(user_validateData.start.date) > 6) {
      memset(file_contents, 0, sizeof(file_contents));
      sprintf(file_contents, "%s", return_Json_SMS_Data("ERROR_ADMIN_ADD"));
      return file_contents;
    }

    memset(file_contents, 0, sizeof(file_contents));
    if (MyUser_Search_User(user_validateData.phone, auxfileContents) !=
        ESP_OK) {
      // ////printf("\n\nuser_validateData.phone add admin %s\n\n",
      // user_validateData.phone);
      uint16_t ACK_Add_User = MyUser_Add(&user_validateData);
      if (ACK_Add_User == ESP_OK) {
        if (BLE_SMS_INDICATION == BLE_INDICATION ||
            BLE_SMS_INDICATION == UDP_INDICATION) {
          memset(file_contents, 0, sizeof(file_contents));

          sprintf(file_contents,
                  "ME S A %s;%s;%s;%s;%s;%s;%c;%s;%c;%c;%c;%c;%s;%s;%c;%c;",
                  user_validateData.phone, user_validateData.firstName,
                  user_validateData.start.date, user_validateData.start.hour,
                  user_validateData.end.days, user_validateData.end.hour,
                  user_validateData.permition, user_validateData.week,
                  user_validateData.relayPermition,
                  user_validateData.ble_security,
                  user_validateData.erase_User_After_Date,
                  user_validateData.wiegand_rele_permition,
                  user_validateData.wiegand_code, user_validateData.rf_serial,
                  user_validateData.rf1_relay, user_validateData.rf2_relay);

          if (BLE_SMS_INDICATION == BLE_INDICATION) {
            // TODO: ADICIONAR TOPICO NESTE
            send_UDP_Send(file_contents, "");
            label_BLE_UDP_send = 0;
          }

          return file_contents;
        } else if (BLE_SMS_INDICATION == SMS_INDICATION) {
          memset(file_contents, 0, sizeof(file_contents));

          char UDP_Rsp[200] = {};
          sprintf(mqttInfo->data, "ME S A %s;%s;%s;%s;%s;%s;%c;%s;%c;%c;%c",
                  user_validateData.phone, user_validateData.firstName,
                  user_validateData.start.date, user_validateData.start.hour,
                  user_validateData.end.days, user_validateData.end.hour,
                  user_validateData.permition, user_validateData.week,
                  user_validateData.relayPermition,
                  user_validateData.ble_security,
                  user_validateData.erase_User_After_Date);

          // send_UDP_Send(UDP_Rsp);
          send_UDP_queue(mqttInfo);
          sprintf(file_contents,
                  return_Json_SMS_Data("NEW_ADMIN_HAS_BEEN_ADDED"));
          return file_contents;
        } else {
          sprintf(file_contents, "%s", ERROR_INPUT_DATA);
          return file_contents;
        }
      } else if (ACK_Add_User == 128) {
        return return_Json_SMS_Data("ERROR_LIMIT_USERS");
      } else if (ACK_Add_User == ERROR_WIEGAND_ALREADY_EXIST) {
        // //printf("\n\nnew user bff 1999\n\n");
        return ERROR_USER_WIEGAND_ALREADY_EXIST;
      } else {
        if (BLE_SMS_INDICATION == BLE_INDICATION ||
            BLE_SMS_INDICATION == UDP_INDICATION) {
          sprintf(file_contents, "%s", return_Json_SMS_Data("ERROR_ADMIN_ADD"));
          return file_contents;
        } else if (BLE_SMS_INDICATION == SMS_INDICATION) {
          sprintf(file_contents, return_Json_SMS_Data("ERROR_ADMIN_ADD"));
          return file_contents;
        }
      }
    } else {
      if (BLE_SMS_INDICATION == BLE_INDICATION ||
          BLE_SMS_INDICATION == UDP_INDICATION) {
        sprintf(file_contents, "%s",
                return_Json_SMS_Data("ERROR_ADMIN_ALREADY_EXISTS"));
        return file_contents;
      } else if (BLE_SMS_INDICATION == SMS_INDICATION) {
        sprintf(file_contents, "%s",
                return_Json_SMS_Data("ERROR_ADMIN_ALREADY_EXISTS"));
        return file_contents;
      }
    }
    // MyUser_List_AllUsers();
  } else {
    if (BLE_SMS_INDICATION == BLE_INDICATION ||
        BLE_SMS_INDICATION == UDP_INDICATION) {
      sprintf(file_contents, "%s",
              return_Json_SMS_Data("ERROR_WRONG_PHONE_NUMBER"));
      return file_contents;
    } else if (BLE_SMS_INDICATION == SMS_INDICATION) {
      sprintf(file_contents, "%s",
              return_Json_SMS_Data("ERROR_WRONG_PHONE_NUMBER"));
      return file_contents;
    }
  }

  sprintf(file_contents, "%s", ERROR_INPUT_DATA);
  return file_contents;
  /*  }
   else
   {
       if (BLE_SMS_INDICATION == BLE_INDICATION)
       {
           sprintf(file_contents, "ME S A %s", ERROR_LIMIT_USERS);
           return file_contents;
       }
       else if (BLE_SMS_INDICATION == SMS_INDICATION)
       {
           sprintf(file_contents, "%s",
   return_Json_SMS_Data("ERROR_LIMIT_USERS")); return file_contents;
       }
   } */
  return "ERROR";
}

char *get_LogFiles_profiles() {
  init_SDCard();
  char fileName[50];
  long int file_Len = 0;
  int append = 0;
  FILE *f;
  // lastMonth = 5;
  memset(file_contents, 0, 200);
  memset(fileName, 0, 20);
  for (int i = 1; i <= 12; i++) {

    sprintf(fileName, "/sdcard/%02d.csv", i);
    // ////printf("\nFILE NAME %s\n", fileName);
    if (!access(fileName, F_OK)) {
      f = fopen(fileName, "r");

      fseek(f, 0, SEEK_END);

      file_Len = ftell(f);
      append += sprintf(file_contents + append, "%d-%ld ", i, file_Len);
      // ////printf("\n%s\n", file_contents);
      fclose(f);
    } else {
      // ////printf("\nThe File %s\t not Found\n", fileName);
      append += sprintf(file_contents + append, "%d-0 ", i);
      // ////printf("\n%s\n", file_contents);
    }
  }

  // ////printf("\n\n unmount 111\n\n");
  free_SPI_Host();
  // esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
  // // ////printf("\n\n unmount 222\n\n");
  // //ESP_LOGI("TAG", "Card unmounted");

  // // deinitialize the bus after all devices are removed
  // spi_bus_free(host.slot);
  // ////printf("\n\n unmount 333\n\n");

  return file_contents;
}

char *send_LogFile(uint8_t gattsIF, uint16_t connID, uint16_t handle_table,
                   char userPermition, char *payload) {

  sendLOGS_parameters_Message message = {
      .gattsIF = gattsIF,
      .connID = connID,
      .handle_table = handle_table,
      .usr_perm = userPermition,
  };
  sprintf(message.payload, "%s", payload);

  // ////printf("queue created\n");
  // ////printf("MyUser_ReadAllUsers CONN ID %d\n", message.connID);
  // ////printf("MyUser_ReadAllUsers GATTSIF %d\n", message.gattsIF);
  // ////printf("MyUser_ReadAllUsers %d\n", message.handle_table);

  if (sendLOG_User_Label == 0) {
    sendLOG_User_Label = 1;
    readAllUser_ConnID = message.connID;
    xTaskCreate(task_Send_LOG_File, "task_Send_LOG_File", 4000,
                (void *)&message, NULL, NULL);
    vTaskDelay(pdMS_TO_TICKS((100)));
  } else {
    return "READ ALL USERS NOT POSSIBLE";
  }

  return "USER ALL OK";
}

void task_Send_LOG_File(void *pvParameter) {

  // heap_trace_start(HEAP_TRACE_LEAKS);
  sendLOGS_parameters_Message *message =
      (sendLOGS_parameters_Message *)pvParameter;

  // ////printf("\n\ntask_Send_LOG_File 00 - %s\n\n", message->payload);

  sendLOGS_parameters_Message cpy_message = {
      .connID = message->connID,
      .gattsIF = message->gattsIF,
      .handle_table = message->handle_table,
      .usr_perm = message->usr_perm,
  };
  // ////printf("\n\ntask_Send_LOG_File 001\n\n");
  sprintf(cpy_message.payload, "%s", message->payload);
  char fileLOGS_content[182];
  // ////printf("\n\ntask_Send_LOG_File 01\n\n");
  init_SDCard();
  // ////printf("\n\ntask_Send_LOG_File 21\n\n");

  // ////printf("\n\ntask_Send_LOG_File 1\n\n");

  // ////printf("\n\ntask_Send_LOG_File 2 - %s\n\n", message->payload);
  if (strlen(cpy_message.payload) > 2) {
    // ////printf("\n\n unmount 111\n\n");
    free_SPI_Host();
    // esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
    // // ////printf("\n\n unmount 222\n\n");
    // //ESP_LOGI("TAG", "Card unmounted");

    // // deinitialize the bus after all devices are removed
    // spi_bus_free(host.slot);
    // ////printf("\n\n unmount 333\n\n");
    sendLOG_User_Label = 0;
    vTaskDelete(NULL);
  } else {
    // ////printf("\n\ntask_Send_LOG_File 3\n\n");
    FILE *ptr;

    if (atoi(cpy_message.payload) > 0 && atoi(cpy_message.payload) <= 12) {
      char *fileName;
      char ch;

      asprintf(&fileName, "/sdcard/%02d.csv", atoi(cpy_message.payload));
      ptr = fopen(fileName, "r");
      // ////printf("\n\ntask_Send_LOG_File 4\n\n");
      if (NULL == ptr) {
        free_SPI_Host();
        // ////printf("\n\n unmount 111\n\n");
        // esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
        // // ////printf("\n\n unmount 222\n\n");
        // //ESP_LOGI("TAG", "Card unmounted");

        // // deinitialize the bus after all devices are removed
        // spi_bus_free(host.slot);
        // ////printf("\n\n unmount 333\n\n");
        // ////printf("file can't be opened \n");
        sendLOG_User_Label = 0;
        free(fileName);
        vTaskDelete(NULL);
      } else {
        // ////printf("\nOPEN FILE: %s\n", fileName);
      }

      // ////printf("\n\ntask_Send_LOG_File 5\n\n");

      while (!feof(ptr)) {
        // ////printf("\n\ntask_Send_LOG_File 6\n\n");
        memset(fileLOGS_content, 0, sizeof(fileLOGS_content));
        for (int i = 0; i < 180; i++) {
          ch = fgetc(ptr);
          fileLOGS_content[i] = ch;

          if (feof(ptr)) {
            break;
          }
        }

        if (strlen(fileLOGS_content) < 180) {
          fileLOGS_content[strlen(fileLOGS_content) - 1] = 0;
        }

        // ////printf("\nLOGS SEND DATA:\n%s\n", fileLOGS_content);

        if (esp_ble_gatts_send_indicate(
                cpy_message.gattsIF, cpy_message.connID,
                cpy_message.handle_table, strlen(fileLOGS_content),
                (uint8_t *)fileLOGS_content, false) != ESP_OK) {
          // free(fileName);
          break;
        }

        // xSemaphoreTake(rdySem_Send_LOGS_Files, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(50));
        // take_LOG_Files_Semph();

        // ////printf("\nlog: %#X\n", ch);
      }
      // free(pvParameter);
      fclose(ptr);
      free(fileName);
    }
    sendLOG_User_Label = 0;

    // ////printf("\n\n unmount 111\n\n");
    // esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
    // ////printf("\n\n unmount 222\n\n");
    free_SPI_Host();
    // ESP_LOGI("TAG", "Card unmounted");

    // deinitialize the bus after all devices are removed
    // spi_bus_free(host.slot);
    // ////printf("\n\n unmount 333\n\n");
    // heap_trace_stop();
    // heap_trace_dump();
    vTaskDelete(NULL);
  }
}