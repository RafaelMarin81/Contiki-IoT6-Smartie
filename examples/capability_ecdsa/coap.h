#include <stdlib.h>
#include <string.h>

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "juart.h"

#ifndef __COAP_H
  #define __COAP_H

#define MOTE_SERVER_LISTEN_PORT 1234

#define COAP_VERSION 0x40 //Ver = 1

typedef enum {
	created = 65,
	deleted = 66,
	valid = 67,
	changed = 68,
	content = 69,
	badrequest = 128,
	unauthorized = 129,
	badoption = 130,
	notallowed = 133,
	preconditionfailed = 140,
	notimplemented = 161
} response_code;

typedef enum {
  MESSAGE_TYPE_CON = 0x00,
  MESSAGE_TYPE_NON = 0x10,
  MESSAGE_TYPE_ACK = 0x20,
  MESSAGE_TYPE_RST = 0x30
} message_type;

typedef enum {
	GET = 1,
	POST = 2,
	PUT = 3,
	DELETE = 4
} request_code_t;

typedef enum {
  OK_200 = 80,
  CREATED_201 = 81,
  NOT_MODIFIED_304 = 124,
  BAD_REQUEST_400 = 160,
  UNAUTHORIZED_401 = 161,
  NOT_FOUND_404 = 164,
  METHOD_NOT_ALLOWED_405 = 165,
  UNSUPPORTED_MADIA_TYPE_415 = 175,
  INTERNAL_SERVER_ERROR_500 = 200,
  BAD_GATEWAY_502 = 202,
  GATEWAY_TIMEOUT_504 = 204
} status_code_t;

typedef enum {
   Option_Type_If_Match= 1,
   Option_Type_Uri_Host = 3,
   Option_Type_Etag = 4,
   Option_Type_If_None_Match = 5,
   Option_Type_Uri_Port = 7,
   Option_Type_Location_Path = 8,
   Option_Type_Uri_Path = 176,
   Option_Type_Content_Format = 12,
   Option_Type_Max_Age = 14,
   Option_Type_Uri_Query = 15,
   Option_Type_Accept = 17,
   Option_Type_Location_Query = 20,
   Option_Type_Proxy_Query = 35,
   Option_Type_Proxy_Scheme = 39,
   Option_Type_Sig = 32
} option_type;

struct OPTION_T
{
	uint8_t header;
	char *option_value;
}__attribute__ ((__packed__));
typedef struct OPTION_T option_t;

struct COAP_PACKET_T
{
	uint8_t header;
	uint8_t code;
	uint16_t id;
	uint8_t option_header_uri;
	char *option_value_uri;
	uint8_t option_header_sig;
	char *option_value_sig;
	char *payload;

}__attribute__ ((__packed__));
typedef struct COAP_PACKET_T coap_packet_t;

int parse_coap_packet(coap_packet_t *packet, char *buf, int *need_confirm);
void create_coap_response(coap_packet_t *packet, message_type type, status_code_t code, char *payload, uint16_t lastId);
int copy_headers(unsigned char *buf, coap_packet_t *packet);
#endif
