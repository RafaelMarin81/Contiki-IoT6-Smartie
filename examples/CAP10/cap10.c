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
#include "juart.c"

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

#define MAX_DATA_CAP 500

/*---------------------------------------------------------------------------*/
PROCESS(cap10_process, "cap10 process");
AUTOSTART_PROCESSES(&cap10_process);
/*---------------------------------------------------------------------------*/

static u16_t addr_dest[8]; //address dest

static struct uip_udp_conn *client_conn;
static struct uip_udp_conn *server_conn;

static uip_ipaddr_t addDes;
static struct etimer et;
static uip_ipaddr_t ipaddr;

struct Point{
	int x;
	int y;
};

//Onda comprimida obtenida mediante un capnógrafo
struct CapWave {
    struct Point pI;
    struct Point pD;
    int etCO2;
    int BRpm;
    int d;
    char diag;
};

/*---------------------------------------------------------------------------*/

double m(int x1, int y1, int x2, int y2){
        return (((double)(y2-y1))/((double)(x2-x1)));
}

/*---------------------------------------------------------------------------*/

//Si ha fallado devuelve la estructura con un -1 en la x e y
int processWave(struct CapWave *cw, int *data, int len, int etCO2, int BRpm){
        struct Point pointI;
        struct Point pointD;
	int i;
	int lastPoint = 0;
	pointI.x = -1;
	pointD.x = -1;
	pointI.y = -1;
	pointD.y = -1;
	PRINTF("Starting wave compression process...\n");
        for(i=0;i<len;i++){
	    	if(data[i] != data[lastPoint]){
		    	if(i>1 && data[i] > 3 && abso(m(lastPoint,data[lastPoint],(i),data[i])) < 0.1){		
				pointI.x = lastPoint;
				pointI.y = data[lastPoint];
				break;
			}
		    	lastPoint = i;
	   	}
        }
	lastPoint = len-1;	

        for(i=len-1;i>-1;i--){
		if(data[i] != data[lastPoint]){
			if(i<len-1 && data[i] >3 && abso(m(i,data[i],(lastPoint),data[lastPoint])) < 0.1){
				pointD.x = lastPoint;
				pointD.y = data[lastPoint];
				break;
			}
			lastPoint = i;
		}
        }
	//PRINTF("{");
	/*for(i=0;i<len;i++){
		PRINTF("%d,",data[i]);

	}*/
	//PRINTF("}\n");
	PRINTF("Ending wave compression process...\n");
  if(pointI.x == -1 || pointD.x == -1){
      PRINTF("Error Wave!\n");
      return 0;
  }
	PRINTF("Correct Wave!!\n");
	cw->pI.x = pointI.x;
	cw->pI.y = pointI.y;
	cw->pD.x = pointD.x;
	cw->pD.y = pointD.y;
	cw->etCO2 = etCO2;
	cw->BRpm = BRpm;
	cw->d = len;
        return 1;
}

/*---------------------------------------------------------------------------*/
int getCap10SerialData(struct CapWave *cw){
	int comienzo = 0;
        int fin=0;
        //int finalizando = 0;
	int data;
	int d[MAX_DATA_CAP];
	int dpos = 0;
	int maxetCO2 = 0;
	int breaths = 0;

	while(!fin)
	{
		data = uGetC_UART1();		
		switch (data) {
                    case 248://Init data frame
                        break;
                    case 254://End data frame
                        break;
                    case 249://Max etCO2
                        maxetCO2 = uGetC_UART1();
                        break;
                    case 250://Breath per minute
                        breaths = uGetC_UART1();
                        /*if(finalizando)
                            fin = 1;*/
                        break;
                    default://The rest of data wave
			//Adding wave point
                        if(comienzo && !((data == 1 || data == 0)&& d[dpos-1] == 1)){
			    d[dpos] = data;
			    dpos++;
			}
			//If we are in the init of the wave
			if((data == 1 || data == 0) && !comienzo){
				comienzo = 1;
	                	d[0] = 1;
			    	dpos++;
				//PRINTF("Comenzando a insertar...\n");
	                    	while((data = uGetC_UART1()) == 1 
					|| !data);
			//If we are in the end of the wave
			}else if((data == 1 || data == 0) && comienzo){
				fin = 1;
				//PRINTF("Finalizando insertar... dpos = %d\n",dpos);
			}
                        
                }

	}
	if(dpos == 1 || dpos == 0)return 0;
	//clock_init();
	//ctiempo = clock_time();
	//finalizando = processWave(cw, d,dpos, maxetCO2, breaths);
	//PRINTF("clock subtraction: %i CLOCK_PER_SECOND: %d \n", (unsigned long)(clock_time() - ctiempo), CLOCK_CONF_SECOND);
	//PRINTF("%i\n",(unsigned long)(clock_time() - ctiempo));
	//return finalizando;
	return processWave(cw, d,dpos, maxetCO2, breaths);
	
	
	
}
/*---------------------------------------------------------------------------*/

void printData(struct CapWave *cw){

	PRINTF("\n--------- Values to reconstruct ---------\n");

        PRINTF("\tLeft Point: (%d,%d)\n",(int)cw->pI.x,(int)cw->pI.y);
	PRINTF("\tRight Point: (%d,%d)\n",(int)cw->pD.x,(int)cw->pD.y);
	PRINTF("\tDistance: %d\n",(int)cw->d);
	PRINTF("\tetCO2: %d\n",(int)cw->etCO2);
	PRINTF("\tBreaths per minute: %d\n",(int)cw->BRpm);

}

static void
timeout_handlerYOAPY(struct CapWave *cwave)
{
  //static int seq_id;
  char buf[MAX_PAYLOAD_LEN];

  buf[0] = (uint8_t)cwave->pI.x;
  buf[1] = (uint8_t)cwave->pI.y;
  
  buf[2] = U16_UPPER_U8((uint16_t)cwave->pD.x);
  buf[3] = U16_LOWER_U8((uint16_t)cwave->pD.x);
  //buf[2] = (cwave->pD.x >> 8) & 0xFF;
  //buf[3] = cwave->pD.x & 0xFF;
  
  buf[4] = (uint8_t)cwave->pD.y;
  
  buf[5] = U16_UPPER_U8((uint16_t)cwave->d);
  buf[6] = U16_LOWER_U8((uint16_t)cwave->d);
  //buf[5] = (cwave->d>>8) & 0xFF;
  //buf[6] = (cwave->d) & 0xFF;
  
  buf[7] = (uint8_t)cwave->etCO2;
  buf[8] = (uint8_t)cwave->BRpm;
  buf[9] = cwave->diag;
  
  uip_udp_packet_send(client_conn, buf, 10);

}


/*---------------------------------------------------------------------------*/

static void
set_global_address(void)
{

    addr_dest[0] = 0x2001;
    addr_dest[1] = 0x720;
    addr_dest[2] = 0x1710;
    addr_dest[3] = 0x13;
    addr_dest[4] = 0;
    addr_dest[5] = 0;
    addr_dest[6] = 0;
    addr_dest[7] = 0x9;


  	uip_ip6addr(&ipaddr, addr_dest[0], addr_dest[1],addr_dest[2],
                addr_dest[3],addr_dest[4],addr_dest[5],addr_dest[6],addr_dest[7]);


	// Not used next line, since it is global
	// uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr); //set the last 64 bits of an IP address based on the MAC address
	 uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL); // ADDR_AUTOCONF

}
/*---------------------------------------------------------------------------*/
static void
set_connection_address(uip_ipaddr_t *addr)
{

	uip_ip6addr(addr,0x2001, 0x720, 0x1710, 0x13, 0, 0, 0, 0x1);
  //uip_ip6addr(ipaddr,0xfe80, 0, 0, 0, 0x215, 0x8dff, 0xfe0b, 0x1cf0);
}
/*---------------------------------------------------------------------------*/
static void
send_packet_udp(void)
{
	
  	
	PRINTF("UDP client process started\n");

  	set_global_address();

  	set_connection_address(&addDes);

	/* new connection with remote host */
	client_conn = udp_new(&addDes, UIP_HTONS(54568), NULL);
	//udp_bind(client_conn, UIP_HTONS(54568));

	PRINTF("Created a connection with the server ");
	PRINTF("local/remote port %d/%d\n", UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));
	
	
}



static void
tcpip_handler(void)
{
 // static int seq_id;
 // char buf[MAX_PAYLOAD_LEN];

  if(uip_newdata()) {
    //tiempo = (clock_time()-tiempo)/CLOCK_SECOND;
	
    ((char *)uip_appdata)[uip_datalen()] = 0;
    //PRINTF("Server received: '%s' \n", (char *)uip_appdata);
    //PRINTF("%d\n",tiempo);
    //PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    //PRINTF("\n");

/*    uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    PRINTF("Responding with message: ");
    sprintf(buf, "Hello from the server! (%d)", ++seq_id);
    PRINTF("%s\n", buf);*/

    //uip_udp_packet_send(server_conn, buf, strlen(buf));
    /* Restore server connection to allow data from any node */
    memset(&server_conn->ripaddr, 0, sizeof(server_conn->ripaddr));
  }
}

/*---------------------------------------------------------------------------*/


struct CapWave cw;

PROCESS_THREAD(cap10_process, ev, data)
{
    PROCESS_BEGIN();
  
    vUART_printInit();
    vUART_DataInit();
    
    
    send_packet_udp();
    etimer_set(&et, 1000);
    PRINTF("Iniciando CAP10...\n");
    /*cw.pI.x=20;
    cw.pI.y=19;
    cw.pD.x=240;
    cw.pD.y=24;
    cw.etCO2=35;
    cw.BRpm=21;
    cw.d=260;
    cw.diag=0;*/

    while(1){		
        PROCESS_YIELD(); //Useful when more of one thread
        if(isAssociated() && getCap10SerialData(&cw)){
            
            //PRINTF("Printing...\n");
    		//printData(&cw);
			//break;			  		
            timeout_handlerYOAPY(&cw);
        }else{
            PRINTF("No data found!\n");
        }

        //UDP Response Manager
        if(ev == tcpip_event) {       
            tcpip_handler();
        }
        if(etimer_expired(&et)) { 
            etimer_restart(&et);
        }
    }		
    PRINTF("Fin CAP10...\n");

	PROCESS_END();
}

/*---------------------------------------------------------------------------*/










