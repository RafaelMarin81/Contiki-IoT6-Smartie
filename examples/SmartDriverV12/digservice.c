/*
 * Author: David Fernandez Ros, Pablo Lopez Martinez
 * email: david.f.r@um.es, p.lopezmartinez@um.es
 * 
 * Last Update: 17/1/2013
*/

#define DEBUG 1

#if DEBUG
#define PRINTF(...) vPrintf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#include "mdns.h"
#include "lightdriver.h"
#include "coap.h"


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


/************************************** MDNS sourcecode ***************************************/


// Packet buffer
#define MDNS_PACKET_BUF 256
static char mdns_buf[MDNS_PACKET_BUF];
// mdns connection
static struct uip_udp_conn *resolv_conn;

static const uip_ipaddr_t resolv_mdns_addr = {
		.u8 = { 
			0xff, 0x02, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0xfb,
			}
		};

// Configure connection
static void init_mdns_connection(void){
		
		uip_ipaddr_t gwaddr;
		
		set_global_address();
		
		set_connection_address(&gwaddr);
		
		resolv_conn = udp_new(&resolv_mdns_addr,UIP_HTONS(MDNS_PORT), NULL);		
		udp_bind(resolv_conn, UIP_HTONS(MDNS_PORT));
		
}

// Create announce
static void mdns_SRV_announcement(char *name,int name_len,
		char *service_type,
        int service_len,
		char *target,
		int target_len,
		char *addons,
		int add_len){
	struct dns_hdr hdr;
	struct dns_question qtion;
	struct rr_entry_srv entry;
	struct rr_entry_txt txt;
	int len;
	memset(mdns_buf,'\0',MDNS_PACKET_BUF);
	memset(&hdr,'\0',sizeof(struct dns_hdr));
	memset(&qtion,'\0',sizeof(struct dns_question));
	memset(&entry,'\0',sizeof(struct rr_entry_srv));
	memset(&txt,'\0',sizeof(struct rr_entry_txt));
	
	//Creating packet header
	len = create_dns_header(mdns_buf, &hdr, 1, 0, 2, 0,DNS_FLAG1_OPCODE_STANDARD,DNS_FLAG1_OPCODE_STANDARD);
	
	//Adding Query
	len += create_query_header(mdns_buf+len, &qtion, name, name_len, service_type, service_len, DNS_TYPE_ANY, DNS_CLASS_IN);
		
	//Adding Authoritative data
	len += create_auth_SRV(mdns_buf+len, &entry, target, target_len, DEFAULT_PORT, DNS_TYPE_SRV, DNS_CLASS_IN, 1000);

	
	//Adding Authoritative txt data
	len += create_auth_TXT(mdns_buf+len, &txt, addons, add_len, DNS_TYPE_TXT, DNS_CLASS_IN, 1000);

	PRINTF("Sending mDNS Announcement \n");
	
	uip_udp_packet_send(resolv_conn, mdns_buf, len);
}


/************************************** MDNS sourcecode END ***************************************/



/************************************** IPSO-CoAP Handler ***************************************/

// Packet buffer
#define COAP_PACKET_BUF 1100
static char coap_buf[COAP_PACKET_BUF];
// coap connection
static struct uip_udp_conn *coap_conn;

// Configure connection
static void init_coap_connection(void){
	uip_ipaddr_t gwaddr;
		
	set_global_address();
	
	set_connection_address(&gwaddr);
	
	coap_conn = udp_new(NULL, uip_htons(0), NULL);
	udp_bind(coap_conn, uip_htons(DEFAULT_PORT));
	
}

// connection handler
static int ipso_handler(){
	char* data = uip_appdata + uip_ext_len;
	u16_t datalen = uip_datalen() - uip_ext_len;
	coap_packet_t request;
	
	memset(coap_buf,0,COAP_PACKET_BUF);
	memset(&request,0,sizeof(coap_packet_t));
	
	if (uip_newdata()) {
		((char *)data)[datalen] = 0;
		
		strcpy(coap_buf,(char*)data);										//Creating middle buffer
				
		// Parse packet
		if(coap12_parse_request(&request,coap_buf, datalen)<0){				//Extract from coap_buf the request data and creates the request packet.
			memset(&coap_conn->ripaddr, 0, sizeof(coap_conn->ripaddr));
			coap_conn->rport = 0;
			PRINTF("Malformed CoAP Packet\n");
			return -1;
		}
		
		memset(coap_buf, 0, COAP_PACKET_BUF);
		coap12_create_response(coap_buf, &request);
		
		// SET IP&Port to response
		uip_ipaddr_copy(&coap_conn->ripaddr, &UDP_HDR->srcipaddr);
		coap_conn->rport = UDP_HDR->srcport;
			
		PRINTF("Sending Packet, size = %d\n",strlen(coap_buf));
		uip_udp_packet_send(coap_conn, coap_buf, strlen(coap_buf));

		
		// Restore server connection to allow data from any node
		memset(&coap_conn->ripaddr, 0, sizeof(coap_conn->ripaddr));
		coap_conn->rport = 0;
	}
	return 0;
}

/************************************** End IPSO-CoAP Handler ***************************************/



PROCESS_THREAD(lampost_process, ev, data)
{
	static struct etimer et;
	static char par_name[9] = "lamppost\0";
	static char par_service[11] = "_coap._udp\0";
	static char par_target[7] = "lights\0";
	static char par_txt[70] = "rt=Lamppost;gps=Murcia;geo=38.023433@-1.174743;ip=2001:720:1710:11::2\0";

	
		
	PROCESS_BEGIN();
		vUART_printInit();
		vUART_DataInit();
		
		init_Light();
		set_off();
		
		init_mdns_connection();
		init_coap_connection();

		etimer_set(&et, 15*CLOCK_SECOND);
		PRINTF("Starting services...\n");
		
		while(1){
			PROCESS_YIELD();	
			if(isAssociated() && ev != tcpip_event){
				PRINTF("Associated!");
				// Create announcement for lampost
				mdns_SRV_announcement(par_name,strlen(par_name),
							par_service,strlen(par_service),
							par_target,strlen(par_target),
							par_txt,strlen(par_txt));
			}
			if(etimer_expired(&et)) { 
			    	etimer_restart(&et);
			}
			if(ev == tcpip_event) {
				PRINTF("Packet received!\n");
				// Manage coap based on ipso profile petition
				ipso_handler();
			}
		}
	PROCESS_END();
}

/*---------------------------------------------------------------------------*/
