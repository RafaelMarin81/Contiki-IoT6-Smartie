/*
 ============================================================================
 Name        : Base64EncodeDecode.c
 Author      : Sam Ernest Kumar
 Version     :
 Copyright   :
 Description : Base64EncoderDecoder in C, Ansi-style
 ============================================================================
 */

#include <stdlib.h>
#include <string.h>

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "juart.h"

#define TABLELEN        63
//#define BUFFFERLEN      128

#define ENCODERLEN      4
#define ENCODEROPLEN    0
#define ENCODERBLOCKLEN 3

#define PADDINGCHAR     '='
#define BASE64CHARSET   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


int Base64Decode(char *input, uint8 *output, int oplen);
int dectohex (uint8 *input, uint32 *output);
//static int decodeblock(char *input, unsigned char *output, int oplen);
