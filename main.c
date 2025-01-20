#include <stdio.h>
#include <sys/socket.h>
// #include <netinet/in.h> //?
#include <stdlib.h>
#include <unistd.h> //? this needed now
#include <arpa/inet.h>
#include <string.h>
#include <netinet/ip_icmp.h>
// #include <netinet/ip.h> //?

//! TODO
// make struct with all the data, int sockfd, const char *dest, icmp(?), the argv and what else?
// add argv parsing
    // add argv -? -v and bonus -ttl -c -i for intervals.
    // Maybe add -W (SO_RCVTIMEO) its the timeout time in recvmsg, -w is the max time ping before exiting, or if -c is done - no idea how to implement that
// find a way to know if i need to do a dns lookup or not. add dns_lookup -> maybe parse if ip is 255.255.255.255 (char.char.char.char)
// find a way to simulate errors and try them with ./ping ping and ft_ping
// // icmp->icmp_ttime = 631043; //! add real time here
// add header file
    // add icmp struct int the header to make it work with c99.
    // make my own typedef with icmp + padding char[36] -> with data? or use icmphdr thats 8 bytes bc i dont really use much of the 28 of icmp  
    // easter egg in data padding of 64 bytes?
// // calculate time between sendto and recvfrom and print it
// print after each ping in the correct format (check if -v format changes)
    // add math of statistics --> 2 packets transmitted, 2 packets received, 0% packet loss - round-trip min/avg/max/stddev = 2.244/3.556/4.867/1.312 ms
// // retrieve ttl from ip packet, how??
// recv from socket with timeout? //! for now if we dont receive the info it just stays locked there in the recv line -> ping waits 10 seconds then says packet lost? ineutils says it waits forever... need to check
// the time in the statistics is updated only after the first packet is sent, so if we do just one its 0, idk yet what it means but my time is wrong then -> tiempo total saltandonse el primer ping de cada intervalo, enotnces si -c 4 -i 0.5 = 1500~ ms
//! ping 3232235777 should work for now doesnt --> output -v PING 3232235777 (192.168.1.1): 56 data bytes
//! for now if ping 127.0.0.1 does work i dont receive the same message i sent. check later

//! format of ping ineutils 2.0
//! missing the dns lookup in case of domain
// ./ping -v 3232235777                                                                                                                                                                                                                                                                                                01:25:34
/*
    PING 3232235777 (192.168.1.1): 56 data bytes, id 0x81e7 = 33255
    64 bytes from 192.168.1.1: icmp_seq=0 ttl=255 time=4.263 ms
    64 bytes from 192.168.1.1: icmp_seq=1 ttl=0 time=8.486 ms
    ^C--- 3232235777 ping statistics ---
    2 packets transmitted, 2 packets received, 0% packet loss
    round-trip min/avg/max/stddev = 4.263/6.375/8.486/2.112 ms
*/
// ./ping 3232235777                                                                                                                                                                                                                                                                                                   01:25:36
/*
    PING 3232235777 (192.168.1.1): 56 data bytes
    64 bytes from 192.168.1.1: icmp_seq=0 ttl=2 time=4.867 ms
    64 bytes from 192.168.1.1: icmp_seq=1 ttl=3 time=2.244 ms
    ^C--- 3232235777 ping statistics ---
    2 packets transmitted, 2 packets received, 0% packet loss
    round-trip min/avg/max/stddev = 2.244/3.556/4.867/1.312 ms
*/

//! change return type etc
int check_argv(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    return 1;
}

#include <time.h>

void send_ping(int sockfd, const char *dest) {
    struct sockaddr_in dest_addr = {0};
    // memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(0);  // Not used for ICMP can erase, and is already init to 0

    // Set destination address
    dest_addr.sin_addr.s_addr = inet_addr(dest);

    //? easter egg with data
    //! make my own typedef with icmp + padding char[36] -> with random or easter egg data?
    char payload[64] = {0};
    struct icmp *icmp = (struct icmp *)(&payload);
    // memset(&payload, 0, sizeof(struct icmp)); //! needed if not payload {0}
    uint16_t seq = 1;
    uint8_t ttl;

    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_code = 0;
    icmp->icmp_id = htons(16962); // done by the kernel so idc, adding for the !lulz //! 4242

    printf("id = %x\n", icmp->icmp_id); //! -v id 0x4242 = 16962

    struct timespec ts, start, end; //? move this later
    //! while 1 or if -c --> i < c or -w close when time is over
    while (1) {
        //! seq is wrong order for some reason its 01 00 instead of 00 01 bytes
        icmp->icmp_seq = htons(seq);

        clock_gettime(CLOCK_REALTIME, &ts);
        uint32_t seconds_since_epoch = (uint32_t)ts.tv_sec;
        uint32_t microseconds = ((uint32_t)ts.tv_nsec) / 1000;

        printf("Seconds since the Epoch: %u, microseconds %u\n", seconds_since_epoch, microseconds);
        icmp->icmp_otime = seconds_since_epoch;
        icmp->icmp_ttime = microseconds;

        clock_gettime(CLOCK_MONOTONIC, &start);
        ssize_t bytes_sent = sendto(sockfd, payload, sizeof(payload), 0,
                                (struct sockaddr*)&dest_addr, sizeof(dest_addr));
        if (bytes_sent < 0) {
            perror("sendto failed");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        printf("ICMP packet sent: Seq: %u, Timestamp: %u.%u\n", ntohs(icmp->icmp_seq), seconds_since_epoch, microseconds);

        char buffer[64] = {0};
        char control[64] = {0};

        struct iovec iov = {
            .iov_base = buffer,
            .iov_len = sizeof(buffer)
        };

        struct msghdr msg = {
            .msg_name = NULL,
            .msg_namelen = 0,
            .msg_iov = &iov,
            .msg_iovlen = 1,
            .msg_control = control,
            .msg_controllen = sizeof(control),
        };

        ssize_t bytes_received = recvmsg(sockfd, &msg, 0); //? MSG_DONTWAIT -> no timeout?
        clock_gettime(CLOCK_MONOTONIC, &end);
        if (bytes_received < 0) {
            perror("recvmsg");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        printf("Waiting for incoming ICMP packets... received size %ld\n", bytes_received);
        // printf("ICMP reply received: Seq=%u\n", ntohs(icmp->icmp_seq));

        // to print data in buffers
        // for (int i = 0; i < 20; i++) {
        //     printf("i: %d -- buff %x | control %x\n", i, (unsigned char) buffer[i], (unsigned char) control[i]);
        // }

        for (struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
            if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_TTL) {
                ttl = *(uint8_t *)CMSG_DATA(cmsg);
                printf("Received TTL: %d\n", ttl);
            }
        }

        double elapsed_time = (end.tv_sec - start.tv_sec) +
                (end.tv_nsec - start.tv_nsec) / 1000000.0; //? changes to millis

        //? get icmp data in case of error (code 8, type with error).
        struct icmp *rec_icmp = (struct icmp *) &buffer;
        printf("type %d code %d millis %f ms -- %d %d %d\n", rec_icmp->icmp_type, rec_icmp->icmp_code, elapsed_time, rec_icmp->icmp_dun.id_ts.its_otime, rec_icmp->icmp_dun.id_ts.its_rtime, rec_icmp->icmp_dun.id_ts.its_ttime);
        // usleep(1000000); //? here can change to add bonus of -i and -w


        printf("64 bytes from %s: icmp_seq=%d ttl=%u time=%.3f ms\n", dest, seq, ttl, elapsed_time);
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

    //? change this into its own function, like socket_init
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
    int optval = 1; //? can i change this?
    if (setsockopt(sockfd, IPPROTO_IP, IP_RECVTTL, &optval, sizeof(optval)) < 0) {
        printf("\nSetting socket options to receive TTL failed!\n");
        return 1;
    }
    //? timeout bonus -W?
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
    send_ping(sockfd, argv[1]);
    return 0;
}