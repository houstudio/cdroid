#include <stdio.h>
#include <stdlib.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
//
int setIpAddr(const char*interface,const char*ipaddr){
    int sockfd;
    struct ifreq ifr;
    struct sockaddr_in addr;

    // Open a socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return -1;
    }

    // Set the interface name
    strncpy(ifr.ifr_name,interface, IFNAMSIZ);

    // Set the IP address
    inet_pton(AF_INET, ipaddr, &(addr.sin_addr));
    memcpy(&ifr.ifr_addr, &addr, sizeof(struct sockaddr));

    // Set the address
    if (ioctl(sockfd, SIOCSIFADDR, &ifr) == -1) {
        perror("ioctl");
        return -1;
    }
    printf("IP address set successfully.\n");
    // Close the socket
    close(sockfd);
    return 0;
}

int setdhcp(const char*intf) {
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];
    if (getifaddrs(&ifaddr) == -1) {
        printf("getifaddrsi\r\n");
        exit(EXIT_FAILURE);
    }

    // Iterate through the linked list of network interfaces
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;

        // Check if the interface is IPv4 and DHCP
        if (family == AF_INET && (ifa->ifa_flags & IFF_DYNAMIC)) {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                            host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                printf("getnameinfo() failed: %s\r\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            printf("Interface: %s\r\n", ifa->ifa_name);
            printf("IP address: %s\r\n", host);
        }
        printf("name=%s line %d\r\n",ifa->ifa_name,__LINE__);
    }

    printf("line %d\r\n",__LINE__);
    freeifaddrs(ifaddr);
    return 0;
}

int setDNS(){
    int sockfd;
    struct ifreq ifr;
    struct sockaddr_in *dns;
#if 0
    // Open a socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set the interface name
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);

    // Set the DNS servers
    dns = (struct sockaddr_in *)&ifr.ifr_dns;
    dns[0].sin_family = AF_INET;
    inet_pton(AF_INET, "8.8.8.8", &(dns[0].sin_addr));
    dns[1].sin_family = AF_INET;
    inet_pton(AF_INET, "8.8.4.4", &(dns[1].sin_addr));

    // Set the DNS servers for the interface
    if (ioctl(sockfd, SIOCSIFDNS, &ifr) == -1) {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }

    printf("DNS servers set successfully.\n");
#endif
    // Close the socket
    close(sockfd);
    return 0;
}
int main(int argc,char*argv[]){
    setdhcp("");
    return 0;
}
