#include "AppHardwareApi.h"

// Set pin DIO13 to Output direction.
#define vSetDIO13_OutputDirection()    vAHI_DioSetDirection(0, E_AHI_DIO13_INT);

// Set pin DIO13 to bit 0.
#define vSetDIO13_0()    vAHI_DioSetOutput(0, E_AHI_DIO13_INT);

// Set pin DIO13 to bit 1.
#define vSetDIO13_1()    vAHI_DioSetOutput(E_AHI_DIO13_INT, 0);


void init_Light();

void set_on();

void set_off();


