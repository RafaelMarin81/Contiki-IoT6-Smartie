/**
 * \file
 *         UDP Client (Send periodically UDP messages and wait for the answer from the UDP SERVER)
 * \author
 *         Adam Dunkels <adam@sics.se>
 *         Adapted for Jennic by Antonio Jara <jara@um.es>
 */

/* make jenprog udp-server.jndevkit.hex */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

//#include <LedControl.h>       // Using Jennic API to Control Leds.
#include "juart.h"            // Jara UART.
#include "logger.h"           // Rafa Logger.
#include <string.h>

#define DEBUG DEBUG_PRINT
#define __MOVITAL__
#define __BA2__ //JN5148 compiler --- export PATH=/opt/msp430/bin:/opt/IBMJava2-142/jre/bin:/opt/IBMJava2-142/bin:/usr/local/avr/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/ba-elf-jn5148/bin:/usr/ba-elf-jn5148/libexec/gcc/ba-elf/4.1.2/

#include "net/uip-debug.h"

#define PRINTF(...) vPrintf(__VA_ARGS__)
#define PRINT6ADDR(uip_ipaddr) PRINTF(" %x:%x:%x:%x:%x:%x:%x:%x \n",((u16_t*)uip_ipaddr)[0], ((u16_t*)uip_ipaddr)[1], ((u16_t*)uip_ipaddr)[2], ((u16_t*)uip_ipaddr)[3],((u16_t*)uip_ipaddr)[4], ((u16_t*)uip_ipaddr)[5], ((u16_t*)uip_ipaddr)[6], ((u16_t*)uip_ipaddr)[7])
#define PRINTLLADDR(lladdr) PRINTF(" %x:%x:%x:%x:%x:%x ",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])

#define SEND_INTERVAL_DATA	CLOCK_SECOND  / 500

#define SEND_INTERVAL		1 * CLOCK_SECOND

//0x00158d00000b1cf0

  static struct etimer et;

static struct uip_udp_conn *client_conn;
static u16_t addr_dest[8]; //address dest

static struct datalog client_datalog;

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client process");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static void tcpip_handler(void) 
{
  struct datalog datalog_received_from_server;

  if(uip_newdata()) {
    memcpy(&datalog_received_from_server, uip_appdata, sizeof(datalog));

    PRINTF(" (datalog_received_from_server  state:%i power:%i tam_paq:%i num_sec:%i num_paq:%i)\n"
        , datalog_received_from_server.state, datalog_received_from_server.power, datalog_received_from_server.tam_paq, datalog_received_from_server.num_sec, datalog_received_from_server.num_paq);

        if(datalog_received_from_server.state == STATE_STARTING) {
            client_datalog.state = STATE_DATA;
            PRINTF("STATE_STARTING...\n");
            
//        enviarInicio();     // button-RESET: Start the communication PAQ_INICIO.       Until a ACK is received.
            
            
        } else if (client_datalog.state == STATE_DATA) {
            PRINTF("STATE_DATA... SEND_INTERVAL_DATA:%i \n",SEND_INTERVAL_DATA);
        } else if (client_datalog.state == STATE_STOPING) {
            client_datalog.state = STATE_FINISHED;
            PRINTF("STATE_STOPING...\n");
        } else if (client_datalog.state == STATE_FINISHED) {
            PRINTF("STATE_FINISHED...\n");
        } else {
            PRINTF("[timeout_handler] ERROR invalid client_datalog.state: %i",client_datalog.state);
        }

  }
}

/****************************************************************************************************/
#define STATE_STARTING 1
#define STATE_DATA 2
#define STATE_STOPING 3
#define STATE_FINISHED 4

#define MAX_PAYLOAD_LEN 92  // 92 = (127byte - 25 for MAC header - 9 for 6lowpan header) 


uint8_t num_paq = 0;
uint8_t num_sec = 0;
uint8_t power = 0;
uint8_t tam_paq = 0; 


static void init_client_datalog(datalog *datos) {
    num_paq = 1;
    num_sec = 1;
    power = 1;
    tam_paq = 1;

    datos->state = STATE_STARTING;
    datos->num_paq = 0;
    datos->num_sec = 0;
    datos->power = 0;
    datos->tam_paq = MAX_PAYLOAD_LEN;    //MAX_PAYLOAD_LEN 92 = (127byte - 25 for MAC header - 9 for 6lowpan header) 
}
static void next_client_datalog(datalog *datos) {

    if( (num_paq == 1) && (num_sec == 1) && (power == 1)) {   // Init the RadioTransmission.
bAHI_PhyRadioSetPower(0);
//        vJPT_RadioSetPower(0);
    }

    if(num_paq < MAX_PAQUETES) {   // Quedan paquetes por enviar.
        datos->num_paq = num_paq++;
    } else {        
        datos->num_paq = 0;
        num_paq = 1;
        if (num_sec < MAX_SECUENCIA) {
            datos->num_sec = num_sec++;
        } else {
            datos->num_sec = 0;
            num_sec = 1;
            if(power < MAX_TX_POWERS) {
                //u8PowerLevel:       0,   1,   2,   3,  4, 5
                //Radio-power(dBm): –30, –24, –18, –12, –6, 0
                datos->power = power++;
bAHI_PhyRadioSetPower(datos->power);
  //              vJPT_RadioSetPower(datos->power);
            } else {
                client_datalog.state = STATE_STOPING;
            }
        }
    }
}

static void timeout_handler(void) {
    static u8_t seq = 0;

    PRINTF("\n\nClient sending to: ");
    PRINT6ADDR(&client_conn->ripaddr);

    if(client_datalog.state == STATE_STARTING) {
       etimer_set(&et, SEND_INTERVAL);
       PRINTF("STATE_STARTING...\n");
    } else if (client_datalog.state == STATE_DATA) {
       etimer_set(&et, SEND_INTERVAL_DATA);
       PRINTF("STATE_DATA...   SEND_INTERVAL_DATA:%i\n",SEND_INTERVAL_DATA);

       next_client_datalog(&client_datalog);    // Generate the next payload.
    } else if (client_datalog.state == STATE_STOPING) {
       etimer_set(&et, SEND_INTERVAL_DATA);
       PRINTF("STATE_STOPING...\n");
    } else if (client_datalog.state == STATE_FINISHED) {
       PRINTF("STATE_FINISHED...\n");
       return;
    } else {
       PRINTF("[timeout_handler] ERROR invalid client_datalog.state: %i",client_datalog.state);
       return;
    }

    PRINTF(" (datalog  client_datalog.state:%i power:%i tam_paq:%i num_sec:%i num_paq:%i)\n"
        , client_datalog.state, client_datalog.power, client_datalog.tam_paq, client_datalog.num_sec, client_datalog.num_paq);

    uip_udp_packet_send(client_conn, &client_datalog, client_datalog.tam_paq); 
}

/****************************************************************************************************/
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

/*----------------------------------------uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);-----------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  uip_ipaddr_t ipaddr;

  PROCESS_BEGIN();

  vUART_DataInit();


  // Set DIO for LEDs.      Control-Leds depend on Programming-Board.    DK1,  DK2,  HPDevKit,  NTS,
//  vLedInitRfd();
//  vLedInitFfd();
  // Turn off all the LEDs.   vLedControl(LED,ON);
//  vLedControl(0,0);
  // Turn on all the LEDs.
//  vLedControl(1,0);

  //u8PowerLevel:       0,   1,   2,   3,  4, 5
  //Radio-power(dBm): –30, –24, –18, –12, –6, 0

  bAHI_PhyRadioSetPower(3);

  //vJPT_RadioSetPower(5);
  init_client_datalog(&client_datalog);

  // DIO2 and DIO3 are not available as signals on high-power modules
  // if you wish to run them on the high-power controller boards. You must also consider
  // the following points when using high-power modules:
  //    • If using the SPI port, you must not attempt to enable more than two extra SPISEL signals
  //          (SPISEL3 is an alternate function of DIO2, and SPISEL4 is an alternate function of DIO3).
  //             • Functions within the Board API that use DIO2 and DIO3 will require modification to work
  //                   correctly.
  vAHI_HighPowerModuleEnable(TRUE,TRUE);

  PRINTF("UDP client process started\n");

  print_local_addresses();

  uip_ip6addr(&ipaddr,0xfe80,0,0,0,0x0215,0x8d00,0x000b,0x1cf1);  // set_connection_address(&ipaddr);

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
//      etimer_restart(&et);
    } else if(ev == tcpip_event) {
      tcpip_handler();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

 
