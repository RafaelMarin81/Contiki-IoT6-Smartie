
/*
	Author: Pablo L�pez Mart�nez
	email: p.lopezmartinez@um.es
*/

#include "coap.h"
//Create a coap response
void create_coap_response(coap_packet_t *packet, message_type type, status_code_t code, char *payload, uint16_t lastId){
	packet->header = COAP_VERSION | type; //TODO: option field
	packet->code = code;
	packet->id = lastId;
	packet->payload = payload;
	
}

//Comprueba que un paquete coap es correcto, devuelve el codigo de operacion y si es necesario confirmar
int parse_coap_request(coap_packet_t *request, int *need_confirm){
	uint8_t option_len;
  
	//Cheking version
	if((request->header & COAP_VERSION) != COAP_VERSION){
	  vPrintf("version unknown\n");
		return 0; //Version unknown
	}
	if((request->header & 0xF)!=0){
	  option_len= request->header & 0xF;
	  request->payload = request->payload+option_len;
	  //vPrintf("option not implemented\n");
		//return -2; //Option no implemented
	}
	//Checking type
	switch(request->header&0x30){
	
		case MESSAGE_TYPE_CON:			
			*need_confirm = 1;
			break;
		case MESSAGE_TYPE_NON:
      vPrintf("message no confirm\n");
			*need_confirm = 0;
			break;
		default:
		  vPrintf("message no need request\n");
			return -1;//Message is not a request
	}
	//Cheking code
	switch(request->code){
		case GET:
		case POST:
		case PUT:
		case DELETE:
		  vPrintf("Code_request: %x\n",request->code);
			return request->code;
		default:
		  vPrintf("message not request: %x\n", request->code);
			return -1;//Message is not a request	
	}
	
}

//Crea un buffer con un paquete coap
void init_coap_packet(coap_packet_t *packet, char *buf){
	packet->header = buf[0];
	packet->code = buf[1];
	packet->id = buf[2];
	packet->id = packet->id <<8;
	packet->id = packet->id | buf[3];
	packet->payload = buf+4;
}
//deprecated
int create_coap_buffer(char *buf, coap_packet_t *packet, int payload_len){
	
	buf[0] = packet->header;
	buf[1] = packet->code;
	buf[2] = (char)((packet->id >> 8) & 0xff);
	buf[3] = (char)(packet->id&0xff);
	memcpy(buf+4,packet->payload, payload_len);
	return 4+payload_len;
}
//Copy headers to buffer
int copy_headers(char *buf, coap_packet_t *packet){
	
	buf[0] = packet->header;
	buf[1] = packet->code;
	buf[2] = (char)((packet->id >> 8) & 0xff);
	buf[3] = (char)(packet->id&0xff);
	return 4;
}


