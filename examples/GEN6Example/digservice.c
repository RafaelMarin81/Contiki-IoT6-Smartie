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

#include "mdns.h"
#include "coap.h"
#include "ipso-webserv.h"
#include "lightdriver.h"


#define UDP_HDR ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])

PROCESS(lampost_process, "Lampost process");
AUTOSTART_PROCESSES(&lampost_process);


/************************************** configurations ***************************************/

static uip_ipaddr_t ipaddr;

// Device Address
static void
set_global_address(void)
{
	uip_ip6addr(&ipaddr, 0x2001, 0x720, 0x1710, 0x11,0,0,0,0x2);
	uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);
	 
}

// End device connection address
static void
set_connection_address(uip_ipaddr_t *addr)
{
	uip_ip6addr(addr,0x2001, 0x720, 0x1710, 0x11,0,0,0,0x1);
}


/************************************** End configurations ***********************************/




/************************************** IPSO-CoAP Handler ***************************************/

// Packet buffer
#define COAP_PACKET_BUF 256
static char coap_buf[COAP_PACKET_BUF];
// coap connection
static struct uip_udp_conn *coap_conn;

// Configure connection
static void init_coap_connection(void){
	uip_ipaddr_t gwaddr;
		
	set_global_address();
	
	set_connection_address(&gwaddr);
	
	coap_conn = udp_new(NULL, uip_htons(0), NULL);
	udp_bind(coap_conn, uip_htons(MOTE_SERVER_LISTEN_PORT));
	
}

// connection handler
static int ipso_handler(){
	char* data = uip_appdata + uip_ext_len;
	u16_t datalen = uip_datalen() - uip_ext_len;
	int data_size = 0;	
	int ptype = 0;
	int need_confirm = 0;
	
	memset(coap_buf,0,COAP_PACKET_BUF);
	
	
	if (uip_newdata()) {
		((char *)data)[datalen] = 0;
		coap_packet_t response;
		coap_packet_t request;
		
		// Parse packet
		if((ptype = parse_coap_packet(&request,data, &need_confirm))<1){
			memset(&coap_conn->ripaddr, 0, sizeof(coap_conn->ripaddr));
			coap_conn->rport = 0;
			VPRINTF("Malformed CoAP Packet\n");
			return -1;
		}
		// Create packet response
		create_coap_response(&response, MESSAGE_TYPE_ACK, OK_200, NULL, request.id);	
		data_size = copy_headers((unsigned char *)coap_buf,&response);	
		// SET IP&Port to response
		uip_ipaddr_copy(&coap_conn->ripaddr, &UDP_HDR->srcipaddr);
		coap_conn->rport = UDP_HDR->srcport;
		
		// Fill the payload with response by IPSO
		data_size = parse_ipso(&request,(char *)&coap_buf,&data_size); 
		
		VPRINTF("Data size: %d\n",data_size);
			
		//Sending data
		if(need_confirm) {
			VPRINTF("Sending Packet\n",data_size);
			uip_udp_packet_send(coap_conn, coap_buf, data_size);
		}
		
		// Restore server connection to allow data from any node
		memset(&coap_conn->ripaddr, 0, sizeof(coap_conn->ripaddr));
		coap_conn->rport = 0;
	}
	return 0;
}

/************************************** End IPSO-CoAP Handler ***************************************/



PROCESS_THREAD(lampost_process, ev, data)
{
	
		
	PROCESS_BEGIN();
		vUART_printInit();
		vUART_DataInit();
			
		init_coap_connection();

		VPRINTF("Starting services...\n");
		
		while(1){
			PROCESS_YIELD();		
			if(ev == tcpip_event) {
				VPRINTF("Packet received!\n");
				// Manage coap based on ipso profile petition
				ipso_handler();
			}
		}
	PROCESS_END();
}

/*---------------------------------------------------------------------------*/
