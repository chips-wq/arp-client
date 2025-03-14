#include <iostream>
#include <cstring>
#include <net/if.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include "interface_manager.h"

InterfaceManager::InterfaceManager(std::string interface) : interface(std::move(interface))
{
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (sockfd < 0)
    {
        throw std::runtime_error("Couldn't create a raw socket " + std::string(strerror(errno)));
    }
    if (interface.size() >= IFNAMSIZ)
    {
        throw std::invalid_argument("Interface name " + interface + " is too big. At max we have " + std::to_string(IFNAMSIZ) + " bytes.");
    }
}

int InterfaceManager::get_sockfd() const
{
    return sockfd;
}

InterfaceManager::~InterfaceManager()
{
    close(sockfd);
}

struct ifreq InterfaceManager::get_ifreq_idx()
{
    struct ifreq if_idx{};
    strncpy(if_idx.ifr_name, interface.c_str(), IFNAMSIZ - 1);

    if_idx.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
    {
        throw std::runtime_error("Couldn't get the index of the interface " + interface + " errno: " + std::string(strerror(errno)));
    }
    return if_idx;
}

struct ifreq InterfaceManager::get_ifreq_hwaddr()
{
    struct ifreq if_hwaddr{};
    strncpy(if_hwaddr.ifr_name, interface.c_str(), IF_NAMESIZE - 1);

    if_hwaddr.ifr_name[IFNAMSIZ - 1] = '\0';
    if (ioctl(sockfd, SIOCGIFHWADDR, &if_hwaddr) < 0)
    {
        throw std::runtime_error("Couldn't get the hwaddr(mac) of the interface " + interface + " errno: " + std::string(strerror(errno)));
    }
    return if_hwaddr;
}

// Get the IP address of the interface
struct in_addr InterfaceManager::get_interface_ip()
{
    struct ifreq if_addr{};
    strncpy(if_addr.ifr_name, interface.c_str(), IFNAMSIZ - 1);
    if_addr.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(sockfd, SIOCGIFADDR, &if_addr) < 0)
    {
        throw std::runtime_error("Couldn't get the IP address of interface " + interface +
                                 ": " + std::string(strerror(errno)));
    }

    // Extract the IP address
    struct sockaddr_in *addr_in = reinterpret_cast<struct sockaddr_in *>(&if_addr.ifr_addr);
    return addr_in->sin_addr;
}