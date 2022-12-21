#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "random.h"
#include "net/routing/routing.h"
#include "net/ipv6/tcp-socket.h"
#include "net/ipv6/tcpip.h"
#include "net/netstack.h"
#include "sys/log.h"
#define LOG_MODULE "relay"
#define LOG_LEVEL LOG_LEVEL_INFO


/*-------------------------DO NOT MODIFY-------------------------------------*/
/* Declare our "main" process, the relay process*/
PROCESS(relay_process, "Clicker relay");
/* The relay process should be started automatically when
 * the node has booted. */
AUTOSTART_PROCESSES(&relay_process);

#define HDR_SIZE 5
#define SERVER_PORT	5666

/* Define the buffer size.*/
#define BUFSIZE 128
static uint8_t inputbuf[BUFSIZE];
static uint8_t outputbuf[BUFSIZE];

/*Define the tcp socket*/
static struct tcp_socket socket;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#define SEND_INTERVAL  CLOCK_SECOND

/* TCP is two-way communication so here is a callback for received packets.
*
* Function does not do anything
*
*/
static int tx_data_callback(struct tcp_socket *s, void *ptr,
            const uint8_t *input_data_ptr, int input_data_len) {
    return 0;
}

static void tx_event_callback(struct tcp_socket *s, void *ptr,
            tcp_socket_event_t event) {
}

/* Our main process. */
PROCESS_THREAD(relay_process, ev, data) {
     static struct etimer timer;

     PROCESS_BEGIN();

	 /* Initialize TCP connection */
     tcp_socket_register(&socket, NULL,
                         inputbuf, sizeof(inputbuf),
                         outputbuf, sizeof(outputbuf),
                         tx_data_callback, tx_event_callback);


     etimer_set(&timer,  10*SEND_INTERVAL);//random_rand() %

     while(1){

	        /* Print routing table every 10s. */
            LOG_INFO("Routing entries: %u\n", uip_ds6_route_num_routes());
            uip_ds6_route_t *route = uip_ds6_route_head();
            while(route) {
              LOG_INFO("Route ");
              LOG_INFO_6ADDR(&route->ipaddr);
              LOG_INFO_(" via ");
              LOG_INFO_6ADDR(uip_ds6_route_nexthop(route));
              LOG_INFO_("\n");
              route = uip_ds6_route_next(route);
            }

            //rpl_print_neighbor_list();

            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
            etimer_restart(&timer);
     }

PROCESS_END();
}
