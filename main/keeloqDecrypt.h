/* 
 * File:   keeloqDecrypt.h
 * Author: cpereira
 *
 * Created on 3 de Outubro de 2014, 11:52
 */
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"

#ifndef KEELOQDECRYPT_H
#define	KEELOQDECRYPT_H
/******************************************************************************/
#ifdef	__cplusplus
extern "C" {
#endif
/******************************************************************************/

/******************************************************************************/
#define  SIZE_ListSyncValues 25
/** Decryption key for the KLQ algorithm. */
extern uint8_t DKEY[8];

typedef enum
{
	Keeloq_RollingCode,
	Keeloq_NoRollingCode,
	DIP_12BIT
}TypeRemote_st;

typedef enum
{
	NoCMD                   = 0,
	TypeFull                = 1,
	TypeWalk                = 2,
	TypeFullOpen            = 3,
	TypeFullClose           = 4,
	TypeFullStop            = 5,
	TypeFollowMe            = 6,
	TypeProgrammingModeWalk = 7,
	TypeProgrammingModeFull = 8,
	TypePush                = 9,
	TypeContinue            = 10,
	TypeSecurity			= 11,
}TypeCMD;


typedef enum  {
    NO_DATA,    ///< No Transmission Received
    VALID_PACKET, /**< Valid Packet Received*/
    NOT_VALID_PACKET,
    NOT_LEARNED,
    SEED_RECEIVED,
    NOT_SEED_PACKET,
    LEARN_SUCCESS,
    NO_SPACE,
    DECRYPT_FAIL,
    RESYNC_REQ,
    HOP_CHECK_FAIL,
    NVM_WRITE_ERROR,
    MAC_ERROR,
    TIMER_OUT_OF_SYNC,
    ENCODER_TIMER_RESYNC,
    TX_LOW_VOLTAGE,
    CNT_OUT_OF_SYNC
    //Add more Errors here
}  result_code_t;

typedef struct  {
    uint32_t raw_data;
    uint32_t serialnumber;
    uint16_t sync;
    uint16_t disc;
    uint8_t fcode1;
    uint8_t fcode2;
    TypeRemote_st type;
    uint8_t positionMem;
}  classic_encoder_t;

typedef struct  {
    uint32_t serial;
    uint16_t sync;
}  classic_nvm_t;



/**
 * KLQ decryption algorithm.
 * \pre DKEY must contain the encryption key and Buffer must contain the data
 */
void Decrypt(uint8_t* packet);
void LoadManufCode(void);
void NormalKeyGen(uint32_t* serial);
void SecureKeyGen(uint8_t* seed);
bool DecCHK(classic_encoder_t* data);
result_code_t ReqResync(uint16_t CurrentHop);
result_code_t HopCHK(classic_encoder_t* data);

void* getClassicManufCode(void);
void setClassicManufCode(const uint8_t* data);
TypeCMD keeloqDecryptPacket(uint8_t* data,classic_encoder_t* classic_data);
void keeloq_classic_decrypt_packet(classic_encoder_t* data);
uint16_t getHopCode(classic_encoder_t* data);

TypeCMD verifyProgramingMode(classic_encoder_t* classic_data);

TypeCMD DecryptPacket_12BIT(uint8_t* data,classic_encoder_t* classic_data);

#ifdef	__cplusplus
}
#endif

#endif	/* KEELOQDECRYPT_H */