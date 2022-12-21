#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "random.h"
#include "dev/button-hal.h"
#include "net/routing/routing.h"
#include "net/ipv6/tcp-socket.h"
#include "net/ipv6/tcpip.h"
#include "net/netstack.h"
#include "sys/log.h"
#define LOG_MODULE "client"
#define LOG_LEVEL LOG_LEVEL_INFO


/*-------------------------DO NOT MODIFY-------------------------------------*/
/* Declare our "main" process, the client process*/
PROCESS(client_process, "Clicker client");
/* The client process should be started automatically when
 * the node has booted. */
AUTOSTART_PROCESSES(&client_process);

#define FILE_SIZE 10000
#define PKT_SIZE 32
#define HDR_SIZE 5
#define SERVER_PORT	5666

/* Define the buffer size.*/
#define BUFSIZE 128
static uint8_t inputbuf[BUFSIZE];
static uint8_t outputbuf[BUFSIZE];

/*Define the tcp socket*/
static struct tcp_socket socket;

/* including a header to the packet:
 * - 1B sequence number
 * - 4B tx_timestamp
 *
 * packet: buffer to be updated with the header
 * seq: sequence number of the packet
 */
static void add_header(uint8_t *packet, uint16_t seq) {
    static uint32_t timestamp = 0;
    /* use first byte of the packet as sequence number. */
    packet[0] = seq & 0xff;
    /* four bytes of timestamp */
    timestamp = (uint32_t)RTIMER_NOW();
    memcpy(&packet[1], &timestamp, 4);
}

/* generaion of a random sequence.
 *
 * packet: buffer to be filled with the random sequence
 * seed: seed to initialize the random generator.
 *       (to have unique and reproducable sequences for every packet.
          Use the sequence number as seed!)
 * length: length of the sequence
 */
static  void compute_sequence(uint8_t *packet, uint16_t seed, uint8_t length) {
    const uint32_t A1 = 1664525;
    const uint32_t C1 = 1013904223;
    const uint32_t RAND_MAX1 = (((uint32_t)1 << 31) - 1);
    const uint8_t MAX_BYTE = ((1<<8) - 1);      // one byte
    uint8_t num = seed | (1<<4);                // seed (4 bites) is part of the seq number
    for (uint8_t i=0; i<length; i++) {          // generate the random payload byte by byte
        num = (num * A1 + C1) & RAND_MAX1;
        packet[i] = (uint8_t)(num & MAX_BYTE);

    }
}
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
PROCESS_THREAD(client_process, ev, data) {
    static uint8_t buffer[PKT_SIZE + HDR_SIZE];  // include 5 header bytes
    static uint16_t counter = 0;
    static bool connected = 0;
    static struct etimer timer;
    uip_ipaddr_t reciever_ip;

    PROCESS_BEGIN();



	 /* Initialize TCP connection */
     tcp_socket_register(&socket, NULL,
                         inputbuf, sizeof(inputbuf),
                         outputbuf, sizeof(outputbuf),
                         tx_data_callback, tx_event_callback);


     etimer_set(&timer,  0.3*SEND_INTERVAL);//random_rand() %

     while(1){
         /* Start transfer 256 package when button pressed. */
         PROCESS_WAIT_EVENT_UNTIL(ev == button_hal_press_event &&
                      data == button_hal_get_by_id(BUTTON_HAL_ID_BUTTON_ZERO));
         printf("\nButton pressed!\n");


         while (counter < 501){
             PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
             if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&reciever_ip)){
                 if (!connected){
                     tcp_socket_connect(&socket, &reciever_ip, SERVER_PORT);

                     printf("connected!\n");
                     LOG_INFO("TARGET ip: ");
                     LOG_INFO_6ADDR(&reciever_ip);
                     LOG_INFO("\n");

                     connected = 1;

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

                 }
                 add_header(&buffer[0], counter);
                 compute_sequence(&buffer[HDR_SIZE], counter, PKT_SIZE);
                 printf("Sending packet %d to Source\n", counter);
                 tcp_socket_send(&socket, buffer, PKT_SIZE + HDR_SIZE);
                 counter++;
             }
             /* Add some jitter */
             etimer_restart(&timer);
             // etimer_set(&timer, SEND_INTERVAL - (CLOCK_SECOND / 12) + (random_rand() % (2 * (CLOCK_SECOND / 12))));
         }
         connected = 0;
         counter = 0;
         printf("clear!");
     }
     etimer_stop(&timer);
     tcp_socket_close(&socket);
PROCESS_END();
}
