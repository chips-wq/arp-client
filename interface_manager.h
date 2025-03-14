#include <string>

#define INTERFACE "eth0"

class InterfaceManager {
private:
    std::string interface;
    int sockfd;

public:
    InterfaceManager(int sockfd, std::string interface);
    ~InterfaceManager();
    struct ifreq get_ifreq_idx();
};