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

#include "logger.h"

#include <AppHardwareApi.h>
#include <AppApi.h>           // Using Jennic API to Get RSSI Radio Signal Streng Indicator.
#include <JPT.h>              // Using Jennic API to RadioSetPower Transmission.
#include <LedControl.h>       // Using Jennic API to Control Leds.

#include <string.h>

#define PRINTF(...) vPrintf(__VA_ARGS__)
#define PRINT6ADDR(uip_ipaddr) PRINTF(" %x:%x:%x:%x:%x:%x:%x:%x \n",((u16_t*)uip_ipaddr)[0], ((u16_t*)uip_ipaddr)[1], ((u16_t*)uip_ipaddr)[2], ((u16_t*)uip_ipaddr)[3],((u16_t*)uip_ipaddr)[4], ((u16_t*)uip_ipaddr)[5], ((u16_t*)uip_ipaddr)[6], ((u16_t*)uip_ipaddr)[7])
#define PRINTLLADDR(lladdr) PRINTF(" %x:%x:%x:%x:%x:%x ",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

//0x00158d00000b1cf1


static struct uip_udp_conn *server_conn;

static int table_datalog[MAX_SECUENCIA][MAX_TX_POWERS];


PROCESS(udp_server_process, "UDP server process");
AUTOSTART_PROCESSES(&udp_server_process);
/*---------------------------------------------------------------------------*/
/****************************************************************************
 *
 * NAME: vToggleLed
 *
 * PARPADEO DEL LED PARA SABER QUE ESTÁ FUNCIONANDO SIN PROBLEMAS
 *
 * DESCRIPTION:
 * Toggles LED1 to indicate we are alive.
 *
 *
 ****************************************************************************/
static void vToggleLed1(void)
{
    static bool bToggle;
    if (bToggle) {
        vLedControl(2,1);
    } else {
        vLedControl(2,0);
    }
    bToggle = !bToggle;
}
static void vToggleLed2(void)
{
    static bool bToggle;
    if (bToggle) {
        vLedControl(3,1);
    } else {
        vLedControl(3,0);
    }
    bToggle = !bToggle;
}
/*---------------------------------------------------------------------------*/

static void init_table_datalog() {
    uint8_t num_sec,power;
 
    for(power=0; power < MAX_TX_POWERS; power++) {
        for(num_sec=0; num_sec < MAX_SECUENCIA; num_sec++) {
            table_datalog[num_sec][power] = 0;
        }
    }

}


static void print_table_datalog() {
   static uint8_t num_location = 0;
   uint8_t num_sec,power;

   PRINTF(" Server print_table_datalog LOCATION %i\n",num_location);  
   for(power=0; power < MAX_TX_POWERS; power++) {
       PRINTF("  POWER  %i  PRR", power );
       for(num_sec=0; num_sec < MAX_SECUENCIA; num_sec++) {
           PRINTF("  %i",table_datalog[num_sec][power]);
       }
       PRINTF("\n");
   }
   num_location++;

}

static void tcpip_handler(void) {
  static bool is_table_datalog_printed = FALSE;
  static struct datalog server_received_datalog;

  if(uip_newdata()) {
    memcpy(&server_received_datalog, uip_appdata, sizeof(datalog));

//    PRINTF("Server (datalog-received  state:%i power:%i tam_paq:%i num_sec:%i num_paq:%i)\n", server_received_datalog.state, server_received_datalog.power, server_received_datalog.tam_paq, server_received_datalog.num_sec, server_received_datalog.num_paq);


    if (server_received_datalog.state == STATE_DATA) {      // No reply to DATA packets.
        PRINTF("STATE_DATA received-Server...");
        //" S   D  Pow Size seq paq rssi lqi"
        //" 2   1  31  10   0   1 -24 107"
        //"LOG State Power Size Seq Paq 0 0" 
        PRINTF("LOG %i %i %i %i %i 0 0\n"
                , server_received_datalog.state, server_received_datalog.power, server_received_datalog.tam_paq, server_received_datalog.num_sec, server_received_datalog.num_paq);

        table_datalog[server_received_datalog.num_sec][server_received_datalog.power]++;

    } else {

        if(server_received_datalog.state == STATE_STARTING) {
            PRINTF("STATE_STARTING  received-Server...\n");
            init_table_datalog();
            is_table_datalog_printed = FALSE;
        } else if(server_received_datalog.state == STATE_STOPING) {
            PRINTF("STATE_STOPING  received-Server...\n");
            if(is_table_datalog_printed == FALSE) {
                is_table_datalog_printed = TRUE;
                print_table_datalog();
            }
        } else if(server_received_datalog.state == STATE_FINISHED) {
            PRINTF("STATE_FINISHED  received-Server...\n");
        } else {
            PRINTF("tcpip_handler invalid state:%i  received-Server...\n", server_received_datalog.state);
        } 

        uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);    
        //It is setted up the remote port
        server_conn->rport = UIP_HTONS(3001);

        PRINTF("Responding with ACK...\n");

        uip_udp_packet_send(server_conn, &server_received_datalog, sizeof(datalog)); 

        /* Restore server connection to allow data from any node */
        memset(&server_conn->ripaddr, 0, sizeof(server_conn->ripaddr));
    }
  }
}

/*---------------------------------------------------------------------------*/

static void print_local_addresses(void) {
    int i;
    uint8_t state;
    
    PRINTF("Server IPv6 addresses: ");
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
  vUART_DataInit();     // Inizialice the juart implemented by Jara.

  // Set DIO for LEDs.      Control-Leds depend on Programming-Board.    DK1,  DK2,  HPDevKit,  NTS,
  vLedInitRfd();
  vLedInitFfd();

  // Turn off all the LEDs.   vLedControl(LED,ON);
  vLedControl(0,0);
  // Turn on all the LEDs.
  vLedControl(1,0); 

//  vLedControl(2,0);   // Turn on LED2
//  vLedControl(3,0);   // Turn on LED1
//  vLedControl(4,0);   // Turn on all the LEDs.
//  vLedControl(5,0);   // Turn on all the LEDs.


  //u8PowerLevel:       0,   1,   2,   3,  4, 5    
  //Radio-power(dBm): –30, –24, –18, –12, –6, 0
//  vJPT_RadioSetPower(5);

  // Enable High-Power Module. for transmit, reception, sleep mode.
  vAHI_HighPowerModuleEnable(TRUE, TRUE);

  PROCESS_BEGIN();


  PRINTF("UDP server started\n");

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



// To get LinkQuality, we have to work directly with the ieee802 layer of Jennic Stack Network.
// The following is an example of handling a deferred active scan confirmation (assumes data is passed as a pointer to a deferred confirm indicator data type i.e. MAC_MlmeDcfmInd_s *psMlmeInd.)
//MAC_MlmeDcfmInd_s *psMlmeInd;  
//MAC_PanDescr_s *psPanDesc = &psMlmeInd->uParam.sDcfmScan.uList.asPanDescr[i]; //Descriptor of the channel.
//uint8 psPanDesc->u8LinkQuality; // Link quality of the received beacon

