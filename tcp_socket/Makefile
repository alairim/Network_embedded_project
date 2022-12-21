# Contiki network configuration
MAKE_MAC = MAKE_MAC_CSMA

CONTIKI_PROJECT = client basestation relay login
APPS=serial-shell

all: $(CONTIKI_PROJECT)

CONTIKI = /home/wcnes/contiki-ng-wcnes

MAKE_WITH_STORING_ROUTING = 1
MAKE_ROUTING = MAKE_ROUTING_RPL_CLASSIC

include $(CONTIKI)/Makefile.include
