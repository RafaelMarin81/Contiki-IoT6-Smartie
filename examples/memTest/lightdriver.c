#include "lightdriver.h"


void init_Light(){
  vSetDIO13_OutputDirection();
}

void set_on(){
  vSetDIO13_1();
}

void set_off(){
  vSetDIO13_0();
}

