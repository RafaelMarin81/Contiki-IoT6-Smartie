
/*
 * Author: David Fernandez Ros, Pablo Lopez Martinez
 * email: david.f.r@um.es, p.lopezmartinez@um.es
 * 
 * Last Update: 17/1/2013
*/

#include "coap.h"

#define DEBUG 0

#if DEBUG
#define VPRINTF(...) vPrintf(__VA_ARGS__)
#else
#define VPRINTF(...)
#endif

// For GET: Replace options length for / on payload
void replace_gaps(char * payload,u8_t pos,u8_t len){
	
	if (pos==0)
		payload[pos] = payload[pos]-0x90;
		
	if (pos<len) {
		u8_t next = payload[pos]+1;
		payload[pos]=0x2f;
		replace_gaps(payload,pos+next,len);
	}
	
}

// Parses the packet received from buff to a initialized structure
int parse_coap_packet(coap_packet_t *request, char *buf, int *need_confirm){
	// Initialize packet
	request->header = buf[0];
	request->code = buf[1];
	request->id=0;
	request->id = request->id | (unsigned char)buf[2];
	request->id = (request->id <<8);
	request->id = request->id | (unsigned char)buf[3];
	request->payload = buf+4;
  
	//Cheking version
	if((request->header & COAP_VERSION) != COAP_VERSION){
	  VPRINTF("version unknown\n");
		return 0; //Version unknown
	}

	//Checking type
	switch(request->header&0x30){
	
		case MESSAGE_TYPE_CON:		
			*need_confirm = 1;
			break;
		case MESSAGE_TYPE_NON:
      VPRINTF("message no confirm\n");
			*need_confirm = 0;
			break;
		default:
		  VPRINTF("message no need request\n");
			return -1;//Message is not a request
	}
	// Check code
	switch(request->code){
		case GET:
		    replace_gaps(request->payload,0,strlen(request->payload));
		    VPRINTF("CoAP Type: GET\n");
		    break;
		case POST:
		    VPRINTF("CoAP Type: POST\n");
		    break;
		case PUT:
		    VPRINTF("CoAP Type: PUT\n");
		    break;
		case DELETE:
		    VPRINTF("CoAP Type: DELETE\n");
		    break;
		default:
		    VPRINTF("message not request: %x\n", request->code);
			return -1;//Message is not a request	
	}
	
	return request->code;
}

//Create a coap response (Create a empty response)
void create_coap_response(coap_packet_t *packet, message_type type, status_code_t code, char *payload, uint16_t lastId){
	packet->header = COAP_VERSION | type; //TODO: option field
	packet->code = code;
	packet->id = lastId;
	packet->payload = payload;
}

//Copy headers to buffer (in order to send message)
int copy_headers(unsigned char *buf, coap_packet_t *packet){
	
	buf[0] = packet->header;
	buf[1] = packet->code;
	buf[2] = (unsigned char)((packet->id >> 8) & 0x00ff);
	buf[3] = (unsigned char)(packet->id&0x00ff);
	
	return 4;
}


