/*
 * Author: José Luis Hernández Ramos
 * email: jluis.hernandez@um.es
 * 
 * Last Update: 25/06/2013
*/

#define DEBUG 1

#if DEBUG
#define VPRINTF(...) vPrintf(__VA_ARGS__)
#else
#define VPRINTF(...)
#endif


//#include "coap.h"
#include "ipso-webserv.h"
#include "Base64EncodeDecode.h"
#include "md5.h"

#define UDP_HDR ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])

PROCESS(capbac, "capbac");
AUTOSTART_PROCESSES(&capbac);


/************************************** configurations ***************************************/
static uip_ipaddr_t addDes;						 /** Destination Address */
static uip_ipaddr_t ipaddr;
unsigned short tsIP[8] = {0xaaaa, 0, 0, 0, 0, 0, 0, 0x1}; 	/**  Server IP */
unsigned short devIP[8] = {0xaaaa, 0, 0, 0, 0, 0, 0, 0x2}; 	/** Device IP*/
unsigned int TEMPERATURE = 23;
unsigned int INIT_TIME = 1369300359;
static unsigned int tiempo_validez , tiempo_permisos, tiempo_firma, tiempo_auth,
tiempo_validez_medio , tiempo_permisos_medio, tiempo_firma_medio, tiempo_auth_medio;

static void set_global_address(void) {
	//getdate(NULL);
	 uip_ip6addr(&ipaddr,devIP[0],devIP[1],devIP[2],devIP[3],devIP[4],devIP[5],devIP[6],devIP[7]);	 /** Source Address (This Mote) */
 
	uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL); 				/** ADDR_AUTOCONF */

}

// End device connection address
static void set_connection_address(uip_ipaddr_t *addr){
	 uip_ip6addr(addr,tsIP[0],tsIP[1],tsIP[2],tsIP[3],tsIP[4],tsIP[5],tsIP[6],tsIP[7]);			 /** Destination Adress (Tepanom Server) */
}


/************************************** End configurations ***********************************/




/************************************** IPSO-CoAP Handler ***************************************/

// Packet buffer Ver tamaño!!! Ver tamaño del bufer en contiki-conf.h (buffer del sistema)
#define COAP_PACKET_BUF 419
static char coap_buf[COAP_PACKET_BUF];
// coap connection
static struct uip_udp_conn *server_conn;

// Configure connection
static void init_coap_connection(void){
	uip_ipaddr_t gwaddr;
	set_global_address();
	set_connection_address(&gwaddr);
	server_conn = udp_new(NULL, uip_htons(0),NULL);
	udp_bind(server_conn, uip_htons(1234));
}

static int is_valid_capability (capability_token_t *ct) {
	unsigned int current_time = INIT_TIME + clock_seconds();
	//VPRINTF("%d\n", current_time);
	if (current_time<ct->ii || current_time<ct->nb || current_time>=ct->na){
		//VPRINTF("Not valid capability\n");
		return 0;
	}
	//VPRINTF("Valid capability\n");
	return 0;
}

static int is_permitted_action (capability_token_t *ct, coap_packet_t *request) {
	//VPRINTF("Funcion: ispermitted_action\n");
	char *method;
	char *resource = request->option_value_uri;
	switch(request->code){
		case GET:
		    method = "GET";
		    break;
		case POST:
		    method = "POST";
		    break;
		case PUT:
		    method = "PUT";
		    break;
		case DELETE:
		    method = "DELETE";
		    break;
		default:
			return -1;//Message is not a request	
	}
	int rights_length = sizeof(ct->rights)/sizeof(ct->rights[0]);
	int i = 0;
	for (i=0; i<rights_length; i++){
		if(strcmp(ct->rights[i].action, method)==0 && strcmp(ct->rights[i].resource, resource)==0){
			//VPRINTF("Action %s on resource %s is authorized\n", method, resource);
			/* Ahora hay que comprobar las condiciones */
			int flag = ct->rights[i].flag;
			int left_condition = 0;
			int left_type_condition = ct->rights[i].conditions[0].con_type;
			int right_condition = 0;
			int right_type_condition = ct->rights[i].conditions[1].con_type;
			switch(left_type_condition){
				case 5://MENOR QUE en conditional observe
					if (TEMPERATURE < ct->rights[i].conditions[0].con_value)
						left_condition = 1;
					break;
			}
			
			switch(right_type_condition){
				case 6://MAYOR QUE en conditional observe
					if (TEMPERATURE > ct->rights[i].conditions[1].con_value) 
						right_condition = 1;
					break;
			}
			
			if (flag == 0){
				if (left_condition == 1 || right_condition == 1){
					//VPRINTF("Action %s on resource %s is authorized\n", method, resource);
					return 0;
				}
			}
			else{
				if (left_condition == 1 && right_condition == 1){
					//VPRINTF("Action %s on resource %s is authorized\n", method, resource);
					return 0;
				}
			}
		}
	}
	//VPRINTF("Action %s on resource %s is unauthorized\n", method, resource);
	return -1;
}
/*void hexNum(uint32 x)
{
  int k;
  char s[9];
  char h[16];
  h[0] = '0';
  h[1] = '1';
  h[2] = '2';
  h[3] = '3';
  h[4] = '4';
  h[5] = '5';
  h[6] = '6';
  h[7] = '7';
  h[8] = '8';
  h[9] = '9';
  h[10] = 'a';
  h[11] = 'b';
  h[12] = 'c';
  h[13] = 'd';
  h[14] = 'e';
  h[15] = 'f';
  
  for(k=7;k>=0;k--)
    s[7-k] = h[(x>>(k*4))&15];
  s[8] = 0;
  vPrintf("%s",s);
}

void hexPrint(uint32 * a)
{
  int j;
  for(j=0;j<5;j++)
    hexNum(a[j]);
}*/
static int is_valid_signature (capability_token_t *ct, char *payload) {
	char r64 [29];
	memset(r64, 0, 29);
	char s64 [29];
	memset(s64, 0, 29);
	strncpy(r64, ct->si, 28);
	strncpy(s64, (ct->si)+28, 28);
	r64[28] = 0;
	s64[28] = 0;
		
	uint8 r64d [20];
	memset(r64d, 0, 20);
	uint8 s64d [20];
	memset(s64d, 0, 20);
	uint32 rhex [5];
	memset(rhex, 0, 5);
	uint32 shex [5];
	memset(shex, 0, 5);

	Base64Decode(r64, r64d, 29);
	dectohex(r64d,rhex);
	Base64Decode(s64, s64d, 29);
	dectohex(s64d, shex);

	/*char beforesig [147];
	memset(beforesig, 0, 147);
	strncpy(beforesig, payload+1, 146);
	beforesig[146] = 0;
	
	char aftersig [135];
	memset(aftersig, 0, 135);
	strncpy(aftersig, payload+211, 134);
	aftersig[134] = 0;

	char hash [283];
	memset(hash, 0, 283);
	memcpy (hash, beforesig, 147);
	strcat(hash, aftersig);*/
	/* Aquí es donde se calcularía el MD5 de "hash" */
	/* 	MD5_CTX MD;
		unsigned char key [16];
		MD5Init (&MD);
		MD5Update(&MD, hash, 283);
		MD5Final(&MD, key);
	 */
		
	/* Aquí es donde se verificaría la firma. Se asume que la mota ya dispone de la clave pública del issuer: "AlicePublic"*/
	/*	verifySignature(rhex,shex,key,&AlicePublic);*/
	return 0;
}
static int is_subject_authenticated (coap_packet_t *request, capability_token_t *ct) {
	//VPRINTF("%s\n", request->option_value_sig);
	char r64 [29];
	memset(r64, 0, 29);
	char s64 [29];
	memset(s64, 0, 29);
	strncpy(r64, request->option_value_sig, 28);
	strncpy(s64, (request->option_value_sig)+28, 28);
	r64[28] = 0;
	s64[28] = 0;
		
	uint8 r64d [20];
	memset(r64d, 0, 20);
	uint8 s64d [20];
	memset(s64d, 0, 20);
	uint32 rhex [5];
	memset(rhex, 0, 5);
	uint32 shex [5];
	memset(shex, 0, 5);
	
	Base64Decode(r64, r64d, 29);
	dectohex(r64d,rhex);
	Base64Decode(s64, s64d, 29);
	dectohex(s64d, shex);
	
	char rsu64 [29];
	memset(rsu64, 0, 29);
	char ssu64 [29];
	memset(ssu64, 0, 29);
	strncpy(rsu64, ct->su, 28);
	strncpy(ssu64, (ct->su)+28, 28);
	rsu64[28] = 0;
	ssu64[28] = 0;
	
	uint8 rsu64d [20];
	memset(rsu64d, 0, 20);
	uint8 ssu64d [20];
	memset(ssu64d, 0, 20);
	uint32 rsuhex [5];
	memset(rsuhex, 0, 5);
	uint32 ssuhex [5];
	memset(ssuhex, 0, 5);
	
	Base64Decode(rsu64, rsu64d, 29);
	dectohex(rsu64d,rsuhex);
	Base64Decode(ssu64, ssu64d, 29);
	dectohex(ssu64d, ssuhex);
	
	char *method;
	char resource [11];
	memset(resource, 0, 11);
	strcpy(resource, request->option_value_uri);
	switch(request->code){
		case GET:
		    method = "GET";
		    break;
		case POST:
		    method = "POST";
		    break;
		case PUT:
		    method = "PUT";
		    break;
		case DELETE:
		    method = "DELETE";
		    break;
		default:
			return -1;//Message is not a request	
	}
	//VPRINTF("%s\n", method);
	//VPRINTF("%s\n", resource);
	char hash [16];
	memset(hash, 0, 16);
	strcpy(hash, method);
	strcat (hash, " ");
	strcat(hash, resource);
	//VPRINTF("%s\n", hash);
	/* Aquí es donde se calcularía el MD5 de "hash" */
	/* 	MD5_CTX MD;
		unsigned char key [16];
		MD5Init (&MD);
		MD5Update(&MD, hash, 16);
		MD5Final(&MD, key);
	 */
		
	/* Aquí es donde se verificaría la firma */
	return 0;
}

static int authorized_response(coap_packet_t * request, char * response, int * writePos) {
	sprintf(response+*writePos, "TEMPERATURE = 23");
	set_response_code(response,content);
	return strlen(response);
}

static int unauthorized_response(coap_packet_t * request, char * response, int * writePos) {
	sprintf(response+*writePos, "Usuario no Autorizado");
	set_response_code(response,unauthorized);
	return strlen(response);
}

// connection handler
static int ipso_handler(){
	char* data = uip_appdata + uip_ext_len;
	/* uip_datalen es la longitud de cualquier dato entrante que está disponible actualmente
	 * en el buffer uip_appdata */
	u16_t datalen = uip_datalen() - uip_ext_len;
	//VPRINTF("UIP_LLH_LEN: %d\n", UIP_LLH_LEN);
	//VPRINTF("datalen: %d\n", datalen);
	int data_size = 0;	
	int ptype = 0;
	int need_confirm = 0;
	
	/* Se reservan 256 bytes de tamaño de buffer */
	memset(coap_buf,0,COAP_PACKET_BUF);
	coap_packet_t response;
	coap_packet_t request;
	/* ¿Hay nuevos datos entrantes disponibles? */
	if (uip_newdata()) {
		((char *)data)[datalen] = 0;
		/* Parseamos el paquete. Si hay error... */
		if((ptype = parse_coap_packet(&request,data, &need_confirm))<1){
			memset(&server_conn->ripaddr, 0, sizeof(server_conn->ripaddr));
			server_conn->rport = 0;
			//VPRINTF("Paquete Coap malformado\n");
			return -1;
		}
		
		capability_token_t ct;
		int auth_decision = 1;
		data_size = parse_capability(&request, &ct); 
		/*VPRINTF("ID: %s\n", ct.id);
		VPRINTF("II: %d\n", ct.ii);
		VPRINTF("IS: %s\n", ct.is);
		VPRINTF("SU: %s\n", ct.su);
		VPRINTF("DE: %s\n", ct.de);
		VPRINTF("SI: %s\n", ct.si);
		VPRINTF("NB: %d\n", ct.nb);
		VPRINTF("NA: %d\n", ct.na);*/
		
		
		//tiempo_validez = clock_time();
		if ((is_valid_capability (&ct)) != 0)
			auth_decision = 0;
		//tiempo_validez = clock_time() - tiempo_validez;
		//tiempo_validez_medio += tiempo_validez;
		
		//tiempo_permisos = clock_time();
		if ((is_permitted_action(&ct, &request)) != 0)
			auth_decision = 0;
		//tiempo_permisos = clock_time() - tiempo_permisos;
		//tiempo_permisos_medio += tiempo_permisos;
			
		//tiempo_firma = clock_time();
		if ((is_valid_signature (&ct, request.payload)) !=0)
			auth_decision = 0;
		//tiempo_firma = clock_time() - tiempo_firma;
		//tiempo_firma_medio +=tiempo_firma;
		
		//tiempo_auth = clock_time();
		if ((is_subject_authenticated (&request, &ct)) != 0)
			auth_decision = 0;
		//tiempo_auth = clock_time() - tiempo_auth;
		//tiempo_auth_medio += tiempo_auth;
		
		if (auth_decision == 0){
			create_coap_response(&response, MESSAGE_TYPE_ACK, UNAUTHORIZED_401, NULL, request.id);	
			data_size = copy_headers((unsigned char *)coap_buf,&response);	
			data_size = unauthorized_response(&request,(char *)&coap_buf,&data_size); 
		}
		else {
			create_coap_response(&response, MESSAGE_TYPE_ACK, OK_200, NULL, request.id);	
			data_size = copy_headers((unsigned char *)coap_buf,&response);	
			data_size = authorized_response(&request,(char *)&coap_buf,&data_size); 
		}
		

		// SET IP&Port to response
		uip_ipaddr_copy(&server_conn->ripaddr, &UDP_HDR->srcipaddr);
		server_conn->rport = UDP_HDR->srcport;

		//VPRINTF("Data size: %d\n",data_size);
			
		//Sending data
		if(need_confirm) {
			//VPRINTF("Sending Packet\n",data_size);
			uip_udp_packet_send(server_conn, coap_buf, data_size);
		}
		
		// Restore server connection to allow data from any node
		memset(&server_conn->ripaddr, 0, sizeof(server_conn->ripaddr));
		server_conn->rport = 0;

	}
	return 0;
}


PROCESS_THREAD(capbac, ev, data)
{
	static struct etimer et;
	PROCESS_BEGIN();
	vUART_printInit();
	vUART_DataInit();
	clock_init();
	init_coap_connection();
	etimer_set(&et, 5*CLOCK_SECOND);

    VPRINTF("PAIND:%x\n",SICSLOWPAN_PANID);

	while(1){
		PROCESS_YIELD();		
		if(isAssociated() && ev != tcpip_event){
					//VPRINTF("tiempo_validez = %d\n", tiempo_validez_medio);
			//VPRINTF("tiempo_permisos = %d\n", tiempo_permisos_medio);	
			//VPRINTF("tiempo_firma = %d\n", tiempo_firma_medio);
			//VPRINTF("tiempo_auth = %d\n", tiempo_auth_medio);
			//VPRINTF("tiempo_firma = %d\n", tiempo_firma);
			//VPRINTF("tiempo_firma = %d\n", tiempo_auth);
			VPRINTF("Asociado!\n");
		} else {
            VPRINTF("NO Asociado!\n");
        }
		if(etimer_expired(&et)) { 
		    	etimer_restart(&et);
		}
		if(ev == tcpip_event) {
			VPRINTF("Paquete recibido!\n");
			// Manage coap based on ipso profile petition
			ipso_handler();
		}
	}
PROCESS_END();
}
