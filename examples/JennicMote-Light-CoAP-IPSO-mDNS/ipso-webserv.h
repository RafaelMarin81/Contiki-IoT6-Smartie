#include <stdlib.h>
#include <string.h>

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "juart.h"
#include "coap.h"


	/*
	 * Author: David Fernandez Ros, Pablo Lopez Martinez
	 * email: david.f.r@um.es, p.lopezmartinez@um.es
	 */

/// IPSO Interface Description

//+-------------------+----------+------------------------------------+
//|         Interface | if=      | Methods                            |
//+-------------------+----------+------------------------------------+
//|         Link List | core.ll  | GET                                |
//|             Batch | core.b   | GET, PUT, POST (where applicable)  |
//|      Linked Batch | core.lb  | GET, PUT, POST, DELETE (where      |
//|                   |          | applicable)                        |
//|            Sensor | core.s   | GET                                |
//|         Parameter | core.p   | GET, PUT                           |
//|         Read-only | core.rp  | GET                                |
//|         Parameter |          |                                    |
//|          Actuator | core.a   | GET, PUT, POST                     |
//|           Binding | core.bnd | GET, POST, DELETE                  |
//+-------------------+----------+------------------------------------+
   
//#define if_linklist "ll"
//#define if_batch "b"
//#define if_linkedbatch "lb"
//#define if_sensor "s"
//#define if_parameter "p"
//#define if_readonly "rp"
//#define if_actuator "a"
//#define if_binding "bnd"
//#define if_wellknow "/.well-known/core"


/// IPSO Framework Description

///							 Resource discovery
#define rd_wellknown "/.well-known/core"

///                          IPSO  Generic Resource Types
//+--------------------+-----------+--------------+
//| Function Set       | Root Path | Resouce Type |
//+--------------------+-----------+--------------+
//| Device 	           | /dev      | ipso.dev  	  |
//| General Purpose IO | /gpio     | ipso.gpio 	  |
//| Power              | /pwr      | ipso.pwr  	  |
//| Load Control       | /load     | ipso.load 	  |
//| Sensors            | /sen  	   | ipso.sen  	  |
//| Light Control      | /lt   	   | ipso.lt   	  |
//| Message            | /msg  	   | ipso.msg  	  |
//| Location           | /loc  	   | ipso.loc  	  |
//| Configuration      | /cfg  	   | ipso.cfg  	  |
//+--------------------+-----------+--------------+

// Root paths
///#define rt_dev_path 			"/dev"
///#define rt_gpio_path 		"/gpio"
//#define rt_pwr_path 			"/pwr"
//#define rt_load_path 			"/load"
//#define rt_sen_path 			"/sen"
///#define rt_lt_path 			"/lt"
///#define rt_msg_path 			"/msg"
///#define rt_loc_path 			"/loc"
///#define rt_cfg_path 			"/cfg"


///                          IPSO.DEV   
//+-------------+---------------+----------------+------+------+------+
//| Type        | Path          | RT             | IF   | Type | Unit |
//+-------------+---------------+----------------+------+------+------+
//| Manufacturer| /dev/mfg      | ipso.dev.mfg   | rp   | s    |      | ONLY GET
//| Model       | /dev/mdl      | ipso.dev.mdl   | rp   | s    |      |	ONLY GET
//| Hardware Rev| /dev/mdl/hw   | ipso.dev.mdl.hw| rp   | s    |      |	ONLY GET
//| Software Ver| /dev/mdl/sw   | ipso.dev.mdl.sw| rp   | s    |      |	ONLY GET
//| Serial      | /dev/ser      | ipso.dev.ser   | rp   | s    |      |	ONLY GET
//| Name        | /dev/n        | ipso.dev.n     | p,rp | s    |      | ONLY GET, GET-PUT
//| Power Supply| /dev/pwr/{#}  | ipso.dev.pwr   | rp   | e    |      | ONLY GET
//| Power Sup. V| /dev/pwr/{#}/v| ipso.dev.pwr.v | s    | d    | V    | ONLY GET
//| Time        | /dev/time     | ipso.dev.time  | p,rp | i    | s    | ONLY GET, GET-PUT
//| Uptime      | /dev/uptime   | ipso.dev.uptime| s    | i    | s    | ONLY GET
//+-------------+---------------+----------------+------+------+------+

// Root Paths
//#define rt_dev_mfg_path		"/dev/mfg"
///#define rt_dev_mdl_path			"/dev/mdl"
//#define rt_dev_mdl_hw_path	"/dev/mdl/hw"
//#define rt_dev_mdl_sw_path	"/dev/mdl/sw"
///#define rt_dev_ser_path			"/dev/ser"
//#define rt_dev_n_path			"/dev/n"
//#define rt_dev_pwr_path		"/dev/pwr"
//#define rt_dev_pwr_s_path		"/dev/pwr/s"
//#define rt_dev_time_path		"/dev/time"
///#define rt_dev_uptime_path		"/dev/uptime"


///							IPSO-DEV Proper defined 
//+-------------+---------------+--------------------+------+------+------+
//| Type        | Path          | RT                 | IF   | Type | Unit |
//+-------------+---------------+--------------------+------+------+------+
//| Keep-Alive  | /dev/keepalive| ipso.dev.keepalive | s    | i ?  |      | ONLY GET
//| Model       | /dev/reboot   | ipso.dev.reboot    | a    | b ?  |      | GET, PUT, POST
//+-------------+---------------+--------------------+------+------+------+

// Root Path
///#define rt_dev_keepalive_path	"/dev/keepalive"
///#define rt_dev_reboot_path		"/dev/reboot"


///							IPSO-LT
//+---------------+-------------+-------------+----+------+---------+
//| Type 	      | Path 	    | RT          | IF | Type | Unit    |
//+---------------+-------------+-------------+----+------+---------+
//| Light Control | /lt/{#}/on  | ipso.lt.on  | a  | b    |         | GET, PUT, POST
//| Light Dimmer  | /lt/{#}/dim | ipso.lt.dim | a  | i    | 0-100 % | GET, PUT, POST
//+---------------+-------------+-------------+----+------+---------+

// Root Paths
///#define rt_lt_on_path 			"/lt/on"
///#define rt_lt_dim_path 			"/lt/dim"

///							IPSO-MSG
//+---------+-------------+-----------------+------+------+------+
//| Type    | Path        | RT              | IF   | Type | Unit |
//+---------+-------------+-----------------+------+------+------+
//| Status  | /msg/status | ipso.msg.status | p,rp | s    |      |	ONLY GET in this case
//| Alarms  | /msg/alarms | ipso.msg.alarms | p,rp | s    |      |	ONLY GET in this case
//| Display | /msg/disp   | ipso.msg.disp   | p,rp | s    |      |	ONLY GET, GET-PUT
//+---------+-------------+-----------------+------+------+------+

// Root paths
///#define rt_msg_status_path 		"/msg/status"
///#define rt_msg_alarms_path 		"/msg/alarms"
//#define rt_msg_disp_path   	"/msg/disp"

///							IPSO-LOC
//+-------------------+----------+--------------+----+------+---------+
//| Type              | Path     | RT           | IF | Type | Unit    |
//+-------------------+----------+--------------+----+------+---------+
//| GPS Location      | /loc/gps | ipso.loc.gps | p  | s    | Lon,Lat |	GET, PUT    (Changed for convenience to "p")
//| XY Location       | /loc/xy  | ipso.loc.xy  | s  | s    | X,Y     | ONLY GET
//| Semantic Location | /loc/sem | ipso.loc.sem | s  | s    |         | ONLY GET
//| Fix               | /loc/fix | ipso.loc.fix | s  | b    |         | ONLY GET
//| Period            | /loc/per | ipso.loc.per | p  | i    | s       | GET, PUT
//+-------------------+----------+--------------+----+------+---------+

// Root paths
///#define rt_loc_gps_path 		"/loc/gps"
//#define rt_loc_xy_path 		"/loc/xy"
//#define rt_loc_sem_path 		"/loc/sem"
//#define rt_loc_fix_path 		"/loc/fix"
//#define rt_loc_per_path 		"/loc/per"

///							IPSO-CFG
//+----------+----------------+--------------------+----+------+------+
//| Type     | Path           | RT                 | IF | Type | Unit |
//+----------+----------------+--------------------+----+------+------+
//| Services | /cfg/services  | ipso.cfg.services  |    | s    |      |
//| Stack    | /cfg/stack     | ipso.cfg.stack     | p  | s    |      | GET, PUT
//| Stack PHY| /cfg/stack/phy | ipso.cfg.stack.phy | p  | s    |      | GET, PUT
//| Stack MAC| /cfg/stack/mac | ipso.cfg.stack.mac | p  | s    |      | GET, PUT
//| Stack NET| /cfg/stack/net | ipso.cfg.stack.net | p  | s    |      | GET, PUT
//| Stack RTG| /cfg/stack/rtg | ipso.cfg.stack.rtg | p  | s    |      | GET, PUT
//+----------+----------------+--------------------+----+------+------+

// Root Paths
//#define rt_cfg_services_path	"/cfg/services"
//#define rt_cfg_stack_path		"/cfg/stack"
//#define rt_cfg_stack_phy_path	"/cfg/stack/phy"
//#define rt_cfg_stack_mac_path	"/cfg/stack/mac"
//#define rt_cfg_stack_net_path	"/cfg/stack/net"
//#define rt_cfg_stack_rtg_path	"/cfg/stack/rtg"

///							IPSO-CFG Proper defined
//+----------+------------------------+----------------------------+----+------+------+
//| Type     | Path                   | RT                         | IF | Type | Unit |
//+----------+------------------------+----------------------------+----+------+------+
//| Stack NET| /cfg/stack/net/ip      | ipso.cfg.stack.net.ip      | p  | s    |      | GET, PUT
//| Stack NET| /cfg/stack/net/port    | ipso.cfg.stack.net.port    | p  | s    |      | GET, PUT
//| Stack NET| /cfg/stack/net/auto    | ipso.cfg.stack.net.auto    | p  | b    |      | GET, PUT
//| Stack MAC| /cfg/stack/mac/addr    | ipso.cfg.stack.mac.addr    | p  | s    |      | GET, PUT
//| Stack MAC| /cfg/stack/mac/pan     | ipso.cfg.stack.mac.pan     | p  | s    |      | GET, PUT
//| Stack MAC| /cfg/stack/mac/chan    | ipso.cfg.stack.mac.chan    | p  | i    |      | GET, PUT
//| Stack MAC| /cfg/stack/mac/sec     | ipso.cfg.stack.mac.sec     | p  | b    |      | GET, PUT
//| Stack MAC| /cfg/stack/mac/sec/key | ipso.cfg.stack.mac.sec.key | p  | s    |      | GET, PUT
//| Stack MAC| /cfg/stack/mac/sec/iv  | ipso.cfg.stack.mac.sec.iv  | p  | s    |      | GET, PUT
//+----------+------------------------+----------------------------+----+------+------+

// Root Paths
///#define rt_cfg_stack_net_ip_path		"/cfg/stack/net/ip"
///#define rt_cfg_stack_net_port_path		"/cfg/stack/net/port"
///#define rt_cfg_stack_net_auto_path		"/cfg/stack/net/auto"
///#define rt_cfg_stack_mac_addr_path		"/cfg/stack/mac/addr"
///#define rt_cfg_stack_mac_pan_path		"/cfg/stack/mac/pan"
///#define rt_cfg_stack_mac_chan_path		"/cfg/stack/mac/chan"
///#define rt_cfg_stack_mac_sec_path		"/cfg/stack/mac/sec"
///#define rt_cfg_stack_mac_sec_key_path	"/cfg/stack/mac/sec/key"
///#define rt_cfg_stack_mac_sec_iv_path	"/cfg/stack/mac/sec/iv"

///							IPSO-GPIO
//+---------------+-----------------+-----------------+----+------+-------+
//| Type          | Path            | RT              | IF | Type | Unit  |
//+---------------+-----------------+-----------------+----+------+-------+
//| Button        | /gpio/btn/{#}   | ipso.gpio.btn   | s  | i    |       | ONLY GET
//| Digital Input | /gpio/din/{#}   | ipso.gpio.din   | s  | b    |       | ONLY GET
//| Digital Output| /gpio/dout/{#}  | ipso.gpio.dout  | a  | b    |       | GET, PUT, POST
//| Analog Input  | /gpio/ain/{#}   | ipso.gpio.ain   | s  | d    | V     | ONLY GET
//| Analog Output | /gpio/aout/{#}  | ipso.gpio.aout  | a  | d    | V     | GET, PUT, POST
//| Dimmer Input  | /gpio/dimin/{#} | ipso.gpio.dimin | s  | i    | 0-100%| ONLY GET
//+---------------+-----------------+-----------------+----+------+-------+

// Root Paths
//#define rt_gpio_btn_path		"/gpio/btn"
//#define rt_gpio_din_path		"/gpio/din"
//#define rt_gpio_dout_path		"/gpio/dout"
//#define rt_gpio_ain_path		"/gpio/ain"
//#define rt_gpio_aout_path		"/gpio/aout"
//#define rt_gpio_dimin_path	"/gpio/dimin"

///							IPSO-GPIO Proper defined
//+---------------+--------------------+-------------------+----+------+-------+
//| Type          | Path               | RT                | IF | Type | Unit  |
//+---------------+--------------------+-------------------+----+------+-------+
//| Crossover     | /gpio/cross/0      | ipso.gpio.cross.0 |    |      |       |
//| Status        | /gpio/cross/status | ipso.gpio.status  | s  | b    |       |	ONLY GET
//| Frecuency     | /gpio/cross/fr     | ipso.gpio.fr      | s  | i    |       |	ONLY GET
//| User counter  | /gpio/cross/ct     | ipso.gpio.ct      | s  | i    |       |	ONLY GET
//| Rst Counter   | /gpio/cross/rst    | ipso.gpio.rst     | a  | b    |       |	GET, PUT, POST
//+---------------+--------------------+-------------------+----+------+-------+

// Root paths
///#define rt_gpio_cross_0_path		"/gpio/cross/"
///#define rt_gpio_cross_status_path	"/gpio/cross/0/status"
///#define rt_gpio_cross_fr_path		"/gpio/cross/0/fr"
///#define rt_gpio_cross_ct_path		"/gpio/cross/0/ct"
///#define rt_gpio_cross_rst_path		"/gpio/cross/0/rst"

// Main function
int parse_ipso(coap_packet_t *request, char *response, int *writePos);

// Specific functions
int response_discover_services(coap_packet_t *request, char *response,int *writePos);
int response_incomplete_ipso_command(char * response,int * writePos);
int response_unknown_ipso_command(char *response,int *writePos);
int response_unimplemented_ipso_command(char *response,int *writePos);
int response_invalid_ipso_command(char *response,int *writePos);

int response_dev_mdl(coap_packet_t *request, char *response,int *writePos);
int response_dev_ser(coap_packet_t *request, char *response,int *writePos);
int response_dev_uptime(coap_packet_t *request, char *response,int *writePos);
int response_dev_keepalive(coap_packet_t *request, char *response,int *writePos);
int response_dev_reboot(coap_packet_t *request, char *response,int *writePos);

int response_msg_status(coap_packet_t *request, char *response,int *writePos);
int response_msg_alarms(coap_packet_t *request, char *response,int *writePos);

int response_cfg_stack_net_ip(coap_packet_t *request, char *response,int *writePos);
int response_cfg_stack_net_port(coap_packet_t *request, char *response,int *writePos);
int response_cfg_stack_net_auto(coap_packet_t *request, char *response,int *writePos);
int response_cfg_stack_mac_addr(coap_packet_t *request, char *response,int *writePos);
int response_cfg_stack_mac_pan(coap_packet_t *request, char *response,int *writePos);
int response_cfg_stack_mac_chan(coap_packet_t *request, char *response,int *writePos);
int response_cfg_stack_mac_sec(coap_packet_t *request, char *response,int *writePos);
int response_cfg_stack_mac_sec_key(coap_packet_t *request, char *response,int *writePos);
int response_cfg_stack_mac_sec_iv(coap_packet_t *request, char *response,int *writePos);

int response_loc_gps(coap_packet_t *request, char *response,int *writePos);

int response_lt_on_all(coap_packet_t *request, char *response,int *writePos);
int response_lt_dim_all(coap_packet_t *request, char *response,int *writePos);
int response_lt_light0_on(coap_packet_t *request, char *response,int *writePos);
int response_lt_light0_dim(coap_packet_t *request, char *response,int *writePos);

int response_gpio_cross_0_status(coap_packet_t *request, char *response,int *writePos);
int response_gpio_cross_0_fr(coap_packet_t *request, char *response,int *writePos);
int response_gpio_cross_0_ct(coap_packet_t *request, char *response,int *writePos);
int response_gpio_cross_0_rst(coap_packet_t *request, char *response,int *writePos);
