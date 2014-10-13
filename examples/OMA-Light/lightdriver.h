#include "AppHardwareApi.h"
#include "juart.h"
#include <stdlib.h>
#include <string.h>
#include "contiki.h"


// Set pin DIO13 to Output direction.
#define vSetDIO13_OutputDirection()    vAHI_DioSetDirection(0, E_AHI_DIO13_INT);

// Set pin DIO13 to bit 0.
#define vSetDIO13_0()    vAHI_DioSetOutput(0, E_AHI_DIO13_INT);

// Set pin DIO13 to bit 1.
#define vSetDIO13_1()    vAHI_DioSetOutput(E_AHI_DIO13_INT, 0);


void init_Light();

void set_on();

void set_off();

int get_light_status();

int switch_light();

int get_luminity();

int set_luminity(int value);

//void set_timer(int n);

void vTimerConfig(uint8_t duty);

