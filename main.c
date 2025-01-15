#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h> //? this needed now
#include <arpa/inet.h>
#include <string.h>
#include <netinet/ip_icmp.h>

//! TODO
// add dns_lookup
// add -c -? -v
// find a way to know if i need to do a dns lookup or not.

//! change return type etc
int check_argv(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    return 1;
}

#include <time.h>

void send_ping(int sockfd) {
    struct sockaddr_in dest_addr = {0};
    //! remove this payload, back to icmp alone
    char payload[28] = {0};
    // payload[99] = '\0';
    struct icmp *icmp = (struct icmp *)(&payload);

    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_code = htons(0);
    icmp->icmp_id = htons(123);
    //! seq is wrong order for some reason its 01 00 instead of 00 01 bytes
    icmp->icmp_seq = htons(1);

    printf("%ld\n", sizeof(icmp));

    // Set destination address
    // memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(0);;  // Not used for ICMP
    // htons(PORT_NO);
    dest_addr.sin_addr.s_addr = inet_addr("8.8.8.8");
    time_t seconds_since_epoch = time(NULL);  // time() returns the current time as time_t
    icmp->icmp_otime = (uint32_t)seconds_since_epoch;
    printf("Seconds since the Epoch: %ld\n", (long) seconds_since_epoch);
    // icmp->icmp_rtime = 1; //? useless
    //! add here real millis and we good with time
    icmp->icmp_ttime = 631043;

    // Send payload (kernel adds ICMP header)
    ssize_t bytes_sent = sendto(sockfd, payload, sizeof(payload), 0,
                                (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    if (bytes_sent < 0) {
        perror("sendto failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Sent ICMP Echo Request with payload\n");

    char recv_buf[400];
    memset(&recv_buf, 0, sizeof(recv_buf));
    ssize_t bytes_recv;
    // if((bytes_recv = recvfrom(sockfd, recv_buf,
    //                     sizeof(recv_buf), 0,
    //                     (struct sockaddr *)&dest_addr,
    //                     (socklen_t *)&dest_addr)) < 0)
    if ( (bytes_recv = recv(sockfd, recv_buf, sizeof(recv_buf), 0)) < 0)
    {
        perror("recvfrom() error");

    } else
        printf("Received %ld byte packet!\n", bytes_recv);

    struct icmp *rec_icmp = (struct icmp *) &recv_buf;
    printf("%d %d %d\n", rec_icmp->icmp_dun.id_ts.its_otime, rec_icmp->icmp_dun.id_ts.its_rtime, rec_icmp->icmp_dun.id_ts.its_ttime);

    //! now parse info of the icmp packet
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

    /*
    The inet_addr() function converts the Internet host address cp from IPv4 numbers-and-dots notation into binary data in network byte order.
    If the input is invalid, INADDR_NONE (usually -1) is returned. Use of this function is problematic because -1 is a valid address (255.255.255.255).
    Avoid its use in favor of inet_aton(), inet_pton(3), or getaddrinfo(3) which provide a cleaner way to indicate error return. 
    */

    //? do i use this? i shouldnt be doing a dns lookup for an ip!!! so nope
    struct addrinfo hints; // Hints or "filters" for getaddrinfo()
    struct addrinfo *res;

    memset(&hints, 0, sizeof hints); // Initialize the structure
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP

    // Get the associated IP address(es)
    int status; // Return value of getaddrinfo()
    status = getaddrinfo(argv[1], 0, &hints, &res);
        if (status != 0) { // error !
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return (2);
    }

    printf("IP adresses for %s\n", argv[1]);
    //? timeout bonus?
    // setsockopt(ping_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof tv_out);
    send_ping(sockfd);
    return 0;
}