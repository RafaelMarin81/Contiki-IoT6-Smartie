
all: pearl100.jndevkit.hex
	
#PROJECT_SOURCEFILES = mdns.c
UIP_CONF_IPV6=1

TARGET=jndevkit

CFLAGS+=-DQUEUEBUF_CONF_NUM=4

CONTIKI = ../..
include $(CONTIKI)/Makefile.include

