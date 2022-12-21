#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "nrf_radio.h"
#include "net/routing/routing.h"
#include "net/ipv6/tcp-socket.h"
#include "net/ipv6/tcpip.h"
#include "net/netstack.h"
#include "sys/log.h"
#define LOG_MODULE "basestation"
#define LOG_LEVEL LOG_LEVEL_INFO

/*-------------------------DO NOT MODIFY-------------------------------------*/
/* Declare our "main" process, the basestation_process */
PROCESS(basestation_process, "Clicker basestation");
/* The basestation process should be started automatically when
 * the node has booted. */
AUTOSTART_PROCESSES(&basestation_process);

#define HDR_SIZE 5
#define SERVER_PORT	5666

/* Define the buffer size.*/
#define BUFSIZE 128
static uint8_t inputbuf[BUFSIZE];
static uint8_t outputbuf[BUFSIZE];

/*Define the tcp socket*/
static struct tcp_socket socket;

/* Holds the number of packets received. */
static int8_t last_rssi;

static void print_packet(const void *data, uint8_t len) {
    /* print sequence number, timestam, data (in hex) and RSSI of received data */
    //last_rssi = -(nrf_radio_rssi_sample_get());
    nrf_radio_task_trigger(NRF_RADIO_TASK_RSSISTART);
    while(nrf_radio_event_check(NRF_RADIO_EVENT_RSSIEND) == false);
    nrf_radio_event_clear(NRF_RADIO_EVENT_RSSIEND);
    last_rssi = nrf_radio_rssi_sample_get();

    printf("%u|%lu|%lu|", *((uint8_t *)data), *((uint32_t *)(data+1)), ((uint32_t)RTIMER_NOW())) ;
    for (uint8_t i=0; i<len-HDR_SIZE; i++) {
      printf("%02x ", *((uint8_t *)(data+i+HDR_SIZE)));

    }
    printf("|%d\n", last_rssi);

}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static int is_coordinator;

/* TCP is two-way communication so here is a callback for received packets.
*/
static int rx_data_callback(struct tcp_socket *s, void *ptr,
            const uint8_t *input_data_ptr, int input_data_len)
{
    print_packet(input_data_ptr, input_data_len);
    return input_data_len;
}

static void rx_event_callback(struct tcp_socket *s, void *ptr,
            tcp_socket_event_t event) {
}


/* Our main process. */
PROCESS_THREAD(basestation_process, ev, data) {
	is_coordinator = 1;

	PROCESS_BEGIN();

    /* Wait for connection. */
	if(is_coordinator) {
        NETSTACK_ROUTING.root_start();
    }

    /* Initialize TCP connection */
    tcp_socket_register(&socket, NULL,
                        inputbuf, sizeof(inputbuf),
                        outputbuf, sizeof(outputbuf),
                        rx_data_callback, rx_event_callback);

    tcp_socket_listen(&socket, SERVER_PORT);

	PROCESS_END();
}
