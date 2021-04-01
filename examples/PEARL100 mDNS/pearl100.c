/* 
 * File:   main.c
 * Author: Pablo López Martínez
 *
 * Created on 19 de diciembre de 2011, 10:54
 */

#include <stdlib.h>
#include <string.h>
#include <AppHardwareApi.h>
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "juart.h"
#include "mdns.h"


#define DEBUG 1
#if DEBUG
#define PRINTF(...) vPrintf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
#define PRINT6ADDR(addr) PRINTF(" %x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x ", ((u8_t *)addr)[0], ((u8_t *)addr)[1], ((u8_t *)addr)[2], ((u8_t *)addr)[3], ((u8_t *)addr)[4], ((u8_t *)addr)[5], ((u8_t *)addr)[6], ((u8_t *)addr)[7], ((u8_t *)addr)[8], ((u8_t *)addr)[9], ((u8_t *)addr)[10], ((u8_t *)addr)[11], ((u8_t *)addr)[12], ((u8_t *)addr)[13], ((u8_t *)addr)[14], ((u8_t *)addr)[15])
#define UDP_HDR ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])


#define length(a) ( sizeof ( a ) / sizeof ( a[0] ) )
#define abso(a) ( a>=0?a:-a )
#define MAX_SIZE 250
#define SEND_INTERVAL		CLOCK_SECOND
#define MAX_PAYLOAD_LEN		250
#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

static u16_t addr_dest[8]; //address dest

/*---------------------------------------------------------------------------*/
PROCESS(pearl100_process, "pearl100 process");
AUTOSTART_PROCESSES (&pearl100_process);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*---------------------UDP Connection Functions------------------------------*/
/*---------------------------------------------------------------------------*/
static struct uip_udp_conn *client_conn = NULL;
static struct uip_udp_conn *server_conn = NULL;
static struct uip_udp_conn *announcement_conn;
//static uint32_t id_packet = 0;

static uint8_t heartData[MAX_SIZE];

static uip_ipaddr_t addDes;
static struct etimer et;
static uip_ipaddr_t ipaddr;

/*static void timeout_handlerYOAPY(struct CompressedWave cwave)
{
  //static int seq_id;
  char buf[MAX_PAYLOAD_LEN];

  buf[0] = cwave.alturaP;
  buf[1] = cwave.alturaQ;
  buf[2] = cwave.alturaR;
  buf[3] = cwave.alturaS;
  buf[4] = cwave.alturaT;
  buf[5] = cwave.segmentoP;
  buf[6] = cwave.segmentoPQ;
  buf[7] = cwave.segmentoQS;
  buf[8] = cwave.segmentoST;
  buf[9] = cwave.segmentoT;
  buf[10] = cwave.bpm;
  buf[11] = cwave.spo2;
  buf[12] = cwave.diag;

  uip_udp_packet_send(client_conn, buf, 13);
}*/

static void timeout_handlerRAW(char bpm, char spo, int heartLen)
{
  char buf[MAX_SIZE];
  
  buf[0] = bpm;
  buf[1] = spo;
  memcpy(buf+2, heartData, heartLen);
  uip_udp_packet_send(client_conn, buf, heartLen + 2);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static void set_global_address(void)
{
  addr_dest[0] = 0x2001;
  addr_dest[1] = 0x720;
  addr_dest[2] = 0x1710;
  addr_dest[3] = 0x11;
  addr_dest[4] = 0;
  addr_dest[5] = 0;
  addr_dest[6] = 0;
  addr_dest[7] = 0x5;

  uip_ip6addr(&ipaddr, addr_dest[0], addr_dest[1], addr_dest[2], addr_dest[3],
              addr_dest[4], addr_dest[5], addr_dest[6], addr_dest[7]);

  uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL); // ADDR_AUTOCONF
}
/*---------------------------------------------------------------------------*/
static void set_connection_address(uip_ipaddr_t *addr)
{
  uip_ip6addr(addr, 0x2001, 0x720, 0x1710, 0x11, 0, 0, 0, 0x1);
}
/*---------------------------------------------------------------------------*/
static void set_udp_connection(void)
{

  //PRINTF("UDP client process started\n");

  set_global_address();
  set_connection_address(&addDes);

  //PRINTF("Connection with the server 2001:720:1710:11::1  PORT 54568\n");

  client_conn = udp_new(&addDes, UIP_HTONS(54568), NULL);
  /* new connection with remote host */
  server_conn = udp_new(NULL, UIP_HTONS(54568), NULL);
  udp_bind(server_conn, UIP_HTONS(54568));
  

}

/*---------------------------------------------------------------------------*/
/*---------------------------PEARL100 Functions------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* 
 Get the serial data of PEARL100 device
 */
int getYoapySerialData(unsigned char *bpm, char *spo2, int *totalLen,
                       uint8_t *hArrAct, uint8_t *sArrAct, uint8_t *hArrAnt,
                       uint8_t *sArrAnt)
{
  unsigned char data = 0;
  int l_len = 0;
  int k = 0;
  int j = 0;
  int dataObtained = 0;
 
 
  while(!dataObtained && bAvaibleC_UART1()) {
    data = uGetC_UART1();
    if(data != 0xF7)  // F7
      continue;
    //PRINTF("data=%x dec:%d\n",data,data); 
    dataObtained = 1;
    while((data = uGetC_UART1()) != 0xF9) {
      if(l_len % 2 == 0) {
        sArrAct[k] = data;
        k++;
      } else {
        hArrAct[j] = data;
        j++;
      }
      l_len++;
    }
    *spo2 = uGetC_UART1();
    uGetC_UART1();
    *bpm = uGetC_UART1();
    uGetC_UART1();
    if(uGetC_UART1() == 5)
      return 0;
    uGetC_UART1();
    uGetC_UART1();

  }
  if(!dataObtained)
    return 0;

  *totalLen = l_len;
  return 1;
/*
     while (1) {
         if ((val=getccb()) == 0xF8)
         {
             while((val=getccb()) < 0xF0)
             {
                 // here "val" always contains a new plethysmogram sample 
                 // process it acccording to your needs ......... 
             }
         }
         switch(val) // now val contains a marker, indicates next byte is a special value 
         {
             case 0xF9:
                 printf(”%02u”,getccb()); // print SpO2 
                 break;
             case 0xFA:
                 printf(”%03u”,(unsigned char)getccb()); // print pulse 
                 break;
             case 0xFB:
                 switch(getccb())
                 {
                     case 0: gotoxy(20,23);
                             printf(“ OK ! “); // print messages 
                             break;
                     case 1: gotoxy(20,23);
                             printf(“ No sensor connected ! “);
                             break;
                     case 2: gotoxy(20,23);
                             printf(“ No finger in probe ! “);
                             break;
                     case 3: gotoxy(20,23);
                             printf(“ Low perfusion ! “);
                             break;
                 }
                 break;
             case 0xFC:
                 val = getccb();
                 PRINTF("%d",getccb()&0x0F); // print quality, mask perf.
                 break;
         }
     }
*/
}

/*---------------------------------------------------------------------------*/
/*
 Emulated serial data to do trys

 */
/*
static int getEmulatedSerialData(unsigned char *bpm, char *spo2, int *totalLen,
                          int primera, uint8_t *hArrAct, uint8_t *sArrAct, uint8_t *hArrAnt,
                          uint8_t *sArrAnt)
{
  int i;
  int arrLen = 42;
  *bpm = 80;
  *totalLen = 42;
  *spo2 = 97;
  const uint8_t arr[] = {
      151,
      152,
      155,
      156,
      154,
      157,
      159,
      158,
      157,
      156,
      157,
      155,
      155,
      161,
      164,
      170,
      180,
      166,
      155,
      151,
      146,
      162,
      246,
      236,
      136,
      142,
      145,
      147,
      153,
      163,
      174,
      184,
      205,
      214,
      206,
      172,
      143,
      125,
      122,
      121,
      128,
      126 };
  const uint8_t arrspo[] = {
      151,
      152,
      155,
      156,
      154,
      157,
      159,
      158,
      157,
      156,
      157,
      155,
      155,
      161,
      164,
      170,
      180,
      166,
      155,
      151,
      146,
      162,
      246,
      236,
      136,
      142,
      145,
      147,
      153,
      163,
      174,
      184,
      5,
      214,
      206,
      172,
      143,
      125,
      122,
      121,
      128,
      126 };

  if(!primera) {
    for(i = 0; i < arrLen / 2; i++) {
      hArrAct[i] = arr[i];
      sArrAct[i] = arrspo[i];
    }
  } else {
    for(i = arrLen / 2; i < arrLen; i++) {
      hArrAct[i - arrLen / 2] = arr[i];
      sArrAct[i - arrLen / 2] = arrspo[i];
    }
  }
  return 0;

}*/
/*---------------------------------------------------------------------------*/
/* This function syncronize the wave using the SPO2 wave obtained through serial port

 */
/*
 * Monitor Function
 * 
 */ 
/*static int syncWave(int *firstSPO2, int len1, int *secondSPO2, int len2)
{
  int minf1 = 255;
  int pminf1 = 0;
  int minf2 = 255;
  int pminf2 = 0;
  int pos = 0;
  int i = 0;

  for(i = 0; i < len1; i++) {
    if(firstSPO2[i] < minf1 && firstSPO2[i] != -1) {
      minf1 = firstSPO2[i];
      pminf1 = pos;
    }
    pos++;
  }

  for(i = 0; i < len2; i++) {
    if(secondSPO2[i] < minf2 && secondSPO2[i] != -1) {
      minf2 = secondSPO2[i];
      pminf2 = pos;
    }
    pos++;
  }

  if(((minf1 < minf2) && (pminf1 < pos / 2))
      || ((minf2 < minf1) && (pminf2 < pos / 2)))
    return 0;
  else
    return 1;
}*/

/*
 * Test function
 */
/*static int syncWaveECG(uint8_t *fECG, int len1, uint8_t *sECG, int len2){
  
  int maxf1 = 0;
  int pmaxf1 = 0;
  int maxf2 = 0;
  int pmaxf2 = 0;
  int pos = 0;
  int i=0;
  
  for(i=0;i<len1;i++){
    if(fECG[i] > maxf1){
      maxf1 = fECG[i];
      pmaxf1 = pos;
    }
    pos++;    
  }
  
  for(i = 0; i < len2; i++) {
    if(sECG[i] < maxf2) {
      maxf2 = sECG[i];
      pmaxf2 = pos;
    }
    pos++;
  }
  
  if(((maxf1 > maxf2) && (pmaxf1 < pos/2))
      || ((maxf2 > maxf1) && (pmaxf2 < pos/2))){
    return 0;
  }else{
    return 1;
  }
  
}*/

/*---------------------------------------------------------------------------*/
/* This function store in heartData the heart data obtained through COM1
 and store in bpm and spo2 another data related with the heart 

 To complete the operation we need 2 frames obtained through Serial port
 
 NOTE: Due  data from the serial port has data collated heart and spo2, 
 the heart's arrays ever is (total_len_of_data/2)
 then spo2 arrays may be total/2 or total/2+1
 
 we must to know if the data array is pair or odd

 */
static int getHeartBeat(uint8_t *heartData, unsigned char *bpm, char *spo2,
                        int *heartLen, uint8_t *hArrAct, uint8_t *sArrAct,
                        uint8_t *hArrAnt, uint8_t *sArrAnt)
{
  int lenAnt = 0;
  int lenAct = 0;
  uint16_t retries;

  vSerialQ_Init();  // RX_QUEUE Init.

  //for(retries = 0; retries < 100; retries++) {  
  while(1) {
    getYoapySerialData(bpm, spo2, &lenAct, hArrAct, sArrAct, hArrAnt, sArrAnt);     
    //PRINTF("getYoapySerialData spo2 %d bpm %d lenAct %d\n", *spo2, *bpm, lenAct);

    //getEmulatedSerialData(bpm,spo2,&lenAct,lenAnt, hArrAct, sArrAct, hArrAnt, sArrAnt);    

    if(lenAnt) {
        // syncWaveECG of hArrAnt
        //if((syncWaveECG(hArrAnt,(lenAnt % 2 == 0 ? lenAnt / 2 : lenAnt / 2 + 1),
          //              hArrAct,(lenAct % 2 == 0 ? lenAct / 2 : lenAct / 2 + 1))))

        if (1) {
    		//vPrintf("va a sincronizar\n");
	    	//vPrintf("sincronizado\n");
            *heartLen = ((lenAnt + lenAct) / 2);
            //Now we can make the complete SPQRST wave
            memset(heartData, '\0', MAX_SIZE);        
            memcpy(heartData, hArrAnt, lenAnt/2);
            memcpy(heartData+(lenAnt/2),hArrAct,lenAct/2);
            uint16_t k;
            PRINTF("Curva Pulsaciones por minuto: \n");
            for(k = 0; k < *heartLen; k++) {
                PRINTF("%x ", heartData[k]);
            }
            PRINTF("\n");

            PRINTF("Curva ECG: \n");
            PRINTF("sArrAnt: \n");
            for(k = 0; k < lenAnt; k++) {
                PRINTF("%x ", sArrAnt[k]);
            }
            PRINTF("sArrAct: \n");
            for(k = 0; k < lenAct; k++) {
                PRINTF("%x ", sArrAct[k]);
            }
            PRINTF("\n");

            return 1;
      
        } else {
            lenAnt = 0;
            PRINTF("Syncro failed\n");
            continue;
        }


    }
    // Extraemos una nueva trama para analizar el siguiente latido
    memcpy(sArrAnt, sArrAct, (lenAct % 2 == 0 ? lenAct / 2 : lenAct / 2 + 1));
    memcpy(hArrAnt, hArrAct, lenAct / 2);
    lenAnt = lenAct;
  }
}
/*---------------------------------------------------------------------------*/
/*-----------------------------UDP Receiver----------------------------------*/
/*---------------------------------------------------------------------------*/
clock_t send_time;
clock_t receive_time;
clock_t yoapy_init_time;
uint8_t yoapy_time;
//static int responseReceived = 1;

static void tcpip_handler(void)
{
  char *buff;
  //if we have received a response..
  if(uip_newdata()) {
    ((char *)uip_appdata)[uip_datalen()] = 0;

    buff = ((char *)uip_appdata);
    //client_conn = udp_new(&(server_conn->ripaddr), UIP_HTONS(54568), NULL);
    PRINT6ADDR(&(server_conn->ripaddr));
	
    memset(&server_conn->ripaddr, 0, sizeof(server_conn->ripaddr));
    server_conn->rport = 0;
  }
}

int errDetected = 0;
/*---------------------------------------------------------------------------*/
/*---------------------------------MDNS--------------------------------------*/
/*---------------------------------------------------------------------------*/

static const uip_ipaddr_t resolv_mdns_addr = {
		.u8 = { 
			0xff, 0x05, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0xfb,
			}
		};

static void init_mdns_announcement_connection(void){
		
		uip_ipaddr_t gwaddr;
		
		set_global_address();
		
		set_connection_address(&gwaddr);
		
		announcement_conn = udp_new(&resolv_mdns_addr,UIP_HTONS(MDNS_PORT), NULL);
		
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
	char buf[256];
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
	uip_udp_packet_send(announcement_conn, buf, len);
}




/*---------------------------------------------------------------------------*/
static char spo2 = 0;
static unsigned char bpm = 0;

PROCESS_THREAD(pearl100_process, ev, data)
{
  static char par_name[4] = "ecg\0";
  static char par_service[11] = "_mdns._udp\0";
  static char par_target[9] = "e-health\0";
  static char par_txt[128] = "rt=e-health;gps=Murcia;geo=38.023433@-1.174743;ip=2001:720:1710:11::5\0";	
	
  PROCESS_BEGIN();
  vUART_printInit();
  vUART_DataInit();
  //struct CompressedWave cwave;
  
  static int heartLen = 0;
  static uint8_t hArrAct[MAX_SIZE/2];
  static uint8_t sArrAct[MAX_SIZE/2];
  static uint8_t hArrAnt[MAX_SIZE/2];
  static uint8_t sArrAnt[MAX_SIZE/2];
  static uint8_t GreenLed = 1, OrangeLed = 1;

  memset(sArrAct, 0, MAX_SIZE/2);
  memset(hArrAct, 0, MAX_SIZE/2);
  memset(sArrAnt, 0, MAX_SIZE/2);
  memset(hArrAnt, 0, MAX_SIZE/2);
  PRINTF("Starting PEARL100...  SICSLOWPAN_PANID: %x\n", SICSLOWPAN_PANID);
  
  set_udp_connection();
  init_mdns_announcement_connection();
 
  vAHI_DioSetDirection(0, E_AHI_DIO2_INT);  // Green
  vAHI_DioSetDirection(0, E_AHI_DIO3_INT);  // Orange

  etimer_set(&et, CLOCK_SECOND);
  errDetected = 0;
  while(1) {
    PROCESS_YIELD();
		
    //UDP Response Manager
    if(ev == tcpip_event) {
		//if(server_conn->lport ==  54568){
			vPrintf("Packete david recibido\n");
			uip_ipaddr_copy(&server_conn->ripaddr, &UDP_HDR->srcipaddr);
			server_conn->rport = UDP_HDR->srcport;
			tcpip_handler();
		//}
    }
   
	vPrintf("Anunciando...\n");
	if(isAssociated()){

		if(GreenLed == 1) {
            GreenLed = 0;
            vAHI_DioSetOutput(0, E_AHI_DIO2_INT); // Green OFF
        } else {
            GreenLed = 1;
            vAHI_DioSetOutput(E_AHI_DIO2_INT, 0); // Green ON
        }
		
		mdns_SRV_announcement(par_name,strlen(par_name),
							par_service,strlen(par_service),
							par_target,strlen(par_target),
							par_txt,strlen(par_txt));
	}

    vPrintf("Preparando para enviar onda\n");
    if(client_conn != NULL && isAssociated() && getHeartBeat(heartData,&bpm,&spo2,&heartLen, hArrAct, sArrAct, hArrAnt, sArrAnt)) {
     
        if(OrangeLed == 1) {
           OrangeLed = 0;
           vAHI_DioSetOutput(0, E_AHI_DIO3_INT); // Orange OFF
        } else {
           OrangeLed = 1;
           vAHI_DioSetOutput(E_AHI_DIO3_INT, 0); // Orange ON
        }

        timeout_handlerRAW(bpm,spo2,heartLen);
        vPrintf("onda enviada\n");
  
	
    }
    vPrintf("fin obtencion de onda\n");
    if(etimer_expired(&et)) {
      etimer_restart(&et);
    }
  }

  PRINTF("Ending PEARL100...\n");
  PROCESS_END();
}

