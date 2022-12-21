#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "dev/leds.h"
#include "nrf_radio.h"
/* UDP */
#include "net/routing/routing.h"
#include "net/ipv6/simple-udp.h"
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO
#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

/* Declare our "main" process, the basestation_process */
PROCESS(basestation_process, "Clicker basestation");
/* The basestation process should be started automatically when
 * the node has booted. */
AUTOSTART_PROCESSES(&basestation_process);

#define HDR_SIZE 5

/* Holds the number of packets received. */
static int count = 0;
static int8_t last_rssi;


static void print_packet(const void *data, uint8_t len) {
    /* print sequence number, timestam, data (in hex) and RSSI of received data */
    last_rssi = -(nrf_radio_rssi_sample_get());
    printf("%u|%lu|%lu|", *((uint8_t *)data), *((uint32_t *)(data+1)), ((uint32_t)RTIMER_NOW())) ;
    for (uint8_t i=0; i<len-HDR_SIZE; i++) {
      printf("%02x ", *((uint8_t *)(data+i+HDR_SIZE)));

    }
    printf("|%d\n", last_rssi);

}

static struct simple_udp_connection udp_conn;
/* Callback function for received packets.
 *
 * Whenever this node receives a packet for its broadcast handle,
 * this function will be called.
 *
 * As the client does not need to receive, the function does not do anything
 */
 static void udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t len)
{
    print_packet(data,len);
}

/* Our main process. */
PROCESS_THREAD(basestation_process, ev, data) {
	PROCESS_BEGIN();

    /* Initialize DAG root */
	NETSTACK_ROUTING.root_start();

	/* Initialize UDP connection */
	simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                        UDP_CLIENT_PORT, udp_rx_callback);

    /* Setup a periodic timer that expires after 10 seconds. */
    static struct etimer timer;
    etimer_set(&timer, CLOCK_SECOND * 10);

    while (1) {
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
        etimer_restart(&timer);
        /* print statistics */
        // printf("\n--- statistics: received %d packets / 10s\n", count);
        count = 0;
    }


	PROCESS_END();
}
