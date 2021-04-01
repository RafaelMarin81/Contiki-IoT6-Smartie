#include <stdlib.h>
#include <string.h>

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "juart.h"

#define DNS_FLAG1_RESPONSE        0x80
#define DNS_FLAG1_OPCODE_STATUS   0x10
#define DNS_FLAG1_OPCODE_INVERSE  0x08
#define DNS_FLAG1_OPCODE_STANDARD 0x00
#define DNS_FLAG1_AUTHORATIVE     0x04
#define DNS_FLAG1_TRUNC           0x02
#define DNS_FLAG1_RD              0x01
#define DNS_FLAG2_RA              0x80
#define DNS_FLAG2_ERR_MASK        0x0f
#define DNS_FLAG2_ERR_NONE        0x00
#define DNS_FLAG2_ERR_NAME        0x03

/** \internal The DNS message header. */
struct dns_hdr {
  u16_t id;
  u8_t flags1, flags2;

  u16_t numquestions;
  u16_t numanswers;
  u16_t numauthrr;
  u16_t numextrarr;
}/*__attribute__((packed))*/;




/** \internal The DNS answer message structure. */
struct dns_answer {
  /* DNS answer record starts with either a domain name or a pointer
     to a name already present somewhere in the packet. */
  u16_t type;
  u16_t class;
  u16_t ttl[2];
  u16_t len;
#if UIP_CONF_IPV6
  u8_t ipaddr[16];
#else
  u8_t ipaddr[4];
#endif
};


/** \internal The DNS question message structure. */
struct dns_question {
  u16_t type;
  u16_t class;
}/*__attribute__((packed))*/;


#define DNS_TYPE_A		(1)
#define DNS_TYPE_CNAME	(5)
#define DNS_TYPE_PTR	(12)
#define DNS_TYPE_MX		(15)
#define DNS_TYPE_TXT	(16)
#define DNS_TYPE_AAAA	(28)
#define DNS_TYPE_SRV	(33)
#define DNS_TYPE_ANY	(255)

#define DNS_CLASS_IN	(1)
#define DNS_CLASS_ANY	(255)
#define DNS_CLASS_QU	(0x8001)

#ifndef DNS_PORT
#define DNS_PORT	(53)
#endif

#ifndef MDNS_PORT
#define MDNS_PORT	(5353)
#endif

#ifndef MDNS_RESPONDER_PORT
#define MDNS_RESPONDER_PORT	(5354)
#endif

#define RESOLV_CONF_MAX_DOMAIN_NAME_SIZE 32


struct rr_entry_srv {
    char name[2];
	u16_t type;
	u16_t class;
	u32_t ttl;
	u16_t len;
	u16_t priority;
	u16_t weight;
	u16_t port;
	u8_t strl;
};

struct rr_entry_txt {
        char name[2];
	u16_t type;
	u16_t class;
	u32_t ttl;
	u16_t len;	
	u8_t strl;
	char txt[128];
};

struct rr_entry_ptr {
	u16_t type;
	u16_t class;
	u32_t ttl;
	u16_t len;	
	u8_t strl;
};

int dns_insert_compress_service_type(char *data, int data_len, char *buf);
int create_dns_header(char *buf, struct dns_hdr *hdr, u16_t numquestions, u16_t numanswers, u16_t numauthrr, u16_t numextrarr,u8_t f1, u8_t f2);
int create_query_header(char *buf, struct dns_question *qtion, char *name, int name_len, char *service, int s_len, u16_t type, u16_t class);
int create_auth_SRV(char *buf, struct rr_entry_srv *entry, char *target, int target_len, u16_t port, u16_t type, u16_t class, int ttl);
int create_auth_TXT(char *buf, struct rr_entry_txt *txt, char *txtadds, int add_len, u16_t type, u16_t class, int ttl);
int create_auth_PTR(char *buf, struct rr_entry_ptr *ptr, char *name, int name_len, char *domain, int domain_len, u16_t type, u16_t class, int ttl, int len_act); 
int parse_dns_question(char *buf,void (* query_action)(u16_t type,char *qname, int qname_len));


