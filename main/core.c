/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|

*/

#include "core.h"
#include "ble_spp_server_demo.h"
#include "cmd_list.h"
#include "erro_list.h"
#include "esp_gatts_api.h"
#include "esp_heap_trace.h"
#include "esp_log.h"
#include "gpio.h"
#include "nvs.h"
#include "pcf85063.h"
#include "rele.h"
#include "routines.h"
#include "sdCard.h"
// #include "rele.h"
#include "UDP_Codes.h"
#include "crc32.h"
#include "mbedtls/aes.h"
#include "system.h"
#include "wiegand.h"
#include <string.h>

char rsp[200];
char sdCard_log[100];
char aux_phNumber[20];

#define NUMBER_OF_STRING 277
#define MAX_STRING_SIZE 6

uint8_t label_MonoStableRelay1;
uint8_t label_MonoStableRelay2;
uint8_t label_Reset_Password_OR_System;
uint8_t label_initSystem_SIMPRE;
uint8_t label_initSystem_CALL;
uint32_t date_To_Send_Periodic_SMS;
uint8_t label_To_Send_Periodic_SMS;
uint8_t network_Activate_Flag;

TimerHandle_t xTimers;

QueueHandle_t queue_EG91_SendSMS;

NOW_TIME nowTime;

char jsonFile[] asm("_binary_pt_json_start");

cJSON *sms_Rsp_Json;
cJSON *default_sms_Rsp_Json;

// const unsigned char IV[] = AES_IV;
const unsigned char KEY_AES[] = AES_KEY1;
// const size_t IV_SIZE = sizeof(IV) - 1;

char countryCodes[NUMBER_OF_STRING][MAX_STRING_SIZE] = {

    {"1"},     {"93"},    {"355"},   {"213"},   {"1-684"}, {"376"},   {"244"},
    {"1-264"}, {"672"},   {"1-268"}, {"54"},    {"374"},   {"297"},   {"247"},
    {"61"},    {"672"},   {"43"},    {"994"},   {"1-242"}, {"973"},   {"880"},
    {"1-246"}, {"1-268"}, {"375"},   {"32"},    {"501"},   {"229"},   {"1-441"},
    {"975"},   {"591"},   {"387"},   {"267"},   {"55"},    {"1-284"}, {"673"},
    {"359"},   {"226"},   {"257"},   {"855"},   {"237"},   {"238"},   {"1-345"},
    {"236"},   {"235"},   {"64"},    {"56"},    {"86"},    {"61-8"},  {"61"},
    {"57"},    {"269"},   {"242"},   {"243"},   {"682"},   {"506"},   {"225"},
    {"385"},   {"53"},    {"5399"},  {"599"},   {"357"},   {"420"},   {"45"},
    {"246"},   {"253"},   {"1-767"}, {"1-809"}, {"1-829"}, {"670"},   {"56"},
    {"593"},   {"20"},    {"503"},   {"8812"},  {"8813"},  {"88213"}, {"240"},
    {"291"},   {"372"},   {"251"},   {"500"},   {"298"},   {"679"},   {"358"},
    {"33"},    {"596"},   {"594"},   {"689"},   {"241"},   {"220"},   {"995"},
    {"49"},    {"233"},   {"350"},   {"881"},   {"8810"},  {"8811"},  {"8812"},
    {"8813"},  {"8816"},  {"8817"},  {"8818"},  {"8819"},  {"8818"},  {"8819"},
    {"30"},    {"299"},   {"1-473"}, {"590"},   {"1-671"}, {"5399"},  {"502"},
    {"245"},   {"224"},   {"592"},   {"509"},   {"504"},   {"852"},   {"36"},
    {"8810"},  {"8811"},  {"354"},   {"91"},    {"62"},    {"871"},   {"874"},
    {"873"},   {"872"},   {"870"},   {"800"},   {"808"},   {"98"},    {"964"},
    {"353"},   {"8816"},  {"8817"},  {"972"},   {"39"},    {"225"},   {"1-876"},
    {"81"},    {"962"},   {"7"},     {"254"},   {"686"},   {"850"},   {"82"},
    {"965"},   {"996"},   {"856"},   {"371"},   {"961"},   {"266"},   {"231"},
    {"218"},   {"423"},   {"370"},   {"352"},   {"853"},   {"389"},   {"261"},
    {"265"},   {"60"},    {"960"},   {"223"},   {"356"},   {"692"},   {"596"},
    {"222"},   {"230"},   {"269"},   {"52"},    {"691"},   {"1-808"}, {"373"},
    {"377"},   {"976"},   {"382"},   {"1-664"}, {"212"},   {"258"},   {"95"},
    {"264"},   {"674"},   {"977"},   {"31"},    {"599"},   {"1-869"}, {"687"},
    {"64"},    {"505"},   {"227"},   {"234"},   {"683"},   {"672"},   {"1-670"},
    {"47"},    {"968"},   {"92"},    {"680"},   {"970"},   {"507"},   {"675"},
    {"595"},   {"51"},    {"63"},    {"48"},    {"351"},   {"1-787"}, {"1-939"},
    {"974"},   {"262"},   {"40"},    {"7"},     {"250"},   {"290"},   {"1-869"},
    {"1-758"}, {"508"},   {"1-784"}, {"685"},   {"378"},   {"239"},   {"966"},
    {"221"},   {"381"},   {"248"},   {"232"},   {"65"},    {"421"},   {"386"},
    {"677"},   {"252"},   {"27"},    {"34"},    {"94"},    {"249"},   {"597"},
    {"268"},   {"46"},    {"41"},    {"963"},   {"886"},   {"992"},   {"255"},
    {"66"},    {"88216"}, {"670"},   {"228"},   {"690"},   {"676"},   {"1-868"},
    {"216"},   {"90"},    {"993"},   {"1-649"}, {"688"},   {"256"},   {"380"},
    {"971"},   {"44"},    {"1-340"}, {"878"},   {"598"},   {"998"},   {"678"},
    {"393"},   {"58"},    {"84"},    {"808"},   {"681"},   {"967"},   {"260"},
    {"255"},   {"263"}

};

uint8_t parse_CountryPhoneNumber(char *data) {
  char auxData[20];
  sprintf(auxData, "%s", data);
  for (int i = 0; i < NUMBER_OF_STRING; i++) {
    if (strcmp(auxData, countryCodes[i]) == 0) {
      // ////printf("Found in row-%d\n", i + 1);
      return 1;
    }
  }

  return 0;
}

char *check_zeroInNumber(char *number) {
  int8_t ACK = 0;
  char PH_Number[20] = {};
  char auxSTR[200] = {};

  ACK = get_Data_Users_From_Storage(number, &auxSTR);

  if (ACK != ESP_OK) {
    if ((number[0] == '0') && number[1] != '0') {

      // ////printf("\n  payload phnumber sms 33 %s \n", aux_phNumber);
      for (size_t i = 1; i < strlen(number); i++) {
        number[i - 1] = number[i];
      }

      number[strlen(number) - 1] = 0;
    } else {
      PH_Number[0] = '0';

      for (size_t i = 0; i < strlen(number); i++) {
        PH_Number[i + 1] = number[i];
      }

      ACK = get_Data_Users_From_Storage(PH_Number, &auxSTR);

      if (ACK == ESP_OK) {
        // memset(number, 0, sizeof(number));
        sprintf(number, "%s", PH_Number);
        return number;
      }
    }
  }

  return number;
}

char *check_IF_haveCountryCode_AUX_Call(char *phoneNumber) {
  int CountryCode_Counter = 0;
  char auxCountryCode[10] = {};

  char auxSTR[200] = {};

  memset(aux_phNumber, 0, sizeof(aux_phNumber));
  while (CountryCode_Counter < 10) {

    if (phoneNumber[CountryCode_Counter] == '1' &&
        phoneNumber[CountryCode_Counter + 1] == '-') {

      memset(auxCountryCode, 0, sizeof(auxCountryCode));
      for (int i = 0; i < 5; i++) {
        auxCountryCode[CountryCode_Counter] = phoneNumber[CountryCode_Counter];
        CountryCode_Counter++;
      }
      // ////printf("aux phnumber11 %s\n", auxCountryCode);
      if (parse_CountryPhoneNumber(auxCountryCode)) {
        for (int i = CountryCode_Counter; i < strlen(phoneNumber); i++) {
          aux_phNumber[i - CountryCode_Counter] = phoneNumber[i];
        }

        sprintf(aux_phNumber, "%s", check_zeroInNumber(aux_phNumber));
        // ////printf("aux country code zero 4- %s", aux_phNumber);
        return aux_phNumber;
      } else {
        return phoneNumber;
      }
    } else {
      auxCountryCode[CountryCode_Counter] = phoneNumber[CountryCode_Counter];

      if (parse_CountryPhoneNumber(auxCountryCode)) {

        for (int i = CountryCode_Counter + 1; i < strlen(phoneNumber); i++) {
          // ////printf("aux phnumberfaf %s, %d\n", aux_phNumber, i);
          aux_phNumber[i - (CountryCode_Counter)-1] = phoneNumber[i];
        }

        sprintf(aux_phNumber, "%s", check_zeroInNumber(aux_phNumber));
        // ////printf("aux country code zero 5- %s", aux_phNumber);
        return aux_phNumber;
      } else {
        CountryCode_Counter++;
      }
    }

    if (CountryCode_Counter == 10) {

      sprintf(aux_phNumber, "%s", check_zeroInNumber(aux_phNumber));
      // ////printf("aux country code zero 6- %s", aux_phNumber);
      return aux_phNumber;
    }
  }

  sprintf(aux_phNumber, "%s", check_zeroInNumber(aux_phNumber));
  // ////printf("aux country code zero 7- %s", aux_phNumber);
  return aux_phNumber;
}

char *check_IF_haveCountryCode(char *phoneNumber, uint8_t label_addUser) {
  char auxCountryCode[10] = {};
  int8_t ACK = 0;
  char auxSTR[200] = {};

  int CountryCode_Counter = 1;
  // TODO: ALTERAR NO CODIGO DO 4G
  char PH_Number[20] = {};
  int aux_counter = 1;
  memset(aux_phNumber, 0, sizeof(aux_phNumber));
  memset(PH_Number, 0, sizeof(PH_Number));
  memset(auxCountryCode, 0, sizeof(auxCountryCode));
  // //printf("\n\nCountryCode_number1 %s\n\n", phoneNumber);
  if (phoneNumber[0] == '+') {

    while (CountryCode_Counter < 10) {

      if (phoneNumber[CountryCode_Counter] == '1' &&
          phoneNumber[CountryCode_Counter + 1] == '-') {
        memset(auxCountryCode, 0, sizeof(auxCountryCode));
        for (int i = 0; i < 5; i++) {
          auxCountryCode[CountryCode_Counter - 1] =
              phoneNumber[CountryCode_Counter];
          CountryCode_Counter++;
        }
        // ////printf("aux phnumber11 %s\n", auxCountryCode);
        if (parse_CountryPhoneNumber(auxCountryCode)) {
          for (int i = CountryCode_Counter; i < strlen(phoneNumber); i++) {
            aux_phNumber[i - CountryCode_Counter] = phoneNumber[i];
          }

          if (!label_addUser) {
            /* code */
            sprintf(aux_phNumber, "%s", check_zeroInNumber(aux_phNumber));
            // ////printf("aux country code zero 1- %s", aux_phNumber);
          }
          // ////printf("aux phnumber %s\n", aux_phNumber);
          return aux_phNumber;
        } else {
          // ////printf("aux phnumber ERROR %s\n", aux_phNumber);
          return "ERROR";
        }
      } else {
        auxCountryCode[CountryCode_Counter - 1] =
            phoneNumber[CountryCode_Counter];
        // ////printf("\n  payload phnumber sms 7894 %s \n", phoneNumber);

        if (parse_CountryPhoneNumber(auxCountryCode)) {

          for (int i = CountryCode_Counter + 1; i < strlen(phoneNumber); i++) {
            // ////printf("aux phnumberfaf %s, %d\n", aux_phNumber, i);
            aux_phNumber[i - (CountryCode_Counter + 1)] = phoneNumber[i];
          }

          if (!label_addUser) {
            sprintf(aux_phNumber, "%s", check_zeroInNumber(aux_phNumber));
            // ////printf("aux country code zero 2- %s", aux_phNumber);
          }
          // ////printf("\n  payload phnumber sms 123456 %s \n", aux_phNumber);
          return aux_phNumber;
        } else {
          CountryCode_Counter++;
        }
      }

      if (CountryCode_Counter == 10) {
        return "ERROR";
      }
    }
  } else if (phoneNumber[0] == '0' && phoneNumber[1] == '0') {
    CountryCode_Counter = 2;
    while (CountryCode_Counter < 10) {
      // ////printf("CountryCode_Counter %d\n", CountryCode_Counter);
      // ////printf("aux phnumber %s\n", auxCountryCode);
      if (phoneNumber[CountryCode_Counter] == '1' &&
          phoneNumber[CountryCode_Counter + 1] == '-') {
        memset(auxCountryCode, 0, sizeof(auxCountryCode));
        for (int i = 0; i < 5; i++) {
          auxCountryCode[CountryCode_Counter - 2] =
              phoneNumber[CountryCode_Counter];
          CountryCode_Counter++;
        }
        // ////printf("aux phnumber11 %s\n", auxCountryCode);
        if (parse_CountryPhoneNumber(auxCountryCode)) {
          for (int i = CountryCode_Counter; i < strlen(phoneNumber); i++) {
            aux_phNumber[i - CountryCode_Counter] = phoneNumber[i];
          }

          if (!label_addUser) {
            sprintf(aux_phNumber, "%s", check_zeroInNumber(aux_phNumber));
            // ////printf("aux country code zero 3- %s", aux_phNumber);
          }

          return aux_phNumber;
        } else {
          if (!label_addUser) {
            sprintf(aux_phNumber, "%s", check_zeroInNumber(aux_phNumber));
            // ////printf("aux country code zero 11- %s", aux_phNumber);

            return aux_phNumber;
          }
        }
      } else {
        auxCountryCode[CountryCode_Counter - 2] =
            phoneNumber[CountryCode_Counter];

        if (parse_CountryPhoneNumber(auxCountryCode)) {

          for (int i = CountryCode_Counter + 1; i < strlen(phoneNumber); i++) {
            aux_phNumber[i - (CountryCode_Counter + 1)] = phoneNumber[i];
          }

          if (!label_addUser) {

            sprintf(aux_phNumber, "%s", check_zeroInNumber(aux_phNumber));
            // ////printf("aux country code zero 8- %s", aux_phNumber);
          }

          return aux_phNumber;
        } else {
          CountryCode_Counter++;
        }
      }

      if (CountryCode_Counter == 10) {
        if (!label_addUser) {
          sprintf(aux_phNumber, "%s", check_zeroInNumber(aux_phNumber));
          // ////printf("aux country code zero 12- %s", aux_phNumber);

          return aux_phNumber;
        }
      }
    }
  } else {
    sprintf(aux_phNumber, "%s", phoneNumber);

    if (!label_addUser) {
      sprintf(aux_phNumber, "%s", check_zeroInNumber(aux_phNumber));
      // ////printf("aux country code zero 10- %s", aux_phNumber);

      return aux_phNumber;
    }
  }

  return phoneNumber;
}

int add_time(int old_time, int addition) {
  /* Calculate minutes */
  int total_minutes = (old_time % 100) + (addition % 100);
  int new_minutes = total_minutes % 60;
  /* Calculate hours, adding in the "carry hour" if it exists */
  int additional_hours = addition / 100 + total_minutes / 60;
  int new_hours = (old_time / 100 + additional_hours) % 24;
  /* Put minutes and hours together */
  return new_hours * 100 + new_minutes;
}

char *compress_UDP_Data(uint8_t code, char *payload) {
  switch (code) {
  case IMEI_ICCID_CODE:
    /* code */
    break;

  default:
    break;
  }

  return "ok";
}

char *parseInputData(uint8_t *int_inputData, uint8_t BLE_SMS_Indication,
                     uint8_t gattsIF, uint16_t connID, uint16_t handle_table,
                     data_EG91_Send_SMS *data_SMS, mqtt_information *mqttInfo) {
  // heap_trace_start(HEAP_TRACE_LEAKS);
  MyUser validateData_user;
  char aabff[200];
  char phPassword[7];
  char Input_Command[7];
  char input_Payload[512];
  char phNumber[18];
  char aux_phNumber[50];
  char element[3];
  char cmd;
  char parameter;
  uint8_t label_remove_non_printable = 0;

  char *output_Data = NULL;
  char *inputData;

  uint8_t count = 0;
  uint8_t stringIndex = 0;
  //////printf("\n\n rrrrrad  3n\n\n");
  asprintf(&inputData, "%s", int_inputData);
  //////printf("\n\n rrrrrad  4n\n\n");
  memset(phPassword, 0, sizeof(phPassword));
  memset(aabff, 0, sizeof(aabff));
  //////printf("\n\n rrrrrad  5n\n\n");
  memset(Input_Command, 0, sizeof(Input_Command));
  memset(input_Payload, 0, sizeof(input_Payload));
  //////printf("\n\n rrrrrad  6n\n\n");
  memset(phNumber, 0, sizeof(phNumber));
  memset(element, 0, sizeof(element));
  //////printf("\n\n rrrrrad  7n\n\n");
  memset(&validateData_user, 0, sizeof(validateData_user));

  char search[50];

  memset(search, 0, 50);

  /* if (BLE_SMS_Indication != UDP_INDICATION)
  {
     sprintf(mqttInfo->topic, "%s", "");
  } */

  //////printf("\n\nlabel_remove_non_printable = 0\n\n");

  for (size_t i = 0; i < strlen(inputData); i++) {
    if (inputData[i] == ' ') {
      count++;
      stringIndex = 0;
    } else {
      if (count == 0) {
        search[stringIndex++] = inputData[i];
      } else {
        break;
      }
    }
  }
  // ////printf("\n\n");
  count = 0;
  stringIndex = 0;

  //////printf("\n\nlabel_remove_non_printable = 2323\n\n");

  if (count > 5) {
    asprintf(&output_Data, "NTRSP");

    return output_Data;
  }

  for (size_t i = 0; i < 6; i++) {
    if (inputData[i] == search[i]) {
      count++;
    }
  }

  // ////printf("\n\n count search = %d\n\n", count);

  count = 0;
  stringIndex = 0;

  // //////printf("\n\ncore input data - %s - %d\n\n", inputData,
  // strlen(inputData));

  if (inputData[0] == 'M' && inputData[1] == 'E' && inputData[2] == '.' &&
      inputData[3] == 'S' && inputData[4] == '.' && inputData[5] == 'O' &&
      BLE_SMS_Indication == BLE_INDICATION) {

    asprintf(&output_Data, "ME S O %s",
             MyUser_add_Owner(inputData, 0, BLE_INDICATION));
    free(inputData);

    return output_Data;
  }

  uint8_t owner_Label =
      get_INT8_Data_From_Storage(NVS_KEY_OWNER_LABEL, nvs_System_handle);

  if (owner_Label != 1) {
    if (BLE_SMS_Indication == BLE_INDICATION) {
      asprintf(&output_Data, "%s-%d", "OWNER NOT EXIST",
               check_IF_System_Have_SIM_and_PIN());
      free(inputData);

      return output_Data;
    } else {
      free(inputData);
      return return_ERROR_Codes(&rsp, "NTRSP");
    }
  }

  if (strlen(inputData) > 0) {
    for (int i = 0; i < strlen(inputData); i++) {
      if (inputData[i] == ' ') {
        count++;

        if (count <= 3) {
          stringIndex = 0;
        } else {
          input_Payload[stringIndex] = ' ';
          stringIndex++;
        }
      } else {

        if (count == 0) {
          memcpy(phNumber + stringIndex, inputData + i, 1);
          stringIndex++;
        }

        if (count == 1) {
          memcpy(phPassword + stringIndex, inputData + i,
                 1); // Input_Command[stringIndex]=x1[i];
          // phNumber[stringIndex]=x1[i];
          stringIndex++;
        }

        if (count == 2) {
          memcpy(Input_Command + stringIndex, inputData + i, 1);
          // phNumber[stringIndex]=x1[i];
          stringIndex++;
        }

        if (count > 2) {
          memcpy(input_Payload + stringIndex, inputData + i, 1);
          // phNumber[stringIndex]=x1[i];
          stringIndex++;
        }
      }
    }

    for (size_t i = 0; i < strlen(Input_Command); i++) {
      Input_Command[i] = toupper(Input_Command[i]);
    }

    ////printf("\n\n input comand3232 %s \n\n", Input_Command);

    // ESP_LOGW("tag", "Input_Command input %s yyy \n", "hello world" /*
    // Input_Command) */);

    if ((strlen(Input_Command) == 7 || strlen(Input_Command) == 6) &&
        Input_Command[2] == '.' && Input_Command[4] == '.') {
      strncpy(element, Input_Command, 2);
      cmd = Input_Command[3];
      parameter = Input_Command[5];
    } else {

      // ////printf(ERROR_INPUT_COMMAND);
      free(inputData);
      return return_ERROR_Codes(&output_Data,
                                return_Json_SMS_Data("ERROR_CMD"));
    }

    if (cmd == 'G' && BLE_SMS_Indication == SMS_INDICATION) {
      data_SMS->labelRsp = 1;
    }

    //////printf("phNumber %s ok\n", phNumber);
    //////printf("phPassword %s ok\n", phPassword);
    //////printf("Input_Command %s ok\n", Input_Command);
    //////printf("input_Payload %s ok\n", input_Payload);

    int line = 0;
    memset(aux_phNumber, 0, sizeof(aux_phNumber));

    // ////printf("aux_phNumber1111 %s ok\n", aux_phNumber);

    sprintf(aux_phNumber, "%s", check_IF_haveCountryCode(phNumber, 0));

    if (MyUser_Search_User(aux_phNumber, aabff) == ESP_OK) {

      // ESP_LOGI("TAG", "xPortGetFreeHeapSize1313 : %d",
      // xPortGetFreeHeapSize()); ESP_LOGI("TAG",
      // "esp_get_minimum_free_heap_size  : %d",
      // esp_get_minimum_free_heap_size()); ESP_LOGI("TAG",
      // "heap_caps_get_largest_free_block: %d",
      // heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT)); ESP_LOGI("TAG",
      // "free heap memory : %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));
      //  memset(&validateData_user,0,sizeof(validateData_user));
      //  ////printf("\n\n SD CARD BI STATE 777- %s  /
      //  %s\n\n",validateData_user.firstName,validateData_user.phone);
      parse_ValidateData_User(aabff, &validateData_user);

      if (get_Feedback_SMS() == 1 && validateData_user.permition != '2' &&
          BLE_SMS_Indication == SMS_INDICATION) {
        data_SMS->labelRsp = 0;
      }

      // ////printf("\nparse_ValidateData_User\n");
      if (!strcmp(validateData_user.key, phPassword)) {
        // ////printf("\nstrcmp\n");
        if (!strcmp(element, RELE1_ELEMENT)) {
          // ////printf("\nstrcmp11\n");
          //  heap_trace_start(HEAP_TRACE_ALL);

          output_Data = parse_ReleData(
              BLE_SMS_Indication, RELE1_NUMBER, Input_Command[3],
              Input_Command[5], phPassword, input_Payload, &validateData_user,
              gattsIF, connID, handle_table, data_SMS, mqttInfo);

          free(inputData);

          return output_Data;
        } else if (!strcmp(element, RELE2_ELEMENT)) {
          ////printf("\nstrcmp12\n");
          output_Data = parse_ReleData(
              BLE_SMS_Indication, RELE2_NUMBER, Input_Command[3],
              Input_Command[5], phPassword, input_Payload, &validateData_user,
              gattsIF, connID, handle_table, data_SMS, mqttInfo);
          free(inputData);

          return output_Data;
        }
        /* else if (label_Routine2_ON == 0)
        {
            sprintf(output_Data, "%s", parse_ReleData(BLE_SMS_Indication,
        RELE2_NUMBER, Input_Command[3], Input_Command[5], phPassword,
        input_Payload, &validateData_user, gattsIF, connID, handle_table,
        data_SMS)); return output_Data;
        } */

        else if (!strcmp(element, INPUT1_ELEMENT)) {

          output_Data =
              readInputs(BLE_SMS_Indication, INPUT1_NUMBER, Input_Command[3],
                         Input_Command[5], phPassword, input_Payload);
          free(inputData);

          return output_Data;
        } else if (!strcmp(element, INPUT2_ELEMENT)) {
          output_Data =
              readInputs(BLE_SMS_Indication, INPUT2_NUMBER, Input_Command[3],
                         Input_Command[5], phPassword, input_Payload);
          free(inputData);

          return output_Data;
        } else if (!strcmp(element, ADMIN_ELEMENT)) {

          if (Input_Command[3] == 'G' && Input_Command[5] == 'H') {

            if (BLE_SMS_Indication == BLE_INDICATION) {
              if (validateData_user.permition == '2') {

                if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD)) {

                  if (atoi(input_Payload) > 0 && atoi(input_Payload) <= 12) {
                    send_LogFile(gattsIF, connID, handle_table,
                                 validateData_user.permition, input_Payload);
                    free(inputData);
                    return return_ERROR_Codes(
                        &output_Data,
                        return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
                  } else if (input_Payload[0] == 'G') {
                    // ////printf("\nget_LogFiles_profiles\n");
                    asprintf(&output_Data, "ME G H %s",
                             get_LogFiles_profiles());
                    free(inputData);
                    return output_Data;
                  }
                } else {
                  free(inputData);
                  return return_ERROR_Codes(&output_Data,
                                            "ERROR CARD NOT INSERTED");
                }
              } else {
                free(inputData);
                return return_ERROR_Codes(&output_Data,
                                          ERROR_USER_NOT_PERMITION);
              }
            } else {
              free(inputData);
              return return_ERROR_Codes(
                  &output_Data, return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
            }
          } else {
            /* memset(&output_Data, 0, sizeof(output_Data)); */
            output_Data =
                parse_SystemData(BLE_SMS_Indication, Input_Command[3],
                                 Input_Command[5], phPassword, input_Payload,
                                 &validateData_user, data_SMS, mqttInfo);
            free(inputData);
            return output_Data;
          }
        } else if (!strcmp(element, WIEGAND_ELEMENT)) {
          /*  if (BLE_SMS_Indication == SMS_INDICATION ||BLE_SMS_Indication ==
           UDP_INDICATION || BLE_SMS_Indication == BLE_INDICATION)
           { */
          output_Data = parseWiegand_data(
              BLE_SMS_Indication, gattsIF, connID, handle_table,
              Input_Command[3], Input_Command[5], phPassword, input_Payload,
              &validateData_user, mqttInfo);
          free(inputData);
          return output_Data;
          /*  }
           else
           {
               free(inputData);
               return return_ERROR_Codes(&output_Data, "ONLY WEB FUNCTION");
           } */
        } else if (!strcmp(element, USER_ELEMENT)) {
          // ////printf("\n\n ENTER USER PARSING DATA\n\n");
          if (Input_Command[3] == 'G' && Input_Command[5] == '*') {
            if (BLE_SMS_Indication == BLE_INDICATION ||
                BLE_SMS_Indication == UDP_INDICATION) {
              if (validateData_user.permition == '1' ||
                  validateData_user.permition == '2') {

                // MyUser_ReadAllUsers(gattsIF, connID, handle_table,
                // validateData_user.permition);
                free(inputData);

                if (readAllUser_Label == 0) {
                  readAllUser_Label = 1;
                  asprintf(&output_Data, "%s",
                           MyUser_ReadAllUsers(gattsIF, connID, handle_table,
                                               validateData_user.permition,
                                               BLE_SMS_Indication));
                } else {
                  asprintf(&output_Data, "%s", "READ ALL USERS NOT POSSIBLE");
                }
                // return_ERROR_Codes(&output_Data, MyUser_ReadAllUsers(gattsIF,
                // connID, handle_table, validateData_user.permition));
                return output_Data;
              } else {
                free(inputData);
                return return_ERROR_Codes(&output_Data,
                                          ERROR_USER_NOT_PERMITION);
              }
            } else {
              free(inputData);
              asprintf(&output_Data, "%s",
                       return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
              // ////printf("outputData rsp - %s", output_Data);
              return output_Data;
            }
          } else {
            //////printf("\n enter users comm\n");
            /* memset(&output_Data, 0, sizeof(output_Data)); */
            output_Data = parse_UserData(
                BLE_SMS_Indication, line, Input_Command[3], Input_Command[5],
                phPassword, input_Payload, &validateData_user, gattsIF, connID,
                handle_table, mqttInfo);

            // ////printf("\n enter users comm 12\n");
            free(inputData);
            // ////printf("\n enter users comm 13\n");
            return output_Data;
          }
        } else if (!strcmp(element, RF_ELEMENT)) {

          if (BLE_SMS_Indication == BLE_INDICATION ||
              BLE_SMS_Indication == UDP_INDICATION) {
            output_Data =
                parseRF_data(BLE_SMS_Indication, gattsIF, connID, handle_table,
                             Input_Command[3], Input_Command[5], input_Payload,
                             &validateData_user, mqttInfo);
            free(inputData);
            return output_Data;
          } else {
            free(inputData);
            return return_ERROR_Codes(
                &output_Data, return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
          }

        } else if (!strcmp(element, ROUTINE_ELEMENT)) {

          // ////printf("\n enter routines 1\n");
          if (BLE_SMS_Indication == BLE_INDICATION ||
              BLE_SMS_Indication == UDP_INDICATION) {
            if (validateData_user.permition == '2') {

              output_Data = parse_RoutineData(
                  BLE_SMS_Indication, gattsIF, connID, handle_table,
                  Input_Command[3], Input_Command[5], input_Payload);
              // ////printf("\n erase routines 13\n");
              free(inputData);
              return output_Data;
            } else {
              free(inputData);
              return return_ERROR_Codes(&output_Data, ERROR_USER_NOT_PERMITION);
            }
          } else {
            free(inputData);
            return return_ERROR_Codes(
                &output_Data, return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
          }
        } else {
          free(inputData);
          return return_ERROR_Codes(&output_Data,
                                    return_Json_SMS_Data("ERROR_ELEMENT"));
        }
      } else {

        // ////printf("\n\n\n wrong password\n\n\n");
        if ((!strcmp(element, RELE1_ELEMENT) ||
             !strcmp(element, RELE2_ELEMENT)) &&
            (cmd == 'S' || cmd == 'R') && parameter == 'R') {
          if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD) ||
              network_Activate_Flag == 1) {

            sdCard_Logs_struct logs_struct;
            memset(&logs_struct, 0, sizeof(logs_struct));

            if (BLE_SMS_Indication == BLE_INDICATION) {
              sprintf(logs_struct.type, "%s", "BLE");
            } else if (BLE_SMS_Indication == SMS_INDICATION) {
              sprintf(logs_struct.type, "%s", "SMS");
            }

            sprintf(logs_struct.name, "%s", validateData_user.firstName);
            sprintf(logs_struct.phone, "%s", phNumber);
            sprintf(logs_struct.relay, "%s", element);
            sprintf(logs_struct.relay_state, "%s",
                    return_Json_SMS_Data("NOT_CHANGE"));
            sprintf(logs_struct.date, "%s",
                    replace_Char_in_String(nowTime.strTime, ',', ';'));

            sprintf(logs_struct.error, "%s",
                    return_Json_SMS_Data("ERROR_LOGS_PASSWORD_WRONG"));
            sdCard_Write_LOGS(&logs_struct);
          }
        }

        if (get_Feedback_SMS() == 1 && validateData_user.permition != '2' &&
            BLE_SMS_Indication == SMS_INDICATION) {
          data_SMS->labelRsp = 0;
        }

        // ////printf("\n\n\n wrong password 222\n\n\n");
        free(inputData);

        // return_ERROR_Codes(&output_Data,
        // return_Json_SMS_Data("ERROR_PASSWORD_WRONG"));

        // ////printf("\nERROR_PASSWORD_WRONG - %s\n", output_Data);
        //   return return_Json_SMS_Data("ERROR_PASSWORD_WRONG");
        return return_ERROR_Codes(&output_Data,
                                  return_Json_SMS_Data("ERROR_PASSWORD_WRONG"));
      }
    } else {
      if (get_Feedback_SMS() == 1 && BLE_SMS_Indication == SMS_INDICATION) {
        data_SMS->labelRsp = 0;
      }

      if ((!strcmp(element, RELE1_ELEMENT) ||
           !strcmp(element, RELE2_ELEMENT)) &&
          (Input_Command[3] == 'S' || Input_Command[3] == 'R') &&
          Input_Command[5] == 'R') {
        if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD) ||
            network_Activate_Flag == 1) {
          sdCard_Logs_struct logs_struct;
          memset(&logs_struct, 0, sizeof(logs_struct));

          if (BLE_SMS_Indication == BLE_INDICATION) {
            sprintf(logs_struct.type, "%s", "BLE");
          } else if (BLE_SMS_Indication == SMS_INDICATION) {
            sprintf(logs_struct.type, "%s", "SMS");
          }

          sprintf(logs_struct.name, "%s", return_Json_SMS_Data("NO_NAME"));
          sprintf(logs_struct.phone, "%s", phNumber);
          sprintf(logs_struct.relay, "%s", element);
          sprintf(logs_struct.relay_state, "%s",
                  return_Json_SMS_Data("NOT_CHANGE"));
          sprintf(logs_struct.date, "%s",
                  replace_Char_in_String(nowTime.strTime, ',', ';'));

          sprintf(logs_struct.error, "%s",
                  return_Json_SMS_Data("ERROR_LOGS_USER_NOT_FOUND"));
          sdCard_Write_LOGS(&logs_struct);
        }
      }

      // ////printf("\n USER NOT FOUND ENTER 4");

      free(inputData);
      // ////printf("\n USER NOT FOUND ENTER 5");
      return return_ERROR_Codes(&output_Data,
                                return_Json_SMS_Data("ERROR_USER_NOT_FOUND"));
    }

    free(inputData);
    return return_ERROR_Codes(&output_Data,
                              return_Json_SMS_Data("ERROR_INPUT_DATA"));
  }
  // ////printf("\n\n count search 464 = %d\n\n", count);

  // if (inputData != NULL)
  // {
  free(inputData);
  //}

  // ////printf("\n\n count search 464 = %d\n\n", 44);
  return return_ERROR_Codes(&output_Data,
                            return_Json_SMS_Data("ERROR_INPUT_DATA"));
}

char *removeSpacesFromStr(char *string) {
  // non_space_count to keep the frequency of non space characters
  int non_space_count = 0;

  // Traverse a string and if it is non space character then, place it at index
  // non_space_count
  for (int i = 0; string[i] != '\0'; i++) {
    if (string[i] != ' ') {
      string[non_space_count] = string[i];
      non_space_count++; // non_space_count incremented
    }
  }

  // Finally placing final character at the string end
  string[non_space_count] = '\0';
  return string;
}

char *replace_Char_in_String(char *str, char c_to_replace, char c_new) {
  // ////printf("\n replace str0 %s\n", str);
  for (int i = 0; i < strlen(str); i++) {
    if (str[i] == c_to_replace) {
      str[i] = c_new;
    }
  }
  // ////printf("\n replace str01 %s\n", str);

  return str;
}

uint8_t restore_FileContacts() {
  FILE *fPtr;
  FILE *fTemp;

  char buffer[200];

  /*  Open all required files */
  fPtr = fopen("/spiffs/backup.txt", "r");
  // ////printf("fPtr = fopen(hello\n");
  fTemp = fopen("/spiffs/hello.txt", "w");
  // ////printf("fPtr = fopen(hello backup \n");

  /* fopen() return NULL if unable to open file in given mode. */
  if (fPtr == NULL) {
    /* Unable to open file hence exit */
    // ////printf("\nUnable to open file.\n");
    // ////printf("Please check whether file exists and you have read/write
    // privilege.\n");
    return 0;
  }

  /*
   * Read line from source file and write to destination
   * file after replacing given line.
   */
  while ((fgets(buffer, 200, fPtr)) != NULL) {

    fputs(buffer, fTemp);
    // ////printf("restore %s\n", buffer);
  }

  // ////printf("\nfile 11\n");
  /* Close all files to release resource */
  fclose(fPtr);
  // ////printf("\nfile 22\n");
  fclose(fTemp);
  // ////printf("\nfile 33\n");

  fTemp = fopen("/spiffs/hello.txt", "r");
  // ////printf("fPtr = fopen(hello backup \n\n");

  /* fopen() return NULL if unable to open file in given mode. */
  if (fTemp == NULL) {
    /* Unable to open file hence exit */
    // ////printf("\nUnable to open file.\n");
    // ////printf("Please check whether file exists and you have read/write
    // privilege.\n");
    return 0;
  } else {
    fclose(fTemp);
    return 1;
  }

  return 0;
}

int calculate_weekDay(int year, int month, int day) {
  static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  year -= month < 3;
  return (year + year / 4 - year / 100 + year / 400 + t[month - 1] + day) % 7;
}

void init_Storage() {

  esp_err_t err = 0;
  nvs_flash_init_partition("keys");
  err = nvs_open_from_partition("keys", NVS_SYSTEM_NAMESPACE, NVS_READWRITE,
                                &nvs_System_handle);
  err = nvs_open_from_partition("keys", NVS_OWNER_NAMESPACE, NVS_READWRITE,
                                &nvs_Owner_handle);
  err = nvs_open_from_partition("keys", NVS_ADMIN_NAMESPACE, NVS_READWRITE,
                                &nvs_Admin_handle);
  err = nvs_open_from_partition("keys", NVS_USERS_NAMESPACE, NVS_READWRITE,
                                &nvs_Users_handle);
  err = nvs_open_from_partition("keys", NVS_ROUTINES_NAMESPACE, NVS_READWRITE,
                                &nvs_Routines_handle);
  err = nvs_open_from_partition("keys", NVS_EXEPTION_DAYS_NAMESPACE,
                                NVS_READWRITE, &nvs_Exeption_Days_handle);
  // ////printf("\nerror 0 %d\n", err);
  err = nvs_open_from_partition("keys", NVS_FEEDBACK_NAMESPACE, NVS_READWRITE,
                                &nvs_Feedback_handle);
  // err = nvs_open_from_partition("keys", "beepNamespace", NVS_READWRITE,
  // &nvs_beep_handle);
  err = nvs_open_from_partition("keys", "beepNamespace", NVS_READWRITE,
                                &nvs_beep_handle);

  err = nvs_open_from_partition("keys", NVS_MOBILE_HOLYDAYS_NAMESPACE,
                                NVS_READWRITE, &nvs_Mobile_Holydays_handle);
  // ////printf("\nerror 123 = %d\n", err);

  err = nvs_open_from_partition("keys", NVS_WIEGAND_CODES_GUEST_NAMESPACE,
                                NVS_READWRITE, &nvs_wiegand_codes_users_handle);

  err = nvs_open_from_partition("keys", NVS_WIEGAND_CODES_ADMIN_NAMESPACE,
                                NVS_READWRITE, &nvs_wiegand_codes_admin_handle);

  err = nvs_open_from_partition("keys", NVS_WIEGAND_CODES_OWNER_NAMESPACE,
                                NVS_READWRITE, &nvs_wiegand_codes_owner_handle);

  err = nvs_open_from_partition("keys", NVS_RF_CODES_OWNER_NAMESPACE,
                                NVS_READWRITE, &nvs_rf_codes_owner_handle);

  err = nvs_open_from_partition("keys", NVS_RF_CODES_ADMIN_NAMESPACE,
                                NVS_READWRITE, &nvs_rf_codes_admin_handle);

  err = nvs_open_from_partition("keys", NVS_RF_CODES_GUEST_NAMESPACE,
                                NVS_READWRITE, &nvs_rf_codes_users_handle);

  err =
      nvs_open_from_partition("keys", NVS_WIEGAND_ANTIPASSBACK_NAMESPACE,
                              NVS_READWRITE, &nvs_wiegand_antipassback_USER_handle);

                              err =
      nvs_open_from_partition("keys", NVS_WIEGAND_ANTIPASSBACK_NAMESPACE,
                              NVS_READWRITE, &nvs_wiegand_antipassback_ADMIN_handle);

                              err =
      nvs_open_from_partition("keys", NVS_WIEGAND_ANTIPASSBACK_NAMESPACE,
                              NVS_READWRITE, &nvs_wiegand_antipassback_OWNER_handle);
}

uint8_t save_STR_Data_In_Storage(char *key, char *payload,
                                 nvs_handle_t my_handle) {

  if (nvs_Users_handle == NULL) {
    printf("\n\n handle null \n\n");
    nvs_open_from_partition("keys", NVS_USERS_NAMESPACE, NVS_READWRITE,
                            &nvs_Users_handle);
  }

  printf("\n\n save str %s - %d - %s - %d\n\n", key, strlen(key), payload,
         strlen(payload));
  // int x =
  //////printf("\n\n str save x %d\n\n", x);
  return nvs_set_str(my_handle, key, (const char *)payload);
}

uint8_t save_INT8_Data_In_Storage(char *key, uint8_t value,
                                  nvs_handle_t my_handle) {
  // ////printf("\n\n int save value %d\n\n", value);
  return nvs_set_u8(my_handle, key, value);
}

int8_t get_Data_Users_From_Storage(char *key, char *output_Data) {
  esp_err_t err = 0;
  size_t required_size;
  char aux_Get_Data_User_str[200];
  memset(aux_Get_Data_User_str, 0, sizeof(aux_Get_Data_User_str));

  if (nvs_Users_handle == NULL) {
    /* code */
  }

  if (nvs_get_str(nvs_Users_handle, key, NULL, &required_size) == ESP_OK) {

    if (nvs_get_str(nvs_Users_handle, key, aux_Get_Data_User_str,
                    &required_size) == ESP_OK) {
      //
      sprintf(output_Data, "%s", aux_Get_Data_User_str);
      return ESP_OK;
    }
  }

  memset(aux_Get_Data_User_str, 0, sizeof(aux_Get_Data_User_str));
  required_size = 0;

  if (nvs_get_str(nvs_Admin_handle, key, NULL, &required_size) == ESP_OK) {
    // ////printf("\nrequire size %d\n", required_size);
    // ////printf("\nGET ADMIN NAMESPACE\n");
    //   txtEX = (char *)malloc(required_size * sizeof(char));
    if (nvs_get_str(nvs_Admin_handle, key, aux_Get_Data_User_str,
                    &required_size) == ESP_OK) {
      // ////printf("\naux get str %s\n", aux_Get_Data_User_str);
      sprintf(output_Data, "%s", aux_Get_Data_User_str);
      return ESP_OK;
    }
  }

  memset(aux_Get_Data_User_str, 0, sizeof(aux_Get_Data_User_str));
  required_size = 0;

  if (nvs_get_str(nvs_Owner_handle, key, NULL, &required_size) == ESP_OK) {
    // ////printf("\nrequire size %d\n", required_size);
    // ////printf("\nGET OWNER NAMESPACE\n");
    if (nvs_get_str(nvs_Owner_handle, key, aux_Get_Data_User_str,
                    &required_size) == ESP_OK) {
      // ////printf("\naux get str %s\n", aux_Get_Data_User_str);
      sprintf(output_Data, "%s", aux_Get_Data_User_str);
      return ESP_OK;
    }
  }

  // err = nvs_get_str(&my_handle, "ex", &txtEX, strlen("motorline"));
  // ////printf("\nerror 2 %d\n", err);
  // ////printf("\nex nvs %s\n", output_Data);
  return ESP_FAIL;
}

uint8_t get_Data_STR_Feedback_From_Storage(char *key, char *output_Data) {

  esp_err_t err = 0;
  size_t required_size;
  char aux_Get_Feedback_User_str[200];
  memset(aux_Get_Feedback_User_str, 0, sizeof(aux_Get_Feedback_User_str));

  // ////printf("\nerror 1 %d\n", err);

  if (nvs_get_str(nvs_Feedback_handle, key, NULL, &required_size) == ESP_OK) {
    // ////printf("\nrequire size %d\n", required_size);
    // ////printf("\nGET USERS NAMESPACE\n");
    if (nvs_get_str(nvs_Feedback_handle, key, aux_Get_Feedback_User_str,
                    &required_size) == ESP_OK) {
      // ////printf("\naux get str feedback %s\n", aux_Get_Feedback_User_str);
      sprintf(output_Data, "%s", aux_Get_Feedback_User_str);
      return ESP_OK;
    }
  }

  return ESP_FAIL;
}

uint8_t get_INT8_Data_From_Storage(char *key, nvs_handle_t my_handle) {
  uint8_t value = 0;

  esp_err_t err = nvs_get_u8(my_handle, key, &value);

  // ////printf("\n\n ERROR INT8 %d\n\n", err);

  if (err == ESP_OK) {
    // ////printf("\n\n get_INT8_Data_From_Storage value %d\n\n", value);
    return value;
  } else {
    // ////printf("\n\n get_INT8_Data_From_Storage value -1\n\n");
    return -1;
  }

  return -1;
}

int strpos(const char *haystack, const char *needle) {
  const char *p = strstr(haystack, needle);
  if (p)
    return p - haystack;
  return -1;
}

uint8_t save_NVS_Last_Month(uint8_t month) {
  if (nvs_set_u8(nvs_System_handle, NVS_LAST_MONTH, month) == ESP_OK) {
    return 1;
  } else {
    return 0;
  }
}

uint8_t get_NVS_Last_Month() {
  uint8_t month = 0;

  if (nvs_get_u8(nvs_System_handle, NVS_LAST_MONTH, &month) == ESP_OK) {
    return month;
  } else {
    return 0;
  }
}

uint8_t get_Data_STR_LastCALL_From_Storage(char *output_Data) {

  esp_err_t err = 0;
  size_t required_size;

  // ////printf("\nENTER get_Data_STR_LastCALL_From_Storage\n");
  // ////printf("\nerror 1 %d\n", err);

  if (nvs_get_str(nvs_System_handle, NVS_LAST_CALL, NULL, &required_size) ==
      ESP_OK) {
    // ////printf("\nrequire size %d\n", required_size);

    if (nvs_get_str(nvs_System_handle, NVS_LAST_CALL, output_Data,
                    &required_size) == ESP_OK) {
      // ////printf("\naux get str %s\n", output_Data);
      //   sprintf(output_Data, "%s", aux_Get_Feedback_User_str);
      return ESP_OK;
    }
  }
  // ////printf("\nerror 1 get fail\n");
  return ESP_FAIL;
}

void insert_SubString(char *mainString, char *subString, int position) {
  int p = 0, r = 0, i = 0;
  int t = 0;
  int x, g, s, n, o;
  char *auxString;

  p = position;
  r = strlen(mainString);
  n = strlen(subString);

  s = n + r;
  o = p + n;
  i = 0;

  auxString = (char *)malloc(s * sizeof(char));
  memset(auxString, 0, r + n);

  while (i <= r) {
    auxString[i] = mainString[i];
    i++;
  }

  for (i = p; i < s; i++) {
    x = auxString[i];
    if (t < n) {
      mainString[i] = subString[t];
      t = t + 1;
    }
    mainString[o] = x;
    o = o + 1;
  }

  // ////printf("%s", mainString);
}

char *return_ERROR_Codes(char *input_array, char *ERROR_Code_Message) {

  asprintf(&input_array, "%s", ERROR_Code_Message);

  return input_array;
}

// Função para modificar o valor correspondente a uma chave em um objeto JSON
void modifyJSONValue(const char *key, const char *newValue) {

  // If array position object have the id equal the position
  if (cJSON_GetObjectItem(sms_Rsp_Json, key) != NULL) {
    // Update diagnostic parameter
    //////printf("\nValor da chave 1223\n");
    cJSON_ReplaceItemInObject(
        sms_Rsp_Json, key,
        cJSON_CreateString(
            newValue)); // Substitui o valor existente pelo novo valor
  }

  if (cJSON_GetObjectItem(default_sms_Rsp_Json, key) != NULL) {
    //////printf("\nValor da chave 4567\n");
    cJSON_ReplaceItemInObject(
        default_sms_Rsp_Json, key,
        cJSON_CreateString(
            newValue)); // Substitui o valor existente pelo novo valor
  }

  //////printf("Valor da chave modificado com sucesso.\n");
}

char *return_Json_SMS_Data(char *json_Key) {
  cJSON *rsp_Json_Data;

  if (sms_Rsp_Json != NULL) {
    // ////printf("\n\nChecking 1212 - %s\n\n", json_Key);
    rsp_Json_Data = cJSON_GetObjectItemCaseSensitive(sms_Rsp_Json, json_Key);
    // ////printf("\n\nChecking 1515 %s\n\n", rsp_Json_Data->string);
    if (cJSON_IsString(rsp_Json_Data) && (rsp_Json_Data->valuestring != NULL)) {
      //////printf("Checking monitor \"%s\"\n", rsp_Json_Data->valuestring);
      return rsp_Json_Data->valuestring;
    } else {
      rsp_Json_Data =
          cJSON_GetObjectItemCaseSensitive(default_sms_Rsp_Json, json_Key);
      //////printf("Checking monitor 11\"%s\"\n", rsp_Json_Data->valuestring);
      if (cJSON_IsString(rsp_Json_Data) &&
          (rsp_Json_Data->valuestring != NULL)) {
        //////printf("Checking monitor 22\"%s\"\n", rsp_Json_Data->valuestring);
        return rsp_Json_Data->valuestring;
      } else {
        return "ERROR";
      }
    }
  } else {
    rsp_Json_Data =
        cJSON_GetObjectItemCaseSensitive(default_sms_Rsp_Json, json_Key);

    if (cJSON_IsString(rsp_Json_Data) && (rsp_Json_Data->valuestring != NULL)) {
      // ////printf("Checking monitor \"%s\"\n", rsp_Json_Data->valuestring);
      return rsp_Json_Data->valuestring;
    }
  }
  return "ERROR";
}

void get_NVS_Parameters() {
  char data[100] = {};
  uint32_t restartSystem_time = 0;
  uint8_t releValue = 0;
  uint8_t routine_label;
  uint8_t qmtstat_label;

  rele1_Mode_Label =
      get_INT8_Data_From_Storage(NVS_RELAY1_MODE, nvs_System_handle);

  if (rele1_Mode_Label == 255) {
    save_INT8_Data_In_Storage(NVS_RELAY1_MODE, MONOESTABLE_MODE_INDEX,
                              nvs_System_handle);
    rele1_Mode_Label = MONOESTABLE_MODE_INDEX;
  }

  rele2_Mode_Label =
      get_INT8_Data_From_Storage(NVS_RELAY2_MODE, nvs_System_handle);

  if (rele2_Mode_Label == 255) {
    save_INT8_Data_In_Storage(NVS_RELAY2_MODE, MONOESTABLE_MODE_INDEX,
                              nvs_System_handle);
    rele2_Mode_Label = MONOESTABLE_MODE_INDEX;
  }

  // ////printf("\n\nget_NVS_Parameters rele2_Mode_Label - %d\n\n",
  // rele2_Mode_Label);

  if (nvs_get_u32(nvs_System_handle, NVS_RELAY1_BISTATE_TIME,
                  (uint32_t)&rele1_Bistate_Time) != ESP_OK) {

    nvs_set_u32(nvs_System_handle, NVS_RELAY1_BISTATE_TIME, 1);
    rele1_Bistate_Time = 1;
  }

  // ////printf("\n\nrele1_Bistate_Time %d \n\n", rele1_Bistate_Time);
  if (nvs_get_u32(nvs_System_handle, NVS_RELAY2_BISTATE_TIME,
                  (uint32_t)&rele2_Bistate_Time) != ESP_OK) {
    nvs_set_u32(nvs_System_handle, NVS_RELAY2_BISTATE_TIME, 1);
    rele2_Bistate_Time = 1;
  }

  rele1_Restriction = get_INT8_Data_From_Storage(NVS_KEY_RELAY_PAIRING_VALUE,
                                                 nvs_System_handle);

  if (rele1_Restriction == 255) {
    save_INT8_Data_In_Storage(NVS_KEY_RELAY_PAIRING_VALUE, 0,
                              nvs_System_handle);
    rele1_Restriction = 0;
  }

  uint32_t limit_users = 0;
  if (nvs_get_u32(nvs_System_handle, NVS_LIMIT_USERS, &limit_users) != ESP_OK) {
    nvs_set_u32(nvs_System_handle, NVS_LIMIT_USERS,
                LIMIT_USERS_REGISTER_NUMBER);
  }

  label_network_portalRegister = get_INT8_Data_From_Storage(
      NVS_NETWORK_PORTAL_REGISTER, nvs_System_handle);

  if (label_network_portalRegister == 255) {
    label_network_portalRegister = 1;
    save_INT8_Data_In_Storage(NVS_NETWORK_PORTAL_REGISTER,
                              label_network_portalRegister, nvs_System_handle);
  }

  char translate_data[50] = {};

  if (get_STR_Data_In_Storage(NVS_SMS_INPUT1_MESSAGE, nvs_System_handle,
                              &translate_data) == ESP_OK) {
    modifyJSONValue("INPUT_HAS_BEEN_ACTIVATED1", translate_data);
  }

  memset(translate_data, 0, sizeof(translate_data));

  if (get_STR_Data_In_Storage(NVS_SMS_INPUT2_MESSAGE, nvs_System_handle,
                              &translate_data) == ESP_OK) {
    modifyJSONValue("INPUT_HAS_BEEN_ACTIVATED2", translate_data);
  }

  UDP_logs_label = get_INT8_Data_From_Storage(NVS_NETWORK_LABEL_SEND_LOGS,
                                              nvs_System_handle);

  if (UDP_logs_label == 255) {
    save_INT8_Data_In_Storage(NVS_NETWORK_LABEL_SEND_LOGS, 0,
                              nvs_System_handle);
    UDP_logs_label = 0;
  }

  qmtstat_label =
      get_INT8_Data_From_Storage(NVS_QMTSTAT_LABEL, nvs_System_handle);

  if (qmtstat_label == 255) {
    save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL, 0, nvs_System_handle);
    qmtstat_label = 0;
  }

  if (nvs_get_u32(nvs_System_handle, NVS_KEY_RESTART_SYSTEM,
                  &restartSystem_time) != ESP_OK) {
    nvs_set_u32(nvs_System_handle, NVS_KEY_RESTART_SYSTEM, 303);
  }

  uint8_t redirectSMS_value =
      get_INT8_Data_From_Storage(NVS_REDIRECT_SMS, nvs_System_handle);

  if (redirectSMS_value == 255) {
    save_INT8_Data_In_Storage(NVS_REDIRECT_SMS, 0, nvs_System_handle);
  }

  uint8_t verification_SMS_CALL =
      get_INT8_Data_From_Storage(NVS_SMS_CALL_VERIFICATION, nvs_System_handle);

  if (verification_SMS_CALL == 255) {
    save_INT8_Data_In_Storage(NVS_SMS_CALL_VERIFICATION, 0, nvs_System_handle);
    verification_SMS_CALL = 0;

    if (nvs_get_u32(nvs_System_handle, NVS_ANTIPASSBACK_PEOPLE_NUMBER,
                    &anti_passback_people_number) != ESP_OK) {

      nvs_set_u32(nvs_System_handle, NVS_ANTIPASSBACK_PEOPLE_NUMBER, 0);
      anti_passback_people_number = 0;
    }

    if (nvs_get_u32(nvs_System_handle, NVS_ANTIPASSBACK_PEOPLE_COUNTER,
                    &antipassback_peopleCounter) != ESP_OK) {

      nvs_set_u32(nvs_System_handle, NVS_ANTIPASSBACK_PEOPLE_COUNTER, 0);
      antipassback_peopleCounter = 0;
    }
  }

  //////printf("\n\n routine clear 5454\n\n");
  initNVS_relay2();
  initNVS_relay1();

  get_Feedback_SMS();

  anti_passback_activation = get_INT8_Data_From_Storage(
      NVS_ANTIPASSBACK_ACTIVATE_LABEL, nvs_System_handle);

  if (anti_passback_activation == 255) {
    save_INT8_Data_In_Storage(NVS_ANTIPASSBACK_ACTIVATE_LABEL, 0,
                              nvs_System_handle);
    anti_passback_activation = 0;
  }

  if (get_INT8_Data_From_Storage(NVS_AL_CONF_AL, nvs_System_handle) == 255) {
    save_INT8_Data_In_Storage(NVS_AL_CONF_AL, 0, nvs_System_handle);
  
  }

  if (get_INT8_Data_From_Storage(NVS_RF_ROLLING_CODE,
                                     nvs_System_handle) == 255) {
    save_INT8_Data_In_Storage(NVS_RF_ROLLING_CODE, 1,
                              nvs_System_handle);
  
  }

  if (get_INT8_Data_From_Storage(NVS_INPUT_REX_VALUE,nvs_System_handle) == 255) {
    save_INT8_Data_In_Storage(NVS_INPUT_REX_VALUE, 0,
                              nvs_System_handle);
    }
}

void initNVS_relay2() {
  uint8_t releValue = 0;
  label_Routine2_ON = get_INT8_Data_From_Storage(
      NVS_KEY_ROUTINE2_LABEL,
      nvs_System_handle); // get a label da rotina 1 se estava ativa
  //////printf("\n\n routine clear 5555\n\n");
  if (label_Routine2_ON ==
      255) // verificação se ja existe, se não existe inicia com 0
  {
    label_Routine2_ON = 0;
    save_INT8_Data_In_Storage(NVS_KEY_ROUTINE2_LABEL, label_Routine2_ON,
                              nvs_System_handle);
  }
  //////printf("\n\n routine clear 6666\n\n");
  releValue = get_INT8_Data_From_Storage(
      NVS_KEY_RELAY2_LAST_VALUE,
      nvs_System_handle); // get ao ultimo estado do rele 1
  //////printf("\n\n routine clear 7777\n\n");
  if (releValue == 255) // verificação se ja se encontra inicializada, se não
                        // estiver inicia o valor do ultimo estado, a label da
                        // rotina e do tempo da rotina a 0
  {
    //////printf("\n\n routine clear 8888\n\n");
    gpio_set_level(GPIO_OUTPUT_IO_1, 0);
    save_INT8_Data_In_Storage(NVS_KEY_RELAY2_LAST_VALUE, 0, nvs_System_handle);
    label_Routine2_ON = 0;
    save_INT8_Data_In_Storage(NVS_KEY_ROUTINE2_LABEL, 0, nvs_System_handle);
    nvs_set_u64(nvs_System_handle, NVS_KEY_ROUTINE2_TIME_ON, 0);
  } else // se ja estiver a label iniciada
  {
    //////printf("\n\n routine clear 9999\n\n");
    label_Routine2_ON = get_INT8_Data_From_Storage(
        NVS_KEY_ROUTINE2_LABEL,
        nvs_System_handle); // get à label do ultimo estado da rotina

    if (label_Routine2_ON == 255) // verificação se esta inicializada
    {
      //////printf("\n\n routine clear 10000\n\n");
      // caso nao esteja inicializada inicia a label a 0
      label_Routine2_ON = 0;
      save_INT8_Data_In_Storage(NVS_KEY_ROUTINE2_LABEL, label_Routine2_ON,
                                nvs_System_handle);
    }
    //////printf("\n\n routine clear 1001\n\n");
    if (label_Routine2_ON == 1) // check if rotine was activated
    {
      //////printf("\n\n routine clear 1003\n\n");
      uint64_t timeRoutine2 = 0;
      if (nvs_get_u64(nvs_System_handle, NVS_KEY_ROUTINE2_TIME_ON,
                      &timeRoutine2) != ESP_OK) // get time off of the rotine
      {
        //////printf("\n\n routine clear 1002\n\n");
        timeRoutine2 = 0;
        nvs_set_u64(nvs_System_handle, NVS_KEY_ROUTINE2_TIME_ON, 0);
        //////printf("\n\n routine clear 1003\n\n");
      }
      // if time routine is 0 or now time is bigger than routine off time put 0
      // in variavels
      //////printf("\n\n routine clear 00\n\n");
      //////printf("\n\n routine clear 000\n\n");
      if (timeRoutine2 == 0 || get_nowTime_in_seconds() > timeRoutine2) {
        // //printf("\n\n routine clear\n\n");
        gpio_set_level(GPIO_OUTPUT_IO_1, 0);
        label_Routine2_ON = 0;
        save_INT8_Data_In_Storage(NVS_KEY_ROUTINE2_LABEL, 0, nvs_System_handle);
        save_INT8_Data_In_Storage(NVS_KEY_ROUTINE2_TIME_ON, 0,
                                  nvs_System_handle);
      } else // put 1 in the variavels
      {
        // //printf("\n\n routine NOT clear\n\n");
        gpio_set_level(GPIO_OUTPUT_IO_1, 1);
        label_Routine2_ON = 1;
        save_INT8_Data_In_Storage(NVS_KEY_ROUTINE2_LABEL, 1, nvs_System_handle);
        save_INT8_Data_In_Storage(NVS_KEY_RELAY2_LAST_VALUE, 1,
                                  nvs_System_handle);
      }
    } else // else label routine is 0 put the relay last value
    {
      label_Routine2_ON = releValue;
      save_INT8_Data_In_Storage(NVS_KEY_ROUTINE2_LABEL, releValue,
                                nvs_System_handle);
      gpio_set_level(GPIO_OUTPUT_IO_1, releValue);
      save_INT8_Data_In_Storage(NVS_KEY_RELAY2_LAST_VALUE, releValue,
                                nvs_System_handle);
    }
  }
}

void initNVS_relay1() {
  uint8_t releValue = 0;
  label_Routine1_ON = get_INT8_Data_From_Storage(
      NVS_KEY_ROUTINE1_LABEL,
      nvs_System_handle); // get a label da rotina 1 se estava ativa

  if (label_Routine1_ON ==
      255) // verificação se ja existe, se não existe inicia com 0
  {
    label_Routine1_ON = 0;
    save_INT8_Data_In_Storage(NVS_KEY_ROUTINE1_LABEL, label_Routine1_ON,
                              nvs_System_handle);
  }

  releValue = get_INT8_Data_From_Storage(
      NVS_KEY_RELAY1_LAST_VALUE,
      nvs_System_handle); // get ao ultimo estado do rele 1

  if (releValue == 255) // verificação se ja se encontra inicializada, se não
                        // estiver inicia o valor do ultimo estado, a label da
                        // rotina e do tempo da rotina a 0
  {
    gpio_set_level(GPIO_OUTPUT_IO_0, 0);
    save_INT8_Data_In_Storage(NVS_KEY_RELAY1_LAST_VALUE, 0, nvs_System_handle);
    label_Routine1_ON = 0;
    save_INT8_Data_In_Storage(NVS_KEY_ROUTINE1_LABEL, 0, nvs_System_handle);
    nvs_set_u64(nvs_System_handle, NVS_KEY_ROUTINE1_TIME_ON, 0);
  } else // se ja estiver a label iniciada
  {
    label_Routine1_ON = get_INT8_Data_From_Storage(
        NVS_KEY_ROUTINE1_LABEL,
        nvs_System_handle); // get à label do ultimo estado da rotina

    if (label_Routine1_ON == 255) // verificação se esta inicializada
    {
      // caso nao esteja inicializada inicia a label a 0
      label_Routine1_ON = 0;
      save_INT8_Data_In_Storage(NVS_KEY_ROUTINE1_LABEL, label_Routine1_ON,
                                nvs_System_handle);
    }

    if (label_Routine1_ON == 1) // check if rotine was activated
    {
      uint64_t timeRoutine1 = 0;
      if (nvs_get_u64(nvs_System_handle, NVS_KEY_ROUTINE1_TIME_ON,
                      &timeRoutine1) != ESP_OK) // get time off of the rotine
      {
        timeRoutine1 = 0;
        nvs_set_u64(nvs_System_handle, NVS_KEY_ROUTINE1_TIME_ON, 0);
      }
      // if time routine is 0 or now time is bigger than routine off time put 0
      // in variavels
      if (timeRoutine1 == 0 || get_nowTime_in_seconds() > timeRoutine1) {
        gpio_set_level(GPIO_OUTPUT_IO_0, 0);
        label_Routine1_ON = 0;
        save_INT8_Data_In_Storage(NVS_KEY_ROUTINE1_LABEL, 0, nvs_System_handle);
        save_INT8_Data_In_Storage(NVS_KEY_ROUTINE1_TIME_ON, 0,
                                  nvs_System_handle);
      } else // put 1 in the variavels
      {
        gpio_set_level(GPIO_OUTPUT_IO_0, 1);
        label_Routine1_ON = 1;
        save_INT8_Data_In_Storage(NVS_KEY_ROUTINE1_LABEL, 1, nvs_System_handle);
        save_INT8_Data_In_Storage(NVS_KEY_RELAY1_LAST_VALUE, 1,
                                  nvs_System_handle);
      }
    } else // else label routine is 0 put the relay last value
    {
      label_Routine1_ON = releValue;
      save_INT8_Data_In_Storage(NVS_KEY_ROUTINE1_LABEL, releValue,
                                nvs_System_handle);
      gpio_set_level(GPIO_OUTPUT_IO_0, releValue);
      save_INT8_Data_In_Storage(NVS_KEY_RELAY1_LAST_VALUE, releValue,
                                nvs_System_handle);
    }
  }
}

uint8_t get_STR_Data_In_Storage(char *key, nvs_handle_t my_handle,
                                char *strRSP) {

  esp_err_t err = 0;
  size_t required_size;
  char aux_Get_Data_User_str[200];
  memset(aux_Get_Data_User_str, 0, sizeof(aux_Get_Data_User_str));

  if (nvs_get_str(my_handle, key, NULL, &required_size) == ESP_OK) {
    if (nvs_get_str(my_handle, key, aux_Get_Data_User_str, &required_size) ==
        ESP_OK) {
      // //printf("\naux get str %s\n", aux_Get_Data_User_str);
      sprintf(strRSP, "%s", aux_Get_Data_User_str);
      return ESP_OK;
    }
  }

  return ESP_FAIL;
}

void vTimerCallback(TimerHandle_t xTimer) {
  const uint32_t ulMaxExpiryCountBeforeStopping = 10;
  uint32_t ulCount;

  if (RSSI_VALUE == 99 || RSSI_VALUE == 199) {
    gpio_set_level(GPIO_OUTPUT_ACT, 1);
  } else {
    // ////printf("\nact toogle\n");
    // ////printf("\nCD CARD PIN %d\n",
    // gpio_get_level(GPIO_INPUT_IO_CD_SDCARD));

    if (label_Reset_Password_OR_System == 1) {
      gpio_set_level(GPIO_OUTPUT_ACT, 1);
      vTaskDelay(pdMS_TO_TICKS(150));
      gpio_set_level(GPIO_OUTPUT_ACT, 0);
      vTaskDelay(pdMS_TO_TICKS(150));
      gpio_set_level(GPIO_OUTPUT_ACT, 1);
      vTaskDelay(pdMS_TO_TICKS(150));
      gpio_set_level(GPIO_OUTPUT_ACT, 0);
      vTaskDelay(pdMS_TO_TICKS(150));
    } else {
      if (gpio_get_level(GPIO_INPUT_IO_SIMPRE) &&
          label_Reset_Password_OR_System != 0) {
        uint8_t times_TO_ACT_Toogle = RSSI_LED_TOOGLE;

        if (times_TO_ACT_Toogle == 5) {
          gpio_set_level(GPIO_OUTPUT_ACT, 1);
        } else {
          while (times_TO_ACT_Toogle > 0) {
            gpio_set_level(GPIO_OUTPUT_ACT, 1);
            vTaskDelay(pdMS_TO_TICKS(300));
            gpio_set_level(GPIO_OUTPUT_ACT, 0);
            vTaskDelay(pdMS_TO_TICKS(300));
            times_TO_ACT_Toogle--;
          }
        }

        /* toggle21 ^= 0x01;
        gpio_set_level(GPIO_OUTPUT_ACT, toggle21); */
      } else if (label_Reset_Password_OR_System == 0) {
        gpio_set_level(GPIO_OUTPUT_ACT, 1);
        vTaskDelay(pdMS_TO_TICKS(150));
        gpio_set_level(GPIO_OUTPUT_ACT, 0);
        vTaskDelay(pdMS_TO_TICKS(150));
        gpio_set_level(GPIO_OUTPUT_ACT, 1);
        vTaskDelay(pdMS_TO_TICKS(150));
        gpio_set_level(GPIO_OUTPUT_ACT, 0);
        vTaskDelay(pdMS_TO_TICKS(150));
        gpio_set_level(GPIO_OUTPUT_ACT, 1);
        vTaskDelay(pdMS_TO_TICKS(150));
        gpio_set_level(GPIO_OUTPUT_ACT, 0);
        vTaskDelay(pdMS_TO_TICKS(150));
        gpio_set_level(GPIO_OUTPUT_ACT, 1);
        vTaskDelay(pdMS_TO_TICKS(150));
        gpio_set_level(GPIO_OUTPUT_ACT, 0);
        vTaskDelay(pdMS_TO_TICKS(150));
      } else {
        gpio_set_level(GPIO_OUTPUT_ACT, 1);
      }
    }

    //  gpio_set_level(GPIO_NUM_21, toggle21);
  }
  //  gpio_set_level(GPIO_NUM_21, toggle21);
}

uint8_t refresh_ESP_RTC() {
  uint8_t data[7];
  char timeZone[50] = {};
  if (PCF85_Readnow(data)) {
    struct tm timeinfo;
    size_t required_size = 0;

    for (size_t i = 0; i < 7; i++) {
      // //printf("\n read rtc 22 %d", data[i]);
    }

    timeinfo.tm_year = data[6] + YEAR_OFFSET - 1900;
    timeinfo.tm_mon = data[5] - 1;
    timeinfo.tm_mday = data[3];
    timeinfo.tm_hour = data[2];
    timeinfo.tm_min = data[1];
    timeinfo.tm_sec = data[0];
    timeinfo.tm_wday = data[4];
    timeinfo.tm_isdst = 0;

    // ////printf("%d:%02d:%02d - %d %d/%d/%d\r\n", data[2], data[1], data[0],
    // data[4], data[6] + YEAR_OFFSET, data[5], data[3]);

    time_t auxEpoch = epoch_Calculator(&timeinfo);

    // ////printf("\n\n epoch refresh %ld\n\n", auxEpoch);

    struct timeval epoch = {auxEpoch, 0};

    if (nvs_get_str(nvs_System_handle, NVS_TIMEZONE, NULL, &required_size) ==
        ESP_OK) {
      // ////printf("\nrequire size %d\n", required_size);

      if (nvs_get_str(nvs_System_handle, NVS_TIMEZONE, timeZone,
                      &required_size) == ESP_OK) {
      } else {
        sprintf(timeZone, "%s", "GMT0");
      }
    } else {
      sprintf(timeZone, "%s", "GMT0");
    }

    // ////printf("\n\n epoch refresh 1234\n\n");
    if (!settimeofday(&epoch, NULL)) {
      // ////printf("\tzset 2\n");
      if (!setenv("TZ", timeZone, 1)) {
        // ////printf("\n\n\n getenv - %s", getenv("TZ"));
        // ////printf("\tzset 3\n");
        save_STR_Data_In_Storage(NVS_TIMEZONE, timeZone, nvs_System_handle);
        tzset();
      } else {
        return 0;
      }
    } else {
      return 0;
    }
  }
  // initRoutines();
  return 1;
}

uint8_t get_RTC_System_Time() {
  uint8_t data[7];

  time_t now;
  struct tm timeinfo;

  char buffer[256];
  int buffer_len = 256;

  time(&now);
  /* setenv("TZ", "UTC1", 1);
  tzset(); */
  localtime_r(&now, &timeinfo);

  nowTime.year = timeinfo.tm_year - 100;

  if (nowTime.year < 0) {
    nowTime.year = 0;
  }

  nowTime.month = timeinfo.tm_mon + 1;
  nowTime.day = timeinfo.tm_mday;
  nowTime.time = timeinfo.tm_hour * 100 + timeinfo.tm_min;

  nowTime.weekDay = timeinfo.tm_wday;

  nowTime.date = nowTime.year * 10000 + nowTime.month * 100 + nowTime.day;

  sprintf(nowTime.strTime, "%02d/%02d/%02d,%02d:%02d:%02d",
          (nowTime.year + 2000), nowTime.month, nowTime.day, timeinfo.tm_hour,
          timeinfo.tm_min, timeinfo.tm_sec);

  // ////printf("\n\n time: %s - %d - %d \n\n", nowTime.strTime, nowTime.time,
  // nowTime.date);

  /*  if (PCF85_Readnow(data))
   {
       //////printf("\r\nRTC NOW >>> 12345 ");
       //////printf("%d:%02d:%02d - %d %d/%d/%d\r\n", data[2], data[1], data[0],
   data[4], data[6] + YEAR_OFFSET, data[5], data[3]);
   } */

  //     nowTime.year = data[6] + YEAR_OFFSET - 2000;

  //     if (nowTime.year < 0)
  //     {
  //         nowTime.year = nowTime.year + 31;
  //     }

  //     nowTime.month = data[5];
  //     nowTime.day = data[3];
  //     nowTime.time = data[2] * 100 + data[1];
  //     nowTime.weekDay = data[4];

  //     // ////printf("\n\nnowTime.year - %d / data[6] - %d\n\n", nowTime.year,
  //     data[6]);

  //     nowTime.date = (nowTime.year * 10000) + (nowTime.month * 100) +
  //     nowTime.day; sprintf(nowTime.strTime, "%02d/%02d/%02d,%02d:%02d",
  //     (nowTime.year + 2000), nowTime.month, nowTime.day, data[2], data[1]);

  //     int weekDay = calculate_weekDay(nowTime.year, nowTime.month,
  //     nowTime.day);

  // struct tm timeinfo;

  // timeinfo.tm_year = data[6] + YEAR_OFFSET - 1900;
  // timeinfo.tm_mon = data[5] - 1;
  // timeinfo.tm_mday = data[3];
  // timeinfo.tm_hour = data[2];
  // timeinfo.tm_min = data[1];
  // timeinfo.tm_sec = data[0];
  // timeinfo.tm_wday = weekDay - 1;
  // timeinfo.tm_isdst = 0;
  //         //  struct timeval *tv = &epoch;

  //         int int_epoch = 0; // mktime(&timeinfo);

  //         ////printf("\n\n\n int epoch %d\n\n\n", int_epoch);
  //         char timeZone[50] = {};

  //         esp_err_t err = 0;
  //         size_t required_size;

  //         ////printf("\nENTER get_Data_STR_LastCALL_From_Storage\n");
  //         ////printf("\nerror 1 %d\n", err);

  //         ////printf("\n\n\n getenv - %s",getenv("TZ"));
  //         // settimeofday(&epoch, NULL);
  //         time_t now;
  //         struct tm *timeinfo_Now;

  //         //unsetenv("TZ");

  //         /* localtime_r(&now, &timeinfo_Now);

  //         ////printf("\n\n year - %d, month - %d, day - %d, dayweek - %d,
  //         hour - %d, minute - %d, sec - %d\n\n", (timeinfo_Now.tm_year +
  //         1900), timeinfo_Now.tm_mon, timeinfo_Now.tm_mday,
  //         timeinfo_Now.tm_wday, timeinfo_Now.tm_hour, timeinfo_Now.tm_min,
  //         timeinfo_Now.tm_sec);
  //  */

  //         int_epoch = mktime(&timeinfo);

  //         /* memset(timezone_network, 0, 5);
  //         sprintf(timezone_network, "%s", "32");
  //         timeZone_Signal = '-'; */
  //         ////printf("\n\n timezone network %d\n\n", atoi(timezone_network));

  //         if (nvs_get_str(nvs_System_handle, NVS_TIMEZONE, NULL,
  //         &required_size) == ESP_OK)
  //         {
  //             ////printf("\nrequire size %d\n", required_size);

  //             if (nvs_get_str(nvs_System_handle, NVS_TIMEZONE, timeZone,
  //             &required_size) == ESP_OK)
  //             {
  //                 ////printf("\naux get str %s\n", timeZone);
  //                 int_epoch = mktime(&timeinfo);
  //             }
  //             else
  //             {

  //                 int_epoch = mktime(&timeinfo);

  //                 memset(timeZone, 0, 50);

  //                 sprintf(timeZone, "%s", "GMT0");
  //             }
  //         }
  //         else
  //         {
  //             int_epoch = mktime(&timeinfo);

  //             memset(timeZone, 0, 50);

  //             sprintf(timeZone, "%s", "GMT0");
  //         }

  //         struct timeval epoch = {int_epoch, 0};

  //         if (!settimeofday(&epoch, NULL))
  //         {

  //             /* time(&now);

  //             timeinfo_Now = localtime(&now);
  //             // localtime_r(&now, &timeinfo_Now);

  //             ////printf("\n\n 14521452 year - %d, month - %d, day - %d,
  //             dayweek - %d, hour - %d, minute - %d, sec - %d\n\n",
  //             (timeinfo_Now->tm_year + 1900), timeinfo_Now->tm_mon,
  //             timeinfo_Now->tm_mday, timeinfo_Now->tm_wday,
  //             timeinfo_Now->tm_hour, timeinfo_Now->tm_min,
  //             timeinfo_Now->tm_sec);
  // */
  //             ////printf("\n\n timezone qlts %s", timeZone);
  //             if (!setenv("TZ", timeZone, 1))
  //             {

  //                 ////printf("\n\n\n getenv22 - %s", getenv("TZ"));
  //                 tzset();

  //                 time(&now);
  //                 timeinfo_Now = localtime(&now);
  //                 localtime_r(&now, &timeinfo_Now);

  //                 ////printf("\n\n year - %d, month - %d, day - %d, dayweek -
  //                 %d, hour - %d, minute - %d, sec - %d\n\n",
  //                 (timeinfo_Now->tm_year + 1900), timeinfo_Now->tm_mon,
  //                 timeinfo_Now->tm_mday, timeinfo_Now->tm_wday,
  //                 timeinfo_Now->tm_hour, timeinfo_Now->tm_min,
  //                 timeinfo_Now->tm_sec);

  //                /* PCF85_SetTime(timeinfo_Now->tm_hour,
  //                timeinfo_Now->tm_min, timeinfo_Now->tm_sec);
  //                 PCF85_SetDate(timeinfo_Now->tm_wday, timeinfo_Now->tm_mday,
  //                 (timeinfo_Now->tm_mon + 1), (timeinfo_Now->tm_year +
  //                 1900)); */
  //                 /* get_RTC_System_Time();
  //                 enableAlarm(); */
  //             }
  //             else
  //             {
  //                 // enableAlarm();
  //                 //  ////printf("\nTIME ZONE ERROR 1212\n");
  //                 return 0;
  //             }
  //         }
  //         else
  //         {
  //             // enableAlarm();
  //             ////printf("\nTIME ZONE ERROR 1313\n");
  //             return 0;
  //         }

  // ////printf("\n\nget_RTC_System_Time nowTime.date - %d\n\n", nowTime.date);
  //  enableAlarm();
  // ////printf("\n\nget_RTC_System_Time nowTime.time - %d\n\n", nowTime.time);

  // ////printf("\n\nget_RTC_System_Time strtime - %s\n\n", nowTime.strTime);
  return 1;
  // }
}

void give_LOG_Files_Semph() { xSemaphoreGive(rdySem_Send_LOGS_Files); }
void take_LOG_Files_Semph() {
  xSemaphoreTake(rdySem_Send_LOGS_Files, pdMS_TO_TICKS(2500));
  // vTaskDelay(pdMS_TO_TICKS(1000));
}

void initSystem() {
  esp_err_t ret;

  readAllUser_Label = 0;
  readAllUser_ConnID = 0;
  label_Reset_Password_OR_System = 0;
  label_BLE_UDP_send = 1;

  rele1_Bistate_Time = 1;
  rele1_Mode_Label = 0;
  rele2_Bistate_Time = 10;
  rele2_Mode_Label = 0;
  label_ResetSystem = 0;
  count_ResetSystem = 0;

  label_Routine1_ON = 0;
  label_Routine2_ON = 0;

  label_Block_Feedback_SMS = 0;
  // label_Translate_Files = 0;
  label_initSystem_SIMPRE = 0;
  label_initSystem_CALL = 0;

  init_Storage();

  //TODO: APAGAR LINHA A BAIXO
  nvs_set_u8(nvs_System_handle, NVS_INPUT_REX_VALUE, 3);

  // TODO: APAGAR LINHA A BAIXO
  // nvs_erase_key(nvs_System_handle, NVS_TIMEZONE);
  label_network_portalRegister = get_INT8_Data_From_Storage(
      NVS_NETWORK_PORTAL_REGISTER, nvs_System_handle);

  // //printf("\n\n label_network_portalRegister %d\n\n",
  // label_network_portalRegister);
  if (label_network_portalRegister == 255) {
    label_network_portalRegister = 1;
    save_INT8_Data_In_Storage(NVS_NETWORK_PORTAL_REGISTER,
                              label_network_portalRegister, nvs_System_handle);
  }

  label_network_portalRegister = get_INT8_Data_From_Storage(
      NVS_NETWORK_PORTAL_REGISTER, nvs_System_handle);
  // //printf("\n\n label_network_portalRegister 11 %d\n\n",
  // label_network_portalRegister);
  ////printf("imei=543219876543210");
  char HW_Version[10] = {};

  size_t required_size = 0;

  uint8_t label_UDP_fail_and_changed =
      get_INT8_Data_From_Storage(NVS_NETWORK_LOCAL_CHANGED, nvs_System_handle);
  ;
  if (label_UDP_fail_and_changed == 255) {
    label_UDP_fail_and_changed = 0;
    label_UDP_fail_and_changed |= 3;
    save_INT8_Data_In_Storage(NVS_NETWORK_LOCAL_CHANGED,
                              label_UDP_fail_and_changed, nvs_System_handle);
  }

  if (nvs_get_str(nvs_System_handle, NVS_KEY_HW_VERSION, NULL,
                  &required_size) == ESP_OK) {
    // ////printf("\nrequire size %d\n", required_size);
    // //printf("\nGET USERS NAMESPACE HARDWARE\n");
    if (nvs_get_str(nvs_System_handle, NVS_KEY_HW_VERSION, HW_Version,
                    &required_size) == ESP_OK) {
      ////printf("\nrequire HW_Version %s\n", HW_Version);
    } else {
      nvs_set_str(nvs_System_handle, NVS_KEY_HW_VERSION, HW_VERSION_PROD);
      sprintf(HW_Version, "%s", HW_VERSION_PROD);
    }
  } else {
    nvs_set_str(nvs_System_handle, NVS_KEY_HW_VERSION, HW_VERSION_PROD);
    sprintf(HW_Version, "%s", HW_VERSION_PROD);
  }

  // ////printf("\n\n HW_Version - %s testes testes\n\n", HW_Version);

  char timeZone[50] = {};
  if (nvs_get_str(nvs_System_handle, NVS_TIMEZONE, timeZone, &required_size) ==
      ESP_OK) {
    // ////printf("\naux get str %s\n", timeZone);

    if (!setenv("TZ", timeZone, 1)) {

      // ////printf("\n\n\n getenv22 - %s", getenv("TZ"));
      tzset();
    }
  }

  // init GPIO

  // xTaskCreate(wiegand1_task2, "TAG", 2048 * 4 + 1024, NULL, 5, NULL);
  //  resetRele1();
  //  resetRele2();

  gpio_set_level(CONFIG_GPIO_OUTPUT_ACT, 1);
  PCF85_Init();
  refresh_ESP_RTC();

  // ////printf("\n\n akakak\n\n");

  // get_RTC_System_Time();
  uint8_t data[7];
  // get_RTC_System_Time();

  // disableAlarm();

  // write_FbNumbers_Struct();

  cron_job *jobs[2];

  routine_list = (list_node *)malloc(sizeof(list_node));
  routine_list->next = NULL;

  // ////printf("\n\n akakak 111\n\n");
  rdySem = xSemaphoreCreateBinary();

  rdySem_RelayMonoStart = xSemaphoreCreateBinary();
  rdySem_Send_LOGS_Files = xSemaphoreCreateBinary();
  rdySem_Reset_System = xSemaphoreCreateBinary();
  rdySem_Send_Routines = xSemaphoreCreateBinary();
  rdySem_Routine_BiState_Send_RelayState1 = xSemaphoreCreateBinary();
  rdySem_Routine_BiState_Send_RelayState2 = xSemaphoreCreateBinary();
  init_rdySem_Control_SD_Card_Write();

  xSemaphoreTake(rdySem_Send_LOGS_Files, 1);
  xSemaphoreTake(rdySem_Routine_BiState_Send_RelayState1, 0);
  xSemaphoreTake(rdySem_Routine_BiState_Send_RelayState2, 0);

  FILE *f = fopen("/spiffs/language.json", "r");

  if (f != NULL) {
    int file_Lenght;
    char *json_Translate_File;
    fseek(f, 0L, SEEK_END);
    file_Lenght = ftell(f);
    fseek(f, 0L, SEEK_SET);
    // rewind(f);
    // ////printf("\n\n\n end lang files 3 - %d\n\n\n", file_Lenght);

    // ////printf("\n\n\n ckf OK \n\n\n");

    json_Translate_File = (char *)malloc(file_Lenght * sizeof(char));

    memset(json_Translate_File, 0, file_Lenght);

    // ////printf("\n\n\n end lang files 4\n\n\n");

    if (json_Translate_File) {
      // ////printf("\n\n\n end lang files 5\n\n\n");

      fread(json_Translate_File, sizeof(char), file_Lenght, f);
      // ////printf("\n\n\n end lang\n%s\n\n\n", json_Translate_File);

      // cJSON_Delete(sms_Rsp_Json);
      sms_Rsp_Json = cJSON_Parse(json_Translate_File);
      default_sms_Rsp_Json = cJSON_Parse(jsonFile);
      free(json_Translate_File);

      // ////printf("\n\n\n end lang files 8\n\n\n");
      if (sms_Rsp_Json == NULL || default_sms_Rsp_Json == NULL) {

        // ////printf("\n\n X10 9999\n\n");
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
          // //printf("Error before: %s\n", error_ptr);
        }
      }
    } else {
      default_sms_Rsp_Json = cJSON_Parse(jsonFile);
      sms_Rsp_Json = cJSON_Parse(jsonFile);
      free(json_Translate_File);
    }

    fclose(f);
  } else {
    default_sms_Rsp_Json = cJSON_Parse(jsonFile);
    sms_Rsp_Json = cJSON_Parse(jsonFile);

    // ////printf("\n\n\n ckf OK \n\n\n");

    if (sms_Rsp_Json == NULL) {
      // ////printf("\n\n X10 9999\n\n");
      const char *error_ptr = cJSON_GetErrorPtr();
      if (error_ptr != NULL) {
        // ////printf("\n\nError before: %s\n", error_ptr);
      }
    }
    // free(json_Translate_File);
  }
  /*  modifyJSONValue( "INPUT_HAS_BEEN_ACTIVATED1","JA FOSTE ENTRADA 1");
   modifyJSONValue( "INPUT_HAS_BEEN_ACTIVATED1","JA FOSTE ENTRADA 1 AHAHAH"); */
  uint32_t value = 0;

   gpio_init();
  value = MyUser_List_AllUsers();
  save_User_Counter_In_Storage(value);

  printf("\n\n\n end lang 7777\n\n\n");

  // ////printf("\n\n X10 1223 -- %d\n\n",
  // gpio_get_level(GPIO_INPUT_IO_SIMPRE));
  get_NVS_Parameters();

  // ////printf("\n\n akakak 221\n\n");
  //  xTaskCreate(task_sendFeedbackData, "task_sendFeedbackData", 2 * 2048,
  //  NULL, 20, NULL);
  xTaskCreate(task_BiState_Send_RelayState1, "task_BiState_Send_RelayState1",
              2 * 2048 + 1024, NULL, 15, &rele1_Bistate_Task_Handle);
  xTaskCreate(task_BiState_Send_RelayState2, "task_BiState_Send_RelayState2",
              2 * 2048 + 1024, NULL, 15, &rele2_Bistate_Task_Handle);
  // ////printf("\n\n BEFORE ROUTINE RELAY\n\n");
  xTaskCreate(task_Routine_BiState_Send_RelayState1,
              "task_Routine_BiState_Send_RelayState1", 1024, NULL, 20,
              &task_Routine_BiState_RelayState1);
  xTaskCreate(task_Routine_BiState_Send_RelayState2,
              "task_Routine_BiState_Send_RelayState2", 1024, NULL, 20,
              &task_Routine_BiState_RelayState2);

  char fb_phoneff[30];

  // ////printf("\n\n akakak 222\n\n");
  //  ////printf("\n F1 label1 %d\n", get_INT8_Data_From_Storage(NVS_FB_CONF_F1,
  //  nvs_Feedback_handle));
  get_Data_STR_Feedback_From_Storage(NVS_FB_CONF_P1, &fb_phoneff);
  // ////printf("\n F1 phone1 %s\n", fb_phoneff);
  // ////printf("\n F1 in cfg1 %d\n", get_INT8_Data_From_Storage(NVS_FB_CONF_I1,
  // nvs_Feedback_handle));
  printf("\n\n akakak 333\n\n");
  xTaskCreate(task_Send_Routines, "task_Send_Routines", 1024 * 4, NULL, 18,
              &task_Routine_BiState_RelayState2);

  init_EG91();
  xTaskCreate(taskMipot_rf, "taskMipot_rf", 6 * 2048, NULL, 15,
              &rele1_Bistate_Task_Handle);
  xTaskCreate(wiegand1_task, "TAG", 2048 * 4 + 1024, NULL, 5, NULL);
  xTaskCreate(wiegand2_task, "TAG", 2048 * 4 + 1024, NULL, 5, NULL);
  printf("\n\n akakak 444\n\n");
  label_initSystem_SIMPRE = 1;
  // save_INT8_Data_In_Storage(NVS_KEY_SDCARD_RESET, 0, nvs_System_handle);

  example_tg_timer_init(TIMER_GROUP_0, TIMER_0, false, 30);
  xTimers = xTimerCreate("Timer", pdMS_TO_TICKS(2000), pdTRUE, (void *)0,
                         vTimerCallback);
  // ////printf("\n\n akakak 444\n\n");
  initTimers();
  // ////printf("\n\n akakak 555\n\n");
  if (xTimers == NULL) {

    /* The timer was not created. */
  } else {

    if (xTimerStart(xTimers, 0) != pdPASS) {

      /* The timer could not be set into the Active
      state. */
    }
  }

  // EG91_writeFile("char *fileName", "char *file", 12);
  // init_SDCard();
  // ////printf("\n\n akakak 666\n\n");

  // ////printf("\n\n akakak 888\n\n");
  //   ////printf("\n\nINIT ROUTINES\n\n");
  //   eraseRoutines();
  // initRoutines();
  printf("\n\n akakak 999\n\n");
}

/// @brief Print stack high water mark value
/// @param tag TAG to insert in the print
extern void system_stack_high_water_mark(char *tag) {
  UBaseType_t uxHighWaterMark;
  uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
  if (uxHighWaterMark > 512) {
    ESP_LOGI(tag, "\n\n[GOOD] water mark: %d\n\n", uxHighWaterMark);
  } else {
    ESP_LOGE(tag, "\n\n[ALERT] water mark: %d\n\n", uxHighWaterMark);
  }
}

/**
 * Erases the password from the response payload.
 *
 * This function removes the password from the input payload and returns the
 * modified response payload.
 *
 * @param payload The original payload containing the password.
 * @param rsp_payload The response payload with the password removed.
 *
 * @returns The response payload with the password erased.
 */
char *erase_Password_For_Rsp(char *payload, char *rsp_payload) {

  uint8_t count = 0;
  uint8_t strIndex = 0;

  // ////printf("\n\nrsp_Data1 - %s\n\n", payload);

  for (size_t i = 0; i < strlen(payload); i++) {
    if (payload[i] == ';') {
      count++;

      if (count != 6) {
        if (payload[i] != ':') {
          rsp_payload[strIndex++] = payload[i];
        }
      }
    } else {
      if (count != 6) {
        if (payload[i] != ':') {
          rsp_payload[strIndex++] = payload[i];
        }
      }
    }
  }

  // ////printf("\n\nrsp_Data2 - %s\n\n", rsp_payload);
  return rsp_payload;
}

char *hexToAscii(char hex[]) {
  int hexLength = strlen(hex);
  char *text = NULL;

  if (hexLength > 0) {
    int symbolCount;
    int oddHexCount = hexLength % 2 == 1;
    if (oddHexCount)
      symbolCount = (hexLength / 2) + 1;
    else
      symbolCount = hexLength / 2;

    text = malloc(symbolCount + 1);

    int lastIndex = hexLength - 1;
    for (int i = lastIndex; i >= 0; --i) {
      if (((lastIndex - i) % 2 != 0)) {
        int dec = 16 * valueOf(hex[i]) + valueOf(hex[i + 1]);
        if (oddHexCount)
          text[i / 2 + 1] = dec;
        else
          text[i / 2] = dec;
      } else if (i == 0) {
        int dec = valueOf(hex[0]);
        text[0] = dec;
      }
    }
    text[symbolCount] = '\n';
  }
  return text;
}

int valueOf(char symbol) {
  switch (symbol) {
  case '0':
    return 0;
  case '1':
    return 1;
  case '2':
    return 2;
  case '3':
    return 3;
  case '4':
    return 4;
  case '5':
    return 5;
  case '6':
    return 6;
  case '7':
    return 7;
  case '8':
    return 8;
  case '9':
    return 9;
  case 'A':
  case 'a':
    return 10;
  case 'B':
  case 'b':
    return 11;
  case 'C':
  case 'c':
    return 12;
  case 'D':
  case 'd':
    return 13;
  case 'E':
  case 'e':
    return 14;
  case 'F':
  case 'f':
    return 15;
  default: {
    // ////printf("Cannot decode that symbol: %c", symbol);
    return -1;
  }
  }
}

int base64_decode(const char *input, size_t input_len, unsigned char **output) {
  size_t output_len = 0;

  mbedtls_base64_decode(NULL, 0, &output_len, (const unsigned char *)input,
                        input_len);

  *output = (unsigned char *)malloc(output_len);
  if (*output == NULL) {
    return -1;
  }

  if (mbedtls_base64_decode(*output, output_len, &output_len,
                            (const unsigned char *)input, input_len) != 0) {
    free(*output);
    return -1;
  }

  return output_len;
}

int remove_padding(unsigned char *plaintext, size_t plaintext_length) {
  unsigned char padding_byte = plaintext[plaintext_length - 1];
  size_t padding_length = (size_t)padding_byte;

  if (padding_length > 0 && padding_length <= plaintext_length) {
    for (size_t i = 0; i < padding_length; i++) {
      if (plaintext[plaintext_length - 1 - i] != padding_byte) {
        // Padding inválido
        return -1;
      }
    }

    // Remove o padding
    plaintext_length -= padding_length;
    plaintext[plaintext_length] = '\0';
  }

  return plaintext_length;
}
uint32_t CRC32_FOTA = 0;
unsigned char stream_block[16] = {};

void decrypt_aes_cfb_padding(const unsigned char *input, size_t input_len,
                             unsigned char *output) {

  mbedtls_aes_context aes;

  unsigned char key[] = "C55YOj8C1em3IAKm";

  /*  mbedtls_aes_init(&aes);
   mbedtls_aes_setkey_dec(&aes, key, 128); */

  size_t iv_offset = 0;
  size_t iv_offset1 = 0;

  // Chave de 256 bits (32 bytes)
  char *base64_output = NULL;
  // IV (vetor de inicialização) de 128 bits (16 bytes)
  size_t nc_off1 = 0;

  unsigned char stream_block1[16] = {};
  mbedtls_aes_context ctx;

  // Decodifica o texto base64
  unsigned char *decoded_input = NULL;
  int decoded_len =
      base64_decode((const char *)input, input_len, &decoded_input);
  if (decoded_len < 0) {
    ////printf("Erro ao decodificar base64\n");
    return;
  }

  mbedtls_aes_init(&ctx);
  unsigned char out[2000] = {};
  mbedtls_aes_setkey_enc(&ctx, key, 128);

  // Descriptografa usando o modo CTR
  // mbedtls_aes_crypt_(&aes, MBEDTLS_AES_DECRYPT, input, output);

  // ESP_LOG_BUFFER_HEX("iv iv ", iv, sizeof(iv));
  // ESP_LOG_BUFFER_HEX("stream stream ", stream_block, sizeof(stream_block));
  memset(EG915_readDataFile_struct.receiveData, 0,
         sizeof(EG915_readDataFile_struct.receiveData));
  // mbedtls_aes_crypt_cbc(&ctx,MBEDTLS_AES_DECRYPT,decoded_len,iv,
  // decoded_input, out);
  EG915_readDataFile_struct.decryptSize = decoded_len;
  mbedtls_aes_crypt_ctr(&ctx, decoded_len, &nc_off1, iv, stream_block,
                        decoded_input, &EG915_readDataFile_struct.receiveData);

  // iv[15]++;
  //
  EG915_readDataFile_struct.packetFile_size = decoded_len;
  // memcpy(iv, out + decoded_len - 16, 16);

  // ESP_LOG_BUFFER_HEX("\n cfb", EG915_readDataFile_struct.receiveData,
  // decoded_len);
  /* ESP_LOG_BUFFER_HEX("stream stream ", stream_block, sizeof(stream_block));
   */
  // memcpy(iv,stream_block,sizeof(stream_block));
  memset(stream_block, 0, sizeof(stream_block));

  // Libera a memória alocada para o texto decodificado

  // ESP_LOG_BUFFER_HEX("ofb", EG915_readDataFile_struct.receiveData,
  // decoded_len);

  // Imprime o texto descriptografado

  // mbedtls_aes_crypt_cfb8(&ctx, MBEDTLS_AES_DECRYPT, output_len/* inputLen */,
  // iv,&base64_output /* input */, output);

  mbedtls_aes_free(&ctx);
  // //printf("\n\n cfb out:\n %s \n\n", out);
  free(decoded_input);
  // mbedtls_cipher_free(&ctx);
}

void decrypt_aes_cbc_padding(const unsigned char *input, size_t input_len,
                             unsigned char *output, const unsigned char *key,
                             const unsigned char *iv) {
  mbedtls_aes_context aes_ctx;
  mbedtls_aes_init(&aes_ctx);
  mbedtls_aes_setkey_dec(&aes_ctx, (const unsigned char *)AES_KEY1,
                         AES_KEY_SIZE * 8);

  char *base64_output = NULL; // malloc(strlen(input));

  // ////printf("String decodificada: %s\n", input);

  /*if (base64_output == NULL)
  {
      //ESP_LOGE("TAG", "Falha ao alocar memória para base64_output");
      free(ciphertext);
      return;
  }

  size_t input_len = strlen(base64_output);
  unsigned char *output = NULL; */
  int output_len =
      base64_decode((const char *)input, input_len, &base64_output);

  if (output_len == -1) {
    // ////printf("Erro ao decodificar a string Base64\n");
    return;
  }

  // ////printf("String decodificada: %.*s\n", output_len, base64_output);

  // Print the encrypted text as hexadecimal
  for (size_t i = 0; i < output_len; i++) {
    // ////printf("%02X ", base64_output[i]);
  }

  unsigned char decrypted_data[100] = {};

  size_t block_count = output_len / AES_KEY_SIZE;
  size_t last_block_size = output_len % AES_KEY_SIZE;

  unsigned char prev_block[AES_KEY_SIZE];
  memcpy(prev_block, iv, AES_KEY_SIZE);

  for (size_t i = 0; i < block_count; i++) {
    unsigned char current_block[AES_KEY_SIZE];
    memcpy(current_block, &base64_output[i * AES_KEY_SIZE], AES_KEY_SIZE);

    mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_DECRYPT, current_block, output);

    for (size_t j = 0; j < AES_KEY_SIZE; j++) {
      output[j] ^= prev_block[j];
    }

    memcpy(prev_block, &base64_output[i * AES_KEY_SIZE], AES_KEY_SIZE);
    output += AES_KEY_SIZE;
  }

  if (last_block_size > 0) {
    unsigned char last_block[AES_KEY_SIZE];
    memcpy(last_block, &base64_output[block_count * AES_KEY_SIZE],
           last_block_size);
    memset(&last_block[last_block_size], 0,
           AES_KEY_SIZE -
               last_block_size); // Preenche o restante do bloco com zeros

    mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_DECRYPT, last_block, output);

    for (size_t j = 0; j < last_block_size; j++) {
      output[j] ^= prev_block[j];
    }
  }

  mbedtls_aes_free(&aes_ctx);

  // remove_padding(&output, strlen((char *)output));

  /* unsigned char padding_len = output[input_len - 1];
  size_t plaintext_len = output_len - (size_t)padding_len;
  output[plaintext_len] = '\0'; */

  // ////printf("\n\n  output[plaintext_len] 23232 - %d - %d - %s\n\n",
  // input_len /* plaintext_len */, block_count, output);

  free(base64_output);
}

void add_padding_pkcs7(unsigned char *input, size_t input_len,
                       size_t block_size) {
  unsigned char padding_value = block_size - (input_len % block_size);
  for (size_t i = input_len; i < input_len + padding_value; i++) {
    input[i] = padding_value;
  }
}

char *encrypt_and_base64_encode(const unsigned char *key,
                                const unsigned char *plaintext,
                                size_t plaintext_size) {
  char AES_IV[] = "1BzUeGptJ3SnvnIv";
  mbedtls_aes_context aes_ctx;
  mbedtls_aes_init(&aes_ctx);

  // Configuração da chave de criptografia AES
  mbedtls_aes_setkey_enc(&aes_ctx, key, KEY_SIZE * 8);

  size_t padded_size = ((plaintext_size / BLOCK_SIZE) + 1) * BLOCK_SIZE;
  unsigned char *padded_plaintext = (unsigned char *)malloc(padded_size);
  if (padded_plaintext == NULL) {
    mbedtls_aes_free(&aes_ctx);
    return NULL;
  }

  memcpy(padded_plaintext, plaintext, plaintext_size);
  add_padding_pkcs7(padded_plaintext, plaintext_size, BLOCK_SIZE);

  size_t ciphertext_size = padded_size;
  unsigned char *ciphertext = (unsigned char *)malloc(ciphertext_size);
  if (ciphertext == NULL) {
    free(padded_plaintext);
    mbedtls_aes_free(&aes_ctx);
    return NULL;
  }

  mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_ENCRYPT, ciphertext_size,
                        (unsigned char *)AES_IV, padded_plaintext, ciphertext);

  free(padded_plaintext);

  size_t base64_output_len = 0;
  mbedtls_base64_encode(NULL, 0, &base64_output_len, ciphertext,
                        ciphertext_size);

  char *base64_str = (char *)malloc(base64_output_len + 1);
  if (base64_str == NULL) {
    free(ciphertext);
    mbedtls_aes_free(&aes_ctx);
    return NULL;
  }

  size_t output_len = 0;
  mbedtls_base64_encode((unsigned char *)base64_str, base64_output_len + 1,
                        &output_len, ciphertext, ciphertext_size);
  free(ciphertext);

  base64_str[output_len] = '\0';

  mbedtls_aes_free(&aes_ctx);

  return base64_str;
}

void encrypt_aes_cbc(const unsigned char *input, size_t input_len,
                     unsigned char *output, const unsigned char *key,
                     const unsigned char *iv) {
  mbedtls_aes_context aes_ctx;
  mbedtls_aes_init(&aes_ctx);

  mbedtls_aes_setkey_enc(&aes_ctx, key, 128);

  size_t block_count = (input_len + 15) / 16;

  unsigned char prev_block[16];
  memcpy(prev_block, iv, 16);

  for (size_t i = 0; i < block_count; i++) {
    unsigned char current_block[16];
    size_t block_size = (i == block_count - 1) ? (input_len % 16) : 16;
    memcpy(current_block, &input[i * 16], block_size);

    for (size_t j = 0; j < 16; j++) {
      current_block[j] ^= prev_block[j];
    }

    mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_ENCRYPT, current_block,
                          output + (i * 16));

    memcpy(prev_block, output + (i * 16), 16);
  }

  mbedtls_aes_free(&aes_ctx);
}

uint8_t save_User_Counter_In_Storage(uint32_t value) {
  nvs_set_u32(nvs_System_handle, NVS_KEY_USER_COUNTER, value);

  return 1;
}

uint32_t get_User_Counter_From_Storage() {

  uint32_t value = 0;

  if (nvs_get_u32(nvs_System_handle, NVS_KEY_USER_COUNTER, &value) == ESP_OK) {
    // //printf("\n\n MyUser_List_AllUsers22 - %d\n\n", value);
    return value;
  } else {
    value = MyUser_List_AllUsers();
    nvs_set_u32(nvs_System_handle, NVS_KEY_USER_COUNTER, value);
    // //printf("\n\n MyUser_List_AllUsers - %d\n\n", value);
    return value;
  }

  return 0;
}

char *remove_non_printable(char *str, size_t len) {
  // Create a new empty string to store the modified version of the input string
  char *new_str = malloc(len + 1);
  int i, j = 0;

  // Iterate through each character in the input string
  for (i = 0; i < len; i++) {
    // If the character is printable, add it to the new string
    if (isprint(str[i])) {
      new_str[j++] = str[i];
    }
  }
  new_str[j] = '\0';

  return new_str;
}

void resetProcess() {

  uint8_t Input1_value = 0;
  uint8_t Input2_value = 0;

  Input1_value = !gpio_get_level(GPIO_INPUT_IO_RESET);
  // Input2_value = !gpio_get_level(CONFIG_GPIO_INPUT_1);
  uint8_t resetPinLabel = 0;
  uint8_t actValue = 0;

  inline uint64_t now_time = get_nowTime_in_seconds();

  // printf("\n\nnonow time %lld\n\n", now_time);

  gpio_set_level(GPIO_OUTPUT_ACT, 1);
  while (get_nowTime_in_seconds() < now_time + 10) {
     printf("\n\nnonow time2 %lld\n\n", get_nowTime_in_seconds());
    if (gpio_get_level(GPIO_INPUT_IO_RESET)) {

      resetPinLabel = 1;
    } else {
      resetPinLabel = 0;
      goto endFunction;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }

/*   if (resetPinLabel) {
    resetPinLabel = 0;
    gpio_set_level(GPIO_OUTPUT_ACT, 0);
    vTaskDelay(pdMS_TO_TICKS(5000));

    now_time = 0;
    now_time = get_nowTime_in_seconds();

    while (get_nowTime_in_seconds() < now_time + 5) {
      gpio_set_level(GPIO_OUTPUT_ACT, actValue);
      actValue = ~actValue;
      // printf("\n\nnonow time3 %lld\n\n", get_nowTime_in_seconds());
      if (!gpio_get_level(GPIO_INPUT_IO_RESET)) {
        resetPinLabel = 1;
      } else {
        resetPinLabel = 0;
        goto endFunction;
      }
      vTaskDelay(pdMS_TO_TICKS(200));
    }
  }

  if (resetPinLabel) {
    gpio_set_level(GPIO_OUTPUT_ACT, 0);
    vTaskDelay(pdMS_TO_TICKS(5000));
    label_Reset_Password_OR_System = 0;
    now_time = 0;
    now_time = get_nowTime_in_seconds();

    while (get_nowTime_in_seconds() < now_time + 3) {

      gpio_set_level(GPIO_OUTPUT_ACT, 1);
      //actValue = ~actValue;
      // printf("\n\nnonow time4 %lld\n\n", get_nowTime_in_seconds());
      if (gpio_get_level(GPIO_INPUT_IO_RESET)) {

        resetPinLabel = 1;
      } else {
        resetPinLabel = 0;
        goto endFunction;
      }

      vTaskDelay(pdMS_TO_TICKS(10));
    }
  } */

  if (resetPinLabel) {
    system_Reset();
  } else {
    // printf("\n\n NO RESET\n\n");
  }

endFunction:
  return;
}