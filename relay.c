#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "dev/leds.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "nrf_radio.h"


/* Declare our "main" process, the relay_process */
PROCESS( relay_process, "Relay node");
/* The  relay process should be started automatically when
 * the node has booted. */
AUTOSTART_PROCESSES(& relay_process);
//#define NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, channel);



/* Holds the number of packets received. */
static int count = 0;

/* Callback function for received packets.
 *
 * Whenever this node receives a packet for its broadcast handle,
 * this function will be called.
 *
 * As the client does not need to receive, the function does not do anything
 */
static void recv(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest) {
NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, 18);
    count++;
    /* 0bxxxxx allows us to write binary values */
    /* for example, 0b10 is 2 */
    leds_off(LEDS_ALL);
    leds_on(count & 0b1111);
	
    printf("packet is recieved from the cilent\n");
    send(data, len);  //send the packet immediatly after received the packets 
    
    printf("packet is sending to  the basestation\n");
  
}

static void send(const void* data, uint16_t len) {
   int channel = 2;
    memcpy(nullnet_buf, data, len);
 NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, 22);
    nullnet_len = len;
    NETSTACK_NETWORK.output(NULL);
}

/* Our main process. */
PROCESS_THREAD(relay_process, ev, data) {
	PROCESS_BEGIN();

    static struct etimer timer;
	/* Initialize NullNet */
	nullnet_set_input_callback(recv);

    /* Setup a periodic timer that expires after 10 seconds. */
    etimer_set(&timer, CLOCK_SECOND * 10);
    
    while (1) {
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
        etimer_restart(&timer);
        count = 0;

    }
    
    
	PROCESS_END();
}
