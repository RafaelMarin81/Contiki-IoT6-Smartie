# all: udp-client-logger.jndevkit.hex udp-server-logger.jndevkit.hex 
#all: udp-client-logger-testing.jndevkit.hex udp-server-logger-testing.jndevkit.hex
all: udp-client-logger-leds.jndevkit.hex udp-server-logger-leds.jndevkit.hex

UIP_CONF_IPV6=1

#
# Testing-Sleep-configuration
# 
SLEEP_RADIO=1
SLEEP_RADIO_NO_LEDS=1

COMPILE_LESS_RAM=1			# Reduce 20KB RAM and 20KB Flash  in compilation of cpu/jennic/Makefile.jn51xx

#CFLAGS += -DNO_MAC_ACK		# Avoid the ACK-scheme in MAC layer.

CFLAGS += -DDEBUG_ERROR
#CFLAGS += -DMOTEN_CLIENT
#CONTIKI_SOURCEFILES += pcf8563-clock.c pcf8563-i2c.c

### This chooses the Jennic processor we're building for.
#    GCC 4.2.1 always builds for JN5139
#    GCC 4.1.2 always builds for JN5148

#ifeq ($(shell ${CC} -dumpversion),4.2.1)
CHIP_5139=1
#else
#CHIP_5148=1
#endif

#CFLAGS+= -DDEBUG_SICSLOWPAN 
#-DDEBUG_ND6
#-DDEBUG_TCPIP 
#CFLAGS+= -DDEBUG_IEEE802 

CONTIKI = ../..


include $(CONTIKI)/Makefile.include
