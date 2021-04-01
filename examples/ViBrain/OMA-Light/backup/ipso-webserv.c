

/*
 * Author: David Fernandez Ros, Pablo Lopez Martinez
 * email: david.f.r@um.es, p.lopezmartinez@um.es
 * 
 * Last Update: 17/1/2013
*/

#define DEBUG 0

#if DEBUG
#define VPRINTF(...) vPrintf(__VA_ARGS__)
#else
#define VPRINTF(...)
#endif

#include "ipso-webserv.h"
#include "lightdriver.h"
#include "coap.h"

static void set_response_code(char * response, response_code cod) {
	response[1] = cod;
}


////////// Error messages  //////////
static int response_error_ipso_command(char * response, int * writePos, int msg){
	switch(msg){
		case 1:
			sprintf(response+*writePos, "Unknown command");
			set_response_code(response,badrequest);
			break;
		case 2:
			sprintf(response+*writePos, "Incomplete IPSO command (Parameters?)");
			set_response_code(response,preconditionfailed);
			break;
		case 3:
			sprintf(response+*writePos, "Unimplemented IPSO command");
			set_response_code(response,notimplemented);
			break;
		case 4:
			sprintf(response+*writePos, "Invalid IPSO command");
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
		sprintf(response+*writePos, "</dev/mdl>;if=\"rp\",</dev/ser>;if=\"rp\",</dev/keepalive>;if=\"s\",</msg/status>;if=\"rp\",</cfg/stack/net/ip>;if=\"p\",</cfg/stack/net/port>;if=\"p\",</loc/gps>;if=\"p\",</lt/on>;if=\"a\",</lt/dim>;if=\"a\"");
		set_response_code(response,content);
		return strlen(response);
	}
	return response_error_ipso_command(response,writePos,4);
}

///////////dev   //////////
static int response_dev_mdl(coap_packet_t * request, char * response, int * writePos) {
	if (request->code==GET) {
		sprintf(response+*writePos, "JN5139");
		set_response_code(response,content);
		return strlen(response);
	}
	return response_error_ipso_command(response,writePos,4);
}

static int response_dev_ser(coap_packet_t * request, char * response, int * writePos) {
	if (request->code==GET) {
		sprintf(response+*writePos, "1234-1234-1234-1234");
		set_response_code(response,content);
		return strlen(response);
	}
	return response_error_ipso_command(response,writePos,4);
}

static int response_dev_uptime(coap_packet_t * request, char * response, int * writePos) {
	if (request->code==GET) {
		response_error_ipso_command(response,writePos,3);
	}
	return response_error_ipso_command(response,writePos,4);
}

static int response_dev_keepalive(coap_packet_t * request, char * response, int * writePos) {
	if (request->code==GET) {
		sprintf(response+*writePos, "Hello");
		set_response_code(response,content);
		return strlen(response);
	}
	return response_error_ipso_command(response,writePos,4);
}

static int response_dev_reboot(coap_packet_t * request, char * response,int * writePos) {
	if (request->code==GET) {
		    response_error_ipso_command(response,writePos,3);
	} else if (request->code==PUT) {
			response_error_ipso_command(response,writePos,3);
	}else if (request->code==POST) {
			response_error_ipso_command(response,writePos,3);
	}
	return response_error_ipso_command(response,writePos,4);

}
////////// msg   //////////
static int response_msg_status(coap_packet_t * request, char * response, int * writePos) {
	if (request->code==GET) {
		if(get_light_status()){				
			sprintf(response+*writePos, "Smart Driver (JN5139) - Status: ON - DIM: %d - Serial Number: 1234-1234-1234-1234",get_luminity());
		set_response_code(response,content);
		}else{
			sprintf(response+*writePos, "Smart Driver (JN5139) - Status: OFF - DIM: %d - Serial Number: 1234-1234-1234-1234",get_luminity());
		set_response_code(response,content);
		}
		return strlen(response);
	}
	return response_error_ipso_command(response,writePos,4);
}

static int response_msg_alarms(coap_packet_t * request, char * response, int * writePos) {
	if (request->code==GET) {
		response_error_ipso_command(response,writePos,3);
	}else if (request->code==PUT) {
		response_error_ipso_command(response,writePos,3);
	}
	return response_error_ipso_command(response,writePos,4);
}

////////// cfg   //////////
static int response_cfg_stack_net_ip(coap_packet_t * request, char * response,int * writePos) {
	if (request->code==GET) {
			sprintf(response+*writePos, "2001:720:1710:11::2");
		set_response_code(response,content);
			return strlen(response);
	} else if (request->code==PUT) {
			response_error_ipso_command(response,writePos,3);
	}
	return response_error_ipso_command(response,writePos,4);
}
static int response_cfg_stack_net_port(coap_packet_t * request, char * response,int * writePos) {
	if (request->code==GET) {
			sprintf(response+*writePos, "1234");
		set_response_code(response,content);
			return strlen(response);
	} else if (request->code==PUT) {
			response_error_ipso_command(response,writePos,3);
	}
	return response_error_ipso_command(response,writePos,4);
}
static int response_cfg_stack_net_auto(coap_packet_t * request, char * response,int * writePos) {
	if (request->code==GET) {
			response_error_ipso_command(response,writePos,3);
	} else if (request->code==PUT) {
			response_error_ipso_command(response,writePos,3);
	}
	return response_error_ipso_command(response,writePos,4);
}
static int response_cfg_stack_mac_addr(coap_packet_t * request, char * response,int * writePos) {
	if (request->code==GET) {
			response_error_ipso_command(response,writePos,3);
	} else if (request->code==PUT) {
			response_error_ipso_command(response,writePos,3);
	}
	return response_error_ipso_command(response,writePos,4);
}
static int response_cfg_stack_mac_pan(coap_packet_t * request, char * response,int * writePos) {
	if (request->code==GET) {
			response_error_ipso_command(response,writePos,3);
	} else if (request->code==PUT) {
			response_error_ipso_command(response,writePos,3);
	}
	return response_error_ipso_command(response,writePos,4);
}
static int response_cfg_stack_mac_chan(coap_packet_t * request, char * response,int * writePos) {
	if (request->code==GET) {
			response_error_ipso_command(response,writePos,3);
	} else if (request->code==PUT) {
			response_error_ipso_command(response,writePos,3);
	}
	return response_error_ipso_command(response,writePos,4);
}
static int response_cfg_stack_mac_sec(coap_packet_t * request, char * response,int * writePos) {
	if (request->code==GET) {
			response_error_ipso_command(response,writePos,3);
	} else if (request->code==PUT) {
			response_error_ipso_command(response,writePos,3);
	}
	return response_error_ipso_command(response,writePos,4);
}
static int response_cfg_stack_mac_sec_key(coap_packet_t * request, char * response,int * writePos) {
	if (request->code==GET) {
			response_error_ipso_command(response,writePos,3);
	} else if (request->code==PUT) {
			response_error_ipso_command(response,writePos,3);
	}
	return response_error_ipso_command(response,writePos,4);
}
static int response_cfg_stack_mac_sec_iv(coap_packet_t * request, char * response,int * writePos) {
	if (request->code==GET) {
			response_error_ipso_command(response,writePos,3);
	} else if (request->code==PUT) {
			response_error_ipso_command(response,writePos,3);
	}
	return response_error_ipso_command(response,writePos,4);
}

////////// cfg  //////////
static int response_loc_gps(coap_packet_t * request, char * response,int * writePos) {
	if (request->code==GET) {
			sprintf(response+*writePos, "38.53463 -1.1434556");
		set_response_code(response,content);
			return strlen(response);
	} else if (request->code==PUT) {
			response_error_ipso_command(response,writePos,3);
	}
	return response_error_ipso_command(response,writePos,4);
}

////////// lt  //////////
static int response_lt_on_all(coap_packet_t * request, char * response,int * writePos) {
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
		if ((contain = strstr(request->payload,"0")) != NULL) {
			set_off();
		} else if ((contain = strstr(request->payload,"OFF")) != NULL) {
			set_off();
		} else if ((contain = strstr(request->payload,"1")) != NULL) {
			set_on();
		} else if ((contain = strstr(request->payload,"ON")) != NULL){
			set_on();
		} else {
			return response_error_ipso_command(response,writePos,2); 
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
	return response_error_ipso_command(response,writePos,4);
}

static int response_lt_dim_all(coap_packet_t * request, char * response,int * writePos) {
	if (request->code==GET) {
		sprintf(response+*writePos, "%d",get_luminity());
		set_response_code(response,content);
		return strlen(response);
	} else if (request->code==PUT) {
		sprintf(response+*writePos, "%d",set_luminity(atoi(request->payload+8)));
		set_response_code(response,changed);
		return strlen(response);
	} else if (request->code==POST) {
		sprintf(response+*writePos, "%d",set_luminity(10));
		set_response_code(response,changed);
		return strlen(response);
	}
	return response_error_ipso_command(response,writePos,4);
}

/*static int response_lt_on_light0(coap_packet_t * request, char * response,int * writePos) {
	char * contain;
	if (request->code==GET) {
		if(get_light_status()){				
			sprintf(response+*writePos, "ON");
		}else{
			sprintf(response+*writePos, "OFF");
		}
		return strlen(response);
	} else if (request->code==PUT) {
		if ((contain = strstr(request->payload,"0")) != NULL) {
			set_off();
		} else if ((contain = strstr(request->payload,"OFF")) != NULL) {
			set_off();
		} else if ((contain = strstr(request->payload,"1")) != NULL) {
			set_on();
		} else if ((contain = strstr(request->payload,"ON")) != NULL){
			set_on();
		} else {
			return response_error_ipso_command(response,writePos,2); 
		}
		if(get_light_status()){				
			sprintf(response+*writePos, "ON");
		}else{
			sprintf(response+*writePos, "OFF");
		}
		return strlen(response);
	} else if (request->code==POST) {
		if(switch_light()){				
			sprintf(response+*writePos, "ON");
		}else{
			sprintf(response+*writePos, "OFF");
		}
		return strlen(response);
	}
	return response_error_ipso_command(response,writePos,4);
}*/

/*static int response_lt_dim_light0(coap_packet_t * request, char * response,int * writePos) {
	if (request->code==GET) {
			sprintf(response+*writePos, "%d",get_luminity());
			return strlen(response);
	} else if (request->code==PUT) {
			sprintf(response+*writePos, "%d",set_luminity(atoi(request->payload+8)));
			return strlen(response);
	} else if (request->code==POST) {
			sprintf(response+*writePos, "%d",set_luminity(10));
			return strlen(response);
	}
	return response_error_ipso_command(response,writePos,4);
}*/

////////// /gpio   //////////
static int response_gpio_cross_0_status(coap_packet_t * request, char * response, int * writePos) {
	if (request->code==GET) {
		response_error_ipso_command(response,writePos,3);
	}
	return response_error_ipso_command(response,writePos,4);
}

static int response_gpio_cross_0_fr(coap_packet_t * request, char * response, int * writePos) {
	if (request->code==GET) {
		response_error_ipso_command(response,writePos,3);
	}
	return response_error_ipso_command(response,writePos,4);
}

static int response_gpio_cross_0_ct(coap_packet_t * request, char * response, int * writePos) {
	if (request->code==GET) {
		response_error_ipso_command(response,writePos,3);
	}
	return response_error_ipso_command(response,writePos,4);
}

static int response_gpio_cross_0_rst(coap_packet_t * request, char * response,int * writePos) {
	if (request->code==GET) {
		response_error_ipso_command(response,writePos,3);
	} else if (request->code==PUT) {
		response_error_ipso_command(response,writePos,3);
	} else if (request->code==POST) {
		response_error_ipso_command(response,writePos,3);
	}
	return response_error_ipso_command(response,writePos,4);
}

// Parser
int parse_ipso(coap_packet_t * request, char * response, int * writePos) {
	
	char * contain;
	
	// If full payload is resource discovery /.well-known/core   // Length its take in account when the command only accepts get in order to avoid more cost to compare
	if (strcmp(request->payload,"/.well-known/core")==0) {
		return response_discover_services(request,response,writePos);
		
	// If another type of message
	} else {
		// If contains /dev
		if ((contain = strstr(request->payload,"/dev")) != NULL) {
			// If contains /dev/mdl GET
			if ((contain = strstr(request->payload,"/dev/mdl")) != NULL) {
				return response_dev_mdl(request,response,writePos);
			// If contains /dev/ser GET
			} else if ((contain = strstr(request->payload,"/dev/ser")) != NULL) {
				return response_dev_ser(request,response,writePos);
			// If contains /dev/uptime GET
			} else if ((contain = strstr(request->payload,"/dev/uptime")) != NULL) {
				return response_dev_uptime(request,response,writePos);
			// If contains /dev/keepalive GET
			} else if ((contain = strstr(request->payload,"/dev/keepalive")) != NULL) {
				return response_dev_keepalive(request,response,writePos);
			// If contains /dev/reboot GET or PUT
			} else if ((contain = strstr(request->payload,"/dev/reboot")) != NULL) {
				return response_dev_reboot(request, response,writePos);
			}
			//Add here new functionalities
		// If contains /msg
		} else if ((contain = strstr(request->payload,"/msg")) != NULL) {
			// If contains /msg/status GET
			if ((contain = strstr(request->payload,"/msg/status")) != NULL) {
				return response_msg_status(request,response,writePos);
			// If contains /msg/alarms GET
			} else if ((contain = strstr(request->payload,"/msg/alarms")) != NULL) {
				return response_msg_alarms(request,response,writePos);
			}
		// If contains /cfg
		} else if ((contain = strstr(request->payload,"/cfg")) != NULL) {
			// If contains /cfg/stack/net/ip GET or PUT
			if ((contain = strstr(request->payload,"/cfg/stack/net/ip")) != NULL) {
				return response_cfg_stack_net_ip(request,response,writePos);
			// If contains /cfg/stack/net/port GET or PUT
			} else if ((contain = strstr(request->payload,"/cfg/stack/net/port")) != NULL) {
				return response_cfg_stack_net_port(request,response,writePos);
			// If contains /cfg/stack/net/auto GET or PUT
			} else if ((contain = strstr(request->payload,"/cfg/stack/net/auto")) != NULL) {
				return response_cfg_stack_net_auto(request,response,writePos);
			//// If contains /cfg/stack/mac/addr GET or PUT
			} else if ((contain = strstr(request->payload,"/cfg/stack/mac/addr")) != NULL) {
				return response_cfg_stack_mac_addr(request,response,writePos);
			//// If contains /cfg/stack/mac/pan GET or PUT
			} else if ((contain = strstr(request->payload,"/cfg/stack/mac/pan")) != NULL) {
				return response_cfg_stack_mac_pan(request,response,writePos);
			//// If contains /cfg/stack/mac/chan GET or PUT
			} else if ((contain = strstr(request->payload,"/cfg/stack/mac/chan")) != NULL) {
				return response_cfg_stack_mac_chan(request,response,writePos);
			//// If contains /cfg/stack/mac/sec GET or PUT
			} else if ((contain = strstr(request->payload,"/cfg/stack/mac/sec")) != NULL) {
				return response_cfg_stack_mac_sec(request,response,writePos);
			//// If contains /cfg/stack/mac/sec/key GET or PUT
			} else if ((contain = strstr(request->payload,"/cfg/stack/mac/sec/key")) != NULL) {
				return response_cfg_stack_mac_sec_key(request,response,writePos);
			//// If contains /cfg/stack/mac/sec/iv GET or PUT
			} else if ((contain = strstr(request->payload,"/cfg/stack/mac/sec/iv")) != NULL) {
				return response_cfg_stack_mac_sec_iv(request,response,writePos);
			}
		
		// If contains /loc
		} else if ((contain = strstr(request->payload,"/loc")) != NULL) {
			// If contains /loc/gps GET or PUT
			if ((contain = strstr(request->payload,"/loc/gps")) != NULL) {
				return response_loc_gps(request,response,writePos);
			}
		
		// If contains /lt
		} else if ((contain = strstr(request->payload,"/lt")) != NULL) {
			// If contains /lt/on
			if ((contain = strstr(request->payload,"/lt/on")) != NULL) {
				return response_lt_on_all(request,response,writePos);
			// If contains /lt/dim
			} else if ((contain = strstr(request->payload,"/lt/dim")) != NULL) {
				return response_lt_dim_all(request,response,writePos);
			// If contains /lt/light0/on (specific resource)
			} else if ((contain = strstr(request->payload,"/lt/light0/on")) != NULL) {
				return response_lt_on_all(request,response,writePos);
			// If contains /lt/light0/dim (specific resource)
			} else if ((contain = strstr(request->payload,"/lt/light0/dim")) != NULL) {
				return response_lt_dim_all(request,response,writePos);
			}
		
		// If contains /gpio
		} else if ((contain = strstr(request->payload,"/gpio")) != NULL) {
			// If contains /gpio/cross/0/status GET
			if ((contain = strstr(request->payload,"/gpio/cross/0/status")) != NULL) {
				return response_gpio_cross_0_status(request,response,writePos);
			// If contains /gpio/cross/0/fr GET
			} else if ((contain = strstr(request->payload,"/gpio/cross/0/fr")) != NULL) {
				return response_gpio_cross_0_fr(request,response,writePos);
			// If contains /gpio/cross/0/ct GET
			} else if ((contain = strstr(request->payload,"/gpio/cross/0/ct")) != NULL) {
				return response_gpio_cross_0_ct(request,response,writePos);
			// If contains /gpio/cross/0/rst GET
			} else if ((contain = strstr(request->payload,"/gpio/cross/0/rst")) != NULL) {
				return response_gpio_cross_0_rst(request,response,writePos);
			}
		}
	}
	
	// Commands that are not implemented or are unknown by parser
	return response_error_ipso_command(response,writePos,1);
}
