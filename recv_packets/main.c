#include <stdint.h>
#include <inttypes.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include <rte_arp.h>
#include <arpa/inet.h>

#define RX_RING_SIZE 128
#define TX_RING_SIZE 512

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

static short nb_ports = 0;

static const struct rte_eth_conf port_conf_default = {
        .rxmode = { .max_rx_pkt_len = ETHER_MAX_LEN, },
};

int handle_buf(struct rte_mbuf *buf);

int
main(int argc, char *argv[])
{
	int ret = rte_eal_init(argc, argv);
    nb_ports = rte_eth_dev_count();
    printf("nb_ports = %d ret = %d\n", nb_ports, ret);

    struct rte_mempool *mbuf_pool;
    mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL",
            NUM_MBUFS * nb_ports, MBUF_CACHE_SIZE, 0,
            RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

    struct rte_eth_conf port_conf = port_conf_default;
    const uint16_t rx_rings = 1, tx_rings = 1;
    int retval;

    int port = 0;
    if (port >= rte_eth_dev_count())
        return -1;

    retval = rte_eth_dev_configure(port, rx_rings, tx_rings, &port_conf);
    printf("[%d]retval = %d\n",__LINE__ ,retval);
    if (retval < 0 ) {
        return -1;
    }
    uint16_t q;
    printf("hellow %d\n", __LINE__);
    for (q = 0; q < rx_rings; ++q) {
        retval = rte_eth_rx_queue_setup(port, q, RX_RING_SIZE, rte_eth_dev_socket_id(port), NULL, mbuf_pool);
        printf("[%d]retval = %d\n",__LINE__ ,retval);
        if (retval < 0 ) {
            return -1;
        }
        retval = rte_eth_tx_queue_setup(port, q, TX_RING_SIZE, rte_eth_dev_socket_id(port), NULL);
        printf("[%d]retval = %d\n",__LINE__ ,retval);
        if (retval < 0 ) {
            return -1;
        }
    }
    printf("hellow %d\n", __LINE__);

    retval = rte_eth_dev_start(port);
    printf("rte_eth_dev_start result is %d\n", retval);
    struct ether_addr addr;

    rte_eth_macaddr_get(port, &addr);
    printf("Port %u MAC: %02"PRIx8" %02"PRIx8" %02"PRIx8
            " %02"PRIx8" %02"PRIx8" %02"PRIx8"\n",
            (unsigned)port,
            addr.addr_bytes[0], addr.addr_bytes[1],
            addr.addr_bytes[2], addr.addr_bytes[3],
            addr.addr_bytes[4], addr.addr_bytes[5]);
    printf("hellow %d\n", __LINE__);
    while(1) {
        struct rte_mbuf *bufs[BURST_SIZE];
        const uint16_t nb_rx = rte_eth_rx_burst(port, 0, bufs, BURST_SIZE);
        if (unlikely(nb_rx == 0)) {
            continue;
        }
        uint16_t buf = 0;
        for (buf = 0; buf < nb_rx; buf++) {
            handle_buf(bufs[buf]);
            rte_pktmbuf_free(bufs[buf]);
            printf("received [%d] packets\n", nb_rx);
        }
    }

    return 0;
}

int handle_buf(struct rte_mbuf *buf) {
    struct ether_hdr *eth;
    eth = rte_pktmbuf_mtod(buf, struct ether_hdr *);
    switch(ntohs(eth->ether_type)) {
        case ETHER_TYPE_ARP :
            printf("get a arp packet\n");
            break;
        case ETHER_TYPE_IPv4 :
            printf("get a ipv4 packet\n");
            break;
        default :
            printf("get a other packet\n");
            break;
    }

    printf("packet type = %d\n", buf->packet_type);
    return 0;
}
