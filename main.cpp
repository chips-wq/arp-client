#include <iostream>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <linux/if_packet.h>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include "interface_manager.h"

#define PAYLOAD "Hello world!"

#define PROT_ADDRESS_LEN 4

unsigned char sha[] = {0x00, 0x15, 0x5d, 0x9f, 0x9e, 0x01};
unsigned char tha[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


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

void print_usage(char *program_name) {
    std::cerr << "Usage: sudo " << program_name << " -i eth0 -a 192.168.1.1" << std::endl;
    std::cerr << "Then run sudo tcpdump -i eth0 proto 0x0806 -XX" << std::endl;

}

int main(int argc, char *argv[]) {
    std::string interface_name;
    struct in_addr target_ip{};

    int opt;
    while ((opt = getopt(argc, argv, "i:a:")) != -1) {
        switch (opt) {
            case 'i':
                std::cout << "Option i set with " << optarg << std::endl;
                interface_name = optarg;
                break;
            case 'a':
                std::cout << "Option a set with " << optarg << std::endl;
                inet_pton(AF_INET, optarg, &target_ip);
                break;
            case '?':
            default:
                print_usage(argv[0]);
                break;
        }
    }

    if (interface_name.size() == 0 || target_ip.s_addr == 0) {
        print_usage(argv[0]);
        return -1;
    }

    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (sockfd < 0) {
        throw std::runtime_error("Couldn't create a raw socket " + std::string(strerror(errno)));
    }

    InterfaceManager interfaceManager = InterfaceManager(sockfd, "eth0");

    struct ifreq eth0_ifreq = interfaceManager.get_ifreq_idx();
    // get_ifreq_ref(sockfd, INTERFACE, eth0_ifreq);


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
