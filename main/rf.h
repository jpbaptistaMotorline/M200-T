/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|

*/

#ifndef _RF_H_
#define _RF_H_

#include "cmd_list.h"
#include "core.h"
#include "erro_list.h"
#include "stdio.h"
#include <stdint.h>
#include <stdio.h>


#define TIMER_AUTO_SAVE_PERIOD pdMS_TO_TICKS(20000) // 1000 ms (1 segundo)
//#include "rele.c"

#define RF1_USER_POSITION 1
#define RF2_USER_POSITION 2
#define RF3_USER_POSITION 3

#define NBIT 65       // number of bit to receive -1
#define NBIT_12BIT 13 // number of bit to receive -1
#define TRFreset 0
#define TRFSYNC 1
#define TRFUNO 2
#define TRFZERO 3
#define TRFUNO_12BIT 4
#define TRFZERO_12BIT 5

#define HIGH_TO -10   // longest high Te
#define LOW_TO 10     // longest low Te
#define SHORT_HEAD 20 // shortest Thead accepted 2,7ms
#define LONG_HEAD 45  // longest Thead accepted 6,2ms

#define LONG_HEAD_12BIT 95  // longest Thead accepted 13,0ms
#define SHORT_HEAD_12BIT 80 // longest Thead accepted 11,0ms

//#endif

// Configurações da onda quadrada
#define PERIOD_US                                                              \
  138 // Período da onda quadrada em microssegundos (1 segundo neste caso)
#define DUTY_CYCLE_US                                                          \
  500000 // Ciclo de trabalho em microssegundos (50% de duty cycle neste caso)

TimerHandle_t xTimer_autoadd_rf;
/* #define RF_RELAY_MODE 1
#define RF_AUTOSAVE_MODE 2 */
/**
 * \typedef    	KEYS_STRUCT
 *
 * \brief    Estrutura com timer para validação da tecla premida.
 */

/**
 * \typedef    	KEYS_STRUCT
 *
 * \brief    Estrutura com timer para validação da tecla premida.
 */

typedef struct {
  unsigned char currentType;
  unsigned char currentKey;
  unsigned long currentSerial;
  unsigned int currentRolling;
  unsigned char currentPosMem;
  // unsigned long timer;                // Pressed time for the current RF key.
  // DM: At the current main timeslot (50ms) it is sufficient for 12 seconds.
  unsigned char
      timer; // Pressed time for the current RF key. DM: At the current main
             // timeslot (50ms) it is sufficient for 12 seconds.
  unsigned char
      processed; // Indicates if the key has been processed by the code.
} RF_KEY_STRUCT;

enum RF_MODE{
  RF_RELAY_MODE,
  RF_AUTOSAVE_MODE,
  RF_SAVE_MODE,
  RF_BUTTONSAVE_MODE,
  RF_GET_RF_MODE
};

  enum RF_MODE rf_mode;
/******  ab476c6d-32aa-4445-8152-b8ce30910313  *******/
void InitReceiver(void);signed char getKeyPressed(void);
signed char getSerialCmd(unsigned long *serial);
void timer_callback(void *arg);

void ClearFlag_rfCMD(void);

void taskMipot_rf(void *pvParameter);

/**
 * @brief Adds a new user with RF data and relay information.
 *
 * @param rf_button The button to be associated with the user.
 * @param rf_serial The serial number of the remote control.
 * @param relay The relay number associated with the button.
 * @param userData Additional user data.
 *
 * @returns ESP_OK if the user is successfully added, ESP_FAIL otherwise.
 */
uint8_t add_RF_auto(char rf_button, char *rf_serial, uint8_t relay,
                    char *userData);

/**
 * @brief Parses the received RF data.
 *
 * This function is responsible for parsing the received RF data and
 * performing the corresponding action. The function takes the command
 * and parameter from the received data, the phone password, the payload,
 * the user structure and the MQTT information structure as parameters.
 *
 * @param cmd The command received.
 * @param param The parameter received.
 * @param phPassword The phone password.
 * @param payload The payload received.
 * @param user_validateData The user structure.
 * @param mqttInfo The MQTT information structure.
 *
 * @return A pointer to a string containing the response to the received
 * data.
 */
char *parseRF_data(uint8_t BLE_SMS_Indication,uint8_t gattsIF, uint16_t connID, uint16_t handle_table,char cmd, char param, char *payload,MyUser *user_validateData, mqtt_information *mqttInfo);

void vTimer_autoSaveRF_callback(TimerHandle_t xTimer_auto);
void rf_relayMode(uint64_t serial, char button);

uint8_t add_default_RF_user(char *serial, char button, uint8_t relay);
/**
 * \fn    		void  readbutton ( void  )
 * \return   	void
 * \param		void
 * \brief    	Função de leitura das teclas e calculo de timer de pressão da
 *tecla. Neste caso só pode estar uma tecla pressiona, quando está duas
 *pressionadas é considerado outra tecla.
 */

void readRFButtons(void);

uint8_t erase_onlyRF(char *key, char permition);
void rxi(void);
char *put_rf_to_user(char *payload);
char *put_button_on_rfUser(char button,uint64_t serial, uint8_t relay);
char *delete_userRF(char *user_number);
char *delete_buttonRF(char *user_number, uint8_t button_number);
char *put_numberOn_rfUser(char *payload);
uint8_t activate_rollingCode();
uint8_t deactivate_rollingCode();

#endif