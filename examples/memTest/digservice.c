/* 
 * File:   discover.c
 * Author: Pablo López Martínez
 *
 * Created on 3 de Mayo de 2012, 10:54
 * email: p.lopezmartinez@um.es
 */



#include "mdns.h"
#include "coap.h"

#define UDP_HDR ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])

#define MAX_PAYLOAD_LEN 256

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
	 uip_ip6addr(&ipaddr, 0x2001, 0x720, 0x1710, 0x11,0,0,0,0x3);
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
  uip_ip6addr(addr,0x2001, 0x720, 0x1710, 0x11, 0, 0x0, 0x0, 0x1);
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
		uip_udp_packet_send(resolv_conn, buf, len);
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
		uip_ipaddr_copy(&resolv_conn->ripaddr, &UDP_HDR->srcipaddr);
		resolv_conn->rport = UDP_HDR->srcport;		
		parse_dns_question(buf, query_function);
		memset(&resolv_conn->ripaddr, 0, sizeof(resolv_conn->ripaddr));
		resolv_conn->rport = 0;
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
	uip_udp_packet_send(resolv_conn, buf, len);
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
	
	uip_udp_packet_send(resolv_conn, buf, len);
	
	//parse_dns_question(buf, query_function);
	
	
	
}

/************************************** MDNS sourcecode END ***************************************/

int temperature = 27;
int light=1;

int getTemperature(){
	return temperature;
	
}

int getLightStatus(){
	return light;
}


/*---------------------------------------------------------------------------*/
/*								CoAP Services								 */					
/*---------------------------------------------------------------------------*/

int coap_handler(){	
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
		//CoAP
		init_coap_packet(&request,data);
		vPrintf("\n\n\nParsing msg...\n");
		if((ptype = parse_coap_request(&request, &need_confirm))<1){
			memset(&coap_conn->ripaddr, 0, sizeof(coap_conn->ripaddr));
			coap_conn->rport = 0;
			return -1;
		}
		
		create_coap_response(&response, MESSAGE_TYPE_ACK, OK_200, NULL, request.id);		
		vPrintf("ID request: %x <-----> ID response: %x\n",request.id, response.id);
		data_size = copy_headers(buff,&response);		
		//UDP Contiki
		uip_ipaddr_copy(&coap_conn->ripaddr, &UDP_HDR->srcipaddr);
		coap_conn->rport = UDP_HDR->srcport;
		//Payload		
		switch(ptype){
      case GET:
      case POST:
          vPrintf("GET detected!\n");
          if(strcmp(request.payload,"cl")==0){
            sprintf(buff+data_size, "[{\"name\":\"light1._coap._udp\",\"get\":\"light_status\",\"put\":\"switch\"},{\"name\":\"temp._coap._udp\",\"get\":\"temp_status\",\"put\":\"null\"}]");			
            data_size = data_size + strlen(buff+data_size);
          }else if(strcmp(request.payload,"temp_status")==0){
            sprintf(buff+data_size, "Temperature:%d", getTemperature());
            data_size = data_size + strlen(buff+data_size);
          }else if(strcmp(request.payload,"light_status")==0){
            if(getLightStatus()){				
              sprintf(buff+data_size, "ON");
              data_size = data_size + strlen(buff+data_size);
            }else{
              sprintf(buff+data_size, "OFF");
              data_size = data_size + strlen(buff+data_size);
            }
          }else if(strcmp(request.payload,"")==0){
              printf(buff+data_size, "[{\"name\":\"light1\",\"status\":\"light_status\",\"set\":\"light_set\"},{\"name\":\"mota_lab.temp\",\"status\":\"temp_status\",\"set\":\"temp_set\"}]");     
              data_size = data_size + strlen(buff+data_size);
          }else{
            sprintf(buff+data_size, "No command found.");
            data_size = data_size + strlen(buff+data_size);
          }		
          break;
      case PUT:
        vPrintf("Put detected!\n");
        if(strcmp(request.payload,"switch")==0){
          vPrintf("Switching...\n");
          if(light){
            set_off();
            light=0;        
            sprintf(buff+data_size, "OFF");
            data_size = data_size + strlen(buff+data_size);
          }else{
            set_on();
            light=1;
            sprintf(buff+data_size, "ON");
            data_size = data_size + strlen(buff+data_size);
          }
        }
        break;
      case DELETE:
      default:
        break;
		  
		}
		//Sending data
		if(need_confirm)
		  uip_udp_packet_send(coap_conn, buff, data_size);		
		/* Restore server connection to allow data from any node */
		memset(&coap_conn->ripaddr, 0, sizeof(coap_conn->ripaddr));
		coap_conn->rport = 0;
		return 0;
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
	static char par_txt[128] = "rt=Temperature;gps=Murcia;geo=38.023433@-1.174743;ip=2001:720:1710:11::3\0";
	//static char par_txt[128] = "rt=Temperature;gps=London;geo=51.522867@-0.132028;ip=2001:630:13:101::104\0";
	
	static char ptrname[11] = "_coap._udp\0";
	static char domname[5] = "temp\0";
	
	static char par_name2[7]= "light1\0";
	static char par_target2[7]= "lights\0";
	static char par_txt2[128] = "rt=Light;gps=Murcia;geo=38.023433@-1.174743;ip=2001:720:1710:11::3\0";
	//static char par_txt2[128] = "rt=Light;gps=London;geo=51.522867@-0.132028;ip=2001:630:13:101::104\0";
		
	PROCESS_BEGIN();
		vUART_printInit();
		vUART_DataInit();

		init_Light();
		set_on();
		
		init_udp_connection();
		init_coap_connection();
		memcpy(par_name,"temp\0",5);
		memcpy(par_service,"_coap._udp\0",11);
		memcpy(par_target,"temperature\0",12);

		etimer_set(&et, 15*CLOCK_SECOND);
		vPrintf("Init main functions...\n");
		
		while(1){
			PROCESS_YIELD();	
			if(isAssociated() && ev != tcpip_event){
				//Create 2 mote announcements
				/*mdns_SRV_announcement(par_name,strlen(par_name),
							par_service,strlen(par_service),
							par_target,strlen(par_target),
							par_txt,strlen(par_txt));*/
							
				mdns_SRV_announcement(par_name2,strlen(par_name2),
				              par_service,strlen(par_service),
				              par_target2,strlen(par_target2),
				              par_txt2,strlen(par_txt2));
				//mdns_SRV_query(((char *)ptrname),10,((char *)domname),4);
  
			}
			if(etimer_expired(&et)) { 
			    	etimer_restart(&et);
			}
			if(ev == tcpip_event) {
				vPrintf("Packet received!\n");
				//if(UDP_HDR->srcport == UIP_HTONS(5353)){
					//TODO: receive dns queries
					//manage_dns_protocol();					
				//}else{
					//Receive and send a response
					coap_handler();
				//}
			}
		}
	PROCESS_END();
}

/*---------------------------------------------------------------------------*/
