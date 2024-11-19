/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|

*/

#include "rele.h"
#include "stdio.h"
// #include "gpio.h"
#include "EG91.h"
#include "ble_spp_server_demo.h"
#include "cmd_list.h"
#include "core.h"
#include "erro_list.h"
#include "esp_gatts_api.h"

#include "routines.h"
#include "sdCard.h"
#include "stdlib.h"

char sdCard_log_relay[100];
char file_RSP[100] = {};

TaskHandle_t rele1_Bistate_Task_Handle;
TaskHandle_t rele2_Bistate_Task_Handle;

uint8_t rele1_Mode_Label;
int rele1_Bistate_Time;
uint8_t rele2_Mode_Label;
int rele2_Bistate_Time;

extern SemaphoreHandle_t rdySem_Control_IncomingCALL;
uint8_t rele1_Restriction;

void task_BiState_Send_RelayState2(void *pvParameter) {
  queue_BLE_Parameters2 =
      xQueueCreate(1, sizeof(/* rsp */ data_BLE_Send_RelayState));
  // //////printf("\nenter task_BiState_Send_RelayState2\n");
  data_BLE_Send_RelayState
      cpy_message; //= (data_BLE_Send_RelayState *)pvParameter;
  char teste[200];
  data_BLE_Send_RelayState cpy_cpy_message;

  // rdySem_Control_Send_RelayState2 = xSemaphoreCreateBinary();
  for (;;) {
    // ////printf("\nenter task_BiState_Send_RelayState2\n");
    memset(&cpy_message, 0, sizeof(cpy_message));
    memset(&cpy_cpy_message, 0, sizeof(cpy_cpy_message));
    if (xQueueReceive(queue_BLE_Parameters2, /* &teste */ &(cpy_cpy_message),
                      portMAX_DELAY)) {
      // //printf("\nsemaphore give 111\n");
      cpy_message = cpy_cpy_message;
      label_MonoStableRelay2 = 1;

      save_INT8_Data_In_Storage(NVS_KEY_RELAY2_LAST_VALUE, 0,
                                nvs_System_handle);
      if (cpy_message.relaynumber == RELE2_NUMBER) {
        if (cpy_message.BLE_SMS_INDICATION != SMS_INDICATION) {
          gpio_set_level(GPIO_OUTPUT_IO_1, 1);
          sprintf(cpy_message.payload, "%s %c %c %d", RELE2_ELEMENT, SET_CMD,
                  RELE_PARAMETER, getRele2());
          // ////printf("\nfeedback relay2- %s - %d\n", cpy_message.payload,
          // cpy_message.BLE_SMS_INDICATION);
          if (label_MonoStableRelay2 == 1) {
            BLE_Broadcast_Notify(cpy_message.payload);
          }

          if (cpy_message.BLE_SMS_INDICATION == UDP_INDICATION) {
            sprintf(cpy_message.mqttInfo_ble.data, "%s", cpy_message.payload);
            send_UDP_queue(&cpy_message.mqttInfo_ble);
          }

          vTaskDelay(pdMS_TO_TICKS((rele2_Bistate_Time * 1000)));

          unsigned int q_size = uxQueueMessagesWaiting(queue_BLE_Parameters2);

          // //printf("\n\nqueue rele 2- %d\n\n", q_size);

          if (q_size < 1) {
            if (label_MonoStableRelay2 == 1) {
              gpio_set_level(GPIO_OUTPUT_IO_1, 0);
            }

            memset(cpy_message.payload, 0, sizeof(cpy_message.payload));
            sprintf(cpy_message.payload, "%s %c %c %d", RELE2_ELEMENT, SET_CMD,
                    RELE_PARAMETER, getRele2());

            // //printf("\nfeedback relay22- %s\n", cpy_message.payload);

            if (cpy_message.BLE_SMS_INDICATION == UDP_INDICATION) {
              sprintf(cpy_message.mqttInfo_ble.data, "%s", cpy_message.payload);
              send_UDP_queue(&cpy_message.mqttInfo_ble);
            }

            BLE_Broadcast_Notify(cpy_message.payload);
            label_MonoStableRelay2 = 0;
          }
        } else if (cpy_message.BLE_SMS_INDICATION == SMS_INDICATION) {
          label_MonoStableRelay2 = 1;

          if (!getRele2()) {

            if (label_MonoStableRelay2 == 1) {
              gpio_set_level(GPIO_OUTPUT_IO_1, 1);
              BLE_Broadcast_Notify("R2 G R 1");
            }

            if (cpy_message.EG91_data.labelIncomingCall != 1 &&
                cpy_message.EG91_data.labelRsp == 1 &&
                label_MonoStableRelay2 == 1) {
              if (getRele2()) {

                sprintf(cpy_message.EG91_data.payload,
                        return_Json_SMS_Data("RELAY_ON"), RELE2_NUMBER, "<03>");
                xQueueSendToBack(queue_EG91_SendSMS,
                                 (void *)&cpy_message.EG91_data,
                                 pdMS_TO_TICKS(1000));
              } else {
                sprintf(cpy_message.EG91_data.payload,
                        return_Json_SMS_Data("RELAY_OFF"), RELE2_NUMBER,
                        "<04>");
                xQueueSendToBack(queue_EG91_SendSMS,
                                 (void *)&cpy_message.EG91_data,
                                 pdMS_TO_TICKS(1000));
              }
            }
          }

          vTaskDelay(pdMS_TO_TICKS(rele2_Bistate_Time * 1000));

          unsigned int q_size = uxQueueMessagesWaiting(queue_BLE_Parameters2);

          // ////printf("\n\nqueue rele - %d\n\n", q_size);

          if (q_size < 1) {
            if (label_MonoStableRelay2 == 1) {
              gpio_set_level(GPIO_OUTPUT_IO_1, 0);
            }

            // //printf("\nfeedback relay2222- %s\n", cpy_message.payload);

            BLE_Broadcast_Notify(cpy_message.payload);

            if (cpy_message.EG91_data.labelIncomingCall != 1 &&
                cpy_message.EG91_data.labelRsp == 1 &&
                label_MonoStableRelay2 == 1) {
              if (getRele2()) {
                // vTaskDelay(pdMS_TO_TICKS(1000));
                sprintf(cpy_message.EG91_data.payload,
                        return_Json_SMS_Data("RELAY_ON"), RELE2_NUMBER, "<03>");
                xQueueSendToBack(queue_EG91_SendSMS,
                                 (void *)&cpy_message.EG91_data,
                                 pdMS_TO_TICKS(1000));
              } else {
                sprintf(cpy_message.EG91_data.payload,
                        return_Json_SMS_Data("RELAY_OFF"), RELE2_NUMBER,
                        "<04>");
                xQueueSendToBack(queue_EG91_SendSMS,
                                 (void *)&cpy_message.EG91_data,
                                 pdMS_TO_TICKS(1000));
              }
            }

            if (label_MonoStableRelay2 == 1) {
              gpio_set_level(GPIO_OUTPUT_IO_1, 0);
              BLE_Broadcast_Notify("R2 G R 0");
            }
            label_MonoStableRelay2 = 0;
          } else {
            sprintf(cpy_message.EG91_data.payload,
                    return_Json_SMS_Data("PULSE_TIME_WAS_RENEWED"),
                    RELE2_NUMBER);
            xQueueSendToBack(queue_EG91_SendSMS, (void *)&cpy_message.EG91_data,
                             pdMS_TO_TICKS(1000));
          }
        } else {
          sprintf(cpy_message.payload, "%s %c %c %s", RELE2_ELEMENT, SET_CMD,
                  RELE_PARAMETER, ERROR_SET);
        }
      }

      // vTaskDelay(pdMS_TO_TICKS((500)));
    }
  }
}

void task_BiState_Send_RelayState1(void *pvParameter) {

  label_MonoStableRelay1 = 0;
  // rdySem_RelayMonoInicial = xSemaphoreCreateBinary();
  // xSemaphoreGive(rdySem_RelayMonoInicial);
  //  xSemaphoreGive(rdySem_RelayMonoInicial);
  rdySem_RelayMonoInicial = xSemaphoreCreateBinary();
  rdySem_RelayMonoStart = xSemaphoreCreateBinary();
  // ////printf("sem 11");
  xSemaphoreGive(rdySem_RelayMonoStart);
  // printf("sem 22\n");
  uint8_t timeout = 0;

  queue_BLE_Parameters1 = xQueueCreate(1, sizeof(data_BLE_Send_RelayState));

  data_BLE_Send_RelayState
      cpy_message; //= (data_BLE_Send_RelayState *)pvParameter;
  data_BLE_Send_RelayState cpy_cpy_message;

  for (;;) {
    memset(&cpy_message, 0, sizeof(cpy_message));
    memset(&cpy_cpy_message, 0, sizeof(cpy_cpy_message));

    if (xQueueReceive(queue_BLE_Parameters1, &(cpy_cpy_message),
                      portMAX_DELAY)) {
      cpy_message = cpy_cpy_message;

      label_MonoStableRelay1 = 1;
      printf("\nfeedback relay hohoho\n");
      save_INT8_Data_In_Storage(NVS_KEY_RELAY1_LAST_VALUE, 0,
                                nvs_System_handle);

      if (cpy_message.relaynumber == RELE1_NUMBER) {
        if (cpy_message.BLE_SMS_INDICATION != SMS_INDICATION) {
          if (label_MonoStableRelay1 == 1) {
            gpio_set_level(GPIO_OUTPUT_IO_0, 1);
          }

          sprintf(cpy_message.payload, "%s %c %c %d", RELE1_ELEMENT, SET_CMD,
                  RELE_PARAMETER, getRele1());

          // printf("\nfeedback relay 11- %s\n", cpy_message.payload);

          BLE_Broadcast_Notify(cpy_message.payload);

          // char UDP_message[1024] = {};

          // sprintf(UDP_message, "%s", cpy_message.payload);

          if (cpy_message.BLE_SMS_INDICATION == UDP_INDICATION) {

            sprintf(cpy_message.mqttInfo_ble.data, "%s", cpy_message.payload);
            // printf("\nfeedback relay6565 22- %s - %s\n",
            // cpy_message.mqttInfo_ble.data, cpy_message.mqttInfo_ble.topic);
            send_UDP_queue(&(cpy_message.mqttInfo_ble));
          }

          // xQueueSendToBack(UDP_Send_queue, (void *)&UDP_message,
          // pdMS_TO_TICKS(1000));

          // ////printf("\nfeedback relay 22- %s\n", cpy_message.payload);

          vTaskDelay(pdMS_TO_TICKS((rele1_Bistate_Time * 1000)));

          unsigned int q_size = uxQueueMessagesWaiting(queue_BLE_Parameters1);

          // //printf("\n\nqueue rele 2 - %d\n\n", q_size);

          if (q_size < 1) {
            if (label_MonoStableRelay1 == 1) {
              gpio_set_level(GPIO_OUTPUT_IO_0, 0);
            }

            memset(cpy_message.payload, 0, sizeof(cpy_message.payload));
            sprintf(cpy_message.payload, "%s %c %c %d", RELE1_ELEMENT, SET_CMD,
                    RELE_PARAMETER, getRele1());
            // //printf("\nfeedback relay33- %s\n", cpy_message.payload);

            BLE_Broadcast_Notify(cpy_message.payload);

            // sprintf(UDP_message, "%s", cpy_message.payload);

            if (cpy_message.BLE_SMS_INDICATION == UDP_INDICATION) {
              sprintf(cpy_message.mqttInfo_ble.data, "%s", cpy_message.payload);
              send_UDP_queue(&cpy_message.mqttInfo_ble);
            }
            // xQueueSendToBack(UDP_Send_queue, (void *)&UDP_message,
            // pdMS_TO_TICKS(1000));

            label_MonoStableRelay1 = 0;
          }
        } else if (cpy_message.BLE_SMS_INDICATION == SMS_INDICATION) {
          if (cpy_message.EG91_data.labelIncomingCall == 1) {
            // vTaskDelay(pdMS_TO_TICKS(500));
            //  giveSem_CtrIncomingCall();
          }

          if (!getRele1()) {
            if (label_MonoStableRelay1 == 1) {

              gpio_set_level(GPIO_OUTPUT_IO_0, 1);
              BLE_Broadcast_Notify("R1 G R 1");
            }

            if (cpy_message.EG91_data.labelIncomingCall != 1 &&
                cpy_message.EG91_data.labelRsp == 1) {
              if (getRele1()) {

                sprintf(cpy_message.EG91_data.payload,
                        return_Json_SMS_Data("RELAY_ON"), RELE1_NUMBER, "<01>");
                xQueueSendToBack(queue_EG91_SendSMS,
                                 (void *)&cpy_message.EG91_data,
                                 pdMS_TO_TICKS(1000));
                // send_UDP_queue("R1 G R 1");
              } else {

                sprintf(cpy_message.EG91_data.payload,
                        return_Json_SMS_Data("RELAY_OFF"), RELE1_NUMBER),
                    "<02>";
                xQueueSendToBack(queue_EG91_SendSMS,
                                 (void *)&cpy_message.EG91_data,
                                 pdMS_TO_TICKS(1000));
                // send_UDP_queue("R1 G R 0");
              }
            }
          }

          vTaskDelay(pdMS_TO_TICKS(rele1_Bistate_Time * 1000));

          unsigned int q_size = uxQueueMessagesWaiting(queue_BLE_Parameters1);

          // ////printf("\n\nqueue rele - %d\n\n", q_size);

          if (q_size < 1) {
            if (label_MonoStableRelay1 == 1) {
              gpio_set_level(GPIO_OUTPUT_IO_0, 0);
            }

            BLE_Broadcast_Notify("R1 G R 0");
            // ////printf("\n\n before PULSE_TIME_WAS_RENEWED11\n\n");
            if (cpy_message.EG91_data.labelIncomingCall != 1 &&
                cpy_message.EG91_data.labelRsp == 1) {

              // ////printf("\n\n before PULSE_TIME_WAS_RENEWED22\n\n");
              if (getRele1()) {
                // vTaskDelay(pdMS_TO_TICKS(1000));
                // send_UDP_queue("R1 G R 1");
                sprintf(cpy_message.EG91_data.payload,
                        return_Json_SMS_Data("RELAY_ON"), RELE1_NUMBER, "<01>");
                xQueueSendToBack(queue_EG91_SendSMS,
                                 (void *)&cpy_message.EG91_data,
                                 pdMS_TO_TICKS(1000));
              } else {
                // send_UDP_queue("R1 G R 0");
                sprintf(cpy_message.EG91_data.payload,
                        return_Json_SMS_Data("RELAY_OFF"), RELE1_NUMBER,
                        "<02>");
                xQueueSendToBack(queue_EG91_SendSMS,
                                 (void *)&cpy_message.EG91_data,
                                 pdMS_TO_TICKS(1000));
              }
            }
          } else {
            // ////printf("\n\n before PULSE_TIME_WAS_RENEWED33\n\n");
            sprintf(cpy_message.EG91_data.payload,
                    return_Json_SMS_Data("PULSE_TIME_WAS_RENEWED"),
                    RELE1_NUMBER);

            if (cpy_message.EG91_data.labelIncomingCall == 1) {
              // ////printf("\n\n before PULSE_TIME_WAS_RENEWED44\n\n");
              give_rdySem_Control_Send_AT_Command();
              //    xSemaphoreGive(rdySem_Control_Send_AT_Command);
            } else {
              // ////printf("\n\n before PULSE_TIME_WAS_RENEWED55\n\n");
              xQueueSendToBack(queue_EG91_SendSMS,
                               (void *)&cpy_message.EG91_data,
                               pdMS_TO_TICKS(1000));
            }
          }
        }
      }

    } else if (cpy_message.relaynumber == RELE2_NUMBER) {
      sprintf(cpy_message.payload, "%s %c %c %s", RELE1_ELEMENT, SET_CMD,
              RELE_PARAMETER, ERROR_SET);
    }
  }
  // vTaskDelay(pdMS_TO_TICKS(1000));
}

char *configuration_Relay_Parameter(char *payload, uint8_t BLE_SMS,
                                    mqtt_information *mqttInfo) {
  uint8_t found_Count = 0;
  char relayTIME_1[10] = {};
  char relayTIME_2[10] = {};
  char relayMODE_1[2] = {};
  char relayMODE_2[2] = {};
  char relayRestriction_1[2] = {};
  uint8_t aux_ERROR = 0;
  uint8_t dot_Counter = 0;
  uint8_t aux_DotCounter = 0;

  for (size_t i = 0; i < strlen(payload); i++) {
    payload[i] = toupper(payload[i]);
    if (payload[i] == ';') {
      dot_Counter++;
    }
  }

  // ////printf("\n\nconfiguration_Relay_Parameter\n\n");

  dot_Counter = dot_Counter + 1;

  int index_Found = -1;
  uint8_t strIndex = 0;

  index_Found = strpos(payload, "T1=");

  if (index_Found != -1) {

    for (size_t i = index_Found + 3; i < strlen(payload); i++) {
      if (payload[i] == ';' || strIndex > 5) {

        if (strIndex > 5) {
          return return_Json_SMS_Data("ERROR_INPUT_DATA");
        }

        break;
      }
      relayTIME_1[strIndex++] = payload[i];
    }

    if (atoi(relayTIME_1) > RELE_BISTATE_MAX_TIME || strIndex > 5) {
      return return_Json_SMS_Data("ERROR_INPUT_DATA");
    }

    for (size_t i = 0; i < strlen(relayTIME_1); i++) {
      if (relayTIME_1[i] < '0' || relayTIME_1[i] > ' 9') {
        return return_Json_SMS_Data("ERROR_INPUT_DATA");
      }
    }

    found_Count |= 16;
    // aux_DotCounter = aux_DotCounter + 1;
    // ////printf("\n\nrelayTIME_1 - %s , %d\n\n", relayTIME_1,
    // atoi(relayTIME_1));
  } else {

    aux_DotCounter++;
  }

  aux_ERROR = 0;
  index_Found = -1;
  strIndex = 0;
  index_Found = strpos(payload, "T2=");

  if (index_Found != -1) {

    for (size_t i = index_Found + 3; i < strlen(payload); i++) {
      if (payload[i] == ';' || strIndex > 5) {
        if (strIndex > 5) {
          return return_Json_SMS_Data("ERROR_INPUT_DATA");
        }
        break;
      }

      relayTIME_2[strIndex++] = payload[i];
    }

    if (atoi(relayTIME_2) > RELE_BISTATE_MAX_TIME || strIndex > 5) {
      return return_Json_SMS_Data("ERROR_INPUT_DATA");
    }

    for (size_t i = 0; i < strlen(relayTIME_2); i++) {
      if (relayTIME_2[i] < '0' || relayTIME_2[i] > ' 9') {
        return return_Json_SMS_Data("ERROR_INPUT_DATA");
      }
    }

    found_Count |= 8;
    // ////printf("\n\nrelayTIME_2 - %s , %d\n\n", relayTIME_2,
    // atoi(relayTIME_2));
  } else {
    aux_DotCounter++;
  }

  index_Found = -1;
  strIndex = 0;
  index_Found = strpos(payload, "M1=");

  if (index_Found != -1) {
    // found_Count++;
    for (size_t i = index_Found + 3; i < strlen(payload); i++) {
      if (payload[i] == ';' || strIndex > 2) {
        break;
      }

      if (strIndex == 1) {
        return return_Json_SMS_Data("ERROR_INPUT_DATA");
      }

      relayMODE_1[strIndex++] = payload[i];
    }

    if ((relayMODE_1[0] == '1' || relayMODE_1[0] == '0') &&
        strlen(relayMODE_1) == 1) {
      // aux_DotCounter = aux_DotCounter + 1;
      found_Count |= 4;
      // ////printf("\n\nrelayMODE_1 - %s , %d\n\n", relayMODE_1,
      // atoi(relayMODE_1));
    } else {
      return return_Json_SMS_Data("ERROR_INPUT_DATA");
    }
  } else {
    aux_DotCounter++;
  }

  index_Found = -1;
  strIndex = 0;
  index_Found = strpos(payload, "M2=");

  if (index_Found != -1) {
    // found_Count++;
    for (size_t i = index_Found + 3; i < strlen(payload); i++) {
      if (payload[i] == ';' || strIndex > 2) {
        break;
      }

      if (strIndex == 1) {
        return return_Json_SMS_Data("ERROR_INPUT_DATA");
      }

      relayMODE_2[strIndex++] = payload[i];
    }

    if ((relayMODE_2[0] == '1' || relayMODE_2[0] == '0') &&
        strlen(relayMODE_2) == 1) {
      // aux_DotCounter = aux_DotCounter + 1;
      found_Count |= 2;
      // ////printf("\n\nrelayMODE_2 - %s , %d\n\n", relayMODE_2,
      // atoi(relayMODE_2));
    } else {
      return return_Json_SMS_Data("ERROR_INPUT_DATA");
    }
  } else {
    aux_DotCounter++;
  }

  index_Found = -1;
  strIndex = 0;

  index_Found = strpos(payload, "B=");

  if (index_Found != -1) {
    // found_Count++;
    for (size_t i = index_Found + 2; i < strlen(payload); i++) {
      if (payload[i] == ';' || strIndex > 2) {
        break;
      }

      if (strIndex == 1) {
        return return_Json_SMS_Data("ERROR_INPUT_DATA");
      }

      relayRestriction_1[strIndex++] = payload[i];
    }

    if ((relayRestriction_1[0] == '1' || relayRestriction_1[0] == '0') &&
        strlen(relayRestriction_1) == 1) {
      // ////printf("\n\relayRestriction_1 - %s , %d\n\n", relayRestriction_1,
      // atoi(relayRestriction_1));

      found_Count |= 1;
      // aux_DotCounter = aux_DotCounter + 1;
      index_Found = -1;
      strIndex = 0;
    } else {
      return return_Json_SMS_Data("ERROR_INPUT_DATA");
    }
  } else {
    aux_DotCounter++;
  }

  if ((dot_Counter + aux_DotCounter) == 5) {
    if (found_Count == 0) {
      return_Json_SMS_Data("ERROR_INPUT_DATA");
    }

    if (((found_Count & 16) >> 4) == 1) {

      // resetRele1();
      rele1_Bistate_Time = atoi(relayTIME_1);
      // ////printf("\n\n rele1_Bistate_Time asas - %d\n\n",
      // rele1_Bistate_Time);
      nvs_set_u32(nvs_System_handle, NVS_RELAY1_BISTATE_TIME,
                  rele1_Bistate_Time);
      // save_STR_Data_In_Storage(NVS_RELAY1_BISTATE_TIME, relayTIME_1,
      // nvs_System_handle);
    }

    // ////printf("\n\nfound_Count1 - %d\n\n", found_Count);
    if (((found_Count & 8) >> 3) == 1) {
      // resetRele2();
      rele2_Bistate_Time = atoi(relayTIME_2);
      // ////printf("\n\n rele2_Bistate_Time asas - %d\n\n",
      // rele2_Bistate_Time);
      nvs_set_u32(nvs_System_handle, NVS_RELAY2_BISTATE_TIME,
                  rele2_Bistate_Time);
      // save_STR_Data_In_Storage(NVS_RELAY2_BISTATE_TIME, relayTIME_2,
      // nvs_System_handle);
    }
    // ////printf("\n\nfound_Count2 - %d\n\n", found_Count);
    if (((found_Count & 4) >> 2) == 1) {
      // resetRele1();
      rele1_Mode_Label = atoi(relayMODE_1);
      save_INT8_Data_In_Storage(NVS_KEY_RELAY1_LAST_VALUE, 0,
                                nvs_System_handle);
      // ////printf("\n\n rele1_Mode_Label asas - %d\n\n", rele1_Mode_Label);
      save_INT8_Data_In_Storage(NVS_RELAY1_MODE, rele1_Mode_Label,
                                nvs_System_handle);
    }
    // ////printf("\n\nfound_Count3 - %d\n\n", found_Count);
    if (((found_Count & 2) >> 1) == 1) {
      // resetRele2();
      rele2_Mode_Label = atoi(relayMODE_2);
      save_INT8_Data_In_Storage(NVS_KEY_RELAY2_LAST_VALUE, 0,
                                nvs_System_handle);
      // ////printf("\n\n rele2_Mode_Label asas - %d\n\n", rele2_Mode_Label);
      save_INT8_Data_In_Storage(NVS_RELAY2_MODE, rele2_Mode_Label,
                                nvs_System_handle);
    }

    // ////printf("\n\nfound_Count4 - %d\n\n", found_Count);
    if (found_Count & 1) {
      // resetRele1();
      rele1_Restriction = atoi(relayRestriction_1);
      // ////printf("\n\n rele1_Restriction asas - %d\n\n", rele1_Restriction);
      save_INT8_Data_In_Storage(NVS_KEY_RELAY_PAIRING_VALUE, rele1_Restriction,
                                nvs_System_handle);
    }

    if (BLE_SMS == BLE_INDICATION || BLE_SMS == UDP_INDICATION) {
      memset(file_RSP, 0, sizeof(file_RSP));

      sprintf(file_RSP, "%s %s", "ME S D", payload);
      return file_RSP;
    } else if (BLE_SMS == SMS_INDICATION) {
      char UDP_Rsp[100] = {};
      // printf("\n\nfound_Count - %s\n\n", mqttInfo->data);
      sprintf(mqttInfo->data, "%s %s", "ME S D", payload);
      // printf("\n\nfound_Count - %d\n\n", found_Count);
      sprintf(mqttInfo->topic, "%s", "");
      // printf("\n\nfound_Count - %d\n\n", found_Count);
      //  send_UDP_Send(UDP_Rsp);
      send_UDP_queue(mqttInfo);

      return return_Json_SMS_Data("NEW_RELAYS_CONFIGURATION");
    }
  }

  return return_Json_SMS_Data("ERROR_INPUT_DATA");
  ;
}

char *getUserApp_id(char *topic) {
  char id[25] = {};
  uint8_t count = 0;
  uint8_t strIndex = 0;
  sprintf(id, "%s", topic);
  memset(topic, 0, 25);

  for (size_t i = 0; i < strlen(id); i++) {
    if (id[i] == '/') {
      count++;
    } else if (count == 2) {
      topic[strIndex++] = id[i];
    }
  }

  // printf("\n\n get user topic %s\n\n",topic);
  return topic;
}

char *parse_ReleData(uint8_t BLE_SMS_Indication, uint8_t releNumber, char cmd,
                     char param, char *phPassword, char *payload,
                     MyUser *user_validateData, uint8_t gattsIF,
                     uint16_t connID, uint16_t handle_table,
                     data_EG91_Send_SMS *data_SMS, mqtt_information *mqttInfo) {
  // ////printf("\n parse relay data inicial\n");
  //   char *sdCard_Date;
  char *rsp = NULL; /* = (char*) malloc(200 * sizeof(char)); */

  /*memset(&rsp, 0, sizeof(rsp));*/
  memset(&sdCard_log_relay, 0, sizeof(sdCard_log_relay));
  printf("pass (11) %s\n", user_validateData->key);
  // ////printf("bistate time = %d\n", rele1_Bistate_Time);
  // ////printf("\n\n SD CARD BI STATE 555- %s  /
  // %s\n\n",user_validateData->firstName,user_validateData->phone);

  if (cmd == SET_CMD) {
    // if ((label_MonoStableRelay1 != 1 && releNumber == RELE1_NUMBER) ||
    // (label_MonoStableRelay2 != 1 && releNumber == RELE2_NUMBER))
    // {
    // printf("\n\n ENTER IN MONOSTABLE CHECK\n\n");
    // printf("\n\n logs_struct.type SMS 11 %d\n\n", BLE_SMS_Indication);
    /* if (BLE_SMS_Indication == REX_INDICATION) {
      asprintf(phPassword, "%s", "REX"); // vsnprintf(, const char *restrict
    format, ...)
    } */
    printUser(*user_validateData);
    if (validate_DataUser(user_validateData, phPassword)) {
      // free(phPassword);
      // printf("\n after validate user reles\n");
      // ////printf("\n\n logs_struct.type SMS 22 %d\n\n", BLE_SMS_Indication);
      if (param == RELE_PARAMETER) {
        data_BLE_Send_RelayState message;
        message.connID = connID;
        message.gattsIF = gattsIF;
        message.handle_table = handle_table;
        message.relaynumber = releNumber;

        sdCard_Logs_struct logs_struct;
        memset(&logs_struct, 0, sizeof(logs_struct));

        if (label_Routine1_ON == 1 && releNumber == RELE1_NUMBER) {
          sprintf(logs_struct.phone, "%s", user_validateData->phone);

          if (user_validateData->permition == '2') {
            label_Routine1_ON = 0;
            save_INT8_Data_In_Storage(NVS_KEY_ROUTINE1_LABEL, 0,
                                      nvs_System_handle);
            save_INT8_Data_In_Storage(NVS_KEY_ROUTINE1_TIME_ON, 0,
                                      nvs_System_handle);

            resetRele1();
            save_INT8_Data_In_Storage(NVS_KEY_ROUTINE1_LABEL, 0,
                                      nvs_System_handle);
            BLE_Broadcast_Notify("R1 S R 0");

            sdCard_Logs_struct logs_struct;
            logs_struct.mqttLogs_info = *mqttInfo;
            memset(&logs_struct, 0, sizeof(logs_struct));

            if (BLE_SMS_Indication == BLE_INDICATION) {
              sprintf(logs_struct.type, "%s", "BLE");
            } else if (BLE_SMS_Indication == SMS_INDICATION) {
              // ////printf("\n\n logs_struct.type SMS 22\n\n");
              sprintf(logs_struct.type, "%s", "SMS");
            } else if (BLE_SMS_Indication == UDP_INDICATION) {
              // ////printf("\n\n logs_struct.type SMS 22\n\n");
              sprintf(logs_struct.phone, "%s", getUserApp_id(mqttInfo->topic));
              sprintf(logs_struct.type, "%s", "WEB");
            } else if (BLE_SMS_Indication == WIEGAND_INDICATION) {
              sprintf(logs_struct.type, "%s", mqttInfo->data);
              sprintf(logs_struct.phone, "%s", user_validateData->wiegand_code);
            } else if (BLE_SMS_Indication == RF_INDICATION) {
              sprintf(logs_struct.type, "%s", "RF");
              sprintf(logs_struct.phone, "%s", user_validateData->rf_serial);
            }

            sprintf(logs_struct.name, "%s",
                    return_Json_SMS_Data("LOGS_DISABLE_ROUTINE"));

            sprintf(logs_struct.relay, "%s", "R1");

            sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("OFF"));

            if (get_RTC_System_Time()) {
              sprintf(logs_struct.date, "%s",
                      replace_Char_in_String(nowTime.strTime, ',', ';'));
            } else {
              sprintf(logs_struct.date, "%s", "0,0");
              sprintf(logs_struct.error, "%s",
                      return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));
            }

            sdCard_Write_LOGS(&logs_struct);

            asprintf(&rsp, "%s", return_Json_SMS_Data("DISABLE_ROUTINE_1"));
            return rsp;
          } else {
            sdCard_Logs_struct logs_struct;
            memset(&logs_struct, 0, sizeof(logs_struct));

            logs_struct.mqttLogs_info = *mqttInfo;
            sprintf(logs_struct.name, "%s", user_validateData->firstName);

            sprintf(logs_struct.relay, "R%d", RELE1_NUMBER);
            sprintf(logs_struct.relay_state, "%s",
                    return_Json_SMS_Data("NOT_CHANGE"));

            if (BLE_SMS_Indication == BLE_INDICATION) {
              sprintf(logs_struct.type, "%s", "BLE");
            } else if (BLE_SMS_Indication == SMS_INDICATION) {
              sprintf(logs_struct.type, "%s", "SMS");
            } else if (BLE_SMS_Indication == UDP_INDICATION) {
              sprintf(logs_struct.phone, "%s", getUserApp_id(mqttInfo->topic));
              sprintf(logs_struct.type, "%s", "WEB");

            } else if (BLE_SMS_Indication == WIEGAND_INDICATION) {
              sprintf(logs_struct.type, "%s", mqttInfo->data);
              sprintf(logs_struct.phone, "%s", user_validateData->wiegand_code);
            } else if (BLE_SMS_Indication == RF_INDICATION) {
              sprintf(logs_struct.type, "%s", "RF");
              sprintf(logs_struct.phone, "%s", user_validateData->rf_serial);
            } else if (BLE_SMS_Indication == REX_INDICATION) {
              return "OK";
            }

            if (get_RTC_System_Time()) {
              sprintf(logs_struct.date, "%s",
                      replace_Char_in_String(nowTime.strTime, ',', ';'));
              sprintf(logs_struct.error, "%s",
                      return_Json_SMS_Data(
                          "ERROR_LOGS_IS_RUNNING_ROUTINE_ON_RELAY"));
            } else {

              sprintf(logs_struct.date, "%s", "0;0");
              sprintf(logs_struct.error, "%s",
                      return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));
              strcat(logs_struct.error, "/\0");
              strcat(logs_struct.error,
                     return_Json_SMS_Data(
                         "ERROR_LOGS_IS_RUNNING_ROUTINE_ON_RELAY"));
            }

            sdCard_Write_LOGS(&logs_struct);

            return return_ERROR_Codes(
                &rsp,
                return_Json_SMS_Data("ERROR_IS_RUNNING_ROUTINE_ON_RELAY"));
          }
        }

        if (label_Routine2_ON == 1 && releNumber == RELE2_NUMBER) {
          sprintf(logs_struct.phone, "%s", user_validateData->phone);

          if (user_validateData->permition == '2') {
            label_Routine2_ON = 0;
            save_INT8_Data_In_Storage(NVS_KEY_ROUTINE2_LABEL, 0,
                                      nvs_System_handle);
            save_INT8_Data_In_Storage(NVS_KEY_ROUTINE2_TIME_ON, 0,
                                      nvs_System_handle);

            resetRele2();
            BLE_Broadcast_Notify("R2 S R 0");
            save_INT8_Data_In_Storage(NVS_KEY_ROUTINE2_LABEL, 0,
                                      nvs_System_handle);
            sdCard_Logs_struct logs_struct;
            memset(&logs_struct, 0, sizeof(logs_struct));

            if (BLE_SMS_Indication == BLE_INDICATION) {
              sprintf(logs_struct.type, "%s", "BLE");
            } else if (BLE_SMS_Indication == SMS_INDICATION) {
              // ////printf("\n\n logs_struct.type SMS aaaa\n\n");
              sprintf(logs_struct.type, "%s", "SMS");
            } else if (BLE_SMS_Indication == UDP_INDICATION) {
              sprintf(logs_struct.phone, "%s", getUserApp_id(mqttInfo->topic));
              sprintf(logs_struct.type, "%s", "WEB");
            } else if (BLE_SMS_Indication == WIEGAND_INDICATION) {
              sprintf(logs_struct.type, "%s", mqttInfo->data);
              sprintf(logs_struct.phone, "%s", user_validateData->wiegand_code);
            } else if (BLE_SMS_Indication == RF_INDICATION) {
              sprintf(logs_struct.type, "%s", "RF");
              sprintf(logs_struct.phone, "%s", user_validateData->rf_serial);
            }

            sprintf(logs_struct.name, "%s",
                    return_Json_SMS_Data("LOGS_DISABLE_ROUTINE"));

            sprintf(logs_struct.relay, "%s", "R2");

            sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("OFF"));

            if (get_RTC_System_Time()) {
              sprintf(logs_struct.date, "%s",
                      replace_Char_in_String(nowTime.strTime, ',', ';'));
            } else {
              sprintf(logs_struct.date, "%s", "0,0");
              sprintf(logs_struct.error, "%s",
                      return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));
            }

            sdCard_Write_LOGS(&logs_struct);

            asprintf(&rsp, "%s", return_Json_SMS_Data("DISABLE_ROUTINE_2"));
            return rsp;
          } else {
            sdCard_Logs_struct logs_struct;
            memset(&logs_struct, 0, sizeof(logs_struct));
            if (BLE_SMS_Indication == BLE_INDICATION) {
              sprintf(logs_struct.type, "%s", "BLE");
            } else if (BLE_SMS_Indication == SMS_INDICATION) {
              sprintf(logs_struct.type, "%s", "SMS");
            } else if (BLE_SMS_Indication == UDP_INDICATION) {
              sprintf(logs_struct.phone, "%s", getUserApp_id(mqttInfo->topic));
              sprintf(logs_struct.type, "%s", "WEB");
            } else if (BLE_SMS_Indication == WIEGAND_INDICATION) {
              sprintf(logs_struct.type, "%s", mqttInfo->data);
              sprintf(logs_struct.phone, "%s", user_validateData->wiegand_code);
            } else if (BLE_SMS_Indication == RF_INDICATION) {
              sprintf(logs_struct.type, "%s", "RF");
              sprintf(logs_struct.phone, "%s", user_validateData->rf_serial);
            } else if (BLE_SMS_Indication == REX_INDICATION) {
              return "OK";
            }

            sprintf(logs_struct.name, "%s", user_validateData->firstName);

            sprintf(logs_struct.relay, "R%d", RELE2_NUMBER);
            sprintf(logs_struct.relay_state, "%s",
                    return_Json_SMS_Data("NOT_CHANGE"));

            if (get_RTC_System_Time()) {
              sprintf(logs_struct.date, "%s",
                      replace_Char_in_String(nowTime.strTime, ',', ';'));
              sprintf(logs_struct.error, "%s",
                      return_Json_SMS_Data(
                          "ERROR_LOGS_IS_RUNNING_ROUTINE_ON_RELAY"));
            } else {

              sprintf(logs_struct.date, "%s", "0;0");
              sprintf(logs_struct.error, "%s",
                      return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));
              strcat(logs_struct.error, "/\0");
              strcat(logs_struct.error,
                     return_Json_SMS_Data(
                         "ERROR_LOGS_IS_RUNNING_ROUTINE_ON_RELAY"));
            }

            sdCard_Write_LOGS(&logs_struct);

            return return_ERROR_Codes(
                &rsp,
                return_Json_SMS_Data("ERROR_IS_RUNNING_ROUTINE_ON_RELAY"));
          }
        }

        if ((user_validateData->permition == '0' &&
             user_validateData->relayPermition - 48 == releNumber) /*&&
             BLE_SMS_Indication != WIEGAND_INDICATION */) {
          if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD) ||
              network_Activate_Flag == 1) {

            sdCard_Logs_struct logs_struct;
            memset(&logs_struct, 0, sizeof(logs_struct));

            sprintf(logs_struct.type, "%s", "BLE");
            sprintf(logs_struct.name, "%s", user_validateData->firstName);
            sprintf(logs_struct.phone, "%s", user_validateData->phone);
            sprintf(logs_struct.relay, "R%d", releNumber);
            sprintf(logs_struct.relay_state, "%s",
                    return_Json_SMS_Data("NOT_CHANGE"));

            if (get_RTC_System_Time()) {
              sprintf(logs_struct.date, "%s",
                      replace_Char_in_String(nowTime.strTime, ',', ';'));
              sprintf(logs_struct.error, "%s",
                      return_Json_SMS_Data(
                          "ERROR_LOGS_NOT_HAVE_PERMITION_THIS_RELAY"));
            } else {

              sprintf(logs_struct.date, "%s", "0;0");
              sprintf(logs_struct.error, "%s",
                      return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));
              strcat(logs_struct.error, "/\0");
              strcat(logs_struct.error,
                     return_Json_SMS_Data(
                         "ERROR_LOGS_NOT_HAVE_PERMITION_THIS_RELAY"));
            }

            sdCard_Write_LOGS(&logs_struct);
          }

          if (BLE_SMS_Indication != SMS_INDICATION) {

            return return_ERROR_Codes(&rsp,
                                      "ERROR USER RELE NOT PERMITION\n<57>");
          } else if (BLE_SMS_Indication == SMS_INDICATION) {
            return return_ERROR_Codes(
                &rsp,
                return_Json_SMS_Data("ERROR_NOT_HAVE_PERMITION_THIS_RELAY"));
          }
        }

        /*  if (fd_configurations.alarmMode.A == 1)
         {
             if (releNumber == RELE1_NUMBER &&
         feedback_SMS_Data.Alarm_I1_Send_SMS_Parameters.relay1_Activation == 1)
             {
                 if (BLE_SMS_Indication == BLE_INDICATION)
                 {
                    return return_ERROR_Codes(&rsp, "ERROR SYSTEM IN ALARM
         TIMEOUT");
                 }
                 else if (BLE_SMS_Indication == SMS_INDICATION)
                 {
                     return return_ERROR_Codes(&rsp, "ERROR ALARM HAS BEEN
         ACTIVATED AND IS STILL WITHIN THE LIMIT TIME");
                 }
                 else
                 {
                     return return_ERROR_Codes(&rsp, ERROR_SET);
                 }
             }
         } */

        if (rele1_Restriction == 1 && user_validateData->permition == '0' &&
            releNumber == 1) {
          sprintf(logs_struct.phone, "%s", user_validateData->phone);

          if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD) ||
              network_Activate_Flag == 1) {

            sdCard_Logs_struct logs_struct;
            memset(&logs_struct, 0, sizeof(logs_struct));

            if (BLE_SMS_Indication == BLE_INDICATION) {
              sprintf(logs_struct.type, "%s", "BLE");
            } else if (BLE_SMS_Indication == SMS_INDICATION) {
              sprintf(logs_struct.type, "%s", "SMS");
            } else if (BLE_SMS_Indication == UDP_INDICATION) {
              sprintf(logs_struct.phone, "%s", getUserApp_id(mqttInfo->topic));
              sprintf(logs_struct.type, "%s", "WEB");
            } else if (BLE_SMS_Indication == WIEGAND_INDICATION) {
              sprintf(logs_struct.type, "%s", mqttInfo->data);
              sprintf(logs_struct.phone, "%s", user_validateData->wiegand_code);
            } else if (BLE_SMS_Indication == RF_INDICATION) {
              sprintf(logs_struct.type, "%s", "RF");
              sprintf(logs_struct.phone, "%s", user_validateData->rf_serial);
            } else if (BLE_SMS_Indication == REX_INDICATION) {
              return "OK";
            }

            sprintf(logs_struct.name, "%s", user_validateData->firstName);

            sprintf(logs_struct.relay, "%s", "R1");
            sprintf(logs_struct.relay_state, "%s",
                    return_Json_SMS_Data("NOT_CHANGE"));

            if (get_RTC_System_Time()) {
              sprintf(logs_struct.date, "%s",
                      replace_Char_in_String(nowTime.strTime, ',', ';'));
              sprintf(logs_struct.error, "%s",
                      return_Json_SMS_Data("LOGS_ONLY_PERMISSION_TO_CALL"));
            } else {

              sprintf(logs_struct.date, "%s", "0;0");
              sprintf(logs_struct.error, "%s",
                      return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));
              strcat(logs_struct.error, "/\0");
              strcat(logs_struct.error,
                     return_Json_SMS_Data("LOGS_ONLY_PERMISSION_TO_CALL"));
            }

            sdCard_Write_LOGS(&logs_struct);
          }

          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ONLY_PERMISSION_TO_CALL"));
        }

        // printf("\n\n logs_struct.type SMS 22 %d\n\n", BLE_SMS_Indication);

        if (setReles(releNumber, BLE_SMS_Indication, gattsIF, connID,
                     handle_table, data_SMS, mqttInfo)) {

          /* if (EG91_send_AT_Command("AT+QLTS=2", "QLTS", 1000))
          {
              asprintf(&sdCard_Date, "%s", nowTime.strTime);
          }
          else
          {
              asprintf(&sdCard_Date, "%s", "ERRO GET TIME");
          } */

          if (releNumber == RELE1_NUMBER) {

            int rele_Level = getRele1();

            if (BLE_SMS_Indication != SMS_INDICATION) {

              if (rele1_Mode_Label == BIESTABLE_MODE_INDEX) {

                sdCard_Logs_struct logs_struct;
                memset(&logs_struct, 0, sizeof(logs_struct));

                sprintf(logs_struct.name, "%s", user_validateData->firstName);
                sprintf(logs_struct.phone, "%s", user_validateData->phone);
                sprintf(logs_struct.relay, "%s", "R1");

                if (BLE_SMS_Indication == BLE_INDICATION) {
                  sprintf(logs_struct.type, "%s", "BLE");
                } else if (BLE_SMS_Indication == WIEGAND_INDICATION) {
                  sprintf(logs_struct.type, "%s", mqttInfo->data);
                  sprintf(logs_struct.phone, "%s",
                          user_validateData->wiegand_code);
                } else if (BLE_SMS_Indication == UDP_INDICATION) {
                  sprintf(logs_struct.phone, "%s",
                          getUserApp_id(mqttInfo->topic));
                  sprintf(logs_struct.type, "%s", "WEB");
                } else if (BLE_SMS_Indication == RF_INDICATION) {
                  sprintf(logs_struct.type, "%s", "RF");
                  sprintf(logs_struct.phone, "%s",
                          user_validateData->rf_serial);
                } else if (BLE_SMS_Indication == REX_INDICATION) {
                  sprintf(logs_struct.type, "%s", "REX 1");
                }

                // ////printf("\n\n SD CARD BI STATE - %s  /
                // %s\n\n",user_validateData->firstName,user_validateData->phone);

                if (rele_Level) {
                  sprintf(logs_struct.relay_state, "%s",
                          return_Json_SMS_Data("ON"));
                } else {
                  sprintf(logs_struct.relay_state, "%s",
                          return_Json_SMS_Data("OFF"));
                }

                if (get_RTC_System_Time()) {
                  sprintf(logs_struct.date, "%s",
                          replace_Char_in_String(nowTime.strTime, ',', ';'));
                } else {

                  sprintf(logs_struct.date, "%s", "0;0");
                  sprintf(logs_struct.error, "%s",
                          return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));
                }

                if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD) ||
                    network_Activate_Flag == 1) {
                  sdCard_Write_LOGS(&logs_struct);
                }

                char rsp_notify[100] = {};
                sprintf(rsp_notify, "%s %c %c %d", RELE1_ELEMENT, cmd, param,
                        getRele1());

                BLE_Broadcast_Notify(rsp_notify);

                return return_ERROR_Codes(&rsp, "NTRSP");
              } else {
                //
                if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD) ||
                    network_Activate_Flag == 1) {
                  // //printf("\nENTER WRITE SD CARD BI STATE\n");
                  sdCard_Logs_struct logs_struct;
                  memset(&logs_struct, 0, sizeof(logs_struct));

                  sprintf(logs_struct.name, "%s", user_validateData->firstName);
                  sprintf(logs_struct.phone, "%s", user_validateData->phone);
                  sprintf(logs_struct.relay, "%s", "R1");

                  if (BLE_SMS_Indication == BLE_INDICATION) {
                    sprintf(logs_struct.type, "%s", "BLE");
                  } else if (BLE_SMS_Indication == WIEGAND_INDICATION) {
                    sprintf(logs_struct.type, "%s", mqttInfo->data);
                    sprintf(logs_struct.phone, "%s",
                            user_validateData->wiegand_code);
                    // //printf("\n SD CARD BI STATE - %s\n",
                    // user_validateData->wiegand_code);
                  } else if (BLE_SMS_Indication == UDP_INDICATION) {
                    sprintf(logs_struct.phone, "%s",
                            getUserApp_id(mqttInfo->topic));
                    sprintf(logs_struct.type, "%s", "WEB");
                  } else if (BLE_SMS_Indication == RF_INDICATION) {
                    sprintf(logs_struct.type, "%s", "RF");
                    sprintf(logs_struct.phone, "%s",
                            user_validateData->rf_serial);
                  } else if (BLE_SMS_Indication == REX_INDICATION) {
                    sprintf(logs_struct.type, "%s", "REX 1");
                  }

                  sprintf(logs_struct.relay_state, "%s",
                          return_Json_SMS_Data("PULSE"));

                  if (get_RTC_System_Time()) {
                    sprintf(logs_struct.date, "%s",
                            replace_Char_in_String(nowTime.strTime, ',', ';'));
                  } else {

                    sprintf(logs_struct.date, "%s", "0;0");
                    sprintf(logs_struct.error, "%s",
                            return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));
                  }

                  /*  char rsp_notify[100] = {};
                   sprintf(rsp_notify, "%s %c %c %d", RELE1_ELEMENT, cmd, param,
                   getRele1());

                   BLE_Broadcast_Notify(rsp_notify); */
                  sdCard_Write_LOGS(&logs_struct);

                  // //printf("\nENTER WRITE SD CARD 1234\n");
                }
                // //printf("\nENTER WRITE SD CARD 6666\n");
                return return_ERROR_Codes(&rsp, "NTRSP");
              }
            } else if (BLE_SMS_Indication == SMS_INDICATION) {

              if (rele1_Mode_Label == BIESTABLE_MODE_INDEX) {
                if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD) ||
                    network_Activate_Flag == 1) {

                  sdCard_Logs_struct logs_struct;
                  memset(&logs_struct, 0, sizeof(logs_struct));

                  sprintf(logs_struct.type, "%s", "SMS");
                  sprintf(logs_struct.name, "%s", user_validateData->firstName);
                  sprintf(logs_struct.phone, "%s", user_validateData->phone);
                  sprintf(logs_struct.relay, "%s", "R1");

                  if (rele_Level) {
                    sprintf(logs_struct.relay_state, "%s",
                            return_Json_SMS_Data("ON"));
                  } else {
                    sprintf(logs_struct.relay_state, "%s",
                            return_Json_SMS_Data("OFF"));
                  }

                  if (get_RTC_System_Time()) {
                    sprintf(logs_struct.date, "%s",
                            replace_Char_in_String(nowTime.strTime, ',', ';'));
                  } else {

                    sprintf(logs_struct.date, "%s", "0;0");
                    sprintf(logs_struct.error, "%s",
                            return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));
                  }

                  sdCard_Write_LOGS(&logs_struct);
                }

                char rsp_notify[100] = {};
                sprintf(rsp_notify, "%s %c %c %d", RELE1_ELEMENT, cmd, param,
                        getRele1());

                BLE_Broadcast_Notify(rsp_notify);

                if (rele_Level) {
                  char auxrsp[50] = {};
                  asprintf(&rsp, return_Json_SMS_Data("RELAY_ON"), RELE1_NUMBER,
                           "<01>");
                  return rsp;
                } else {
                  char auxrsp[50] = {};
                  asprintf(&rsp, return_Json_SMS_Data("RELAY_OFF"),
                           RELE1_NUMBER, "<02>");
                  return rsp;
                }
              } else {
                if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD) ||
                    network_Activate_Flag == 1) {

                  sdCard_Logs_struct logs_struct;
                  memset(&logs_struct, 0, sizeof(logs_struct));

                  sprintf(logs_struct.type, "%s", "SMS");
                  sprintf(logs_struct.name, "%s", user_validateData->firstName);
                  sprintf(logs_struct.phone, "%s", user_validateData->phone);
                  sprintf(logs_struct.relay, "%s", "R1");

                  sprintf(logs_struct.relay_state, "%s",
                          return_Json_SMS_Data("PULSE"));

                  if (get_RTC_System_Time()) {
                    sprintf(logs_struct.date, "%s",
                            replace_Char_in_String(nowTime.strTime, ',', ';'));
                  } else {

                    sprintf(logs_struct.date, "%s", "0;0");
                    sprintf(logs_struct.error, "%s",
                            return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));
                  }

                  sdCard_Write_LOGS(&logs_struct);
                }

                /* char rsp_notify[100] = {};
                sprintf(rsp_notify, "%s %c %c %d", RELE1_ELEMENT, cmd, param,
                getRele1());

                BLE_Broadcast_Notify(rsp_notify); */

                return return_ERROR_Codes(&rsp, "NTRSP");
              }
            } else {
              return return_ERROR_Codes(
                  &rsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
            }
          } else if (releNumber == RELE2_NUMBER) {

            int rele_Level = getRele2();
            printf("\n\nif fd_configurations.alarmMode.A %d", rele_Level);
            // fd_configurations.normal_FB_Mode %d \n\n", 10,releValue);
            // fd_configurations.alarmMode.A,
            // fd_configurat12ions.normal_FB_Mode);

            if (BLE_SMS_Indication != SMS_INDICATION) {

              if (rele2_Mode_Label == BIESTABLE_MODE_INDEX) {
                if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD) ||
                    network_Activate_Flag == 1) {

                  sdCard_Logs_struct logs_struct;
                  memset(&logs_struct, 0, sizeof(logs_struct));

                  // sprintf(logs_struct.type, "%s", "BLE");
                  sprintf(logs_struct.name, "%s", user_validateData->firstName);
                  sprintf(logs_struct.phone, "%s", user_validateData->phone);
                  sprintf(logs_struct.relay, "%s", "R2");

                  if (BLE_SMS_Indication == BLE_INDICATION) {
                    sprintf(logs_struct.type, "%s", "BLE");
                  } else if (BLE_SMS_Indication == WIEGAND_INDICATION) {
                    sprintf(logs_struct.type, "%s", mqttInfo->data);
                    sprintf(logs_struct.phone, "%s",
                            user_validateData->wiegand_code);
                  } else if (BLE_SMS_Indication == UDP_INDICATION) {
                    sprintf(logs_struct.phone, "%s",
                            getUserApp_id(mqttInfo->topic));
                    sprintf(logs_struct.type, "%s", "WEB");
                  } else if (BLE_SMS_Indication == RF_INDICATION) {
                    sprintf(logs_struct.type, "%s", "RF");
                    sprintf(logs_struct.phone, "%s",
                            user_validateData->rf_serial);
                  } else if (BLE_SMS_Indication == REX_INDICATION) {
                    sprintf(logs_struct.type, "%s", "REX 2");
                  }

                  if (rele_Level) {
                    sprintf(logs_struct.relay_state, "%s",
                            return_Json_SMS_Data("ON"));
                  } else {
                    sprintf(logs_struct.relay_state, "%s",
                            return_Json_SMS_Data("OFF"));
                  }

                  if (get_RTC_System_Time()) {
                    sprintf(logs_struct.date, "%s",
                            replace_Char_in_String(nowTime.strTime, ',', ';'));
                  } else {

                    sprintf(logs_struct.date, "%s", "0;0");
                    sprintf(logs_struct.error, "%s",
                            return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));
                  }

                  sdCard_Write_LOGS(&logs_struct);
                }

                char rsp_notify[100] = {};

                sprintf(rsp_notify, "%s %c %c %d", RELE2_ELEMENT, cmd, param,
                        getRele2());
                BLE_Broadcast_Notify(rsp_notify);

                return return_ERROR_Codes(&rsp, "NTRSP");
              } else {
                if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD) ||
                    network_Activate_Flag == 1) {

                  sdCard_Logs_struct logs_struct;
                  memset(&logs_struct, 0, sizeof(logs_struct));

                  sprintf(logs_struct.name, "%s", user_validateData->firstName);
                  sprintf(logs_struct.phone, "%s", user_validateData->phone);
                  sprintf(logs_struct.relay, "%s", "R2");

                  if (BLE_SMS_Indication == BLE_INDICATION) {
                    sprintf(logs_struct.type, "%s", "BLE");
                  } else if (BLE_SMS_Indication == WIEGAND_INDICATION) {
                    sprintf(logs_struct.type, "%s", mqttInfo->data);
                    sprintf(logs_struct.phone, "%s",
                            user_validateData->wiegand_code);
                  } else if (BLE_SMS_Indication == UDP_INDICATION) {
                    sprintf(logs_struct.phone, "%s",
                            getUserApp_id(mqttInfo->topic));
                    sprintf(logs_struct.type, "%s", "WEB");
                  } else if (BLE_SMS_Indication == RF_INDICATION) {
                    sprintf(logs_struct.type, "%s", "RF");
                    sprintf(logs_struct.phone, "%s",
                            user_validateData->rf_serial);
                  } else if (BLE_SMS_Indication == REX_INDICATION) {
                    sprintf(logs_struct.type, "%s", "REX 2");
                  }

                  sprintf(logs_struct.relay_state, "%s",
                          return_Json_SMS_Data("PULSE"));

                  if (get_RTC_System_Time()) {
                    sprintf(logs_struct.date, "%s",
                            replace_Char_in_String(nowTime.strTime, ',', ';'));
                  } else {

                    sprintf(logs_struct.date, "%s", "0;0");
                    sprintf(logs_struct.error, "%s",
                            return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));
                  }

                  sdCard_Write_LOGS(&logs_struct);
                }

                return return_ERROR_Codes(&rsp, "NTRSP");
              }
              // return "NTRSP";
            } else if (BLE_SMS_Indication == SMS_INDICATION) {

              // free(sdCard_Date);
              // ////printf("data_SMS->labelRsp == %d", data_SMS->labelRsp);
              // if (data_SMS->labelRsp == 1)
              // {

              if (rele2_Mode_Label == BIESTABLE_MODE_INDEX) {
                if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD) ||
                    network_Activate_Flag == 1) {

                  sdCard_Logs_struct logs_struct;
                  memset(&logs_struct, 0, sizeof(logs_struct));

                  sprintf(logs_struct.type, "%s", "SMS");
                  sprintf(logs_struct.name, "%s", user_validateData->firstName);
                  sprintf(logs_struct.phone, "%s", user_validateData->phone);
                  sprintf(logs_struct.relay, "%s", "R2");

                  if (rele_Level) {
                    sprintf(logs_struct.relay_state, "%s",
                            return_Json_SMS_Data("ON"));
                  } else {
                    sprintf(logs_struct.relay_state, "%s",
                            return_Json_SMS_Data("OFF"));
                  }

                  if (get_RTC_System_Time()) {
                    sprintf(logs_struct.date, "%s",
                            replace_Char_in_String(nowTime.strTime, ',', ';'));
                  } else {

                    sprintf(logs_struct.date, "%s", "0;0");
                    sprintf(logs_struct.error, "%s",
                            return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));
                  }

                  char rsp_notify[100] = {};

                  sprintf(rsp_notify, "%s %c %c %d", RELE2_ELEMENT, cmd, param,
                          getRele2());
                  BLE_Broadcast_Notify(rsp_notify);
                  sdCard_Write_LOGS(&logs_struct);
                }

                if (rele_Level) {

                  asprintf(&rsp, return_Json_SMS_Data("RELAY_ON"), RELE2_NUMBER,
                           "<03>");
                  return rsp;
                } else {

                  asprintf(&rsp, return_Json_SMS_Data("RELAY_OFF"),
                           RELE2_NUMBER, "<04>");
                  return rsp;
                }
              } else {
                if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD) ||
                    network_Activate_Flag == 1) {

                  sdCard_Logs_struct logs_struct;
                  memset(&logs_struct, 0, sizeof(logs_struct));

                  sprintf(logs_struct.type, "%s", "SMS");
                  sprintf(logs_struct.name, "%s", user_validateData->firstName);
                  sprintf(logs_struct.phone, "%s", user_validateData->phone);
                  sprintf(logs_struct.relay, "%s", "R2");

                  sprintf(logs_struct.relay_state, "%s",
                          return_Json_SMS_Data("PULSE"));

                  if (get_RTC_System_Time()) {
                    sprintf(logs_struct.date, "%s",
                            replace_Char_in_String(nowTime.strTime, ',', ';'));
                  } else {

                    sprintf(logs_struct.date, "%s", "0;0");
                    sprintf(logs_struct.error, "%s",
                            return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));
                  }

                  sdCard_Write_LOGS(&logs_struct);
                }

                return return_ERROR_Codes(&rsp, "NTRSP");
              }
              // }
            } else {
              return return_ERROR_Codes(
                  &rsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
            }
          }
        } else {
          printf("\n\n error set 123\n\n");
          return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_SET"));
        }
        return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_SET"));
      } else if (param == RELE_MODE_PARAMETER) {

        if (user_validateData->permition == '1' ||
            user_validateData->permition == '2') {
          if (releNumber == RELE1_NUMBER) {
            save_INT8_Data_In_Storage(NVS_KEY_RELAY1_LAST_VALUE, 0,
                                      nvs_System_handle);
            // resetRele1();
            rele1_Mode_Label = BIESTABLE_MODE_INDEX;

            save_INT8_Data_In_Storage(NVS_RELAY1_MODE, rele1_Mode_Label,
                                      nvs_System_handle);

            if (BLE_SMS_Indication == BLE_INDICATION ||
                BLE_SMS_Indication == UDP_INDICATION) {
              char auxrsp[50] = {};

              asprintf(&rsp, "%s %c %c %d", RELE1_ELEMENT, cmd, param,
                       rele1_Mode_Label);

              return rsp;
            } else if (BLE_SMS_Indication == SMS_INDICATION) {

              char UDP_Rsp[50] = {};
              sprintf(mqttInfo->data, "%s %c %c %d", RELE1_ELEMENT, cmd, param,
                      rele1_Mode_Label);
              send_UDP_queue(mqttInfo);
              // send_UDP_Send(UDP_Rsp);
              asprintf(&rsp, return_Json_SMS_Data("RELAY_BIESTABLE_MODE_ON"),
                       RELE1_NUMBER);
              return rsp;
            } else {
              return return_ERROR_Codes(
                  &rsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
            }
          } else if (releNumber == RELE2_NUMBER) {
            save_INT8_Data_In_Storage(NVS_KEY_RELAY2_LAST_VALUE, 0,
                                      nvs_System_handle);
            // resetRele2();
            rele2_Mode_Label = BIESTABLE_MODE_INDEX;

            save_INT8_Data_In_Storage(NVS_RELAY2_MODE, rele2_Mode_Label,
                                      nvs_System_handle);

            if (BLE_SMS_Indication == BLE_INDICATION ||
                BLE_SMS_Indication == UDP_INDICATION) {

              asprintf(&rsp, "%s %c %c %d", RELE2_ELEMENT, cmd, param,
                       rele2_Mode_Label);

              return rsp;
            } else if (BLE_SMS_Indication == SMS_INDICATION) {

              char UDP_Rsp[50] = {};
              sprintf(mqttInfo->data, "%s %c %c %d", RELE2_ELEMENT, cmd, param,
                      rele2_Mode_Label);
              send_UDP_queue(mqttInfo);
              // send_UDP_Send(UDP_Rsp);
              asprintf(&rsp, return_Json_SMS_Data("RELAY_BIESTABLE_MODE_ON"),
                       RELE2_NUMBER);
              return rsp;
            } else {
              return return_ERROR_Codes(
                  &rsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
            }
          } else {

            // ////printf(ERROR_SET);
            return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_SET"));
          }
        } else {

          // ////printf(ERROR_USER_NOT_PERMITION);
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
        }
      } else if (param == TIME_PARAMETER) {
        int aux_Bistate_Time = rele1_Bistate_Time;
        if (user_validateData->permition == '1' ||
            user_validateData->permition == '2') {
          if (strlen(payload) == 0) {
            return return_ERROR_Codes(&rsp,
                                      return_Json_SMS_Data("ERROR_INPUT_DATA"));
          }

          for (size_t i = 0; i < strlen(payload); i++) {
            if ((payload[i] < '0' || payload[i] > '9')) {
              return return_ERROR_Codes(
                  &rsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
            }
          }

          if (releNumber == RELE1_NUMBER) {

            if (atoi(payload) > RELE_BISTATE_MAX_TIME) {
              if (BLE_SMS_Indication == BLE_INDICATION ||
                  BLE_SMS_Indication == UDP_INDICATION) {
                // rele1_Bistate_Time = aux_Bistate_Time;
                asprintf(&rsp, "%s %c %c %s", RELE1_ELEMENT, cmd, param,
                         "RELAY TIME ERROR");
                return rsp;
              } else if (BLE_SMS_Indication == SMS_INDICATION) {
                // rele1_Bistate_Time = aux_Bistate_Time;

                asprintf(&rsp, return_Json_SMS_Data("ERROR_RELAY_TIME"),
                         RELE1_NUMBER);
                return rsp;
              } else {
                return return_ERROR_Codes(
                    &rsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
              }
            } else {
              rele1_Bistate_Time = atoi(payload);
              nvs_set_u32(nvs_System_handle, NVS_RELAY1_BISTATE_TIME,
                          rele1_Bistate_Time);

              if (BLE_SMS_Indication == BLE_INDICATION ||
                  BLE_SMS_Indication == UDP_INDICATION) {

                asprintf(&rsp, "%s %c %c %d", RELE1_ELEMENT, cmd, param,
                         rele1_Bistate_Time);
                return rsp;
              } else if (BLE_SMS_Indication == SMS_INDICATION) {
                char UDP_Rsp[50] = {};
                sprintf(mqttInfo->data, "%s %c %c %d", RELE1_ELEMENT, cmd,
                        param, rele1_Bistate_Time);
                send_UDP_queue(mqttInfo);
                // send_UDP_Send(UDP_Rsp);

                asprintf(&rsp, return_Json_SMS_Data("RELAY_TIME_IS_SECONDS"),
                         RELE1_NUMBER, rele1_Bistate_Time);
                return rsp;
              } else {
                return return_ERROR_Codes(
                    &rsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
              }
            }
          } else if (releNumber == RELE2_NUMBER) {

            if (atoi(payload) > RELE_BISTATE_MAX_TIME) {
              if (BLE_SMS_Indication == BLE_INDICATION ||
                  BLE_SMS_Indication == UDP_INDICATION) {
                // rele2_Bistate_Time = aux_Bistate_Time;
                asprintf(&rsp, "%s %c %c %s", RELE2_ELEMENT, cmd, param,
                         "RELAY TIME ERROR");
                return rsp;
              } else if (BLE_SMS_Indication == SMS_INDICATION) {
                // rele2_Bistate_Time = aux_Bistate_Time;
                asprintf(&rsp, return_Json_SMS_Data("ERROR_RELAY_TIME"),
                         RELE2_NUMBER);
                return rsp;
              } else {
                return return_ERROR_Codes(
                    &rsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
              }
            } else {
              rele2_Bistate_Time = atoi(payload);
              nvs_set_u32(nvs_System_handle, NVS_RELAY2_BISTATE_TIME,
                          rele2_Bistate_Time);

              if (BLE_SMS_Indication == BLE_INDICATION ||
                  BLE_SMS_Indication == UDP_INDICATION) {
                asprintf(&rsp, "%s %c %c %d", RELE2_ELEMENT, cmd, param,
                         rele2_Bistate_Time);
                return rsp;
              } else if (BLE_SMS_Indication == SMS_INDICATION) {
                char UDP_Rsp[50] = {};
                sprintf(mqttInfo->data, "%s %c %c %d", RELE2_ELEMENT, cmd,
                        param, rele2_Bistate_Time);
                send_UDP_queue(mqttInfo);
                // send_UDP_Send(UDP_Rsp);
                asprintf(&rsp, return_Json_SMS_Data("RELAY_TIME_IS_SECONDS"),
                         RELE2_NUMBER, rele2_Bistate_Time);
                return rsp;
              } else {
                return return_ERROR_Codes(
                    &rsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
              }
            }
          } else {
            return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_SET"));
          }
        } else {

          // ////printf(ERROR_USER_NOT_PERMITION);
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
        }
      } else if (param == PAIRING_PARAMETER) {
        if (user_validateData->permition == '1' ||
            user_validateData->permition == '2') {
          if (releNumber == RELE1_NUMBER) {
            if (BLE_SMS_Indication == BLE_INDICATION ||
                BLE_SMS_Indication == UDP_INDICATION) {
              rele1_Restriction = 1;
              save_INT8_Data_In_Storage(NVS_KEY_RELAY_PAIRING_VALUE,
                                        rele1_Restriction, nvs_System_handle);
              asprintf(&rsp, "%s %c %c %d", RELE1_ELEMENT, cmd, param,
                       rele1_Restriction);
              return rsp;
            } else if (BLE_SMS_Indication == SMS_INDICATION) {
              rele1_Restriction = 1;
              save_INT8_Data_In_Storage(NVS_KEY_RELAY_PAIRING_VALUE,
                                        rele1_Restriction, nvs_System_handle);

              char UDP_Rsp[50] = {};
              sprintf(mqttInfo->data, "%s %c %c %d", RELE1_ELEMENT, cmd, param,
                      rele1_Restriction);
              // send_UDP_Send(UDP_Rsp);
              send_UDP_queue(mqttInfo);
              asprintf(&rsp, return_Json_SMS_Data(
                                 "RESTRICTION_OF_RELAY_1_HAS_BEEN_ENABLED"));
              return rsp;
            } else {
              return return_ERROR_Codes(
                  &rsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
            }
          } else {
            // ////printf(ERROR_SET);
            return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_SET"));
          }
        } else {
          // ////printf(ERROR_USER_NOT_PERMITION);
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
        }
      } else {

        // ////printf(ERROR_PARAMETER);
        return return_ERROR_Codes(&rsp,
                                  return_Json_SMS_Data("ERROR_PARAMETER"));
      }
    } else {
      if (param == RELE_PARAMETER && cmd == SET_CMD) {
        if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD) ||
            network_Activate_Flag == 1) {
          sdCard_Logs_struct logs_struct;
          memset(&logs_struct, 0, sizeof(logs_struct));

          if (BLE_SMS_Indication == BLE_INDICATION) {
            sprintf(logs_struct.type, "%s", "BLE");
            sprintf(logs_struct.phone, "%s", user_validateData->phone);
          } else if (BLE_SMS_Indication == WIEGAND_INDICATION) {
            sprintf(logs_struct.type, "%s", mqttInfo->data);
            sprintf(logs_struct.phone, "%s", user_validateData->wiegand_code);
          } else if (BLE_SMS_Indication == SMS_INDICATION) {
            sprintf(logs_struct.type, "%s", "SMS");
            sprintf(logs_struct.phone, "%s", user_validateData->phone);
          } else if (BLE_SMS_Indication == WIEGAND_INDICATION) {
            sprintf(logs_struct.type, "%s", mqttInfo->data);
            sprintf(logs_struct.phone, "%s", user_validateData->wiegand_code);
          } else if (BLE_SMS_Indication == RF_INDICATION) {
            sprintf(logs_struct.type, "%s", "RF");
            sprintf(logs_struct.phone, "%s", user_validateData->rf_serial);
          }

          printf("\nlogs_struct.name1 %s\n", logs_struct.name);
          printf("\nuser_validateData->rf_serial %s\n", user_validateData->rf_serial);
          printf("\nuser_validateData->rf_serial %s\n", logs_struct.phone);
          // ////printf("\nlogs_struct.name2 %s\n",
          // user_validateData->firstName);
          sprintf(logs_struct.name, "%s", user_validateData->firstName);
          // ////printf("\nlogs_struct.name3 %s\n", logs_struct.name);
          // ////printf("\nlogs_struct.name4 %s\n",
          // user_validateData->firstName);
          
          sprintf(logs_struct.relay, "%c%c", 'R', releNumber + 48);
          sprintf(logs_struct.relay_state, "%s",
                  return_Json_SMS_Data("NOT_CHANGE"));
          sprintf(logs_struct.date, "%s",
                  replace_Char_in_String(nowTime.strTime, ',', ';'));

          sprintf(logs_struct.error, "%s",
                  return_Json_SMS_Data("ERROR_LOGS_USER_NOT_PERMITION"));
          sdCard_Write_LOGS(&logs_struct);
        }
      }

      // ////printf(ERROR_USER_NOT_PERMITION);
      return return_ERROR_Codes(
          &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
    }
    /*  }
   else
    {
        //////printf("\n\n NOT ENTER IN MONOSTABLE CHECK\n\n");
        if (BLE_SMS_Indication == BLE_INDICATION)
        {
            //////printf("\n\n NOT ENTER IN MONOSTABLE CHECK 11\n\n");
            //////printf(ERROR_SET);
            return return_ERROR_Codes(&rsp, return_Json_SMS_Data(ERROR_SET));
        }
        else if (BLE_SMS_Indication == SMS_INDICATION)
        {
            // data_EG91_Send_SMS cpy_message12 = *data_SMS;
            asprintf(&rsp, return_Json_SMS_Data("RELAY_IS_ON_UNABLE_TURN_ON"),
   releNumber);
            // sprintf(cpy_message12.payload, "RELAY %d IS ON. UNABLE TO TURN
   ON", releNumber);
            //////printf("\nlabel led %d,%d\n", cpy_message12.labelRsp,
   data_SMS->labelRsp);
            // xQueueSendToFront(queue_EG91_SendSMS, (void *)&cpy_message12,
   (TickType_t)0); return rsp;
        }
    } */
  } else if (cmd == GET_CMD) {
    if (validate_DataUser(user_validateData, phPassword)) {
      if (param == RELE_PARAMETER) {
        if (releNumber == RELE1_NUMBER) {
          // if (BLE_SMS_Indication == BLE_INDICATION && rele1_Restriction == 1)
          // {
          //     //////printf("NAO PERMITIDO GET. SO POR CHAMADA\n");
          //     return return_ERROR_Codes(&rsp,
          //     return_Json_SMS_Data("ERROR_GET"));
          // }
          // else
          // {
          if (BLE_SMS_Indication == BLE_INDICATION ||
              BLE_SMS_Indication == UDP_INDICATION) {
            asprintf(&rsp, "%s %c %c %d", RELE1_ELEMENT, cmd, param,
                     getRele1());
            return rsp;
          } else if (BLE_SMS_Indication == SMS_INDICATION) {
            if (getRele1()) {
              asprintf(&rsp, return_Json_SMS_Data("RELAY_GET_ON"),
                       RELE1_NUMBER);
              return rsp;
            } else {
              asprintf(&rsp, return_Json_SMS_Data("RELAY_GET_OFF"),
                       RELE1_NUMBER);
              return rsp;
            }
          } else {
            return return_ERROR_Codes(&rsp,
                                      return_Json_SMS_Data("ERROR_INPUT_DATA"));
          }
          //}
        } else if (releNumber == RELE2_NUMBER) {
          if (BLE_SMS_Indication == BLE_INDICATION ||
              BLE_SMS_Indication == UDP_INDICATION) {
            asprintf(&rsp, "%s %c %c %d", RELE2_ELEMENT, cmd, param,
                     getRele2());
            return rsp;
          } else if (BLE_SMS_Indication == SMS_INDICATION) {
            if (getRele2()) {
              asprintf(&rsp, return_Json_SMS_Data("RELAY_GET_ON"),
                       RELE2_NUMBER);
              return rsp;
            } else {
              asprintf(&rsp, return_Json_SMS_Data("RELAY_GET_OFF"),
                       RELE2_NUMBER);
              return rsp;
            }
          } else {
            return return_ERROR_Codes(&rsp,
                                      return_Json_SMS_Data("ERROR_INPUT_DATA"));
          }
        } else {
          // ////printf(ERROR_GET);
          return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_GET"));
        }
      } else if (param == RELE_MODE_PARAMETER) {
        if (user_validateData->permition == '1' ||
            user_validateData->permition == '2') {
          if (BLE_SMS_Indication == BLE_INDICATION ||
              BLE_SMS_Indication == UDP_INDICATION) {
            if (releNumber == RELE1_NUMBER) {
              asprintf(&rsp, "%s %c %c %d", RELE1_ELEMENT, cmd, param,
                       rele1_Mode_Label);
              return rsp;
            } else if (releNumber == RELE2_NUMBER) {
              asprintf(&rsp, "%s %c %c %d", RELE2_ELEMENT, cmd, param,
                       rele2_Mode_Label);
              return rsp;
            } else {
              // ////printf(ERROR_GET);
              return return_ERROR_Codes(&rsp,
                                        return_Json_SMS_Data("ERROR_GET"));
            }
          } else if (BLE_SMS_Indication == SMS_INDICATION) {
            if (releNumber == RELE1_NUMBER) {
              if (rele1_Mode_Label == MONOESTABLE_MODE_INDEX) { /*  */
                asprintf(&rsp,
                         return_Json_SMS_Data("RELAY_IN_MONOESTABLE_MODE"),
                         RELE1_NUMBER);
                return rsp;
              } else {
                asprintf(&rsp, return_Json_SMS_Data("RELAY_IN_BIESTABLE_MODE"),
                         RELE1_NUMBER);
                return rsp;
              }
            } else if (releNumber == RELE2_NUMBER) {
              if (rele2_Mode_Label == MONOESTABLE_MODE_INDEX) {
                asprintf(&rsp,
                         return_Json_SMS_Data("RELAY_IN_MONOESTABLE_MODE"),
                         RELE2_NUMBER);

                return rsp;
              } else {
                asprintf(&rsp, return_Json_SMS_Data("RELAY_IN_BIESTABLE_MODE"),
                         RELE2_NUMBER);
                return rsp;
              }
            } else {
              // ////printf(ERROR_GET);
              return return_ERROR_Codes(&rsp,
                                        return_Json_SMS_Data("ERROR_GET"));
            }
          } else {
            return return_ERROR_Codes(&rsp,
                                      return_Json_SMS_Data("ERROR_INPUT_DATA"));
          }
        } else {
          // ////printf(ERROR_USER_NOT_PERMITION);
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
        }
      } else if (param == TIME_PARAMETER) {
        if (user_validateData->permition == '1' ||
            user_validateData->permition == '2') {
          if (BLE_SMS_Indication == BLE_INDICATION ||
              BLE_SMS_Indication == UDP_INDICATION) {
            if (releNumber == RELE1_NUMBER) {
              asprintf(&rsp, "%s %c %c %d", RELE1_ELEMENT, cmd, param,
                       rele1_Bistate_Time);
              return rsp;
            } else if (releNumber == RELE2_NUMBER) {
              asprintf(&rsp, "%s %c %c %d", RELE2_ELEMENT, cmd, param,
                       rele2_Bistate_Time);
              return rsp;
            } else {
              // ////printf(ERROR_GET);
              return return_ERROR_Codes(&rsp,
                                        return_Json_SMS_Data("ERROR_GET"));
            }
          } else if (BLE_SMS_Indication == SMS_INDICATION) {
            if (releNumber == RELE1_NUMBER) {
              asprintf(&rsp, return_Json_SMS_Data("RELAY_GET_TIME_IS_SECONDS"),
                       RELE1_NUMBER, rele1_Bistate_Time);
              return rsp;
            } else if (releNumber == RELE2_NUMBER) {
              asprintf(&rsp, return_Json_SMS_Data("RELAY_GET_TIME_IS_SECONDS"),
                       RELE2_NUMBER, rele2_Bistate_Time);
              return rsp;
            } else {
              // ////printf(ERROR_GET);
              return return_ERROR_Codes(&rsp,
                                        return_Json_SMS_Data("ERROR_GET"));
            }
          } else {
            return return_ERROR_Codes(&rsp,
                                      return_Json_SMS_Data("ERROR_INPUT_DATA"));
          }
        } else {
          // ////printf(ERROR_USER_NOT_PERMITION);
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
        }
      } else if (param == PAIRING_PARAMETER) {
        if (user_validateData->permition == '1' ||
            user_validateData->permition == '2') {
          if (releNumber == RELE1_NUMBER) {
            if (BLE_SMS_Indication == BLE_INDICATION ||
                BLE_SMS_Indication == UDP_INDICATION) {
              asprintf(&rsp, "%s %c %c %d", RELE1_ELEMENT, cmd, param,
                       rele1_Restriction);
              return rsp;
            } else if (BLE_SMS_Indication == SMS_INDICATION) {
              if (rele1_Restriction) {
                asprintf(&rsp,
                         return_Json_SMS_Data("RELAY_1_GET_RESTRICTION_ON"));
                return rsp;
              } else {
                asprintf(&rsp,
                         return_Json_SMS_Data("RELAY_1_GET_RESTRICTION_OFF"));
                return rsp;
              }
            } else {
              return return_ERROR_Codes(
                  &rsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
            }
          } else {
            // ////printf(ERROR_GET);
            return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_GET"));
          }
        } else {
          // ////printf(ERROR_USER_NOT_PERMITION);
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
        }
      } else {
        // ////printf(ERROR_GET);
        return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_GET"));
      }
    } else {
      // ////printf(ERROR_USER_NOT_PERMITION);
      return return_ERROR_Codes(
          &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
    }
  } else if (cmd == RESET_CMD) {
    if (validate_DataUser(user_validateData, phPassword)) {
      if (param == RELE_PARAMETER) {

        if (label_Routine1_ON == 1 && releNumber == RELE1_NUMBER) {
          if (user_validateData->permition == '2') {
            label_Routine1_ON = 0;
            resetRele1();

            sdCard_Logs_struct logs_struct;
            memset(&logs_struct, 0, sizeof(logs_struct));

            // ////printf("\n\n logs_struct.type SMS 11\n\n");

            if (BLE_SMS_Indication == BLE_INDICATION) {
              sprintf(logs_struct.type, "%s", "BLE");
            } else if (BLE_SMS_Indication == SMS_INDICATION) {
              // ////printf("\n\n logs_struct.type SMS 22\n\n");
              sprintf(logs_struct.type, "%s", "SMS");
            } else if (BLE_SMS_Indication == UDP_INDICATION) {
              sprintf(logs_struct.type, "%s", "WEB");
            }

            sprintf(logs_struct.name, "%s",
                    return_Json_SMS_Data("LOGS_DISABLE_ROUTINE"));
            sprintf(logs_struct.phone, "%s", user_validateData->phone);
            sprintf(logs_struct.relay, "%s", "R1");

            sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("OFF"));

            if (get_RTC_System_Time()) {
              sprintf(logs_struct.date, "%s",
                      replace_Char_in_String(nowTime.strTime, ',', ';'));
            } else {
              sprintf(logs_struct.date, "%s", "0,0");
              sprintf(logs_struct.error, "%s",
                      return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));
            }

            sdCard_Write_LOGS(&logs_struct);

            asprintf(&rsp, "%s", return_Json_SMS_Data("DISABLE_ROUTINE_1"));
            return rsp;
          } else {
            sdCard_Logs_struct logs_struct;
            memset(&logs_struct, 0, sizeof(logs_struct));

            if (BLE_SMS_Indication == BLE_INDICATION) {
              sprintf(logs_struct.type, "%s", "BLE");
            } else if (BLE_SMS_Indication == SMS_INDICATION) {
              sprintf(logs_struct.type, "%s", "SMS");
            } else if (BLE_SMS_Indication == UDP_INDICATION) {
              sprintf(logs_struct.type, "%s", "WEB");
            }

            sprintf(logs_struct.name, "%s", user_validateData->firstName);
            sprintf(logs_struct.phone, "%s", user_validateData->phone);
            sprintf(logs_struct.relay, "R%d", RELE1_NUMBER);
            sprintf(logs_struct.relay_state, "%s",
                    return_Json_SMS_Data("NOT_CHANGE"));

            if (get_RTC_System_Time()) {
              sprintf(logs_struct.date, "%s",
                      replace_Char_in_String(nowTime.strTime, ',', ';'));
              sprintf(logs_struct.error, "%s",
                      return_Json_SMS_Data(
                          "ERROR_LOGS_IS_RUNNING_ROUTINE_ON_RELAY"));
            } else {

              sprintf(logs_struct.date, "%s", "0;0");
              sprintf(logs_struct.error, "%s",
                      return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));
              strcat(logs_struct.error, "/\0");
              strcat(logs_struct.error,
                     return_Json_SMS_Data(
                         "ERROR_LOGS_IS_RUNNING_ROUTINE_ON_RELAY"));
            }

            sdCard_Write_LOGS(&logs_struct);

            return return_ERROR_Codes(
                &rsp,
                return_Json_SMS_Data("ERROR_IS_RUNNING_ROUTINE_ON_RELAY"));
          }
        }

        if (label_Routine2_ON == 1 && releNumber == RELE2_NUMBER) {
          if (user_validateData->permition == '2') {
            label_Routine2_ON = 0;
            resetRele2();
            sdCard_Logs_struct logs_struct;
            memset(&logs_struct, 0, sizeof(logs_struct));

            if (BLE_SMS_Indication == BLE_INDICATION) {
              sprintf(logs_struct.type, "%s", "BLE");
            } else if (BLE_SMS_Indication == SMS_INDICATION) {
              // ////printf("\n\n logs_struct.type SMS aaaa\n\n");
              sprintf(logs_struct.type, "%s", "SMS");
            } else if (BLE_SMS_Indication == UDP_INDICATION) {
              sprintf(logs_struct.type, "%s", "WEB");
            }

            sprintf(logs_struct.name, "%s",
                    return_Json_SMS_Data("LOGS_DISABLE_ROUTINE"));
            sprintf(logs_struct.phone, "%s", user_validateData->phone);
            sprintf(logs_struct.relay, "%s", "R2");

            sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("OFF"));

            if (get_RTC_System_Time()) {
              sprintf(logs_struct.date, "%s",
                      replace_Char_in_String(nowTime.strTime, ',', ';'));
            } else {
              sprintf(logs_struct.date, "%s", "0,0");
              sprintf(logs_struct.error, "%s",
                      return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));
            }

            sdCard_Write_LOGS(&logs_struct);

            asprintf(&rsp, "%s", return_Json_SMS_Data("DISABLE_ROUTINE_2"));
            return rsp;
          } else {
            sdCard_Logs_struct logs_struct;
            memset(&logs_struct, 0, sizeof(logs_struct));

            if (BLE_SMS_Indication == BLE_INDICATION) {
              sprintf(logs_struct.type, "%s", "BLE");
            } else if (BLE_SMS_Indication == SMS_INDICATION) {
              sprintf(logs_struct.type, "%s", "SMS");
            } else if (BLE_SMS_Indication == UDP_INDICATION) {
              sprintf(logs_struct.type, "%s", "WEB");
            }

            sprintf(logs_struct.name, "%s", user_validateData->firstName);
            sprintf(logs_struct.phone, "%s", user_validateData->phone);
            sprintf(logs_struct.relay, "R%d", RELE2_NUMBER);
            sprintf(logs_struct.relay_state, "%s",
                    return_Json_SMS_Data("NOT_CHANGE"));

            if (get_RTC_System_Time()) {
              sprintf(logs_struct.date, "%s",
                      replace_Char_in_String(nowTime.strTime, ',', ';'));
              sprintf(logs_struct.error, "%s",
                      return_Json_SMS_Data(
                          "ERROR_LOGS_IS_RUNNING_ROUTINE_ON_RELAY"));
            } else {

              sprintf(logs_struct.date, "%s", "0;0");
              sprintf(logs_struct.error, "%s",
                      return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));
              strcat(logs_struct.error, "/\0");
              strcat(logs_struct.error,
                     return_Json_SMS_Data(
                         "ERROR_LOGS_IS_RUNNING_ROUTINE_ON_RELAY"));
            }

            sdCard_Write_LOGS(&logs_struct);

            return return_ERROR_Codes(
                &rsp,
                return_Json_SMS_Data("ERROR_IS_RUNNING_ROUTINE_ON_RELAY"));
          }
        }

        if (user_validateData->permition == '0' &&
            user_validateData->relayPermition - 48 == releNumber) {
          if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD) ||
              network_Activate_Flag == 1) {

            sdCard_Logs_struct logs_struct;
            memset(&logs_struct, 0, sizeof(logs_struct));

            if (BLE_SMS_Indication == BLE_INDICATION) {
              sprintf(logs_struct.type, "%s", "BLE");
            } else if (BLE_SMS_Indication == SMS_INDICATION) {
              sprintf(logs_struct.type, "%s", "SMS");
            } else if (BLE_SMS_Indication == UDP_INDICATION) {
              sprintf(logs_struct.type, "%s", "WEB");
            }

            sprintf(logs_struct.name, "%s", user_validateData->firstName);
            sprintf(logs_struct.phone, "%s", user_validateData->phone);
            sprintf(logs_struct.relay, "R%d", releNumber);
            sprintf(logs_struct.relay_state, "%s",
                    return_Json_SMS_Data("NOT_CHANGE"));

            if (get_RTC_System_Time()) {
              sprintf(logs_struct.date, "%s",
                      replace_Char_in_String(nowTime.strTime, ',', ';'));
              sprintf(logs_struct.error, "%s",
                      return_Json_SMS_Data(
                          "ERROR_LOGS_NOT_HAVE_PERMITION_THIS_RELAY"));
            } else {

              sprintf(logs_struct.date, "%s", "0;0");
              sprintf(logs_struct.error, "%s",
                      return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));
              strcat(logs_struct.error, "/\0");
              strcat(logs_struct.error,
                     return_Json_SMS_Data(
                         "ERROR_LOGS_NOT_HAVE_PERMITION_THIS_RELAY"));
            }

            sdCard_Write_LOGS(&logs_struct);
          }

          if (BLE_SMS_Indication == BLE_INDICATION ||
              BLE_SMS_Indication == UDP_INDICATION) {

            return return_ERROR_Codes(&rsp,
                                      "ERROR USER RELE NOT PERMITION\n<57>");
          } else if (BLE_SMS_Indication == SMS_INDICATION) {
            return return_ERROR_Codes(
                &rsp,
                return_Json_SMS_Data("ERROR_NOT_HAVE_PERMITION_THIS_RELAY"));
          }
        }

        if (releNumber == RELE1_NUMBER) {

          if (BLE_SMS_Indication == BLE_INDICATION && rele1_Restriction == 1) {
            if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD) ||
                network_Activate_Flag == 1) {

              sdCard_Logs_struct logs_struct;
              memset(&logs_struct, 0, sizeof(logs_struct));

              if (BLE_SMS_Indication == BLE_INDICATION) {
                sprintf(logs_struct.type, "%s", "BLE");
              } else if (BLE_SMS_Indication == SMS_INDICATION) {
                sprintf(logs_struct.type, "%s", "SMS");
              } else if (BLE_SMS_Indication == UDP_INDICATION) {
                sprintf(logs_struct.type, "%s", "WEB");
              }

              sprintf(logs_struct.name, "%s", user_validateData->firstName);
              sprintf(logs_struct.phone, "%s", user_validateData->phone);
              sprintf(logs_struct.relay, "%s", "R1");
              sprintf(logs_struct.relay_state, "%s",
                      return_Json_SMS_Data("NOT_CHANGE"));
              sprintf(logs_struct.date, "%s",
                      replace_Char_in_String(nowTime.strTime, ',', ';'));

              sprintf(logs_struct.error, "%s",
                      return_Json_SMS_Data("LOGS_ONLY_PERMISSION_TO_CALL"));
              sdCard_Write_LOGS(&logs_struct);
            }
            // free(sdCard_Date);
            // ////printf("NAO PERMITIDO RESET. SO POR CHAMADA\n");
            return return_ERROR_Codes(
                &rsp, return_Json_SMS_Data("LOGS_ONLY_PERMISSION_TO_CALL"));
          } else {
            if (BLE_SMS_Indication == BLE_INDICATION ||
                BLE_SMS_Indication == UDP_INDICATION) {
              if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD) ||
                  network_Activate_Flag == 1) {

                sdCard_Logs_struct logs_struct;
                memset(&logs_struct, 0, sizeof(logs_struct));

                sprintf(logs_struct.type, "%s", "BLE");
                sprintf(logs_struct.name, "%s", user_validateData->firstName);
                sprintf(logs_struct.phone, "%s", user_validateData->phone);
                sprintf(logs_struct.relay, "%s", "R1");
                sprintf(logs_struct.relay_state, "%s",
                        return_Json_SMS_Data("OFF"));
                sprintf(logs_struct.date, "%s",
                        replace_Char_in_String(nowTime.strTime, ',', ';'));
                sdCard_Write_LOGS(&logs_struct);
              }
              // free(sdCard_Date);
              resetRele1();

              // asprintf(&rsp, "%s %c %c %d", RELE1_ELEMENT, cmd, param,
              // getRele1());
              BLE_Broadcast_Notify("R1 R R 0");
              return return_ERROR_Codes(&rsp, "NTRSP");
            } else if (BLE_SMS_Indication == SMS_INDICATION) {
              if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD) ||
                  network_Activate_Flag == 1) {

                sdCard_Logs_struct logs_struct;
                memset(&logs_struct, 0, sizeof(logs_struct));

                sprintf(logs_struct.type, "%s", "SMS");
                sprintf(logs_struct.name, "%s", user_validateData->firstName);
                sprintf(logs_struct.phone, "%s", user_validateData->phone);
                sprintf(logs_struct.relay, "%s", "R1");
                sprintf(logs_struct.relay_state, "%s",
                        return_Json_SMS_Data("OFF"));
                sprintf(logs_struct.date, "%s",
                        replace_Char_in_String(nowTime.strTime, ',', ';'));
                sdCard_Write_LOGS(&logs_struct);
              }

              if (!getRele1()) {
                asprintf(&rsp, return_Json_SMS_Data("RELAY_WAS_ALREADY_OFF"),
                         RELE1_NUMBER);
                return rsp;
              } else {
                resetRele1();

                BLE_Broadcast_Notify("R1 R R 0");
                asprintf(&rsp, return_Json_SMS_Data("RELAY_OFF"), RELE1_NUMBER,
                         "<02>");
                return rsp;
              }

              return rsp;
            }
          }
        } else if (releNumber == RELE2_NUMBER) {
          if (BLE_SMS_Indication == BLE_INDICATION ||
              BLE_SMS_Indication == UDP_INDICATION) {
            if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD) ||
                network_Activate_Flag == 1) {

              sdCard_Logs_struct logs_struct;
              memset(&logs_struct, 0, sizeof(logs_struct));

              sprintf(logs_struct.type, "%s", "BLE");
              sprintf(logs_struct.name, "%s", user_validateData->firstName);
              sprintf(logs_struct.phone, "%s", user_validateData->phone);
              sprintf(logs_struct.relay, "%s", "R2");
              sprintf(logs_struct.relay_state, "%s",
                      return_Json_SMS_Data("OFF"));
              sprintf(logs_struct.date, "%s",
                      replace_Char_in_String(nowTime.strTime, ',', ';'));
              sdCard_Write_LOGS(&logs_struct);
            }

            resetRele2();

            // asprintf(&rsp, "%s %c %c %d", RELE2_ELEMENT, cmd, param,
            // getRele2());
            BLE_Broadcast_Notify("R2 R R 0");

            return return_ERROR_Codes(&rsp, "NTRSP");
          } else {
            if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD) ||
                network_Activate_Flag == 1) {

              sdCard_Logs_struct logs_struct;
              memset(&logs_struct, 0, sizeof(logs_struct));

              sprintf(logs_struct.type, "%s", "SMS");
              sprintf(logs_struct.name, "%s", user_validateData->firstName);
              sprintf(logs_struct.phone, "%s", user_validateData->phone);
              sprintf(logs_struct.relay, "%s", "R2");
              sprintf(logs_struct.relay_state, "%s",
                      return_Json_SMS_Data("OFF"));
              sprintf(logs_struct.date, "%s",
                      replace_Char_in_String(nowTime.strTime, ',', ';'));
              sdCard_Write_LOGS(&logs_struct);
            }
            // free(sdCard_Date);
          }

          if (!getRele2()) {
            asprintf(&rsp, return_Json_SMS_Data("RELAY_WAS_ALREADY_OFF"),
                     RELE2_NUMBER);
            return rsp;
          } else {
            resetRele2();
            BLE_Broadcast_Notify("R2 R R 0");

            asprintf(&rsp, return_Json_SMS_Data("RELAY_OFF"), RELE2_NUMBER,
                     "<04>");
            return rsp;
          }

          return rsp;
        } else {
          if (BLE_SMS_Indication == BLE_INDICATION ||
              BLE_SMS_Indication == UDP_INDICATION) {
            if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD) ||
                network_Activate_Flag == 1) {

              sdCard_Logs_struct logs_struct;
              memset(&logs_struct, 0, sizeof(logs_struct));

              sprintf(logs_struct.type, "%s", "BLE");
              sprintf(logs_struct.name, "%s", user_validateData->firstName);
              sprintf(logs_struct.phone, "%s", user_validateData->phone);
              sprintf(logs_struct.relay, "%s", "R?");
              sprintf(logs_struct.relay_state, "%s",
                      return_Json_SMS_Data("OFF"));
              sprintf(logs_struct.date, "%s",
                      replace_Char_in_String(nowTime.strTime, ',', ';'));
              sdCard_Write_LOGS(&logs_struct);

              sprintf(logs_struct.error, "%s",
                      return_Json_SMS_Data("ERROR_LOGS_RESET"));
            }
            // free(sdCard_Date);
          } else {
            if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD) ||
                network_Activate_Flag == 1) {

              sdCard_Logs_struct logs_struct;
              memset(&logs_struct, 0, sizeof(logs_struct));

              sprintf(logs_struct.type, "%s", "SMS");
              sprintf(logs_struct.name, "%s", user_validateData->firstName);
              sprintf(logs_struct.phone, "%s", user_validateData->phone);
              sprintf(logs_struct.relay, "%s", "R?");
              sprintf(logs_struct.relay_state, "%s",
                      return_Json_SMS_Data("OFF"));
              sprintf(logs_struct.date, "%s",
                      replace_Char_in_String(nowTime.strTime, ',', ';'));
              sdCard_Write_LOGS(&logs_struct);
              sprintf(logs_struct.error, "%s",
                      return_Json_SMS_Data("ERROR_LOGS_RESET"));
            }
            // free(sdCard_Date);
          }

          // ////printf(ERROR_RESET);
          return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_RESET"));
        }
      } else if (param == RELE_MODE_PARAMETER) {
        if (user_validateData->permition == '1' ||
            user_validateData->permition == '2') {

          if (BLE_SMS_Indication == BLE_INDICATION ||
              BLE_SMS_Indication == UDP_INDICATION) {
            if (releNumber == RELE1_NUMBER) {
              save_INT8_Data_In_Storage(NVS_KEY_RELAY1_LAST_VALUE, 0,
                                        nvs_System_handle);
              // resetRele1();
              rele1_Mode_Label = MONOESTABLE_MODE_INDEX;

              save_INT8_Data_In_Storage(NVS_RELAY1_MODE, rele1_Mode_Label,
                                        nvs_System_handle);

              // BLE_Broadcast_Notify("R1 R R 0");
              asprintf(&rsp, "%s %c %c %d", RELE1_ELEMENT, cmd, param,
                       rele1_Mode_Label);
              return rsp;
            } else if (releNumber == RELE2_NUMBER) {
              save_INT8_Data_In_Storage(NVS_KEY_RELAY2_LAST_VALUE, 0,
                                        nvs_System_handle);
              // resetRele2();
              rele2_Mode_Label = MONOESTABLE_MODE_INDEX;

              save_INT8_Data_In_Storage(NVS_RELAY2_MODE, rele2_Mode_Label,
                                        nvs_System_handle);

              // BLE_Broadcast_Notify("R2 R R 0");
              asprintf(&rsp, "%s %c %c %d", RELE2_ELEMENT, cmd, param,
                       rele2_Mode_Label);
              return rsp;
            } else {
              // ////printf(ERROR_RESET);
              return return_ERROR_Codes(&rsp,
                                        return_Json_SMS_Data("ERROR_RESET"));
            }
          } else if (BLE_SMS_Indication == SMS_INDICATION) {
            if (releNumber == RELE1_NUMBER) {
              save_INT8_Data_In_Storage(NVS_KEY_RELAY1_LAST_VALUE, 0,
                                        nvs_System_handle);
              // resetRele1();
              rele1_Mode_Label = MONOESTABLE_MODE_INDEX;

              save_INT8_Data_In_Storage(NVS_RELAY1_MODE, rele1_Mode_Label,
                                        nvs_System_handle);

              char UDP_Rsp[50] = {};
              sprintf(mqttInfo->data, "%s %c %c %d", RELE1_ELEMENT, cmd, param,
                      rele1_Mode_Label);
              // send_UDP_Send(UDP_Rsp);
              send_UDP_queue(mqttInfo);

              asprintf(&rsp, return_Json_SMS_Data("RELAY_MONOESTABLE_MODE_ON"),
                       RELE1_NUMBER);
              return rsp;
            } else if (releNumber == RELE2_NUMBER) {
              save_INT8_Data_In_Storage(NVS_KEY_RELAY2_LAST_VALUE, 0,
                                        nvs_System_handle);
              // resetRele2();
              rele2_Mode_Label = MONOESTABLE_MODE_INDEX;

              save_INT8_Data_In_Storage(NVS_RELAY2_MODE, rele2_Mode_Label,
                                        nvs_System_handle);

              char UDP_Rsp[50] = {};
              sprintf(&mqttInfo->data, "%s %c %c %d", RELE2_ELEMENT, cmd, param,
                      rele2_Mode_Label);
              // send_UDP_Send(UDP_Rsp);
              send_UDP_queue(mqttInfo);

              asprintf(&rsp, return_Json_SMS_Data("RELAY_MONOESTABLE_MODE_ON"),
                       RELE2_NUMBER);
              return rsp;
            } else {
              // ////printf(ERROR_RESET);
              return return_ERROR_Codes(&rsp,
                                        return_Json_SMS_Data("ERROR_RESET"));
            }
          } else {
            return return_ERROR_Codes(&rsp,
                                      return_Json_SMS_Data("ERROR_INPUT_DATA"));
          }
        } else {
          // ////printf(ERROR_USER_NOT_PERMITION);
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
        }
      } else if (param == TIME_PARAMETER) {
        if (user_validateData->permition == '1' ||
            user_validateData->permition == '2') {
          if (releNumber == RELE1_NUMBER) {

            rele1_Bistate_Time = 1;
            nvs_set_u32(nvs_System_handle, NVS_RELAY1_BISTATE_TIME,
                        rele1_Bistate_Time);

            if (BLE_SMS_Indication == BLE_INDICATION ||
                BLE_SMS_Indication == UDP_INDICATION) {
              asprintf(&rsp, "%s %c %c %d", RELE1_ELEMENT, cmd, param,
                       rele1_Bistate_Time);
              return rsp;
            } else if (BLE_SMS_Indication == SMS_INDICATION) {
              char UDP_Rsp[50] = {};
              sprintf(&mqttInfo->data, "%s %c %c %d", RELE1_ELEMENT, cmd, param,
                      rele1_Bistate_Time);
              // send_UDP_Send(UDP_Rsp);
              send_UDP_queue(mqttInfo);

              asprintf(&rsp, return_Json_SMS_Data("RELAY_RESET_TIME"),
                       RELE1_NUMBER);
              return rsp;
            } else {
              return return_ERROR_Codes(
                  &rsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
            }
          } else if (releNumber == RELE2_NUMBER) {
            rele2_Bistate_Time = 1;

            nvs_set_u32(nvs_System_handle, NVS_RELAY2_BISTATE_TIME,
                        rele2_Bistate_Time);

            if (BLE_SMS_Indication == BLE_INDICATION ||
                BLE_SMS_Indication == UDP_INDICATION) {
              asprintf(&rsp, "%s %c %c %d", RELE2_ELEMENT, cmd, param,
                       rele2_Bistate_Time);
              return rsp;
            } else if (BLE_SMS_Indication == SMS_INDICATION) {
              char UDP_Rsp[50] = {};
              sprintf(mqttInfo->data, "%s %c %c %d", RELE2_ELEMENT, cmd, param,
                      rele2_Bistate_Time);
              // send_UDP_Send(UDP_Rsp);
              send_UDP_queue(mqttInfo);

              asprintf(&rsp, return_Json_SMS_Data("RELAY_RESET_TIME"),
                       RELE2_NUMBER);
              return rsp;
            } else {
              return return_ERROR_Codes(
                  &rsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
            }
          } else {
            // ////printf(ERROR_RESET);
            return return_ERROR_Codes(&rsp,
                                      return_Json_SMS_Data("ERROR_RESET"));
          }
        } else {
          // ////printf(ERROR_USER_NOT_PERMITION);
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
        }
      } else if (param == PAIRING_PARAMETER) {
        if (user_validateData->permition == '1' ||
            user_validateData->permition == '2') {
          if (releNumber == RELE1_NUMBER) {
            rele1_Restriction = 0;

            if (BLE_SMS_Indication == BLE_INDICATION ||
                BLE_SMS_Indication == UDP_INDICATION) {
              save_INT8_Data_In_Storage(NVS_KEY_RELAY_PAIRING_VALUE,
                                        rele1_Restriction, nvs_System_handle);

              asprintf(&rsp, "%s %c %c %d", RELE1_ELEMENT, cmd, param,
                       rele1_Restriction);
              return rsp;
            } else if (BLE_SMS_Indication == SMS_INDICATION) {
              save_INT8_Data_In_Storage(NVS_KEY_RELAY_PAIRING_VALUE,
                                        rele1_Restriction, nvs_System_handle);

              char UDP_Rsp[50] = {};
              sprintf(mqttInfo->data, "%s %c %c %d", RELE1_ELEMENT, cmd, param,
                      rele1_Restriction);
              // send_UDP_Send(UDP_Rsp);
              send_UDP_queue(mqttInfo);

              asprintf(&rsp,
                       return_Json_SMS_Data("RESTRICTION_OF_RELAY_1_DISABLED"));
              return rsp;
            } else {
              return return_ERROR_Codes(
                  &rsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
            }
          } else {
            // ////printf(ERROR_RESET);
            return return_ERROR_Codes(&rsp,
                                      return_Json_SMS_Data("ERROR_RESET"));
          }
        } else {
          // ////printf(ERROR_USER_NOT_PERMITION);
          return return_ERROR_Codes(
              &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
        }
      } else {
        // ////printf(ERROR_PARAMETER);
        return return_ERROR_Codes(&rsp,
                                  return_Json_SMS_Data("ERROR_PARAMETER"));
      }
    } else {
      if (param == RELE_PARAMETER && cmd == RESET_CMD) {
        if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD) ||
            network_Activate_Flag == 1) {
          sdCard_Logs_struct logs_struct;
          memset(&logs_struct, 0, sizeof(logs_struct));

          if (BLE_SMS_Indication == BLE_INDICATION) {
            sprintf(logs_struct.type, "%s", "BLE");
          } else if (BLE_SMS_Indication == SMS_INDICATION) {
            sprintf(logs_struct.type, "%s", "SMS");
          } else if (BLE_SMS_Indication == WIEGAND_INDICATION) {
            sprintf(logs_struct.type, "%s", mqttInfo->data);
            sprintf(logs_struct.phone, "%s", user_validateData->wiegand_code);
          }
          
          // ////printf("\nlogs_struct.name1 %s\n", logs_struct.name);
          // ////printf("\nlogs_struct.name2 %s\n",
          // user_validateData->firstName);
          sprintf(logs_struct.name, "%s", user_validateData->firstName);
          // ////printf("\nlogs_struct.name3 %s\n", logs_struct.name);
          // ////printf("\nlogs_struct.name4 %s\n",
          // user_validateData->firstName);
          sprintf(logs_struct.phone, "%s", user_validateData->phone);
          sprintf(logs_struct.relay, "%c%c", 'R', releNumber + 48);
          sprintf(logs_struct.relay_state, "%s",
                  return_Json_SMS_Data("NOT_CHANGE"));
          sprintf(logs_struct.date, "%s",
                  replace_Char_in_String(nowTime.strTime, ',', ';'));

          sprintf(logs_struct.error, "%s",
                  return_Json_SMS_Data("ERROR_LOGS_USER_NOT_PERMITION"));
          sdCard_Write_LOGS(&logs_struct);
        }
      }

      // ////printf(ERROR_USER_NOT_PERMITION);
      return return_ERROR_Codes(
          &rsp, return_Json_SMS_Data("ERROR_USER_NOT_PERMITION"));
    }
  } else {
    // ////printf(ERROR_CMD);
    return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_CMD"));
  }

  return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_CMD"));
}

void giveMonoSt_semaphore() { xSemaphoreGive(rdySem_RelayMonoInicial); }

void takeMonoSt_semaphore() {
  xSemaphoreTake(rdySem_RelayMonoInicial, portMAX_DELAY);
}

uint8_t setReles(int releNumber, uint8_t BLE_SMS_Indication, uint8_t gattsIF,
                 uint16_t connID, uint16_t handle_table,
                 data_EG91_Send_SMS *data_SMS, mqtt_information *mqttInfo) {

  char feedBackRelay[50];
  memset(feedBackRelay, 0, sizeof(feedBackRelay));
  // printf("\n\n logs_struct.type SMS 555 \n\n");
  data_BLE_Send_RelayState message;

  // printf("\n\n logs_struct.type SMS 666 \n\n");
  memset(&message, 0, sizeof(message));

  // ////printf("\n\n logs_struct.type SMS 777 \n\n");
  if (releNumber == RELE1_NUMBER || releNumber == 3) {
    // printf("\n\n logs_struct.type SMS 44 %d\n\n",
    // message.BLE_SMS_INDICATION);
    printf("setReles 11\n");
    if (releNumber == 3) {
      releNumber = 1;
    }

    if (rele1_Mode_Label == MONOESTABLE_MODE_INDEX) {
      printf("setReles 22\n");
      if (BLE_SMS_Indication != SMS_INDICATION) {
        if (label_MonoStableRelay1 != 1) {
          // printf("\n\n logs_struct.type SMS 55 %d\n\n",
          // message.BLE_SMS_INDICATION);

          printf("setReles 33\n");
          message.connID = connID;
          message.gattsIF = gattsIF;
          message.handle_table = handle_table;
          message.relaynumber = releNumber;

          printf("\n\n logs_struct.type SMS 66 %d\n\n",
                 message.BLE_SMS_INDICATION);
          if (BLE_SMS_Indication == BLE_INDICATION) {
            // printf("\n\n\n rele in bistate1\n\n\n");
            sprintf(message.mqttInfo_ble.topic, "%s", "");
            message.BLE_SMS_INDICATION = BLE_INDICATION;
          } else if (BLE_SMS_Indication == WIEGAND_INDICATION) {
            message.BLE_SMS_INDICATION = WIEGAND_INDICATION;
            sprintf(message.mqttInfo_ble.topic, "%s", "");
          } else if (BLE_SMS_Indication == UDP_INDICATION) {
            sprintf(message.mqttInfo_ble.topic, "%s", mqttInfo->topic);
            message.BLE_SMS_INDICATION = UDP_INDICATION;
          } else if (BLE_SMS_Indication == RF_INDICATION) {
            message.BLE_SMS_INDICATION = RF_INDICATION;
          } else if (BLE_SMS_Indication == REX_INDICATION) {
            message.BLE_SMS_INDICATION = REX_INDICATION;
          }
          // ////printf("\n\n logs_struct.type SMS 66 %d\n\n",
          // message.BLE_SMS_INDICATION);
          sprintf(feedBackRelay, "%s %c %c %d", RELE1_ELEMENT, SET_CMD,
                  RELE_PARAMETER, getRele1());
          // printf("\nfeedback relay- %s\n", feedBackRelay);
          sprintf(message.payload, "%s", feedBackRelay);
          // sprintf(message.mqttInfo_ble.data,"%s",feedBackRelay);

          printf("\n\n\n rele in bistate1\n\n\n");
          xQueueSendToBack(queue_BLE_Parameters1, (void *)&message,
                           pdMS_TO_TICKS(1000));

          return 1;
        } else {
          // printf("\n\n\n rele in bistate2\n\n\n");
          message.connID = connID;
          message.gattsIF = gattsIF;
          message.handle_table = handle_table;
          message.relaynumber = releNumber;

          if (BLE_SMS_Indication == BLE_INDICATION) {
            // ////printf("\n\n\n rele in bistate1\n\n\n");
            message.BLE_SMS_INDICATION = BLE_INDICATION;
          } else if (BLE_SMS_Indication == WIEGAND_INDICATION) {
            message.BLE_SMS_INDICATION = WIEGAND_INDICATION;
          } else if (BLE_SMS_Indication == UDP_INDICATION) {
            sprintf(message.mqttInfo_ble.topic, "%s", mqttInfo->topic);
            message.BLE_SMS_INDICATION = UDP_INDICATION;
          } else if (BLE_SMS_Indication == RF_INDICATION) {
            message.BLE_SMS_INDICATION = RF_INDICATION;
          } else if (BLE_SMS_Indication == REX_INDICATION) {
            message.BLE_SMS_INDICATION = REX_INDICATION;
          }

          sprintf(feedBackRelay, "%s %c %c %d", RELE1_ELEMENT, SET_CMD,
                  RELE_PARAMETER, getRele1());
          // ////printf("\nfeedback relay- %s\n", feedBackRelay);
          sprintf(message.payload, "%s", feedBackRelay);
          sprintf(message.mqttInfo_ble.data, "%s", feedBackRelay);

          xQueueSendToBack(queue_BLE_Parameters1, (void *)&message,
                           pdMS_TO_TICKS(1000));
          xTaskAbortDelay(rele1_Bistate_Task_Handle);

          return 1;
        }
        // }
      } else if (BLE_SMS_Indication == SMS_INDICATION ||
                 data_SMS->labelIncomingCall == 1) {
        if (label_MonoStableRelay1 != 1) {

          message.relaynumber = releNumber;
          message.BLE_SMS_INDICATION = (uint8_t)SMS_INDICATION;
          message.EG91_data = *data_SMS;

          xQueueSendToBack(queue_BLE_Parameters1, (void *)&message,
                           pdMS_TO_TICKS(100));
          // printf("\n label after cpy call2\n");
        } else {

          if (gpio_get_level(GPIO_OUTPUT_IO_0)) {
            message.relaynumber = releNumber;
            message.BLE_SMS_INDICATION = (uint8_t)SMS_INDICATION;
            message.EG91_data = *data_SMS;
            sprintf(message.EG91_data.payload,
                    return_Json_SMS_Data("RENEWED_RELAY_PULSE_TIME"),
                    RELE1_NUMBER);
            xTaskAbortDelay(rele1_Bistate_Task_Handle);
            // sprintf(feedBackRelay, "%s",message.EG91_data.payload);
            xQueueSendToBack(queue_BLE_Parameters1, (void *)&message,
                             pdMS_TO_TICKS(1000));

            if (message.EG91_data.labelRsp == 1) {
              xQueueSendToBack(queue_EG91_SendSMS, (void *)&message.EG91_data,
                               pdMS_TO_TICKS(1000));
            }
          } else {
            message.relaynumber = releNumber;
            message.BLE_SMS_INDICATION = (uint8_t)SMS_INDICATION;
            message.EG91_data = *data_SMS;

            xQueueSendToBack(queue_BLE_Parameters1, (void *)&message,
                             pdMS_TO_TICKS(1000));
          }
        }

        return 1;
      } else {
        return 0;
      }
    } else if (rele1_Mode_Label == BIESTABLE_MODE_INDEX) {
      setRele1();
      if (BLE_SMS_Indication == UDP_INDICATION) {
        sprintf(feedBackRelay, "%s %c %c %d", RELE1_ELEMENT, SET_CMD,
                RELE_PARAMETER, getRele1());
        sprintf(message.mqttInfo_ble.topic, "%s", mqttInfo->topic);
        sprintf(message.mqttInfo_ble.data, "%s", feedBackRelay);
        send_UDP_queue(&message.mqttInfo_ble);
      }
      return 1;
    } else {
      return 0;
    }
  }

  if (releNumber == RELE2_NUMBER || releNumber == 3) {

    if (releNumber == 3) {
      releNumber = 2;
    }
    // printf("\n\n set set rele 2 rele 2\n\n");
    if (rele2_Mode_Label == MONOESTABLE_MODE_INDEX) {
      if (BLE_SMS_Indication != SMS_INDICATION) {
        if (label_MonoStableRelay2 != 1) {

          message.connID = connID;
          message.gattsIF = gattsIF;
          message.handle_table = handle_table;
          message.relaynumber = releNumber;
          // message.BLE_SMS_INDICATION = BLE_INDICATION;

          // //printf("\nfeedback relay- %s\n", feedBackRelay);
          sprintf(message.payload, "%s", feedBackRelay);

          if (BLE_SMS_Indication == BLE_INDICATION) {
            // ////printf("\n\n\n rele in bistate1\n\n\n");
            message.BLE_SMS_INDICATION = BLE_INDICATION;
          } else if (BLE_SMS_Indication == WIEGAND_INDICATION) {
            message.BLE_SMS_INDICATION = WIEGAND_INDICATION;
          } else if (BLE_SMS_Indication == UDP_INDICATION) {
            sprintf(message.mqttInfo_ble.topic, "%s", mqttInfo->topic);
            message.BLE_SMS_INDICATION = UDP_INDICATION;
          } else if (BLE_SMS_Indication == RF_INDICATION) {
            message.BLE_SMS_INDICATION = RF_INDICATION;
          } else if (BLE_SMS_Indication == REX_INDICATION) {
            message.BLE_SMS_INDICATION = REX_INDICATION;
          }

          // printf("\n\n logs_struct.type SMS 66
          // %d\n\n",message.BLE_SMS_INDICATION);
          sprintf(feedBackRelay, "%s %c %c %d", RELE2_ELEMENT, SET_CMD,
                  RELE_PARAMETER, getRele2());
          // ////printf("\nfeedback relay- %s\n", feedBackRelay);
          sprintf(message.payload, "%s", feedBackRelay);

          // ////printf("\n\n\n rele in bistate1\n\n\n");

          xQueueSendToBack(queue_BLE_Parameters2, (void *)&message,
                           (TickType_t)0);

          return 1;
        } else {
          // ////printf("\n\n\n rele in bistate3\n\n\n");
          message.connID = connID;
          message.gattsIF = gattsIF;
          message.handle_table = handle_table;
          message.relaynumber = releNumber;

          if (BLE_SMS_Indication == BLE_INDICATION) {
            // ////printf("\n\n\n rele in bistate1\n\n\n");
            message.BLE_SMS_INDICATION = BLE_INDICATION;
          } else if (BLE_SMS_Indication == WIEGAND_INDICATION) {
            message.BLE_SMS_INDICATION = WIEGAND_INDICATION;
          } else if (BLE_SMS_Indication == UDP_INDICATION) {
            sprintf(message.mqttInfo_ble.topic, "%s", mqttInfo->topic);
            message.BLE_SMS_INDICATION = UDP_INDICATION;
          } else if (BLE_SMS_Indication == RF_INDICATION) {
            message.BLE_SMS_INDICATION = RF_INDICATION;
          } else if (BLE_SMS_Indication == REX_INDICATION) {
            message.BLE_SMS_INDICATION = REX_INDICATION;
          }

          sprintf(feedBackRelay, "%s %c %c %d", RELE2_ELEMENT, SET_CMD,
                  RELE_PARAMETER, getRele2());
          // ////printf("\nfeedback relay- %s\n", feedBackRelay);
          sprintf(message.payload, "%s", feedBackRelay);
          sprintf(message.mqttInfo_ble.data, "%s", feedBackRelay);

          xQueueSendToBack(queue_BLE_Parameters2, (void *)&message,
                           pdMS_TO_TICKS(1000));
          xTaskAbortDelay(rele2_Bistate_Task_Handle);

          /*  if (gpio_get_level(GPIO_OUTPUT_IO_0))
           {
               data_BLE_Send_RelayState cpy_relay_message;
           } */

          return 1;
        }
      } else if (BLE_SMS_Indication == SMS_INDICATION) {

        if (label_MonoStableRelay2 != 1) {

          message.relaynumber = releNumber;
          message.BLE_SMS_INDICATION = (uint8_t)SMS_INDICATION;
          message.EG91_data = *data_SMS;

          // ////printf("\nmessage struct relay %d - indiciation %d - payload %s
          // - rsp %d - incall %d ph %s", message.relaynumber,
          // message.BLE_SMS_INDICATION, message.EG91_data.payload,
          // message.EG91_data.labelRsp, message.EG91_data.labelIncomingCall,
          // message.EG91_data.phoneNumber);

          // ////printf("\n label cpy rsp sms %d - %d - %s\n",
          // message.EG91_data.labelRsp, message.EG91_data.labelRsp,
          // message.EG91_data.phoneNumber);
          char txt111[] = "qwerty";
          xQueueSendToBack(queue_BLE_Parameters2,
                           /* (void *)&txt111 */ (void *)&message,
                           pdMS_TO_TICKS(100));
          // ////printf("\n label after cpy call2\n");
          // ////printf("\nafter send queue 1\n");
        } else {
          if (gpio_get_level(GPIO_OUTPUT_IO_1)) {

            message.relaynumber = releNumber;
            message.BLE_SMS_INDICATION = (uint8_t)SMS_INDICATION;
            message.EG91_data = *data_SMS;
            sprintf(message.EG91_data.payload,
                    return_Json_SMS_Data("RENEWED_RELAY_PULSE_TIME"),
                    RELE2_NUMBER);
            xTaskAbortDelay(rele2_Bistate_Task_Handle);

            xQueueSendToBack(queue_BLE_Parameters2,
                             /* (void *)&txt111 */ (void *)&message,
                             pdMS_TO_TICKS(100));

            if (message.EG91_data.labelRsp == 1) {
              xQueueSendToBack(queue_EG91_SendSMS, (void *)&message.EG91_data,
                               pdMS_TO_TICKS(1000));
            }
          } else {
            message.relaynumber = releNumber;
            message.BLE_SMS_INDICATION = (uint8_t)SMS_INDICATION;
            message.EG91_data = *data_SMS;

            xQueueSendToBack(queue_BLE_Parameters2,
                             /* (void *)&txt111 */ (void *)&message,
                             pdMS_TO_TICKS(100));
          }
        }

        return 1;
      }

      return 1;
    } else if (rele2_Mode_Label == BIESTABLE_MODE_INDEX) {
      setRele2();

      if (BLE_SMS_Indication == UDP_INDICATION) {
        sprintf(feedBackRelay, "%s %c %c %d", RELE2_ELEMENT, SET_CMD,
                RELE_PARAMETER, getRele2());
        sprintf(message.mqttInfo_ble.data, "%s", feedBackRelay);
        sprintf(message.mqttInfo_ble.topic, "%s", mqttInfo->topic);
        send_UDP_queue(&message.mqttInfo_ble);
      }

      return 1;
    }
  } else {
    return 0;
  }
  return 0;
}
void setRele1() {
  int level = getRele1();
  // ////printf("\nsetRele1\n");
  gpio_set_level(GPIO_OUTPUT_IO_0, !level);
  save_INT8_Data_In_Storage(NVS_KEY_RELAY1_LAST_VALUE, !level,
                            nvs_System_handle);
}

void setRele2() {
  int level = getRele2();
  gpio_set_level(GPIO_OUTPUT_IO_1, !level);
  save_INT8_Data_In_Storage(NVS_KEY_RELAY2_LAST_VALUE, !level,
                            nvs_System_handle);
}

void resetRele1() {
  if (label_MonoStableRelay1 == 1) {
    label_MonoStableRelay1 = 0;
    xTaskAbortDelay(rele1_Bistate_Task_Handle);
  }

  gpio_set_level(GPIO_OUTPUT_IO_0, 0);
  save_INT8_Data_In_Storage(NVS_KEY_RELAY1_LAST_VALUE, 0, nvs_System_handle);
}

void resetRele2() {
  if (label_MonoStableRelay2 == 1) {
    label_MonoStableRelay2 = 0;
    xTaskAbortDelay(rele2_Bistate_Task_Handle);
  }
  gpio_set_level(GPIO_OUTPUT_IO_1, 0);
  save_INT8_Data_In_Storage(NVS_KEY_RELAY2_LAST_VALUE, 0, nvs_System_handle);
}

uint8_t getRele1() { return gpio_get_level(GPIO_OUTPUT_IO_0); }

uint8_t getRele2() { return gpio_get_level(GPIO_OUTPUT_IO_1); }
