#include "ft_ping.h"

uint8_t stop = 0;

void send_ping(struct s_ping *ping_data)
{
    printf("PING %s (%s): 56 data bytes", ping_data->ip_argv, ping_data->ip_address);
    if (ping_data->flags.verbose)
    {
        printf (", id 0x%04x = %u", ping_data->id, ping_data->id);
    }
    printf("\n");
    
    struct icmp icmp = {
        .data = {
            // debug data
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
            0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
            0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
            0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27
        }
    };
    icmp.icmp_type = ICMP_ECHO;
    icmp.icmp_code = 0;
    icmp.icmp_id = htons(ping_data->id);

    char buffer[64] = {0};
    char control[64] = {0};
    struct sockaddr srcAddress;
    struct iovec iov = {
        .iov_base = buffer,
        .iov_len = sizeof(buffer)
    };
    struct msghdr msg = {
        .msg_name = &srcAddress,
        .msg_namelen = sizeof(srcAddress),
        .msg_iov = &iov,
        .msg_iovlen = 1,
        .msg_control = control,
        .msg_controllen = sizeof(control),
    };

    uint8_t ttl;
    uint16_t seq = 0;
    unsigned int counter = 0;
    struct timeval start, end;
    char ip_received[INET_ADDRSTRLEN];
    struct s_ping_stats *stats = &ping_data->stats;

    while (!stop && (ping_data->flags.count == 0 || counter < ping_data->flags.count))
    {
        icmp.icmp_seq = htons(seq);

        // add timestamp to ping
        gettimeofday(&start, NULL);
        icmp.icmp_otime = (uint32_t)start.tv_sec;
        icmp.icmp_ttime = (uint32_t)start.tv_usec;

        gettimeofday(&start, NULL);
        ssize_t bytes_sent = sendto(ping_data->sockfd, &icmp, sizeof(icmp), 0,
                                (struct sockaddr*)&ping_data->dest_addr, sizeof(ping_data->dest_addr));
        if (bytes_sent < 0)
        {
            stats->packets_sent++;
            stats->packets_lost++;
            counter++;
            continue;
        }

        ssize_t bytes_received = recvmsg(ping_data->sockfd, &msg, 0);
        gettimeofday(&end, NULL);

        // validate icmp packet
        struct icmp *rec_icmp = (struct icmp *) &buffer;
        if (rec_icmp->icmp_type != ICMP_ECHOREPLY && rec_icmp->icmp_code != 0 && \
                rec_icmp->icmp_id == icmp.icmp_id && rec_icmp->icmp_seq == icmp.icmp_seq)
        {
            print_icmp_error(rec_icmp->icmp_type, rec_icmp->icmp_code);
            stats->packets_sent++;
            stats->packets_lost++;
            counter++;
            usleep(ping_data->flags.interval);
            continue;
        }

        if (bytes_received < 0) {
            stats->packets_sent++;
            stats->packets_lost++;
            counter++;
            continue;
        }

        // retrieve ttl
        for (struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg))
        {
            if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_TTL)
            {
                ttl = *(uint8_t *)CMSG_DATA(cmsg);
            }
        }

        long elapsed_micros = ((end.tv_sec - start.tv_sec) * 1000000L) + (end.tv_usec - start.tv_usec);
        update_stats(ping_data, elapsed_micros);
        if (!ping_data->flags.quiet)
        {
            inet_ntop(AF_INET, &((struct sockaddr_in*)msg.msg_name)->sin_addr, ip_received, sizeof(ip_received));
            printf("%ld bytes from %s: icmp_seq=%d ttl=%u time=%ld.%03ld ms\n", 
                bytes_received, ip_received, seq, ttl, elapsed_micros / 1000, elapsed_micros % 1000);
        }
        counter++;
        seq++;
        usleep(ping_data->flags.interval);
    }
}

void init_ping(struct s_ping *ping_data, int argc, char *argv[])
{
    // argv init
    srand(time(NULL));
    ping_data->id =  rand() % 65535;
    ping_data->flags.ttl = 64;
    ping_data->flags.interval = 1.0;
    ping_data->flags.recv_timeout = 1;
    ping_data->stats.min = LONG_MAX;
    check_argv(ping_data, argc, argv);

    // socket init
    ping_data->sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    if (ping_data->sockfd < 0)
    {
        fprintf(stderr, "ft_ping: Socket file descriptor not received\n");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(ping_data->sockfd, SOL_IP, IP_TTL, &ping_data->flags.ttl, sizeof(ping_data->flags.ttl)) != 0)
    {
        fprintf(stderr, "ft_ping: Setting socket options of TTL failed\n");
        exit(EXIT_FAILURE);
    }
    int optval = 1;
    if (setsockopt(ping_data->sockfd, IPPROTO_IP, IP_RECVTTL, &optval, sizeof(optval)) < 0)
    {
        fprintf(stderr, "ft_ping: Setting socket options to receive TTL failed\n");
        exit(EXIT_FAILURE);
    }
    struct timeval tv = {.tv_sec = ping_data->flags.recv_timeout};
    if (setsockopt(ping_data->sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0)
    {
        fprintf(stderr, "ft_ping: Setting socket options of linger failed\n");
        exit(EXIT_FAILURE);
    }

    // id bonus init
    struct sockaddr_in local_addr = {0};
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(ping_data->id);

    if (bind(ping_data->sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        fprintf(stderr, "ft_ping: Failed to bind the socket for custom ICMP identifier.\n");
        exit(EXIT_FAILURE);
    }
    
    // dns check
    struct hostent *host = gethostbyname(ping_data->ip_argv); // Resolve hostname
    if (host == NULL) {
        fprintf(stderr, "ft_ping: Unknown host %s\n", ping_data->ip_argv);
        exit(EXIT_FAILURE);
    }

    ping_data->dest_addr.sin_family = AF_INET;
    memcpy(&ping_data->dest_addr.sin_addr, host->h_addr_list[0], host->h_length);

    // add ip_address
    inet_ntop(AF_INET, &ping_data->dest_addr.sin_addr, ping_data->ip_address, sizeof(ping_data->ip_address));
}

int main(int argc, char *argv[])
{
    struct s_ping ping_data = {0};
    set_signal_action();
    init_ping(&ping_data, argc, argv);
    send_ping(&ping_data);
    print_stats(&ping_data);
    close(ping_data.sockfd);
    return 0;
}