
/**
 * 
 * Author: Pablo Lopez Martinez, David Fernandez Ros
 * email: p.lopezmartinez@um.es, david.f.r@um.es
 * 
 * Last Update: 16/7/2013
 * 
 **/

#include "coap.h"
#include "lightdriver.h"

#define DEBUG 1

#if DEBUG
#define PRINTF(...) vPrintf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/**
 * Creates an application-layer buffer from a request.
 * 
 * IMPORTANT: Here you can add new resources and responses for it or create a new function with your profile.
 * 
 * The response format is as follows:
 * 			1.- Set the payload into "char *response_buf"
 * 			2.- Return the CoAP-CODE of the response.
 * 
 **/
static option_type manage_response(char *response_buf, coap_packet_t *request){	
	
	if (strcmp(request->uri_path,"/.well-known/core")==0) {
		PRINTF("Request Query to: /.well-known/core \n");
		sprintf(response_buf, "</sensors/light>;if=\"sensor\",\n</sensors/mdl>;if=\"sensor\",\n</sensors/ip>;if\"sensor\"");
		return CONTENT;
	}else if(strcmp(request->uri_path,"/sensor/mdl")==0){		
		PRINTF("Request Query to: /sensor/mdl \n");
		if(request->code == GET){
			sprintf(response_buf, "Clitech Mote 6LoWPAN-JN5149");
			return CONTENT;
		}
		return NOT_FOUND;
	}else if(strcmp(request->uri_path,"/sensor/ip")==0){
		PRINTF("Request Query to: /sensor/ip \n");
		if (request->code==GET) {			
			sprintf(response_buf, "2001:720:1710:11::2");
			return CONTENT;
		}
		return NOT_FOUND;
	}else if(strcmp(request->uri_path,"/sensor/light")==0){
		PRINTF("Request Query to: /sensor/light \n");
		if (request->code==GET) {
			if(get_light_status()){				
				sprintf(response_buf, "ON");
			}else{
				sprintf(response_buf, "OFF");
			}			
			return CONTENT;
		} else if (request->code==PUT) {
			char * contain;
			if ((contain = strstr((char *)request->payload,"0")) != NULL) {
				set_off();
			} else if ((contain = strstr((char *)request->payload,"OFF")) != NULL) {
				set_off();
			} else if ((contain = strstr((char *)request->payload,"1")) != NULL) {
				set_on();
			} else if ((contain = strstr((char *)request->payload,"ON")) != NULL){
				set_on();
			} else {
				return NOT_FOUND; 
			}
			if(get_light_status()){				
				sprintf(response_buf, "ON");
			}else{
				sprintf(response_buf, "OFF");
			}
			return CHANGED;
		} else if (request->code==POST) {
			if(switch_light()){				
				sprintf(response_buf, "ON");
			}else{
				sprintf(response_buf, "OFF");
			}
			return CHANGED;
		}
		return NOT_FOUND;
	}
	
	return NOT_FOUND;
	
}

/**
 *  Parses the packet received from buff to the structure.
 * 
 *  CoAP Version 1, draft 12.
 *  Options implements:
 * 		-URI-PATH (11)
 * 
 **/
int coap12_parse_request(coap_packet_t * request, char *buf, int size){
	
	int i = 0;
	uint8_t next_option = 0;								//Used for extract the option_code
	uint8_t last_option = 0;
	int buffer_pos = 0;										//Used for store the current buffer position
	uint8_t option_len = 0;									//Used for 
	
	
	request->version = buf[0] & 0xC0;						//Getting the first 2 bits
	if(request->version != COAP_VERSION_DRAFT_12)			//If the version is different then the request is invalid
		return -1;
	request->type = buf[0] & 0x30;							//Getting the next 2 bits
	request->option_count = buf[0] & 0xF;					//Getting the last 4 bits
	
	request->code = buf[1];									//Getting the code (GET,POST,PUT,DELETE) (1 byte)
	
	request->id = 0;										//Getting the id (2 bytes)
	request->id = request->id | (unsigned char)buf[2];
	request->id = (request->id <<8);
	request->id = request->id | (unsigned char)buf[3];
	
	request->uri_len = 0;									//Init uri_len
	
	
	if(request->option_count > 0){							//If there are Option
		buffer_pos = 4;
		
		for(i=0;i<request->option_count;i++){				//For each option		
		
			next_option = ((unsigned char)buf[buffer_pos]>>4);//Getting delta code
			if(next_option == 0 && last_option != 0){
				//~ PRINTF("Same Option found.\n");
				next_option = last_option;
			}
			switch(next_option){
				
				case 11:												//Token (Uri-Path)
					PRINTF("Option Token Found...\n");
					option_len = buf[buffer_pos] &  0x0F; 				//Getting the Token String len
					buffer_pos++;										//Going to String position
					if(next_option == last_option)						//Parsing URI allocation for secondaries resources
						request->uri_len++;
					memcpy(request->uri_path+request->uri_len, buf+(buffer_pos-1), option_len+1);					
					request->uri_path[request->uri_len] = (unsigned char)0x2f;
					request->uri_len += option_len;						//Storing URI-Len
					buffer_pos=buffer_pos + option_len; 				//Updating buffer position
					last_option = next_option;
					
					break;
				default:
					PRINTF("Unkown Option... nº: %d\n",next_option);
					return -1;
					break;
			}
		}
		if(size > buffer_pos){
			memcpy(request->payload, buf+buffer_pos, size-buffer_pos);	//If there are payload, get it
		}
		
	}else{																//If no options			
		//~ request->payload = buf+4;		
		return -1;
	}
	PRINTF("---------------PACKET RESULT----------------\n");
	PRINTF("Request ID: %d \n",request->id);	
	PRINTF("Request Code: %d \n",request->code);
	PRINTF("Nº Options: %d \n",request->option_count);
	PRINTF("Request URI-PATH: %s\n",request->uri_path);
	PRINTF("Request Payload: %s\n", request->payload);
	PRINTF("--------------------------------------------\n");
	
	return 0;
}


/**
 *	Creates a response from a request and stores it in a byte buffer.
 * 	
 *  CoAP Version 1, draft 12.
 * 
 **/
int coap12_create_response(char *destination_buf, coap_packet_t *request){
	
	char payload_response[1000];
	
	destination_buf[0] = COAP_VERSION_DRAFT_12 | MESSAGE_TYPE_ACK;		//CoAP Header first byte (CoAP_Version | Message Type | Option Count)
	destination_buf[1] = manage_response(destination_buf+4, request);									//Code of response
	destination_buf[2] = (unsigned char)((request->id >> 8) & 0x00ff);	//ID of message
	destination_buf[3] = (unsigned char)(request->id&0x00ff);
	
	
	return 0;
}


//Create a coap response (Create a empty response)
//~ void create_coap_response(coap_packet_t *packet, message_type type, status_code_t code, char *payload, uint16_t lastId){
	//~ packet->header = COAP_VERSION | type; //TODO: option field
	//~ packet->code = code;
	//~ packet->id = lastId;
	//~ packet->payload = payload;
//~ }
//~ 
//Copy headers to buffer (in order to send message)
//~ int copy_headers(unsigned char *buf, coap_packet_t *packet){
	//~ 
	//~ buf[0] = packet->header;
	//~ buf[1] = packet->code;
	//~ buf[2] = (unsigned char)((packet->id >> 8) & 0x00ff);
	//~ buf[3] = (unsigned char)(packet->id&0x00ff);
	//~ 
	//~ return 4;
//~ }
//~ 

