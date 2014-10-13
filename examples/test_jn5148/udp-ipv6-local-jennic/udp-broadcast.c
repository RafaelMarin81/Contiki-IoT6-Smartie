/**
 * \file
 *         UDP Broadcast (Send periodically UDP broadcast)
 * \author
 *         Adam Dunkels <adam@sics.se>
 *         Adapted for Jennic by Antonio Jara <jara@um.es>
 *         Adapted for Broadcast by Rafael Marin <rafael81@um.es>
 */


#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <string.h>

#include <stdio.h>

#include "juart.h"
#define PRINTF(...) vPrintf(__VA_ARGS__)


#define PRINTLLADDR(lladdr) PRINTF(" %x:%x:%x:%x:%x:%x ",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#define PRINT6ADDR(uip_ipaddr) PRINTF(" %x:%x:%x:%x:%x:%x:%x:%x \n",((u16_t*)uip_ipaddr)[0], ((u16_t*)uip_ipaddr)[1], ((u16_t*)uip_ipaddr)[2], ((u16_t*)uip_ipaddr)[3],((u16_t*)uip_ipaddr)[4], ((u16_t*)uip_ipaddr)[5], ((u16_t*)uip_ipaddr)[6], ((u16_t*)uip_ipaddr)[7])

#define SEND_INTERVAL		5 * CLOCK_SECOND
#define MAX_PAYLOAD_LEN		40

//0x00158d00000b1cf0

static struct uip_udp_conn *broadcast_conn;
static struct uip_udp_conn *broadcast_conn_receive;

/*---------------------------------------------------------------------------*/
PROCESS(udp_broadcast_process, "UDP broadcast process");
AUTOSTART_PROCESSES(&udp_broadcast_process);
/*---------------------------------------------------------------------------*/
static void tcpip_handler(void)
{
  char *str;

  if(uip_newdata()) {
    str = uip_appdata;
    str[uip_datalen()] = '\0';
    PRINTF("Received a broadcast message from the server: '%s'\n", str);
  }
}
/*---------------------------------------------------------------------------*/
static void timeout_handler(void)
{
  static int seq_id;
  char buf[MAX_PAYLOAD_LEN];

  PRINTF("Broadcast sending to: ");
  PRINT6ADDR(&broadcast_conn->ripaddr);
//  uip_create_linklocal_allnodes_mcast(&broadcast_conn->ripaddr);
 
  uip_ip6addr(&broadcast_conn->ripaddr, 0xff02,0,0,0,0,0,0,0x1); 

  PRINT6ADDR(&broadcast_conn->ripaddr);

  sprintf(buf, "Broad %d", ++seq_id);
  PRINTF(" (msg: %s)\n", buf);
  uip_udp_packet_send(broadcast_conn, buf, strlen(buf));
}
/*---------------------------------------------------------------------------*/
static void print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("UDP Broadcast IPv6 addresses: ");
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

///* OPCION BROADCAST - UTIL PARA RAFA :-)
//Beispiel: udp broadcast echo
PROCESS_THREAD(udp_broadcast_process, ev, data) 
{
    static struct etimer et;
    uip_ipaddr_t ipaddr;
    
    PROCESS_BEGIN();

    vUART_DataInit();

    PRINTF("UDP broadcast process started\n");
    print_local_addresses();

    broadcast_conn = udp_broadcast_new(UIP_HTONS(3000), NULL);  // remote port.
    udp_bind(broadcast_conn, UIP_HTONS(3001));  // local port
/*  struct uip_udp_conn* udp_broadcast_new(u16_t port, void* appstate);
 *  Create a new UDP broadcast connection.
 *  This function creates a new (link-local) broadcast UDP connection to a specified port.
 *  Parameters:
 *      port    Port number in network byte order.
 *      appstate    Pointer to application defined data.
 *  Returns:
 *      A pointer to the newly created connection, or NULL if memory could not be allocated for the connection. 
 */  

    broadcast_conn_receive = udp_broadcast_new(UIP_HTONS(3001), NULL);  // remote port.
    udp_bind(broadcast_conn_receive, UIP_HTONS(3000));  // local port

    PRINTF("Created a broadcast connection.");
    PRINTF("local/remote port %d/%d\n", UIP_HTONS(broadcast_conn->lport), UIP_HTONS(broadcast_conn->rport));

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

/*    while (1) {
        PROCESS_YIELD_UNTIL(ev==tcpip_event);
        uip_udp_packet_send(broadcast_conn, uip_appdata, uip_datalen());
    }
*/    
    PROCESS_END();

}
 
