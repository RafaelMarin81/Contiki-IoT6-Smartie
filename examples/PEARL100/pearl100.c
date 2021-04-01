/* 
 * File:   main.c
 * Author: Pablo López Martínez
 *
 * Created on 19 de diciembre de 2011, 10:54
 */

#include <stdlib.h>
#include <string.h>

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "juart.h"
#include "yoapy.c"

#define DEBUG 0
#if DEBUG
#define PRINTF(...) vPrintf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

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
static struct uip_udp_conn *client_conn;
static struct uip_udp_conn *server_conn;
uint32_t id_packet = 0;

uint8_t heartData[MAX_SIZE];

static uip_ipaddr_t addDes;
static struct etimer et;
static uip_ipaddr_t ipaddr;

static void timeout_handlerYOAPY(struct CompressedWave cwave)
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
}

static void timeout_handlerRAW(char bpm, char spo, int heartLen)
{
  char buf[MAX_SIZE];
  int i;
  buf[0] = bpm;
  buf[1] = spo;
  memcpy(buf+2, heartData, heartLen);
//  for(i = 2; i < heartLen; i++) {
//    buf[i] = heartData[i - 2];
//  }
  uip_udp_packet_send(client_conn, buf, heartLen + 2);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static void set_global_address(void)
{
  addr_dest[0] = 0x2001;
  addr_dest[1] = 0x720;
  addr_dest[2] = 0x1710;
  addr_dest[3] = 0x13;
  addr_dest[4] = 0;
  addr_dest[5] = 0;
  addr_dest[6] = 0;
  addr_dest[7] = 0x8;

  uip_ip6addr(&ipaddr, addr_dest[0], addr_dest[1], addr_dest[2], addr_dest[3],
              addr_dest[4], addr_dest[5], addr_dest[6], addr_dest[7]);

  uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL); // ADDR_AUTOCONF
}
/*---------------------------------------------------------------------------*/
static void set_connection_address(uip_ipaddr_t *addr)
{
  uip_ip6addr(addr, 0x2001, 0x720, 0x1710, 0x13, 0, 0, 0, 0x1);
}
/*---------------------------------------------------------------------------*/
static void send_packet_udp(void)
{

  PRINTF("UDP client process started\n");

  set_global_address();
  set_connection_address(&addDes);

  /* new connection with remote host */
  client_conn = udp_new(&addDes, UIP_HTONS(54568), NULL);
  //client_conn = udp_new(NULL, UIP_HTONS(54568), NULL);
  //udp_bind(client_conn, UIP_HTONS(54568));
  //uip_ipaddr_copy(&client_conn->ripaddr, &addDes); 
  PRINTF("Created a connection with the server ");

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
    if(data != 0xF7)
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
}
/*---------------------------------------------------------------------------*/
/*
 Emulated serial data to do trys

 */

int getEmulatedSerialData(unsigned char *bpm, char *spo2, int *totalLen,
                          int primera, int *hArrAct, int *sArrAct, int *hArrAnt,
                          int *sArrAnt)
{
  int i;
  int arrLen = 42;
  *bpm = 80;
  *totalLen = 42;
  *spo2 = 50;
  int arr[] = {
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
  int arrspo[] = {
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

}
/*---------------------------------------------------------------------------*/
/* This function syncronize the wave using the SPO2 wave obtained through serial port

 */
static int syncWave(int *firstSPO2, int len1, int *secondSPO2, int len2)
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
}

static int syncWaveECG(uint8_t *fECG, int len1, uint8_t *sECG, int len2){
  
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
  
}

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
  int syncType = 0;
  int lenAnt = 0;
  int lenAct = 0;
  int i = 0;
  int iAcum = 0;
  int retries = 0;
  //for(retries = 0; retries < 100; retries++) {
  while(1) {
    getYoapySerialData(bpm, spo2, &lenAct, hArrAct, sArrAct, hArrAnt, sArrAnt);
    //getEmulatedSerialData(bpm,spo2,&lenAct,lenAnt, hArrAct, sArrAct, hArrAnt, sArrAnt);
    if(lenAnt) {
      //We must to syncronize the wave using that function and the spo2 wave.
      if((syncType = syncWaveECG(hArrAnt,(lenAnt % 2 == 0 ? lenAnt / 2 : lenAnt / 2 + 1),
                              hArrAct,(lenAct % 2 == 0 ? lenAct / 2 : lenAct / 2 + 1)))) 
      {
        *heartLen = ((lenAnt + lenAct) / 2);
        //Now we can make the complete SPQRST wave
        memset(heartData, '\0', MAX_SIZE);        
//        for(i = 0; i < (lenAnt / 2); i++) {
//          heartData[i] = hArrAnt[i];
//        }
        memcpy(heartData, hArrAnt, lenAnt/2);
//        iAcum = i;
//        for(i = 0; i < lenAct / 2; i++) {
//          heartData[iAcum] = hArrAct[i];
//          iAcum++;
//        }
        memcpy(heartData+lenAnt/2,hArrAct,lenAct/2);
        return 1;

      }// else {
//        lenAnt = 0;
//        PRINTF("Syncro failed\n");
//        continue;
//      }
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
int responseReceived = 1;

static void tcpip_handler(void)
{
  char *buff;
  //if we have received a response..
  if(uip_newdata()) {
    ((char *)uip_appdata)[uip_datalen()] = 0;

    buff = ((char *)uip_appdata);

    memset(&server_conn->ripaddr, 0, sizeof(server_conn->ripaddr));
    responseReceived = 1;
  }
}

int errDetected = 0;
/*---------------------------------------------------------------------------*/
//int main()
PROCESS_THREAD(pearl100_process, ev, data)
{
  PROCESS_BEGIN();
  vUART_printInit();
  vUART_DataInit();
  struct CompressedWave cwave;
  char spo2 = 0;
  unsigned char bpm = 0;
  int heartLen = 0;

  uint8_t hArrAct[MAX_SIZE/2];
  uint8_t sArrAct[MAX_SIZE/2];
  uint8_t hArrAnt[MAX_SIZE/2];
  uint8_t sArrAnt[MAX_SIZE/2];
  int i =0;
  memset(sArrAct, 0, MAX_SIZE/2);
  memset(hArrAct, 0, MAX_SIZE/2);
  memset(sArrAnt, 0, MAX_SIZE/2);
  memset(hArrAnt, 0, MAX_SIZE/2);
  PRINTF("Starting PEARL100...\n");
  send_packet_udp();

  //UDP
  etimer_set(&et, 1000);
  errDetected = 0;

  while(1) {
    PROCESS_YIELD();
    //UDP Response Manager
    if(ev == tcpip_event) {
      tcpip_handler();
    }

    if(isAssociated() && getHeartBeat(heartData,&bpm,&spo2,&heartLen, hArrAct, sArrAct, hArrAnt, sArrAnt)) {
//      PRINTF("entra");
//      cwave = compressWave(heartData, heartLen, bpm, spo2);
//      PRINTF("termina compresion");
//      if(cwave.diag != -1) {
//
//        printData(cwave,spo2, bpm);
//
//        timeout_handlerYOAPY(cwave);
//        PRINTF("Packet sent.\n");
//      } else {
//        PRINTF("Compression Error.\n");
//      }
//      for(i=0;i<heartLen;i++) {
//        PRINTF("heartlen[%d]=%d\n",i,heartData[i]);
//		  }
      timeout_handlerRAW(bpm,spo2,heartLen);
    } else {
      PRINTF("Serial Error Exception\n");
    }
    if(etimer_expired(&et)) {
      etimer_restart(&et);
    }

  }

  PRINTF("Ending PEARL100...\n");
  PROCESS_END();
}

