/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|

*/

#include "ble_spp_server_demo.h"
#include "cmd_list.h"
#include "core.h"
#include "erro_list.h"
#include "esp_err.h"
#include "esp_gatts_api.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "math.h"
#include "nvs.h"
#include "system.h"
#include "wiegand.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <users.h>

char file_contents_Users[300];
char buffer[200] = {};
char buffer_UDP_users[250] = {};
char aux_FileContens[300] = {};

extern nvs_handle_t nvs_Users_handle;

uint8_t readAllUser_Label;
uint16_t readAllUser_ConnID;
int UsersCountNumbers;
uint32_t GuestCountNumbers;
// int readAllUser_Label = 0;
const char *weekDaysFeedback[] = {"SUN", "MON", "TUE", "WED",
                                  "THU", "FRI", "SAT"};

SemaphoreHandle_t rdySem;

void ReadALLUsers_task(void *pvParameter);
void ReadALLUsersFormName_task(void *pvParameter);
// char *multiRSP;

char *return_ERROR_New_Add_User(char *output, uint8_t error_ID) {

  switch (error_ID) {

  case ERROR_WRONG_LIMIT_DATE:

    asprintf(&output, "%s", return_Json_SMS_Data("ERROR_WRONG_LIMIT_DATE"));
    return output;
    break;

  case ERROR_WRONG_WEEK_DAYS:

    asprintf(&output, "%s", return_Json_SMS_Data("ERROR_WRONG_WEEK_DAYS"));
    return output;
    break;

  case ERROR_WRONG_END_TIME:

    asprintf(&output, "%s", return_Json_SMS_Data("ERROR_WRONG_END_TIME"));
    return output;
    break;

  case ERROR_WRONG_START_TIME:

    asprintf(&output, "%s", return_Json_SMS_Data("ERROR_WRONG_START_TIME"));
    return output;
    break;

  case ERROR_WRONG_START_DATE:
    // ////printf("\n\n case ERROR_WRONG_START_DATE:\n\n");
    asprintf(&output, "%s", return_Json_SMS_Data("ERROR_WRONG_START_DATE"));
    return output;
    break;

  case ERROR_WRONG_PHONE_NUMBER:

    asprintf(&output, "%s", return_Json_SMS_Data("ERROR_WRONG_PHONE_NUMBER"));
    return output;
    break;

  case ERROR_WRONG_RELAY_PERMITION:

    asprintf(&output, "%s",
             return_Json_SMS_Data("ERROR_WRONG_RELAY_PERMITION"));
    return output;
    break;

  case ERROR_WRONG_TIME_DATA:

    asprintf(&output, "%s", return_Json_SMS_Data("ERROR_WRONG_TIME_DATA"));
    return output;
    break;

  case ERROR_NAME_LENGTH:

    asprintf(&output, "%s", return_Json_SMS_Data("ERROR_NAME_SIZE"));
    return output;
    break;

  case ERROR_NEW_USER_ALREADY_EXISTS:

    asprintf(&output, "%s",
             return_Json_SMS_Data("ERROR_NEW_USER_ALREADY_EXISTS"));
    return output;
    break;

  case ERROR_NEW_USER_SET:

    asprintf(&output, "%s", return_Json_SMS_Data("ERROR_NEW_USER_ADD"));
    return output;
    break;

  case ERROR_NEW_USER_INPUT_DATA:

    asprintf(&output, "%s", return_Json_SMS_Data("ERROR_INPUT_DATA"));
    return output;
    break;

  case ERROR_NEW_USER_ADD:

    asprintf(&output, "%s", return_Json_SMS_Data("ERROR_NEW_USER_ADD"));
    return output;
    break;

  case ERROR_WRONG_ERASE_USER_AFTER_DATE:

    asprintf(&output, "%s",
             return_Json_SMS_Data("ERROR_ERASE_USER_AFTER_DATE"));
    return output;
    break;

  case ERROR_LIMIT_USERS:
    asprintf(&output, "%s", return_Json_SMS_Data("ERROR_LIMIT_USERS"));
    return output;
    break;

  case ERROR_WIEGAND_ALREADY_EXIST:
    asprintf(&output, "%s", ERROR_USER_WIEGAND_ALREADY_EXIST);
    return output;
    break;

  case ERROR_RF_ALREADY_EXIST:
    asprintf(&output, "%s", ERROR_USER_RF_ALREADY_EXIST);
    return output;
    break;

  default:

    asprintf(&output, "%s", return_Json_SMS_Data("ERROR_NEW_USER_ADD"));
    return output;
    break;
  }
}

uint8_t erase_Users_With_LastTime(int nowDate) {
  int count = 0;
  MyUser myUser;

  memset(&myUser, 0, sizeof(myUser));
  // ////printf("\nLIST USERS:\n");
  nvs_iterator_t it;
  it = nvs_entry_find("keys", NVS_USERS_NAMESPACE, NVS_TYPE_STR);
  char value[200] = {};
  // ////printf("Iterate NVS\n");

  while (it != NULL) {
    count++;
    nvs_entry_info_t info;
    nvs_entry_info(it, &info);
    it = nvs_entry_next(it);
    // ////printf("key '%s', type '%d' \n", info.key, info.type);
    get_Data_Users_From_Storage(info.key, &value);
    parse_ValidateData_User(value, &myUser);

    // ////printf("\n\nerase_Users value %c - %s\n\n",
    // myUser.erase_User_After_Date, value);

    if (myUser.erase_User_After_Date == '1') {
      if (myUser.permition == '0') {
        /* code */
        // ////printf("\n\nerase_Users_With_LastTime 2\n\n");
        // ////printf("\n\parseDatetoInt %d\n\n",
        // parseDatetoInt(atoi(myUser.start.date), atoi(myUser.end.days)));
        // ////printf("\n\nowDate %d\n\n", nowDate);

        if (parseDatetoInt(atoi(myUser.start.date), atoi(myUser.end.days)) <
            nowDate) {
          // ////printf("\n\nerase_Users_With_LastTime 3\n\n");
          Myuser_deleteUser(&myUser);
        }
      }
    }

    memset(&myUser, 0, sizeof(myUser));
    memset(value, 0, 200);
    // ////printf("\nvalue: %s\n", value);
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  free(it);
  return 1;
}

char *MyUser_get_ReleRestrition(char *payload, uint8_t BLE_SMS) {
  char aux_payload[200] = {};
  char phnumber[30] = {};
  if (strlen(payload) > 20) {
    return_Json_SMS_Data("ERROR_INPUT_DATA");
  }

  for (size_t i = 0; i < strlen(payload); i++) {
    if (payload[i] < '0' || payload[i] > '9') {
      if (payload[i] != '+') {
        return_Json_SMS_Data("ERROR_INPUT_DATA");
      }
    }
  }

  sprintf(phnumber, "%s", check_IF_haveCountryCode(payload, 0));
  /*memset(&rsp, 0, sizeof(rsp));*/

  if (MyUser_Search_User(phnumber, aux_payload) == ESP_OK) {
    MyUser MyUser_relayRestriction;
    memset(&MyUser_relayRestriction, 0, sizeof(MyUser_relayRestriction));
    parse_ValidateData_User(aux_payload, &MyUser_relayRestriction);

    if (MyUser_relayRestriction.permition == '1' ||
        MyUser_relayRestriction.permition == '2') {

      if (MyUser_relayRestriction.permition == '1') {
        return return_Json_SMS_Data("ERROR_THIS_IMPOSSIBLE_CHANGE_ADMIN");
      } else {
        return return_Json_SMS_Data("ERROR_THIS_IMPOSSIBLE_CHANGE_OWNER");
      }
    }

    if (BLE_SMS == SMS_INDICATION) {
      memset(file_contents_Users, 0, sizeof(file_contents_Users));
      // ////printf("\n\nBLE LABEL RESTORE 62\n\n");

      if (MyUser_relayRestriction.relayPermition == '0') {
        return return_Json_SMS_Data("USER_NOT_HAVE_RELAY_RESTRICTION");
      } else {
        sprintf(file_contents_Users,
                return_Json_SMS_Data("RELAY_RESTRITION_ON_RELAY"),
                MyUser_relayRestriction.relayPermition);
        return file_contents_Users;
      }

      return return_Json_SMS_Data("ERROR_INPUT_DATA");
    }
  } else {
    return return_Json_SMS_Data("ERROR_USER_NOT_FOUND");
  }

  return return_Json_SMS_Data("ERROR_INPUT_DATA");
}

uint8_t checkIf_wiegandExist(char *payload, char *user_contents) {

  // printf("\n\nphone= qqqq 098765 - %s -> user_contents - %s\n\n", payload,
  // user_contents);
  if (get_STR_Data_In_Storage(payload, nvs_wiegand_codes_users_handle,
                              user_contents) == ESP_OK) {
    // printf("\n\nphone= qqqq 11 - %s\n\n", payload);
    return 1;
  }

  if (get_STR_Data_In_Storage(payload, nvs_wiegand_codes_admin_handle,
                              user_contents) == ESP_OK) {
    // //printf("\n\nphone= qqqq 221133 - %s\n\n", payload);
    return 1;
  }

  if (get_STR_Data_In_Storage(payload, nvs_wiegand_codes_owner_handle,
                              user_contents) == ESP_OK) {
    // //printf("\n\nphone= qqqq 33\n\n");
    return 1;
  }

  return 0;
}

uint8_t checkIf_rfSerialExist(char *payload, char *user_contents) {

  // printf("\n\nphone= qqqq 098765 - %s -> user_contents - %s\n\n", payload,
  // user_contents);
  if (get_STR_Data_In_Storage(payload, nvs_rf_codes_users_handle,
                              user_contents) == ESP_OK) {
    printf("\n\nphone= qqqq 11 - %s\n\n", payload);
    return 1;
  }

  if (get_STR_Data_In_Storage(payload, nvs_rf_codes_admin_handle,
                              user_contents) == ESP_OK) {
    printf("\n\nphone= qqqq 221133 - %s\n\n", payload);
    return 1;
  }

  if (get_STR_Data_In_Storage(payload, nvs_rf_codes_owner_handle,
                              user_contents) == ESP_OK) {
    printf("\n\nphone= qqqq 33\n\n");
    return 1;
  }

  return 0;
}

#include "esp_heap_trace.h"

#define NUM_RECORDS 10000

char *parse_UserData(uint8_t BLE_SMS_Indication, int line, char cmd, char param,
                     char *phPassword, char *payload, MyUser *user_validateData,
                     uint8_t gattsIF, uint16_t connID, uint16_t handle_table,
                     mqtt_information *mqttInfo) {
  char *rsp;
  /*memset(&rsp, 0, sizeof(rsp));*/
  // printf("\n\nPCHANGED_TO 11 - \n\n");
  /*  if (BLE_SMS_Indication != UDP_INDICATION)
   {
      sprintf(mqttInfo->topic, "%s", "\0");
   } */

  if (cmd == SET_CMD) {
    if (param == ADMIN_PARAMETER) {

      // ////printf("\n\nPASSWORD_HAS_BEEN_CHANGED_TO 11 -
      // %c\n\n",user_validateData->key);

      if (user_validateData->permition != '2') {
        /* code */

        if (MyUser_newPassword(user_validateData, payload, line)) {
          if (BLE_SMS_Indication == BLE_INDICATION ||
              BLE_SMS_Indication == UDP_INDICATION) {
            /*memset(&rsp, 0, sizeof(rsp));*/
            asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param, payload);
            return rsp;
          } else if (BLE_SMS_Indication == SMS_INDICATION) {
            /*memset(&rsp, 0, sizeof(rsp));*/
            // ////printf("\n\nPASSWORD_HAS_BEEN_CHANGED_TO\n\n");
            asprintf(&rsp, return_Json_SMS_Data("PASSWORD_HAS_BEEN_CHANGED_TO"),
                     payload);
            return rsp;
          } else {
            return return_ERROR_Codes(&rsp,
                                      return_Json_SMS_Data("ERROR_INPUT_DATA"));
          }
        } else {
          return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_SET"));
        }
      } else {
        // ////printf("\n\nPASSWORD_HAS_BEEN_CHANGED_TO 222\n\n");
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("OWNER_WRONG_CMD_RESET_PASSWORD"));
      }
    }
    /* else if (param == WIEGAND_USER_PARAMETER)
    {
        if (BLE_SMS_Indication == UDP_INDICATION)
        {
        }
    } */
    else if (param == USER_PARAMETER) {

      // printf("\n\n\nnew user start1\n\n\n");
      if (user_validateData->permition == '1' ||
          user_validateData->permition == '2') {
        printf("\n\n\nnew user start111\n\n\n");
        char aux_USR_Data[300] = {};
        uint8_t newUser_ACK =
            MyUser_newUSER(payload, &aux_USR_Data, user_validateData->phone,
                           user_validateData->key);
        printf("\n\n\nnew user start222 - %d\n\n\n", newUser_ACK);
        if (newUser_ACK == 0) {
          // printf("\n\n\nnew user start2\n\n\n");
          if (BLE_SMS_Indication == BLE_INDICATION ||
              BLE_SMS_Indication == UDP_INDICATION) {

            printf("\n\n\nnew user start222 333\n\n\n");
            memset(&file_contents_Users, 0, sizeof(file_contents_Users));
            erase_Password_For_Rsp(aux_USR_Data, file_contents_Users);
            asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param,
                     file_contents_Users);
            // printf("\n\n\nnew user start3\n\n\n");
            if (BLE_SMS_Indication == BLE_INDICATION) {
              send_UDP_Send(rsp, "");
              label_BLE_UDP_send = 0;
            }

            return rsp;
          } else if (BLE_SMS_Indication == SMS_INDICATION) {
            /*memset(&rsp, 0, sizeof(rsp));*/
            memset(&file_contents_Users, 0, sizeof(file_contents_Users));
            memset(&buffer, 0, sizeof(buffer));
            // printf("\n\n\nnew user start2222\n\n\n");
            erase_Password_For_Rsp(aux_USR_Data, buffer);
            // printf("\n\n\nnew user start3333\n\n\n");
            sprintf(buffer_UDP_users, "%s %c %c %s", USER_ELEMENT, cmd, param,
                    buffer);
            send_UDP_Send(buffer_UDP_users, "");
            // memset(&mqttInfo, 0, sizeof(mqttInfo));
            // //printf("\n\n\nnew user start4444\n\n\n");
            // memcpy(mqttInfo->data,buffer_UDP_users,strlen(buffer_UDP_users));
            // sprintf(mqttInfo->data, "%s", buffer_UDP_users);
            // printf("\n\n\nnew user start5555\n\n\n");
            // send_UDP_queue(&mqttInfo);
            // printf("\n\n\nnew user start6666\n\n\n");
            asprintf(&rsp, "%s",
                     return_Json_SMS_Data("NEW_USER_HAS_BEEN_ADDED"));
            return rsp;
          } else {
            return return_ERROR_Codes(&rsp,
                                      return_Json_SMS_Data("ERROR_INPUT_DATA"));
          }
        } else if (newUser_ACK == 128) {
          return return_ERROR_Codes(&rsp, "NTRSP");
        } else {
          // ////printf("\n\nnewUser_ACK %d\n\n", newUser_ACK);
          printf("\n\n\nnew user start3333\n\n\n");
          return return_ERROR_New_Add_User(&rsp, newUser_ACK);
        }
      } else {
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == MULTI_USERS_PARAMETER) {
      if (user_validateData->permition == '1' ||
          user_validateData->permition == '2') {
        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {
          /*memset(&rsp, 0, sizeof(rsp));*/
          memset(buffer, 0, sizeof(buffer));
          MyUser_new_MultiUSERS(payload, &buffer);
          // ////printf("\n asprintf(&multiRSP,");
          asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param, buffer);
          return rsp;
        } else if (BLE_SMS_Indication == SMS_INDICATION) {
          /*memset(&rsp, 0, sizeof(rsp));*/
          memset(buffer, 0, sizeof(buffer));
          MyUser_new_MultiUSERS(payload, &buffer);
          // ////printf("\n asprintf(&multiRSP,");
          asprintf(&rsp, " Users Add: %s", buffer);
          return rsp;
        } else {
          return return_ERROR_Codes(&rsp,
                                    return_Json_SMS_Data("ERROR_INPUT_DATA"));
        }
      } else {
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == IMPORT_USERS_HTTPS_PARAMETER) {
      if (user_validateData->permition == '2' &&
          BLE_SMS_Indication == UDP_INDICATION) {

        asprintf(&rsp, "%s",
                 import_mqtt_users(payload, user_validateData->phone,
                                   user_validateData->key));

        return rsp;
      }
    } else if (param == HOUR_PARAMETER) {
      char aux_USR_Data[300] = {};
      if (user_validateData->permition == '1' ||
          user_validateData->permition == '2') {

        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {
          /*memset(&rsp, 0, sizeof(rsp));*/
          asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param,
                   MyUser_new_hour(BLE_SMS_Indication, payload, mqttInfo));
          return rsp;
        } else if (BLE_SMS_Indication == SMS_INDICATION) {
          // memset(&file_contents_Users, 0, sizeof(file_contents_Users));

          // sprintf(file_contents_Users, "%s %c %c %s", USER_ELEMENT, cmd,
          // param, payload); send_UDP_queue(file_contents_Users);
          /*memset(&rsp, 0, sizeof(rsp));*/
          asprintf(&rsp, "%s",
                   MyUser_new_hour(BLE_SMS_Indication, payload, mqttInfo));
          return rsp;
        } else {
          return return_ERROR_Codes(&rsp,
                                    return_Json_SMS_Data("ERROR_INPUT_DATA"));
        }
      } else {
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == WEEK_PARAMETER) {
      char aux_USR_Data[300] = {};
      if (user_validateData->permition == '1' ||
          user_validateData->permition == '2') {
        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {
          /*memset(&rsp, 0, sizeof(rsp));*/
          // //printf("\n\n weekdays change - %s\n\n", payload);
          asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param,
                   MyUser_new_WeekDays(BLE_SMS_Indication, payload, mqttInfo));
          return rsp;
        } else if (BLE_SMS_Indication == SMS_INDICATION) {
          /*memset(&rsp, 0, sizeof(rsp));*/
          asprintf(&rsp, "%s",
                   MyUser_new_WeekDays(BLE_SMS_Indication, payload, mqttInfo));
          return rsp;
        } else {
          return return_ERROR_Codes(&rsp,
                                    return_Json_SMS_Data("ERROR_INPUT_DATA"));
        }
      } else {
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == LAST_PARAMETER) {
      char aux_USR_Data[300] = {};

      if (user_validateData->permition == '1' ||
          user_validateData->permition == '2') {
        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {
          /*memset(&rsp, 0, sizeof(rsp));*/
          asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param,
                   MyUser_new_LimitTime(BLE_SMS_Indication, payload, mqttInfo));

          return rsp;
        } else if (BLE_SMS_Indication == SMS_INDICATION) {
          /*memset(&rsp, 0, sizeof(rsp));*/
          // memset(&file_contents_Users, 0, sizeof(file_contents_Users));

          // sprintf(file_contents_Users, "%s %c %c %s", USER_ELEMENT, cmd,
          // param, payload); send_UDP_queue(file_contents_Users);
          asprintf(&rsp, "%s",
                   MyUser_new_LimitTime(BLE_SMS_Indication, payload, mqttInfo));

          // ////printf("\n\n last time 22\n\n");
          return rsp;
        } else {
          return return_ERROR_Codes(&rsp,
                                    return_Json_SMS_Data("ERROR_INPUT_DATA"));
        }
      } else {
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == DATE_PARAMETER) {
      char aux_USR_Data[300] = {};
      if (user_validateData->permition == '1' ||
          user_validateData->permition == '2') {
        /*memset(&rsp, 0, sizeof(rsp));*/
        memset(buffer, 0, sizeof(buffer));

        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {
          /*memset(&rsp, 0, sizeof(rsp));*/
          memset(buffer, 0, sizeof(buffer));
          asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param,
                   MyUser_new_StartDate(BLE_SMS_Indication, payload, &rsp,
                                        mqttInfo));
          return rsp;
        } else if (BLE_SMS_Indication == SMS_INDICATION) {
          /*memset(&rsp, 0, sizeof(rsp));*/
          memset(&file_contents_Users, 0, sizeof(file_contents_Users));

          asprintf(&rsp, "%s",
                   MyUser_new_StartDate(BLE_SMS_Indication, payload, &rsp,
                                        mqttInfo));
          // ////printf("\n\n new start date sms  - %s\n\n", rsp);
          return rsp;
        } else {
          return return_ERROR_Codes(&rsp,
                                    return_Json_SMS_Data("ERROR_INPUT_DATA"));
        }
      } else {
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == NAME_PARAMETER) {
      char aux_USR_Data[300] = {};
      if (user_validateData->permition == '1' ||
          user_validateData->permition == '2') {
        /*memset(&rsp, 0, sizeof(rsp));*/
        memset(buffer, 0, sizeof(buffer));
        MyUser_new_UserName(BLE_SMS_Indication, payload, &buffer,
                            user_validateData->permition, mqttInfo);

        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {
          asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param, buffer);
          return rsp;
        } else if (BLE_SMS_Indication == SMS_INDICATION) {
          /*memset(&rsp, 0, sizeof(rsp));*/
          memset(&file_contents_Users, 0, sizeof(file_contents_Users));

          asprintf(&rsp, "%s", buffer);
          // ////printf("\n\n NAME NAME %s\n\n", rsp);
          return rsp;
        } else {
          return return_ERROR_Codes(&rsp,
                                    return_Json_SMS_Data("ERROR_INPUT_DATA"));
        }
      } else {
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == RELE_RESTRITION_PARAMETER) {
      char aux_USR_Data[300] = {};

      if (user_validateData->permition == '1' ||
          user_validateData->permition == '2') {
        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {
          asprintf(&rsp, "UR S R %s",
                   MyUser_Set_Relay_Restrition(user_validateData,
                                               BLE_SMS_Indication, payload,
                                               mqttInfo));
        } else if (BLE_SMS_Indication == SMS_INDICATION) {
          /*memset(&rsp, 0, sizeof(rsp));*/

          /*  memset(&file_contents_Users, 0, sizeof(file_contents_Users));
           erase_Password_For_Rsp(aux_USR_Data, file_contents_Users);
           asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param,
           file_contents_Users); asprintf(&rsp, "%s", buffer); */
          asprintf(&rsp, "%s",
                   MyUser_Set_Relay_Restrition(user_validateData,
                                               BLE_SMS_Indication, payload,
                                               mqttInfo));
          // ////printf("\n\n NAME NAME %s\n\n", rsp);
          return rsp;
        } else {
          return return_ERROR_Codes(&rsp,
                                    return_Json_SMS_Data("ERROR_INPUT_DATA"));
        }

        return rsp;
      } else {
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else {

      // ////printf(ERROR_PARAMETER);
      return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_PARAMETER"));
    }
  } else if (cmd == GET_CMD) {
    // if (param == ADMIN_PARAMETER)
    // {
    //     if (BLE_SMS_Indication == BLE_INDICATION)
    //     {
    //         /*memset(&rsp, 0, sizeof(rsp));*/
    //         asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param,
    //         user_validateData->key);
    //         ////printf("key key %s\n", user_validateData->key);
    //         return rsp;
    //     }
    //     else if (BLE_SMS_Indication == SMS_INDICATION)
    //     {
    //         /*memset(&rsp, 0, sizeof(rsp));*/
    //         asprintf(&rsp, "YOUR KEY IS: %s", user_validateData->key);
    //         ////printf("key key %s\n", user_validateData->key);
    //         return rsp;
    //     }
    //     else
    //     {
    //         return return_ERROR_Codes(&rsp,
    //         return_Json_SMS_Data("ERROR_INPUT_DATA"));
    //     }
    // }
    // else
    if (param == USER_PARAMETER) {
      char auxPhoneNumber[20] = {};
      sprintf(auxPhoneNumber, "%s",
              check_IF_haveCountryCode(user_validateData->phone, 0));
      if (user_validateData->permition == '1' ||
          user_validateData->permition == '2') {

        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {
          /*memset(&rsp, 0, sizeof(rsp));*/
          // ////printf("\n enter get ble user\n");
          asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param,
                   MyUser_ReadUser(BLE_SMS_Indication, payload,
                                   user_validateData->permition,
                                   auxPhoneNumber));

          return rsp;
        } else if (BLE_SMS_Indication == SMS_INDICATION) {
          /*memset(&rsp, 0, sizeof(rsp));*/
          // ////printf("\n enter get sms user payload %s\n", payload);
          asprintf(&rsp, "%s",
                   MyUser_ReadUser(BLE_SMS_Indication, payload,
                                   user_validateData->permition,
                                   auxPhoneNumber));
          // ////printf("end read user");
          return rsp;
        } else {
          return return_ERROR_Codes(&rsp,
                                    return_Json_SMS_Data("ERROR_INPUT_DATA"));
        }
      } else if (BLE_SMS_Indication == BLE_INDICATION &&
                 !strcmp(check_IF_haveCountryCode(auxPhoneNumber, 0),
                         check_IF_haveCountryCode(payload, 0))) {

        /*memset(&rsp, 0, sizeof(rsp));*/
        // ////printf("\n enter get ble user same number\n");
        asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param,
                 MyUser_ReadUser(BLE_SMS_Indication, payload,
                                 user_validateData->permition, auxPhoneNumber));
        return rsp;
      } else {
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    }
    /*         else if (param == ALL_PARAMETER)
            {
            } */
    else if (param == HOUR_PARAMETER) {
      if (user_validateData->permition == '1' ||
          user_validateData->permition == '2') {
        int line = 0;
        char hour_rsp[20] = {};
        memset(file_contents_Users, 0, sizeof(file_contents_Users));
        memset(hour_rsp, 0, sizeof(hour_rsp));
        /*memset(&rsp, 0, sizeof(rsp));*/

        if (MyUser_Search_User(payload, file_contents_Users) == ESP_OK) {
          MyUser user_Hour_ValidateData;
          memset(&user_Hour_ValidateData, 0, sizeof(user_Hour_ValidateData));
          parse_ValidateData_User(file_contents_Users, &user_Hour_ValidateData);

          if (user_Hour_ValidateData.permition == '1' ||
              user_Hour_ValidateData.permition == '2') {

            if (user_Hour_ValidateData.permition == '1') {
              return return_ERROR_Codes(
                  &rsp,
                  return_Json_SMS_Data("ERROR_THIS_IMPOSSIBLE_CHANGE_ADMIN"));
            } else {
              return return_ERROR_Codes(
                  &rsp,
                  return_Json_SMS_Data("ERROR_THIS_IMPOSSIBLE_CHANGE_OWNER"));
            }
          }
          // ////printf("read user 1q2w3e %s\n", file_contents_Users);

          for (int i = 0; i < 8; i++) {
            if (i < 4) {
              hour_rsp[i] = user_Hour_ValidateData.start.hour[i];
            } else {
              hour_rsp[i] = user_Hour_ValidateData.end.hour[i - 4];
            }
          }

          if (BLE_SMS_Indication == BLE_INDICATION ||
              BLE_SMS_Indication == UDP_INDICATION) {
            asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param, hour_rsp);
            // ////printf("rsp get hour %s\n", rsp);
            //   asprintf(&rsp, "%s%s", user_Hour_ValidateData.start.hour,
            //   user_Hour_ValidateData.end.hour);
            return rsp;
          } else if (BLE_SMS_Indication == SMS_INDICATION) {
            asprintf(&rsp, return_Json_SMS_Data("USER_START_HOUR_END_HOUR"),
                     user_Hour_ValidateData.firstName,
                     user_Hour_ValidateData.phone, hour_rsp[0], hour_rsp[1],
                     hour_rsp[2], hour_rsp[3], hour_rsp[4], hour_rsp[5],
                     hour_rsp[6], hour_rsp[7]);
            // ////printf("rsp get hour %s\n", rsp);
            //   asprintf(&rsp, "%s%s", user_Hour_ValidateData.start.hour,
            //   user_Hour_ValidateData.end.hour);
            return rsp;
          } else {
            return return_ERROR_Codes(&rsp,
                                      return_Json_SMS_Data("ERROR_INPUT_DATA"));
          }
        } else {
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_USER_NOT_FOUND"));
        }
      } else {
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == NAME_PARAMETER) {
      if (BLE_SMS_Indication == BLE_INDICATION ||
          BLE_SMS_Indication == UDP_INDICATION) {
        if (user_validateData->permition == '1' ||
            user_validateData->permition == '2') {
          /*memset(&rsp, 0, sizeof(rsp));*/
          MyUser_getFromName(payload, gattsIF, connID, handle_table);
          return rsp;
        } else {
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
        }
      } else {
        return return_ERROR_Codes(&rsp,
                                  return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
      }
    } else if (param == RELE_RESTRITION_PARAMETER) {
      if (BLE_SMS_Indication == SMS_INDICATION) {
        if (user_validateData->permition == '1' ||
            user_validateData->permition == '2') {
          memset(&rsp, 0, sizeof(rsp));
          asprintf(&rsp, "%s",
                   MyUser_get_ReleRestrition(payload, SMS_INDICATION));

          return rsp;
        } else {
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
        }
      }
    } else if (param == WEEK_PARAMETER) {
      if (user_validateData->permition == '1' ||
          user_validateData->permition == '2') {
        int line = 0;
        char week_rsp[200];
        memset(file_contents_Users, 0, sizeof(file_contents_Users));
        memset(week_rsp, 0, sizeof(week_rsp));
        /*memset(&rsp, 0, sizeof(rsp));*/

        if (MyUser_Search_User(payload, file_contents_Users) == ESP_OK) {
          MyUser user_Week_ValidateData;
          memset(&user_Week_ValidateData, 0, sizeof(user_Week_ValidateData));

          parse_ValidateData_User(file_contents_Users, &user_Week_ValidateData);

          if (user_Week_ValidateData.permition == '1' ||
              user_Week_ValidateData.permition == '2') {

            if (user_Week_ValidateData.permition == '1') {
              return return_ERROR_Codes(
                  &rsp,
                  return_Json_SMS_Data("ERROR_THIS_IMPOSSIBLE_CHANGE_ADMIN"));
            } else {
              return return_ERROR_Codes(
                  &rsp,
                  return_Json_SMS_Data("ERROR_THIS_IMPOSSIBLE_CHANGE_OWNER"));
            }
          }

          if (BLE_SMS_Indication == BLE_INDICATION ||
              BLE_SMS_Indication == UDP_INDICATION) {
            // ////printf("read user 1q2w3e %s\n", file_contents_Users);
            asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param,
                     user_Week_ValidateData.week);
            // ////printf("rsp get week %s\n", rsp);
            //   asprintf(&rsp, "%s%s", user_Hour_ValidateData.start.hour,
            //   user_Hour_ValidateData.end.hour);
            return rsp;
          } else if (BLE_SMS_Indication == SMS_INDICATION) {
            char auxWeekdays[30];
            char ch[2] = {' ', '\0'};
            memset(auxWeekdays, 0, sizeof(auxWeekdays));
            uint8_t weekdays_Count = 0;

            for (int i = 0; i < strlen(user_Week_ValidateData.week); i++) {
              if (user_Week_ValidateData.week[i] == '1') {
                weekdays_Count++;
                strcat(auxWeekdays, return_Json_SMS_Data(weekDaysFeedback[i]));
                strcat(auxWeekdays, ch);
              }
            }

            if (weekdays_Count == 7 || user_Week_ValidateData.permition > '0') {
              strcpy(auxWeekdays, return_Json_SMS_Data("ALL DAYS"));
            }

            asprintf(&rsp, return_Json_SMS_Data("USER_WEEK_DAYS"),
                     user_Week_ValidateData.firstName,
                     user_Week_ValidateData.phone, auxWeekdays);
            // ////printf("rsp get week %s\n", rsp);
            //   asprintf(&rsp, "%s%s", user_Hour_ValidateData.start.hour,
            //   user_Hour_ValidateData.end.hour);
            return rsp;
          } else {
            return return_ERROR_Codes(&rsp,
                                      return_Json_SMS_Data("ERROR_INPUT_DATA"));
          }
        } else {
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_USER_NOT_FOUND"));
        }
      } else {
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == LAST_PARAMETER) {

      if (user_validateData->permition == '1' ||
          user_validateData->permition == '2') {
        /*memset(&rsp, 0, sizeof(rsp));*/
        memset(buffer, 0, sizeof(buffer));
        MyUser_get_LimitTime(BLE_SMS_Indication, payload, &buffer);

        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {
          asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param, buffer);
          return rsp; /* code */
        } else if (BLE_SMS_Indication == SMS_INDICATION) {
          /* char aux_USR_Data[300] = {};
          memset(&file_contents_Users, 0, sizeof(file_contents_Users));
          erase_Password_For_Rsp(aux_USR_Data, file_contents_Users);
          asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param,
          file_contents_Users); asprintf(&rsp, "%s", buffer); */
          asprintf(&rsp, "%s", buffer);
          return rsp; /* code */
        }
      } else {
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else if (param == DATE_PARAMETER) {
      if (user_validateData->permition == '1' ||
          user_validateData->permition == '2') {
        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {
          /*memset(&rsp, 0, sizeof(rsp));*/
          asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param,
                   MyUser_get_startDate(BLE_SMS_Indication, payload));
          return rsp;
        } else if (BLE_SMS_Indication == SMS_INDICATION) {
          asprintf(&rsp, "%s",
                   MyUser_get_startDate(BLE_SMS_Indication, payload));
          return rsp;
        } else {
          return return_ERROR_Codes(&rsp,
                                    return_Json_SMS_Data("ERROR_INPUT_DATA"));
        }
      } else {
        return return_ERROR_Codes(
            &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      }
    } else {

      // ////printf(ERROR_PARAMETER);
      return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_PARAMETER"));
    }
  } else if (cmd == RESET_CMD) {
    if (user_validateData->permition == '1' ||
        user_validateData->permition == '2') {
      if (param == USER_PARAMETER) {
        memset(file_contents_Users, 0, sizeof(file_contents_Users));
        if (MyUser_Search_User(payload, file_contents_Users) == ESP_OK) {
          MyUser user_deleteValidateData = {};

          parse_ValidateData_User(file_contents_Users,
                                  &user_deleteValidateData);
          // Se for owner ou proprietario nao deixa eliminar
          if (user_deleteValidateData.permition == '1' ||
              user_deleteValidateData.permition == '2') {

            // ////printf("este user é admin nao eliminado\n");
            return return_ERROR_Codes(
                &rsp, return_Json_SMS_Data("ERROR_USER_NOT_IS_GUEST"));
          } else {
            if (Myuser_deleteUser(&user_deleteValidateData) == ESP_OK) {
              if (BLE_SMS_Indication == BLE_INDICATION ||
                  BLE_SMS_Indication == UDP_INDICATION) {
                /*memset(&rsp, 0, sizeof(rsp));*/
                if (strlen(user_deleteValidateData.phone) < 1 &&
                    user_deleteValidateData.wiegand_code[0] != ':') {
                  asprintf(&rsp, "%s %c %c $%s", USER_ELEMENT, cmd, param,
                           user_deleteValidateData.wiegand_code);
                } else if (strlen(user_deleteValidateData.phone) < 1 &&
                           user_deleteValidateData.wiegand_code[0] == ':') {
                  asprintf(&rsp, "%s %c %c &%s", USER_ELEMENT, cmd, param,
                           user_deleteValidateData.rf_serial);
                } else {
                  asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param,
                           user_deleteValidateData.phone);
                }

                return rsp;
              } else if (BLE_SMS_Indication == SMS_INDICATION) {
                /*  char aux_USR_Data[300] = {};
                 memset(&file_contents_Users, 0, sizeof(file_contents_Users));
                 erase_Password_For_Rsp(aux_USR_Data, file_contents_Users);
                 asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param,
                 file_contents_Users); asprintf(&rsp, "%s", buffer); */

                memset(buffer_UDP_users, 0, sizeof(buffer_UDP_users));

                sprintf(mqttInfo->data, "%s %c %c %s", USER_ELEMENT, RESET_CMD,
                        USER_PARAMETER, user_deleteValidateData.phone);
                send_UDP_queue(mqttInfo);
                /*memset(&rsp, 0, sizeof(rsp));*/
                asprintf(&rsp, return_Json_SMS_Data("USER_HAS_BEEN_DELETED"),
                         user_deleteValidateData.phone);
                return rsp;
              } else {
                return return_ERROR_Codes(
                    &rsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
              }
            } else {
              if (BLE_SMS_Indication == BLE_INDICATION ||
                  BLE_SMS_Indication == UDP_INDICATION) {
                /*memset(&rsp, 0, sizeof(rsp));*/
                asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param,
                         ERROR_RESET);
                return rsp;
              } else if (BLE_SMS_Indication == SMS_INDICATION) {
                /*memset(&rsp, 0, sizeof(rsp));*/
                /*  char aux_USR_Data[300] = {};
                 memset(&file_contents_Users, 0, sizeof(file_contents_Users));
                 erase_Password_For_Rsp(aux_USR_Data, file_contents_Users);
                 asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param,
                 file_contents_Users); asprintf(&rsp, "%s", buffer); */

                asprintf(
                    &rsp,
                    return_Json_SMS_Data("ERROR_USER_HAS_NOT_BEEN_DELETED"),
                    user_deleteValidateData.phone);
                return rsp;
              } else {
                return return_ERROR_Codes(
                    &rsp,
                    return_Json_SMS_Data("ERROR_USER_HAS_NOT_BEEN_DELETED"));
              }
            }
          }

          // ////printf("read user 1123 %s\n", file_contents_Users);
          return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_RESET"));
        } else {

          // ////printf(ERROR_USER_NOT_FOUND);
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_USER_NOT_FOUND"));
        }
      } else if (param == ALL_PARAMETER) {
        if (user_validateData->permition == '1' ||
            user_validateData->permition == '2') {
          if (Myuser_delete_ALLUser()) {

            if (BLE_SMS_Indication == BLE_INDICATION ||
                BLE_SMS_Indication == UDP_INDICATION) {
              /*memset(&rsp, 0, sizeof(rsp));*/
              asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param, "OK");
              return rsp;
            } else if (BLE_SMS_Indication == SMS_INDICATION) {
              /*memset(&rsp, 0, sizeof(rsp));*/
              /* char aux_USR_Data[300] = {};
              memset(&file_contents_Users, 0, sizeof(file_contents_Users));
              erase_Password_For_Rsp(aux_USR_Data, file_contents_Users);
              asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param,
              file_contents_Users); asprintf(&rsp, "%s", buffer); */
              memset(buffer_UDP_users, 0, sizeof(buffer_UDP_users));

              sprintf(mqttInfo->data, "%s %c %c %s", USER_ELEMENT, RESET_CMD,
                      ALL_PARAMETER, "OK");
              send_UDP_queue(mqttInfo);

              asprintf(&rsp, "%s",
                       return_Json_SMS_Data("ALL_USERS_HAVE_BEEN_DELETED"));
              return rsp;
            } else {
              return return_ERROR_Codes(
                  &rsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
            }
          } else {

            return return_ERROR_Codes(&rsp,
                                      return_Json_SMS_Data("ERROR_RESET"));
          }
        } else {
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
        }
      } else if (param == USER_RESET_PASSWORD_PARAMETER) {
        // Myuser_delete_ALLUser();

        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {
          /*memset(&rsp, 0, sizeof(rsp));*/
          asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param,
                   MyUser_Restore_Password(payload,
                                           user_validateData->permition,
                                           BLE_SMS_Indication));
          return rsp;
        } else if (BLE_SMS_Indication == SMS_INDICATION) {
          /*  char aux_USR_Data[300] = {};
           memset(&file_contents_Users, 0, sizeof(file_contents_Users));
           erase_Password_For_Rsp(aux_USR_Data, file_contents_Users);
           asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param,
           file_contents_Users); asprintf(&rsp, "%s", buffer); */

          /*memset(&rsp, 0, sizeof(rsp));*/
          asprintf(&rsp, "%s",
                   MyUser_Restore_Password(payload,
                                           user_validateData->permition,
                                           BLE_SMS_Indication));
          return rsp;
        } else {
          return return_ERROR_Codes(&rsp,
                                    return_Json_SMS_Data("ERROR_INPUT_DATA"));
        }
      } else if (param == USER_RESET_BLE_SECURITY_PARAMETER) {
        if (user_validateData->permition == '2') {

          asprintf(&rsp, "%s",
                   reset_Label_BLE_Security(payload, BLE_SMS_Indication));
          return rsp;
        } else {
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
        }
      } else if (param == LAST_PARAMETER) {
        /*memset(&rsp, 0, sizeof(rsp));*/
        memset(buffer, 0, sizeof(buffer));
        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {
          asprintf(
              &rsp, "%s %c %c %s", USER_ELEMENT, cmd, param,
              MyUser_reset_LimitTime(BLE_SMS_Indication, payload, mqttInfo));
          return rsp;
        } else if (BLE_SMS_Indication == SMS_INDICATION) {
          /* char aux_USR_Data[300] = {};
          memset(&file_contents_Users, 0, sizeof(file_contents_Users));
          erase_Password_For_Rsp(aux_USR_Data, file_contents_Users);
          asprintf(&rsp, "%s %c %c %s", USER_ELEMENT, cmd, param,
          file_contents_Users); asprintf(&rsp, "%s", buffer); */

          asprintf(
              &rsp, "%s",
              MyUser_reset_LimitTime(BLE_SMS_Indication, payload, mqttInfo));
          return rsp;
        } else {
          return return_ERROR_Codes(&rsp,
                                    return_Json_SMS_Data("ERROR_INPUT_DATA"));
        }
      } else {

        // ////printf(ERROR_PARAMETER);
        return return_ERROR_Codes(&rsp,
                                  return_Json_SMS_Data("ERROR_PARAMETER"));
      }
    } else {

      // ////printf(ERROR_USER_NOT_PERMITION);
      return return_ERROR_Codes(
          &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
    }
  } else {

    // ////printf(ERROR_CMD);
    return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_CMD"));
  }

  return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
}

char totalErrorUsers[2000];

char *import_mqtt_users(char *URL, char *ownerNumber, char *ownerPassword) {
  char AT_Command[100];
  EG915_readDataFile_struct.mode = EG91_FILE_USERS_MODE;

  // timer_pause(TIMER_GROUP_1, TIMER_0);
  //  ////printf("\nsms\n");
  // disableAlarm();
  // vTaskSuspend(xHandle_Timer_VerSystem);
  // xSemaphoreGive(rdySem_Control_Send_AT_Command);

  EG91_send_AT_Command("AT+QIACT?", "OK", 1000);
  EG91_send_AT_Command("AT+QHTTPCFG=\"contextid\",1", "OK", 1000);
  EG91_send_AT_Command("AT+QHTTPCFG=\"responseheader\",1 ", "OK", 1000);
  // snprintf(atCommand, sizeof(atCommand), "AT+QHTTPURL=%d,%d\r\n%s%c",
  // strlen("https://www.alipay.com"), 60, "https://www.alipay.com",13);

  EG91_send_AT_Command("AT+QHTTPCFG=\"sslctxid\",1", "OK", 1000);
  EG91_send_AT_Command("AT+QSSLCFG=\"sslversion\",1,3", "OK", 1000);
  EG91_send_AT_Command("AT+QSSLCFG=\"ciphersuite\",1,0XC02F", "OK", 1000);
  EG91_send_AT_Command("AT+QSSLCFG=\"seclevel\",1,0 ", "OK", 1000);
  EG91_send_AT_Command("AT+QSSLCFG=\"ignoreinvalidcertsign\",1,1 ", "OK", 1000);
  EG91_send_AT_Command("AT+QSSLCFG=\"sni\",1,1", "OK", 1000);
  EG91_send_AT_Command("AT+QHTTPCFG=\"responseheader\",0", "OK", 1000);

  // EG91_send_AT_Command("AT+QSSLCFG=\"renegotiation\",1,1","OK",1000);
  // EG91_send_AT_Command("AT+QSSLOPEN=1,1,4,\"https://api.mconnect.motorline.pt/health\",443,0","OK",1000);
  // vTaskDelay(pdMS_TO_TICKS(5000));
  sprintf(AT_Command, "%s%d%s%c", "AT+QHTTPURL=", strlen(URL), ",80", 13);
  EG91_send_AT_Command(AT_Command, "CONNECT", 60000);
  // uart_write_bytes(UART_NUM_1, atCommand, strlen(atCommand));
  vTaskDelay(pdMS_TO_TICKS(520));
  // EG91_send_AT_Command(atCommand,"OK",60000);
  EG91_send_AT_Command(URL, "OK", 60000);

  // EG91_send_AT_Command("AT","OK",10000);"https://static.portal.motorline.pt/firmwares/testeImport.txt"

  EG91_send_AT_Command("AT+QHTTPGET=80", "QHTTPGET", 10000);
  // vTaskDelay(pdMS_TO_TICKS(10000));
  // EG91_send_AT_Command("AT+QHTTPREAD=80","OK",10000);

  // EG91_send_AT_Command("AT+QHTTPREAD=80", "OK", 10000);
  EG91_send_AT_Command("AT+QHTTPREADFILE=\"UFS:users.txt\",80", "OK", 10000);
  EG91_send_AT_Command("ATE1", "OK", 1000);
  uint8_t ACK = 0;
  ACK = EG91_send_AT_Command("AT+QFOPEN=\"UFS:users.txt\"", "+QFOPEN:", 20000);

  esp_err_t err;
  int idFile = atoi(fileID);
  int nowFileSize = 0;
  int usersErroCount = 1;
  char saveImportUsers[300] = {};
  memset(totalErrorUsers, 0, sizeof(totalErrorUsers));
  // printf("\nfile id %d - %s", idFile, fileID);

  char importUsers_data[200] = {};
  sprintf(totalErrorUsers, "%s", "UR S I ");

  if (ACK) {

    sprintf(AT_Command, "AT+QFREAD=%d,1024", idFile);
    int count = 0;
    int strIndex = 0;

    // ESP_ERROR_CHECK(heap_trace_start(HEAP_TRACE_LEAKS));

    while (nowFileSize < EG915_readDataFile_struct.fileSize) {
      // xSemaphoreGive(rdySem_Control_Send_AT_Command);
      EG91_send_AT_Command(/* "AT+QFREAD=1027,1024" */ AT_Command, "+QFREAD",
                           3000);
      nowFileSize += EG915_readDataFile_struct.nowFileSize;

      for (size_t i = 0; i < strlen(EG915_readDataFile_struct.receiveData);
           i++) {
        if (EG915_readDataFile_struct.receiveData[i] != '\n') {
          importUsers_data[strIndex++] =
              EG915_readDataFile_struct.receiveData[i];
        } else {

          // printf("\nimport users - %s - %d", importUsers_data,
          // strlen(EG915_readDataFile_struct.receiveData));

          sprintf(saveImportUsers, "%s %s %s", ownerNumber, ownerPassword,
                  importUsers_data);
          // printf("\n\nimport users data- %s\n\n", saveImportUsers);

          if (strlen(importUsers_data) > 2) {
            // printf("\n\nimport users data421- \n\n");
            char *mqttImportUsers_send = parseInputData(
                &saveImportUsers, UDP_INDICATION, NULL, NULL, NULL, NULL, NULL);

            // printf("\n\nimport users dat1111- %s\n\n",mqttImportUsers_send);

            char strCount[10] = {};
            sprintf(strCount, "%d", usersErroCount);
            char *newline_position = strchr(mqttImportUsers_send, '\n');
            // printf("\n\nimport users data9876- \n\n");
            if (newline_position != NULL) {
              strcat(totalErrorUsers, strCount);
              strcat(totalErrorUsers, newline_position + 1);
              strcat(totalErrorUsers, ";\0");
              usersErroCount++;
            } else {
              usersErroCount++;
            }

            // free(newline_position);
            //  send_UDP_Package(mqttImportUsers_send,
            //  strlen(mqttImportUsers_send), "");
            //   send_UDP_Send(mqttImportUsers_send, "");
            ////printf("\n\nimport users data6366- %s \n\n",newline_position);

            strIndex = 0;

            // printf("\n\nimport users dataqqwq- \n\n");
            memset(importUsers_data, 0, sizeof(importUsers_data));
            free(mqttImportUsers_send);
            // printf("\n\nimport users datacqawq- \n\n");
          }
        }
        // vTaskDelay(pdMS_TO_TICKS(5));

        if (EG915_readDataFile_struct.receiveData[i] == EOF) {
          break;
        }
      }
    }
    // printf("\n\nimport users data22- %s\n\n", importUsers_data);
    if (strlen(importUsers_data) > 2) {
      char saveImportUsers[300] = {};
      sprintf(saveImportUsers, "%s %s %s", ownerNumber, ownerPassword,
              importUsers_data);
      // printf("\n\nimport users data232- \n\n");
      char *mqttImportUsers_send = parseInputData(
          &saveImportUsers, UDP_INDICATION, NULL, NULL, NULL, NULL, NULL);

      char strCount[10] = {};
      sprintf(strCount, "%d", usersErroCount);
      char *newline_position = strchr(mqttImportUsers_send, '\n');
      // printf("\n\nimport users data5656- \n\n");
      if (newline_position != NULL) {
        strcat(totalErrorUsers, strCount);
        strcat(totalErrorUsers, newline_position + 1);
        strcat(totalErrorUsers, ";\0");
        usersErroCount++;
      } else {
        usersErroCount++;
      }
      // free(newline_position);
      // printf("\n\nimport users data888- \n\n");
      // free(newline_position);
      // printf("\n\nimport users data555- \n\n");
      free(mqttImportUsers_send);
    }
  }

  sprintf(AT_Command, "AT+QFCLOSE=%d", idFile);
  EG91_send_AT_Command(AT_Command, "OK", 3000);

  EG91_send_AT_Command("AT+QFDEL=\"UFS:users.txt\"", "OK", 1000);
  // vTaskResume(xHandle_Timer_VerSystem);
  // timer_start(TIMER_GROUP_1, TIMER_0);

  // send_UDP_Send(totalErrorUsers, "");

  EG915_readDataFile_struct.mode = EG91_FILE_NORMAL_MODE;
  // enableAlarm();
  return totalErrorUsers;
}

char *reset_Label_BLE_Security(char *payload, uint8_t BLE_SMS) {
  char aux_payload[200] = {};
  if (strlen(payload) > 20) {
    return_Json_SMS_Data("ERROR_INPUT_DATA");
  }

  for (size_t i = 0; i < strlen(payload); i++) {
    if (payload[i] < '0' || payload[i] > '9') {
      if (payload[i] != '+') {
        return_Json_SMS_Data("ERROR_INPUT_DATA");
      }
    }
  }

  /*memset(&rsp, 0, sizeof(rsp));*/

  if (MyUser_Search_User(payload, aux_payload) == ESP_OK) {
    MyUser MyUser_label_BLE_Security;
    memset(&MyUser_label_BLE_Security, 0, sizeof(MyUser_label_BLE_Security));
    parse_ValidateData_User(aux_payload, &MyUser_label_BLE_Security);

    MyUser_label_BLE_Security.ble_security = '0';

    if (replaceUser(&MyUser_label_BLE_Security) == ESP_OK) {
      // ////printf("\n\nBLE LABEL RESTORE 55\n\n");
      if (BLE_SMS == BLE_INDICATION || BLE_SMS == UDP_INDICATION) {
        // ////printf("\n\nBLE LABEL RESTORE 61\n\n");
        return "UR R B OK";
      } else if (BLE_SMS == SMS_INDICATION) {
        memset(file_contents_Users, 0, sizeof(file_contents_Users));
        // ////printf("\n\nBLE LABEL RESTORE 62\n\n");
        sprintf(file_contents_Users,
                return_Json_SMS_Data("RESET_LABEL_BLE_SECURITY"));
        return file_contents_Users;
      }
    } else {
      return return_Json_SMS_Data("ERROR_RESET_LABEL_BLE_SECURITY");
    }
  } else {
    return return_Json_SMS_Data("ERROR_USER_NOT_FOUND");
  }

  return "ok";
}

char *MyUser_Restore_Password(char *payload, char permition, uint8_t BLE_SMS) {
  MyUser user_validateData;
  memset(&user_validateData, 0, sizeof(user_validateData));
  char auxPhNumber[30] = {};
  /* char aux_rsp[200] = {}; */
  char user_Restore_Data[200] = {};

  if (strlen(payload) < 20) {

    // ////printf("\n\nMyUser_Restore_Password 11\n\n");
    sprintf(auxPhNumber, "%s", check_IF_haveCountryCode(payload, 0));
    // ////printf("\n\nMyUser_Restore_Password 22 - %s\n\n", auxPhNumber);
    if (MyUser_Search_User(auxPhNumber, &user_Restore_Data) == ESP_OK) {
      // ////printf("\n\nuser_Restore_Data - %s\n\n", user_Restore_Data);
      parse_ValidateData_User(user_Restore_Data, &user_validateData);
      // ////printf("\n\nMyUser_Restore_Password 33\n\n");
      if (user_validateData.permition < permition) {
        // ////printf("\n\nMyUser_Restore_Password 44\n\n");
        //  memset(user_validateData.key, 0, sizeof(user_validateData.key));

        if (user_validateData.permition == '0') {
          sprintf(user_validateData.key, "%s", DEFAULT_USER_PASSWORD);
        } else if (user_validateData.permition == '1') {
          sprintf(user_validateData.key, "%s", DEFAULT_ADMIN_PASSWORD);
        }

        // user_validateData.key[7] = 0;

        char newline[200];

        /* Remove extra new line character from stdin */
        // fflush(stdin);
        memset(&newline, 0, 200);
        // user_validateData.week[7] = '\0';

        sprintf(&newline, "%s;%s;%s;%s;%s;%s;%s;%c;%s;%c;%c;%c;",
                user_validateData.phone, user_validateData.firstName,
                user_validateData.start.date, user_validateData.start.hour,
                user_validateData.end.days, user_validateData.end.hour,
                user_validateData.key, user_validateData.permition,
                user_validateData.week, user_validateData.relayPermition,
                user_validateData.ble_security,
                user_validateData.erase_User_After_Date);
        // ////printf("new line MyUser_Restore_Password %s\n", newline);
        if (replaceUser(&user_validateData) == ESP_OK) {
          // ////printf("\n\nMyUser_Restore_Password 55\n\n");
          if (BLE_SMS == BLE_INDICATION || BLE_SMS == UDP_INDICATION) {
            // ////printf("\n\nMyUser_Restore_Password 61\n\n");
            return "OK";
          } else if (BLE_SMS == SMS_INDICATION) {
            memset(file_contents_Users, 0, sizeof(file_contents_Users));
            // ////printf("\n\nMyUser_Restore_Password 62\n\n");
            sprintf(file_contents_Users,
                    return_Json_SMS_Data("RESTORE_PASSWORD"),
                    user_validateData.key);
            return file_contents_Users;
          }
        } else {
          return return_Json_SMS_Data("ERROR_RESTORE_PASSWORD");
        }
      } else {

        if ((user_validateData.permition == '2' && permition == '2')) {
          return return_Json_SMS_Data("OWNER_WRONG_CMD_RESET_PASSWORD");
        }

        return return_Json_SMS_Data("ERROR_USER_NOT_PERMITION");
      }
    } else {
      return return_Json_SMS_Data("ERROR_USER_NOT_FOUND");
    }
  }

  return return_Json_SMS_Data("ERROR_INPUT_DATA");
}

uint8_t MyUser_newUSER(char *payload, char *newUserData, char *phNumber,
                       char *password) {
  MyUser user_validateData; //= malloc(sizeof(MyUser));
  memset(&user_validateData, 0, sizeof(user_validateData));
  int dotCounter = 0;

  char auxfileContents[200] = {};

  // ////printf("\n\n\nMYNEWUSER FUNCTION\n\n\n");

  for (int i = 0; i < strlen(payload); i++) {
    if (payload[i] == '.') {
      dotCounter++;
    }
  }

  if (payload[strlen(payload) - 1] == ' ') {
    payload[strlen(payload) - 1] = 0;
  }

  uint8_t auxIndex = 0;
  uint8_t aux = 0;

  // ////printf("add user payload-%s len-%d - dotcounter %d\n", payload,
  // strlen(payload), dotCounter);

  user_validateData.permition = '0';
  printf("\n\n\n dot counter %d\n\n\n", dotCounter);
  if (dotCounter == 7 || dotCounter == 9 || dotCounter == 12) {
    for (int i = 0; i < strlen(payload); i++) {
      if (payload[i] == '.') {
        aux++;
        auxIndex = 0;
      } else {
        if (aux == 0) {
          user_validateData.phone[auxIndex] = payload[i];
          auxIndex++;

          if (auxIndex == 20) {
            // ////printf("\nERROR_SET phone\n");
            // ////printf(ERROR_SET);
            return ERROR_WRONG_PHONE_NUMBER;
          }
        }

        if (aux == 1) {
          user_validateData.firstName[auxIndex] = payload[i];
          auxIndex++;

          if (auxIndex == 21) {
            // ////printf("\nERROR_SET firstname\n");
            return ERROR_NAME_LENGTH;
          }
        }

        if (aux == 2) {
          if (auxIndex == 8) {
            // ////printf("\nERROR_SET start/end hour\n");
            return ERROR_WRONG_TIME_DATA;
          }

          if (auxIndex < 4) {
            user_validateData.start.hour[auxIndex] = payload[i];
            auxIndex++;
          } else {
            user_validateData.end.hour[auxIndex - 4] = payload[i];
            auxIndex++;
          }
        }

        if (aux == 3) {
          if (auxIndex == 6) {
            // ////printf("\nERROR_SET startdate\n");
            return ERROR_WRONG_START_DATE;
          }

          user_validateData.start.date[auxIndex] = payload[i];
          auxIndex++;
        }

        if (aux == 4) {
          if (auxIndex == 7) {
            // ////printf("\nERROR_SET week\n");
            return ERROR_WRONG_WEEK_DAYS;
          }

          user_validateData.week[auxIndex] = payload[i];
          auxIndex++;
        }

        if (aux == 5) {
          if (auxIndex == 2) {
            // ////printf("\nERROR_SET end days\n");
            return ERROR_WRONG_LIMIT_DATE;
          }

          user_validateData.end.days[auxIndex] = payload[i];
          auxIndex++;
        }

        if (aux == 6) {
          if (auxIndex == 1) {
            // ////printf("\nERROR_SET end days\n");
            return ERROR_WRONG_RELAY_PERMITION;
          }

          user_validateData.relayPermition = payload[i];
          auxIndex++;
        }

        if (aux == 7) {
          if (auxIndex == 1) {
            // ////printf("\nERROR_SET end days\n");
            return ERROR_WRONG_ERASE_USER_AFTER_DATE;
          }

          user_validateData.erase_User_After_Date = payload[i];

          if (user_validateData.erase_User_After_Date < '0' ||
              user_validateData.erase_User_After_Date > '1') {
            return ERROR_WRONG_ERASE_USER_AFTER_DATE;
          }

          auxIndex++;
        }

#if CONFIG_WIEGAND_CODE == 1

        if (aux == 8) {
          if (auxIndex > 0) {
            // //printf("\nERROR_SET week\n");
            return 15;
          }

          user_validateData.wiegand_rele_permition = payload[i];
          auxIndex++;
        }

        if (aux == 9) {
          if (auxIndex > 19) {
            // //printf("\nERROR_SET week\n");
            return 15;
          }

          user_validateData.wiegand_code[auxIndex] = payload[i];
          auxIndex++;
        }

        if (aux == 10) {
          user_validateData.rf_serial[auxIndex] = payload[i];
          auxIndex++;
        }

        if (aux == 11) {
          if (auxIndex == 1) {
            // ////printf("\nERROR_SET end days\n");
            return ERROR_NEW_USER_ADD;
          }
          user_validateData.rf1_relay = payload[i];
          auxIndex++;
        }

        if (aux == 12) {
          if (auxIndex == 1) {
            // ////printf("\nERROR_SET end days\n");
            return ERROR_NEW_USER_ADD;
          }
          user_validateData.rf2_relay = payload[i];
          auxIndex++;
        }

        /*  if (aux == 13) {

           if (auxIndex == 1) {
             // ////printf("\nERROR_SET end days\n");
             return ERROR_NEW_USER_ADD;
           }

           user_validateData.rf3_relay = payload[i];
           auxIndex++;
         } */
        /*else
        {
             user_validateData.wiegand_code[0] = '#';
            user_validateData.wiegand_code[1] = 0;
        }*/

#endif // DEBUG
      }
    }

    /*   printf("phone - %s;start_date - %s,start_hour - %s,days - %s, end hour
       - %s, week - %s,key - %s,W.Permition - %c wiegand - %s\n",
     user_validateData.phone, user_validateData.start.date,
       user_validateData.start.hour, user_validateData.end.days,
       user_validateData.end.hour, user_validateData.week,
      user_validateData.key, user_validateData.wiegand_rele_permition,
      user_validateData.wiegand_code); */

    sprintf(user_validateData.key, "%s", DEFAULT_USER_PASSWORD);

    // funçao das horas
    if (user_validateData.start.date[0] == '*') {

      if (get_RTC_System_Time() == 0) {
        // ////printf("\n\nERROR_NEW_USER_ADD 2131\n\n");
        return ERROR_NEW_USER_ADD;
      }

      sprintf(user_validateData.start.date, "%d", nowTime.date);
    }

    user_validateData.ble_security = '0';
    // ////printf("phone112 - %s;start_date - %s,start_hour - %s,days - %s, end
    // hour - %s, week - %s,key - %s\n", user_validateData.phone,
    // user_validateData.start.date, user_validateData.start.hour,
    // user_validateData.end.days, user_validateData.end.hour,
    // user_validateData.week, user_validateData.key);
    memset(newUserData, 0, 200 /* sizeof(newUserData) */);
    sprintf(newUserData, "%s;%s;%s;%s;%s;%s;%s;%c;%s;%c;%c;%c",
            user_validateData.phone, user_validateData.firstName,
            user_validateData.start.date, user_validateData.start.hour,
            user_validateData.end.days, user_validateData.end.hour,
            user_validateData.key, user_validateData.permition,
            user_validateData.week, user_validateData.relayPermition,
            user_validateData.ble_security,
            user_validateData.erase_User_After_Date);

    // check if a var is /0
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

    /* if (user_validateData.rf3_relay == 0) {
      user_validateData.rf3_relay = ':';
    } */

    if (strlen(user_validateData.rf_serial) == 0) {
      user_validateData.rf1_relay = ':';
      user_validateData.rf2_relay = ':';
      // user_validateData.rf3_relay = ':';
      user_validateData.rf_serial[0] = ':';
    }

    char concatWie_rele[5] = {};
    sprintf(concatWie_rele, ";%c", user_validateData.wiegand_rele_permition);
    strcat(newUserData, concatWie_rele);
    strcat(newUserData, ";\0");
    strcat(newUserData, user_validateData.wiegand_code);

    strcat(newUserData, ";\0");
    strcat(newUserData, user_validateData.rf_serial);
    sprintf(concatWie_rele, ";%c", user_validateData.rf1_relay);
    strcat(newUserData, concatWie_rele);
    sprintf(concatWie_rele, ";%c;", user_validateData.rf2_relay);
    strcat(newUserData, concatWie_rele);
    /* sprintf(concatWie_rele, ";%c", user_validateData.rf3_relay);
    strcat(newUserData, concatWie_rele); */

    printf("new user bff %s\n", newUserData);

    if (MyUser_Search_User(user_validateData.phone, auxfileContents) !=
        ESP_OK) {
      printf("\n\nncheck gd 0\n\n");
      uint8_t checkUser_ACK = check_NewUser_Data(&user_validateData);
      // //printf("\n\n checkuser %d\n\n", checkUser_ACK);
      printf("\n\nncheck gd 1 - %d\n\n", checkUser_ACK);
      if (checkUser_ACK == 0) {
        return ERROR_WRONG_LIMIT_DATE;
      } else if (checkUser_ACK == 1) {
        return ERROR_PASSWORD_WRONG;
      } else if (checkUser_ACK == 2) {
        return ERROR_WRONG_WEEK_DAYS;
      } else if (checkUser_ACK == 3) {
        return ERROR_WRONG_END_TIME;
      } else if (checkUser_ACK == 4) {
        return ERROR_WRONG_START_TIME;
      } else if (checkUser_ACK == 5) {
        return ERROR_WRONG_START_DATE;
      } else if (checkUser_ACK == 6) {
        return ERROR_WRONG_PHONE_NUMBER;
      } else if (checkUser_ACK == 7) {
        return ERROR_WRONG_RELAY_PERMITION;
      } else if (checkUser_ACK == 8) {
        // //printf("CHECK USER OK\n");
        uint16_t ACK_Add_User = MyUser_Add(&user_validateData);
        if (ACK_Add_User == ESP_OK) {
          return 0;
        } else if (ACK_Add_User == 128) {
          return ERROR_LIMIT_USERS;
        } else if (ACK_Add_User == ERROR_WIEGAND_ALREADY_EXIST) {
          // //printf("\n\nnew user bff 1999\n\n");
          return ERROR_WIEGAND_ALREADY_EXIST;
        } else {
          return ERROR_NEW_USER_ADD;
        }
      } else {
        // //printf(ERROR_SET);
        return ERROR_NEW_USER_ADD;
      }
    } else {
      return ERROR_NEW_USER_ALREADY_EXISTS;
    }
  } else if (dotCounter == 1 && strlen(payload) > 1) {
    aux = 0;
    auxIndex = 0;
    for (int i = 0; i < strlen(payload); i++) {
      if (payload[i] == '.') {
        aux++;
        auxIndex = 0;
      } else {
        if (aux == 0) {
          user_validateData.phone[auxIndex] = payload[i];
          auxIndex++;

          if (auxIndex == 18) {
            // ////printf("\nERROR_SET ERROR_WRONG_PHONE_NUMBER\n");
            // ////printf(ERROR_WRONG_PHONE_NUMBER);
            return ERROR_WRONG_PHONE_NUMBER;
          }
        }

        if (aux == 1) {
          user_validateData.firstName[auxIndex] = payload[i];
          auxIndex++;

          if (auxIndex == 25) {
            // ////printf("\nERROR_NAME_LENGTH\n");
            return ERROR_NAME_LENGTH;
          }
        }
      }
    }

    for (int i = 0; i < strlen(user_validateData.phone); i++) {
      if (user_validateData.phone[i] > '9' ||
          user_validateData.phone[i] < '0') {
        if (user_validateData.phone[i] != '+') {
          // ////printf("PHONE NUMBER NAO CORRECTA\n");
          return ERROR_WRONG_PHONE_NUMBER;
        }
      }
    }

    if (MyUser_Search_User(user_validateData.phone, auxfileContents) ==
        ESP_OK) {
      return ERROR_NEW_USER_ALREADY_EXISTS;
    }

    if (strlen(user_validateData.firstName) < 2) {
      return ERROR_NAME_LENGTH;
    }

    if (get_RTC_System_Time() == 0) {
      return ERROR_NEW_USER_ADD;
    }

    sprintf(user_validateData.start.date, "%d", nowTime.date);
    sprintf(user_validateData.start.hour, "%s", "0000");
    sprintf(user_validateData.end.days, "%c", '*');
    sprintf(user_validateData.end.hour, "%s", "2359");
    sprintf(user_validateData.key, "%s", DEFAULT_USER_PASSWORD);
    sprintf(user_validateData.week, "%s", "1111111");
    user_validateData.relayPermition = '0';
    user_validateData.permition = '0';
    user_validateData.ble_security = '0';
    user_validateData.erase_User_After_Date = '0';

    memset(newUserData, 0, 200 /* sizeof(newUserData) */);
    sprintf(newUserData, "%s;%s;%s;%s;%s;%s;%s;%c;%s;%c;%c;%c",
            user_validateData.phone, user_validateData.firstName,
            user_validateData.start.date, user_validateData.start.hour,
            user_validateData.end.days, user_validateData.end.hour,
            user_validateData.key, user_validateData.permition,
            user_validateData.week, user_validateData.relayPermition,
            user_validateData.ble_security,
            user_validateData.erase_User_After_Date);

#ifdef CONFIG_WIEGAND_CODE

    // user_validateData.wiegand_code[0] = '#';
    // user_validateData.wiegand_code[1] = 0;
    strcat(newUserData, ";\0");

#endif

    // ////printf("add default number 4\n");

    uint16_t ACK = MyUser_Add(&user_validateData);

    if (ACK == ESP_OK) {
      // ////printf("\nuser add with 1 name\n");
      return 0; // file_contents_Users;
    } else if (ACK == 128) {
      return ERROR_LIMIT_USERS;
    } else if (ACK == ERROR_WIEGAND_ALREADY_EXIST) {
      return ERROR_WIEGAND_ALREADY_EXIST;
    } else {
      return ERROR_NEW_USER_ADD;
    }
  } else if (dotCounter == 0 && strlen(payload) > 1 && strlen(payload) < 18) {

    for (int i = 0; i < strlen(payload); i++) {
      if (payload[i] > '9' || payload[i] < '0') {
        if (payload[i] != '+') {
          // ////printf("PHONE NUMBER NAO CORRECTA\n");
          return ERROR_WRONG_PHONE_NUMBER;
        }
      }
    }

    if (MyUser_Search_User(payload, auxfileContents) == ESP_OK) {
      return ERROR_NEW_USER_ALREADY_EXISTS;
    }

    uint8_t add_Default_Number_ACK = add_Default_Number(payload);

    if (add_Default_Number_ACK == 1) {
      sprintf(newUserData, "%s", payload);
      return 0;
    } else if (add_Default_Number_ACK == ERROR_NEW_USER_ALREADY_EXISTS) {
      return ERROR_NEW_USER_ALREADY_EXISTS;
    } else if (add_Default_Number_ACK == ERROR_LIMIT_USERS) {
      return ERROR_LIMIT_USERS;
    } else {
      return ERROR_NEW_USER_ADD;
    }
  } else if (dotCounter > 1 && dotCounter < 8 && strlen(payload) > 4) {
    memset(&lost_SMS_addUser, 0, sizeof(lost_SMS_addUser));
    // //printf("\n\n\n lost_SMS_addUser.flag_Lost_SMS_Add_User = 1,
    // %s\n\n\n",phNumber);
    lost_SMS_addUser.flag_Lost_SMS_Add_User = 1;
    sprintf(lost_SMS_addUser.payload, "%s %s UR.S.U %s", phNumber, password,
            payload);
    sprintf(lost_SMS_addUser.phone, "%s", phNumber);
    lost_SMS_addUser.point_Numbers = (uint8_t)dotCounter;
    give_rdySem_Control_SMS_Task();
    return 128;
  } else {
    // ////printf(ERROR_INPUT_DATA);
    return ERROR_NEW_USER_INPUT_DATA;
  }

  return ERROR_NEW_USER_ADD;
}

uint8_t check_NewUser_Data(MyUser *user_validateData) {

  if (!(strlen(user_validateData->firstName) > 2)) {
    sprintf(user_validateData->firstName, "%s", "S/N");
    // memset(user_validateData->firstName,("S/N"),strlen("S/N"));
    // ////printf("NO FIRSTNAME %s\n", user_validateData->firstName);
  }

  if (strlen(user_validateData->end.days) == 0) {
    user_validateData->end.days[0] = '*';
    user_validateData->end.days[1] = 0;
  } else {
    if (atoi(user_validateData->end.days) > 90) {
      return 0;
    }

    for (int i = 0; i < strlen(user_validateData->end.days) - 1; i++) {
      if (user_validateData->end.days[i] < '0' ||
          user_validateData->end.days[i] > '9') {
        // ////printf("DAYS NAO CORRECTA\n");
        return 0;
      }
    }
  }

  if (strlen(user_validateData->key) != 6) {
    // ////printf("PASS ERRO MENOS DE 4 CHARS\n");
    return 1;
  } else {
    for (int i = 0; i < strlen(user_validateData->key); i++) {
      if (user_validateData->key[i] > '9' || user_validateData->key[i] < '0') {
        // ////printf("KEY NAO CORRECTA\n");
        return 1;
      }
    }
  }

  if (strlen(user_validateData->week) != 7) {
    // ////printf("WEEK ERRO MENOS DE 7 CHARS\n");
    return 2;
  } else {
    uint8_t zero_Week_Count = 0;
    for (int i = 0; i < strlen(user_validateData->week); i++) {

      if (user_validateData->week[i] == '0') {
        zero_Week_Count++;
      }

      if (user_validateData->week[i] > '1' ||
          user_validateData->week[i] < '0') {
        // ////printf("WEEK NAO CORRECTA\n");
        return 2;
      }
    }

    if (zero_Week_Count == 7) {
      return 2;
    }
  }

  if (!validate_Hour(user_validateData->end.hour)) {
    // ////printf("END HOUR NAO CORRECTA\n");
    return 3;
  }

  if (!validate_Hour(user_validateData->start.hour)) {
    // ////printf("START HOUR NAO CORRECTA\n");
    return 4;
  }

  // if (atoi(user_validateData->end.hour) <
  // atoi(user_validateData->start.hour))
  // {
  //     ////printf("END HOUR > START HOUR\n");
  //     return 0;
  // }

  if (!check_DateisValid(user_validateData->start.date)) {
    // ////printf("START DATE NAO CORRECTA\n");
    return 5;
  }

  char auxPhoneNumber[20] = {};

  sprintf(auxPhoneNumber, "%s", removeSpacesFromStr(user_validateData->phone));
  memset(user_validateData->phone, 0, sizeof(user_validateData->phone));

  sprintf(user_validateData->phone, "%s", auxPhoneNumber);

  /* if (strlen(user_validateData->phone) < 2)
  {
      return 6;
  } */

  for (int i = 0; i < strlen(user_validateData->phone); i++) {

    if (user_validateData->phone[i] > '9' ||
        user_validateData->phone[i] < '0') {
      if (!((user_validateData->phone[i] != '+') ^
            (user_validateData->phone[i] != '$'))) {
        // ////printf("PHONE NUMBER NAO CORRECTA\n");
        return 6;
      }
    }
  }

  // ////printf("\nrelayPermition NAO CORRECTA %c\n",
  // user_validateData->relayPermition);
  if (user_validateData->relayPermition < '0' ||
      user_validateData->relayPermition > '2') {
    // ////printf("\nrelayPermition NAO CORRECTA\n");
    return 7;
  }

  /*  sprintf(buffer, "%s;%s;%s;%s;%s;%s;%s;%c;%s;%c;",
     user_validateData->phone, user_validateData->firstName,
     user_validateData->start.date, user_validateData->start.hour,
     user_validateData->end.days, user_validateData->end.hour,
     user_validateData->key, user_validateData->permition,
     user_validateData->week, user_validateData->relayPermition); */

  // //printf("new user bff 1234 %s\n", buffer);
  return 8;
}

char *MyUser_new_MultiUSERS(char *payload, char *mrsp) {

  char *rsp;
  char *feedback_AddUsers = malloc(strlen(payload) + 200);
  memset(feedback_AddUsers, 0, strlen(payload) + 200);
  char phone_number[20] = {};
  int dotCounter = 0;
  int spaceCounter = 0;
  int secondDot_Position = 0;
  char ch[2] = {' ', '\0'};

  uint8_t ACK_AddMultiUsers = 0;

  if (strlen(payload) > 190) {
    strcpy(mrsp, return_Json_SMS_Data("ERROR_MULTIUSERS_LIMIT_USERS"));
    return ERROR_LIMIT_USERS;
  }

  memset(phone_number, 0, sizeof(phone_number));
  for (int i = 0; i < strlen(payload); i++) {
    if (payload[i] == '.') {
      dotCounter++;
    }
  }

  // ////printf(" MyUser_new_MultiUSERS payload %s\n", payload);
  for (int i = 0; i < strlen(payload); i++) {
    if (payload[i] == ' ') {
      spaceCounter++;
    }
  }

  if ((UsersCountNumbers + spaceCounter) < LIMIT_USERS_REGISTER_NUMBER) {

    // ////printf("\nUsersCountNumbers %d\n", UsersCountNumbers);
    if (dotCounter == 0) {
      int str_index = 0;
      if (spaceCounter > 0) {
        uint8_t auxSpaceCounter = 0;
        for (int i = 0; i < strlen(payload); i++) {

          if (payload[i] == ' ') {
            str_index = 0;
            // ////printf(" add multi user num %d, ph =%s\n", i, phone_number);
            if (strlen(phone_number) > 4) {

              if (add_Default_Number(phone_number)) {
                if (auxSpaceCounter == 0) {
                  strcpy(feedback_AddUsers, phone_number);
                  strcat(feedback_AddUsers, ch);
                  auxSpaceCounter++;
                } else {

                  strcat(feedback_AddUsers, phone_number);
                  strcat(feedback_AddUsers, ch);
                }
              }
            }
            memset(phone_number, 0, sizeof(phone_number));
          } else {
            phone_number[str_index] = payload[i];
            str_index++;
          }
        }
        // ////printf(" add multi user num %d, ph =%s\n", 1, phone_number);

        if (strlen(phone_number) > 4) {
          if (add_Default_Number(phone_number)) {

            // strncat(feedback_AddUsers,' ', 1);
            // ////printf("\nstrcat(feedback_AddUsers, phone_number); \n");
            strcat(feedback_AddUsers, phone_number);
            // ////printf("\nstrcat(feedback_AddUsers 11 \n");
          }
        }
      } else {
        // ////printf("MENOS 1 UTILIZADOR\n");
        free(feedback_AddUsers);
        strcpy(mrsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
        return return_ERROR_Codes(&rsp,
                                  return_Json_SMS_Data("ERROR_INPUT_DATA"));
      }
    } else if (dotCounter > 0 && dotCounter <= 7) {
      char addUser_payload[150];

      if (spaceCounter > 0) {
        char definition[50];

        for (int i = 0; i < strlen(payload); i++) {
          if (payload[i] == '.') {
            memset(definition, 0, sizeof(definition));
            for (int x = i; x < strlen(payload); x++) {
              definition[x - i] = payload[x];
            }

            break;
          }
        }

        for (int i = 0; i < strlen(addUser_payload); i++) {
          if (payload[i] == '.') {
            dotCounter++;

            if (dotCounter == 2) {
              secondDot_Position = i;
              break;
            }
          }
        }

        insert_SubString(&definition, ".S/N", secondDot_Position);
        int str_index = 0;

        memset(addUser_payload, 0, sizeof(addUser_payload));
        uint8_t auxSpaceCounter = 0;
        for (int i = 0; i < strlen(payload); i++) {

          if (payload[i] == ' ' || payload[i] == '.') {
            str_index = 0;
            // definition[strlen(definition)-1]='\0';
            memset(addUser_payload, 0, sizeof(addUser_payload));
            sprintf(addUser_payload, "%s%s", phone_number, definition);

            if (addUser_payload[strlen(addUser_payload) - 1] == ' ') {
              addUser_payload[strlen(addUser_payload) - 1] = 0;
            }

            dotCounter = 0;

            // addUser_payload[strlen(addUser_payload)]='\0';
            char auxSTR_NEWUSER[200] = {};
            // ////printf("\n\nadd multi user num %d, ph =%s///\n\n", i,
            // addUser_payload);

            if (MyUser_newUSER(addUser_payload, &auxSTR_NEWUSER, "0123",
                               "aasas") == 0) {

              if (auxSpaceCounter == 0) {
                strcpy(feedback_AddUsers, phone_number);
                auxSpaceCounter++;
              } else {
                strcat(feedback_AddUsers, ch);
                strcat(feedback_AddUsers, phone_number);
              }
            } else {
              // ////printf("\n\n MULTIUSERS - USER EXISTS\n\n");
            }

            memset(addUser_payload, 0, sizeof(addUser_payload));
            memset(phone_number, 0, sizeof(phone_number));

            if (payload[i] == '.') {
              break;
            }
          } else {
            phone_number[str_index] = payload[i];
            str_index++;
          }
        }
      } else {
        free(feedback_AddUsers);
        // ////printf("MENOS 1 UTILIZADOR\n");
        strcpy(mrsp, return_Json_SMS_Data("ERROR_NEW_USER_ADD"));
        return return_Json_SMS_Data("ERROR_NEW_USER_ADD");
      }
    } else {
      free(feedback_AddUsers);
      strcpy(mrsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
      return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
    }
  } else {
    free(feedback_AddUsers);
    strcpy(mrsp, return_Json_SMS_Data("ERROR_MULTIUSERS_LIMIT_USERS"));
    return return_Json_SMS_Data("ERROR_MULTIUSERS_LIMIT_USERS");
  }

  // ////printf("\nstrcat(feedback_AddUsers 22 \n");

  strcpy(mrsp, feedback_AddUsers);
  // ////printf("\nstrcat(feedback_AddUsers 33 \n");
  free(feedback_AddUsers);
  // ////printf("\n feedback numbers %s\n", payload);
  return payload;
}

uint8_t rf_auto_save(char rfButon, uint64_t rfSerial, uint8_t relay,
                     char *outputData) {

  ESP_LOGI("TAG",
           "rf_auto_save: rfButon=%c, rfSerial=%llx, relay=%d, outputData=%s",
           rfButon, rfSerial, relay, outputData);

  char auxSerial[30] = {};
  char payload_rfSerialExist[30] = {};
  sprintf(auxSerial, "&%llx", rfSerial);
  MyUser autoSaveRF_user;

  if (!checkIf_rfSerialExist(auxSerial, payload_rfSerialExist)) {

    ESP_LOGI("TAG", "rf_auto_save: serial not found, adding new user");

    sprintf(autoSaveRF_user.phone, "%s", "");
    sprintf(autoSaveRF_user.firstName, "%s", "S/N");
    autoSaveRF_user.erase_User_After_Date = '0';
    autoSaveRF_user.ble_security = '0';
    autoSaveRF_user.permition = '0';
    sprintf(autoSaveRF_user.key, "%s", "888888");
    sprintf(autoSaveRF_user.end.days, "%s", "*\0");
    sprintf(autoSaveRF_user.end.hour, "%s", "2359");
    sprintf(autoSaveRF_user.start.hour, "%s", "0000");
    sprintf(autoSaveRF_user.start.date, "%s", "240916");
    autoSaveRF_user.relayPermition = '0';
    autoSaveRF_user.wiegand_rele_permition = ':';
    sprintf(autoSaveRF_user.wiegand_code, "%s", ":\0");
    sprintf(autoSaveRF_user.week, "%s", "1111111");
    sprintf(autoSaveRF_user.rf_serial, "%llx", rfSerial);

    if (relay == 1) {
      autoSaveRF_user.rf1_relay = rfButon;
      autoSaveRF_user.rf2_relay = ':';
      // autoSaveRF_user.rf3_relay = ':';
    } else if (relay == 2) {
      autoSaveRF_user.rf1_relay = ':';
      autoSaveRF_user.rf2_relay = rfButon;
      // autoSaveRF_user.rf3_relay = ':';
    } /* else if (relay == 3) {
      autoSaveRF_user.rf1_relay = ':';
      autoSaveRF_user.rf2_relay = ':';
     // autoSaveRF_user.rf3_relay = rfButon;
    } */
    else {

      ESP_LOGE("TAG", "rf_auto_save: invalid relay number %d", relay);
      return ESP_FAIL;
    }

    if (MyUser_Add(&autoSaveRF_user) == ESP_OK) {
      ESP_LOGI("TAG", "rf_auto_save: user added successfully");
      sprintf(outputData, "RF S S %c.%c.%s", autoSaveRF_user.rf1_relay,
              autoSaveRF_user.rf2_relay, autoSaveRF_user.rf_serial);
      return ESP_OK;
    } else {
      ESP_LOGE("TAG", "rf_auto_save: failed to add user");
      return ESP_FAIL;
    }
  } else {
    ESP_LOGI("TAG", "rf_auto_save: serial found, updating existing user");

    char searchUser_file_cintents[200] = {};
    MyUser myUser_AutoAdd;
    memset(&myUser_AutoAdd, 0, sizeof(myUser_AutoAdd));
    MyUser_Search_User(payload_rfSerialExist, searchUser_file_cintents);
    printf("\n\nsearchUser_file_cintents: %s\n\n", searchUser_file_cintents);
    parse_ValidateData_User(searchUser_file_cintents, &myUser_AutoAdd);

    /*  if (rfButon != myUser_AutoAdd.rf1_relay &&
         rfButon != myUser_AutoAdd.rf2_relay &&
         rfButon != myUser_AutoAdd.rf3_relay) { */

    if (relay == 1) {
      myUser_AutoAdd.rf1_relay = rfButon;
    } else if (relay == 2) {
      myUser_AutoAdd.rf2_relay = rfButon;
    } /* else if (relay == 3) {
      myUser_AutoAdd.rf3_relay = rfButon;
    } */

    replaceUser(&myUser_AutoAdd);
    sprintf(outputData, "RF S S %c.%c.%s", myUser_AutoAdd.rf1_relay,
            myUser_AutoAdd.rf2_relay, myUser_AutoAdd.rf_serial);
    ESP_LOGI("TAG", "rf_auto_save: user updated successfully");
    return ESP_OK;
    //}
    ESP_LOGI("TAG", "rf_auto_save: user not updated, button already exists");
  }
  ESP_LOGE("TAG", "rf_auto_save: failed to add or update user");
  return ESP_FAIL;
}
uint8_t add_defaultUser_wiegand(uint64_t wiegandCode) {
  char *wiegandNumber;

  char auxWiegand_str[200] = {};

  asprintf(&wiegandNumber, "$%lld", wiegandCode);

  if (!checkIf_wiegandExist(wiegandNumber, &auxWiegand_str)) {
    if (get_RTC_System_Time() == 0) {
      return 0;
    }

#ifdef CONFIG_WIEGAND_CODE

    sprintf(user_auto_controlAcess.wiegand_code, "%lld", wiegandCode);
    // strcat(qwerty,user_auto_controlAcess.wiegand_code);

#endif

    // //printf("\n\nnew auto user bff 1313 %s\n\n",
    // user_auto_controlAcess.wiegand_code);

    uint16_t ACK = MyUser_Add(&user_auto_controlAcess);

    if (ACK == ESP_OK) {
      // //printf("\n\nWIEGAND GRAVADO - %s\n\n",
      // user_auto_controlAcess.wiegand_code);
      char *wiegand_MQTT_rsp;
      memset(auxWiegand_str, 0, sizeof(auxWiegand_str));
      MyUser_Search_User(wiegandNumber, &auxWiegand_str);
      char wiegandSTR[200] = {};
      erase_Password_For_Rsp(auxWiegand_str, wiegandSTR);
      asprintf(&wiegand_MQTT_rsp, "WI G W %s", wiegandSTR);
      send_UDP_Send(wiegand_MQTT_rsp, "");

      if (BLE_SMS_Indication_wiegand_autoSave == BLE_INDICATION) {
        esp_ble_gatts_send_indicate(
            gattsIF_wiegand_autoSave, connID_wiegand_autoSave,
            handle_table_wiegand_autoSave, strlen(wiegand_MQTT_rsp),
            (uint8_t *)wiegand_MQTT_rsp, false);
      }
      free(wiegand_MQTT_rsp);
      return 1;
    } else if (ACK == 128) {
      return ERROR_LIMIT_USERS;
    } else {
      return 0;
    }
    // MyUser_List_AllUsers();
  } else {
    return ERROR_NEW_USER_ALREADY_EXISTS;
  }

  return 0;
}

uint8_t add_Default_Number(char *number) {
  // ////printf("add default number 1\n");
  // ////printf("number add=%s len=%d", number, strlen(number));
  MyUser user_validateData; //= malloc(sizeof(MyUser));
  memset(&user_validateData, 0, sizeof(user_validateData));
  char td_Date[] = "220215";
  char td_hour[] = "0000";

  char auxfileContents[200];

  if (MyUser_Search_User(number, auxfileContents) != ESP_OK) {

    if (get_RTC_System_Time() == 0) {
      return 0;
    }

    if (strlen(number) < 20) {
      sprintf(user_validateData.firstName, "%s", "S/N");
      sprintf(user_validateData.phone, "%s", number);
      sprintf(user_validateData.start.date, "%d", nowTime.date);
      sprintf(user_validateData.start.hour, "%s", "0000");
      sprintf(user_validateData.end.days, "%c", '*');
      sprintf(user_validateData.end.hour, "%s", "2359");
      sprintf(user_validateData.key, "%s", DEFAULT_USER_PASSWORD);
      sprintf(user_validateData.week, "%s", "1111111");
      user_validateData.permition = '0';
      user_validateData.relayPermition = '0';
      user_validateData.ble_security = '0';
      user_validateData.erase_User_After_Date = '0';
      // ////printf("add default number 4\n");
      char qwerty[200];
      sprintf(qwerty, "%s;%s;%s;%s;%s;%s;%s;%c;%s;%c;%c;%c;",
              user_validateData.phone, user_validateData.firstName,
              user_validateData.start.date, user_validateData.start.hour,
              user_validateData.end.days, user_validateData.end.hour,
              user_validateData.key, user_validateData.permition,
              user_validateData.week, user_validateData.relayPermition,
              user_validateData.ble_security,
              user_validateData.erase_User_After_Date);

#ifdef CONFIG_WIEGAND_CODE

      user_validateData.wiegand_code[0] = ':';
      user_validateData.wiegand_code[1] = 0;
      user_validateData.wiegand_rele_permition = ':';
      user_validateData.rf_serial[0] = ':';
      user_validateData.rf1_relay = ':';
      user_validateData.rf2_relay = ':';
      // user_validateData.rf3_relay = ':';

#endif

      // ////printf("new user bff 1313 %s\n", qwerty);

      uint16_t ACK = MyUser_Add(&user_validateData);

      if (ACK == ESP_OK) {
        return 1;
      } else if (ACK == 128) {
        return ERROR_LIMIT_USERS;
      } else {
        return 0;
      }
      // MyUser_List_AllUsers();
    }
  } else {
    return ERROR_NEW_USER_ALREADY_EXISTS;
  }

  return 0;
}

uint8_t MyUser_add_rfSerial(char *rfSerial, char *payload, char permition) {
  char auxPayload[200] = {};
  sprintf(auxPayload, "%s", payload);
  // //printf("\n\nMyUser_add444_wie\n\n");
  //////printf("\n\nMyUser_add_wiegand - %s - %s - %c\n\n", rfSerial,
  /// payload, *permition);

  if (permition == '0') {
    return save_STR_Data_In_Storage(rfSerial, auxPayload,
                                    nvs_rf_codes_users_handle);
  } else if (permition == '1') {
    // //printf("\n\nsave admin card %s\n", auxPayload);
    return save_STR_Data_In_Storage(rfSerial, auxPayload,
                                    nvs_rf_codes_admin_handle);
  } else if (permition == '2') {
    return save_STR_Data_In_Storage(rfSerial, auxPayload,
                                    nvs_rf_codes_owner_handle);
  } else {
    return ESP_FAIL;
  }

  return ESP_FAIL;
}

uint8_t MyUser_add_wiegand(char *wiegandCode, char *payload, char permition) {
  char auxPayload[200] = {};
  sprintf(auxPayload, "%s", payload);
  // //printf("\n\nMyUser_add444_wie\n\n");
  //////printf("\n\nMyUser_add_wiegand - %s - %s - %c\n\n", wiegandCode,
  /// payload, *permition);

  if (permition == '0') {
    return save_STR_Data_In_Storage(wiegandCode, auxPayload,
                                    nvs_wiegand_codes_users_handle);
  } else if (permition == '1') {
    // //printf("\n\nsave admin card %s\n", auxPayload);
    return save_STR_Data_In_Storage(wiegandCode, auxPayload,
                                    nvs_wiegand_codes_admin_handle);
  } else if (permition == '2') {
    return save_STR_Data_In_Storage(wiegandCode, auxPayload,
                                    nvs_wiegand_codes_owner_handle);
  } else {
    return ESP_FAIL;
  }

  return ESP_FAIL;
}

uint16_t MyUser_Add(MyUser *user) {
  char aux_phNumber[30] = {};
  char aux_search_USER[30] = {};
  // sprintf(aux_phone,"%s",check_IF_haveCountryCode(user->phone));
  // sprintf(phone, "%s", check_IF_haveCountryCode(user->phone));
  UsersCountNumbers = get_User_Counter_From_Storage();
  char auxRF_number[40] = {};
  uint32_t limit_users = 0;

  if (nvs_get_u32(nvs_System_handle, NVS_LIMIT_USERS, &limit_users) != ESP_OK) {
    nvs_set_u32(nvs_System_handle, NVS_LIMIT_USERS,
                LIMIT_USERS_REGISTER_NUMBER);
  }
  printf("\n\nrf auto save 2\n\n");
  // //printf("\n\nphone= fffff 11\n\n");
  if (UsersCountNumbers < limit_users) {
    /* code */

    memset(buffer, 0, 200);
    // //printf("user->phone = %s\n", user->phone);
    //  ////printf(" phone add user buffer = %s\n", user->phone);
    memset(aux_phNumber, 0, sizeof(aux_phNumber));
    sprintf(aux_phNumber, "%s", check_IF_haveCountryCode(user->phone, 1));
    sprintf(aux_search_USER, "%s", user->phone);
    // ////printf(" phone add user buffer = %s\n", aux_phNumber);
    // //printf("\nauxphone add = %s\n", user->phone);

    for (int i = 0; i < strlen(user->phone); i++) {
      if ((user->phone[i] < '0' || user->phone[i] > '9')) {
        if (user->phone[i] != '+' && user->phone[i] != '$' &&
            user->phone[i] != '&') {
          // //printf("phone = ERROR 11\n");
          return ESP_FAIL;
        }
      }
    }

    /*  if (user->permition != '2')
     {

     } */

    user->ble_security = '0';

    /* if (user->phone[0] == '$' && user->wiegand_code[0] != '$')
    {
        strcat(aux_phNumber, user->wiegand_code);
    } */
    printf("\n\nrf auto save 3\n\n");
    // //printf("\n\nphone= fffff 11 - %s\n\n", aux_search_USER);
    sprintf(buffer, "%s;%s;%s;%s;%s;%s;%s;%c;%s;%c;%c;%c;%c;%s;%s;%c;%c;",
            aux_search_USER, user->firstName, user->start.date,
            user->start.hour, user->end.days, user->end.hour, user->key,
            user->permition, user->week, user->relayPermition,
            user->ble_security, user->erase_User_After_Date,
            user->wiegand_rele_permition, user->wiegand_code, user->rf_serial,
            user->rf1_relay, user->rf2_relay);
    printf("\n\n WIGAND buffer 44 %s\n\n", buffer);

    if (strlen(aux_phNumber) == 0 && (user->rf_serial[0] == ':') &&
        user->wiegand_code[0] != ':') {
      sprintf(aux_phNumber, "$%s", user->wiegand_code);
    } else if ((strlen(aux_phNumber) == 0) && user->wiegand_code[0] == ':' &&
               user->rf_serial[0] != ':') {
      sprintf(aux_phNumber, "&%s", user->rf_serial);
    } else if ((strlen(aux_phNumber) == 0) && user->wiegand_code[0] != ':' &&
               user->rf_serial[0] != ':') {
      sprintf(aux_phNumber, "$%s", user->wiegand_code);
    }

    char aaauuxx[200] = {};
    char auxWiegand_number[40] = {};

    if (user->wiegand_code[0] == ':') {
      sprintf(auxWiegand_number, "%s", aux_phNumber);

    } else {
      sprintf(auxWiegand_number, "$%s", user->wiegand_code);
    }

    if (user->wiegand_code[0] != ':') {
      printf("\n\nphone= 4444411 - %s -> %s\n\n", auxWiegand_number,
             user->wiegand_code);
      int UACK = checkIf_wiegandExist(
          auxWiegand_number,
          &aaauuxx); /*!get_STR_Data_In_Storage(user->wiegand_code,
                        nvs_wiegand_codes_handle, &aaauuxx);*/

      printf("\n\nphone= 4444411hhhh - %d\n\n", UACK);
      if (!UACK) {

        // //printf("\n\n WIGAND buffers - %c\n\n", user->permition);
        if (MyUser_add_wiegand(auxWiegand_number, aux_phNumber,
                               user->permition) != ESP_OK) {
          printf("\n\n WIGAND EXIST 798\n\n");
          return ESP_FAIL;
        }
      } else {
        printf("\n\n WIGAND EXIST222\n\n");
        return ERROR_WIEGAND_ALREADY_EXIST;
      }

      printf("\n\nphone= 77777\n\n");
    }

    if (strlen(user->rf_serial) > 0) {
      if (user->rf_serial[0] == ':') {
        sprintf(auxRF_number, "%s", aux_phNumber);
      } else {
        sprintf(auxRF_number, "&%s", user->rf_serial);
      }

      if (user->rf_serial[0] != ':') {

        int UACK = checkIf_rfSerialExist(auxRF_number, aaauuxx);

        if (!UACK) {
          printf("\n\n WIGAND buffers - %c\n\n", user->permition);
          if (MyUser_add_rfSerial(auxRF_number, aux_phNumber,
                                  user->permition) != ESP_OK) {
            erase_only_wiegand(auxWiegand_number, '0');
            printf("\n\n RF EXIST 798\n\n");
            return ESP_FAIL;
          }
        } else {
          erase_only_wiegand(auxWiegand_number, '0');
          // //printf("\n\n WIGAND EXIST222\n\n");
          return ERROR_RF_ALREADY_EXIST;
        }
      }
    }
    printf("add user buffer = %s\n%s\n\n", buffer, aux_phNumber);

    if (user->permition == '0') {
      // //printf("\n ADD USERS NAMESPACE\n");

      if (save_STR_Data_In_Storage(aux_phNumber, &buffer, nvs_Users_handle) ==
          ESP_OK) {
        // ////printf("\n ADD USERS NAMESPACE1\n");
        UsersCountNumbers++;
        nvs_get_u32(nvs_System_handle, NVS_KEY_GUEST_COUNTER,
                    &GuestCountNumbers);
        GuestCountNumbers++;
        nvs_set_u32(nvs_System_handle, NVS_KEY_GUEST_COUNTER,
                    GuestCountNumbers);
        save_User_Counter_In_Storage(UsersCountNumbers);
        return ESP_OK;
      } else {
        erase_only_wiegand(auxWiegand_number, '0');
        erase_onlyRF(auxRF_number, user->permition);
        return ESP_FAIL;
      }
    } else if (user->permition == '1' && user->phone[0] != '#') {
      // ////printf("\n ADD ADMIN NAMESPACE\n");
      if (save_STR_Data_In_Storage(aux_phNumber, &buffer, nvs_Admin_handle) ==
          ESP_OK) {
        // ////printf("\n ADD ADMIN NAMESPACE1\n");
        UsersCountNumbers++;
        save_User_Counter_In_Storage(UsersCountNumbers);
        return ESP_OK;
      } else {
        erase_only_wiegand(auxWiegand_number, '0');
        erase_onlyRF(auxRF_number, user->permition);
        return ESP_FAIL;
      }
    } else if (user->permition == '2' && user->phone[0] != '#') {
      // ////printf("\n ADD OWNER NAMESPACE %s\n", aux_phNumber);
      if (save_STR_Data_In_Storage(aux_phNumber, &buffer, nvs_Owner_handle) ==
          ESP_OK) {
        // ////printf("\n ADD OWNER NAMESPACE1 %s\n", aux_phNumber);
        UsersCountNumbers++;
        save_User_Counter_In_Storage(UsersCountNumbers);
        return ESP_OK;
      } else {
        erase_only_wiegand(auxWiegand_number, '0');
        erase_onlyRF(auxRF_number, user->permition);
        return ESP_FAIL;
      }
    } else {
      erase_only_wiegand(auxWiegand_number, '0');
      erase_onlyRF(auxRF_number, user->permition);
      return ESP_FAIL;
    }
  } else {
    // ////printf("%s", ERROR_LIMIT_USERS);
    return 128;
  }

  return ESP_FAIL;
}

uint8_t add_onlyNewWiegand_number(char *payload) {
  uint8_t strIndex = 0;

  return 0;
}

uint32_t MyUser_List_AllUsers() {
  int count = 0;
  int guestCount = 0;
  // ////printf("\nLIST USERS:\n");
  nvs_iterator_t it;
  it = nvs_entry_find("keys", NVS_USERS_NAMESPACE, NVS_TYPE_STR);
  char value[200];
  // ////printf("Iterate NVS\n");
  //  int64_t start_time = esp_timer_get_time();
  while (it != NULL) {
    count++;
    guestCount++;
    nvs_entry_info_t info;
    nvs_entry_info(it, &info);
    it = nvs_entry_next(it);
    // printftf("key '%s', type '%d' \n", info.key, info.type);
    get_Data_Users_From_Storage(info.key, &value);
    // ////printf("\nvalue: %s\n", value);
    vTaskDelay(pdMS_TO_TICKS(20));
  }

  nvs_set_u32(nvs_System_handle, NVS_KEY_GUEST_COUNTER, guestCount);
  // ////printf("\nLIST ADMIN:\n");
  it = nvs_entry_find("keys", NVS_ADMIN_NAMESPACE, NVS_TYPE_STR);

  // ////printf("Iterate NVS\n");

  while (it != NULL) {
    count++;
    nvs_entry_info_t info;
    nvs_entry_info(it, &info);
    it = nvs_entry_next(it);
    // ////printf("key '%s', type '%d' \n", info.key, info.type);
    get_Data_Users_From_Storage(info.key, &value);
    // ////printf("\nvalue: %s\n", value);
    vTaskDelay(pdMS_TO_TICKS(20));
  }

  // ////printf("\nLIST OWNER:\n");
  it = nvs_entry_find("keys", NVS_OWNER_NAMESPACE, NVS_TYPE_STR);

  // ////printf("Iterate NVS\n");

  while (it != NULL) {
    count++;
    nvs_entry_info_t info;
    nvs_entry_info(it, &info);
    it = nvs_entry_next(it);
    // ////printf("key '%s', type '%d' \n", info.key, info.type);
    get_Data_Users_From_Storage(info.key, &value);
    // ////printf("\nvalue: %s\n", value);
    vTaskDelay(pdMS_TO_TICKS(20));
  }
  UsersCountNumbers = count;

  free(it);
  // int64_t time = esp_timer_get_time() - start_time;
  // ////printf("Iterate NVS END\n");
  // ////printf("Time: %lld", time);
  // ////printf("\ncount numbers: %d\n", count);
  return UsersCountNumbers;
}

uint8_t Myuser_deleteUser(MyUser *user) {

  char aux_phNumber[25] = {};
  char auxWiegand_code[30] = {};
  char auxRF_serial[30] = {};
  memset(aux_phNumber, 0, sizeof(aux_phNumber));

  if (strlen(user->phone) == 0 && (user->rf_serial[0] == ':') &&
      user->wiegand_code[0] != ':') {
    sprintf(aux_phNumber, "$%s", user->wiegand_code);
  } else if ((strlen(user->phone) == 0) && user->wiegand_code[0] == ':' &&
             user->rf_serial[0] != ':') {
    sprintf(aux_phNumber, "&%s", user->rf_serial);
  } else if ((strlen(user->phone) == 0) && user->wiegand_code[0] != ':' &&
             user->rf_serial[0] != ':') {
    sprintf(aux_phNumber, "$%s", user->wiegand_code);
  } else {

    sprintf(aux_phNumber, "%s", check_IF_haveCountryCode(user->phone, 0));
  }

  ESP_LOGD("TAG", "Deleting user with phone number: %s", aux_phNumber);

  /* if (strlen(user->phone) < 1 && user->wiegand_code[0] != ':') {

    sprintf(aux_phNumber, "$%s", user->wiegand_code);
    user->phone = user->wiegand_code;
    // sprintf(user->phone, "$%s", user->wiegand_code);
  } else {

    sprintf(aux_phNumber, "%s", check_IF_haveCountryCode(user->phone, 0));
  } */

  sprintf(auxWiegand_code, "$%s", user->wiegand_code);
  sprintf(auxRF_serial, "&%s", user->rf_serial);
  ESP_LOGD("TAG", "Deleting user with Wiegand code: %s", auxWiegand_code);
  ESP_LOGD("TAG", "Deleting user with RF code: %s", auxRF_serial);

  if (get_antipassback_user(user->wiegand_code) == ESP_OK) {
    erase_antipassback_user(user->wiegand_code);

    nvs_get_u32(nvs_System_handle, NVS_ANTIPASSBACK_PEOPLE_NUMBER,
                &anti_passback_people_number);
    nvs_get_u32(nvs_System_handle, NVS_ANTIPASSBACK_PEOPLE_COUNTER,
                &antipassback_peopleCounter);

    if (anti_passback_people_number == 0) {
      return;
    } else {
      anti_passback_people_number--;
      nvs_set_u32(nvs_System_handle, NVS_ANTIPASSBACK_PEOPLE_NUMBER,
                  anti_passback_people_number);
    }
  }

  if (user->permition == '0') {

    if (nvs_erase_key(nvs_Users_handle, aux_phNumber) == ESP_OK) {

      ESP_LOGD("TAG", "Deleted user with phone number: %s", aux_phNumber);

      nvs_erase_key(nvs_wiegand_codes_users_handle, auxWiegand_code);
      nvs_erase_key(nvs_rf_codes_users_handle, auxRF_serial);

      UsersCountNumbers--;
      nvs_get_u32(nvs_System_handle, NVS_KEY_GUEST_COUNTER, &GuestCountNumbers);
      GuestCountNumbers--;
      nvs_set_u32(nvs_System_handle, NVS_KEY_GUEST_COUNTER, GuestCountNumbers);
      save_User_Counter_In_Storage(UsersCountNumbers);
      return ESP_OK;
    } else {
      ESP_LOGE("TAG", "Failed to delete user with phone number: %s",
               aux_phNumber);
      return ESP_FAIL;
    }
  } else if (user->permition == '1') {
    if (nvs_erase_key(nvs_Admin_handle, aux_phNumber) == ESP_OK) {

      ESP_LOGD("TAG", "Deleted user with phone number: %s", aux_phNumber);

      // sprintf(auxWiegand_code, "$%s", user->wiegand_code);
      nvs_erase_key(nvs_wiegand_codes_admin_handle, auxWiegand_code);
      nvs_erase_key(nvs_rf_codes_admin_handle, auxRF_serial);

      UsersCountNumbers--;
      save_User_Counter_In_Storage(UsersCountNumbers);
      return ESP_OK;
    } else {
      ESP_LOGE("TAG", "Failed to delete user with phone number: %s",
               aux_phNumber);
      return ESP_FAIL;
    }
  } else if (user->permition == '2') {
    if (nvs_erase_key(nvs_Owner_handle, aux_phNumber) == ESP_OK) {

      ESP_LOGD("TAG", "Deleted user with phone number: %s", aux_phNumber);

      nvs_erase_key(nvs_rf_codes_owner_handle, auxRF_serial);
      // sprintf(auxWiegand_code, "$%s", user->wiegand_code);
      nvs_erase_key(nvs_wiegand_codes_owner_handle, auxWiegand_code);

      UsersCountNumbers--;
      save_User_Counter_In_Storage(UsersCountNumbers);
      return ESP_OK;
    } else {
      ESP_LOGE("TAG", "Failed to delete user with phone number: %s",
               aux_phNumber);
      return ESP_FAIL;
    }
  } else {
    ESP_LOGE("TAG", "Invalid user permission: %c", user->permition);
    return ESP_FAIL;
  }

  return ESP_FAIL;
}

char *MyUser_getFromName(char *payload, uint8_t gattsIF, uint16_t connID,
                         uint16_t handle_table) {

  allUsers_parameters_Message message = {
      .gattsIF = gattsIF,
      .connID = connID,
      .handle_table = handle_table,
  };
  sprintf(message.payload, "%s", payload);

  // ////printf("queue created\n");
  // ////printf("MyUser_ReadAllUsers CONN ID %d\n", message.connID);
  // ////printf("MyUser_ReadAllUsers GATTSIF %d\n", message.gattsIF);
  // ////printf("MyUser_ReadAllUsers %d\n", message.handle_table);
  // ////printf("MyUser_ReadAllUsers %s\n", message.payload);
  //  readAllUser_Label = 0;
  readAllUser_ConnID = 0;

  if (readAllUser_Label == 0) {
    readAllUser_Label = 1;
    readAllUser_ConnID = message.connID;
    xTaskCreate(ReadALLUsersFormName_task, "ReadALLUsersFormName_task", 4096,
                (void *)&message, NULL, NULL);
    vTaskDelay(pdMS_TO_TICKS((100)));
  } else {
    return "READ ALL USERS NOT POSSIBLE";
  }

  return "USER ALL OK";
}

void ReadALLUsersFormName_task(void *pvParameter) {

  allUsers_parameters_Message *message =
      (allUsers_parameters_Message *)pvParameter;

  allUsers_parameters_Message cpy_message = {
      .connID = message->connID,
      .gattsIF = message->gattsIF,
      .handle_table = message->handle_table,
  };

  sprintf(cpy_message.payload, "%s", message->payload);
  // ////printf("cpy_message.payload %s\n", cpy_message.payload);

  char fullName[50];
  char firstName[20];
  char surname[20];
  char payload[200];
  // ////printf("cpy_message.payload %s\n", cpy_message.payload);

  int line = 0;
  uint8_t dotCounter = 0;

  memset(fullName, 0, sizeof(fullName));
  memset(firstName, 0, sizeof(firstName));
  memset(surname, 0, sizeof(surname));
  // ////printf("cpy_message.payload %s\n", cpy_message.payload);

  for (int i = 0; i < strlen(cpy_message.payload); i++) {
    if (cpy_message.payload[i] == '.') {
      dotCounter++;
    }
  }

  uint8_t aux = 0;
  uint8_t strIndex = 0;

  if (dotCounter == 0 && strlen(cpy_message.payload) < 50) {
    for (int i = 0; i < strlen(cpy_message.payload); i++) {
      fullName[i] = cpy_message.payload[i];
    }
  } else if (dotCounter == 1 && strlen(cpy_message.payload) < 50) {
    for (int i = 0; i < strlen(cpy_message.payload); i++) {
      if (cpy_message.payload[i] == '.') {
        aux++;
        strIndex = 0;
      } else if (aux == 0) {
        if (strIndex < 20) {
          firstName[strIndex] = cpy_message.payload[i];
          strIndex++;
        }
      } else if (aux == 1) {

        surname[strIndex] = cpy_message.payload[i];
        strIndex++;
      } else {
        esp_ble_gatts_send_indicate(
            cpy_message.gattsIF, cpy_message.connID, cpy_message.handle_table,
            strlen(ERROR_INPUT_DATA), (uint8_t *)ERROR_INPUT_DATA, true);
        xSemaphoreTake(rdySem, portMAX_DELAY); // Wait until slave is ready
        return;
      }
    }
    sprintf(fullName, "%s;%s", firstName, surname);
  } else {
    esp_ble_gatts_send_indicate(
        cpy_message.gattsIF, cpy_message.connID, cpy_message.handle_table,
        strlen(ERROR_INPUT_DATA), (uint8_t *)ERROR_INPUT_DATA, true);
    xSemaphoreTake(rdySem, portMAX_DELAY); // Wait until slave is ready
    return;
  }

  // ////printf("fullname %s\n", fullName);
  int aux_fileContents_Count = 0;
  // ESP_LOGI("TAG", "Reading file");
  FILE *f = fopen("/spiffs/hello.txt", "r");
  fseek(f, 0, SEEK_SET);

  while (fgets(file_contents_Users, 200, f) != NULL) {
    // ////printf("line: %d\n", line);
    if ((stristr(file_contents_Users, fullName)) != NULL) {

      // ////printf("A match found on line: %d\n", line);
      aux_fileContents_Count++;
      esp_ble_gatts_send_indicate(
          cpy_message.gattsIF, cpy_message.connID, cpy_message.handle_table,
          strlen(file_contents_Users), (uint8_t *)file_contents_Users, true);
      // ////printf("\n%s\n", file_contents_Users);
      memset(file_contents_Users, 0, 200);
      // vTaskDelay(pdMS_TO_TICKS((30)));
      xSemaphoreTake(rdySem, portMAX_DELAY); // Wait until slave is ready
    }
    line = line + 1;
  }

  if (aux_fileContents_Count == 0) {
    esp_ble_gatts_send_indicate(cpy_message.gattsIF, cpy_message.connID,
                                cpy_message.handle_table, strlen("NO FILES"),
                                (uint8_t *)"NO FILES", true);
    xSemaphoreTake(rdySem, portMAX_DELAY); // Wait until slave is ready
  }

  readAllUser_Label = 0;
  fclose(f);
  vTaskDelete(NULL);
}

static inline char *stristr(const char *haystack, const char *needle) {
  char *pptr = (char *)needle;    /* Pattern to search for    */
  char *start = (char *)haystack; /* Start with a bowl of hay */
  char *sptr;                     /* Substring pointer        */
  int slen = strlen(haystack);    /* Total size of haystack   */
  int plen = strlen(needle);      /* Length of our needle     */

  /* while string length not shorter than pattern length */
  for (; slen >= plen; start++, slen--) {
    /* find start of pattern in string */
    while (toupper(*start) != toupper(*needle)) {
      start++;
      slen--;
      /* if pattern longer than string */
      if (slen < plen) {
        return NULL;
      }
    }

    sptr = start;
    pptr = (char *)needle;
    while (toupper(*sptr) == toupper(*pptr)) {
      sptr++;
      pptr++;
      /* if end of pattern then pattern was found */
      if ('\0' == *pptr) {
        return start;
      }
    }
  }

  return NULL;
}

uint8_t MyUser_Search_User(char *phoneNumber, char *file_contents_Users) {

  char ch;
  esp_err_t ACK = 0;
  int find_result = 0;
  uint8_t auxCount = 0;
  char auxPhone[50];
  char auxfileContents[200];
  int file_contents_lengh = 0;
  // //printf("\n search user 1\n");
  sprintf(auxPhone, "%s", check_IF_haveCountryCode(phoneNumber, 0));
  // ////printf("\n search user 2 - %s\n",aux);
  printf("\n aux PhoneNumber %s\n", auxPhone);
  ACK = get_Data_Users_From_Storage(auxPhone, &auxfileContents);
  // //printf("\n MyUser_Search_User %s\n", auxfileContents);
  sprintf(file_contents_Users, "%s", auxfileContents);

  return ACK;
}

uint8_t MyUser_Search_User_AUX_Call(char *phoneNumber,
                                    char *file_contents_Users) {

  char ch;
  esp_err_t ACK = 0;
  int find_result = 0;
  uint8_t auxCount = 0;
  char auxPhone[50];
  char auxfileContents[200];
  int file_contents_lengh = 0;

  if (phoneNumber[0] == '+') {
    return ESP_FAIL;
  }

  // ////printf("\n search user 1\n");
  sprintf(auxPhone, "%s", check_IF_haveCountryCode_AUX_Call(phoneNumber));
  // ////printf("\n search user 2\n");
  // ////printf("\n aux PhoneNumber %s\n", auxPhone);
  ACK = get_Data_Users_From_Storage(auxPhone, &auxfileContents);
  // ////printf("\n MyUser_Search_User %s\n", auxfileContents);
  sprintf(file_contents_Users, "%s", auxfileContents);

  return ACK;
}

char *MyUser_add_Owner(char *payload, char *SMS_phoneNumber,
                       uint8_t BLE_SMS_Indication) {
  MyUser user_validateData; //= malloc(sizeof(MyUser));
  memset(&user_validateData, 0, sizeof(user_validateData));
  int dotCounter = 0;
  uint8_t strIndex = 0;

  char aux_payload[200] = {};

  // ////printf("\n\n\nMYNEWUSER FUNCTION\n\n\n");

  for (int i = 0; i < strlen(payload); i++) {
    if (payload[i] == '.') {
      dotCounter++;
    }
  }

  if (dotCounter != 4) {
    return return_Json_SMS_Data("ERROR_INPUT_DATA");
  }

  if (payload[strlen(payload) - 1] == '?') {
    payload[strlen(payload) - 1] = 0;
  }

  uint8_t owner_Label =
      get_INT8_Data_From_Storage(NVS_KEY_OWNER_LABEL, nvs_System_handle);

  // ////printf("\n\n owner label %d\n\n", owner_Label);

  if (owner_Label != 1) {
    dotCounter = 0;
    strIndex = 0;

    for (size_t i = 0; i < strlen(payload); i++) {
      if (/* payload[i] == '.' ||  */ payload[i] == ' ') {
        if (dotCounter == 0) {
          strIndex = 0;
        } else {
          aux_payload[strIndex++] = payload[i];
        }

        dotCounter++;
      } else {
        if (dotCounter >= 1) {
          aux_payload[strIndex++] = payload[i];
        }
      }
    }

    // ////printf("\n\naux_payload - %s", aux_payload);
    dotCounter = 0;
    strIndex = 0;

    for (size_t i = 0; i < strlen(aux_payload); i++) {
      if (aux_payload[i] == '.') {
        dotCounter++;
        strIndex = 0;
      } else {
        if (dotCounter == 0) {
          if (strIndex == 20) {
            return return_Json_SMS_Data("ERROR_ADD_OWNER");
          }

          user_validateData.phone[strIndex++] = aux_payload[i];
        } else if (dotCounter == 1) {
          if (strIndex == 20) {
            return return_Json_SMS_Data("ERROR_ADD_OWNER");
          }

          user_validateData.firstName[strIndex++] = aux_payload[i];
        } else if (dotCounter == 2) {
          if (strIndex == 6) {
            return return_Json_SMS_Data("ERROR_ADD_OWNER");
          }

          user_validateData.key[strIndex++] = aux_payload[i];
        } else {
          break;
        }
      }
    }

    if (strlen(user_validateData.firstName) < 1) {
      sprintf(user_validateData.firstName, "%s", "S/N");
    }

    if (strlen(user_validateData.key) != 6) {
      return return_Json_SMS_Data("ERROR_ADD_OWNER");
    }

    for (size_t i = 0; i < strlen(user_validateData.key); i++) {
      if (user_validateData.key[i] > '9' || user_validateData.key[i] < '0') {
        return return_Json_SMS_Data("ERROR_ADD_OWNER");
      }
    }

    sprintf(user_validateData.start.date, "%s", "220101");
    sprintf(user_validateData.start.hour, "%s", "0000");
    sprintf(user_validateData.end.days, "%c", '*');
    sprintf(user_validateData.end.hour, "%s", "2359");
    sprintf(user_validateData.week, "%s", "1111111");
    user_validateData.permition = '2';
    user_validateData.relayPermition = '0';
    user_validateData.ble_security = '0';
    user_validateData.erase_User_After_Date = '0';
    user_validateData.wiegand_rele_permition = ':';
    user_validateData.wiegand_code[0] = ':';
    user_validateData.rf_serial[0] = ':';
    user_validateData.rf1_relay = ':';
    user_validateData.rf2_relay = ':';

    char aux_PhoneNumber[20] = {};

    char auxPhoneNumber[20] = {};

    sprintf(auxPhoneNumber, "%s", removeSpacesFromStr(user_validateData.phone));
    memset(user_validateData.phone, 0, sizeof(user_validateData.phone));

    sprintf(user_validateData.phone, "%s", auxPhoneNumber);

    // ////printf("\n\nuser_validateData.phone before - %s",
    // user_validateData.phone);
    // ////printf("\n\nuser_validateData.firstName before - %s",
    // user_validateData.firstName);
    // ////printf("\n\n user_validateData.key before - %s",
    // user_validateData.key);
    sprintf(aux_PhoneNumber, "%s",
            check_IF_haveCountryCode(user_validateData.phone, 1));
    // ////printf("\n\aux_PhoneNumber before - %s", aux_PhoneNumber);

    if (BLE_SMS_Indication == SMS_INDICATION) {

      if ((SMS_phoneNumber[0] == '0') && SMS_phoneNumber[1] != '0') {

        // ////printf("\n  payload phnumber sms 33 %s \n", SMS_phoneNumber);
        for (size_t i = 1; i < strlen(SMS_phoneNumber); i++) {
          SMS_phoneNumber[i - 1] = SMS_phoneNumber[i];
        }

        SMS_phoneNumber[strlen(SMS_phoneNumber) - 1] = 0;
      }

      if ((aux_PhoneNumber[0] == '0') && aux_PhoneNumber[1] != '0') {

        // ////printf("\n  payload phnumber sms 33 %s \n", aux_PhoneNumber);
        for (size_t i = 1; i < strlen(aux_PhoneNumber); i++) {
          aux_PhoneNumber[i - 1] = aux_PhoneNumber[i];
        }

        aux_PhoneNumber[strlen(aux_PhoneNumber) - 1] = 0;
      }

      if (strcmp(aux_PhoneNumber, SMS_phoneNumber)) {
        return return_Json_SMS_Data("ERROR_ADD_OWNER");
      }
    }

    // ////printf("\n\n\n owner aux_PhoneNumber - %s\n\n", aux_PhoneNumber);

    if (MyUser_Add(&user_validateData) == ESP_OK) {
      char new_Owner_Data[200];

      memset(new_Owner_Data, 0, 200 /* sizeof(newUserData) */);
      sprintf(new_Owner_Data,
              "%s;%s;%s;%s;%s;%s;%s;%c;%s;%c;%c;%c;%c;%s;%s;%c;%c;",
              user_validateData.phone, user_validateData.firstName,
              user_validateData.start.date, user_validateData.start.hour,
              user_validateData.end.days, user_validateData.end.hour,
              user_validateData.key, user_validateData.permition,
              user_validateData.week, user_validateData.relayPermition,
              user_validateData.ble_security,
              user_validateData.erase_User_After_Date,
              user_validateData.wiegand_rele_permition,
              user_validateData.wiegand_code, user_validateData.rf_serial,
              user_validateData.rf1_relay, user_validateData.rf2_relay);

      // ////printf("\n\nnew owner data - %s\n\n", new_Owner_Data);

      if (save_STR_Data_In_Storage(NVS_KEY_OWNER_INFORMATION, new_Owner_Data,
                                   nvs_System_handle) != ESP_OK) {
        Myuser_deleteUser(&user_validateData);
        return return_Json_SMS_Data("ERROR_ADD_OWNER");
      }

      if (save_INT8_Data_In_Storage(NVS_KEY_OWNER_LABEL, 1,
                                    nvs_System_handle) != ESP_OK) {
        Myuser_deleteUser(&user_validateData);
        nvs_erase_key(nvs_System_handle, NVS_KEY_OWNER_INFORMATION);
        return return_Json_SMS_Data("ERROR_ADD_OWNER");
      }

      return return_Json_SMS_Data("OWNER_ADD");
    } else {
      return return_Json_SMS_Data("ERROR_ADD_OWNER");
    }
  } else {
    return return_Json_SMS_Data("ERROR_OWNER_ALREADY_EXIST");
  }
}

void parse_ValidateData_User(char *file_contents_Users,
                             MyUser *user_validateData) {

  uint8_t aux = 0;
  uint8_t auxIndex = 0;

  for (int i = 0; i < strlen(file_contents_Users); i++) {
    if (file_contents_Users[i] == ';') {
      aux++;
      auxIndex = 0;
    } else {
      if (aux == 0) {
        user_validateData->phone[auxIndex] = file_contents_Users[i];
        auxIndex++;
      }
      // ////printf(" \n\nfrfrfrfr11\n\n");
      if (aux == 1) {
        user_validateData->firstName[auxIndex] = file_contents_Users[i];
        auxIndex++;
      }
      // ////printf(" \n\nfrfrfrfr22\n\n");
      if (aux == 2) {
        user_validateData->start.date[auxIndex] = file_contents_Users[i];
        auxIndex++;
      }
      // ////printf(" \n\nfrfrfrfr33\n\n");
      if (aux == 3) {
        user_validateData->start.hour[auxIndex] = file_contents_Users[i];
        auxIndex++;
      }
      // ////printf(" \n\nfrfrfrfr44\n\n");
      if (aux == 4) {
        user_validateData->end.days[auxIndex] = file_contents_Users[i];
        auxIndex++;
      }
      // ////printf(" \n\nfrfrfrfr55\n\n");
      if (aux == 5) {
        user_validateData->end.hour[auxIndex] = file_contents_Users[i];
        auxIndex++;
      }
      // ////printf(" \n\nfrfrfrfr66\n\n");
      if (aux == 6) {
        user_validateData->key[auxIndex] = file_contents_Users[i];
        auxIndex++;
      }
      // ////printf(" \n\nfrfrfrfr77\n\n");
      if (aux == 7) {
        user_validateData->permition = file_contents_Users[i];
        auxIndex++;
      }
      // ////printf(" \n\nfrfrfrfr88\n\n");
      if (aux == 8) {
        if (file_contents_Users[i] == '0' || file_contents_Users[i] == '1') {
          user_validateData->week[auxIndex] = file_contents_Users[i];
          auxIndex++;
        }
      }

      // ////printf(" \n\nfrfrfrfr99\n\n");
      if (aux == 9) {
        if (file_contents_Users[i] >= '0' || file_contents_Users[i] <= '2') {
          user_validateData->relayPermition = file_contents_Users[i];
          auxIndex++;
        }
      }

      // ////printf(" \n\nfrfrfrfr100\n\n");
      if (aux == 10) {
        if (file_contents_Users[i] >= '0' || file_contents_Users[i] <= '1') {
          user_validateData->ble_security = file_contents_Users[i];
        }
      }
      //  ////printf(" \n\nfrfrfrfr101\n\n");
      if (aux == 11) {
        if (file_contents_Users[i] >= '0' || file_contents_Users[i] <= '1') {
          user_validateData->erase_User_After_Date = file_contents_Users[i];
        }
      }

      if (aux == 12) {
        if (file_contents_Users[i] >= '1' || file_contents_Users[i] <= '2') {
          user_validateData->wiegand_rele_permition = file_contents_Users[i];
        }
      }
      // ////printf(" \n\nfrfrfrfr103\n\n");
      if (aux == 13) {

        user_validateData->wiegand_code[auxIndex] = file_contents_Users[i];
        auxIndex++;
      }

      if (aux == 14) {
        user_validateData->rf_serial[auxIndex] = file_contents_Users[i];
        auxIndex++;
      }

      if (aux == 15) {
        user_validateData->rf1_relay = file_contents_Users[i];
        auxIndex++;
      }

      if (aux == 16) {
        user_validateData->rf2_relay = file_contents_Users[i];
        auxIndex++;
      }

      /*  if (aux == 17) {
         user_validateData->rf3_relay = file_contents_Users[i];
         auxIndex++;
       } */

      // ////printf(" \n\nfrfrfrfr104\n\n");
    }
  }
  // ////printf(" \n\nfrfrfrfr105\n\n");
  // printf("\n\nuser_validateData %s\n\n", user_validateData->week);
}

uint8_t verify_WeekAcess(MyUser *user_validateData) {

  uint8_t access = 0;
  uint8_t weekUser = 0;
  uint8_t tdweek = 0;

  tdweek = pow(2, calculate_weekDay(nowTime.year, nowTime.month, nowTime.day));

  // printf("tdweek = %d", tdweek);

  for (int i = 0; i < 7; i++) {
    if (user_validateData->week[i] == '1') {
      weekUser = weekUser + pow(2, i);
    }
  }

  // ////printf("\nuser week1111 %s\n", user_validateData->week);
  // ////printf("\nuser week %d\n", weekUser);
  access = (tdweek & weekUser);

  if (access == tdweek) {
    // printf("acess %d\n", access);
    return 1;
  } else {
    // printf("not acess %d\n", access);
    return 0;
  }

  return 0;
}

uint8_t verify_TimeAcess(MyUser *user_validateData) {
  int start_Hour = atoi(user_validateData->start.hour);
  int end_Hour = atoi(user_validateData->end.hour);

  if (get_RTC_System_Time() == 0) {
    printf("\n\n error get rtc verify time access \n\n");
    return 0;
  }

  if (verify_WeekAcess(user_validateData)) {

    if (user_validateData->end.days[0] != '*') {
      int v_date = parseDatetoInt(atoi(user_validateData->start.date),
                                  atoi(user_validateData->end.days));
      printf("v_date %d\n", v_date);

      if ((nowTime.date >= atoi(user_validateData->start.date)) &&
          (nowTime.date <= v_date)) {
        printf("data ok\n");
        if (start_Hour > end_Hour) {
          // ////printf("\natoi(user_validateData->start.hour) %d\n",
          // start_Hour);
          // ////printf("\natoi(user_validateData->end.hour) %d\n", end_Hour);
          // ////printf("nowTime.time %d\n", nowTime.time);
          // ////printf("\n START HOUR > END HOUR1\n");
          if (start_Hour <= nowTime.time) {
            // ////printf("\n START HOUR > END HOUR2\n");
            return 1;
          }

          // ////printf("\n START HOUR > END HOUR1.2\n");
          if (end_Hour >= nowTime.time) {
            printf("\n START HOUR > END HOUR2\n");
            return 1;
          } else {
            printf("\n START HOUR > END HOUR4\n");
            return 0;
          }
        } else {
          if ((nowTime.time >= start_Hour) && (nowTime.time <= end_Hour)) {
            printf("**** hora ok\n");
            return 1;
          } else {
            printf("**** hora errada\n");
            return 0;
          }
        }
      } else {
        // //printf("data errada\n");
        return 0;
      }
    } else {
      if (nowTime.date < atoi(user_validateData->start.date)) {
        return 0;
      }

      if (start_Hour > end_Hour) {
        // ////printf("\n START HOUR > END HOUR1\n");
        if (start_Hour <= nowTime.time) {
          // ////printf("\n START HOUR > END HOUR2\n");
          return 1;
        } else if (end_Hour >= nowTime.time) {
          // ////printf("\n START HOUR > END HOUR2\n");
          return 1;
        } else {
          // ////printf("\n START HOUR > END HOUR4\n");
          return 0;
        }
      } else {
        if ((nowTime.time >= start_Hour) && (nowTime.time <= end_Hour)) {
          // ////printf("**** hora ok\n");
          return 1;
        } else {
          // ////printf("**** hora errada\n");
          return 0;
        }
      }
    }
  } else {
    // ////printf("**** week error ****\n");
    return 0;
  }
  return 0;
}

uint8_t validate_DataUser(MyUser *user_validateData, char *password) {

  if (!strcmp(password, user_validateData->key)) {

    if (user_validateData->permition == '1' ||
        user_validateData->permition == '2') {

      get_RTC_System_Time();
      return 1;
    } else {
      printf("\n\npass errada222\n\n");
      return verify_TimeAcess(user_validateData);
    }
  } else {
    printf("pass errada\n");
    return 0;
  }

  return 0;
}

bool isLeapYear(int year) {

  if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
    return true; // é bissexto
  } else {
    return false; // não é bissexto
  }
}

int parseDatetoInt(int date, int days) {

  int daysPerMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  int year = date / 10000;
  int month = (date / 100) - (year * 100);
  int day = date - ((year * 10000) + (month * 100));

  day += days;

  while (day > daysPerMonth[month - 1]) {
    if (month == 2 && isLeapYear(year)) {
      if (day > 29) {
        day -= 29;
        month++;
      } else {
        break;
      }
    } else {
      day -= daysPerMonth[month - 1];
      month++;
    }

    if (month > 12) {
      month = 1;
      year++;
    }
  }
  int finalDate = year * 10000 + month * 100 + day;
  // ////printf("new year %d\n", finalDate);

  return finalDate;
}

void printUser(MyUser user) {
    printf("Phone: %s\n", user.phone);
    printf("First Name: %s\n", user.firstName);
    printf("Start Date: %s\n", user.start.date);
    printf("Start Hour: %s\n", user.start.hour);
    printf("End Days: %s\n", user.end.days);
    printf("End Hour: %s\n", user.end.hour);
    printf("Key: %s\n", user.key);
    printf("Permition: %c\n", user.permition);
    printf("Week: %s\n", user.week);
    printf("Relay Permition: %c\n", user.relayPermition);
    printf("BLE Security: %c\n", user.ble_security);
    printf("Erase User After Date: %c\n", user.erase_User_After_Date);

#if CONFIG_WIEGAND_CODE == 1
    printf("Wiegand Code: %s\n", user.wiegand_code);
    printf("Wiegand Relay Permition: %c\n", user.wiegand_rele_permition);
#endif

    printf("RF Serial: %s\n", user.rf_serial);
    printf("RF1 Relay: %c\n", user.rf1_relay);
    printf("RF2 Relay: %c\n", user.rf2_relay);
}

uint8_t validate_Hour(char *str) {
  if (strlen(str) != 4) {
    return 0;
  }
  // ////printf("\nstr  len\n");
  if (str[0] < '0' || str[0] > '2') {
    return 0;
  }
  // ////printf("\nstr  11\n");
  if (str[1] < '0' || str[1] > '9') {
    return 0;
  }
  // ////printf("\nstr  22\n");
  if (str[2] < '0' || str[2] > '5') {
    return 0;
  }

  // ////printf("\nstr  33\n");
  if (str[3] < '0' || str[3] > '9') {
    return 0;
  }

  if (str[0] == '2' && str[1] > '3') {
    return 0;
  }

  return 1;
}

uint8_t check_DateisValid(char *date) {

  char year[3] = {};
  char month[3] = {};
  char day[3] = {};

  // ////printf("start date %s\n", date);
  for (int i = 0; i < strlen(date); i++) {
    if ((date[i] > '9' || date[i] < '0') || strlen(date) != 6) {
      return 0;
    }
  }

  year[0] = date[0];
  year[1] = date[1];

  month[0] = date[2];
  month[1] = date[3];

  day[0] = date[4];
  day[1] = date[5];

  int auxYear = 2000 + atoi(year);
  // ////printf("auxyear %d, year %s\n", auxYear, year);
  // ////printf("Date --.%d %d %d\n", auxYear, atoi(month), atoi(day));
  //   check year
  if (auxYear >= 2022 && auxYear < 2100) {

    // ////printf("Date --.%d %d %d\n", auxYear, atoi(month), atoi(day));
    //   check month
    if (atoi(month) >= 1 && atoi(month) <= 12) {
      // check days
      if ((atoi(day) >= 1 && atoi(day) <= 31) &&
          (atoi(month) == 1 || atoi(month) == 3 || atoi(month) == 5 ||
           atoi(month) == 7 || atoi(month) == 8 || atoi(month) == 10 ||
           atoi(month) == 12)) {
        // ////printf("Date is valid.%d %d %d\n", auxYear, atoi(month),
        // atoi(day));
      } else if ((atoi(day) >= 1 && atoi(day) <= 30) &&
                 (atoi(month) == 4 || atoi(month) == 6 || atoi(month) == 9 ||
                  atoi(month) == 11)) {
        // ////printf("Date is valid.\n");
      } else if ((atoi(day) >= 1 && atoi(day) <= 28) && (atoi(month) == 2)) {
        // ////printf("Date is valid.\n");
      } else if ((atoi(day) == 29 && atoi(month) == 2) &&
                 ((auxYear % 400 == 0) ||
                  ((auxYear % 4 == 0) && (auxYear % 100 != 0)))) {
        // ////printf("Date is valid.\n");
      } else {
        return 0;
      }
    } else {
      return 0;
    }
  } else {
    return 0;
  }

  return 1;
}

uint8_t MyUser_newPassword(MyUser *user_validateData, char *password,
                           int line) {

  if (strlen(password) == 6) {
    for (int i = 0; i < 6; i++) {
      if (password[i] < '0' || password[i] > '9') {
        return 0;
      }
    }

    memset(user_validateData->key, 0, sizeof(user_validateData->key));
    sprintf(user_validateData->key, "%s", password);

    // ////printf("CHANGE PASS 1\n");

    if (replaceUser(user_validateData) == ESP_OK) {
      return 1;
    } else {
      return 0;
    }
    // MyUser_List_AllUsers();
  }

  return 0;
}

/* uint8_t put_wiegand_to_phNumber(char *phNumber, char *wiegandNumber)
{
    char strUser[200] = {};
    MyUser validateData_user;

    if (MyUser_Search_User(phNumber, strUser) == ESP_OK)
    {
        parse_ValidateData_User(strUser, &validateData_user);

        if (!checkIf_wiegandExist(validateData_user.wiegand_code, strUser))
        {
            strcpy(validateData_user.wiegand_code, wiegandNumber);

            replaceUser(&validateData_user);

            return 1;
        }
        else
        {
            return 0;
        }
    }

    return 0;
} */

uint8_t put_phNumber_to_wiegand(char *phNumber, char *wiegandNumber) {
  char strUser[200] = {};
  MyUser validateData_user;

  if (MyUser_Search_User(phNumber, strUser) == ESP_FAIL) {
    if (checkIf_wiegandExist(wiegandNumber, strUser)) {
      parse_ValidateData_User(strUser, &validateData_user);
      strcpy(validateData_user.phone, phNumber);

      replaceUser(&validateData_user);
      return 1;
    } else {
      return 0;
    }
  } else {
    return 0;
  }

  return 0;
}

/**
 * @brief Replace user data in the storage.
 *
 * This function takes a pointer to a MyUser structure and replace the user
 * data in the storage with the new data.
 *
 * @param user_validateData pointer to a MyUser structure
 * @return ESP_OK if the data was replaced successfully, ESP_FAIL otherwise
 */
uint8_t replaceUser(MyUser *user_validateData) {

  char newline[200] = {};
  char auxPhone[30] = {};

  printf("new line replaceUser 11 -- %c\n", user_validateData->permition);

  sprintf(auxPhone, "%s",
          check_IF_haveCountryCode(user_validateData->phone, 1));

  printf("new line replaceUser 12 -- %c\n", user_validateData->permition);

  if (strlen(auxPhone) < 1) {
    if (user_validateData->wiegand_code[0] == ':' &&
        user_validateData->rf_serial[0] != ':') {
      char *auxW_del;
      asprintf(&auxW_del, "$%s", user_validateData->wiegand_code);
      if (user_validateData->permition == '1') {
        nvs_erase_key(nvs_Admin_handle, auxW_del);
      } else if (user_validateData->permition == '0') {
        nvs_erase_key(nvs_Users_handle, auxW_del);
      } else {
        free(auxW_del);
        return ESP_FAIL;
      }

      free(auxW_del);
      sprintf(auxPhone, "&%s", user_validateData->rf_serial);
    } else {
      sprintf(auxPhone, "$%s", user_validateData->wiegand_code);
    }
  }
  printf("new line replaceUser 13 -- %c\n", user_validateData->permition);
  /* Remove extra new line character from stdin */
  // fflush(stdin);
  memset(&newline, 0, 200);
  user_validateData->week[7] = '\0';

  printf("new line replaceUser 14 -- %c\n", user_validateData->permition);
  // #ifdef CONFIG_WIEGAND_CODE

  sprintf(&newline, "%s;%s;%s;%s;%s;%s;%s;%c;%s;%c;%c;%c;%c;%s;%s;%c;%c;",
          user_validateData->phone, user_validateData->firstName,
          user_validateData->start.date, user_validateData->start.hour,
          user_validateData->end.days, user_validateData->end.hour,
          user_validateData->key, user_validateData->permition,
          user_validateData->week, user_validateData->relayPermition,
          user_validateData->ble_security,
          user_validateData->erase_User_After_Date,
          user_validateData->wiegand_rele_permition,
          user_validateData->wiegand_code, user_validateData->rf_serial,
          user_validateData->rf1_relay, user_validateData->rf2_relay);

  /* #elif

    sprintf(&newline, "%s;%s;%s;%s;%s;%s;%s;%c;%s;%c;%c;%c;",
            user_validateData->phone, user_validateData->firstName,
            user_validateData->start.date, user_validateData->start.hour,
            user_validateData->end.days, user_validateData->end.hour,
            user_validateData->key, user_validateData->permition,
            user_validateData->week, user_validateData->relayPermition,
            user_validateData->ble_security,
            user_validateData->erase_User_After_Date);

  #endif */

  printf("new line replaceUser %s - %c\n", newline,
         user_validateData->permition);

  if (user_validateData->permition == '0') {
    // //printf("\n\nnew line replaceUser1\n\n");

    if (save_STR_Data_In_Storage(auxPhone, &newline, nvs_Users_handle) ==
        ESP_OK) {

      /* #ifdef CONFIG_WIEGAND_CODE

                  if (user_validateData->wiegand_code[0] != '#')
                  {
                      save_STR_Data_In_Storage(user_validateData->wiegand_code,
      &newline, nvs_wiegand_codes_users_handle);
                  }

      #endif */

      return ESP_OK;
    } else {
      return ESP_FAIL;
    }
  } else if (user_validateData->permition == '1') {
    if (save_STR_Data_In_Storage(auxPhone, &newline, nvs_Admin_handle) ==
        ESP_OK) {

      /* #ifdef CONFIG_WIEGAND_CODE

                  if (user_validateData->wiegand_code[0] != '#')
                  {
                      save_STR_Data_In_Storage(user_validateData->wiegand_code,
      &newline, nvs_wiegand_codes_admin_handle);
                  }
      #endif */
      return ESP_OK;
    } else {
      return ESP_FAIL;
    }
  } else if (user_validateData->permition == '2') {
    if (save_STR_Data_In_Storage(auxPhone, &newline, nvs_Owner_handle) ==
        ESP_OK) {
      if (save_STR_Data_In_Storage(NVS_KEY_OWNER_INFORMATION, newline,
                                   nvs_System_handle)) {

        /* #ifdef CONFIG_WIEGAND_CODE

                        if (user_validateData->wiegand_code[0] != '#')
                        {
                            save_STR_Data_In_Storage(user_validateData->wiegand_code,
        &newline, nvs_wiegand_codes_owner_handle);
                        }

        #endif */
        return ESP_OK;
      } else {
        return ESP_FAIL;
      }
      // UsersCountNumbers++;
    } else {
      return ESP_FAIL;
    }
  } else {
    return ESP_FAIL;
  }

  return ESP_FAIL;
}
char *MyUser_ReadUser(uint8_t BLE_SMS_Indication, char *payload,
                      char usrPermition, char *phNumber_Get) {
  int line = 0;
  MyUser user_Get_ValidateData;
  memset(file_contents_Users, 0, sizeof(file_contents_Users));
  memset(aux_FileContens, 0, sizeof(aux_FileContens));
  // ////printf("read user 11 payload %s\n", payload);

  if (MyUser_Search_User(payload, file_contents_Users) == ESP_OK) {
    memset(&user_Get_ValidateData, 0, sizeof(user_Get_ValidateData));
    parse_ValidateData_User(file_contents_Users, &user_Get_ValidateData);
    // ////printf("read user after parse user\n");

    if (BLE_SMS_Indication == BLE_INDICATION) {
      // ////printf("read user 1123 %s\n", file_contents_Users);
      //   parse_ValidateData_User(file_contents_Users, &user_Get_ValidateData);
      if (user_Get_ValidateData.ble_security == '0') {
        if (usrPermition != '2') {
          if (!strcmp(phNumber_Get, check_IF_haveCountryCode(
                                        user_Get_ValidateData.phone, 0))) {
            // ////printf("\n\n enter change ble security\n\n");
            user_Get_ValidateData.ble_security = '1';
            replaceUser(&user_Get_ValidateData);
          }
        }
      }

      erase_Password_For_Rsp(file_contents_Users, aux_FileContens);
      return aux_FileContens;
    } else if (BLE_SMS_Indication == SMS_INDICATION) {
      if (usrPermition > user_Get_ValidateData.permition ||
          usrPermition == '2') {

        char permitionSTR[75];
        memset(permitionSTR, 0, sizeof(permitionSTR));
        char ch[2] = {' ', '\0'};
        char auxWeekdays[30];
        memset(auxWeekdays, 0, sizeof(auxWeekdays));
        uint8_t weekdays_Count = 0;

        // ////printf("\nuser_Get_ValidateData.permition %c\n",
        // user_Get_ValidateData.permition);
        if (user_Get_ValidateData.permition == '1' ||
            user_Get_ValidateData.permition == '2') {
          strcpy(auxWeekdays, return_Json_SMS_Data("ALL DAYS"));

          if (user_Get_ValidateData.permition == '0') {
            strcpy(permitionSTR, return_Json_SMS_Data("USER"));
          } else if (user_Get_ValidateData.permition == '1') {
            strcpy(permitionSTR, return_Json_SMS_Data("ADMIN"));
          } else if (user_Get_ValidateData.permition == '2') {
            strcpy(permitionSTR, return_Json_SMS_Data("OWNER"));
          } else {
            strcpy(permitionSTR, "ERRO");
          }

          memset(file_contents_Users, 0, sizeof(file_contents_Users));

          if (strcmp(user_Get_ValidateData.firstName, "S/N")) {
            sprintf(file_contents_Users,
                    return_Json_SMS_Data("OWNER_ADMIN_READ_SMS"),
                    user_Get_ValidateData.firstName,
                    user_Get_ValidateData.phone, permitionSTR);
          } else {
            sprintf(file_contents_Users,
                    return_Json_SMS_Data("OWNER_ADMIN_READ_SMS"),
                    return_Json_SMS_Data("NO_NAME"),
                    user_Get_ValidateData.phone, permitionSTR);
          }

          // ////printf("read user SMS %s\n", file_contents_Users);
          return file_contents_Users;
        } else if (user_Get_ValidateData.permition == '0') {
          char str_Relay_Permition[20];
          memset(str_Relay_Permition, 0, 20);

          if (user_Get_ValidateData.relayPermition == '0') {
            strcpy(str_Relay_Permition,
                   return_Json_SMS_Data("NOT_RESTRICTION"));
          } else if (user_Get_ValidateData.relayPermition == '1') {
            strcpy(str_Relay_Permition, return_Json_SMS_Data("RELAY_1"));
          } else if (user_Get_ValidateData.relayPermition == '2') {
            strcpy(str_Relay_Permition, return_Json_SMS_Data("RELAY_2"));
          } else {
            return return_Json_SMS_Data("ERROR_GET");
          }

          for (int i = 0; i < strlen(user_Get_ValidateData.week); i++) {
            if (user_Get_ValidateData.week[i] == '1') {
              weekdays_Count++;
              strcat(auxWeekdays, return_Json_SMS_Data(weekDaysFeedback[i]));
              strcat(auxWeekdays, ch);
            }
          }

          if (weekdays_Count == 7) {
            strcpy(auxWeekdays, return_Json_SMS_Data("ALL DAYS"));
          }

          char aux_endDate[20];
          memset(aux_endDate, 0, sizeof(aux_endDate));

          if (user_Get_ValidateData.end.days[0] == '*') {
            sprintf(aux_endDate, "%s", return_Json_SMS_Data("NOT_LIMIT"));
          } else {
            int auxInte_endDate =
                parseDatetoInt(atoi(user_Get_ValidateData.start.date),
                               atoi(user_Get_ValidateData.end.days));
            sprintf(aux_endDate, "%d", auxInte_endDate);
            sprintf(aux_endDate, "%c%c/%c%c/%c%c", aux_endDate[0],
                    aux_endDate[1], aux_endDate[2], aux_endDate[3],
                    aux_endDate[4], aux_endDate[5]);
          }

          char nameToSMS[50];

          if (strcmp(user_Get_ValidateData.firstName, "S/N")) {
            strcpy(nameToSMS, user_Get_ValidateData.firstName);
          } else {
            strcpy(nameToSMS, return_Json_SMS_Data("NO_NAME"));
          }

          if (user_Get_ValidateData.permition == '0') {
            strcpy(permitionSTR, return_Json_SMS_Data("USER"));
          } else if (user_Get_ValidateData.permition == '1') {
            strcpy(permitionSTR, return_Json_SMS_Data("ADMIN"));
          } else if (user_Get_ValidateData.permition == '2') {
            strcpy(permitionSTR, return_Json_SMS_Data("OWNER"));
          } else {
            strcpy(permitionSTR, "ERRO");
          }

          memset(file_contents_Users, 0, sizeof(file_contents_Users));
          sprintf(file_contents_Users, return_Json_SMS_Data("USER_READ_SMS"),
                  nameToSMS, user_Get_ValidateData.phone, permitionSTR,
                  user_Get_ValidateData.start.date[0],
                  user_Get_ValidateData.start.date[1],
                  user_Get_ValidateData.start.date[2],
                  user_Get_ValidateData.start.date[3],
                  user_Get_ValidateData.start.date[4],
                  user_Get_ValidateData.start.date[5], aux_endDate,
                  user_Get_ValidateData.start.hour[0],
                  user_Get_ValidateData.start.hour[1],
                  user_Get_ValidateData.start.hour[2],
                  user_Get_ValidateData.start.hour[3],
                  user_Get_ValidateData.end.hour[0],
                  user_Get_ValidateData.end.hour[1],
                  user_Get_ValidateData.end.hour[2],
                  user_Get_ValidateData.end.hour[3], auxWeekdays,
                  str_Relay_Permition);

          return file_contents_Users;
        } else {
          sprintf(file_contents_Users, "%s", ERROR_GET);
          return file_contents_Users;
        }
      } else {
        return return_Json_SMS_Data("ERROR_USER_NOT_PERMITION");
      }
    }
  } else {
    return return_Json_SMS_Data("ERROR_USER_NOT_FOUND");
  }

  return file_contents_Users;
}

void copiar_a_partir_do_segundo_caractere(const char *original, char *destino) {
  // Verifica se a string original tem mais de 1 caractere
  if (strlen(original) > 1) {
    // Copia a string a partir do segundo caractere (índice 1)
    strcpy(destino, original + 1);
  } else {
    // Se a string for muito curta, copia uma string vazia
    destino[0] = '\0';
  }
}

uint8_t Myuser_delete_ALLUser() {

  int count = 0;
  char value[200] = {};
  nvs_iterator_t it;
  it = nvs_entry_find("keys", NVS_USERS_NAMESPACE, NVS_TYPE_STR);
   char w_key[30] = {};
  // ////printf("Iterate NVS\n");

  while (it != NULL) {
    count++;
    nvs_entry_info_t info;
    nvs_entry_info(it, &info);
    it = nvs_entry_next(it);
    printf("key '%s', type '%d' \n", info.key, info.type);
    get_Data_Users_From_Storage(info.key, &value);

    // ////printf("  * value: %s\n", value);
  }

  it = nvs_entry_find("keys", NVS_WIEGAND_CODES_GUEST_NAMESPACE, NVS_TYPE_STR);

  // ////printf("Iterate NVS\n");
  
  while (it != NULL) {
    nvs_entry_info_t info;
    nvs_entry_info(it, &info);

    it = nvs_entry_next(it);

    get_Data_Users_From_Storage(info.key, &value);
    memset(w_key, 0, sizeof(w_key));
    copiar_a_partir_do_segundo_caractere(info.key, w_key);

    if (get_antipassback_user(w_key) == ESP_OK) {
      erase_antipassback_user(w_key);

      nvs_get_u32(nvs_System_handle, NVS_ANTIPASSBACK_PEOPLE_NUMBER,
                  &anti_passback_people_number);
    
      if (anti_passback_people_number > 0) {       
        anti_passback_people_number--;
        nvs_set_u32(nvs_System_handle, NVS_ANTIPASSBACK_PEOPLE_NUMBER,
                    anti_passback_people_number);
      }
    }

    if (value[0] == '#') {
      count++;
    }

    memset(value, 0, sizeof(value));
  }

  if (nvs_erase_all(nvs_Users_handle) == ESP_OK) {
    nvs_erase_all(nvs_rf_codes_users_handle);
    nvs_erase_all(nvs_wiegand_codes_users_handle);
    nvs_erase_all(nvs_rf_codes_users_handle);

    UsersCountNumbers = get_User_Counter_From_Storage();
    UsersCountNumbers = UsersCountNumbers - count;

    nvs_set_u32(nvs_System_handle, NVS_KEY_GUEST_COUNTER, 0);
    GuestCountNumbers = 0;

    save_User_Counter_In_Storage(UsersCountNumbers);
    free(it);
    return 1;
  } else {
    free(it);
    return 0;
  }

  return 0;
}

char *MyUser_new_hour(uint8_t BLE_SMS_Indication, char *payload,
                      mqtt_information *mqttInfo) {
  char phone_Number[20];
  char newDate[10];
  char startHour[5];
  char endHour[5];
  int line = 0;

  uint8_t dotCounter = 0;
  MyUser *myUser_newHour = malloc(sizeof(MyUser));
  memset(phone_Number, 0, sizeof(phone_Number));
  memset(newDate, 0, sizeof(newDate));
  memset(startHour, 0, sizeof(startHour));
  memset(endHour, 0, sizeof(endHour));
  memset(myUser_newHour, 0, sizeof(MyUser));
  memset(myUser_newHour->week, 0, sizeof(myUser_newHour->week));

  for (int i = 0; i < strlen(payload); i++) {
    if (payload[i] == '.') {
      dotCounter++;
    }
  }

  uint8_t aux = 0;
  uint8_t strIndex = 0;
  if (dotCounter == 1) {
    for (int i = 0; i < strlen(payload); i++) {
      if (payload[i] == '.') {
        aux++;
        strIndex = 0;
      } else if (aux == 0) {
        if (strIndex < 20) {
          phone_Number[strIndex] = payload[i];
          strIndex++;
        }
      } else if (aux == 1) {
        if (strIndex < 8) {
          if (strIndex < 4) {
            startHour[strIndex] = payload[i];
            strIndex++;
          } else {
            endHour[strIndex - 4] = payload[i];
            strIndex++;
          }
        } else {
          free(myUser_newHour);
          return return_Json_SMS_Data("ERROR_ADD_TIME_SIZE");
        }
      } else {
        free(myUser_newHour);
        // ////printf(ERROR_INPUT_DATA);
        return return_Json_SMS_Data("ERROR_INPUT_DATA");
      }
    }
  } else {
    free(myUser_newHour);
    // ////printf(ERROR_INPUT_DATA);
    return return_Json_SMS_Data("ERROR_INPUT_DATA");
  }
  // ////printf("startHour  new date %s, %d\n", startHour, strlen(startHour));
  // ////printf("endHour  new date %s, %d\n", endHour, strlen(endHour));
  if (((strlen(endHour) + strlen(startHour)) ==
       8) /* && (atoi(endHour) < 2400) && (atoi(startHour) < 2400) */) {

    int inicial_H = (startHour[0] - '0') * 10 + (startHour[1] - '0');
    int inicial_M = (startHour[2] - '0') * 10 + (startHour[3] - '0');
    int final_H = (endHour[0] - '0') * 10 + (endHour[1] - '0');
    int final_M = (endHour[2] - '0') * 10 + (endHour[3] - '0');

    // ////printf("\n\n inicial_H - %d, inicial_M - %d, final_H - %d,final_M -
    // %d\n\n", inicial_H, inicial_M, final_H, final_M);

    if (final_H < 0 || final_H > 23 || final_M < 0 || final_M > 59) {
      free(myUser_newHour);
      return return_Json_SMS_Data("ERROR_WRONG_END_TIME");
    }

    if (inicial_H < 0 || inicial_H > 23 || inicial_M < 0 || inicial_M > 59) {
      // ////printf("\n\n starthour tttt - %c, %c, %c, %c\n\n", startHour[0],
      // startHour[1], startHour[2], startHour[3]);
      free(myUser_newHour);
      return return_Json_SMS_Data("ERROR_WRONG_START_TIME");
    }

    if (!strcmp(endHour, startHour)) {
      free(myUser_newHour);
      // ////printf(ERROR_INPUT_DATA);
      return return_Json_SMS_Data("ERROR_INPUT_DATA");
    }

    // ////printf("phone number new date %s\n", phone_Number);
    // ////printf("startHour  new date %s\n", startHour);
    // ////printf("endHour  new date %s\n", endHour);

    memset(file_contents_Users, 0, sizeof(file_contents_Users));
    if (MyUser_Search_User(phone_Number, file_contents_Users) == ESP_OK) {
      // ////printf("endHour  filecontents %s\n", file_contents_Users);
      parse_ValidateData_User(file_contents_Users, myUser_newHour);

      if (myUser_newHour->permition == '1' ||
          myUser_newHour->permition == '2') {

        if (myUser_newHour->permition == '1') {
          free(myUser_newHour);
          return return_Json_SMS_Data("ERROR_THIS_IMPOSSIBLE_CHANGE_ADMIN");
        } else {
          free(myUser_newHour);
          return return_Json_SMS_Data("ERROR_THIS_IMPOSSIBLE_CHANGE_OWNER");
        }
      }

      // ////printf("myUser_newDate week  %s\n", myUser_newHour->week);

      memset(myUser_newHour->start.hour, 0, sizeof(myUser_newHour->start.hour));
      memset(myUser_newHour->end.hour, 0, sizeof(myUser_newHour->end.hour));

      memcpy(myUser_newHour->start.hour, startHour, sizeof(startHour));
      memcpy(myUser_newHour->end.hour, endHour, sizeof(endHour));

      // ////printf("myUser_newDate startHour   %s\n",
      // myUser_newHour->start.hour);
      // ////printf("myUser_newDate.end %s\n", myUser_newHour->end.hour);

      replaceUser(myUser_newHour);
      memset(file_contents_Users, 0, sizeof(file_contents_Users));
      memset(aux_FileContens, 0, sizeof(aux_FileContens));
      MyUser_Search_User(phone_Number, aux_FileContens);

      // ////printf("file contents new pass %s\n", file_contents_Users);

      if (BLE_SMS_Indication == BLE_INDICATION ||
          BLE_SMS_Indication == UDP_INDICATION) {

        erase_Password_For_Rsp(aux_FileContens, file_contents_Users);
        free(myUser_newHour);

        return file_contents_Users;
      } else if (BLE_SMS_Indication == SMS_INDICATION) {
        erase_Password_For_Rsp(aux_FileContens, buffer);

        sprintf(mqttInfo->data, "%s %c %c %s", USER_ELEMENT, SET_CMD,
                HOUR_PARAMETER, buffer);
        send_UDP_queue(mqttInfo);

        memset(file_contents_Users, 0, sizeof(file_contents_Users));
        sprintf(file_contents_Users,
                return_Json_SMS_Data("ACCESS_HOURS_HAVE_BEEN_UPDATED_BETWEEN"),
                phone_Number, myUser_newHour->start.hour[0],
                myUser_newHour->start.hour[1], myUser_newHour->start.hour[2],
                myUser_newHour->start.hour[3], myUser_newHour->end.hour[0],
                myUser_newHour->end.hour[1], myUser_newHour->end.hour[2],
                myUser_newHour->end.hour[3]);
        free(myUser_newHour);
        return file_contents_Users;
      } else {
        free(myUser_newHour);
        return return_Json_SMS_Data("ERROR_INPUT_DATA");
      }
    } else {
      free(myUser_newHour);
      return return_Json_SMS_Data("ERROR_USER_NOT_FOUND");
    }

    // parse_ValidateData_User(file_contents_Users, &myUser_newHour);
  } else {
    free(myUser_newHour);
    return return_Json_SMS_Data("ERROR_ADD_TIME_SIZE");
  }
}

char *MyUser_new_WeekDays(uint8_t BLE_SMS_Indication, char *payload,
                          mqtt_information *mqttInfo) {
  char phone_Number[20];
  char weekdays[8];

  int line;
  uint8_t dotCounter = 0;
  MyUser *myUser_newHour = malloc(sizeof(MyUser));
  memset(phone_Number, 0, sizeof(phone_Number));
  memset(weekdays, 0, sizeof(weekdays));
  memset(myUser_newHour, 0, sizeof(MyUser));
  memset(myUser_newHour->week, 0, sizeof(myUser_newHour->week));

  for (int i = 0; i < strlen(payload); i++) {
    if (payload[i] == '.') {
      dotCounter++;
    }
  }

  uint8_t aux = 0;
  uint8_t strIndex = 0;
  if (dotCounter == 1) {
    for (int i = 0; i < strlen(payload); i++) {
      if (payload[i] == '.') {
        aux++;
        strIndex = 0;
      } else if (aux == 0) {
        if (strIndex < 20) {
          phone_Number[strIndex] = payload[i];
          strIndex++;
        }
      } else if (aux == 1) {
        if (strIndex < 8) {
          weekdays[strIndex] = payload[i];
          strIndex++;
        }
      } else {
        free(myUser_newHour);
        // ////printf(ERROR_INPUT_DATA);
        return return_Json_SMS_Data("ERROR_INPUT_DATA");
      }
    }
  } else {
    free(myUser_newHour);
    // ////printf(ERROR_INPUT_DATA);
    return return_Json_SMS_Data("ERROR_INPUT_DATA");
  }

  // ////printf("weekdays %s\n", weekdays);
  for (int i = 0; i < 7; i++) {
    if ((weekdays[i] != '0' && weekdays[i] != '1') || strlen(weekdays) != 7) {
      free(myUser_newHour);
      return return_Json_SMS_Data("ERROR_WRONG_WEEK_DAYS");
    }
  }

  memset(file_contents_Users, 0, sizeof(file_contents_Users));

  if (MyUser_Search_User(phone_Number, file_contents_Users) == ESP_OK) {
    // ////printf("endHour  filecontents %s\n", file_contents_Users);
    parse_ValidateData_User(file_contents_Users, myUser_newHour);
    // ////printf("myUser_newDate week  %s\n", myUser_newHour->week);

    if (myUser_newHour->permition == '1' || myUser_newHour->permition == '2') {

      if (myUser_newHour->permition == '1') {
        free(myUser_newHour);
        return return_Json_SMS_Data("ERROR_THIS_IMPOSSIBLE_CHANGE_ADMIN");
      } else {
        free(myUser_newHour);
        return return_Json_SMS_Data("ERROR_THIS_IMPOSSIBLE_CHANGE_OWNER");
      }
    }

    memset(myUser_newHour->week, 0, sizeof(myUser_newHour->week));

    memcpy(myUser_newHour->week, weekdays, sizeof(weekdays));

    // //printf("myUser_newDate weekdays %s\n", myUser_newHour->week);

    replaceUser(myUser_newHour);
    memset(file_contents_Users, 0, sizeof(file_contents_Users));
    memset(aux_FileContens, 0, sizeof(aux_FileContens));
    MyUser_Search_User(phone_Number, aux_FileContens);

    if (BLE_SMS_Indication == BLE_INDICATION ||
        BLE_SMS_Indication == UDP_INDICATION) {
      // memset(aux_FileContens, 0, sizeof(aux_FileContens));

      // MyUser_Search_User(phone_Number, aux_FileContens);
      erase_Password_For_Rsp(aux_FileContens, file_contents_Users);
      // ////printf("file contents new pass %s\n", file_contents_Users);
      return file_contents_Users;
    } else if (BLE_SMS_Indication == SMS_INDICATION) {
      memset(buffer, 0, sizeof(buffer));
      memset(buffer_UDP_users, 0, sizeof(buffer_UDP_users));
      erase_Password_For_Rsp(aux_FileContens, buffer);

      sprintf(mqttInfo->data, "%s %c %c %s", USER_ELEMENT, SET_CMD,
              WEEK_PARAMETER, buffer);
      send_UDP_queue(mqttInfo);

      char ch[2] = {' ', '\0'};
      char auxWeekdays[30];
      memset(auxWeekdays, 0, sizeof(auxWeekdays));

      for (int i = 0; i < strlen(myUser_newHour->week); i++) {
        if (myUser_newHour->week[i] == '1') {
          strcat(auxWeekdays, return_Json_SMS_Data(weekDaysFeedback[i]));
          strcat(auxWeekdays, ch);
        }
      }

      sprintf(file_contents_Users,
              return_Json_SMS_Data(
                  "DAYS_OF_THE_WEEK_WITH_ACCESS_HAS_BEEN_UPDATED_TO"),
              phone_Number, auxWeekdays);

      // ////printf("\nweekdays file cont %s\n", file_contents_Users);
      free(myUser_newHour);
      return file_contents_Users;
    } else {
      free(myUser_newHour);
      return return_Json_SMS_Data("ERROR_INPUT_DATA");
    }
  } else {
    free(myUser_newHour);
    return return_Json_SMS_Data("ERROR_USER_NOT_FOUND");
  }

  return return_Json_SMS_Data("ERROR_INPUT_DATA");
  // parse_ValidateData_User(file_contents_Users, &myUser_newHour);
}

char *MyUser_new_UserName(uint8_t BLE_SMS_Indication, char *payload, char *mrsp,
                          char permition, mqtt_information *mqttInfo) {

  char phone_Number[20] = {};
  char firstName[25] = {};
  char surname[4] = {};

  int line = 1;
  uint8_t dotCounter = 0;
  MyUser *myUser_newName = malloc(sizeof(MyUser));
  memset(phone_Number, 0, sizeof(phone_Number));
  memset(firstName, 0, sizeof(firstName));
  memset(surname, 0, sizeof(surname));
  memset(myUser_newName, 0, sizeof(MyUser));
  // memset(myUser_newName->week, 0, sizeof(myUser_newName->week));

  for (int i = 0; i < strlen(payload); i++) {
    if (payload[i] == '.') {
      dotCounter++;
    }
  }

  uint8_t aux = 0;
  uint8_t strIndex = 0;
  // ////printf("set name 1111 %s\n", firstName);

  if (dotCounter == 1) {
    for (int i = 0; i < strlen(payload); i++) {
      if (payload[i] == '.') {
        aux++;
        strIndex = 0;
      } else if (aux == 0) {
        if (strIndex < 20) {
          phone_Number[strIndex] = payload[i];
          strIndex++;
        } else {
          sprintf(mrsp, "%s", return_Json_SMS_Data("ERROR_INPUT_DATA"));
          return return_Json_SMS_Data("ERROR_INPUT_DATA");
        }
      } else if (aux == 1) {
        if (strIndex < 20) {
          firstName[strIndex] = payload[i];
          strIndex++;
        } else {
          sprintf(mrsp, "%s", return_Json_SMS_Data("ERROR_NAME_SIZE"));
          return return_Json_SMS_Data("ERROR_NAME_SIZE");
        }
      } else {
        sprintf(mrsp, "%s", return_Json_SMS_Data("ERROR_INPUT_DATA"));
        // ////printf(ERROR_INPUT_DATA);
        return return_Json_SMS_Data("ERROR_INPUT_DATA");
      }
    }

    if (strlen(firstName) < 1) {
      sprintf(mrsp, "%s", return_Json_SMS_Data("ERROR_INPUT_DATA"));
      return return_Json_SMS_Data("ERROR_INPUT_DATA");
    }
  } else {
    sprintf(mrsp, "%s", return_Json_SMS_Data("ERROR_INPUT_DATA"));
    // ////printf(ERROR_INPUT_DATA);
    return return_Json_SMS_Data("ERROR_INPUT_DATA");
  }

  // ////printf("firsname %s\n", firstName);

  memset(file_contents_Users, 0, sizeof(file_contents_Users));
  /*  char auxPhoneNumber[20];
   sprintf(auxPhoneNumber,"%s",phone_Number);
   memset(phone_Number,0,strlen(phone_Number));

   sprintf(phone_Number,"%s",check_IF_haveCountryCode(auxPhoneNumber));

   //////printf("") */

  if (MyUser_Search_User(phone_Number, file_contents_Users) == ESP_OK) {

    // ////printf("endHour  filecontents %s\n", file_contents_Users);
    parse_ValidateData_User(file_contents_Users, myUser_newName);

    if (myUser_newName->permition >= permition && permition != '2') {
      sprintf(mrsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
      return return_Json_SMS_Data("ERROR_USER_NOT_PERMITION");
    }

    // ////printf("aux_line_user2 %d\n", line);
    memset(myUser_newName->firstName, 0, sizeof(myUser_newName->firstName));
    sprintf(myUser_newName->firstName, "%s", firstName);

    // ////printf("myUser_newName->firstName %s\n", myUser_newName->firstName);

    replaceUser(myUser_newName);
    // ////printf("aux_line_user2 11\n");
    memset(file_contents_Users, 0, sizeof(file_contents_Users));
    memset(aux_FileContens, 0, sizeof(aux_FileContens));
    // ////printf("aux_line_user2 11\n");

    if (BLE_SMS_Indication == BLE_INDICATION ||
        BLE_SMS_Indication == UDP_INDICATION) {
      MyUser_Search_User(phone_Number, aux_FileContens);

      erase_Password_For_Rsp(aux_FileContens, file_contents_Users);
      // ////printf("file contents new pass %s\n", file_contents_Users);
      sprintf(mrsp, "%s", file_contents_Users);
    } else if (BLE_SMS_Indication == SMS_INDICATION) {
      MyUser_Search_User(phone_Number, aux_FileContens);

      memset(buffer, 0, sizeof(buffer));
      memset(buffer_UDP_users, 0, sizeof(buffer_UDP_users));
      erase_Password_For_Rsp(aux_FileContens, buffer);

      sprintf(mqttInfo->data, "%s %c %c %s", USER_ELEMENT, SET_CMD,
              NAME_PARAMETER, buffer);
      send_UDP_queue(mqttInfo);

      memset(buffer, 0, sizeof(buffer));

      sprintf(mrsp, return_Json_SMS_Data("NAME_HAS_BEEN_CHANGED_TO"),
              phone_Number, myUser_newName->firstName);
    } else {
      return return_Json_SMS_Data("ERROR_INPUT_DATA");
    }

    free(myUser_newName);
    return "OK";
  } else {
    sprintf(mrsp, "%s", return_Json_SMS_Data("ERROR_USER_NOT_FOUND"));
    free(myUser_newName);
    return return_Json_SMS_Data("ERROR_USER_NOT_FOUND");
  }
  // parse_ValidateData_User(file_contents_Users, &myUser_newHour);
  sprintf(mrsp, "%s", return_Json_SMS_Data("ERROR_INPUT_DATA"));
  free(myUser_newName);
  return return_Json_SMS_Data("ERROR_INPUT_DATA");
}

char *MyUser_new_StartDate(uint8_t BLE_SMS_Indication, char *payload,
                           char *mrsp, mqtt_information *mqttInfo) {
  char phone_Number[20] = {};
  char startDate[7] = {};
  char endDays[3] = {};
  char year[3] = {};
  char month[3] = {};
  char day[3] = {};
  int line = 0;

  uint8_t dotCounter = 0;

  MyUser myUser_newDate;
  memset(&myUser_newDate, 0, sizeof(myUser_newDate));
  memset(phone_Number, 0, sizeof(phone_Number));
  memset(startDate, 0, sizeof(startDate));

  for (int i = 0; i < strlen(payload); i++) {
    if (payload[i] == '.') {
      dotCounter++;
    }
  }

  uint8_t aux = 0;
  uint8_t strIndex = 0;
  if (dotCounter == 1 || dotCounter == 2) {
    for (int i = 0; i < strlen(payload); i++) {
      if (payload[i] == '.') {
        aux++;
        strIndex = 0;
      } else if (aux == 0) {
        if (strIndex < 20) {
          phone_Number[strIndex] = payload[i];
          strIndex++;
        } else {
          strcpy(mrsp, return_Json_SMS_Data("ERROR_WRONG_PHONE_NUMBER"));
          return return_Json_SMS_Data("ERROR_WRONG_PHONE_NUMBER");
        }
      } else if (aux == 1) {
        if (strIndex < 7) {
          startDate[strIndex] = payload[i];
          strIndex++;
        } else {
          strcpy(mrsp, return_Json_SMS_Data("ERROR_WRONG_START_DATE"));
          return return_Json_SMS_Data("ERROR_WRONG_START_DATE");
        }
      } else if (aux == 2) {
        if (strIndex < 3) {
          endDays[strIndex] = payload[i];
          strIndex++;
        } else {
          strcpy(mrsp, return_Json_SMS_Data("ERROR_WRONG_LIMIT_DATE"));
          return return_Json_SMS_Data("ERROR_WRONG_LIMIT_DATE");
        }
      } else {
        // ////printf(ERROR_INPUT_DATA);
        strcpy(mrsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
        return return_Json_SMS_Data("ERROR_INPUT_DATA");
      }
    }
  } else {
    // ////printf(ERROR_INPUT_DATA);
    strcpy(mrsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
    return return_Json_SMS_Data("ERROR_INPUT_DATA");
  }

  phone_Number[strlen(phone_Number)] = '\0';

  if (dotCounter == 2) {

    if (strlen(endDays) == 0 || startDate[0] > '9' || startDate[0] < '0') {
      strcpy(mrsp, return_Json_SMS_Data("ERROR_WRONG_LIMIT_DATE"));
      return return_Json_SMS_Data("ERROR_WRONG_LIMIT_DATE");
    }

    if (endDays[0] != '*') {
      /* code */

      for (int i = 0; i < strlen(endDays); i++) {
        if ((endDays[i] > '9' || endDays[i] < '0')) {

          strcpy(mrsp, return_Json_SMS_Data("ERROR_WRONG_LIMIT_DATE"));
          return return_Json_SMS_Data("ERROR_WRONG_LIMIT_DATE");
        }
      }

      if (atoi(endDays) > 90) {

        strcpy(mrsp, return_Json_SMS_Data("ERROR_WRONG_LIMIT_DATE"));
        return return_Json_SMS_Data("ERROR_WRONG_LIMIT_DATE");
      }
    } else {
      if (endDays[1] != '\0') {

        strcpy(mrsp, return_Json_SMS_Data("ERROR_WRONG_LIMIT_DATE"));
        return return_Json_SMS_Data("ERROR_WRONG_LIMIT_DATE");
      }
    }
  }

  // ////printf("start date %s\n", startDate);

  if (strlen(startDate) != 6) {

    strcpy(mrsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
    return return_Json_SMS_Data("ERROR_INPUT_DATA");
  }

  for (int i = 0; i < strlen(startDate); i++) {
    if ((startDate[i] > '9' || startDate[i] < '0')) {

      strcpy(mrsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
      return return_Json_SMS_Data("ERROR_INPUT_DATA");
    }
  }

  year[0] = startDate[0];
  year[1] = startDate[1];

  month[0] = startDate[2];
  month[1] = startDate[3];

  day[0] = startDate[4];
  day[1] = startDate[5];

  int auxYear = 2000 + atoi(year);
  // ////printf("auxyear %d, year %s\n", auxYear, year);
  // ////printf("Date --.%d %d %d\n", auxYear, atoi(month), atoi(day));
  //   check year
  if (auxYear >= 2021 && auxYear < 2100) {

    // ////printf("Date --.%d %d %d\n", auxYear, atoi(month), atoi(day));
    //   check month
    if (atoi(month) >= 1 && atoi(month) <= 12) {
      // check days
      if ((atoi(day) >= 1 && atoi(day) <= 31) &&
          (atoi(month) == 1 || atoi(month) == 3 || atoi(month) == 5 ||
           atoi(month) == 7 || atoi(month) == 8 || atoi(month) == 10 ||
           atoi(month) == 12)) {
        // ////printf("Date is valid.%d %d %d\n", auxYear, atoi(month),
        // atoi(day));
      } else if ((atoi(day) >= 1 && atoi(day) <= 30) &&
                 (atoi(month) == 4 || atoi(month) == 6 || atoi(month) == 9 ||
                  atoi(month) == 11)) {
        // ////printf("Date is valid.\n");
      } else if ((atoi(day) >= 1 && atoi(day) <= 28) && (atoi(month) == 2)) {
        // ////printf("Date is valid.\n");
      } else if ((atoi(day) == 29 && atoi(month) == 2) &&
                 ((auxYear % 400 == 0) ||
                  ((auxYear % 4 == 0) && (auxYear % 100 != 0)))) {
        // ////printf("Date is valid.\n");
      } else {
        // strcpy(mrsp, return_Json_SMS_Data("ERROR_DAY_IS_NOT_VALID"));
        return return_Json_SMS_Data("ERROR_DAY_IS_NOT_VALID");
      }
    } else {
      // ////printf("Month is not valid.\n");
      //  strcpy(mrsp, return_Json_SMS_Data("ERROR_MONTH_IS_NOT_VALID"));
      return return_Json_SMS_Data("ERROR_MONTH_IS_NOT_VALID");
    }
  } else {
    // ////printf("Year is not valid.\n");
    //  strcpy(mrsp, return_Json_SMS_Data("ERROR_YEAR_IS_NOT_VALID"));
    return return_Json_SMS_Data("ERROR_YEAR_IS_NOT_VALID");
  }
  memset(file_contents_Users, 0, sizeof(file_contents_Users));

  if (MyUser_Search_User(phone_Number, file_contents_Users) == ESP_OK) {
    // ////printf("limit time  filecontents %s\n", file_contents_Users);
    parse_ValidateData_User(file_contents_Users, &myUser_newDate);

    if (myUser_newDate.permition == '1' || myUser_newDate.permition == '2') {

      if (myUser_newDate.permition == '1') {

        return return_Json_SMS_Data("ERROR_THIS_IMPOSSIBLE_CHANGE_ADMIN");
      } else {

        return return_Json_SMS_Data("ERROR_THIS_IMPOSSIBLE_CHANGE_OWNER");
      }
    }

    // ////printf("myUser_newDate.end.hour 1  %s\n", myUser_newDate.end.hour);

    if (dotCounter == 2) {

      memset(myUser_newDate.end.days, 0, sizeof(myUser_newDate.end.days));
      // endDays[2] = '\0';
      sprintf(myUser_newDate.end.days, "%s", endDays);
      // myUser_newDate.end.days[2] = '\0';
      // ////printf("\n\n change date time limit time - %s - %s \n\n", endDays,
      // myUser_newDate.end.days);
    }

    memset(myUser_newDate.start.date, 0, sizeof(myUser_newDate.start.date));

    memcpy(myUser_newDate.start.date, startDate, sizeof(startDate));

    // ////printf("myUser_newDate->start.date %s\n",
    // myUser_newDate->start.date);

    replaceUser(&myUser_newDate);

    MyUser_Search_User(phone_Number, file_contents_Users);
    // ////printf("file contents new pass %s\n", file_contents_Users);
    //   parse_ValidateData_User(file_contents_Users, &myUser_newHour);

    if (BLE_SMS_Indication == BLE_INDICATION ||
        BLE_SMS_Indication == UDP_INDICATION) {
      memset(aux_FileContens, 0, sizeof(aux_FileContens));
      erase_Password_For_Rsp(file_contents_Users, aux_FileContens);
      // asprintf(&mrsp, "%s", aux_FileContens);
      //  ////printf("file contents new date ble%s\n", file_contents_Users);
      return aux_FileContens;
    } else if (BLE_SMS_Indication == SMS_INDICATION) {
      char endDate[40] = {};

      memset(buffer, 0, sizeof(buffer));
      memset(buffer_UDP_users, 0, sizeof(buffer_UDP_users));
      erase_Password_For_Rsp(file_contents_Users, buffer);

      sprintf(mqttInfo->data, "%s %c %c %s", USER_ELEMENT, SET_CMD,
              DATE_PARAMETER, buffer);
      send_UDP_queue(mqttInfo);

      if (myUser_newDate.end.days[0] == '*') {
        sprintf(endDate, "%s", return_Json_SMS_Data("NOT_LIMIT"));
      } else {

        int limitDay = parseDatetoInt(atoi(myUser_newDate.start.date),
                                      atoi(myUser_newDate.end.days));
        char feedback_String_Date[15] = {};
        sprintf(feedback_String_Date, "%d", limitDay);
        sprintf(endDate, "%c%c/%c%c/%c%c", feedback_String_Date[0],
                feedback_String_Date[1], feedback_String_Date[2],
                feedback_String_Date[3], feedback_String_Date[4],
                feedback_String_Date[5]);
      }

      char USR_name[25] = {};

      if (strcmp(myUser_newDate.firstName, "S/N")) {
        strcpy(USR_name, myUser_newDate.firstName);
      } else {
        strcpy(USR_name, return_Json_SMS_Data("NO_NAME"));
      }

      memset(file_contents_Users, 0, sizeof(file_contents_Users));
      sprintf(&file_contents_Users,
              return_Json_SMS_Data("START_DATE_HAS_BEEN_CHANGED_TO"), USR_name,
              phone_Number, startDate[0], startDate[1], startDate[2],
              startDate[3], startDate[4], startDate[5], endDate);
      // ////printf("file contents new pass %s\n", file_contents_Users);
      //   free(myUser_newDate);
      return file_contents_Users;
    } else {
      return return_Json_SMS_Data("ERROR_INPUT_DATA");
    }

    // strcpy(mrsp,pho);
    return "OK";
  } else {

    strcpy(mrsp, return_Json_SMS_Data("ERROR_USER_NOT_FOUND"));
    return return_Json_SMS_Data("ERROR_USER_NOT_FOUND");
  }
  strcpy(mrsp, return_Json_SMS_Data("ERROR_SET"));
  return return_Json_SMS_Data("ERROR_SET");
}

char *MyUser_new_LimitTime(uint8_t BLE_SMS_INDICATION, char *payload,
                           mqtt_information *mqttInfo) {
  char phone_Number[20];
  char Limit_Time[3];

  int line;
  uint8_t dotCounter = 0;

  MyUser *myUser_newHour = malloc(sizeof(MyUser));
  memset(phone_Number, 0, sizeof(phone_Number));
  memset(Limit_Time, 0, sizeof(Limit_Time));
  memset(myUser_newHour, 0, sizeof(MyUser));
  memset(myUser_newHour->week, 0, sizeof(myUser_newHour->week));

  for (int i = 0; i < strlen(payload); i++) {
    if (payload[i] == '.') {
      dotCounter++;
    }
  }

  uint8_t aux = 0;
  uint8_t strIndex = 0;
  if (dotCounter == 1) {
    for (int i = 0; i < strlen(payload); i++) {
      if (payload[i] == '.') {
        aux++;
        strIndex = 0;
      } else if (aux == 0) {
        if (strIndex < 20) {
          phone_Number[strIndex] = payload[i];
          strIndex++;
        }
      } else if (aux == 1) {
        if (strIndex < 2) {
          Limit_Time[strIndex] = payload[i];
          strIndex++;
        } else {
          free(myUser_newHour);
          return return_Json_SMS_Data("ERROR_INPUT_DATA");
        }
      } else {
        // ////printf("erro payload 11 new date\n");
        return return_Json_SMS_Data("ERROR_INPUT_DATA");
      }
    }
  } else {
    // ////printf(ERROR_INPUT_DATA);
    return return_Json_SMS_Data("ERROR_INPUT_DATA");
  }

  if (strlen(Limit_Time) == 0) {
    free(myUser_newHour);
    return return_Json_SMS_Data("ERROR_INPUT_DATA");
  }

  // ////printf("limit time %s\n", Limit_Time);
  for (int i = 0; i < strlen(Limit_Time); i++) {
    if ((Limit_Time[i] > '9' || Limit_Time[i] < '0') ||
        strlen(Limit_Time) > 2) {
      free(myUser_newHour);
      return return_Json_SMS_Data("ERROR_INPUT_DATA");
    }
  }

  if (atoi(Limit_Time) > 90) {
    free(myUser_newHour);
    return return_Json_SMS_Data("ERROR_INPUT_DATA");
  }

  memset(file_contents_Users, 0, sizeof(file_contents_Users));

  if (MyUser_Search_User(phone_Number, file_contents_Users) == ESP_OK) {

    // ////printf("limit time  filecontents %s\n", file_contents_Users);
    parse_ValidateData_User(file_contents_Users, myUser_newHour);

    if (myUser_newHour->permition == '1' || myUser_newHour->permition == '2') {

      if (myUser_newHour->permition == '1') {
        free(myUser_newHour);
        return return_Json_SMS_Data("ERROR_THIS_IMPOSSIBLE_CHANGE_ADMIN");
      } else {
        free(myUser_newHour);
        return return_Json_SMS_Data("ERROR_THIS_IMPOSSIBLE_CHANGE_OWNER");
      }
    }

    memset(myUser_newHour->end.days, 0, strlen(myUser_newHour->end.days));
    // ////printf("myUser_newHour->end.hour %s\n", myUser_newHour->end.hour);
    char endHour[5];
    sprintf(endHour, "%s", myUser_newHour->end.hour);
    sprintf(myUser_newHour->end.days, "%s", Limit_Time);

    sprintf(myUser_newHour->end.hour, "%s", endHour);

    myUser_newHour->end.days[2] = '\0';

    replaceUser(myUser_newHour);

    MyUser_Search_User(phone_Number, file_contents_Users);
    // ////printf("file contents new pass %s\n", file_contents_Users);
    //   parse_ValidateData_User(file_contents_Users, &myUser_newHour);

    if (BLE_SMS_INDICATION == BLE_INDICATION ||
        BLE_SMS_INDICATION == UDP_INDICATION) {
      memset(aux_FileContens, 0, sizeof(aux_FileContens));
      free(myUser_newHour);
      erase_Password_For_Rsp(file_contents_Users, aux_FileContens);
      return aux_FileContens;
    } else if (BLE_SMS_INDICATION == SMS_INDICATION) {
      memset(buffer, 0, sizeof(buffer));
      memset(buffer_UDP_users, 0, sizeof(buffer_UDP_users));
      erase_Password_For_Rsp(file_contents_Users, buffer);

      sprintf(mqttInfo->data, "%s %c %c %s", USER_ELEMENT, SET_CMD,
              LAST_PARAMETER, buffer);
      send_UDP_queue(mqttInfo);

      int limitTime = parseDatetoInt(atoi(myUser_newHour->start.date),
                                     atoi(myUser_newHour->end.days));
      char feedback_String_Date[10];
      sprintf(feedback_String_Date, "%d", limitTime);

      sprintf(file_contents_Users,
              return_Json_SMS_Data("USER_WITH_LIMITED_TIME_UP_TO"),
              myUser_newHour->firstName, phone_Number, feedback_String_Date[0],
              feedback_String_Date[1], feedback_String_Date[2],
              feedback_String_Date[3], feedback_String_Date[4],
              feedback_String_Date[5]);
      free(myUser_newHour);
      // ////printf("\n\nlast time 11\n\n");
      return file_contents_Users;
    } else {
      free(myUser_newHour);
      return return_Json_SMS_Data("ERROR_INPUT_DATA");
    }
  } else {
    free(myUser_newHour);
    return return_Json_SMS_Data("ERROR_INPUT_DATA");
  }
}

char *MyUser_reset_LimitTime(uint8_t BLE_SMS_Indication, char *payload,
                             mqtt_information *mqttInfo) {
  char phone_Number[20];

  int line = 0;
  uint8_t dotCounter = 0;

  MyUser *myUser_newHour = malloc(sizeof(MyUser));
  memset(phone_Number, 0, sizeof(phone_Number));
  memset(myUser_newHour, 0, sizeof(MyUser));
  memset(myUser_newHour->week, 0, sizeof(myUser_newHour->week));

  for (int i = 0; i < strlen(payload); i++) {
    if (i < 20) {
      phone_Number[i] = payload[i];
    } else {
      // ////printf(ERROR_INPUT_DATA);
      return return_Json_SMS_Data("ERROR_WRONG_PHONE_NUMBER");
    }
  }

  if (MyUser_Search_User(phone_Number, file_contents_Users) == ESP_OK) {
    // ////printf("limit time  filecontents %s\n", file_contents_Users);
    parse_ValidateData_User(file_contents_Users, myUser_newHour);

    if (myUser_newHour->permition == '1' || myUser_newHour->permition == '2') {

      if (myUser_newHour->permition == '1') {
        free(myUser_newHour);
        return return_Json_SMS_Data("ERROR_THIS_IMPOSSIBLE_CHANGE_ADMIN");
      } else {
        free(myUser_newHour);
        return return_Json_SMS_Data("ERROR_THIS_IMPOSSIBLE_CHANGE_OWNER");
      }
    }
    // ////printf("myUser_newDate limit days  %s\n", myUser_newHour->end.days);

    memset(myUser_newHour->end.days, 0, sizeof(myUser_newHour->end.days));

    myUser_newHour->end.days[0] = '*';
    myUser_newHour->end.days[1] = 0;

    // ////printf("myUser_newHour->end.days %s\n", myUser_newHour->end.days);

    replaceUser(myUser_newHour);

    if (BLE_SMS_Indication == BLE_INDICATION ||
        BLE_SMS_Indication == UDP_INDICATION) {
      MyUser_Search_User(phone_Number, file_contents_Users);
      free(myUser_newHour);
      memset(aux_FileContens, 0, sizeof(aux_FileContens));
      erase_Password_For_Rsp(file_contents_Users, aux_FileContens);

      return aux_FileContens;
    } else if (BLE_SMS_Indication == SMS_INDICATION) {
      MyUser_Search_User(phone_Number, aux_FileContens);

      memset(buffer, 0, sizeof(buffer));
      memset(buffer_UDP_users, 0, sizeof(buffer_UDP_users));
      erase_Password_For_Rsp(aux_FileContens, buffer);
      sprintf(mqttInfo->data, "%s %c %c %s", USER_ELEMENT, RESET_CMD,
              LAST_PARAMETER, buffer);
      send_UDP_queue(mqttInfo);

      sprintf(file_contents_Users,
              return_Json_SMS_Data("THE_END_DATE_HAS_BEEN_DISABLED"),
              phone_Number);
      free(myUser_newHour);

      return file_contents_Users;
    } else {
      return return_Json_SMS_Data("ERROR_INPUT_DATA");
    }
  } else {
    free(myUser_newHour);
    return return_Json_SMS_Data("ERROR_USER_NOT_FOUND");
  }
}

char *MyUser_get_startDate(uint8_t BLE_SMS_Indication, char *payload) {
  char phone_Number[20];

  int line = 0;
  uint8_t dotCounter = 0;

  MyUser myUser_newDate;
  memset(myUser_newDate.firstName, 0, sizeof(myUser_newDate.firstName));
  memset(phone_Number, 0, sizeof(phone_Number));
  memset(myUser_newDate.week, 0, sizeof(myUser_newDate.week));
  memset(file_contents_Users, 0, sizeof(file_contents_Users));

  for (int i = 0; i < strlen(payload); i++) {
    if (i < 20) {
      phone_Number[i] = payload[i];
    } else {
      // ////printf(ERROR_INPUT_DATA);
      return return_Json_SMS_Data("ERROR_INPUT_DATA");
    }
  }

  if (MyUser_Search_User(phone_Number, file_contents_Users) == ESP_OK) {
    memset(myUser_newDate.start.date, 0, sizeof(myUser_newDate.start.date));
    // ////printf("limit time  filecontents %s\n", file_contents_Users);
    parse_ValidateData_User(file_contents_Users, &myUser_newDate);

    if (myUser_newDate.permition == '1' || myUser_newDate.permition == '2') {

      if (myUser_newDate.permition == '1') {
        return return_Json_SMS_Data("ERROR_THIS_IMPOSSIBLE_CHANGE_ADMIN");
      } else {
        return return_Json_SMS_Data("ERROR_THIS_IMPOSSIBLE_CHANGE_OWNER");
      }
    }

    if (BLE_SMS_Indication == BLE_INDICATION) {

      // ////printf("myUser_get date  %s\n", myUser_newDate.start.date);
      sprintf(file_contents_Users, "%s", myUser_newDate.start.date);

      return file_contents_Users;
    } else if (BLE_SMS_Indication == SMS_INDICATION) {

      sprintf(file_contents_Users, return_Json_SMS_Data("USER_START_DATE"),
              myUser_newDate.firstName, phone_Number,
              myUser_newDate.start.date[0], myUser_newDate.start.date[1],
              myUser_newDate.start.date[2], myUser_newDate.start.date[3],
              myUser_newDate.start.date[4], myUser_newDate.start.date[5]);

      return file_contents_Users;
    } else {
      return return_Json_SMS_Data("ERROR_INPUT_DATA");
    }
  } else {

    return return_Json_SMS_Data("ERROR_USER_NOT_FOUND");
  }
}

char *MyUser_get_LimitTime(uint8_t BLE_SMS_Indication, char *payload,
                           char *mrsp) {
  char phone_Number[20];

  int line = 0;
  uint8_t dotCounter = 0;

  MyUser *myUser_newHour = malloc(sizeof(MyUser));
  memset(phone_Number, 0, sizeof(phone_Number));
  memset(myUser_newHour->week, 0, sizeof(myUser_newHour->week));
  memset(myUser_newHour->end.days, 0, sizeof(myUser_newHour->end.days));
  memset(myUser_newHour->start.date, 0, sizeof(myUser_newHour->start.date));
  memset(myUser_newHour->firstName, 0, sizeof(myUser_newHour->firstName));
  memset(file_contents_Users, 0, sizeof(file_contents_Users));

  for (int i = 0; i < strlen(payload); i++) {
    if (i < 20) {
      phone_Number[i] = payload[i];
    } else {
      // ////printf(ERROR_INPUT_DATA);
      sprintf(mrsp, "%s", return_Json_SMS_Data("ERROR_INPUT_DATA"));
      return mrsp;
    }
  }

  if (MyUser_Search_User(phone_Number, file_contents_Users) == ESP_OK) {
    // ////printf("limit time  filecontents %s\n", file_contents_Users);
    parse_ValidateData_User(file_contents_Users, myUser_newHour);

    if (myUser_newHour->permition == '1' || myUser_newHour->permition == '2') {

      if (myUser_newHour->permition == '1') {
        sprintf(mrsp,
                return_Json_SMS_Data("ERROR_THIS_IMPOSSIBLE_CHANGE_ADMIN"));
      } else {
        sprintf(mrsp,
                return_Json_SMS_Data("ERROR_THIS_IMPOSSIBLE_CHANGE_OWNER"));
      }

      free(myUser_newHour);
      return file_contents_Users;
    }

    if (myUser_newHour->end.days[0] == '*') {
      if (BLE_SMS_Indication == BLE_INDICATION) {
        sprintf(mrsp, "%s", file_contents_Users);
      } else {
        sprintf(mrsp, return_Json_SMS_Data("USER_WITH_NOT_LIMIT_TIME"));
      }
    } else {
      if (BLE_SMS_Indication == BLE_INDICATION) {
        sprintf(mrsp, "%s", file_contents_Users);
      } else {
        char auxName[250] = {};
        int limitDay = parseDatetoInt(atoi(myUser_newHour->start.date),
                                      atoi(myUser_newHour->end.days));
        char feedback_String_Date[10];
        sprintf(feedback_String_Date, "%d", limitDay);
        // ////printf("\nstruct date - %d , struct day - %d\n",
        // atoi(myUser_newHour->start.date), atoi(myUser_newHour->end.days));
        // ////printf("\n limit day %d\n", limitDay);

        if (!strcmp(myUser_newHour->firstName, "S/N")) {
          sprintf(auxName, "%s", return_Json_SMS_Data("NO_NAME"));
        } else {
          sprintf(auxName, "%s", myUser_newHour->firstName);
        }

        sprintf(mrsp, return_Json_SMS_Data("USER_LIMIT_TIME"), auxName,
                phone_Number, myUser_newHour->start.date[0],
                myUser_newHour->start.date[1], myUser_newHour->start.date[2],
                myUser_newHour->start.date[3], myUser_newHour->start.date[4],
                myUser_newHour->start.date[5], feedback_String_Date[0],
                feedback_String_Date[1], feedback_String_Date[2],
                feedback_String_Date[3], feedback_String_Date[4],
                feedback_String_Date[5], myUser_newHour->start.hour[0],
                myUser_newHour->start.hour[1], myUser_newHour->start.hour[2],
                myUser_newHour->start.hour[3], myUser_newHour->end.hour[0],
                myUser_newHour->end.hour[1], myUser_newHour->end.hour[2],
                myUser_newHour->end.hour[3]);
      }
    }

    free(myUser_newHour);
    return file_contents_Users;
  } else {
    free(myUser_newHour);
    sprintf(mrsp, "%s", return_Json_SMS_Data("ERROR_USER_NOT_FOUND"));
    return return_Json_SMS_Data("ERROR_USER_NOT_FOUND");
  }
  return return_Json_SMS_Data("ERROR_INPUT_DATA");
}

char *MyUser_ReadAllUsers(uint8_t gattsIF, uint16_t connID,
                          uint16_t handle_table, char userPermition,
                          uint8_t BLE_UDP_Indication) {
  allUsers_parameters_Message message = {
      .gattsIF = gattsIF,
      .connID = connID,
      .handle_table = handle_table,
      .usr_perm = userPermition,
      .BLE_UDP_Indication = BLE_UDP_Indication,
  };

  readAllUser_ConnID = 0;

  readAllUser_Label = 1;

  readAllUser_ConnID = message.connID;
  xTaskCreate(ReadALLUsers_task, "ReadALLUsers_task", 6000, (void *)&message,
              NULL, NULL);
  vTaskDelay(pdMS_TO_TICKS((100)));

  return "NTRSP";
}

uint8_t sendUDP_all_User_funtion() {
  allUsers_parameters_Message cpy_message = {
      .connID = 0,
      .gattsIF = 0,
      .handle_table = 0,
      .usr_perm = '2',
      .BLE_UDP_Indication = UDP_INDICATION,
  };

  char aux_data[509] = {};

  char buffer[75];
  // char ch[512];
  char *ch = (char *)malloc(512 * sizeof(char));
  char ch1[75];
  char ch2[75];
  char ch3[75];
  memset(buffer, 0, 75);
  memset(ch, 0, 512);
  char temp[512]; // Buffer temporário para concatenar as strings
  nvs_entry_info_t info;
  uint8_t UDP_Users_OK = 0;
  uint8_t label_usersUDP_send = 0;

  int count = 0;
  // ////printf("\nLIST USERS:\n");
  nvs_iterator_t it =
      NULL; //= nvs_entry_find("keys", NVS_USERS_NAMESPACE, NVS_TYPE_STR);
  char value[200];
  // ////printf("Iterate NVS\n");
  int64_t start_time = esp_timer_get_time();
  int file_senddata_Index = 1;

  if (!send_UDP_Send((char *)("*"), "")) {
    // ////printf("\n\n\n send udp ççççç \n\n\n");
    return 0;
  }

  vTaskDelay(pdMS_TO_TICKS(500));
  // ////printf("\n\n\n send udp 44llll44 \n\n\n");
  for (int i = 3; i > 0; i--) {
    // ////printf("\n\n\n send udp 4444 \n\n\n");

    if (i - 1 == 1) {

      // //printf("\n\n\n send udp 1234 admin \n\n\n");
      it = nvs_entry_find("keys", NVS_ADMIN_NAMESPACE, NVS_TYPE_STR);

      /* else
      {
          break;
      } */
    } else if (i - 1 == 2) {

      // ////printf("\n\n\n send udp 4566 \n\n\n");
      it = nvs_entry_find("keys", NVS_OWNER_NAMESPACE, NVS_TYPE_STR);
    } else if (i - 1 == 0) {
      // ////printf("\n NVS_USERS_NAMESPACE 1 \n");

      // ////printf("\n NVS_USERS_NAMESPACE 2 \n");
      it = nvs_entry_find("keys", NVS_USERS_NAMESPACE, NVS_TYPE_STR);
    }

    while (it != NULL) {

      nvs_entry_info(it, &info);
      it = nvs_entry_next(it);
      // ////printf("key '%s', type '%d' \n", info.key, info.type);
      get_Data_Users_From_Storage(info.key, &value);

      if (cpy_message.BLE_UDP_Indication == UDP_INDICATION) {

        if (file_senddata_Index == 1) {
          // ////printf("\n\n send udp index = 1\n\n");
          sprintf(ch, "%s", "UR.G.* ");
          erase_Password_For_Rsp(value, aux_data);
          // ////printf("\n\n send udp index = 2 - %s\n\n", aux_data);
          sprintf(ch + strlen(ch), "%s", aux_data);
          sprintf(ch + strlen(ch), "%s", "\n\0");
          file_senddata_Index++;
          memset(value, 0, 200);
          memset(aux_data, 0, 200);
        } else if (file_senddata_Index > 1 && file_senddata_Index <= 5) {
          erase_Password_For_Rsp(value, aux_data);

          sprintf(temp, "%s", ch);
          sprintf(temp + strlen(temp), "%s", aux_data);
          sprintf(temp + strlen(temp), "%s", "\n\0");
          // ////printf("\n\n send udp index = 3 - %s\n\n", aux_data);
          //  ////printf("\n\n send udp jjjjj - %s\n\n", temp);
          memset(value, 0, 200);
          memset(aux_data, 0, 509);

          if (file_senddata_Index == 5) {
            // ////printf("\n\n send udp all users\n\n");
            if (strlen(temp) > 0) {
              file_senddata_Index = 1;
              if (!send_UDP_Send(temp, "")) {
                free(it);
                free(ch);
                return 0;
              }
            }

            // send_UDP_Package(temp, strlen(temp));
          } else {
            file_senddata_Index++;
          }

          memset(ch, 0, 512);
          strcpy(ch, temp); // Copiar o conteúdo concatenado para ch
          // ////printf("\n\n send udp ch - %s\n\n", ch);
          memset(temp, 0, 512); // Limpar o buffer temporário
          // ESP_LOGI("TAG", "xPortGetFreeHeapSize            : %d",
          // xPortGetFreeHeapSize()); ESP_LOGI("TAG",
          // "esp_get_minimum_free_heap_size  : %d",
          // esp_get_minimum_free_heap_size()); ESP_LOGI("TAG",
          // "heap_caps_get_largest_free_block: %d",
          // heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
          // ESP_LOGI("TAG", "free heap memory                : %d",
          // heap_caps_get_free_size(MALLOC_CAP_8BIT));
          //  vTaskDelay(pdMS_TO_TICKS((30)));

          // ////printf("filee %s\n", ch);
        }
      }
    }

    if (cpy_message.BLE_UDP_Indication == UDP_INDICATION) {
      if (strlen(ch) > 0 && file_senddata_Index > 1) {
        // ////printf("filee tmp 123 %s\n", ch);
        if (!send_UDP_Send(ch, "")) {
          free(it);
          free(ch);
          return 0;
        }
      }
    }
    file_senddata_Index = 1;
    memset(ch, 0, 512);
  }

  free(it);
  free(ch);

  readAllUser_Label = 0;
  return 1;
}

void ReadALLUsers_task(void *pvParameter) {
  allUsers_parameters_Message *message =
      (allUsers_parameters_Message *)pvParameter;

  allUsers_parameters_Message cpy_message = {
      .connID = message->connID,
      .gattsIF = message->gattsIF,
      .handle_table = message->handle_table,
      .usr_perm = message->usr_perm,
      .BLE_UDP_Indication = message->BLE_UDP_Indication,
  };

  char aux_data[509] = {};

  char buffer[75];
  // char ch[512];
  char *ch = (char *)malloc(512 * sizeof(char));
  char ch1[75];
  char ch2[75];
  char ch3[75];
  memset(buffer, 0, 75);
  memset(ch, 0, 512);
  char temp[512]; // Buffer temporário para concatenar as strings
  nvs_entry_info_t info;
  uint8_t UDP_Users_OK = 0;
  uint8_t label_usersUDP_send = 0;

  int count = 0;
  // ////printf("\nLIST USERS:\n");
  nvs_iterator_t it =
      NULL; //= nvs_entry_find("keys", NVS_USERS_NAMESPACE, NVS_TYPE_STR);
  char value[200];
  // ////printf("Iterate NVS\n");
  int64_t start_time = esp_timer_get_time();
  int file_senddata_Index = 1;

  for (int i = 3; i > 0; i--) {
    if (i - 1 == 1) {
      if (((i - 1) + 48) < cpy_message.usr_perm) {
        it = nvs_entry_find("keys", NVS_ADMIN_NAMESPACE, NVS_TYPE_STR);
      }
      /* else
      {
          break;
      } */
    } else if (i - 1 == 2) {
      if (((i - 1) + 48) == cpy_message.usr_perm) {
        it = nvs_entry_find("keys", NVS_OWNER_NAMESPACE, NVS_TYPE_STR);
      }
      /* else
      {
          break;
      } */
    } else if (i - 1 == 0) {
      // ////printf("\n NVS_USERS_NAMESPACE 1 \n");
      if (((i - 1) + 48) < cpy_message.usr_perm) {
        // ////printf("\n NVS_USERS_NAMESPACE 2 \n");
        it = nvs_entry_find("keys", NVS_USERS_NAMESPACE, NVS_TYPE_STR);
      }
      /*  else
       {
           break;
       } */
    }

    while (it != NULL) {

      nvs_entry_info(it, &info);
      it = nvs_entry_next(it);
      printf("key '%s', type '%d' \n", info.key, info.type);
      get_Data_Users_From_Storage(info.key, &value);

      if (cpy_message.BLE_UDP_Indication == UDP_INDICATION) {

        if (file_senddata_Index == 1) {
          // ////printf("\n\n send udp index = 1\n\n");
          sprintf(ch, "%s", "UR.G.* ");
          erase_Password_For_Rsp(value, aux_data);
          // ////printf("\n\n send udp index = 2 - %s\n\n", aux_data);
          sprintf(ch + strlen(ch), "%s", aux_data);
          sprintf(ch + strlen(ch), "%s", "\n\0");
          file_senddata_Index++;
          memset(value, 0, 200);
          memset(aux_data, 0, 509);
        } else if (file_senddata_Index > 1 && file_senddata_Index <= 5) {
          erase_Password_For_Rsp(value, aux_data);

          sprintf(temp, "%s", ch);
          sprintf(temp + strlen(temp), "%s", aux_data);
          sprintf(temp + strlen(temp), "%s", "\n\0");
          // ////printf("\n\n send udp index = 3 - %s\n\n", aux_data);
          // ////printf("\n\n send udp jjjjj - %s\n\n", temp);
          memset(value, 0, 200);
          memset(aux_data, 0, 509);

          if (file_senddata_Index == 5) {
            // ////printf("\n\n send udp all users\n\n");
            if (strlen(temp) > 0) {
              file_senddata_Index = 1;
              if (!send_UDP_Send(temp, "")) {
                label_BLE_UDP_send = 1;
                break;
              }
            }

            // send_UDP_Package(temp, strlen(temp));
          } else {
            file_senddata_Index++;
          }

          memset(ch, 0, 512);
          strcpy(ch, temp); // Copiar o conteúdo concatenado para ch
          // ////printf("\n\n send udp ch - %s\n\n", ch);
          memset(temp, 0, 512); // Limpar o buffer temporário
          // ESP_LOGI("TAG", "xPortGetFreeHeapSize            : %d",
          // xPortGetFreeHeapSize()); ESP_LOGI("TAG",
          // "esp_get_minimum_free_heap_size  : %d",
          // esp_get_minimum_free_heap_size()); ESP_LOGI("TAG",
          // "heap_caps_get_largest_free_block: %d",
          // heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
          // ESP_LOGI("TAG", "free heap memory                : %d",
          // heap_caps_get_free_size(MALLOC_CAP_8BIT));
          //  vTaskDelay(pdMS_TO_TICKS((30)));

          // ////printf("filee %s\n", ch);
        }
      } else {

        if (file_senddata_Index == 1) {
          sprintf(ch, "%s", "UR.G.* ");
          erase_Password_For_Rsp(value, aux_data);
          strcat(ch, aux_data);
          strcat(ch, "\n\0");
          file_senddata_Index++;
          memset(value, 0, 200);
          memset(aux_data, 0, 200);
        } else if (file_senddata_Index == 2) {
          erase_Password_For_Rsp(value, aux_data);
          strcat(ch, aux_data);
          strcat(ch, "\n\0");
          file_senddata_Index = 1;
          memset(value, 0, 200);
          memset(aux_data, 0, 200);

          if (esp_ble_gatts_send_indicate(cpy_message.gattsIF,
                                          cpy_message.connID,
                                          cpy_message.handle_table, strlen(ch),
                                          (uint8_t *)ch, true) != ESP_OK) {
            break;
          }

          // ////printf("%s", ch);

          memset(ch, 0, 512);
          // ESP_LOGI("TAG", "xPortGetFreeHeapSize            : %d",
          // xPortGetFreeHeapSize()); ESP_LOGI("TAG",
          // "esp_get_minimum_free_heap_size  : %d",
          // esp_get_minimum_free_heap_size()); ESP_LOGI("TAG",
          // "heap_caps_get_largest_free_block: %d",
          // heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
          // ESP_LOGI("TAG", "free heap memory                : %d",
          // heap_caps_get_free_size(MALLOC_CAP_8BIT));
          //  vTaskDelay(pdMS_TO_TICKS((30)));
          xSemaphoreTake(rdySem,
                         pdMS_TO_TICKS(3500)); // Wait until slave is ready
                                               // ////printf("filee %s\n", ch);
        }
        // ////printf("\nvalue: %s\n", value);
        vTaskDelay(pdMS_TO_TICKS(50));
      }
    }

    // ////printf("filee tmp %s\n", ch);
    // ////printf("filee file_senddata_Index %d\n", file_senddata_Index);

    if (cpy_message.BLE_UDP_Indication == UDP_INDICATION) {
      if (strlen(ch) > 0 &&
          file_senddata_Index > 1 /* && file_senddata_Index < 5 */) {
        // ////printf("filee tmp 123 %s\n", ch);
        if (!send_UDP_Send(ch, "")) {
          label_BLE_UDP_send = 1;
        }
        // send_UDP_queue(ch);
        // vTaskDelay(pdMS_TO_TICKS(50));
      }

      // send_UDP_Package(ch, strlen(ch));
    } else {
      strcat(ch, buffer);

      memset(buffer, 0, 75);
      esp_ble_gatts_send_indicate(cpy_message.gattsIF, cpy_message.connID,
                                  cpy_message.handle_table, strlen(ch),
                                  (uint8_t *)ch, true);
    }

    file_senddata_Index = 1;

    // ////printf("%s", ch);

    memset(ch, 0, 200);
    // vTaskDelay(pdMS_TO_TICKS((30)));
    if (cpy_message.BLE_UDP_Indication == BLE_INDICATION) {
      xSemaphoreTake(rdySem, pdMS_TO_TICKS(3500)); // Wait until slave is ready
    }

    /* if (i==0)
    {
        break;
    } */
  }
  free(ch);
  free(it);

  if (label_BLE_UDP_send == 1) {
    uint8_t label_UDP_fail_and_changed = get_INT8_Data_From_Storage(
        NVS_NETWORK_LOCAL_CHANGED, nvs_System_handle);
    label_UDP_fail_and_changed |= 2;
    save_INT8_Data_In_Storage(NVS_NETWORK_LOCAL_CHANGED,
                              label_UDP_fail_and_changed, nvs_System_handle);
  }

  readAllUser_Label = 0;

  vTaskDelete(NULL);
}

char *MyUser_Set_Relay_Restrition(MyUser *user_validateData,
                                  uint8_t BLE_SMS_Indication, char *payload,
                                  mqtt_information *mqttInfo) {

  char phone_Number[20] = {};
  char permition = 0;
  char dataContents[200] = {};

  int line;
  uint8_t dotCounter = 0;
  MyUser *myUser_new_Relay_Restriction = malloc(sizeof(MyUser));
  memset(myUser_new_Relay_Restriction, 0, sizeof(MyUser));

  for (int i = 0; i < strlen(payload); i++) {
    if (payload[i] == '.') {
      dotCounter++;
    }
  }

  uint8_t aux = 0;
  uint8_t strIndex = 0;
  if (dotCounter == 1) {
    for (int i = 0; i < strlen(payload); i++) {
      if (payload[i] == '.') {
        aux++;
        strIndex = 0;
      } else if (aux == 0) {
        if (strIndex < 20) {
          phone_Number[strIndex] = payload[i];
          strIndex++;
        }
      } else if (aux == 1) {
        if (strIndex >= 1) {
          // ////printf(ERROR_INPUT_DATA);
          free(myUser_new_Relay_Restriction);
          return return_Json_SMS_Data("ERROR_INPUT_DATA");
        }
        permition = payload[i];
        strIndex++;
      } else {
        // ////printf(ERROR_INPUT_DATA);
        free(myUser_new_Relay_Restriction);
        return return_Json_SMS_Data("ERROR_INPUT_DATA");
      }
    }
  } else {
    // ////printf(ERROR_INPUT_DATA);
    free(myUser_new_Relay_Restriction);
    return return_Json_SMS_Data("ERROR_INPUT_DATA");
  }

  // ////printf("\n\nMyUser_Set_Relay_Restrition phone %s\n\n", phone_Number);

  if (MyUser_Search_User(phone_Number, dataContents) == ESP_OK) {
    parse_ValidateData_User(dataContents, myUser_new_Relay_Restriction);

    if (myUser_new_Relay_Restriction->permition == '1' ||
        myUser_new_Relay_Restriction->permition == '2') {

      if (myUser_new_Relay_Restriction->permition == '1') {
        free(myUser_new_Relay_Restriction);
        return return_Json_SMS_Data("ERROR_THIS_IMPOSSIBLE_CHANGE_ADMIN");
      } else {
        free(myUser_new_Relay_Restriction);
        return return_Json_SMS_Data("ERROR_THIS_IMPOSSIBLE_CHANGE_OWNER");
      }
    }

    if (permition < '0' || permition > '2') {
      free(myUser_new_Relay_Restriction);
      return return_Json_SMS_Data("ERROR_INPUT_DATA");
    } else {
      myUser_new_Relay_Restriction->relayPermition = permition;
      // ////printf("CHANGE RELAY RECTRITION 1\n");
      if (replaceUser(myUser_new_Relay_Restriction) == ESP_OK) {
        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {
          memset(&dataContents, 0, sizeof(dataContents));
          if (MyUser_Search_User(myUser_new_Relay_Restriction->phone,
                                 dataContents) == ESP_OK) {
            parse_ValidateData_User(dataContents, myUser_new_Relay_Restriction);
            memset(&file_contents_Users, 0, sizeof(file_contents_Users));
            erase_Password_For_Rsp(dataContents, file_contents_Users);
            // sprintf(&file_contents_Users, "UR S R %s", dataContents);
            free(myUser_new_Relay_Restriction);
            return erase_Password_For_Rsp(
                dataContents, file_contents_Users); // file_contents_Users;
          }
        } else if (BLE_SMS_Indication == SMS_INDICATION) {

          MyUser_Search_User(phone_Number, aux_FileContens);

          memset(buffer, 0, sizeof(buffer));
          memset(buffer_UDP_users, 0, sizeof(buffer_UDP_users));
          erase_Password_For_Rsp(aux_FileContens, buffer);

          sprintf(mqttInfo->data, "%s %c %c %s", USER_ELEMENT, SET_CMD,
                  RELE_RESTRITION_PARAMETER, buffer);
          send_UDP_queue(mqttInfo);

          if (myUser_new_Relay_Restriction->relayPermition == '0') {
            memset(&file_contents_Users, 0, sizeof(file_contents_Users));
            sprintf(
                &file_contents_Users,
                return_Json_SMS_Data("CHANGED_TO_NO_RESTRICTIONS_ON_RELAYS"),
                myUser_new_Relay_Restriction->phone);
            free(myUser_new_Relay_Restriction);
            return file_contents_Users;
          } else if (myUser_new_Relay_Restriction->relayPermition == '1') {
            memset(&file_contents_Users, 0, sizeof(file_contents_Users));
            sprintf(
                &file_contents_Users,
                return_Json_SMS_Data("CHANGED_TO_HAVE_RESTRICTION_ON_RELAY_1"),
                myUser_new_Relay_Restriction->phone);
            free(myUser_new_Relay_Restriction);
            return file_contents_Users;
          } else if (myUser_new_Relay_Restriction->relayPermition == '2') {
            memset(&file_contents_Users, 0, sizeof(file_contents_Users));
            sprintf(
                &file_contents_Users,
                return_Json_SMS_Data("CHANGED_TO_HAVE_RESTRICTION_ON_RELAY_2"),
                myUser_new_Relay_Restriction->phone);
            free(myUser_new_Relay_Restriction);
            return file_contents_Users;
          } else {
            free(myUser_new_Relay_Restriction);
            return return_Json_SMS_Data("ERROR_INPUT_DATA");
          }
        }
      } else {
        if (BLE_SMS_Indication == BLE_INDICATION) {
          memset(&file_contents_Users, 0, sizeof(file_contents_Users));
          sprintf(&file_contents_Users, "UR S R %s", ERROR_RESET);
          free(myUser_new_Relay_Restriction);
          return file_contents_Users;
        } else if (BLE_SMS_Indication == SMS_INDICATION) {
          memset(&file_contents_Users, 0, sizeof(file_contents_Users));
          sprintf(&file_contents_Users,
                  return_Json_SMS_Data("ERRO_USER_HAS_NOT_BEEN_CHANGED"));
          free(myUser_new_Relay_Restriction);
          return file_contents_Users;
        } else {
          free(myUser_new_Relay_Restriction);
          return return_Json_SMS_Data("ERRO_USER_HAS_NOT_BEEN_CHANGED");
        }
      }
    }
  } else {
    free(myUser_new_Relay_Restriction);
    return return_Json_SMS_Data("ERROR_USER_NOT_FOUND");
  }

  free(myUser_new_Relay_Restriction);
  return return_Json_SMS_Data("ERROR_INPUT_DATA");
}

/* uint8_t check_IF_Exist_IN_Other_NVS(char *key)
{
} */