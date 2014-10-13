

//PROCESS(mdns_process, "mDNS process");
//PROCESS(mDNSDiscover_process, "mDNSDiscover process");
//AUTOSTART_PROCESSES(&mDNSDiscover_process, &mdns_process);

/*PROCESS_THREAD(mdns_process, ev, data)
{  	
	//static char ptrname[11] = "_coap._udp\0";
	//static char domname[5] = "temp\0";

	PROCESS_BEGIN();
		
		init_udp_connection();
		while(1) {
			PROCESS_WAIT_EVENT();
			if(ev == PROCESS_EVENT_TIMER) {					
				uip_udp_packet_send(resolv_conn, buf, data);
				//mdns_PTR_query(((char *)ptrname),10,((char *)domname),4);

			}
		}
	PROCESS_END();
}*/


/*-----------------------------------------------------------------------------------*/
/** \internal
 * The main UDP function.
 */
/*-----------------------------------------------------------------------------------*/


/*static void tcpip_manager(void){
	int seq_id=0;
	char buff[200];
	memset(buff,'\0',200);
	if(uip_newdata()) {    
   
    ((char *)uip_appdata)[uip_datalen()] = 0;
    vPrintf("Server received: '%s' \n", (char *)uip_appdata);    
	
	if((*(&UDP_HDR->destport)) == 1234){
		vPrintf("Temperature service petition received\n");
		uip_ipaddr_copy(&serviceTemperature->ripaddr, &UDP_HDR->srcipaddr);
		vPrintf("Responding with Temperature message: ");
		sprintf(buff, "Temperature: %d", getTemperature());
		vPrintf("%s\n", buff);

		uip_udp_packet_send(serviceTemperature, buff, strlen(buff));
	}*/
    /* Restore server connection to allow data from any node */
   /* memset(&serviceTemperature->ripaddr, 0, sizeof(serviceTemperature->ripaddr));
  }
	
}*/