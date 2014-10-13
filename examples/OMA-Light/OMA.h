
/*
 * Author:  Pablo Lopez Martinez, David Fernandez Ros
 * email: p.lopezmartinez@um.es, david.f.r@um.es
 * 
 * Last Update: 30/5/2013
*/
#ifndef __OMA_H
#define __OMA_H

#include <stdlib.h>
#include <string.h>

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "juart.h"
#include "coap.h"



// Main function
int parse_OMA(coap_packet_t *request, char *response, int *writePos);

void send_OMA_RD_announcement(char *buf);


#endif
