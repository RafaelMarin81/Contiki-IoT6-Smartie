#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

/****************************************************************************
 *
 * MODULE:             Logger header
 *
 * COMPONENT:          $RCSfile: logger.h,v $
 *
 * DATED:              $Date: 2011/05/25 17:46:00 $
 *
 * AUTHOR:             Rafael Marin-Perez
 *
 * DESCRIPTION:        Logger use header file
 *
 ****/

// Data Structure.
typedef struct datalog {
    uint8_t  state;
    uint8_t  power;
    uint8_t  tam_paq;
    uint8_t  num_sec;
    uint16_t num_paq;
} datalog;

//u8PowerLevel:       0,   1,   2,   3,  4, 5    
//Radio-power(dBm): –30, –24, –18, –12, –6, 0
// mapping  {0=>-30dBm, 1=>-24dBm, 2=>-18dBm, 3=>-12dBm, 4=>-6dBm, 5=>0dBm}
#define MAX_TX_POWERS 4 //6

//#define NUM_TAM 8
//const int TAM[NUM_TAM] = { 10, 25, 40, 55, 70, 85, 100, 115};

#define MAX_PAQUETES   50
#define MAX_SECUENCIA  5 //10


#define STATE_STARTING 1
#define STATE_DATA     2
#define STATE_STOPING  3
#define STATE_FINISHED 4

#endif // LOGGER_H_INCLUDED
