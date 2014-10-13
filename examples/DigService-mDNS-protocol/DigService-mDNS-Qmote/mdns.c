#include "mdns.h"

//Descomprime el formato DNS
int convertPointToLenght(char *data){
	int i;
	int j;
	int lastj = 0;
	int acum = 0;
	int firstLen = 0;

	for(j=0;j<strlen(data);j++){
				
		if(data[j] == '.'){
			lastj = j;
			break;					
		}
		acum++;			
	}
	firstLen = acum;


	for(i=lastj;i<strlen(data);i++){
	  	if(data[i] == '.'){
			acum = 0;
			for(j=i+1;j<strlen(data);j++){
				
				if(data[j] == '.'){
					lastj = j;
					break;					
				}
				acum++;			
			}
			data[i] = acum;
			i=lastj;
		}

	}
	return firstLen;

}

//Convert the service_type String in a dns_service_compressed_type and inserting it in the data buffer
int dns_insert_compress_service_type(char *data, int data_len, char *buf){
	int i;
	int j;
	int lastj = 0;
	int acum = 0;
	int firstLen = 0;

	for(j=0;j<data_len;j++){
		
		if(data[j] == '.'){
			lastj = j;
			break;					
		}else{
			buf[j+1] = data[j];	
		}
		acum++;			
	}
	buf[0] = acum;


	for(i=lastj;i<data_len;i++){
	  	if(data[i] == '.'){
			acum = 0;
			for(j=i+1;j<data_len;j++){
				
				if(data[j] == '.'){
					lastj = j;
					break;					
				}else{
					buf[j+1] = data[j];
				}
				acum++;			
			}
			buf[i+1] = acum;
			i=lastj;
		}else{
			buf[i+1] = data[i];
		}

	}
	return firstLen;

}

//Create a dns header
int create_dns_header(char *buf, struct dns_hdr *hdr, u16_t numquestions, u16_t numanswers, u16_t numauthrr, u16_t numextrarr, u8_t f1, u8_t f2){
	int len =0;
	
	hdr->id = 1;
	hdr->flags1 = f1;
	hdr->flags2 = f2;
	hdr->numquestions = numquestions;
	hdr->numanswers = numanswers;
	hdr->numauthrr = numauthrr;
	hdr->numextrarr = numextrarr;
	memcpy(buf,hdr,sizeof(*hdr));
	len = sizeof(*hdr);
	return len;
}

int create_query_header(char *buf, struct dns_question *qtion, char *name, int name_len, char *service, int s_len, u16_t type, u16_t class){
	int len = 0;
	
	//Adding label length
	*buf = name_len;
	len++;
	
	//Adding label 
	memcpy(buf+len,name,name_len);
	len = len + name_len;

	//Adding service_type 
	dns_insert_compress_service_type(service,s_len,buf+len);
	len++;//Adding couse of lenght_byte allocated inside of service_str
	len += s_len;
	
	//End of name and service_type
	(*(buf+len))= 0;
	len++;
	
	//Adding question data
	qtion->type = (type);
	qtion->class = (class);	
	memcpy(buf+len,(u8_t *)qtion, sizeof(struct dns_question));
	len = len + sizeof(*qtion);
	return len;

}

int create_auth_SRV(char *buf, struct rr_entry_srv *entry, char *target, int target_len, u16_t port, u16_t type, u16_t class, int ttl){
	int len = 0;
	
	//Adding Authoritative data
	entry->name[0] = 0xc0;
	entry->name[1] = 0x0c;
	entry->type = type;
	entry->class = class;
	entry->ttl = ttl;
	
	entry->priority = 0;
	entry->weight = 0;
	entry->port = port;
	entry->strl = target_len;
	entry->len = 6+entry->strl+2;
	
	//entry.target[entry.strl] = 0xc0;
	//entry.target[entry.strl+1] = 0x2b;
	
	memcpy(buf,(u8_t *)entry, sizeof(struct rr_entry_srv));
	len = sizeof(struct rr_entry_srv);
	
	
	memcpy(buf+len,target, target_len);
	len += target_len;
	len++;
	return len;
}

int create_auth_TXT(char *buf, struct rr_entry_txt *txt, char *txtadds, int add_len, u16_t type, u16_t class, int ttl){
	int len = 0;
	
	//Adding Authoritative txt data
	txt->name[0] = 0xc0;
	txt->name[1] = 0x0c;
	txt->type = type;
	txt->class = class;
	txt->ttl = ttl;
	txt->strl = add_len;
	txt->len = txt->strl+1;
	memcpy(&(txt->txt),txtadds, add_len);

	memcpy(buf+len,(u8_t *)txt, sizeof(*txt));
	len += sizeof(*txt)-(32-add_len);
	return len;
}

int create_auth_PTR(char *buf, struct rr_entry_ptr *ptr, char *name, int name_len, char *domain, int domain_len, u16_t type, u16_t class, int ttl, int len_act){
	
	int len = 0;
	
	//Adding label length
	*buf = name_len;
	len++;
	
	//Adding label 
	memcpy(buf+len,name,name_len);
	len = len + name_len + 1;

	ptr->type = type;
	ptr->class = class;
	ptr->ttl = ttl;
	ptr->strl = domain_len;
	ptr->len = ptr->strl+1;
	//copy ptr struct to buffer
	memcpy(buf+len,(u8_t *)ptr, sizeof(*ptr));
	len += sizeof(*ptr)-1;	
	
	//Adding service_type 
	dns_insert_compress_service_type(domain,domain_len,buf+len);
	len += domain_len+1;//string + lenstr
	
	buf[len] = 0xc0;
	buf[++len]= len_act;
	len++;
	
	return len;
}

int get_dns_header(char *buf, struct dns_hdr *hdr){	
	memcpy(hdr,buf,sizeof(struct dns_hdr));	
	return sizeof(struct dns_hdr);
}

int get_dns_query(char *buf, struct dns_question *qtion, char *name){
	int i;
	for(i=0;i<64;i++){
		if(*(buf+i) < 48){
			name[i] = '.';
		}else{		
			name[i] = *(buf+i);
		}
		if(*(buf+i) == '\0'){
			i++;
			break;
		}
	}
	qtion->type = buf[i];i++;
	qtion->type = qtion->type <<8;
	qtion->type = qtion->type | buf[i];i++;
	qtion->class = buf[i];i++;
	qtion->class = qtion->class << 8;
	qtion->class = qtion->class | buf[i];i++;
	return i;
}

int get_dns_PTR(char *buf, struct rr_entry_ptr *ptr, char *ptrname, char *domain){
	int i;
	for(i=0;i<64;i++){
		if(*(buf+i) < 48){
			ptrname[i] = '.';
		}else{		
			ptrname[i] = *(buf+i);
		}
		if(*(buf+i) == '\0'){
			i++;
			break;
		}
	}
	ptr->type = buf[i];i++;
	ptr->type = ptr->type <<8;
	ptr->type = ptr->type | buf[i];i++;
	ptr->class = buf[i];i++;
	ptr->class = ptr->class << 8;
	ptr->class = ptr->class | buf[i];i++;
	
	ptr->ttl = buf[i];i++;
	ptr->ttl = ptr->ttl << 8;
	ptr->ttl = ptr->ttl | buf[i];i++;
	ptr->ttl = ptr->ttl << 8;
	ptr->ttl = ptr->ttl | buf[i];i++;
	ptr->ttl = ptr->ttl << 8;
	ptr->ttl = ptr->ttl | buf[i];i++;
	
	ptr->len = buf[i];i++;
	ptr->len = ptr->len << 8;
	ptr->len = ptr->len | buf[i];i++; 
	
	ptr->strl = buf[i];i++;
	for(i=i;i<64;i++){
		if(*(buf+i) < 48){
			domain[i] = '.';
		}else{		
			domain[i] = *(buf+i);
		}
		if(*(buf+i) == '\0' || *(buf+i) == 0x0c){
			i++;
			break;
		}
	}	
	return i;
}

int parse_dns_question(char *buf, void (* query_action)(u16_t type,char *qname, int qname_len)){
	int len=0;
	struct dns_hdr hdr;	
	struct dns_question qtion;
	char name[64];
	memset(&hdr,'\0',sizeof(struct dns_hdr));
	
	len = get_dns_header(buf, &hdr);
	//vPrintf("id: %x, flag1: %x, flag2: %x, questions: %x, answers: %x, authrr: %x, extrarr: %x \n",hdr.id, hdr.flags1, hdr.flags2, hdr.numquestions, hdr.numanswers, hdr.numauthrr, hdr.numextrarr);
	if(hdr.numquestions > 0){
		//for(i=0;i<hdr.numquestions;i++){
			len += get_dns_query(buf+len, &qtion, name);
			if((*query_action) != NULL && qtion.class == DNS_CLASS_QU){
				(*query_action)(qtion.type, name, strlen(name));
				return 1;
			}
			
		//}
	
	}
	return 0;

}

