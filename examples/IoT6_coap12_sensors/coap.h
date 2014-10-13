#include <stdlib.h>
#include <string.h>
#include <stdint.h>


/**
 * 
 * Author: Pablo Lopez Martinez
 * email: p.lopezmartinez@um.es
 * 
 * Last Update: 16/7/2013
 * 
 **/

#ifndef __COAP_H
  #define __COAP_H

#define DEFAULT_PORT 5683

/*
 * Version (Ver): 2-bit unsigned integer. Indicates the CoAP version
 * number. Implementations of this specification MUST set this field
 * to 1. Other values are reserved for future versions.
 * 
 * Type (T): 2-bit unsigned integer. Indicates if this message is of
 * type Confirmable (0), Non-Confirmable (1), Acknowledgement (2) or
 * Reset (3). See Section 4 for the semantics of these message
 * types.
 * 
 * Option Count (OC): 4-bit unsigned integer. Indicates the number of
 * options after the header. If set to 0, there are no options and
 * the payload (if any) immediately follows the header. The format
 * of options is defined below.
 * 
 * Code: 8-bit unsigned integer. Indicates if the message carries a
 * request (1-31) or a response (64-191), or is empty (0). (All
 * other code values are reserved.) In case of a request, the Code
 * field indicates the Request Method; in case of a response a
 * Response Code. Possible values are maintained in the CoAP Code
 * Registry (Section 11.1). See Section 5 for the semantics of
 * requests and responses.
 * 
 * Message ID: 16-bit unsigned integer. Used for the detection of
 * message duplication, and to match messages of type
 * Acknowledgement/Reset and messages of type Confirmable. See
 * Section 4 for Message ID generation rules and how messages are
 * matched.
 * 
 * 
*/
/*
 0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Ver| T |  OC   |      Code     |          Message ID           |   HEADER
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Options (if any) ...											OPTIONS
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Payload (if any) ...											PAYLOAD
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
* 
* 
*/

#define COAP_VERSION_DRAFT_12 0x40

typedef enum {
	created = 65,
	deleted = 66,
	valid = 67,
	changed = 68,
	content = 69,
	badrequest = 128,
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

/**
+------+---------------------------------+-----------+
| Code | Description                     | Reference |
+------+---------------------------------+-----------+
|   65 | 2.01 Created                    | [RFCXXXX] |
|   66 | 2.02 Deleted                    | [RFCXXXX] |
|   67 | 2.03 Valid                      | [RFCXXXX] |
|   68 | 2.04 Changed                    | [RFCXXXX] |
|   69 | 2.05 Content                    | [RFCXXXX] |
|  128 | 4.00 Bad Request                | [RFCXXXX] |
|  129 | 4.01 Unauthorized               | [RFCXXXX] |
|  130 | 4.02 Bad Option                 | [RFCXXXX] |
|  131 | 4.03 Forbidden                  | [RFCXXXX] |
|  132 | 4.04 Not Found                  | [RFCXXXX] |
|  133 | 4.05 Method Not Allowed         | [RFCXXXX] |
|  134 | 4.06 Not Acceptable             | [RFCXXXX] |
|  140 | 4.12 Precondition Failed        | [RFCXXXX] |
|  141 | 4.13 Request Entity Too Large   | [RFCXXXX] |
|  143 | 4.15 Unsupported Content-Format | [RFCXXXX] |
|  160 | 5.00 Internal Server Error      | [RFCXXXX] |
|  161 | 5.01 Not Implemented            | [RFCXXXX] |
|  162 | 5.02 Bad Gateway                | [RFCXXXX] |
|  163 | 5.03 Service Unavailable        | [RFCXXXX] |
|  164 | 5.04 Gateway Timeout            | [RFCXXXX] |
|  165 | 5.05 Proxying Not Supported     | [RFCXXXX] |
+------+---------------------------------+-----------+
*/

typedef enum {
  CREATED = 65,
  DELETED = 66,
  VALID = 67,
  CHANGED = 68,
  CONTENT = 69,
  BAD_REQUEST = 128,
  METHOD_NOT_ALLOWED = 133,
  NOT_FOUND = 132
} option_type;


struct COAP_OPTION_T
{
	uint8_t delta;
	char buf[127];

}__attribute__ ((__packed__));
typedef struct COAP_OPTION_T coap_option_t;

struct COAP_PACKET_T
{
	uint8_t version;
	uint8_t type;
	uint8_t option_count;
	uint8_t code;
	uint16_t id;
	
	uint8_t uri_len;
	char uri_path[127];
	char *uri_query;	
	char *payload[256];

}__attribute__ ((__packed__));
typedef struct COAP_PACKET_T coap_packet_t;

/**
 *  Parses the packet received from buff and store it on the coap_packet_t.
 * 
 *  CoAP Version 1, draft 12.
 *  Options implements:
 * 		-URI-PATH (11)
 * 
 **/
int coap12_parse_request(coap_packet_t * destination_request, char *udp_buf, int size);

/**
 *	Creates a response from a request and stores it in a byte buffer.
 * 	
 *  CoAP Version 1, draft 12.
 * 
 **/
int coap12_create_response(char *destination_buf, coap_packet_t *request, int (*manager)(unsigned char*,coap_packet_t *));


int coap12_create_request(coap_packet_t *request, message_type type, unsigned char opt_count, request_code_t code);



#endif

