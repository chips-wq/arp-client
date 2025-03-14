#include <string>

class InterfaceManager
{
private:
    std::string interface;
    int sockfd;

public:
    InterfaceManager(std::string interface);
    ~InterfaceManager();
    struct ifreq get_ifreq_idx();
    struct ifreq get_ifreq_hwaddr();
    struct in_addr get_interface_ip();

    int get_sockfd() const;
};