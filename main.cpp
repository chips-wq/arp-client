#include <iostream>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/if_ether.h>
#include <linux/if_packet.h>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include "interface_manager.h"

#define IPV4_ADDR_LEN 4

static unsigned char broadcast_address[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

struct eth_frame
{
    struct ethhdr eh;
    unsigned char buffer[ETH_DATA_LEN];
} __attribute__((packed));

struct arp_packet
{
    struct arphdr header;
    unsigned char sha[ETH_ALEN];
    unsigned char spa[IPV4_ADDR_LEN];
    unsigned char broadcast_address[ETH_ALEN];
    unsigned char tpa[IPV4_ADDR_LEN];
} __attribute__((packed));

arp_packet construct_arp_packet(unsigned char sha[], unsigned char spa[], struct in_addr *tpa)
{
    struct arp_packet router_arp;
    router_arp.header.ar_hrd = htons(ARPHRD_ETHER);
    router_arp.header.ar_pro = htons(ETH_P_IP);
    router_arp.header.ar_hln = ETH_ALEN;
    router_arp.header.ar_pln = IPV4_ADDR_LEN;       // 4 bytes for IPv4 find constants
    router_arp.header.ar_op = htons(ARPOP_REQUEST); // request operation

    // MAC Of sender
    memcpy(router_arp.sha, sha, ETH_ALEN);
    // IP of sender
    memcpy(router_arp.spa, spa, IPV4_ADDR_LEN);
    // MAC of receiver, all 0xFF (Broadcast address)
    memcpy(router_arp.broadcast_address, broadcast_address, ETH_ALEN);
    // IP of receiver
    memcpy(router_arp.tpa, (void *)(&(tpa->s_addr)), IPV4_ADDR_LEN);

    return router_arp;
}

void print_usage(const char* program_name)
{
    std::cerr << "Usage: sudo " << program_name << " -i eth0 -a 192.168.1.1" << std::endl;
    std::cerr << "Then run sudo tcpdump -i eth0 proto 0x0806 -XX" << std::endl;
}

int main(int argc, char *argv[])
{
    std::string interface_name;
    struct in_addr target_ip{};

    int opt;
    while ((opt = getopt(argc, argv, "i:a:")) != -1)
    {
        switch (opt)
        {
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

    if (interface_name.size() == 0 || target_ip.s_addr == 0)
    {
        print_usage(argv[0]);
        return -1;
    }

    // Initialize the interface manager
    InterfaceManager interfaceManager = InterfaceManager(interface_name);

    struct ifreq eth0_ifreq = interfaceManager.get_ifreq_idx();
    struct ifreq eth0_hwaddr = interfaceManager.get_ifreq_hwaddr();
    struct in_addr source_address = interfaceManager.get_interface_ip();

    unsigned char source_mac_address[ETH_ALEN];
    memcpy(source_mac_address, eth0_hwaddr.ifr_hwaddr.sa_data, ETH_ALEN);

    // Setup the ethernet_frame 
    struct eth_frame arp_frame{};
    memcpy(arp_frame.eh.h_source, source_mac_address, sizeof(source_mac_address));
    memcpy(arp_frame.eh.h_dest, broadcast_address, sizeof(broadcast_address));
    // Ethertype protocol
    arp_frame.eh.h_proto = htons(ETH_P_ARP);

    // Prepare sockaddr_ll
    struct sockaddr_ll socket_address{};

    socket_address.sll_family = AF_PACKET;
    socket_address.sll_ifindex = eth0_ifreq.ifr_ifindex;
    socket_address.sll_halen = ETH_ALEN;

    // Add payload (arp_packet) after Ethernet header
    struct arp_packet router_arp = construct_arp_packet(source_mac_address, (unsigned char *)(&source_address), &target_ip);
    memcpy(arp_frame.buffer, (void *)(&router_arp), sizeof(router_arp));

    size_t buffer_size = ETH_HLEN + sizeof(router_arp);

    // Send the packet
    ssize_t bytes_sent = sendto(interfaceManager.get_sockfd(), &arp_frame, buffer_size, 0,
                                (struct sockaddr *)&socket_address, sizeof(struct sockaddr_ll));

    if (bytes_sent < 0)
    {
        throw std::runtime_error("Failed to send packet: " + std::string(strerror(errno)));
    }

    std::cout << "Successfully sent " << bytes_sent << " bytes" << std::endl;
    std::cout << "ifreq name is " << eth0_ifreq.ifr_name << std::endl;
    std::cout << "ifreq index is " << eth0_ifreq.ifr_ifindex << std::endl;

    return 0;
}
