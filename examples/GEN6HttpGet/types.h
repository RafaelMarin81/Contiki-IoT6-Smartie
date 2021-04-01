#ifndef TYPES_H_
#define TYPES_H_

#include "contiki.h"    // Define types of variables:   uint8_t  &  uint16_t  &  uint32_t

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

#include "AppHardwareApi.h"


typedef uint8_t BYTE;   

typedef uint8_t BOOL;  

typedef uint16_t WORD;

typedef uint32_t DWORD;

// typedef int INT;

/*#define FALSE			0
#define TRUE			1*/
/*
// MAC ethernet
typedef struct {
    BYTE v[6];
} MAC_ADDR;

// IPv4
typedef union {
    BYTE v[4];
    
    DWORD Val;
} IP_ADDR;

#define PACKET_SWAP_REQUIRED

// permite saber si se usa o no la interrupción de RX de la USART SPI
#define USART_SPI_INT_RX_ENABLED
*/
#endif
