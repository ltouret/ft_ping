#include "ping.h"

uint8_t stop = 1;

void update_stats(struct s_ping *ping_data, long elapsed_micros)
{
    struct s_ping_stats *stats = &ping_data->stats;
    stats->sum += elapsed_micros;
    stats->sum_sq += elapsed_micros * elapsed_micros;
    
    if (elapsed_micros < stats->min)
    {
        stats->min = elapsed_micros;
    }
    if (elapsed_micros > stats->max)
    {
        stats->max = elapsed_micros;
    }
    stats->packets_sent++;
}

void my_usleep(double seconds)
{
    struct timeval tval;
    tval.tv_sec = (int)seconds;
    tval.tv_usec = (int)((seconds - tval.tv_sec) * 1000000);
    select(0, NULL, NULL, NULL, &tval);
}

double square_root(double val)
{
	double ans = 1, sqr = 1, i = 1;
	while (sqr <= val)
	{
		i++;
		sqr = i * i;
	}
	ans = i - 1;
	return ans;
}

void print_stats(struct s_ping *ping_data)
{
    int loss_percent = 0;
    struct s_ping_stats *stats = &ping_data->stats;

    printf("--- %s ping statistics ---\n", ping_data->ip_argv);
    if (stats->packets_sent != 0)
    {
        loss_percent = (int)(((float)stats->packets_lost / stats->packets_sent) * 100.0);
    }
    printf("%ld packets transmitted, %ld packets received, %d%% packet loss\n", stats->packets_sent, stats->packets_sent - stats->packets_lost, loss_percent);
    if (stats->packets_sent > 0 && stats->packets_sent - stats->packets_lost > 0) {
        long total = stats->packets_sent - stats->packets_lost;
        long avg = stats->sum / total;
        long long variance;

        if (stats->sum < INT_MAX)
        {
            variance = (stats->sum_sq - ((stats->sum * stats->sum) / total)) / total;
        }
        else
        {
            variance = (stats->sum_sq / total) - (avg * avg);
        }

        long stddev = square_root(variance);
        printf("round-trip min/avg/max/stddev = %ld.%03ld/%ld.%03ld/%ld.%03ld/%ld.%03ld ms\n",
            stats->min / 1000, stats->min % 1000,
            avg / 1000, avg % 1000,
            stats->max / 1000, stats->max % 1000,
            stddev / 1000, stddev % 1000);
    }
}

void print_icmp_error(u_int8_t type, u_int8_t code)
{
    switch (type) {
        // Destination Unreachable
        case ICMP_DEST_UNREACH:
            switch (code) {
                case ICMP_NET_UNREACH:
                    printf("Destination network unreachable\n");
                    break;
                case ICMP_HOST_UNREACH:
                    printf("Destination host unreachable\n");
                    break;
                case ICMP_PROT_UNREACH:
                    printf("Destination protocol unreachable\n");
                    break;
                case ICMP_PORT_UNREACH:
                    printf("Destination port unreachable\n");
                    break;
                case ICMP_FRAG_NEEDED:
                    printf("Fragmentation required, and DF flag set\n");
                    break;
                case ICMP_SR_FAILED:
                    printf("Source route failed\n");
                    break;
                case ICMP_NET_UNKNOWN:
                    printf("Destination network unknown\n");
                    break;
                case ICMP_HOST_UNKNOWN:
                    printf("Destination host unknown\n");
                    break;
                case ICMP_HOST_ISOLATED:
                    printf("Source host isolated\n");
                    break;
                case ICMP_NET_ANO:
                    printf("Network administratively prohibited\n");
                    break;
                case ICMP_HOST_ANO:
                    printf("Host administratively prohibited\n");
                    break;
                case ICMP_NET_UNR_TOS:
                    printf("Network unreachable for ToS\n");
                    break;
                case ICMP_HOST_UNR_TOS:
                    printf("Host unreachable for ToS\n");
                    break;
                case ICMP_PKT_FILTERED:
                    printf("Communication administratively prohibited\n");
                    break;
                case ICMP_PREC_VIOLATION:
                    printf("Host Precedence Violation\n");
                    break;
                case ICMP_PREC_CUTOFF:
                    printf("Precedence cutoff in effect\n");
                    break;
                default:
                    printf("Unknown Destination Unreachable code\n");
                    break;
            }
            break;
        // Redirect Message
        case ICMP_REDIRECT:
            switch (code) {
                case ICMP_REDIR_NET:
                    printf("Redirect for network\n");
                    break;
                case ICMP_REDIR_HOST:
                    printf("Redirect for host\n");
                    break;
                case ICMP_REDIR_NETTOS:
                    printf("Redirect for ToS and network\n");
                    break;
                case ICMP_REDIR_HOSTTOS:
                    printf("Redirect for ToS and host\n");
                    break;
                default:
                    printf("Unknown Redirect code\n");
                    break;
            }
            break;
        // Time Exceeded
        case ICMP_TIME_EXCEEDED:
            switch (code) {
                case ICMP_EXC_TTL:
                    printf("Time to Live exceeded in transit\n");
                    break;
                case ICMP_EXC_FRAGTIME:
                    printf("Fragment reassembly time exceeded\n");
                    break;
                default:
                    printf("Unknown Time Exceeded code\n");
                    break;
            }
            break;
        // Parameter Problem
        case ICMP_PARAMETERPROB:
            switch (code) {
                case 0:
                    printf("Pointer indicates the error\n");
                    break;
                case 1:
                    printf("Missing a required option\n");
                    break;
                case 2:
                    printf("Bad length\n");
                    break;
                default:
                    printf("Unknown Parameter Problem code\n");
                    break;
            }
            break;
        default:
            printf("Unknown ICMP type\n");
            break;
    }
}

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

    uint8_t ttl;
    uint16_t seq = 0;
    unsigned int counter = 0;
    struct timeval start, end;
    struct s_ping_stats *stats = &ping_data->stats;

    while (stop && (ping_data->flags.count == 0 || counter < ping_data->flags.count))
    {
        icmp.icmp_seq = htons(++seq);

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

        // check icmp packet
        struct icmp *rec_icmp = (struct icmp *) &buffer;
        if (rec_icmp->icmp_type != ICMP_ECHOREPLY && rec_icmp->icmp_code != 0 && \
                rec_icmp->icmp_id == icmp.icmp_id && rec_icmp->icmp_seq == icmp.icmp_seq)
        {
            print_icmp_error(rec_icmp->icmp_type, rec_icmp->icmp_code);
            stats->packets_sent++;
            stats->packets_lost++;
            counter++;
            my_usleep(ping_data->flags.interval);
            continue;
        }

        if (bytes_received < 0) {
            stats->packets_sent++;
            stats->packets_lost++;
            counter++;
            continue;
        }

        // retrieve ttl
        for (struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
            if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_TTL) {
                ttl = *(uint8_t *)CMSG_DATA(cmsg);
            }
        }

        long elapsed_micros = ((end.tv_sec - start.tv_sec) * 1000000L) + (end.tv_usec - start.tv_usec);
        update_stats(ping_data, elapsed_micros);
        if (!ping_data->flags.quiet)
        {
            printf("64 bytes from %s: icmp_seq=%d ttl=%u time=%ld.%03ld ms\n", ping_data->ip_address, seq, ttl, elapsed_micros / 1000, elapsed_micros % 1000);
        }
        counter++;
        my_usleep(ping_data->flags.interval);
    }
}

void sigint_handler(int signal)
{
    if (signal == SIGINT)
    {
        stop = 0;
    }
}

void set_signal_action(void)
{
    signal(SIGINT, sigint_handler);
}

void panic_argv(const char *format, const char *var)
{
    fprintf(stderr, format, var);
    fprintf(stderr, "Try 'ft_ping --help' or 'ping --usage' for more information.\n");
    exit(EXIT_FAILURE);
}

double check_bonus_argv_double(char *str)
{
    char *endptr;
    double num = strtod(str, &endptr);
    if (endptr == str || *endptr != '\0')
    {
        fprintf(stderr, "ft_ping: invalid value (`%s' near `%s')\n", str, endptr);
        exit(EXIT_FAILURE);
    }
    return num;
}

int check_bonus_argv_int(char *str)
{
    char *endptr;
    int num = (int)strtol(str, &endptr, 10);
    if (endptr == str || *endptr != '\0')
    {
        fprintf(stderr, "ft_ping: invalid value (`%s' near `%s')\n", str, endptr);
        exit(EXIT_FAILURE);
    }
    return num;
}

void check_argv(struct s_ping *ping_data, int argc, char *argv[])
{
    int flag_val = 0;
    double interval = 0.0;
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (!strcmp(argv[i], "-?") || !strcmp(argv[i], "--help"))
            {
                printf("Usage: ping [OPTION...] HOST ...\n");
                printf("Send ICMP ECHO_REQUEST packets to network hosts.\n");
                printf("--ttl=N                specify N as time-to-live\n");
                printf("-c=N                   stop after sending NUMBER packets\n");
                printf("-i=N                   wait NUMBER seconds between sending each packet\n");
                printf("-v                     verbose output\n");
                printf("-q                     quiet output\n");
                printf("-?, --help             give this help list\n");
                printf("--usage                give a short usage message\n");
                exit(EXIT_SUCCESS);
            }
            else if (!strcmp(argv[i], "-V"))
            {
                printf("(Fixed your fucking) ping (GNU inetutils) 2.0\n");
                printf("You're welcome :)\n");
                printf("Written by Leo motherfucking G.\n");
                exit(EXIT_SUCCESS);
            }
            else if (!strcmp(argv[i], "--usage"))
            {
                printf("Usage: ping [-vq?] [-c N] [-i N] [-W N] [--ttl=N] [--help] [--usage] HOST ...\n");
                exit(EXIT_SUCCESS);
            }
            else if (!strcmp(argv[i], "-v"))
            {
                ping_data->flags.verbose = 1;
            }
            else if (!strcmp(argv[i], "-q"))
            {
                ping_data->flags.quiet = 1;
            }
            // change IP_RECVTTL
            else if (!strcmp(argv[i], "--ttl"))
            {
                if (i + 1 < argc)
                {
                    flag_val = check_bonus_argv_int(argv[i + 1]);
                    if (flag_val <= 0 || flag_val > 255)
                    {
                        fprintf(stderr, "ft_ping: invalid value for option --ttl, must be more than 0 and less than 256\n");
                        exit(EXIT_FAILURE);
                    }
                    ping_data->flags.ttl = flag_val;
                    i++;
                }
                else
                {
                    panic_argv("ft_ping: option '%s' requires an argument\n", argv[i]);
                    exit(EXIT_FAILURE);
                }
            }
            // count till -c then exit
            else if (strstr(argv[i], "-c"))
            {
                if (strlen(argv[i]) == 2)
                {

                    if (i + 1 < argc)
                    {
                        flag_val = check_bonus_argv_int(argv[i + 1]);
                        i++;
                    }
                    else
                    {
                        panic_argv("ft_ping: option requires an argument -- '%s'\n", (char[]){argv[i][1], '\0'});
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    char *str = argv[i] + 2;
                    flag_val = check_bonus_argv_int(str);
                }
                if (flag_val < 0)
                {
                    fprintf(stderr, "ft_ping: invalid value for option -c, must be non-negative\n");
                    exit(EXIT_FAILURE);
                }
                ping_data->flags.count = flag_val;
            }
            // change my_usleep
            else if (strstr(argv[i], "-i"))
            {
                if (strlen(argv[i]) == 2)
                {

                    if (i + 1 < argc)
                    {
                        interval = check_bonus_argv_double(argv[i + 1]);
                        i++;
                    }
                    else
                    {
                        panic_argv("ft_ping: option requires an argument -- '%s'\n", (char[]){argv[i][1], '\0'});
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    char *str = argv[i] + 2;
                    interval = check_bonus_argv_double(str);
                }
                if (interval < 0.2)
                {
                    fprintf(stderr, "ft_ping: invalid value for option -i, must be at least 0.2\n");
                    exit(EXIT_FAILURE);
                }
                ping_data->flags.interval = interval;
            }
            // change linger SO_RCVTIMEO
            else if (strstr(argv[i], "-W"))
            {
                if (strlen(argv[i]) == 2)
                {

                    if (i + 1 < argc)
                    {
                        flag_val = check_bonus_argv_int(argv[i + 1]);
                        i++;
                    }
                    else
                    {
                        panic_argv("ft_ping: option requires an argument -- '%s'\n", (char[]){argv[i][1], '\0'});
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    char *str = argv[i] + 2;
                    flag_val = check_bonus_argv_int(str);
                }
                if (flag_val <= 0)
                {
                    fprintf(stderr, "ft_ping: invalid value for option -W, must be more than 0\n");
                    exit(EXIT_FAILURE);
                }
                ping_data->flags.recv_timeout = flag_val;
            }
            // change icmp_id
            else if (strstr(argv[i], "-e"))
            {
                if (strlen(argv[i]) == 2)
                {

                    if (i + 1 < argc)
                    {
                        flag_val = check_bonus_argv_int(argv[i + 1]);
                        i++;
                    }
                    else
                    {
                        panic_argv("ft_ping: option requires an argument -- '%s'\n", (char[]){argv[i][1], '\0'});
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    char *str = argv[i] + 2;
                    flag_val = check_bonus_argv_int(str);
                }
                if (flag_val <= 0 || flag_val > 65535)
                {
                    fprintf(stderr, "ft_ping: invalid value for option -e, must be more than 0 and less than 65536\n");
                    exit(EXIT_FAILURE);
                }
                ping_data->id = flag_val;
            }
            else
            {
                if (argv[i][1] == '-')
                {
                    panic_argv("ft_ping: unrecognized option '%s'\n", argv[i]);
                }
                panic_argv("ft_ping: invalid option -- '%s'\n", (char[]){argv[i][1], '\0'});
            }
        }
        // add ip
        else
        {
            if (!ping_data->ip_argv)
            {
                ping_data->ip_argv = argv[i];
            }
            else
            {
                panic_argv("ft_ping: only one IP address allowed\n", "");
            }
        }
    }
    if (!ping_data->ip_argv)
    {
        panic_argv("ft_ping: missing host operand\n", "");
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
    // printf("name %d\n", ICMP_NET_UNREACH);
    struct s_ping ping_data = {0};
    set_signal_action();
    init_ping(&ping_data, argc, argv);
    send_ping(&ping_data);
    print_stats(&ping_data);
    close(ping_data.sockfd);
    return 0;
}