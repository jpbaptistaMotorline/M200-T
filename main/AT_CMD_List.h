/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______ 
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__   
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|  
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____ 
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|
                                                                                                                          
*/

#ifndef _AT_CMD_LIST_H_
#define _AT_CMD_LIST_H_

/*******************************************************************
*                      General Commands
*******************************************************************/

#define AT      "AT"        // AT COMMAND 
#define ATI     "ATI"       // DISPLAY PRODUCT ID INFORMATION
#define AT_GSN  "AT+GSN"    // IMEI READER COMMAND
#define AT_CGSN "AT+CGSN"   // IMEI READER PRODUCT SERIAL NUMBER
#define A_      "A/"        // REPEAT PREVIOUS COMMAND
#define AT_CFUN "AT+CFUN"   // SET PHONE FUNCTIONALITY
#define AT_CMEE "AT+CMEE"   // ERROR MESSAGE FORMAT 

/*******************************************************************
*                    (U)SIM Related Commands
*******************************************************************/

#define AT_CPIN "AT+CPIN"   // PIN COMMAND

/*******************************************************************
*                     Network Service Command
*******************************************************************/

#define AT_COPS "AT+COPS"   // OPERATOR SELECTION
#define AT_CREG "AT+CREG"   // NETWORK REGISTRATION  
#define AT_CSQ  "AT+CSQ"    // SIGNAL QUALITY REPORT  
#define AT_CTZU "AT+CTZU"   // AUTOMATIC TIME ZONE UPDATE


/*******************************************************************
*                     Call Related Commands
*******************************************************************/

#define ATA     "ATA"       // ANSWER AN INCOMING CALL
#define ATD     "ATD"       // CALL NUMBER
#define ATH     "ATH"       // DISCONNECT EXISTING CONNECTION 
#define AT_CVHU "AT+CVHU"   // VOICE HANG UP CONTROL
#define AT_CHUP "AT+CHUP"   // HANG UP CALL

/*******************************************************************
*                         SMS Commands
*******************************************************************/

#define AT_CSMS "AT+CSMS"   // SELECT MESSAGE SERVICE
#define AT_CMGD "AT+CMGD"   // DELETE MESSAGE 
#define AT_CMGL "AT+CMGL"   // LIST MESSAGES
#define AT_CMGR "AT+CMGR"   // READ MESSAGE 
#define AT_CMGS "AT+CMGS"   // SEND MESSAGE

/*******************************************************************
*                    Hardware Related Commands
*******************************************************************/

#define AT_QPOWD "AT+QPOWD" // POWER OFF 
#define AT_CCLK  "AT+CCLK"  // CLOCK 

#endif