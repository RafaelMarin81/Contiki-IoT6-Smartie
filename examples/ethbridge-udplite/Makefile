CONTIKI_PROJECT = ethbridge
all: $(CONTIKI_PROJECT)

CONTIKI_TARGET_SOURCEFILES = cdc_dev.c 

APPS = ethbridge
UIP_CONF_IPV6=1

CFLAGS+=-DJENNIC_CONF_COORDINATOR

CONTIKI = ../..
include $(CONTIKI)/Makefile.include
