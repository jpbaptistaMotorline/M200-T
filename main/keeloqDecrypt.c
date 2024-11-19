/*
 * � 2014 Microchip Technology Inc. and its subsidiaries.  You may use
 * this software and any derivatives exclusively with Microchip
 * products.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,
 * MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE, OR ITS
 * INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION WITH ANY OTHER
 * PRODUCTS, OR USE IN ANY APPLICATION.
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL,
 * PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE
 * OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN
 * IF MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
 * FORESEEABLE.  TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S
 * TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE
 * WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID
 * DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE
 * OF THESE TERMS.
 */

#include "keeloqDecrypt.h"
#include "stdio.h"
#include "string.h"

#define BIT_TEST(x, y) (((x) & (1 << (y))) != 0)

// extern volatile varSystem_NVM var_sys_NVM;

/** Resync required */
bool F2Chance;

/** resync value for 2 Chance */
static uint16_t NextHop;
static uint16_t Hop;
static uint16_t EHop;
uint8_t DKEY[8];
volatile classic_nvm_t listSyncValues[SIZE_ListSyncValues];
volatile uint16_t pointSync = 0;

void *getClassicManufCode(void) {
  LoadManufCode();
  return DKEY;
}

void setClassicManufCode(const uint8_t *data) { memcpy(DKEY, data, 8); }

void LoadManufCode(void) {
  DKEY[0] = 0xFF; // DKEY= 572768229476CAFF
  DKEY[1] = 0xCA;
  DKEY[2] = 0x76;
  DKEY[3] = 0x94;
  DKEY[4] = 0x22;
  DKEY[5] = 0x68;
  DKEY[6] = 0x27;
  DKEY[7] = 0x57;
}

//----------------------------------------------------------------------
//
// Key Generation routine
//
// Normal Learn algorithm
//
// INPUT:  Serial Number (Buffer[4..7])
//         Manufacturer code
// OUTPUT: DKEY[0..7] computed decryption key
//
//----------------------------------------------------------------------

void NormalKeyGen(uint32_t *serial) {
  static uint8_t SEED[4];
  uint8_t Buffer[8] = {0};
  uint8_t HOPtemp[4];  // HOP temp buffer
  uint8_t SKEYtemp[4]; // Temp decryption key

  memcpy(&Buffer[4], serial, 4);

  memcpy(HOPtemp, Buffer, 4);

  memcpy(SEED, &Buffer[4], 4); // make seed = Serial Number
  SEED[3] &= 0x0f;             // mask out function codes

  // compute LSb of decryption key first
  memcpy(Buffer, SEED, 4); // get SEED in
  Buffer[3] |= 0x20;       // add constant 0x20,
  LoadManufCode();
  Decrypt(Buffer);
  memcpy(SKEYtemp, Buffer, 4); // save result for later

  // compute MSb of decryption key
  memcpy(Buffer, SEED, 4); // get SEED in
  Buffer[3] |= 0x60;       // add constant 0x60,
  LoadManufCode();
  Decrypt(Buffer);
  memcpy(&DKEY[4], Buffer, 4); // move it into DKEY MSb
  memcpy(DKEY, SKEYtemp, 4);   // add LSb

  // ready for Decrypt
  memcpy(Buffer, HOPtemp, 4); // restore hopping code
}

void SecureKeyGen(uint8_t *seed) {
  uint8_t Buffer[8];

  uint8_t HOPtemp[4];  // HOP temp buffer
  uint8_t SKEYtemp[8]; // Temp decryption key

  memcpy(Buffer, seed, 8);

  // compute LSb of decryption key first
  memcpy(HOPtemp, Buffer, 4); // Copy LSB of seed
  LoadManufCode();
  Decrypt(HOPtemp);
  memcpy(SKEYtemp, HOPtemp, 4); // save result for later

  // compute MSb of decryption key
  memcpy(HOPtemp, &Buffer[4], 4); // Copy MSB of seed
  LoadManufCode();
  Decrypt(HOPtemp);
  memcpy(&SKEYtemp[4], HOPtemp, 4); // save result in Upper 4 bytes
  memcpy(DKEY, SKEYtemp, 8);        // Move final result into DKEY
}

void Decrypt(uint8_t *packet) {
  uint_fast8_t i, j;
  uint8_t key, aux;

  int8_t p;
  uint8_t Buffer[4];

  memcpy(Buffer, packet, 4);

  p = 1;

  for (j = 66; j > 0; j--) {
    key = DKEY[p--];
    if (p < 0)
      p += 8;
    for (i = 8; i > 0; i--) {
      // NLF
      if (BIT_TEST(Buffer[3], 6)) {
        if (!BIT_TEST(Buffer[3], 1))
          aux = 0b00111010; // 10
        else
          aux = 0b01011100; // 11
      } else {
        if (!BIT_TEST(Buffer[3], 1))
          aux = 0b01110100; // 00
        else
          aux = 0b00101110; // 01
      }

      // move bit in position 7
      if (BIT_TEST(Buffer[2], 3))
        aux = (aux << 4) | (aux >> 4);
      if (BIT_TEST(Buffer[1], 0))
        aux <<= 2;
      if (BIT_TEST(Buffer[0], 0))
        aux <<= 1;

      // xor with Buffer and Dkey
      aux ^= Buffer[1] ^ Buffer[3] ^ key;

      Buffer[3] = (Buffer[3] << 1) | (Buffer[2] >> 7);
      Buffer[2] = (Buffer[2] << 1) | (Buffer[1] >> 7);
      Buffer[1] = (Buffer[1] << 1) | (Buffer[0] >> 7);
      Buffer[0] = (Buffer[0] << 1) | (aux >> 7);

      // rotate Dkey
      key <<= 1;
    } // for i
  } // for j

  memcpy(packet, Buffer, 4);
} // decrypt

//----------------------------------------------------------------------
//
// Valid Decryption Check
//
// INPUT:  Serial Number (Buffer[4..7])
//         Hopping Code  (Buffer[0..3])
// OUTPUT: TRUE if discrimination bits == lsb Serial Number
//              and decrypted function code == plain text function code
//----------------------------------------------------------------------

bool DecCHK(classic_encoder_t *data) {
  // verify function code

  //    if (((uint16_t)(data->disc & 0x3FF))!= ((uint16_t) (data->serialnumber &
  //    0x3FF))) // compare low 8bit of Serial Number

  //        return 0;

  volatile uint16_t ser, dsc;

  data->fcode2 = (data->disc >> 12) & 0x0F; // Coloca o valor da função(Botão)) no fcode2

  ser = data->serialnumber >> 24;

  dsc = data->disc >> 8;
    //printf("\n\nDecrypted data: 3 - ser = %d | dsc = %d\n\n", ser,dsc);
  if (ser != dsc) {
    return false;
  }
    
  //printf("\n\n f1 - %x | f2 - %x\n\n", data->fcode1, data->fcode2);
  if (((data->fcode1 ^ data->fcode2) & 0x0f) != 0)
    return false;

  return true;
}

//----------------------------------------------------------------------
//
// Hopping Code Verification
//
// INPUT:  Hopping Code  (Buffer[0..3])
//         and previous value stored in EEPROM EHop
// OUTPUT: TRUE if hopping code is incrementing and inside a safe window (16)
//
//----------------------------------------------------------------------

result_code_t ReqResync(uint16_t CurrentHop) {
  F2Chance = true;          // flag that a second (sequential) transmission
  NextHop = CurrentHop + 1; // is needed to resynchronize receiver
                            //    EUSART_Write_String((char*)"\r\n RESYNC_REQ");
  return RESYNC_REQ;        // cannot accept for now
}

//----------------------------------------------------------------------
// HopCHK()
//  Verify hopping code is within the acceptable window
//
//----------------------------------------------------------------------
result_code_t HopCHK(classic_encoder_t *data) {
  int16_t hopDiff;
  Hop = data->sync;
  EHop = getHopCode(data);

  if (F2Chance)
    if (NextHop == Hop) {
      F2Chance = false; // resync success
      //            EUSART_Write_String((char*)"\r\n VALID_PACKET RESYNC");
      return VALID_PACKET;
    }

  // main comparison
  hopDiff = Hop - EHop; // subtract last value from new one
  //printf("\n\n hopdiff - %d\n\n", hopDiff);
  if (hopDiff < 0) { // locked region
    // Hop Check Fail
    //        EUSART_Write_String((char*)"\r\n HOP_CHECK_FAIL");
    return HOP_CHECK_FAIL; // fail
  } else if (hopDiff > 16) // resync region
    // Resync Required
    return ReqResync(Hop);

  else // 0>= EHop >16 ; open window
  {
    // Pass
    return VALID_PACKET;
  }
}

uint16_t getHopCode(classic_encoder_t *data) {
  uint8_t i = 0;
  uint16_t tempHop = 0;
  int8_t tempPos = -1;
  for (i = 0; i < SIZE_ListSyncValues; i++) {
    if (listSyncValues[i].serial == data->serialnumber) {
      tempHop = listSyncValues[i].sync;
      tempPos = i;
      //            EUSART_Write_String((char*)"Serial");
    }
  }

  if (tempPos < 0) {
    listSyncValues[pointSync].serial = data->serialnumber;
    listSyncValues[pointSync].sync = data->sync;
    pointSync++;
    if (pointSync >= SIZE_ListSyncValues) {
      pointSync = 0;
    }
    return ReqResync(Hop);
  } else {
    listSyncValues[tempPos].sync = Hop;
  }
  // N�o tem nenhum sync guardado para este despositivo.
  return tempHop;
}

TypeCMD keeloqDecryptPacket(uint8_t *data, classic_encoder_t *classic_data) {
  result_code_t hop_error;
  TypeCMD commandType;
  memcpy(classic_data, data, 8);
  memcpy(&classic_data->sync, &classic_data->raw_data,
         4); // Copies raw data into sync and disc for decryption
  classic_data->fcode1 = *((uint8_t *)(&classic_data->serialnumber) + 3) >> 4;
  // commandType=verifyProgramingMode(classic_data);//Confima se as teclas
  // pressionadas sao de entrada em programa��o

  // if(commandType==NoCMD)
  // {
  //     commandType=validateRemoteSerialNumber(classic_data->serialnumber,NO,&classic_data->positionMem);
  // }
  // else if(var_sys_NVM.programmingDistance==0)//Caso sejam a combina��o de
  // programa��o mas a program��o a distancia est� desactivada limpa o comando.
  // {
  //     commandType=NoCMD;
  // }
  classic_data->serialnumber = classic_data->serialnumber & 0x0FFFFFFF;

  // TODO: IMPLEMENTAR IR VER SE EXISTE SERIAL NA MEMORIA. SE EXISTIR CONTINUA E
  // FAZ O DECRYP E VERIFICA O ROULINGCODE

  keeloq_classic_decrypt_packet(classic_data);
    printf("\n\nDecrypted data: 1\n\n");
    if (DecCHK(classic_data) == false)
    {
        /* if(var_sys_NVM.OnlyRollingCode==YES) {
            classic_data->type=Keeloq_NoRollingCode;
            //EUSART_Write_String((char*)"\r\n decryptFail");
            return NoCMD;//DECRYPT_FAIL;
        }
        else
        { */
            classic_data->type=Keeloq_NoRollingCode;
           // return commandType;//
       // }
    }
    else {
         classic_data->type = Keeloq_RollingCode;
    }

  // hop_error = /Verify Hop Code
  hop_error = HopCHK(classic_data);
  if (hop_error != VALID_PACKET) {

    return NoCMD; // hop_error;
  }
 
  return 1; // commandType;//

  // Execute Task Associate of command.
}

TypeCMD DecryptPacket_12BIT(uint8_t *data, classic_encoder_t *classic_data) {
  TypeCMD commandType;
  memcpy(&classic_data->serialnumber, data, 4);
  // commandType=validateRemoteSerialNumber(classic_data->serialnumber,NO,&classic_data->positionMem);
  classic_data->fcode1 = 0;
  classic_data->sync = 0;
  classic_data->type = DIP_12BIT;
  return 0; // commandType;
}

void keeloq_classic_decrypt_packet(classic_encoder_t *data) {

  NormalKeyGen(
      &data->serialnumber); // Compute the decryption key using serial number

  Decrypt((uint8_t *)&data->sync); // Decrypt the hopping code portion

  // Populate Struct for Validation
  data->fcode2 = (data->disc >> 12) & 0x0F;
}

TypeCMD verifyProgramingMode(classic_encoder_t *classic_data) {
  if (classic_data->fcode1 ==
      0x0A) // Tecla 2 e 4 no caso do comando MXS4 as duas teclas grandes.
  {
    return TypeProgrammingModeFull;
  } else if (classic_data->fcode1 == 0x05) // Tecla 1 e 3 no caso do comando
                                           // MXS4SP as duas teclas pequenas.
  {
    return TypeProgrammingModeWalk;
  } else {
    return NoCMD;
  }
}