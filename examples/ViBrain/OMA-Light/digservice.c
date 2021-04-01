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


//~ #include "mdns.h"
#include "coap.h"
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
	uip_ip6addr(&ipaddr, 0x2001, 0x720, 0x1710, 0x11,0,0,0,0x3);
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


//~ // Packet buffer
//~ #define MDNS_PACKET_BUF 256
//~ static char mdns_buf[MDNS_PACKET_BUF];
//~ // mdns connection
//~ static struct uip_udp_conn *resolv_conn;
//~ 
//~ static const uip_ipaddr_t resolv_mdns_addr = {
		//~ .u8 = { 
			//~ 0xff, 0x02, 0x00, 0x00,
			//~ 0x00, 0x00, 0x00, 0x00,
			//~ 0x00, 0x00, 0x00, 0x00,
			//~ 0x00, 0x00, 0x00, 0xfb,
			//~ }
		//~ };
//~ 
//~ // Configure connection
//~ static void init_mdns_connection(void){
		//~ 
		//~ uip_ipaddr_t gwaddr;
		//~ 
		//~ set_global_address();
		//~ 
		//~ set_connection_address(&gwaddr);
		//~ 
		//~ resolv_conn = udp_new(&resolv_mdns_addr,UIP_HTONS(MDNS_PORT), NULL);		
		//~ udp_bind(resolv_conn, UIP_HTONS(MDNS_PORT));
		//~ 
//~ }
//~ 
//~ // Create announce
//~ static void mdns_SRV_announcement(char *name,int name_len,
		//~ char *service_type,
        //~ int service_len,
		//~ char *target,
		//~ int target_len,
		//~ char *addons,
		//~ int add_len){
	//~ struct dns_hdr hdr;
	//~ struct dns_question qtion;
	//~ struct rr_entry_srv entry;
	//~ struct rr_entry_txt txt;
	//~ int len;
	//~ memset(mdns_buf,'\0',MDNS_PACKET_BUF);
	//~ memset(&hdr,'\0',sizeof(struct dns_hdr));
	//~ memset(&qtion,'\0',sizeof(struct dns_question));
	//~ memset(&entry,'\0',sizeof(struct rr_entry_srv));
	//~ memset(&txt,'\0',sizeof(struct rr_entry_txt));
	//~ 
	//~ //Creating packet header
	//~ len = create_dns_header(mdns_buf, &hdr, 1, 0, 2, 0,DNS_FLAG1_OPCODE_STANDARD,DNS_FLAG1_OPCODE_STANDARD);
	//~ 
	//~ //Adding Query
	//~ len += create_query_header(mdns_buf+len, &qtion, name, name_len, service_type, service_len, DNS_TYPE_ANY, DNS_CLASS_IN);
		//~ 
	//~ //Adding Authoritative data
	//~ len += create_auth_SRV(mdns_buf+len, &entry, target, target_len, 1234, DNS_TYPE_SRV, DNS_CLASS_IN, 1000);
//~ 
	//~ 
	//~ //Adding Authoritative txt data
	//~ len += create_auth_TXT(mdns_buf+len, &txt, addons, add_len, DNS_TYPE_TXT, DNS_CLASS_IN, 1000);
//~ 
	//~ VPRINTF("Sending mDNS");
	//~ 
	//~ uip_udp_packet_send(resolv_conn, mdns_buf, len);
//~ }


/************************************** MDNS sourcecode END ***************************************/



/************************************** IPSO-CoAP Handler ***************************************/

// Packet buffer
#define COAP_PACKET_BUF 500
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
		data_size = parse_OMA(&request,(char *)&coap_buf,&data_size); 
		
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


static char oma_rd_buff[64];
static int oma_rd_size = 0;
static struct uip_udp_conn *oma_coap_conn;
static uip_ipaddr_t destination_announcement;


void set_new_rd_destination(u16_t *newAddr){
	uip_ip6addr(&destination_announcement, newAddr[0], newAddr[1], newAddr[2], newAddr[3],newAddr[4],newAddr[5],newAddr[6],newAddr[7]);
	//~ uip_ds6_addr_add(&destination_announcement, 0, ADDR_MANUAL);
}

uip_ipaddr_t get_rd_ip_destination(){
	return destination_announcement;
}

// Configure connection
static void init_oma_rd_connection(void){
	uip_ipaddr_t gwaddr;
		
	set_global_address();
	
	set_connection_address(&gwaddr);
	set_connection_address(&destination_announcement);
	set_newAddress("2001:720:1710:11:0:0:0:1");
	
	oma_coap_conn = udp_new(NULL, uip_htons(MOTE_SERVER_LISTEN_PORT), NULL);
	
	
	
}
static void create_Global_announcememet(){
	oma_rd_size = create_OMA_RD_announcement(oma_rd_buff);
	// Create announcement for lampost
	uip_udp_packet_sendto(oma_coap_conn, oma_rd_buff, oma_rd_size, &destination_announcement, 5683);
}

PROCESS_THREAD(lampost_process, ev, data)
{
	static struct etimer et;

	
		
	PROCESS_BEGIN();
		vUART_printInit();
		vUART_DataInit();
		
		init_Light();
		set_off();
		
		//~ init_mdns_connection();
		init_oma_rd_connection();
		init_coap_connection();

		etimer_set(&et, 15*CLOCK_SECOND);
		VPRINTF("Starting services...\n");
		
		while(1){
			PROCESS_YIELD();	
			if(isAssociated() && ev != tcpip_event){
				VPRINTF("Associated!");
				create_Global_announcememet();
				
			}
			if(etimer_expired(&et)) { 
			    	etimer_restart(&et);
			}
			if(ev == tcpip_event) {
				VPRINTF("Packet received!\n");
				// Manage coap based on ipso profile petition
				ipso_handler();
			}
		}
	PROCESS_END();
}

/*---------------------------------------------------------------------------*/
