#include <iostream>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#define INTERFACE "tap0"
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

    std::cout << "ifreq name is " << eth0_ifreq.ifr_name << std::endl;
    std::cout << "ifreq index is " << eth0_ifreq.ifr_ifindex << std::endl;

    close(sockfd);

    return 0;
}
