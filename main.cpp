#include <iostream>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <linux/if_packet.h>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#define INTERFACE "eth0"
#define PAYLOAD "Hello world!"

#define PROT_ADDRESS_LEN 4

unsigned char sha[] = {0x00, 0x15, 0x5d, 0x9f, 0x9e, 0x01};
unsigned char tha[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void get_ifreq_ref(int sockfd, const char* eth_name, struct ifreq& if_idx) {
    memset(&if_idx, 0, sizeof(struct ifreq));
    strncpy(if_idx.ifr_name, INTERFACE, IFNAMSIZ - 1);

    if_idx.ifr_name[IFNAMSIZ-1] = '\0';

    if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0) {
        throw std::runtime_error("Couldn't get the index of the interface " + std::string(INTERFACE) + " errno: " + std::string(strerror(errno)));
    }
}


struct eth_frame {
    struct ethhdr eh;
    unsigned char buffer[ETH_DATA_LEN];
} __attribute__((packed));


struct arp_packet {
    unsigned short htype;
    unsigned short ptype;
    unsigned char hlen;
    unsigned char plen;
    unsigned short oper;
    unsigned char sha[ETH_ALEN];
    unsigned char spa[PROT_ADDRESS_LEN];
    unsigned char tha[ETH_ALEN];
    unsigned char tpa[PROT_ADDRESS_LEN];
} __attribute__((packed));

arp_packet construct_arp_packet() {
    unsigned char spa[] = {10, 40, 6, 123};
    unsigned char tpa[] = {10, 40, 141, 224};

    struct arp_packet router_arp;
    router_arp.htype = htons(0x0001);
    router_arp.ptype = htons(0x0800);
    router_arp.hlen = ETH_ALEN;
    router_arp.plen = PROT_ADDRESS_LEN; // 4 bytes for IPv4 find constants
    router_arp.oper = htons(0x0001); // request operation

    // MAC Of sender
    memcpy(router_arp.sha, sha, ETH_ALEN);
    // IP of sender
    memcpy(router_arp.spa, spa, PROT_ADDRESS_LEN);
    // MAC of receiver, all 0xff
    memcpy(router_arp.tha, tha, ETH_ALEN);
    // IP of receiver
    memcpy(router_arp.tpa, tpa, PROT_ADDRESS_LEN);

    return router_arp;
}

int main() {
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        throw std::runtime_error("Couldn't create a raw socket " + std::string(strerror(errno)));
    }

    struct ifreq eth0_ifreq;
    get_ifreq_ref(sockfd, INTERFACE, eth0_ifreq);


    struct eth_frame arp_frame;
    memcpy(arp_frame.eh.h_source, sha, sizeof(sha));
    memcpy(arp_frame.eh.h_dest, tha, sizeof(tha));


    // Ethertype protocol
    arp_frame.eh.h_proto = htons(ETH_P_ARP);


    // Prepare sockaddr_ll
    struct sockaddr_ll socket_address;
    socket_address.sll_family = AF_PACKET;
    socket_address.sll_ifindex = eth0_ifreq.ifr_ifindex;
    socket_address.sll_halen = ETH_ALEN;
    // socket_address.sll_protocol = htons(ETH_P_ARP);

    // Set destination MAC in sockaddr_ll (broadcast)
    // socket_address.sll_addr[0] = 0xFF;
    // socket_address.sll_addr[1] = 0xFF;
    // socket_address.sll_addr[2] = 0xFF;
    // socket_address.sll_addr[3] = 0xFF;
    // socket_address.sll_addr[4] = 0xFF;
    // socket_address.sll_addr[5] = 0xFF;

    // Add payload after Ethernet header
    struct arp_packet router_arp = construct_arp_packet();
    memcpy(arp_frame.buffer, (void*)(&router_arp), sizeof(router_arp));

    size_t buffer_size = ETH_HLEN + sizeof(router_arp);
    // Send the packet
    ssize_t bytes_sent = sendto(sockfd, &arp_frame, buffer_size, 0,
                                (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll));

    if (bytes_sent < 0) {
        throw std::runtime_error("Failed to send packet: " + std::string(strerror(errno)));
    }

    std::cout << "Successfully sent " << bytes_sent << " bytes" << std::endl;




    std::cout << "ifreq name is " << eth0_ifreq.ifr_name << std::endl;
    std::cout << "ifreq index is " << eth0_ifreq.ifr_ifindex << std::endl;

    close(sockfd);

    return 0;
}
