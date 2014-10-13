
/**
 * 
 * Author: Pablo Lopez Martinez
 *
 */
#include "coap.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define DEBUG 0

#if DEBUG
	#define PRINTF(...) printf(__VA_ARGS__)
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
		PRINTF("Request to /.well-known/core received.\n")
		sprintf(response_buf, "</ble/002A>;if=\"sensor\",\n</dev/name>;if=\"sensor\"");
		return CONTENT;
	}else if(strcmp(request->uri_path,"/dev/name")==0) {
		PRINTF("Request to /dev/name received.\n")
		sprintf(response_buf, "EXAMPLE NAME");
		return CONTENT;
	
	}else if(strcmp(request->uri_path,"/temperature")==0){
		PRINTF("Request Query to: /sensor/temperature \n");
		if(request->code == GET){
			sprintf(response_buf, "{\"tag\":\"obj\",\"href\":\"/temperature\",\"is\":\"iot:TemperatureSensor\",\"nodes\":[{\"val\":%d,\"unit\":\"obix:units/celsius\",\"tag\":\"real\",\"name\":\"value\",\"href\":\"value\"}]}",(int)temp);
			return CONTENT;
		}
		return METHOD_NOT_ALLOWED;
	}else if(strcmp(request->uri_path,"/lum")==0){
		PRINTF("Request Query to: /sensor/lum \n");
		if(request->code == GET){
			sprintf(response_buf, "{\"tag\":\"obj\",\"href\":\"/lum\",\"is\":\"iot:LuminositySensor\",\"nodes\":[{\"val\":%d,\"unit\":\"obix:units/lumens\",\"tag\":\"real\",\"name\":\"value\",\"href\":\"value\"}]}",(int)lum);
			return CONTENT;
		}
		return METHOD_NOT_ALLOWED;
	}else if(strcmp(request->uri_path,"/hum")==0){
		PRINTF("Request Query to: /sensor/hum \n");
		if(request->code == GET){
			sprintf(response_buf, "{\"tag\":\"obj\",\"href\":\"/hum\",\"is\":\"iot:HumiditySensor\",\"nodes\":[{\"val\":%d,\"unit\":\"obix:units/relativeHumidity\",\"tag\":\"real\",\"name\":\"value\",\"href\":\"value\"}]}",(int)hum);
			return CONTENT;
		}
		return METHOD_NOT_ALLOWED;
	}
	sprintf(response_buf, "The resource does not exists.");
	return NOT_FOUND;

}



/**
 *  Parses the packet received from buff and store it on the coap_packet_t.
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
	
	memset(request->uri_path, 0, 127);
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


int coap12_create_request(coap_packet_t *request, message_type type, unsigned char opt_count, request_code_t code){
	
	request->version = COAP_VERSION_DRAFT_12;
	request->type = type;
	request->option_count = opt_count;
	request->id = 0xC049;
	request->code = code;
	
	return 0;
	
}


int coap12_copy_headers(char *destination_buf, coap_packet_t *request){
  	
	destination_buf[0] = COAP_VERSION_DRAFT_12 | request->type;			//CoAP Header first byte (CoAP_Version | Message Type | Option Count)
	destination_buf[1] = manage_response(destination_buf+4, request);		//Code of response
	//destination_buf[1] = manager((unsigned char *)destination_buf+4, request); 	//Code of response
									
	destination_buf[2] = (unsigned char)((request->id >> 8) & 0x00ff);		//ID of message
	destination_buf[3] = (unsigned char)(request->id&0x00ff);	

	return 0;
}

/**
 *	Creates a response from a request and stores it in a byte buffer.
 * 	
 *  CoAP Version 1, draft 12.
 * 
 **/
int coap12_create_response(char *destination_buf, coap_packet_t *request, int (*manager)(unsigned char*,coap_packet_t *) ){
	
	destination_buf[0] = COAP_VERSION_DRAFT_12 | MESSAGE_TYPE_ACK;			//CoAP Header first byte (CoAP_Version | Message Type | Option Count)
	destination_buf[1] = manage_response(destination_buf+4, request);		//Code of response
	//destination_buf[1] = manager((unsigned char *)destination_buf+4, request); 	//Code of response
									
	destination_buf[2] = (unsigned char)((request->id >> 8) & 0x00ff);		//ID of message
	destination_buf[3] = (unsigned char)(request->id&0x00ff);	

	return 0;
}



