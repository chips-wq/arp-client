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

void get_ifreq_ref(int sockfd, const char* eth_name, struct ifreq& if_idx) {
    memset(&if_idx, 0, sizeof(struct ifreq));
    strncpy(if_idx.ifr_name, INTERFACE, IFNAMSIZ - 1);

    if_idx.ifr_name[IFNAMSIZ-1] = '\0';

    if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0) {
        throw std::runtime_error("Couldn't get the index of the interface " + std::string(INTERFACE) + " errno: " + std::string(strerror(errno)));
    }
}

int main() {
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        throw std::runtime_error("Couldn't create a raw socket " + std::string(strerror(errno)));
    }

    struct ifreq eth0_ifreq;
    get_ifreq_ref(sockfd, INTERFACE, eth0_ifreq);

    // Construct a buffer and a buffer size
    const size_t buffer_size = ETH_HLEN + strlen(PAYLOAD);
    unsigned char buffer[buffer_size];
    memset(buffer, 0, buffer_size);

    struct ethhdr* eh = (struct ethhdr*)buffer;

    eh->h_dest[0] = 0xFF;
    eh->h_dest[1] = 0xFF;
    eh->h_dest[2] = 0xFF;
    eh->h_dest[3] = 0xFF;
    eh->h_dest[4] = 0xFF;
    eh->h_dest[5] = 0xFF;

        // Ethertype (using ETH_P_IP, could be anything)
    eh->h_proto = htons(0x1234);

    // Add payload after Ethernet header
    memcpy(buffer + ETH_HLEN, PAYLOAD, strlen(PAYLOAD));

    // Prepare sockaddr_ll
    struct sockaddr_ll socket_address;
    socket_address.sll_family = AF_PACKET;
    socket_address.sll_protocol = htons(ETH_P_ALL);
    socket_address.sll_ifindex = eth0_ifreq.ifr_ifindex;
    socket_address.sll_halen = ETH_ALEN;

    // Set destination MAC in sockaddr_ll (broadcast)
    socket_address.sll_addr[0] = 0xFF;
    socket_address.sll_addr[1] = 0xFF;
    socket_address.sll_addr[2] = 0xFF;
    socket_address.sll_addr[3] = 0xFF;
    socket_address.sll_addr[4] = 0xFF;
    socket_address.sll_addr[5] = 0xFF;

    // Send the packet
    ssize_t bytes_sent = sendto(sockfd, buffer, buffer_size, 0,
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
