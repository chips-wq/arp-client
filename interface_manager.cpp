#include <iostream>
#include <cstring>
#include <net/if.h>
#include <sys/ioctl.h>
#include <assert.h>
#include "interface_manager.h"

void get_ifreq_ref(int sockfd, const char *eth_name, struct ifreq &if_idx)
{
    memset(&if_idx, 0, sizeof(struct ifreq));
    strncpy(if_idx.ifr_name, INTERFACE, IFNAMSIZ - 1);

    if_idx.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
    {
        throw std::runtime_error("Couldn't get the index of the interface " + std::string(INTERFACE) + " errno: " + std::string(strerror(errno)));
    }
}

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
        throw std::runtime_error("Couldn't get the index of the interface " + std::string(INTERFACE) + " errno: " + std::string(strerror(errno)));
    }
    return if_idx;
}