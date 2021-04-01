#include <stdlib.h>
#include <string.h>

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "juart.h"
#include "sht-sensor.h"
#include <AppHardwareApi.h>
#include <lightlevel-sensor.h>

#define MAX_PAYLOAD_LEN 450
#define MAX_URI_LEN 300
#define MAX_HTTP_CONTENT_LEN 256


PROCESS(gen6_getp, "HTTP GET Proc");
PROCESS(gen6_webServ, "HTTP Server Proc");
AUTOSTART_PROCESSES(&gen6_getp,&gen6_webServ);



static char buffer[MAX_PAYLOAD_LEN];
//static char host[32] = "2001:1470:fffe:20::174";
static char host[32] = "2a01:7e00::f03c:91ff:fe96:190\0";
static char uri[MAX_URI_LEN];
static char payload[MAX_URI_LEN];
static int plen = 0;

int tempSent = 0;
int lumSent = 0;
int humSent = 0;
static struct etimer et;

static int is_sent = 0;
static int is_first = 1;	

static int createHttpGetRequest(char *buffer, char *host, char *uri, char* payload){
	sprintf(buffer, "POST %s HTTP/1.1\r\nHost: backend.intelen.com\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\nConnection: close\r\n\r\n",uri, strlen(payload));
	buffer[0] = 'P';
	
	sprintf(buffer+strlen(buffer), "%s", payload);
	vPrintf("Packet payload: %s\n Payload Len: %d\n", buffer, strlen(buffer));
	return strlen(buffer);
}

static int createHTTPSensorValueResponse(char *buffer){
	double hum, temp = 0;
	char content[MAX_HTTP_CONTENT_LEN];
	
	SHT11_Init_MeasureTemperatureAndHumidity(&temp,&hum);
	sprintf(content, "<http><head></head><body><h1>Sensor values:</h1><ul><li>Temperature:%d</li><li>Luminosity:%d</li><li>Humidity:%d</li></ul></body></http>\r\n\r\n\0",(int)temp, 0, (int)hum);//value(TSL_VALUE_VISIBLE),(int)hum);
	
	sprintf(buffer, "HTTP/1.1 200 OK\r\nServer: Clitech Mote Server\r\nContent-Length: %d\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n%s",(int)strlen(content),content);
	return strlen(buffer);
}

static void
set_global_address(void)
{
	 uip_ipaddr_t ipaddr;
	 uip_ip6addr(&ipaddr, 0x2001, 0x720, 0x1710, 0x11,0,0,0,0x1052);
	 uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);	 
	 
     //uip_ipaddr_t loc_fipaddr;
	 //uip_create_linklocal_prefix(&loc_fipaddr);
	 //uip_ds6_addr_add(&loc_fipaddr, 0, ADDR_AUTOCONF);
	 
}



int http_get_request_response_handler(){
	
	char* data = uip_appdata + uip_ext_len;
	u16_t datalen = uip_datalen() - uip_ext_len;
	
	
	if (uip_newdata()) {
		((char *)data)[datalen] = 0;
		
		vPrintf("Data Response: %s\n",data);
		
	}
	return 0;
}

PROCESS_THREAD(gen6_getp, ev, data)
{
	static uip_ipaddr_t destiny;
	static double hum, temp = 0;
	static int lum = 0;
	
	PROCESS_BEGIN();
		vUART_printInit();
		vUART_DataInit();
		
        vAHI_HighPowerModuleEnable(TRUE,TRUE);
		bAHI_PhyRadioSetPower(3);
		
        // 1. Set 32 bit RISC CPU to 4Mhz.
        bAHI_SetClockRate(0);       
        SHT11_Init_MeasureTemperatureAndHumidity(&temp, &hum); // Before  Set 32 bit RISC CPU to 4Mhz.   bAHI_SetClockRate(0);
        
		//luminosity sensor init
		configure(SENSORS_HW_INIT,0);
		
				
		vPrintf("Initiating process\n");
		//Setting network config
		set_global_address();
		
		uip_ip6addr(&destiny, 0x2a01, 0x7e00, 0, 0, 0xf03c, 0x91ff, 0xfe96, 0x190);

		//Setting parameters
		
		//Setting timer
		etimer_set(&et, 2*CLOCK_SECOND);
		while(1){
			PROCESS_YIELD();

			if(isAssociated() && uip_rexmit()){
				uip_send(buffer,plen);
				return 0;
			}


			if(isAssociated() && ev == tcpip_event) {				
				vPrintf("Packet received!\n");
				http_get_request_response_handler();
			}

			//When we are associated, no connected, no gen6 request sent and the timer expires (the packets are sent each 60 seconds)
			if(isAssociated() && !uip_connected() && etimer_expired(&et)){
				vPrintf("TCPIP: Trying to connect...\n");
				
				tcp_connect(&destiny, UIP_HTONS(80), NULL); 		
				if(etimer_expired(&et)) { 
					etimer_set(&et, 2*CLOCK_SECOND);
				}
				
				PROCESS_YIELD();
				
				if(uip_connected()){
					vPrintf("TCPIP Message: Connected!!!\n");
				}else{
					vPrintf("TCPIP Message: Can't connect with the server.\n");
				}
			}
			
			if(isAssociated() && uip_connected()){
				SHT11_Init_MeasureTemperatureAndHumidity(&temp,&hum);
				memset(buffer,'\0',MAX_PAYLOAD_LEN);
				memset(uri,'\0',MAX_URI_LEN);
				memset(payload,'\0',MAX_URI_LEN);

				sprintf(uri,"/index.php/insert/data/mac/234234/hash/017466352925ce489bb97a7356cfabe1/");
				//sprintf(payload, "data={temp:%d,hum:%d,lum:%d}", (int)temp, (int)hum, value(TSL_VALUE_VISIBLE));
				sprintf(payload, "data={temp:%d,hum:%d,lum:%d}", (int)temp, (int)hum, 0);

				plen = createHttpGetRequest(buffer, host, uri, payload);
				vPrintf("Sending request.\n");
				uip_send(buffer,plen);
				vPrintf("Request sent.\n");
				etimer_set(&et, 60*CLOCK_SECOND);		
			}else if(!isAssociated()){				
				vPrintf("Not Associated\n");
			}else if(!uip_connected()){
				vPrintf("Not Connected\n");
			}

			if(etimer_expired(&et)) { 
				etimer_set(&et, 2*CLOCK_SECOND);
			}
		}

	PROCESS_END();
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
PROCESS_THREAD(gen6_webServ, ev, data)
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
