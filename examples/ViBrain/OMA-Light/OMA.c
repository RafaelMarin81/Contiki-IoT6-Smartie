
/*
 * Author:  Pablo Lopez Martinez, David Fernandez Ros
 * email: p.lopezmartinez@um.es, david.f.r@um.es
 * 
 * Last Update: 30/5/2013
*/
#ifndef __OMA_C
#define __OMA_C


#define DEBUG 0

#if DEBUG
#define VPRINTF(...) vPrintf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x ", ((u8_t *)addr)[0], ((u8_t *)addr)[1], ((u8_t *)addr)[2], ((u8_t *)addr)[3], ((u8_t *)addr)[4], ((u8_t *)addr)[5], ((u8_t *)addr)[6], ((u8_t *)addr)[7], ((u8_t *)addr)[8], ((u8_t *)addr)[9], ((u8_t *)addr)[10], ((u8_t *)addr)[11], ((u8_t *)addr)[12], ((u8_t *)addr)[13], ((u8_t *)addr)[14], ((u8_t *)addr)[15])
#else
#define VPRINTF(...)
#endif

#include "OMA.h"
#include "lightdriver.h"
#include "coap.h"

static uint32_t dimmer_level = 0;

static void set_response_code(char * response, response_code cod) {
	response[1] = cod;
}

////////// Error messages  //////////
static int response_error_OMA_command(char * response, int * writePos, int msg){
	switch(msg){
		case 1:
			sprintf(response+*writePos, "Unknown command");
			set_response_code(response,badrequest);
			break;
		case 2:
			sprintf(response+*writePos, "Incomplete OMA command (Parameters?)");
			set_response_code(response,preconditionfailed);
			break;
		case 3:
			sprintf(response+*writePos, "Unimplemented OMA command");
			set_response_code(response,notimplemented);
			break;
		case 4:
			sprintf(response+*writePos, "Invalid OMA command");
			set_response_code(response,notallowed);
			break;
		case 5:
			sprintf(response+*writePos, "OMA Object not found");
			set_response_code(response,notallowed);
			break;
		default:
			break;
	}
	return strlen(response);
}

////////// Resource Discovery //////////
static int response_discover_services(coap_packet_t * request, char * response, int * writePos) {
	if (request->code==GET) {
		sprintf(response+*writePos, "{\"e\":[{\"n\":\"109/0/0\",\"sv\":\"On/Off\"},{\"n\":\"109/0/1\",\"sv\":\"Dimmer\"},{\"n\":\"109/0/2\",\"sv\":\"On time\"},{\"n\":\"109/0/3\",\"sv\":\"Total Power Used\"},{\"n\":\"109/0/4\",\"sv\":\"Power Factor\"},{\"n\":\"109/0/5\",\"sv\":\"Relative Dimmer\"},{\"n\":\"110/0/0\",\"sv\":\"State\"},{\"n\":\"110/0/1\",\"sv\":\"Frequency\"},{\"n\":\"110/0/2\",\"sv\":\"Counter\"},{\"n\":\"110/0/3\",\"sv\":\"Total Power Used\"},{\"n\":\"110/0/4\",\"sv\":\"Power Factor\"},{\"n\":\"110/0/5\",\"sv\":\"Relative Dimmer\"},{\"n\":\"111/0/0\",\"sv\":\"Resource Directory IP\"}]}");
		set_response_code(response,content);
		return strlen(response);
	}
	return response_error_OMA_command(response,writePos,4);
}


////////// Object0's Resources //////////

static int response_resource_0_0_onoff(coap_packet_t * request, char * response, int * writePos) {
	char * contain;
	if (request->code==GET) {
		if(get_light_status()){				
			sprintf(response+*writePos, "ON");
		}else{
			sprintf(response+*writePos, "OFF");
		}
		set_response_code(response,content);
		return strlen(response);
	} else if (request->code==PUT) {
		if ((contain = strstr(request->payload+10,"0")) != NULL) {
			set_off();
		} else if ((contain = strstr(request->payload+10,"OFF")) != NULL) {
			set_off();
		} else if ((contain = strstr(request->payload+10,"1")) != NULL) {
			set_on();
		} else if ((contain = strstr(request->payload+10,"ON")) != NULL){
			set_on();
		} else {
			return response_error_OMA_command(response,writePos,2); 
		}
		if(get_light_status()){				
			sprintf(response+*writePos, "ON");
		}else{
			sprintf(response+*writePos, "OFF");
		}
		set_response_code(response,changed);
		return strlen(response);
	} else if (request->code==POST) {
		if(switch_light()){				
			sprintf(response+*writePos, "ON");
		}else{
			sprintf(response+*writePos, "OFF");
		}
		set_response_code(response,changed);
		return strlen(response);
	}
	return response_error_OMA_command(response,writePos,4);
}

static int response_resource_0_1_dimmer(coap_packet_t * request, char * response,int * writePos) {
	if (request->code==GET) {
		sprintf(response+*writePos, "%d",dimmer_level);
		set_response_code(response,content);
		return strlen(response);
	} else if (request->code==PUT) {
		dimmer_level = atoi(request->payload+10);
		sprintf(response+*writePos, "%d",set_luminity(dimmer_level));
		set_response_code(response,changed);
		return strlen(response);
	} else if (request->code==POST) {
		dimmer_level = atoi(request->payload+10);
		sprintf(response+*writePos, "%d",set_luminity(dimmer_level));
		set_response_code(response,changed);
		return strlen(response);
	}
	return response_error_OMA_command(response,writePos,4);
}

//TODO: How to make a ON light timer
static int response_resource_0_2_onTime(coap_packet_t * request, char * response,int * writePos){
	
	if (request->code==GET) {
		sprintf(response+*writePos, "0");
		set_response_code(response,content);
		return strlen(response);
	} 
	/*else if (request->code==PUT) {
		sprintf(response+*writePos, "%d",set_luminity(atoi(request->payload+10)));
		set_response_code(response,changed);
		return strlen(response);
	} else if (request->code==POST) {
		sprintf(response+*writePos, "%d",set_luminity(10));
		set_response_code(response,changed);
		return strlen(response);
	}*/
	return response_error_OMA_command(response,writePos,4);
}

static int response_resource_0_3_powerused(coap_packet_t * request, char * response,int * writePos) {
	if (request->code==GET) {
		sprintf(response+*writePos, "0");
		set_response_code(response,content);
		return strlen(response);
	} 
	
	return response_error_OMA_command(response,writePos,4);
	
}

static int response_resource_0_4_powerfactor(coap_packet_t * request, char * response,int * writePos) {
	if (request->code==GET) {
		sprintf(response+*writePos, "0");
		set_response_code(response,content);
		return strlen(response);
	} 
	
	return response_error_OMA_command(response,writePos,4);
}

static int response_resource_0_5_relativedimmer(coap_packet_t * request, char * response,int * writePos) {	
		
	if (request->code==GET) {
		sprintf(response+*writePos, "%d",dimmer_level);
		set_response_code(response,content);
		return strlen(response);
	} else if (request->code==PUT) {
		VPRINTF("Setting new dimmer number %s\n", request->payload+10);
		dimmer_level = atoi(request->payload+10);
		sprintf(response+*writePos, "%d",set_luminity(dimmer_level));
		set_response_code(response,changed);
		return strlen(response);
	}
	return response_error_OMA_command(response,writePos,4);	
}


////////// Object1's Resources //////////

//TODO: I dont know the mean of PUT and POST codes.
static int response_resource_1_0_crossover_state(coap_packet_t * request, char * response,int * writePos) {
	if (request->code==GET) {
		sprintf(response+*writePos, "%d",get_luminity());
		set_response_code(response,content);
		return strlen(response);
	} else if (request->code==PUT) {
		//~ sprintf(response+*writePos, "%d",set_luminity(atoi(request->payload+10)));
		//~ set_response_code(response,changed);
		//~ return strlen(response);
	} else if (request->code==POST) {
		//~ sprintf(response+*writePos, "%d",set_luminity(10));
		//~ set_response_code(response,changed);
		//~ return strlen(response);
	}
	return response_error_OMA_command(response,writePos,4);	
}

//TODO: how to get the Electricity Signal Frequency.
static int response_resource_1_1_crossover_freq(coap_packet_t * request, char * response,int * writePos) {
	if (request->code==GET) {
		sprintf(response+*writePos, "%d",get_luminity());
		set_response_code(response,content);
		return strlen(response);
	} else if (request->code==PUT) {
		//~ sprintf(response+*writePos, "%d",set_luminity(atoi(request->payload+10)));
		//~ set_response_code(response,changed);
		//~ return strlen(response);
	} else if (request->code==POST) {
		//~ sprintf(response+*writePos, "%d",set_luminity(10));
		//~ set_response_code(response,changed);
		//~ return strlen(response);
	}
	return response_error_OMA_command(response,writePos,4);	
}

//TODO: How to make a ON light timer
static int response_resource_1_2_crossover_counter(coap_packet_t * request, char * response,int * writePos) {
	if (request->code==GET) {
		sprintf(response+*writePos, "%d",get_luminity());
		set_response_code(response,content);
		return strlen(response);
	} else if (request->code==PUT) {
		//~ sprintf(response+*writePos, "%d",set_luminity(atoi(request->payload+10)));
		//~ set_response_code(response,changed);
		//~ return strlen(response);
	} else if (request->code==POST) {
		//~ sprintf(response+*writePos, "%d",set_luminity(10));
		//~ set_response_code(response,changed);
		//~ return strlen(response);
	}
	return response_error_OMA_command(response,writePos,4);	
}

//TODO: How to get the power used
static int response_resource_1_3_crossover_powerused(coap_packet_t * request, char * response,int * writePos) {
	if (request->code==GET) {
		sprintf(response+*writePos, "%d",dimmer_level);
		set_response_code(response,content);
		return strlen(response);
	} else if (request->code==PUT) {
		//~ sprintf(response+*writePos, "%d",set_luminity(atoi(request->payload+10)));
		//~ set_response_code(response,changed);
		//~ return strlen(response);
	} else if (request->code==POST) {
		//~ sprintf(response+*writePos, "%d",set_luminity(10));
		//~ set_response_code(response,changed);
		//~ return strlen(response);
	}
	return response_error_OMA_command(response,writePos,4);	
}
//TODO: How to get the power factor of the light
static int response_resource_1_4_crossover_powerfactor(coap_packet_t * request, char * response,int * writePos) {
	if (request->code==GET) {
		sprintf(response+*writePos, "%d",get_luminity());
		set_response_code(response,content);
		return strlen(response);
	} else if (request->code==PUT) {
		//~ sprintf(response+*writePos, "%d",set_luminity(atoi(request->payload+10)));
		//~ set_response_code(response,changed);
		//~ return strlen(response);
	} else if (request->code==POST) {
		//~ sprintf(response+*writePos, "%d",set_luminity(10));
		//~ set_response_code(response,changed);
		//~ return strlen(response);
	}
	return response_error_OMA_command(response,writePos,4);	
}


static int response_resource_1_5_crossover_relativedimmer(coap_packet_t * request, char * response,int * writePos) {
	if (request->code==GET) {
		sprintf(response+*writePos, "%d",dimmer_level);
		set_response_code(response,content);
		return strlen(response);
	} else if (request->code==PUT) {
		dimmer_level = atoi(request->payload+10);
		sprintf(response+*writePos, "%d",set_luminity(dimmer_level));
		set_response_code(response,changed);
		return strlen(response);
	}
	return response_error_OMA_command(response,writePos,4);	
}

u16_t newAddress[8];

void set_newAddress(char *ip){
	int i=0;
	char *aux;
	aux = strtok(ip,":");
	while(aux != NULL && i<8){
		newAddress[i] = (u16_t)strtol(aux, NULL, 16);
		i++;
		aux = strtok (NULL, ":");
		
	}
}

static int response_resource_2_0_rd_ip_conf(coap_packet_t * request, char * response,int * writePos){
	//~ int i=0;
	//~ char *aux;
	if(request->code==GET){
		sprintf(response+*writePos," %x:%x:%x:%x:%x:%x:%x:%x ",newAddress[0], newAddress[1], newAddress[2], newAddress[3], newAddress[4], newAddress[5], newAddress[6], newAddress[7]);
		set_response_code(response,content);
		return strlen(response);		
	}else if(request->code==PUT){
		VPRINTF("Entro %s\n",request->payload+10);
		set_newAddress(request->payload+10);
		//~ aux = strtok(request->payload+10,":");
		//~ VPRINTF("Entro1\n");
		//~ while(aux != NULL && i<8){
			//~ newAddress[i] = (u16_t)strtol(aux, NULL, 16);
			//~ i++;
			//~ aux = strtok (NULL, ":");
			//~ 
		//~ }
		VPRINTF("Entro2\n");
		set_new_rd_destination(newAddress);
		VPRINTF("Entro3\n");
		sprintf(response+*writePos, " %x:%x:%x:%x:%x:%x:%x:%x ",newAddress[0], newAddress[1], newAddress[2], newAddress[3], newAddress[4], newAddress[5], newAddress[6], newAddress[7]);
		set_response_code(response,content);
		return strlen(response);
	}
	
	
}

static int response_resource_3_0_0_manufacturer(coap_packet_t * request, char * response,int * writePos){
	
	if (request->code==GET) {
		sprintf(response+*writePos, "ViBrain Solitions");
		set_response_code(response,content);
		return strlen(response);
	}
	return response_error_OMA_command(response,writePos,4);	
}

static int response_resource_3_1_0_modelNumber(coap_packet_t * request, char * response,int * writePos){
	
	if (request->code==GET) {
		sprintf(response+*writePos, "0001");
		set_response_code(response,content);
		return strlen(response);
	}
	return response_error_OMA_command(response,writePos,4);	
}

static int response_resource_3_2_0_serialNumber(coap_packet_t * request, char * response,int * writePos){
	
	if (request->code==GET) {
		sprintf(response+*writePos, "0001");
		set_response_code(response,content);
		return strlen(response);
	}
	return response_error_OMA_command(response,writePos,4);	
}

static int response_resource_3_5_0_client_version(coap_packet_t * request, char * response,int * writePos){
	
	if (request->code==GET) {
		sprintf(response+*writePos, "0001");
		set_response_code(response,content);
		return strlen(response);
	}
	return response_error_OMA_command(response,writePos,4);	
}

static int response_resource_3_7_0_firmware(coap_packet_t * request, char * response,int * writePos){
	
	if (request->code==GET) {
		sprintf(response+*writePos, "0001");
		set_response_code(response,content);
		return strlen(response);
	}
	return response_error_OMA_command(response,writePos,4);	
}

static int response_resource_3_8_0_reboot(coap_packet_t * request, char * response,int * writePos){
	
	//~ if (request->code==GET) {
		//~ sprintf(response+*writePos, "0001");
		//~ set_response_code(response,content);
		//~ return strlen(response);
	//~ }
	return response_error_OMA_command(response,writePos,3);	
}

static int response_resource_3_9_0_factory_reset(coap_packet_t * request, char * response,int * writePos){
	
	//~ if (request->code==GET) {
		//~ sprintf(response+*writePos, "0001");
		//~ set_response_code(response,content);
		//~ return strlen(response);
	//~ }
	return response_error_OMA_command(response,writePos,3);	
}

static int response_resource_3_10_0_power_status(coap_packet_t * request, char * response,int * writePos){
	
	if (request->code==GET) {
		sprintf(response+*writePos, "0");
		set_response_code(response,content);
		return strlen(response);
	}
	return response_error_OMA_command(response,writePos,3);	
}

static int response_resource_3_11_0_battery_level(coap_packet_t * request, char * response,int * writePos){
	
	if (request->code==GET) {
		sprintf(response+*writePos, "100%");
		set_response_code(response,content);
		return strlen(response);
	}
	return response_error_OMA_command(response,writePos,3);	
}

static int response_resource_3_16_0_memory_free(coap_packet_t * request, char * response,int * writePos){
	
	//~ if (request->code==GET) {
		//~ sprintf(response+*writePos, "0001");
		//~ set_response_code(response,content);
		//~ return strlen(response);
	//~ }
	return response_error_OMA_command(response,writePos,3);	
}

static int response_resource_3_17_0_device_state(coap_packet_t * request, char * response,int * writePos){
	
	if (request->code==GET) {
		sprintf(response+*writePos, "1");
		set_response_code(response,content);
		return strlen(response);
	}
	return response_error_OMA_command(response,writePos,3);	
}

static int response_resource_3_18_0_error_condition(coap_packet_t * request, char * response,int * writePos){
	
	if (request->code==GET) {
		sprintf(response+*writePos, "0");
		set_response_code(response,content);
		return strlen(response);
	}
	return response_error_OMA_command(response,writePos,3);	
}



// Parser
int parse_OMA(coap_packet_t * request, char * response, int * writePos) {
	
	char * contain;
	VPRINTF("\nPAYLOAD: %s\n", request->payload);
	// If full payload is resource discovery /.well-known/core   // Length its take in account when the command only accepts get in order to avoid more cost to compare
	if (strcmp(request->payload,"/.well-known/core")==0) {
		return response_discover_services(request,response,writePos);
		
	// If another type of message
	} else {
			if ((contain = strstr(request->payload,"/109/0/0")) != NULL) {
				
				return response_resource_0_0_onoff(request,response,writePos);
				
			} else if ((contain = strstr(request->payload,"/109/0/1")) != NULL) {
				
				return response_resource_0_1_dimmer(request, response, writePos);
				 
			} else if ((contain = strstr(request->payload,"/109/0/2")) != NULL) {
				
				return response_resource_0_2_onTime(request, response, writePos);
				
			} else if ((contain = strstr(request->payload,"/109/0/3")) != NULL) {
				
				return response_resource_0_3_powerused(request, response, writePos);
				
			} else if ((contain = strstr(request->payload,"/109/0/4")) != NULL) {
				
				return response_resource_0_4_powerfactor(request, response, writePos);
				
			} else if ((contain = strstr(request->payload,"/109/0/5")) != NULL) {
				
				return response_resource_0_5_relativedimmer(request, response, writePos);
				
			}else if ((contain = strstr(request->payload,"/110/0/0")) != NULL) {
				
				return response_resource_1_0_crossover_state(request,response,writePos);
				
			} else if ((contain = strstr(request->payload,"/110/0/1")) != NULL) {
				
				return response_resource_1_1_crossover_freq(request, response, writePos);
				 
			} else if ((contain = strstr(request->payload,"/110/0/2")) != NULL) {
				
				return response_resource_1_2_crossover_counter(request, response, writePos);
				
			} else if ((contain = strstr(request->payload,"/110/0/3")) != NULL) {
				
				return response_resource_1_3_crossover_powerused(request, response, writePos);
				
			} else if ((contain = strstr(request->payload,"/110/0/4")) != NULL) {
				
				return response_resource_1_4_crossover_powerfactor(request, response, writePos);
				
			} else if ((contain = strstr(request->payload,"/110/0/5")) != NULL) {
				
				return response_resource_1_5_crossover_relativedimmer(request, response, writePos);
				
			}else if ((contain = strstr(request->payload,"111/0/0")) != NULL) {			
				
				return response_resource_2_0_rd_ip_conf(request,response,writePos);
				
			} else if ((contain = strstr(request->payload,"111/0/1")) != NULL) {
				
				//~ return response_resource_2_1_rd_port_conf(request, response, writePos);			
								
			} else if ((contain = strstr(request->payload,"3/0/0")) != NULL) {
				
				return response_resource_3_0_0_manufacturer(request,response,writePos);
			
			} else if ((contain = strstr(request->payload,"3/1/0")) != NULL) {
				
				return response_resource_3_1_0_modelNumber(request,response,writePos);
				
			} else if ((contain = strstr(request->payload,"3/2/0")) != NULL) {
				
				return response_resource_3_2_0_serialNumber(request,response,writePos);
				
			} else if ((contain = strstr(request->payload,"3/5/0")) != NULL) {
				
				return response_resource_3_5_0_client_version(request,response,writePos);
				
			} else if ((contain = strstr(request->payload,"3/7/0")) != NULL) {
				
				return response_resource_3_7_0_firmware(request,response,writePos);
				
			} else if ((contain = strstr(request->payload,"3/8/0")) != NULL) {
				
				return response_resource_3_8_0_reboot(request,response,writePos);
				
			} else if ((contain = strstr(request->payload,"3/9/0")) != NULL) {
				
				return response_resource_3_9_0_factory_reset(request,response,writePos);
				
			} else if ((contain = strstr(request->payload,"3/10/0")) != NULL) {
				
				return response_resource_3_10_0_power_status(request,response,writePos);
				
			} else if ((contain = strstr(request->payload,"3/11/0")) != NULL) {
				
				return response_resource_3_11_0_battery_level(request,response,writePos);
				
			} else if ((contain = strstr(request->payload,"3/16/0")) != NULL) {
				
				return response_resource_3_16_0_memory_free(request,response,writePos);
				
			} else if ((contain = strstr(request->payload,"3/17/0")) != NULL) {
				
				return response_resource_3_17_0_device_state(request,response,writePos);
				
			} else if ((contain = strstr(request->payload,"3/18/0")) != NULL) {	
				
				return response_resource_3_18_0_error_condition(request,response,writePos);
				
			}else{
				return response_error_OMA_command(response,writePos,5);
			}
		
	}
}


int create_OMA_RD_announcement(char *buf){
	coap_packet_t request;
	int data_size = 0;
	char payload[1];
	buf[1] = 0;
	create_coap_request(&request,MESSAGE_TYPE_NON,PUT,payload);
	data_size = copy_headers((unsigned char *)buf,&request);
	return data_size;
}








#endif



