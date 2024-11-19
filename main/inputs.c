/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|

*/

#include "inputs.h"
#include "esp_err.h"
#include "gpio.h"
#include "keeloqDecrypt.h"
#include "nvs.h"
#include "users.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include "rele.c"
#include "core.h"
#include "rf.h"



/**
 * @brief Function to read inputs (Digital Inputs)
 *
 * @param BLE_SMS_Indication Indicates the type of the received command
 * @param inputNumber Number of the input to read
 * @param cmd Command received
 * @param param Parameter received
 * @param phPassword Password received
 * @param payload Payload received
 * @return char* String with the response
 */
char *readInputs(uint8_t BLE_SMS_Indication, uint8_t inputNumber, char cmd,
                 char param, char *phPassword, char *payload) {

  char *rsp = NULL /* (char *)malloc(200 * sizeof(char)) */;
  /*memset(&rsp, 0, sizeof(rsp));*/
  if (cmd == GET_CMD) {

    if (param == INPUT_PARAMETER) {
      if (inputNumber == INPUT1_NUMBER) {
        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {
          // Response in JSON format
          asprintf(&rsp, "%s %c %c %d", INPUT1_ELEMENT, cmd, param,
                   read_Input1());
          //////printf("\nread inputs %s ",rsp);
          return rsp;
        } else if (BLE_SMS_Indication == SMS_INDICATION) {
          // Response in SMS format
          if (read_Input1()) {
            asprintf(&rsp, return_Json_SMS_Data("INPUT_ON"), 1, "<05>");
            //////printf("\nread inputs %s ",rsp);
            return rsp;
          } else {
            asprintf(&rsp, return_Json_SMS_Data("INPUT_OFF"), 1, "<06>");
            return rsp;
          }
        } else {
          return return_ERROR_Codes(&rsp,
                                    return_Json_SMS_Data("ERROR_INPUT_DATA"));
        }
      } else if (inputNumber == INPUT2_NUMBER) {

        if (BLE_SMS_Indication == BLE_INDICATION ||
            BLE_SMS_Indication == UDP_INDICATION) {
          // Response in JSON format
          asprintf(&rsp, "%s %c %c %d", INPUT2_ELEMENT, cmd, param,
                   read_Input2());
          //////printf("\nread inputs %s ",rsp);
          return rsp;
        } else if (BLE_SMS_Indication == SMS_INDICATION) {
          // Response in SMS format
          if (read_Input2()) {
            asprintf(&rsp, return_Json_SMS_Data("INPUT_ON"), 2, "<07>");
            return rsp;
          } else {
            asprintf(&rsp, return_Json_SMS_Data("INPUT_OFF"), 2, "<08>");
            return rsp;
          }
        } else {
          return return_ERROR_Codes(&rsp,
                                    return_Json_SMS_Data("ERROR_INPUT_DATA"));
        }
      } else {

        //////printf(ERROR_GET);
        return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_GET"));
      }
    } else {

      //////printf(ERROR_PARAMETER);
      return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_PARAMETER"));
    }
  } else {

    //////printf(ERROR_CMD);
    return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_CMD"));
  }
  return "ERROR READ INPUTS";
}
int read_Input1() { return !gpio_get_level(GPIO_INPUT_IO_0); }

int read_Input2() { return !gpio_get_level(GPIO_INPUT_IO_1); }
