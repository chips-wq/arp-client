#include <iostream>
#include <cstring>
#include <net/if.h>
#include <sys/ioctl.h>
#include <assert.h>
#include "interface_manager.h"

InterfaceManager::InterfaceManager(int sockfd, std::string interface) : sockfd(sockfd), interface(std::move(interface)) {
    if (interface.size() >= IFNAMSIZ) {
        throw std::invalid_argument("Interface name " + interface + " is too big. At max we have " + std::to_string(IFNAMSIZ) + " bytes.");
    }
}

InterfaceManager::~InterfaceManager() {

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

struct ifreq InterfaceManager::get_ifreq_hwaddr() {
    struct ifreq if_hwaddr{};
    strncpy(if_hwaddr.ifr_name, interface.c_str(), IF_NAMESIZE - 1);

    if_hwaddr.ifr_name[IFNAMSIZ - 1] = '\0';
    if (ioctl(sockfd, SIOCGIFHWADDR, &if_hwaddr) < 0) {
        throw std::runtime_error("Couldn't get the hwaddr(mac) of the interface " + interface + " errno: " + std::string(strerror(errno)));
    }
    return if_hwaddr;
}