#include <stdlib.h>
#include <string.h>

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "juart.h"

/*
	Author: Pablo López Martínez
	email: p.lopezmartinez@um.es
*/



#define MOTE_SERVER_LISTEN_PORT 1234

#define COAP_VERSION 0x40 //Ver = 1

/*
 Version (Ver):  2-bit unsigned integer.  Indicates the CoAP version
      number.  Implementations of this specification MUST set this field
      to 1.  Other values are reserved for future versions.

   Type (T):  2-bit unsigned integer.  Indicates if this message is of
      type Confirmable (0), Non-Confirmable (1), Acknowledgement (2) or
      Reset (3).  See Section 4 for the semantics of these message
      types.

   Option Count (OC):  4-bit unsigned integer.  Indicates the number of
      options after the header (0-14).  If set to 0, there are no
      options and the payload (if any) immediately follows the header.
      If set to 15, then an end-of-options marker is used to indicate
      the end of options and the start of the payload.  The format of
      options is defined below.

   Code:  8-bit unsigned integer.  Indicates if the message carries a
      request (1-31) or a response (64-191), or is empty (0).  (All
      other code values are reserved.)  In case of a request, the Code
      field indicates the Request Method; in case of a response a
      Response Code.  Possible values are maintained in the CoAP Code
      Registry (Section 12.1).  See Section 5 for the semantics of
      requests and responses.

   Message ID:  16-bit unsigned integer in network byte order.  Used for
      the detection of message duplication, and to match messages of
      type Acknowledgement/Reset and messages of type Confirmable/
      Non-confirmable.  See Section 4 for Message ID generation rules
      and how messages are matched.
*/
/*
 0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Ver| T |  OC   |      Code     |          Message ID           |   HEADER
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Options (if any) ...											TODO:
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Payload (if any) ...											PAYLOAD
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
*/


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
  NOT_FOUND_404 = 164,
  METHOD_NOT_ALLOWED_405 = 165,
  UNSUPPORTED_MADIA_TYPE_415 = 175,
  INTERNAL_SERVER_ERROR_500 = 200,
  BAD_GATEWAY_502 = 202,
  GATEWAY_TIMEOUT_504 = 204
} status_code_t;

typedef enum {
  Option_Type_Content_Type = 1,
  Option_Type_Max_Age = 2,
  Option_Type_Etag = 4,
  Option_Type_Uri_Authority = 5,
  Option_Type_Location = 6,
  Option_Type_Uri_Path = 9,
  Option_Type_Subscription_Lifetime = 10,
  Option_Type_Token = 11,
  Option_Type_Block = 13,
  Option_Type_Uri_Query = 15
} option_type;

struct COAP_PACKET_T
{
	uint8_t header;
	uint8_t code;
	uint16_t id;
	char *payload;

}__attribute__ ((__packed__));
typedef struct COAP_PACKET_T coap_packet_t;


void init_coap_packet(coap_packet_t *packet, char *buf);
int parse_coap_request(coap_packet_t *request, int *need_confirm);
void create_coap_response(coap_packet_t *packet, message_type type, status_code_t code, char *payload, uint16_t lastId);
int create_coap_buffer(char *buf, coap_packet_t *packet, int payload_len);
int copy_headers(char *buf, coap_packet_t *packet);

