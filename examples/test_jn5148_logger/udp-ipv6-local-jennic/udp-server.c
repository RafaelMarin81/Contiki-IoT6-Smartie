/**
 * \file
 *         UDP Server (Receives UDP messages and reply them)
 * \author
 *         Adam Dunkels <adam@sics.se>
 *         Adapted for Jennic by Antonio Jara <jara@um.es>
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "juart.h"

#include <string.h>

#define PRINTF(...) vPrintf(__VA_ARGS__)
#define PRINT6ADDR(uip_ipaddr) PRINTF(" %x:%x:%x:%x:%x:%x:%x:%x \n",((u16_t*)uip_ipaddr)[0], ((u16_t*)uip_ipaddr)[1], ((u16_t*)uip_ipaddr)[2], ((u16_t*)uip_ipaddr)[3],((u16_t*)uip_ipaddr)[4], ((u16_t*)uip_ipaddr)[5], ((u16_t*)uip_ipaddr)[6], ((u16_t*)uip_ipaddr)[7])
#define PRINTLLADDR(lladdr) PRINTF(" %x:%x:%x:%x:%x:%x ",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

//0x00158d00000b1cf1

#define MAX_PAYLOAD_LEN 40

static struct uip_udp_conn *server_conn;

PROCESS(udp_server_process, "UDP server process");
AUTOSTART_PROCESSES(&udp_server_process);
/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
  static int seq_id;
  char buf[MAX_PAYLOAD_LEN];

  if(uip_newdata()) {
    ((char *)uip_appdata)[uip_datalen()] = 0;
    PRINTF("Server received: '%s' from ", (char *)uip_appdata);
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    PRINTF("\n");

    uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);    
    //It is setted up the remote port
    server_conn->rport = UIP_HTONS(3001);

    PRINTF("Responding with message: ");
    sprintf(buf, "Hello %d", ++seq_id);
    PRINTF("%s\n", buf);

    uip_udp_packet_send(server_conn, buf, strlen(buf));
    /* Restore server connection to allow data from any node */
    memset(&server_conn->ripaddr, 0, sizeof(server_conn->ripaddr));
  }
}
/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Server IPv6 addresses: \n");
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
PROCESS_THREAD(udp_server_process, ev, data)
{
// uip_ipaddr_t ipaddr;
 vUART_DataInit();

  PROCESS_BEGIN();

  PRINTF("UDP server started\n");

  //uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
 // uip_ip6addr(&ipaddr,0x2002,0,0,0,0,0,0,2);
  
  // Not used next line, since it is global
  //uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr); //set the last 64 bits of an IP address based on the MAC address
 // uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);

  print_local_addresses();

  // set NULL and 0 as IP address and port to accept packet from any node and port.
  // udp_new(remote_ip, port, appstate)
  server_conn = udp_new(NULL, UIP_HTONS(0), NULL); //for only 3001 port udp_new(NULL, UIP_HTONS(3001), NULL);
  udp_bind(server_conn, UIP_HTONS(3000));
  
  PRINTF("Listening on UDP port %d\n", UIP_HTONS(server_conn->lport));

  PRINTF("Created a server connection with remote address ");
  PRINT6ADDR(&server_conn->ripaddr);
  PRINTF("local/remote port %d/%d\n", UIP_HTONS(server_conn->lport),
         UIP_HTONS(server_conn->rport));

  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      tcpip_handler();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
