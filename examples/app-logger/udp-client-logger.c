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

#include "juart.h"            // Using the juart implemented by Jara.

#include "logger.h"

#include <AppHardwareApi.h>

#ifdef CHIP_JN5148
#include <AppHardwareApi_JN514x.h>  // Using Jennic API to vAHI_Sleep
#endif

#include <AppApi.h>           // Using Jennic API to Get RSSI Radio Signal Streng Indicator.
#include <LedControl.h>       // Using Jennic API to Control Leds.

#include "mac_pib.h"          // Checking My MAC Address */


#define DEBUG DEBUG_PRINT
#define __MOVITAL__            // Using the juart implemented by Jara.

#include "net/uip-debug.h"

#include <string.h>

#define PRINTF(...) vPrintf(__VA_ARGS__)
#define PRINTLLADDR(lladdr) PRINTF(" %x:%x:%x:%x:%x:%x ",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])




#define SEND_INTERVAL_DATA	CLOCK_SECOND  / 500

#define SEND_INTERVAL		1 * CLOCK_SECOND

static struct etimer et;

static struct uip_udp_conn *client_conn;
static u16_t addr_dest[8]; //address dest

static struct datalog client_datalog;       // After Wakeup,    This struct is removed.
static u8_t seq = 0;                        // After Wakeup,    This variable keeps its value.

static bool isWakeupWithRAM = FALSE;

//0x00158d00000b1cf0
/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client process");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static bool check_My_MACAddr(void) {
    MAC_Addr_s MACAddr_Client;  // MAC_Addr_s  -> MAC_Addr_u uAddr; -> MAC_ExtAddr_s sExt;
    static uint16_t u16ShortAddr_MAC_Client = 0x00158d00000b1cf0;


    MACAddr_Client.uAddr.sExt.u32L = 0x00158d00;
    MACAddr_Client.uAddr.sExt.u32H = 0x000b1cf0;

    if((((MAC_ExtAddr_s*) pvAppApiGetMacAddrLocation())->u32H != MACAddr_Client.uAddr.sExt.u32H) || 
            (((MAC_ExtAddr_s*) pvAppApiGetMacAddrLocation())->u32L != MACAddr_Client.uAddr.sExt.u32L)) {

        PRINTF("check_My_MACAddr ERROR.... MAC_ExtAddr_s   My(%i,%i) != Client(%i,%i)\n\n"
                , ((MAC_ExtAddr_s*) pvAppApiGetMacAddrLocation())->u32L
                , ((MAC_ExtAddr_s*) pvAppApiGetMacAddrLocation())->u32H
                , MACAddr_Client.uAddr.sExt.u32L
                , MACAddr_Client.uAddr.sExt.u32H);

        return FALSE;
    }


    return TRUE;
}

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


static void vToggleLed1(void)       // Led1 == RED Light.
{
    static bool bToggle;
    if (bToggle) {
        vLedControl(2,1);
    } else {
        vLedControl(2,0);
    }
    bToggle = !bToggle;
}
static void vToggleLed2(void)       // Led2 == YELLOW Light.
{
    static bool bToggle;
    if (bToggle) {
        vLedControl(3,1);
    } else {
        vLedControl(3,0);
    }
    bToggle = !bToggle;
}
//-----------------------------------------------------------------------------
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
            if(power < MAX_POTENCIAS) {
                datos->power = power++;
            } else {
                client_datalog.state = STATE_STOPING;
            }
        }
    }
 
   //u8PowerLevel:       0,   1,   2,   3,  4, 5    
   //JN5139-power(dBm): –30, –24, –18, –12, –6, 0
   //JN5148-power(dBm): -32, -20, -9,  +2.5
   bAHI_PhyRadioSetPower(datos->power);       // In STATE_STARTING,  set the Radio power.
}

static void timeout_handler(void) {

    PRINTF("\n\nClient sending to: ");
    PRINT6ADDR(&client_conn->ripaddr);

    
    vToggleLed2();  // Prove toggle leds2 YELLOW


    if(client_datalog.state == STATE_STARTING) {
       etimer_set(&et, SEND_INTERVAL);
       PRINTF("STATE_STARTING...\n");
    } else if (client_datalog.state == STATE_DATA) {
       etimer_set(&et, SEND_INTERVAL_DATA);
       PRINTF("STATE_DATA...   SEND_INTERVAL_DATA:%i\n",SEND_INTERVAL_DATA);

       next_client_datalog(&client_datalog);    // In STATE_STARTING,  set the Radio power and Generate the next payload.

       seq++;   // Increase sequence for Sleep mode.

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

    // Before go to sleep,  Checking the status-idle of ADC-Sensor and Network-Stack.       NOT WORKING FINE.
    // 1. Ieee802 MAC Layer.
//    PRINTF("[timeout_handler]  bTACframeInProgress()\n");
//    while (bTACframeInProgress());

    // 2. 6lowpan Layer.
    // 3. TCPIP Layer.
//    PRINTF("[timeout_handler]  (uip_slen != 0)\n");
//    while (uip_slen != 0);

    // 4. UIP6 Layer.
//    PRINTF("[timeout_handler]  uip_poll()...\n");
//    while (uip_poll());


//    PRINTF("[timeout_handler]     vAHI_Sleep();   seq:%i  \n",seq);
//    if((seq % 2) && (client_datalog.state == STATE_DATA))  {        // Sleep mode, in STATE_DATA each 2 times.  1, 3, 5, 7, 9, 11, ...
//        vAHI_Sleep(E_AHI_SLEEP_OSCON_RAMON);    // Turn On-Oscillator and On-RAM
//    }

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

  PRINTF("\n\n\nabcdefghijklmnopqrstuvwyz\n");

  // Set DIO for LEDs.      Control-Leds depend on Programming-Board.    DK1,  DK2,  HPDevKit,  NTS,
  vLedInitRfd();
  vLedInitFfd();
  // Turn off all the LEDs.   vLedControl(LED,ON);
  vLedControl(0,0);
  // Turn on all the LEDs.
  vLedControl(1,0);

  /*  vAHI_HighPowerModuleEnable (JN513x Only)      PUBLIC void vAHI_HighPowerModuleEnable(bool_t bRFTXEn, bool_t bRFRXEn);
            bool_t bRFTXEn                   TRUE to enable high power module transmitter
                                             FALSE to disable high power module transmitter
            bool_t bRFRXEn                   TRUE to enable high power module receiver
                                             FALSE to disable high power module receiver
                                                                                                                                                                        Description: Allows the transmitter and receiver sections of a High-Power Module (HPM) to be
            individually enabled or disabled. Must be called before using radio on an HPM.
            The high-power modules use DIO2 and DIO3 to control the external LNA and PA circuitry. 
            For this reason, DIO2 and DIO3 are not available as signals on high-power modules. */
  vAHI_HighPowerModuleEnable(TRUE,TRUE);


  //u8PowerLevel:       0,   1,   2,   3,  4, 5    
  //JN5139-power(dBm): –30, –24, –18, –12, –6, 0
  //JN5148-power(dBm): -32, -20, -9,  +2.5
  bAHI_PhyRadioSetPower(MAX_TX_POWER);      //Replace to vJPT_RadioSetPower(MAX_TX_POWER);

  // - WARNING: Upon Wakeup, tcpip_process starts and repeat 2 times, "MAC 802. Send a BROADCAST" before send any UDP-packets.
  if(isWakeupWithRAM) { 

      // Keep values after wakup with memory.
      PRINTF("UDPclient  WarmStart with Memory..  client_datalog.state:%i power:%i tam_paq:%i num_sec:%i num_paq:%i)\n"
                      , client_datalog.state, client_datalog.power, client_datalog.tam_paq, client_datalog.num_sec, client_datalog.num_paq); 
  } else {
      PRINTF("UDPclient  ColdStart without Memory.....\n");
      isWakeupWithRAM = TRUE;
      init_client_datalog(&client_datalog);
  }


  PRINTF("UDP client process started\n");

  PRINTF("UDP client: You MUST Disable the ACK in ieee802-MAC Layer: TxFrameData->u8TxOptions = 1  //  Bit0 is 1  Then  ACK is Active.");


  print_local_addresses();

  uip_ip6addr(&ipaddr,0xfe80,0,0,0,0x0215,0x8d00,0x000b,0x1cf1);  // set_connection_address(&ipaddr);   for remote host.

  /* new connection with remote host */
  client_conn = udp_new(&ipaddr, UIP_HTONS(3000), NULL);
  udp_bind(client_conn, UIP_HTONS(3001));

  PRINTF("Created a connection with the server ");
  PRINT6ADDR(&client_conn->ripaddr);
  PRINTF("local/remote port %d/%d\n",
	UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));

  if(! check_My_MACAddr()) {    // 0x1cf0   - My address of Client
      PRINTF("check_My_MACAddr ERROR udp_client_process  finished.....\n\n");
      vAHI_Sleep(E_AHI_SLEEP_DEEP);
  }

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

 
