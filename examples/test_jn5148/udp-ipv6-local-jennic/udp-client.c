/**
 * \file
 *         UDP Client (Send periodically UDP messages and wait for the answer from the UDP SERVER)
 * \author
 *         Adam Dunkels <adam@sics.se>
 *         Adapted for Jennic by Antonio Jara <jara@um.es>
 */

/*
make
jenprog udp-server.jndevkit.hex

*/

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "juart.h"

#include <string.h>

#include <stdio.h>

#define PRINTF(...) vPrintf(__VA_ARGS__)
#define PRINT6ADDR(uip_ipaddr) PRINTF(" %x:%x:%x:%x:%x:%x:%x:%x \n",((u16_t*)uip_ipaddr)[0], ((u16_t*)uip_ipaddr)[1], ((u16_t*)uip_ipaddr)[2], ((u16_t*)uip_ipaddr)[3],((u16_t*)uip_ipaddr)[4], ((u16_t*)uip_ipaddr)[5], ((u16_t*)uip_ipaddr)[6], ((u16_t*)uip_ipaddr)[7])
#define PRINTLLADDR(lladdr) PRINTF(" %x:%x:%x:%x:%x:%x ",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])

#define SEND_INTERVAL		15 * CLOCK_SECOND
#define MAX_PAYLOAD_LEN		40

//0x00158d00000b1cf0

static struct uip_udp_conn *client_conn;
static u16_t addr_dest[8]; //address dest

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client process");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
  char *str;

  if(uip_newdata()) {
    str = uip_appdata;
    str[uip_datalen()] = '\0';
    PRINTF("Response from the server: '%s'\n", str);
  }
}
/*---------------------------------------------------------------------------*/
static void
timeout_handler(void)
{
  static int seq_id;
  char buf[MAX_PAYLOAD_LEN];

  PRINTF("Client sending to: ");
  PRINT6ADDR(&client_conn->ripaddr);

  sprintf(buf, "Hello %d", ++seq_id);
  PRINTF(" (msg: %s)\n", buf);
  uip_udp_packet_send(client_conn, buf, strlen(buf));
}
/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Client IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
set_connection_address(uip_ipaddr_t *ipaddr)
{
//uip_ip6addr(ipaddr,0x2002,0,0,0,0,0,0,0x0002);
//ff02::1	
//uip_ip6addr(ipaddr,0xff02,0,0,0,0,0,0,0x1);
uip_ip6addr(ipaddr,0xfe80,0,0,0,0x0215,0x8d00,0x000b,0x1cf1);
//0x0 0x0:0x0 0x0:0x0 0x0:0x2 0x15:0x8d 0x0:0x0 0xb:0x1c 0xf1
}
/*----------------------------------------uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);-----------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer et;
  uip_ipaddr_t ipaddr;

  PROCESS_BEGIN();

  vUART_DataInit();

  PRINTF("UDP client process started\n");

 // set_global_address();

  print_local_addresses();

  set_connection_address(&ipaddr);

  /* new connection with remote host */
  client_conn = udp_new(&ipaddr, UIP_HTONS(3000), NULL);
  udp_bind(client_conn, UIP_HTONS(3001));

  PRINTF("Created a connection with the server ");
  PRINT6ADDR(&client_conn->ripaddr);
  PRINTF("local/remote port %d/%d\n",
	UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));

  etimer_set(&et, SEND_INTERVAL);
  while(1) {
    PROCESS_YIELD();
    if(etimer_expired(&et)) {
      timeout_handler();
      etimer_restart(&et);
    } else if(ev == tcpip_event) {
      tcpip_handler();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

/* OPCION BROADCAST - UTIL PARA RAFA :-)
Beispiel: udp broadcast echo
PROCESS_THREAD(udp, ev, data) {
static struct uip_udp_conn *s;
PROCESS_BEGIN();
s = udp_broadcast_new(UIP_HTONS(3000), NULL);
udp_bind(s, UIP_HTONS(3001));
while (1) {
PROCESS_YIELD_UNTIL(ev==tcpip_event);
uip_udp_packet_send(s, uip_appdata,
uip_datalen());
}
PROCESS_END();
}
weitere Beispiele in examples/
*/
 
