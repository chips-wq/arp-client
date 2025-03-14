#include <string>

class InterfaceManager {
private:
    std::string interface;
    int sockfd;

public:
    InterfaceManager(int sockfd, std::string interface);
    ~InterfaceManager();
    struct ifreq get_ifreq_idx();
    struct ifreq get_ifreq_hwaddr();
};