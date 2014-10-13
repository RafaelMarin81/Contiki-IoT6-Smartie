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

#include "sht-sensor.h"
#include <AppHardwareApi.h>
#include <lightlevel-sensor.h>
#include "mdns.h"
#include "er-coap-12.h"


#define UDP_HDR ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])

PROCESS(digmote_obix_coap, "CoAP server");
PROCESS(webServ, "HTTP server");
AUTOSTART_PROCESSES(&digmote_obix_coap, &webServ);


/************************************** configurations ***************************************/

static uip_ipaddr_t ipaddr;

// Device Address
static void
set_global_address(void)
{
	uip_ip6addr(&ipaddr, 0x2001, 0x720, 0x1710, 0x11,0,0,0,0x55);	
	uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);
	 
}

static void
set_auto_global_address(void)
{
	uip_ipaddr_t loc_fipaddr;

	uip_create_linklocal_prefix(&loc_fipaddr);
	uip_ds6_addr_add(&loc_fipaddr, 0, ADDR_AUTOCONF);
	 
}

// End device connection address
static void
set_connection_address(uip_ipaddr_t *addr)
{
	uip_ip6addr(addr,0x2001, 0x720, 0x1710, 0x11,0,0,0,0x1);
}

static void
set_digcovery_address(uip_ipaddr_t *addr)
{
	uip_ip6addr(addr,0x2001, 0x720, 0x1710, 0x10,0,0,0,0x1000);
}
/************************************** End configurations ***********************************/

/************************************** STIS sourcecode ***************************************/
// Packet buffer
#define STIS_PACKET_BUF 125
 
static char stis_buf[STIS_PACKET_BUF];
// coap connection
static struct uip_udp_conn *stis_conn;

// Configure connection
static void init_STIS_connection(void){
	//~ uip_ipaddr_t gwaddr;
		
	set_global_address();
	//~ set_auto_global_address();
	//~ set_connection_address(&gwaddr);
	
	stis_conn = udp_new(NULL, uip_htons(0), NULL);
	udp_bind(stis_conn, uip_htons(DEFAULT_PORT));
	
}

// Create announce
static void send_STIS_announcement(){
	
	stis_buf[0] = 0x41;
	stis_buf[1] = 0x03;
	stis_buf[2] = 0x6a;
	stis_buf[3] = 0x27;
	
	
	
	uip_udp_packet_send(stis_conn, stis_buf, len);
}


/*********************************** END STIS sourcecode **************************************/

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



  5

/************************************** MDNS sourcecode END ***************************************/

/************************************** IPSO-CoAP Handler ***************************************/

// Packet buffer
#define COAP_PACKET_BUF 1100
static char coap_buf[COAP_PACKET_BUF];
// coap connection
static struct uip_udp_conn *coap_conn;

// Configure connection
static void init_coap_connection(void){
	//~ uip_ipaddr_t gwaddr;
		
	set_global_address();
	//~ set_auto_global_address();
	//~ set_connection_address(&gwaddr);
	
	coap_conn = udp_new(NULL, uip_htons(0), NULL);
	udp_bind(coap_conn, uip_htons(DEFAULT_PORT));
	
}

static void coap_digcovery_announcement(char *query,int q_len){
 
  uip_ipaddr_t dst_addr;
  char coap_req_buf[1100];
  coap_packet_t request;
  int current_len = 0;
  
  coap_init_message(&request, COAP_TYPE_ACK, GET, COAP_PUT);
  coap_set_header_uri_path(&request, "/dig");
  coap_set_header_uri_query(&request, query);
  current_len = coap_serialize_message(&request, coap_req_buf);
  
  set_digcovery_address(&dst_addr);
  uip_udp_packet_sendto(coap_conn, coap_req_buf, current_len, &dst_addr, 5683);
    
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
		/*if(coap12_parse_request(&request,coap_buf, datalen)<0){				//Extract from coap_buf the request data and creates the request packet.
			memset(&coap_conn->ripaddr, 0, sizeof(coap_conn->ripaddr));
			coap_conn->rport = 0;
			PRINTF("Malformed CoAP Packet\n");
			return -1;
		}
		
		memset(coap_buf, 0, COAP_PACKET_BUF);
		coap12_create_response(coap_buf, &request);*/
		
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



PROCESS_THREAD(digmote_obix_coap, ev, data)
{
	static struct etimer et;
	/*static char par_name[10] = "temp\0";
	static char par_service[11] = "_coap._udp\0";
	static char par_target[10] = "sensors\0";
	static char par_txt[70] = "rt=sensors;gps=Murcia;geo=38.023433@-1.174743;ip=2001:720:1710:11::2\0";*/
	
	static char *dig_query = "?ep=temp&proto=_coap._udp&port=5683&d=um.es&lat=31&long=-1&z=Murcia&addr=2001:720:1710:11::55\0"
		
	PROCESS_BEGIN();
		vUART_printInit();
		vUART_DataInit();
		
		bAHI_SetClockRate(0);
		vAHI_HighPowerModuleEnable(TRUE,TRUE);
		bAHI_PhyRadioSetPower(3);
		SHT11_Init();
		
		init_coap_connection();
		init_mdns_connection();

		etimer_set(&et, 15*CLOCK_SECOND);
		PRINTF("Starting services...\n");
		
		while(1){
			PROCESS_YIELD();	
			
			if(isAssociated() && ev != tcpip_event){
				PRINTF("Associated!");
				// Create announcement for lampost
				/*mdns_SRV_announcement(par_name,strlen(par_name),
							par_service,strlen(par_service),
							par_target,strlen(par_target),
							par_txt,strlen(par_txt));*/
				coap_digcovery_announcement(dig_query, strlen(dig_query));
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

/*****************************************************************************/
/*************************** HTTP Server *************************************/
/*****************************************************************************/

#define MAX_HTTP_CONTENT_LEN 256
#define MAX_PAYLOAD_LEN 450

static int createHTTPSensorValueResponse(char *buffer){
	double hum, temp = 0;
	char content[MAX_HTTP_CONTENT_LEN];
	
	SHT11_MeasureTemperatureAndHumidity(&temp,&hum);
	sprintf(content, "<http><head></head><body><h1>Sensor values:</h1><ul><li>Temperature:%d</li><li>Luminosity:%d</li><li>Humidity:%d</li></ul></body></http>\r\n\r\n\0",(int)temp, value(TSL_VALUE_VISIBLE),(int)hum);
	
	sprintf(buffer, "HTTP/1.1 200 OK\r\nServer: Clitech Mote Server\r\nContent-Length: %d\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n%s",(int)strlen(content),content);
	return strlen(buffer);
}


static char server_buffer[MAX_PAYLOAD_LEN];

/*---------------------------------------------------------------------------*/
/*
 * A protosocket always requires a protothread. The protothread
 * contains the code that uses the protosocket. We define the
 * protothread here.
 */
 static struct psock ps;
 
static
PT_THREAD(handle_connection(struct psock *p))
{
	static int plen = 0;
  /*
   * A protosocket's protothread must start with a PSOCK_BEGIN(), with
   * the protosocket as argument.
   *
   * Remember that the same rules as for protothreads apply: do NOT
   * use local variables unless you are very sure what you are doing!
   * Local (stack) variables are not preserved when the protothread
   * blocks.
   */
  PSOCK_BEGIN(p);
		char* data = uip_appdata + uip_ext_len;
		u16_t datalen = uip_datalen() - uip_ext_len;
		//~ struct uip_tcpip_hdr* tcpiph = (struct uip_tcpip_hdr*) &uip_buf[UIP_LLH_LEN];
		vUART_printInit();
		vUART_DataInit();
		
		if(datalen>0){
			vPrintf("HTTP Server Data Received: %s\n",data);
			if(data[0] = 'G' && data[1] == 'E' && data[2] == 'T'){
				plen = createHTTPSensorValueResponse(server_buffer);
				vPrintf("Data Response from http Server: %s\n",server_buffer);
				PSOCK_SEND_STR(p, server_buffer);
			}else{
				vPrintf("Message Unkown.\n");
			}
		}
		
		/*
		* We close the protosocket.
		*/
		PSOCK_CLOSE(p);

		/*
		* And end the protosocket's protothread.
		*/
		PSOCK_END(p);
}


/*---------------------------------------------------------------------------*/
/*
 * The definition of the process.
 */
PROCESS_THREAD(webServ, ev, data)
{
  /*
   * The process begins here.
   */
  PROCESS_BEGIN();
	vUART_printInit();
	vUART_DataInit();
  /*
   * We start with setting up a listening TCP port. Note how we're
   * using the UIP_HTONS() macro to convert the port number (1010) to
   * network byte order as required by the tcp_listen() function.
   */
  tcp_listen(UIP_HTONS(80));

  /*
   * We loop for ever, accepting new connections.
   */
  while(1) {

    /*
     * We wait until we get the first TCP/IP event, which probably
     * comes because someone connected to us.
     */
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);

    /*
     * If a peer connected with us, we'll initialize the protosocket
     * with PSOCK_INIT().
     */
    if(uip_connected()) {
      
      /*
       * The PSOCK_INIT() function initializes the protosocket and
       * binds the input buffer to the protosocket.
       */
      PSOCK_INIT(&ps, server_buffer, MAX_PAYLOAD_LEN);

      /*
       * We loop until the connection is aborted, closed, or times out.
       */
      while(!(uip_aborted() || uip_closed() || uip_timedout())) {

        /*
         * We wait until we get a TCP/IP event. Remember that we
         * always need to wait for events inside a process, to let
         * other processes run while we are waiting.
         */
        PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);

        /*
         * Here is where the real work is taking place: we call the
         * handle_connection() protothread that we defined above. This
         * protothread uses the protosocket to receive the data that
         * we want it to.
         */
        vPrintf("\nHTTPSERVER Data found!\n");
        handle_connection(&ps);
      }
    }
  }
  
  /*
   * We must always declare the end of a process.
   */
  PROCESS_END();
}



