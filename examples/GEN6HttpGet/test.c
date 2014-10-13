#include <stdlib.h>
#include <string.h>

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "juart.h"

static struct psock ps;
static char buffer[100];

PROCESS(gen6_getp, "Protosocket HTTP client");
AUTOSTART_PROCESSES(&gen6_getp);
/*---------------------------------------------------------------------------*/
static int
handle_connection(struct psock *p)
{
  PSOCK_BEGIN(p);

  PSOCK_SEND_STR(p, "GET / HTTP/1.0\r\n");
  PSOCK_SEND_STR(p, "Server: Contiki example protosocket client\r\n");
  PSOCK_SEND_STR(p, "\r\n");

  while(1) {
    PSOCK_READTO(p, '\n');
    vPrintf("Got: %s", buffer);
  }
  
  PSOCK_END(p);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(gen6_getp, ev, data)
{
  uip_ipaddr_t destiny;
  static struct etimer et;
  PROCESS_BEGIN();
	  vUART_printInit();
		vUART_DataInit();
		uip_init();
	  vPrintf("Starting HTTP client...\n");
	  uip_ip6addr(&destiny, 0x2001, 0x1470, 0xfffe, 0x20,0,0,0,0x174);
	  tcp_connect(&destiny, UIP_HTONS(80), NULL);
	  vPrintf("Connecting...\n");
	  etimer_set(&et, 15*CLOCK_SECOND);
	  while(1){
		  PROCESS_YIELD();
		  //PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);

		  if(uip_aborted() || uip_timedout() || uip_closed()) {
			vPrintf("Could not establish connection\n");
		  } else { //if(uip_connected()) {
			vPrintf("Connected\n");
			
			PSOCK_INIT(&ps, buffer, sizeof(buffer));

			do {
			  handle_connection(&ps);
			  PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
			} while(!(uip_closed() || uip_aborted() || uip_timedout()));

			vPrintf("\nConnection closed.\n");
		  }
		  //else{
			//vPrintf("Something rare happends\n");
		  //}
	  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
