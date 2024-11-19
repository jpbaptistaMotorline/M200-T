#include "rf.h"
#include "EG91.h"
#include "cmd_list.h"
#include "core.h"
#include "esp_err.h"
#include "freertos/projdefs.h"
#include "sdCard.h"
#include "users.h"
// #include <cstdio>
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_spiffs.h"
#include "users.h"
#include "wiegand.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

uint8_t BufferRF[9];           // receive buffer
uint8_t BufferRF_12BIT[2];     // temp receive buffer for Validation
static unsigned char RFstate;  // receiver state
static signed int RFcount;     // timer counter
static uint8_t Bptr;           // receive buffer pointer
static uint8_t BitCount;       // received bits counter
volatile uint8_t RFFull;       // buffer full
volatile uint8_t RFFull_12BIT; // Buffer full for 12Bit
volatile uint8_t RFBit;        // sampled RF signal

classic_encoder_t classic_data;

volatile RF_KEY_STRUCT rfCMD;

uint8_t rf_mode_label = 0;
uint8_t rf_autoSAve_relay = 0;

char file_contents[310];

char rf_save_userNumber[50] = {};
char rf_save_userButton = 0;

static const char *TAG = "FreeRTOS Timer";
TimerHandle_t rf_autoSave_xTimer;

uint8_t BLE_SMS_Indication_rf_autoSave = 0;
uint8_t gattsIF_autoSave = 0;
uint16_t connID_autoSave = 0;
uint16_t handle_table_autoSave = 0;

// Função de callback do timer
void timer_callback(void *arg) {

  rxi();

  // Inverte o estado do pino GPIO a cada ciclo de timer
  // output_state = !output_state;
  // gpio_set_level(OUTPUT_GPIO, output_state);

  // printf("Output state: %d\n", output_state);
}

/******************************************************************************/
void rxi(void) {
  // this routine gets called every time TMR0 overflows
  RFBit =
      gpio_get_level(OUTPUT_GPIO); // HAL_GPIO_ReadPin(RFIN_GPIO_Port,
                                   // RFIN_Pin); // sampling RF pin verify!!!
  // TMR0 -= PERIOD; // reload
  // T0IF = 0;
  // XTMR++; // extended 16 long timer update
  if (RFFull) // avoid overrun
    return;

  switch (RFstate) // state machine main switch
  {
  case TRFUNO:
    if (RFBit == 0) { // falling edge detected ----+
      // |? 2001 Microchip Technology Inc. Preliminary DS00744A-page 17 AN744
      // +----
      RFstate = TRFZERO;
    } else { // while high
      RFcount--;
      if (RFcount < HIGH_TO) {

        RFstate = TRFreset; // reset if too long
      }
    }
    break;
  case TRFZERO:
    if (RFBit) { // rising edge detected +----
      // |
      // ----+
      RFstate = TRFUNO;
      BufferRF[Bptr] >>= 1; // rotate
      if (RFcount >= 0) {
        BufferRF[Bptr] += 0x80; // shift in bit
      } else {
        // EUSART_Write('0');
      }
      RFcount = 0; // reset length counter

      if ((++BitCount & 7) == 0)
        Bptr++; // advance one byte
      if (BitCount == NBIT) {
        RFstate = TRFreset; // finished receiving
        RFFull = 1;
      }
    } else { // still low
      RFcount++;
      if (RFcount >= LOW_TO) // too long low
      {

        RFstate = TRFSYNC; // fall back into RFSYNC state
        Bptr = 0;          // reset pointers, while keep counting on
        BitCount = 0;
      }
    }
    break;
  case TRFSYNC:
    if (RFBit) { // rising edge detected +---+ +---..
      // | | <-Theader-> |
      // +----------------+
      if ((RFcount < SHORT_HEAD) || (RFcount >= LONG_HEAD)) {
        if ((RFcount < SHORT_HEAD_12BIT) || RFcount >= LONG_HEAD_12BIT) {
          RFstate = TRFreset;
        } else {
          RFcount = -10; // restart counter
          RFstate = TRFUNO_12BIT;
        }
      } else {
        RFcount = 0; // restart counter
        RFstate = TRFUNO;
      }
    } else { // still low
      RFcount++;
    }
    break;
    // alTERAÇÃO DO ALGORITMO PARA 12 bIT //
  case TRFUNO_12BIT:
    if (RFBit == 0) { // falling edge detected ----+
      // |? 2001 Microchip Technology Inc. Preliminary DS00744A-page 17 AN744
      // +----
      RFstate = TRFZERO_12BIT;
      BufferRF[Bptr] >>= 1; // rotate
      if (RFcount >= 0) {
        BufferRF[Bptr] += 0x80; // shift in bit
      }
      RFcount = 0; // reset length counter

      if ((++BitCount & 7) == 0) {
        Bptr++; // advance one byte
        BufferRF[Bptr] = 0;
      }
      if (BitCount == NBIT_12BIT) {
        RFstate = TRFreset; // finished receiving
        if (RFFull_12BIT == 1) {
          if (BufferRF_12BIT[0] == BufferRF[0] &&
              BufferRF_12BIT[1] == BufferRF[1]) {
            RFFull = 1;
            BufferRF[2] = 0;
            BufferRF[3] = 0;
          } else {
            BufferRF_12BIT[0] = BufferRF[0];
            BufferRF_12BIT[1] = BufferRF[1];
          }

        } else {
          RFFull_12BIT = 1;
          BufferRF_12BIT[0] = BufferRF[0];
          BufferRF_12BIT[1] = BufferRF[1];
        }
      }
    } else { // while high
      RFcount++;
      if (RFcount >= LOW_TO) {

        RFstate = TRFreset; // reset if too long
      }
    }
    break;
  case TRFZERO_12BIT:
    if (RFBit) { // rising edge detected +----
      // |
      // ----+

      RFstate = TRFUNO_12BIT;

    } else { // still low
      RFcount--;
      if (RFcount < HIGH_TO) // too long low
      {

        RFstate = TRFSYNC; // fall back into RFSYNC state
        Bptr = 0;          // reset pointers, while keep counting on
        BitCount = 0;
      }
    }
    break;
  // AN744 DS00744A-page 18 Preliminary ? 2001 Microchip Technology Inc.
  case TRFreset:
  default:
    RFstate = TRFSYNC; // reset state machine in all other cases
    RFcount = 0;
    Bptr = 0;
    BitCount = 0;
    break;

  } // switch

} // rxi
/******************************************************************************/

uint8_t activate_rollingCode(){
  return save_INT8_Data_In_Storage(NVS_RF_ROLLING_CODE, 1, nvs_System_handle);
}
uint8_t deactivate_rollingCode(){
  return save_INT8_Data_In_Storage(NVS_RF_ROLLING_CODE, 0, nvs_System_handle);
}

void InitReceiver(void) {
  RFstate = TRFreset; // reset state machine in all other cases
  RFFull_12BIT = 0;   // Mark buffer 12BIT empty
  RFFull = 0;         // start with buffer empty
}
/******************************************************************************/
signed char getKeyPressed(void) {
  signed char temp = -1;
  if (RFFull)
    temp = (BufferRF[7] & 0xF0) >>
           4; // Remove os 4 bit do byte 7 buffer corresponde ao ultimo byte do
              // serial number e favria comfrome os teclas pressionada
  return temp;
}

/******************************************************************************/
/* This task reads the RF keys state and resets the processed key indicator   */
/* flag. It runs at the main timeslot frequency, currently at 50ms            */
/******************************************************************************/
void readRFButtons(void) {
  ////////////////////////////////////////////////////////////////////////////
  static unsigned char key1 = 0;
  static uint32_t serial = 0;
  static uint16_t rollingCode = 0;
  static TypeCMD keyType = 0;
  static TypeRemote_st typeRemote;
  // printf("\n\n rolling code333 :%x - %x\n\n", rfCMD.currentRolling,
  // rollingCode);
  typeRemote = classic_data.type;
  ////////////////////////////////////////////////////////////////////////////
  if (RFFull) // Verifica se existe alguma tecla RF permida
  {
    ////////////////////////////////////////////////////////////////////////
    if (RFFull_12BIT == 0)
      keyType = keeloqDecryptPacket(
          BufferRF,
          &classic_data); // Faz desencyptação do comando keeloQ recebido
    else
      keyType = DecryptPacket_12BIT(
          BufferRF,
          &classic_data); // faz desencyptação do comando 12 bIT recebido
    ////////////////////////////////////////////////////////////////////////
    key1 = classic_data.fcode1;
    serial = classic_data.serialnumber;
    rollingCode = classic_data.sync;

    // printf("\n\n rolling code :%x - %x\n\n", rfCMD.currentRolling,
    // rollingCode);
    typeRemote = classic_data.type;

    ////////////////////////////////////////////////////////////////////////
    /*    if(keyType == NoCMD || (keyType != rfCMD.currentType &&
       ts_system.TimeLastCommand <= 0 && rfCMD.currentRolling != rollingCode) ||
       rfCMD.currentKey != key1 || rfCMD.currentSerial != serial)
       {
          rfCMD.processed = 0;
       }
       else if(ts_system.TimeLastCommand <= 0 && typeRemote == DIP_12BIT)
       {
          rfCMD.processed = 0;
       } */
    ////////////////////////////////////////////////////////////////////////
    // ts_system.TimeLastCommand = TIME_500MS;	//Renicia o contador entre
    // comandos válidos. Ou seja tem que estar 500Ms sem receber qualquer
    // comando para aceitar um novo comando.
    ////////////////////////////////////////////////////////////////////////
    rfCMD.currentType = keyType;
    ////////////////////////////////////////////////////////////////////////
    if (rfCMD.currentType == NoCMD || rfCMD.currentKey != key1 ||
        rfCMD.currentSerial != serial || rfCMD.currentRolling != rollingCode) {
      rfCMD.currentKey = key1;
      rfCMD.currentSerial = serial;
      rfCMD.currentPosMem = classic_data.positionMem;
    }
    ////////////////////////////////////////////////////////////////////////
    rfCMD.currentRolling = rollingCode;
    ////////////////////////////////////////////////////////////////////////
  }
  // printf("serial %ld\n\n", rfCMD.currentSerial);
  ////////////////////////////////////////////////////////////////////////////
  if (rfCMD.processed == 1)
    rfCMD.currentType = NoCMD;
  else if (rfCMD.currentType == TypeProgrammingModeFull ||
           rfCMD.currentType == TypeProgrammingModeWalk) {
    if (rfCMD.timer < 250) {
      rfCMD.timer++;
    }
  }
  ////////////////////////////////////////////////////////////////////////////
}

/******************************************************************************/

/******************************************************************************/
signed char getSerialCmd(unsigned long *serial) {
  signed char temp = -1;
  if (RFFull) {
    if (RFFull_12BIT) {
      memcpy((char *)serial, &classic_data.serialnumber, 4);
    } else {
      memcpy((char *)serial, (char *)BufferRF + 4, 4);
    }
    temp = classic_data.fcode1; // KeyLOQ
  }
  return temp;
}
/******************************************************************************/

/*************  ✨ Codeium Command 🌟  *************/
/**
 * @brief Enters the RF relay mode based on the serial number and button press.
 *
 * This function takes a serial number and a button press as input and checks if
 * the serial number exists in the user database. If the serial number exists,
 * the function checks if the button press matches the relay number associated
 * with the serial number and turns on the relay if it does.
 *
 * @param serial The serial number for RF communication.
 * @param button The button pressed to trigger the relay mode.
 *
 * @returns None
 */
void rf_relayMode(uint64_t serial, char button) {
  char rfSerial_payload[30] = {};
  char rfSerialDataUser[200] = {};
  MyUser rfSerialDataStruct;
  char *rf_str;
  mqtt_information mqttInfo;
  uint8_t releNumber = 0;

  char rfSerial[20];
  sprintf(rfSerial, "&%llx", serial);

  memset(&rfSerialDataStruct, 0, sizeof(rfSerialDataStruct));

  // Check if the serial number exists in the user database
  if (checkIf_rfSerialExist(rfSerial, rfSerial_payload)) {
    // printf("\n\nrf1 serial %s\n\n", rfSerial_payload);

    // Search for the user associated with the serial number
    if (MyUser_Search_User(rfSerial_payload, rfSerialDataUser) == ESP_OK) {

      // Parse the user data
      parse_ValidateData_User(rfSerialDataUser, &rfSerialDataStruct);

      // Check if the button press matches the relay number associated with
      // the serial number
      if (button + 48 == rfSerialDataStruct.rf1_relay &&
          button + 48 == rfSerialDataStruct.rf2_relay) {
        releNumber = 3;
      } else if (button + 48 == rfSerialDataStruct.rf1_relay) {
        releNumber = 1;

      } else if (button + 48 == rfSerialDataStruct.rf2_relay) {
        releNumber = 2;
      }
      else {
        printf("\n\nrelay rf number %c -- %c\n\n", button + 48,
      rfSerialDataStruct.rf1_relay);
        return;
      } /* else if (button + 48 == rfSerialDataStruct.rf3_relay) {
        releNumber = 3;
      } */

      
      if (releNumber == 3) {
        // If the relay number is 3, turn on relay 1 and 2
        rf_str = parse_ReleData(
            RF_INDICATION, 1, 'S', 'R', rfSerialDataStruct.key, NULL,
            &rfSerialDataStruct, NULL, NULL, NULL, NULL, &mqttInfo);

        rf_str = parse_ReleData(
            RF_INDICATION, 2, 'S', 'R', rfSerialDataStruct.key, NULL,
            &rfSerialDataStruct, NULL, NULL, NULL, NULL, &mqttInfo);
      } else {
        // If the relay number is not 3, turn on the relay associated with
        // the serial number
        rf_str = parse_ReleData(
            RF_INDICATION, releNumber, 'S', 'R', rfSerialDataStruct.key,
            NULL, &rfSerialDataStruct, NULL, NULL, NULL, NULL, &mqttInfo);
      }

      // Free the memory allocated for the parsed user data
      free(rf_str);
    } else {
      printf("\n\n search user fail\n\n");
    }
  } else {
    printf("\n\n RF not exist\n\n");
  }
} ///@param pvParameter
/**
 * @brief Task responsible for reading the RF signals and performing the
 * appropriate actions.
 *
 * @param pvParameter Pointer to a structure containing the data necessary
 * for the task.
 */
void taskMipot_rf(void *pvParameter) {

  gpio_pad_select_gpio(OUTPUT_GPIO);
  gpio_set_direction(OUTPUT_GPIO, GPIO_MODE_INPUT);
  InitReceiver();
  char outputData_rf[200] = {};

  uint8_t label_roll = 1;
  uint8_t label_roll_auth = 0;
  // RF_mode = 0;
  //  Configuração do timer
  const esp_timer_create_args_t periodic_timer_args = {
      .callback = &timer_callback, .name = "periodic_timer"};
  esp_timer_handle_t periodic_timer;
  esp_timer_create(&periodic_timer_args, &periodic_timer);
  uint8_t rf_save_relay = 2;

  // rf_mode = RF_RELAY_MODE;
  rf_mode = /* RF_AUTOSAVE_MODE;// */ /* RF_BUTTONSAVE_MODE;  */ RF_RELAY_MODE;
  char lastButton = 0;
  uint64_t lastSerial = 0;

  // rf_mode = RF_SAVE_MODE;// RF_AUTOSAVE_MODE;
  label_roll_auth = 1;

  ESP_LOGI(TAG, "parseRF_data: creating timer");
  xTimer_autoadd_rf = xTimerCreate(
      "Timer_auto_save",      // Nome do timer
      TIMER_AUTO_SAVE_PERIOD, // Período do timer (em ticks)
      pdFALSE,   // pdTRUE para reiniciar automaticamente (auto-reload)
      (void *)2, // ID do timer
      vTimer_autoSaveRF_callback // Função de callback
  );

  // sprintf(rf_save_userNumber, "%s", "+3514321");
  //  Inicia o timer
  esp_timer_start_periodic(periodic_timer, PERIOD_US);

  while (1) {

    uint64_t y = 0;

    readRFButtons();

    char x = /* rfCMD.currentKey; //  */ getKeyPressed();
    getSerialCmd(&y);         //
    y &= rfCMD.currentSerial; // 0x0FFFFFFF;
    //printf("\n\n key pressed : %c - %llx\n\n", x + 48, y);

    if (y != 0) {

      if (lastButton != x && lastSerial != y) {

        if (get_INT8_Data_From_Storage(NVS_RF_ROLLING_CODE,
                                     nvs_System_handle) == 1) {

          if (rfCMD.currentType != Keeloq_NoRollingCode) {

            printf("\n\n No rolling code\n\n");
            label_roll_auth = 0;

          } else {
            label_roll_auth = 1;
            printf("\n\n é rolling code\n\n");
          }
        } else {
          label_roll_auth = 1;
        }

        if (label_roll_auth == 1) {
          if (rf_mode == RF_RELAY_MODE) {
            rf_relayMode(y, x);
          } else if (rf_mode == RF_AUTOSAVE_MODE) {

            if (xTimer_autoadd_rf != NULL) {
              if (xTimerIsTimerActive(xTimer_autoadd_rf) == pdFALSE) {
                rf_mode = RF_RELAY_MODE;
              } else {
                char auto_rsp[250] = {};
                if (xTimerChangePeriod(xTimer_autoadd_rf,
                                       TIMER_AUTO_SAVE_PERIOD, 0) == pdPASS) {

                  printf("\n\n é START AUTOSAVE %c\n\n", x);
                  memset(outputData_rf, 0, sizeof(outputData_rf));
                  uint8_t ACK =
                      rf_auto_save(x + 48, y, rf_autoSAve_relay, outputData_rf);

                  if (BLE_SMS_Indication_rf_autoSave == BLE_INDICATION || BLE_SMS_Indication_rf_autoSave == UDP_INDICATION) {
                    
                    if (ACK == ESP_OK) {

                      sprintf(auto_rsp, "%s", outputData_rf);
                    } else {
                      sprintf(auto_rsp, "RF S S #");
                    }
                    esp_ble_gatts_send_indicate(
                        gattsIF_autoSave, connID_autoSave,
                        handle_table_autoSave, strlen(auto_rsp),
                        (uint8_t *)auto_rsp, false);
                  }
                  send_UDP_Send(auto_rsp, "");

                } else {
                  rf_mode = RF_RELAY_MODE;
                }
              }
            }
          } else if (rf_mode == RF_GET_RF_MODE) {
            char auto_rsp[50] = {};
            sprintf(auto_rsp, "RF G F %d.%llx", x, y);
            if (BLE_SMS_Indication_rf_autoSave == BLE_INDICATION) {
              
              esp_ble_gatts_send_indicate(
                  gattsIF_autoSave, connID_autoSave, handle_table_autoSave,
                  strlen(auto_rsp), (uint8_t *)auto_rsp, false);
            }
            else {
              send_UDP_Send(auto_rsp, "");
            }
            
            rf_mode = RF_RELAY_MODE;

          } else if (rf_mode == RF_SAVE_MODE) {
            printf("\n\n é START SAVE %c\n\n", x);
            // put_rf_to_user(y, x + 48, rf_save_relay, rf_save_userNumber);
          } else if (rf_mode == RF_BUTTONSAVE_MODE) {
            put_button_on_rfUser(x + 48, y, rf_save_relay);
          }
        } else {
          printf("\n\n não é autorizado\n\n");
        }
        // rfCMD.currentType = 0;
      }

      /* else {
        lastButton = 0;
        lastSerial = 0;
      } */
    }
    lastButton = x;
    lastSerial = y;
    // x=0;
    // y=0;
    InitReceiver();
    vTaskDelay(pdMS_TO_TICKS(250));
  }
  if (rf_mode_label == 0) {
  }
} /******  3baf5171-efd1-4f3a-925d-b20ae15c0c35  *******/

/**
 * Places a button on a remote control user interface.
 *
 * @param button The button to be placed on the remote control.
 * @param serial The serial number of the remote control.
 * @param relay The relay number associated with the button.
 *
 * @returns The contents of the file after placing the button on the remote
 * control user interface.
 */
char *put_button_on_rfUser(char button, uint64_t serial, uint8_t relay) {
  char aux_rf_userPayload[50] = {};
  char aux_rfSerial[50] = {};
  char userPayload[200] = {};
  MyUser user_rf_to_user;
  memset(&user_rf_to_user, 0, sizeof(user_rf_to_user));
  memset(file_contents, 0, sizeof(user_rf_to_user));

  printf("\n\nput_button_on_rfUser - %c - %llu - %u\n\n", button, serial,
         relay);
  sprintf(aux_rfSerial, "&%llx", serial);
  if (checkIf_rfSerialExist(aux_rfSerial, aux_rf_userPayload)) {
    printf("\n\ncheckIf_rfSerialExist - OK\n\n");
    MyUser_Search_User(aux_rf_userPayload, userPayload);
    {

      parse_ValidateData_User(userPayload, &user_rf_to_user);
      printf("\n\nparse_ValidateData_User - OK\n\n");
      /*  if (button != user_rf_to_user.rf1_relay &&
           button != user_rf_to_user.rf2_relay &&
           button != user_rf_to_user.rf3_relay) { */

      printf("\n\nbutton is not already in use - OK\n\n");
      if (relay == RF1_USER_POSITION) {
        user_rf_to_user.rf1_relay = button;
      } else if (relay == RF2_USER_POSITION) {
        user_rf_to_user.rf2_relay = button;
      } /* else if (relay == RF3_USER_POSITION) {
        user_rf_to_user.rf3_relay = button;
      } */

      replaceUser(&user_rf_to_user);

      memset(userPayload, 0, sizeof(userPayload));

      MyUser_Search_User(aux_rf_userPayload, userPayload);
      sprintf(file_contents, "%s", userPayload);

      return file_contents;
      /* } else {
        printf("\n\nbutton is already in use - ERROR\n\n");
        return "ERROR";
      } */
    }
  } else {
    printf("\n\ncheckIf_rfSerialExist - ERROR\n\n");
    return "ERROR";
  }
}

/*************  ✨ Codeium Command 🌟  *************/
/*************  ✨ Codeium Command 🌟  *************/
/*************  ✨ Codeium Command 🌟  *************/
/*************  ✨ Codeium Command 🌟  *************/
/*************  ✨ Codeium Command 🌟  *************/
/*************  ✨ Codeium Command 🌟  *************/
/*************  ✨ Codeium Command 🌟  *************/
/*************  ✨ Codeium Command 🌟  *************/
/**
 * Updates user information with the given RF number, button number, and relay
 * number.
 *

 * @param rf_number The RF number to be associated with the user.
 * @param button_number The button number to be assigned to the user.
 * @param relayNumber The relay number to be updated for the user.
 * @param user_number The user number to update the information.
 *
 * @returns The updated user information or "ERROR" if an error occurs.
 */
char *put_rf_to_user(char *payload) {

  char userPayload[200] = {};
  char rfSerial[20] = {};
  char button_relay1 = 0;
  char button_relay2 = 0;
  char userId[50] = {};
  uint8_t dotCount = 0;
  uint8_t strIndex = 0;

  printf("\n\n[put_rf_to_user] payload: %s\n\n", payload);

  MyUser user_rf_to_user;
  memset(&user_rf_to_user, 0, sizeof(user_rf_to_user));

  for (int i = 0; i < strlen(payload); i++) {

    if (payload[i] == '.') {
      dotCount++;
      strIndex = 0;
    } else {
      if (dotCount == 0) {
        userId[strIndex] = payload[i];
        strIndex++;
      } else if (dotCount == 1) {
        rfSerial[strIndex] = payload[i];
        strIndex++;
      } else if (dotCount == 2) {
        button_relay1 = payload[i];
      } else if (dotCount == 3) {
        button_relay2 = payload[i];
      }
    }
  }

  char aux_rfSerial[60] = {};
  sprintf(aux_rfSerial, "&%s", rfSerial);

  printf("\n\n jj nn mm\n\n");

  memset(userPayload, 0, sizeof(userPayload));
  // search user in memory
  if (MyUser_Search_User(userId, userPayload) == ESP_OK) {
    printf("\n\n jj nn mm33 - %s\n\n", userPayload);

    parse_ValidateData_User(userPayload, &user_rf_to_user);

    if (!strcmp(rfSerial, user_rf_to_user.rf_serial) ||
        user_rf_to_user.rf_serial[0] == ':') {

      if (user_rf_to_user.rf_serial[0] == ':') {
        memset(userPayload, 0, sizeof(userPayload));
        if (checkIf_rfSerialExist(aux_rfSerial, userPayload)) {
          printf("\n\n jj nn mm22 - %s\n\n", aux_rfSerial);
          return "ERROR";
        }
      }
      /*  if (user_rf_to_user.rf1_relay != button_number &&
           user_rf_to_user.rf2_relay != button_number &&
           user_rf_to_user.rf3_relay != button_number) { */

      user_rf_to_user.rf1_relay = button_relay1;
      user_rf_to_user.rf2_relay = button_relay2;
      sprintf(user_rf_to_user.rf_serial, "%s", rfSerial);
      /* else if (relayNumber == RF3_USER_POSITION) {
       user_rf_to_user.rf3_relay = button_number;
     } */
      printf("\n\n jj nn mm33 - %s\n\n", rfSerial);
      if (MyUser_add_rfSerial(aux_rfSerial, userId,
                              user_rf_to_user.permition) == ESP_OK) {

        replaceUser(&user_rf_to_user);
        memset(file_contents, 0, sizeof(file_contents));
        char aux_fileContents[200] = {};
        MyUser_Search_User(userId, aux_fileContents);
        erase_Password_For_Rsp(aux_fileContents, file_contents);
        printf("\n\n put rf str - %s\n\n", file_contents);
        return file_contents;
      } else {
        return "ERROR";
      }
    }
  }
  //}
  return "ERROR";
}/**
 * Deletes a user from the user database based on the user number.
 *
 * @param user_number The user number to be deleted.
 *
 * @returns The updated user database after deletion.
 */
char *delete_userRF(char *user_number) {
  char userPayload[200] = {};
  MyUser user_rf_to_user;

  if (MyUser_Search_User(user_number, userPayload) == ESP_OK) {
    memset(&user_rf_to_user, 0, sizeof(user_rf_to_user));
    parse_ValidateData_User(userPayload, &user_rf_to_user);

    char eraseRF[50] = {};
    sprintf(eraseRF, "&%s", user_rf_to_user.rf_serial);
    user_rf_to_user.rf1_relay = ':';
    user_rf_to_user.rf2_relay = ':';
    // user_rf_to_user.rf3_relay = ':';
    memset(user_rf_to_user.rf_serial, 0, sizeof(user_rf_to_user.rf_serial));
    user_rf_to_user.rf_serial[0] = ':';

    replaceUser(&user_rf_to_user);

    erase_onlyRF(eraseRF, user_rf_to_user.permition);
    memset(userPayload, 0, sizeof(userPayload));
    memset(file_contents, 0, sizeof(file_contents));
    MyUser_Search_User(user_number, userPayload);
    erase_Password_For_Rsp(userPayload, file_contents);
    return file_contents;
  }

  return "ERROR";
}

char *delete_buttonRF(char *user_number, uint8_t button_number) {
  char userPayload[200] = {};
  MyUser user_rf_to_user;

  if (MyUser_Search_User(user_number, userPayload)) {
    memset(&user_rf_to_user, 0, sizeof(user_rf_to_user));
    parse_ValidateData_User(user_number, &user_rf_to_user);

    if (button_number == 1) {
      user_rf_to_user.rf1_relay = ':';
    } else if (button_number == 2) {
      user_rf_to_user.rf2_relay = ':';
    } /* else if (button_number == 3) {
      user_rf_to_user.rf3_relay = ':';
    }  */
    else {
      return "ERROR";
    }

    replaceUser(&user_rf_to_user);

  } else {
    return "ERROR";
  }

  return "ERROR";
}

char *put_numberOn_rfUser(char *payload) {

  char userPayload[200] = {};
  char aux_userPayload[200] = {};
  char rfSerial[20] = {};
  char phNumber[20] = {};
  uint8_t count = 0;
  uint8_t strIndex = 0;
  MyUser user_rf_to_user;
  char aux_rfSerial[60] = {};

  for (int i = 0; i < strlen(payload); i++) {

    if (payload[i] == '.') {
      count++;
      strIndex = 0;
    } else if (count == 0) {
      rfSerial[strIndex] = payload[i];
      strIndex++;
    } else if (count == 1) {
      phNumber[strIndex] = payload[i];
      strIndex++;
    }
  }

  sprintf(aux_rfSerial, "%s", rfSerial);
  if (MyUser_Search_User(aux_rfSerial, userPayload) == ESP_OK) {

    if (MyUser_Search_User(phNumber, aux_userPayload) != ESP_OK) {

      memset(&user_rf_to_user, 0, sizeof(user_rf_to_user));
      parse_ValidateData_User(userPayload, &user_rf_to_user);
      if (Myuser_deleteUser(&user_rf_to_user) == ESP_OK) {

        strcpy(user_rf_to_user.phone, phNumber);
        if (MyUser_Add(&user_rf_to_user) == ESP_OK) {
          memset(userPayload, 0, sizeof(userPayload));
          // //printf("\n\n 6666wiegand -  / phone - %s\n\n",
          // wiegand_myUser_data.phone);
          if (MyUser_Search_User(user_rf_to_user.phone, userPayload) ==
              ESP_OK) {
            memset(file_contents, 0, sizeof(file_contents));
            erase_Password_For_Rsp(userPayload, file_contents);
            return file_contents;
          }
        } else {
          MyUser_Add(&user_rf_to_user);
          return "ERROR";
        }
      } else {
        return "ERROR";
      }
    }
  }
  return "ERROR";
}
///@brief erase rf key of the memory
///@param key
///@param permition
///@return uint8_t
uint8_t erase_onlyRF(char *key, char permition) {
  // //printf("\n\nerase_only_wiegand - %s - %c",wiegandNumber,permition);
  if (permition == '0') {

    if (nvs_erase_key(nvs_rf_codes_users_handle, key) == ESP_OK) {

      return ESP_OK;
    } else {
      return ESP_FAIL;
    }
  } else if (permition == '1') {

    if (nvs_erase_key(nvs_rf_codes_admin_handle, key) == ESP_OK) {
      return ESP_OK;
    } else {
      return ESP_FAIL;
    }
  } else if (permition == '2') {

    if (nvs_erase_key(nvs_rf_codes_owner_handle, key) == ESP_OK) {

      return ESP_OK;
    } else {
      return ESP_FAIL;
    }
  } else {
    return ESP_FAIL;
  }

  return ESP_FAIL;
}

/**
 * Adds a new user with RF data and relay information.
 *
 * @param rf_button The RF button pressed.
 * @param rf_serial The RF serial data.
 * @param relay The relay number.
 * @param userData Additional user data.
 *
 * @returns ESP_OK if the operation was successful.
 */
uint8_t add_RF_auto(char rf_button, char *rf_serial, uint8_t relay,
                    char *userData) {
  char userRF_data[50] = {};
  char *aux_rfSerial;
  MyUser userRF_struct;

  memset(&userRF_struct, 0, sizeof(userRF_struct));

  ///@brief
  asprintf(&aux_rfSerial, "&%s", rf_serial);
  // Put button in relay position

  // Check if rf serial exist, if not exist add a new rf user
  ///@brief Construct a new if object
  if (checkIf_rfSerialExist(aux_rfSerial, userRF_data)) {

    MyUser_Search_User(userRF_data, userData);
    parse_ValidateData_User(userData, &userRF_struct);
    ///@brief Construct a new if object
    if (relay == 1) {
      if (userRF_struct.rf1_relay != ':') {
        userRF_struct.rf1_relay = rf_button;
      } else {
        return ESP_FAIL;
      }
    } else if (relay == 2) {
      if (userRF_struct.rf2_relay != ':') {
        userRF_struct.rf2_relay = rf_button;
      } else {
        return ESP_FAIL;
      }
    } /* else if (relay == 3) {
      if (userRF_struct.rf3_relay != ':') {
        userRF_struct.rf3_relay = rf_button;
      } */
    else {
      return ESP_FAIL;
    }
  } else {
    return ESP_FAIL;
  }

  // memset(userData, 0, sizeof(userData));

  if (replaceUser(&userRF_struct) != ESP_OK) {

    printf("\n\n WIGAND  44 %s\n\n", userData);
    return ESP_FAIL;
  }
  /*  } else {

     add_default_RF_user(rf_serial, rf_button, relay);
   }

   // copy user data for
   sprintf(userData,
   "%s;%s;%s;%s;%s;%s;%s;%c;%s;%c;%c;%c;%c;%s;%s;%c;%c;%c",
           userRF_struct.phone, userRF_struct.firstName,
           userRF_struct.start.date, userRF_struct.start.hour,
           userRF_struct.end.days, userRF_struct.end.hour,
   userRF_struct.key, userRF_struct.permition, userRF_struct.week,
           userRF_struct.relayPermition, userRF_struct.ble_security,
           userRF_struct.erase_User_After_Date,
           userRF_struct.wiegand_rele_permition, userRF_struct.wiegand_code,
           userRF_struct.rf_serial, userRF_struct.rf1_relay,
           userRF_struct.rf2_relay , userRF_struct.rf3_relay ); */

  return ESP_OK;
}

/**
 * Adds a default RF user with the given serial, button, and relay
 * information.
 *
 * @param serial The serial number of the user.
 * @param button The button associated with the user.
 * @param relay The relay number associated with the user.
 *
 * @returns ESP_OK if the user is successfully added, ESP_FAIL otherwise.
 */
uint8_t add_default_RF_user(char *serial, char button, uint8_t relay) {

  MyUser user;

  if (get_RTC_System_Time() == 0) {
    return ESP_FAIL;
  }
  // ////printf("add default number 4\n");
  sprintf(user.firstName, "%s", "S/N");
  sprintf(user.phone, "%s", "");
  sprintf(user.start.date, "%d", nowTime.date);
  sprintf(user.start.hour, "%s", "0000");
  sprintf(user.end.days, "%c", '*');
  sprintf(user.end.hour, "%s", "2359");
  sprintf(user.key, "%s", "888888");
  sprintf(user.week, "%s", "1111111");
  user.permition = '0';
  user.relayPermition = '0';
  user.ble_security = '0';
  user.erase_User_After_Date = '0';
  user.wiegand_code[0] = ':';
  user.wiegand_code[1] = 0;
  user.wiegand_rele_permition = ':';
  sprintf(user.rf_serial, "%s", serial);

  if (relay == 1) {
    user.rf1_relay = button;
    user.rf2_relay = ':';
    // user.rf3_relay = ':';
  } else if (relay == 2) {
    user.rf1_relay = ':';
    user.rf2_relay = button;
    // user.rf3_relay = ':';
  } else if (relay == 3) {
    user.rf1_relay = ':';
    user.rf2_relay = ':';
    // user.rf3_relay = button;
  }

  MyUser_Add(&user);

  return ESP_OK;
}
void vTimer_autoSaveRF_callback(TimerHandle_t xTimer_auto) {
  // Fun o de callback do timer

  ESP_LOGI(TAG, "vTimer_autoSave_callback: timer deleted");

  // Salvar dados do Wiegand automaticamente
  xTimerStop(xTimer_auto, 10);
  ESP_LOGI(TAG, "vTimer_autoSave_callback: timer deleted successfully");
  rf_mode = RF_RELAY_MODE;
  ESP_LOGI(TAG, "vTimer_autoSave_callback: rf_mode set to RF_RELAY_MODE");
}
char *parseRF_data(uint8_t BLE_SMS_Indication, uint8_t gattsIF, uint16_t connID,
                   uint16_t handle_table, char cmd, char param, char *payload,
                   MyUser *user_validateData, mqtt_information *mqttInfo) {

  char *rsp = NULL;

  ESP_LOGI(TAG, "parseRF_data: cmd = %c, param = %c, payload = %s", cmd, param,
           payload);

  if (cmd == SET_CMD) {
    if (param == RF_SAVE_AUTO_PARAMETER) {

      ESP_LOGI(TAG, "parseRF_data: rf_autoSAve_relay = %d", atoi(payload));

      rf_autoSAve_relay = atoi(payload);

      // Create timer

      // Iniciar o timer
      ESP_LOGI(TAG, "parseRF_data: starting timer");
      if (xTimerStart(xTimer_autoadd_rf, 0) != pdPASS) {
        ESP_LOGE(TAG, "parseRF_data: error starting timer");

        asprintf(&rsp, "RF S S %s", "ERROR");
      } else {
        xTimerChangePeriod(xTimer_autoadd_rf, TIMER_AUTO_SAVE_PERIOD, 0);
        ESP_LOGI(TAG, "parseRF_data: timer started");
        asprintf(&rsp, "RF S S %s", "OK");

        rf_mode = RF_AUTOSAVE_MODE;
      }

      BLE_SMS_Indication_rf_autoSave = BLE_SMS_Indication;
      gattsIF_autoSave = gattsIF;
      connID_autoSave = connID;
      handle_table_autoSave = handle_table;

      return rsp;
    } else if (param == RF_SAVE_COMMAND_PARAMETER) {

      asprintf(&rsp, "RF S C %s", put_rf_to_user(payload));
      return rsp;
    }else if (param == RF_ROLLING_CODE_PARAMETER) { 

      asprintf(&rsp, "RF S R %d", activate_rollingCode());
      return rsp;    
    } 
    else if (param == RF_CELL_PHONE_NUMBER_PARAMETER) {

      asprintf(&rsp, "RF S P %s", put_numberOn_rfUser(payload));
      return rsp;
    }

  } else if (cmd == GET_CMD) {
    if (param == RF_GET_PARAMETER) {
      rf_mode = RF_GET_RF_MODE;
      if (xTimerStart(xTimer_autoadd_rf, 0) != pdPASS) {
        ESP_LOGE(TAG, "parseRF_data: error starting timer");

        asprintf(&rsp, "RF G F %s", "ERROR");
      } else {
        xTimerChangePeriod(xTimer_autoadd_rf, TIMER_AUTO_SAVE_PERIOD, 0);
        ESP_LOGI(TAG, "parseRF_data: timer started");
        BLE_SMS_Indication_rf_autoSave = BLE_SMS_Indication;
        gattsIF_autoSave = gattsIF;
        connID_autoSave = connID;
        handle_table_autoSave = handle_table;
        asprintf(&rsp, "RF G F %s", "OK");

        
      }

      return rsp;
    }
  } else if (cmd == RESET_CMD) {
    if (param == RF_SAVE_AUTO_PARAMETER) {

      xTimerStop(xTimer_autoadd_rf, 10);
      rf_mode = RF_RELAY_MODE;
      asprintf(&rsp, "RF R S %s", "OK");
      return rsp;
    }else if (param == RF_ROLLING_CODE_PARAMETER) { 

      asprintf(&rsp, "RF R R %d", deactivate_rollingCode());
      return rsp;    
    }else if (param == RF_SAVE_COMMAND_PARAMETER) {
      asprintf(&rsp, "RF R C %s", delete_userRF(payload));
      return rsp;
    }
  } else {
    ESP_LOGE(TAG, "parseRF_data: invalid command");
    asprintf(&rsp, "RF S S %s", "ERROR");
    return rsp;
  }
  asprintf(&rsp, "RF S S %s", "ERROR");
  ESP_LOGE(TAG, "parseRF_data: unknown error");
  return rsp;
}
