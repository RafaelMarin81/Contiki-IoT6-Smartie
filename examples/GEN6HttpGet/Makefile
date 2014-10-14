all: gen6get.jndevkit.hex

TARGET=jndevkit

PROJECT_SOURCEFILES = sht-sensor.c
#APPS=webserver 
UIP_CONF_IPV6=1
CFLAGS += -DUIP_CONF_TCP=1
CONTIKI = ../..
include $(CONTIKI)/Makefile.include


#clean:
#	rm -fr *.o yoapy.hex *.ghc
