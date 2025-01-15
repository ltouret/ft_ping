#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h> //? this needed now
#include <arpa/inet.h>
#include <string.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>

//! TODO
// add dns_lookup
// add argv -? -v and bonus -ttl -c -W?
// find a way to know if i need to do a dns lookup or not.
// find a way to simulate errors and try them with ./ping ping and ft_ping
// add argv parsing
// make my own typedef with icmp + padding char[36] -> with data? or use icmphdr thats 8 bytes bc i dont really use much of the 28 of icmp  
// icmp->icmp_ttime = 631043; //! add real time here
// char recv_buf[400]; //! this buff should be same as payload
// calculate time between sendto and recv and print it
// print after each ping in the correct format (check if -v format changes)
// easter egg in data padding of 64 bytes?
// retrieve ttl from ip packet, how??


//! change return type etc
int check_argv(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    return 1;
}

/*
intentar con recvfrom y sacar de ahi con getsockopt o directo del buffer que recibo sino sera recvmsg puta la weaaaa
*/

#include <time.h>

void send_ping(int sockfd) {
    struct sockaddr_in dest_addr = {0};
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(0);;  // Not used for ICMP
    dest_addr.sin_addr.s_addr = inet_addr("8.8.8.8");

    //? easter egg with data
    //! make my own typedef with icmp + padding char[36] -> with data?
    // char payload[64] = {0};
    char payload[64];
    payload[63] = '\0';
    struct icmp *icmp = (struct icmp *)(&payload);
    uint16_t seq = 1;

    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_code = htons(0);
    icmp->icmp_id = htons(42); // done by the kernel so idc, adding for the !lulz

    //! while 1 or if -c --> i < c 
    while (1) {
        //! seq is wrong order for some reason its 01 00 instead of 00 01 bytes
        icmp->icmp_seq = htons(seq);

        printf("%ld\n", sizeof(struct icmphdr));

        // Set destination address
        // memset(&dest_addr, 0, sizeof(dest_addr));

        time_t seconds_since_epoch = time(NULL);  // time() returns the current time as time_t
        icmp->icmp_otime = (uint32_t)seconds_since_epoch;
        printf("Seconds since the Epoch: %ld\n", (long) seconds_since_epoch);
        // icmp->icmp_rtime = 1; //? useless
        //! add here real millis and we good with time
        icmp->icmp_ttime = 631043; //! add real time here

        // Send payload (kernel adds ICMP header)
        ssize_t bytes_sent = sendto(sockfd, payload, sizeof(payload), 0,
                                    (struct sockaddr*)&dest_addr, sizeof(dest_addr));
        if (bytes_sent < 0) {
            perror("sendto failed");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        printf("Sent ICMP Echo Request with payload\n");

        //! this buff should be same as payload
        char recv_buf[400];
        memset(&recv_buf, 0, sizeof(recv_buf));
        ssize_t bytes_recv;
        // if((bytes_recv = recvfrom(sock, recv_buf,
        //     sizeof(ip) + sizeof(icmp) + sizeof(recv_buf), 0,
        //     (struct sockaddr *)&dst,
        //     (socklen_t *)&dst_addr_len)) < 0)
        if ( (bytes_recv = recv(sockfd, recv_buf, sizeof(recv_buf), 0)) < 0)
        {
            perror("recvfrom() error");
        } else
            printf("Received %ld byte packet!\n", bytes_recv);

        struct icmp *rec_icmp = (struct icmp *) &recv_buf;
        // struct ip *ip_reply = (struct ip *) &recv_buf;
        // printf("TTL: %d\n", ip_reply->ip_ttl);
        printf("%d %d %d %d %d\n", rec_icmp->icmp_type, rec_icmp->icmp_code, rec_icmp->icmp_dun.id_ts.its_otime, rec_icmp->icmp_dun.id_ts.its_rtime, rec_icmp->icmp_dun.id_ts.its_ttime);
        // usleep(1000000); //? here can change to add bonus of -W
        seq++;

        break; //! remove this later
    }

    //! print stats here if control c or -c
    close(sockfd);
}

#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char *argv[])
{
    //! argv into its own function
    check_argv(argc, argv);

    int sockfd;
    int ttl_val = 64;
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    // sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {// #include <arpa/inet.h>

        printf("\nSocket file descriptor not received!\n");
        return 0;
    } else {
        printf("\nSocket file descriptor %d received\n", sockfd);
    }
    if (setsockopt(sockfd, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0) {
        printf("\nSetting socket options to TTL failed!\n");
        return 1;
    } else {
        printf("\nSocket set to TTL...\n");
    }
    //? timeout bonus -w?
    // setsockopt(ping_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof tv_out);

    /*
    The inet_addr() function converts the Internet host address cp from IPv4 numbers-and-dots notation into binary data in network byte order.
    If the input is invalid, INADDR_NONE (usually -1) is returned. Use of this function is problematic because -1 is a valid address (255.255.255.255).
    Avoid its use in favor of inet_aton(), inet_pton(3), or getaddrinfo(3) which provide a cleaner way to indicate error return. 
    */

    //? do i use this? i shouldnt be doing a dns lookup for an ip!!! so nope
    // struct addrinfo hints; // Hints or "filters" for getaddrinfo()
    // struct addrinfo *res;

    // memset(&hints, 0, sizeof hints); // Initialize the structure
    // hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    // hints.ai_socktype = SOCK_STREAM; // TCP

    // // Get the associated IP address(es)
    // int status; // Return value of getaddrinfo()
    // status = getaddrinfo(argv[1], 0, &hints, &res);
    //     if (status != 0) { // error !
    //     fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    //     return (2);
    // }

    // printf("IP adresses for %s\n", argv[1]);

    send_ping(sockfd);
    return 0;
}