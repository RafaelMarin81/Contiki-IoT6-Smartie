#define DEBUG 1

#if DEBUG
#define VPRINTF(...) vPrintf(__VA_ARGS__)
#else
#define VPRINTF(...)
#endif

#include "ipso-webserv.h"
#include "coap.h"
#include "jsonparse.h"

void set_response_code(char * response, response_code cod) {
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

int parse_capability (coap_packet_t * request, capability_token_t *ct) {
	int type;
	int current_action = -1;
	int current_condition = -1;
	struct jsonparse_state state;
	jsonparse_setup (&state, request->payload, 354);
	//Hay que saltarse el espacio en blanco del payload (según draft 18 de CoAP)
	type = jsonparse_next(&state);
	
	while((type = jsonparse_next(&state)) != 0) {
		if(type == JSON_TYPE_PAIR_NAME) {
			if(jsonparse_strcmp_value(&state, "id") == 0) {
				type = jsonparse_next(&state);
				type = jsonparse_next(&state);
				static char id [17];
				ct->id = jsonparse_get_value_as_string(&state, id);
			}
			if(jsonparse_strcmp_value(&state, "ii") == 0) {
				type = jsonparse_next(&state);
				type = jsonparse_next(&state);
				ct->ii = jsonparse_get_value_as_long(&state);
			}
			if(jsonparse_strcmp_value(&state, "is") == 0) {
				type = jsonparse_next(&state);
				type = jsonparse_next(&state);
				static char is [11];
				ct->is = jsonparse_get_value_as_string(&state, is);
			}
			if(jsonparse_strcmp_value(&state, "su") == 0) {
				type = jsonparse_next(&state);
				type = jsonparse_next(&state);
				static char su [57];
				ct->su = jsonparse_get_value_as_string(&state, su);
			}
			if(jsonparse_strcmp_value(&state, "de") == 0) {
				type = jsonparse_next(&state);
				type = jsonparse_next(&state);
				static char de [16];
				ct->de = jsonparse_get_value_as_string(&state, de);
			}
			if(jsonparse_strcmp_value(&state, "si") == 0) {
				type = jsonparse_next(&state);
				type = jsonparse_next(&state);
				static char si [57];
				ct->si = jsonparse_get_value_as_string(&state, si);
			}
			if(jsonparse_strcmp_value(&state, "ac") == 0) {
				current_action++;
				type = jsonparse_next(&state);
				type = jsonparse_next(&state);
				static char ac [4];
				ct->rights[current_action].action = jsonparse_get_value_as_string(&state, ac);
			}
			if(jsonparse_strcmp_value(&state, "re") == 0) {
				type = jsonparse_next(&state);
				type = jsonparse_next(&state);
				static char re [12];
				ct->rights[current_action].resource = jsonparse_get_value_as_string(&state, re);
			}
			if(jsonparse_strcmp_value(&state, "f") == 0) {
				type = jsonparse_next(&state);
				type = jsonparse_next(&state);
				ct->rights[current_action].flag = jsonparse_get_value_as_int(&state);
			}
			if(jsonparse_strcmp_value(&state, "t") == 0) {
				current_condition++;
				type = jsonparse_next(&state);
				type = jsonparse_next(&state);
				ct->rights[current_action].conditions[current_condition].con_type = jsonparse_get_value_as_int(&state);
			}
			if(jsonparse_strcmp_value(&state, "v") == 0) {
				type = jsonparse_next(&state);
				type = jsonparse_next(&state);
				ct->rights[current_action].conditions[current_condition].con_value = jsonparse_get_value_as_int(&state);
			}
			if(jsonparse_strcmp_value(&state, "u") == 0) {
				type = jsonparse_next(&state);
				type = jsonparse_next(&state);
				static char unit [4];
				ct->rights[current_action].conditions[current_condition].con_unit = jsonparse_get_value_as_string(&state, unit);
			}
			if(jsonparse_strcmp_value(&state, "nb") == 0) {
				type = jsonparse_next(&state);
				type = jsonparse_next(&state);
				ct->nb = jsonparse_get_value_as_long(&state);
			}
			if(jsonparse_strcmp_value(&state, "na") == 0) {
				type = jsonparse_next(&state);
				type = jsonparse_next(&state);
				ct->na = jsonparse_get_value_as_long(&state);
			}
		}
	}
	return 0;
}
