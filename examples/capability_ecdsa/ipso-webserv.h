#include <stdlib.h>
#include <string.h>

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "juart.h"
#include "coap.h"
#include "capability_server.h"

int parse_ipso(coap_packet_t *request, char *response, int *writePos);
int parse_capability (coap_packet_t * request, capability_token_t *ct);

