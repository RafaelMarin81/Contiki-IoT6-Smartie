#yoapy.hex: pearl100.c yoapy.c yoapy.h simpleList.c simpleList.h
#	gcc simpleList.c simpleList.h yoapy.c yoapy.h pearl100.c -o yoapy.hex
all: digservice.jndevkit.hex

TARGET=jndevkit

PROJECT_SOURCEFILES = coap.c lightdriver.c OMA.c

UIP_CONF_IPV6=1
UIP_CONF_TCP=0


#CFLAGS += -DUIP_CONF_TCP=0                  ## Disable the TCP support.

CONTIKI = ../..
include $(CONTIKI)/Makefile.include



#clean:
#	rm -fr *.o *.hex *.jndevkit *.ghc

