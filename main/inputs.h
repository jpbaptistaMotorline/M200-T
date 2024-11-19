/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|

*/

#ifndef _INPUTS_H_
#define _INPUTS_H_

#include <stdint.h>
#include <stdio.h>
#include "stdio.h"
#include "core.h"
#include "cmd_list.h"
#include "erro_list.h"
//#include "rele.c"

int read_Input1();
int read_Input2();
char *readInputs(uint8_t BLE_SMS_Indication, uint8_t inputNumber, char cmd, char param, char *phPassword, char *payload);

#endif