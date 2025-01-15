#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h> //? this needed now
#include <arpa/inet.h>
#include <string.h>
#include <netinet/ip_icmp.h>

//! TODO
// add dns_lookup and reverse_dns_lookup

//! change return type etc
int check_argv(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    return 1;
}

void send_ping(int sockfd) {
    struct sockaddr_in dest_addr;
    char payload[100];
    payload[99] = '\0';
    struct icmp *icmp = struct icmp *(payload);

    icmp.icmp_type = ICMP_ECHO;
    icmp.icmp_code = 0;
    icmp.icmp_id = 123;
    icmp.icmp_seq = 1;

    printf("%ld\n", sizeof(icmp));

    // Set destination address
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(0);;  // Not used for ICMP
    // htons(PORT_NO);
    dest_addr.sin_addr.s_addr = inet_addr("8.8.8.8");
    icmp.icmp_dun.id_ts.its_otime = 12345678;

    // Send payload (kernel adds ICMP header)
    ssize_t bytes_sent = sendto(sockfd, &icmp, sizeof(icmp), 0,
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
    if ( (bytes_recv = recv(sockfd, recv_buf,sizeof(recv_buf), 0)) < 0)
    {
        perror("recvfrom() error");

    } else
        printf("Received %ld byte packet!\n", bytes_recv);

    struct icmp *rec_icmp = (struct icmp *) &recv_buf;
    printf("%d %d %d\n", rec_icmp->icmp_dun.id_ts.its_otime, rec_icmp->icmp_dun.id_ts.its_rtime, rec_icmp->icmp_dun.id_ts.its_ttime);
    close(sockfd);
}

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
    //? timeout bonus?
    // setsockopt(ping_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof tv_out);
    send_ping(sockfd);
    return 0;
}