/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|

*/
#ifndef _UDP_CODES_H
#define _UDP_CODES_H

#define IMEI_ICCID_CODE 101

#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"

/* char list_Codes_Codec[101][7] = {
    "R1.G.B",
    "R1.G.M",
    "R1.G.R",
    "R1.G.T",
    "R1.S.B",
    "R1.S.M",
    "R1.S.R",
    "R1.S.T",
    "R1.R.B",
    "R1.R.M",
    "R1.R.R",
    "R2.G.M",
    "R2.G.R",
    "R2.G.T",
    "R2.S.M",
    "R2.S.R",
    "R2.S.T",
    "R2.R.M",
    "R2.R.R",
    "I1.G.I",
    "I2.G.I",
    "UR.G.*",
    "UR.G.D",
    "UR.G.H",
    "UR.G.L",
    "UR.G.R",
    "UR.G.U",
    "UR.G.W",
    "UR.S.A",
    "UR.S.D",
    "UR.S.H",
    "UR.S.L",
    "UR.S.N",
    "UR.S.R",
    "UR.S.U",
    "UR.S.W",
    "UR.R.*",
    "UR.R.A",
    "UR.R.B",
    "UR.R.L",
    "UR.R.U",
    "ME.G.B",
    "ME.G.C",
    "ME.G.F",
    "ME.G.H",
    "ME.G.I",
    "ME.G.L",
    "ME.G.M",
    "ME.G.Q",
    "ME.G.P",
    "ME.G.S",
    "ME.S.A",
    "ME.S.C",
    "ME.S.D",
    "ME.S.F",
    "ME.S.N",
    "ME.S.O",
    "ME.S.S",
    "ME.S.T",
    "ME.S.U",
    "ME.R.A",
    "ME.R.F",
    "ME.R.K",
    "ME.R.M",
    "ME.R.O",
    "ME.R.P",
    "ME.R.S",
    "ME.R.U",
    "AL.G.*",
    "AL.S.A",
    "AL.S.C",
    "AL.S.I",
    "AL.S.O",
    "AL.S.T",
    "AL.R.A",
    "AL.R.C",
    "AL.R.O",
    "F1.S.P",
    "F1.R.P",
    "F2.S.P",
    "F2.R.P",
    "F3.S.P",
    "F3.R.P",
    "F4.S.P",
    "F4.R.P",
    "F5.S.P",
    "F5.R.P",
    "F6.S.P",
    "F6.R.P",
    "FX.S.M",
    "FX.R.M",
    "RT.G.D",
    "RT.G.R",
    "RT.G.T",
    "RT.S.A",
    "RT.S.D",
    "RT.S.R",
    "RT.S.T",
    "RT.R.D",
    "RT.R.R",
    "RT.R.T"

}; */

/* typedef struct UDP_Codes
{
    uint8_t code;

}udp_Codes; */





uint8_t *parse_INT_To_STR(char* data,uint8_t size,char* output);



#endif // !UDP_CODES_H

