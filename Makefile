# Contiki network configuration
MAKE_MAC = MAKE_MAC_CSMA


CONTIKI_PROJECT = client relay basestation login
APPS=serial-shell

all: $(CONTIKI_PROJECT)
CONTIKI = /home/wcnes/contiki-ng-wcnes
include $(CONTIKI)/Makefile.include
