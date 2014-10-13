/* 
 * File:   discover.c
 * Author: Pablo López Martínez
 *
 * Created on 3 de Mayo de 2012, 10:54
 */

#include "mdns.h"

#define UDP_HDR ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])

#define MAX_PAYLOAD_LEN 256

PROCESS(mDNSDiscover_process, "mDNSDiscover process");
AUTOSTART_PROCESSES(&mDNSDiscover_process);
/*---------------------------------------------------------------------------*/
static struct uip_udp_conn *mdns_conn;
static uip_ipaddr_t ipaddr;

/******************** UDP functions *******************************************/


/*---------------------------------------------------------------------------*/

static void
set_global_address(void)
{
	 uip_ip6addr(&ipaddr, 0x2001, 0x720, 0x1710, 0x11,0,0,0,0x21);
	 //uip_ip6addr(&ipaddr,0x2001, 0x630, 0x13, 0x101, 0x0, 0x0, 0x0, 0x104);
	 uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);
	 
}


/*********************************************************************************/


/************************************** MDNS sourcecode ***************************************/

/*-------------------------------------------------------------------------------------------*/
/* mDNS Announcement
 *
 *
 *-------------------------------------------------------------------------------------------*/


static struct uip_udp_conn *announcement_conn = NULL;

//Packet buffer
static char buf[256];


static void
set_connection_address(uip_ipaddr_t *addr)
{
  uip_ip6addr(addr,0x2001, 0x720, 0x1710, 0x11, 0, 0x0, 0x0, 0x1);
}

static const uip_ipaddr_t resolv_mdns_addr = {
		.u8 = { 
			0xff, 0x02, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0xfb,
			}
		};

static void init_mdns_announcement_connection(void){
		
		uip_ipaddr_t gwaddr;
		
		set_global_address();
		
		set_connection_address(&gwaddr);
		
		announcement_conn = udp_new(&resolv_mdns_addr,UIP_HTONS(MDNS_PORT), NULL);
		
}

void init_mdns_query_connection(void){
	uip_ipaddr_t gwaddr;
		
	set_global_address();
	
	set_connection_address(&gwaddr);
	
	mdns_conn = udp_new(NULL, uip_htons(5353), NULL);
	udp_bind(mdns_conn, uip_htons(MDNS_PORT));
	
}

void response_query(u16_t type, char *name, int name_len){
	int len;
	struct dns_hdr hdr;
	struct dns_question qtion;
	struct rr_entry_srv entry;
	memset(buf,'\0',256);
	vPrintf("Recibida peticion de respuesta\n");
	if(type == DNS_TYPE_SRV){
		//Creating packet header
		len = create_dns_header(buf, &hdr, 1, 1, 0, 0,DNS_FLAG1_RESPONSE,DNS_FLAG2_ERR_NONE);
		len += create_query_header(buf+len, &qtion, "temp\0", 5, "_coap._udp\0", 11, type, DNS_CLASS_IN);
		len += create_auth_SRV(buf+len, &entry, "temperature\0", 12, 1234, DNS_TYPE_SRV, DNS_CLASS_IN, 1000);
		uip_udp_packet_send(mdns_conn, buf, len);
	}

}

static void manage_dns_protocol(void){

	char buff[MAX_PAYLOAD_LEN];
	char* data = uip_appdata + uip_ext_len;
	u16_t datalen = uip_datalen() - uip_ext_len;	
	void (* query_function)(u16_t type,char*,int) = response_query;
	memset(buff,0,MAX_PAYLOAD_LEN);
	
	if (uip_newdata()){
		((char *)data)[datalen] = 0;
		uip_ipaddr_copy(&mdns_conn->ripaddr, &UDP_HDR->srcipaddr);
		mdns_conn->rport = UDP_HDR->srcport;		
		parse_dns_question(buf, query_function);
		memset(&mdns_conn->ripaddr, 0, sizeof(mdns_conn->ripaddr));
		mdns_conn->rport = 0;
	}

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

	//process_post(&mdns_process, PROCESS_EVENT_TIMER, len);
	uip_udp_packet_send(announcement_conn, buf, len);
}


static void mdns_SRV_query(char *name,int name_len,
							char *domain,int domain_len){
	struct dns_hdr hdr;
	struct dns_question qtion;
	struct rr_entry_ptr ptr;
	int len;
	memset(buf,'\0',256);
	memset(&qtion,'\0',sizeof(struct dns_question));
	memset(&hdr,'\0',sizeof(struct dns_hdr));
	memset(&ptr,'\0',sizeof(struct rr_entry_ptr));
	
	//Creating packet header
	len = create_dns_header(buf, &hdr, 1, 0, 0, 0,DNS_FLAG1_RD,DNS_FLAG1_OPCODE_STANDARD);
	
	//Adding Query
	len += create_query_header(buf+len, &qtion, domain, domain_len, name, name_len, DNS_TYPE_SRV, DNS_CLASS_QU);
	
	uip_udp_packet_send(announcement_conn, buf, len);
	
	//parse_dns_question(buf, query_function);
	
	
	
}

/************************************** MDNS sourcecode END ***************************************/



/*---------------------------------------------------------------------------*/


PROCESS_THREAD(mDNSDiscover_process, ev, data)
{
	static struct etimer et;
	static char par_name[5] = "temp\0";
	static char par_service[11] = "_coap._udp\0";
	static char par_target[12] = "temperature\0";
	static char par_txt[128] = "rt=Temperature;gps=Murcia;geo=38.023433@-1.174743;ip=2001:720:1710:11::2\0";
	
	static char ptrname[11] = "_coap._udp\0";
	static char domname[5] = "temp\0";	
	
	static char par_name2[6]= "light\0";
	static char par_target2[7]= "lights\0";
	static char par_txt2[128] = "rt=Light;gps=Murcia;geo=38.023433@-1.174743;ip=2001:720:1710:11::2\0";
		
	PROCESS_BEGIN();
		vUART_printInit();
		vUART_DataInit();

		init_mdns_query_connection();
		init_mdns_announcement_connection();		

		etimer_set(&et, 15*CLOCK_SECOND);
		vPrintf("Init main functions...\n");
		
		while(1){
			PROCESS_YIELD();	
			if(isAssociated() && ev != tcpip_event){
				//Create 2 mote announcements
				mdns_SRV_query(((char *)ptrname),10,((char *)domname),4);
  
			}
			
			if(ev == tcpip_event) {
				vPrintf("Respuesta a query recibida!\n");
				//manage_dns_protocol();
			}
			if(etimer_expired(&et)) { 
			    	etimer_restart(&et);
			}
		}
	PROCESS_END();
}

/*---------------------------------------------------------------------------*/
