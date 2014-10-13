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

#define MAX_PAYLOAD_LEN 256 // Must be 800

PROCESS(mDNSDiscover_process, "mDNSDiscover process");
AUTOSTART_PROCESSES(&mDNSDiscover_process);
/*---------------------------------------------------------------------------*/
static struct uip_udp_conn *coap_conn;
static uip_ipaddr_t ipaddr;

/******************** UDP functions *******************************************/


/*---------------------------------------------------------------------------*/

static void
set_global_address(void)
{
	 uip_ip6addr(&ipaddr, 0x2001, 0x720, 0x1710, 0x11,0,0,0,0x2);
	 //uip_ip6addr(&ipaddr,0x2001, 0x630, 0x13, 0x101, 0x0, 0x0, 0x0, 0x104);
	 uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);
	 
}


/*********************************************************************************/

/*---------------------------------------------------------------------------*/


/************************************** MDNS sourcecode ***************************************/



/*-------------------------------------------------------------------------------------------*/
/* mDNS Announcement
 *
 *
 *-------------------------------------------------------------------------------------------*/


static struct uip_udp_conn *resolv_conn = NULL;

//Packet buffer
static char buf[256];


static void
set_connection_address(uip_ipaddr_t *addr)
{
  uip_ip6addr(addr,0x2001, 0x720, 0x1710, 0x11,0,0,0,0x1);
  //uip_ip6addr(addr,0x2001, 0x630, 0x13, 0x101, 0, 0x0, 0x0, 0x102);
}

static const uip_ipaddr_t resolv_mdns_addr = {
		.u8 = { 
			0xff, 0x02, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0xfb,
			}
		};

static void init_udp_connection(void){
		
		uip_ipaddr_t gwaddr;
		
		set_global_address();
		
		set_connection_address(&gwaddr);
		
		resolv_conn = udp_new(&resolv_mdns_addr,UIP_HTONS(MDNS_PORT), NULL);		
		udp_bind(resolv_conn, UIP_HTONS(MDNS_PORT));
		
}

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
	memset(buf,'\0',256);
	memset(&hdr,'\0',sizeof(struct dns_hdr));
	memset(&qtion,'\0',sizeof(struct dns_question));
	memset(&entry,'\0',sizeof(struct rr_entry_srv));
	memset(&txt,'\0',sizeof(struct rr_entry_txt));
	
	//Creating packet header
	len = create_dns_header(buf, &hdr, 1, 0, 2, 0,DNS_FLAG1_OPCODE_STANDARD,DNS_FLAG1_OPCODE_STANDARD);
	
	//Adding Query
	len += create_query_header(buf+len, &qtion, name, name_len, service_type, service_len, DNS_TYPE_ANY, DNS_CLASS_IN);
		
	//Adding Authoritative data
	len += create_auth_SRV(buf+len, &entry, target, target_len, 1234, DNS_TYPE_SRV, DNS_CLASS_IN, 1000);

	
	//Adding Authoritative txt data
	len += create_auth_TXT(buf+len, &txt, addons, add_len, DNS_TYPE_TXT, DNS_CLASS_IN, 1000);

	VPRINTF("Sending mDNS");
	//process_post(&mdns_process, PROCESS_EVENT_TIMER, len);
	uip_udp_packet_send(resolv_conn, buf, len);
}


/************************************** MDNS sourcecode END ***************************************/

/*---------------------------------------------------------------------------*/
/*								IPSO-CoAP Services								 */					
/*---------------------------------------------------------------------------*/

int ipso_handler(){
	char buff[MAX_PAYLOAD_LEN];
	char* data = uip_appdata + uip_ext_len;
	u16_t datalen = uip_datalen() - uip_ext_len;
	int data_size = 0;	
	int ptype = 0;
	int need_confirm = 0;
	memset(buff,0,MAX_PAYLOAD_LEN);
	
	
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
		data_size = copy_headers((unsigned char *)buff,&response);	
		// SET IP&Port to response
		uip_ipaddr_copy(&coap_conn->ripaddr, &UDP_HDR->srcipaddr);
		coap_conn->rport = UDP_HDR->srcport;
		
		// Fill the payload with response by IPSO
		data_size = parse_ipso(&request,(char *)&buff,&data_size); 
		
		VPRINTF("Data size: %d\n",data_size);
			
		//Sending data
		if(need_confirm) {
			VPRINTF("Sending Packet\n",data_size);
			uip_udp_packet_send(coap_conn, buff, data_size);
		}
		
		// Restore server connection to allow data from any node
		memset(&coap_conn->ripaddr, 0, sizeof(coap_conn->ripaddr));
		coap_conn->rport = 0;
	}
	return 0;
}

void init_coap_connection(void){
	uip_ipaddr_t gwaddr;
		
	set_global_address();
	
	set_connection_address(&gwaddr);
	
	coap_conn = udp_new(NULL, uip_htons(0), NULL);
	udp_bind(coap_conn, uip_htons(MOTE_SERVER_LISTEN_PORT));
	
}

/*---------------------------------------------------------------------------*/



PROCESS_THREAD(mDNSDiscover_process, ev, data)
{
	static struct etimer et;
	static char par_name[5];
	static char par_service[11];
	static char par_target[12];
	static char par_txt[128] = "rt=Lamppost;gps=Murcia;geo=38.023433@-1.174743;ip=2001:720:1710:11::2\0";

	
		
	PROCESS_BEGIN();
		vUART_printInit();
		vUART_DataInit();
		
		init_Light();
		set_off();
		
		init_udp_connection();
		init_coap_connection();
		memcpy(par_name,"lamppost\0",9);
		memcpy(par_service,"_coap._udp\0",11);
		memcpy(par_target,"lights\0",7);

		etimer_set(&et, 15*CLOCK_SECOND);
		VPRINTF("Starting services...\n");
		
		while(1){
			PROCESS_YIELD();	
			if(isAssociated() && ev != tcpip_event){
				VPRINTF("Associated!");
				//Create 2 mote announcements
				mdns_SRV_announcement(par_name,strlen(par_name),
							par_service,strlen(par_service),
							par_target,strlen(par_target),
							par_txt,strlen(par_txt));
			}
			if(etimer_expired(&et)) { 
			    	etimer_restart(&et);
			}
			if(ev == tcpip_event) {
				VPRINTF("Packet received!\n");
				ipso_handler();
			}
		}
	PROCESS_END();
}

/*---------------------------------------------------------------------------*/
