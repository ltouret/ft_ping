#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

//! TODO
// // add real printf of usage of ping! printf("print usage info of pingu\n");
// // code panic funtion for check_argv
// // code my own sqrt
// //put all printfs possible into send_ping or print stats, no weird mix between main send_ping etc
// // change t_ping_stats and s_ping to normal struct and merge them
// // its 3 diff unique things, and seq goes ++ each time
    // // recvmsg needs to check if id == id, icmp_seq ==, echo_reply i sent
// // panic_argv("ft_ping: only one IP address allowed\n", "");
// // use EXIT_FAILURE and EXIT_SUCCESS in exit
//! clean all, cut into smaller functions
// // if (bytes_sent == 0 or it block what do i do, just pack_lost++ and continue, this can lead to an infinite loop
// // add control + c signal catch
// // add argv parsing
//    // add argv -? -v and bonus -ttl -c -i for intervals.
//    // Maybe add -W (SO_RCVTIMEO) its the timeout time in recvmsg, -w is the max time ping before exiting, or if -c is done - no idea how to implement that
//    // if -W 2 ping will wait 2 seconds if the packet doesnt arrive back before sending the next interval (-c or 1 second if no option)
//    // recv from socket with timeout? //! for now if we dont receive the info it just stays locked there in the recv line -> ping waits 10 seconds then says packet lost? ineutils says it waits forever... need to check
//    // maybe add -q? seems ez
//    // maybe add -e? seems ez

// // find a way to know if i need to do a dns lookup or not. add dns_lookup -> maybe parse if ip is 255.255.255.255 (char.char.char.char)
//! find a way to simulate errors and try them with ./ping ping and ft_ping
// // icmp->icmp_ttime = 631043; //! add real time here
//! add header file
    // // make struct with all the data, int sockfd, const char *dest, icmp(?), the argv and what else?
    // // add icmp struct int the header to make it work with c99.
    // // make my own typedef with icmp + padding char[36] -> with data? or use icmphdr thats 8 bytes bc i dont really use much of the 28 of icmp  
    // // easter egg in data padding of 64 bytes?
// // calculate time between sendto and recvfrom and print it
// // retrieve ttl from ip packet, how??
// // ping 3232235777 should work for now doesnt --> output -v PING 3232235777 (192.168.1.1): 56 data bytes (add the ip)
// // for now if ping 127.0.0.1 does work i dont receive the same message i sent. check later
//// uint16_t seq = 0; //? for ./ping work froms 0 check in real debian - local machine starts from 1...
// // what do i do with sendto, does it block if wrong ip? if it does find a way for it to have a timeout and not block, same for recvmsg

// // ! format of ping ineutils 2.0 -> only diff is id 0x81e7 = 33255
// // print verbose -> 
// // printf ("PING %s (%s): %zu data bytes",
// //     if (options & OPT_VERBOSE)
// //     printf (", id 0x%04x = %u", ping->ping_ident, ping->ping_ident);
// //  missing the dns lookup in case of domain
// // print after each ping in the correct format (check if -v format changes)
// // add math of statistics --> 2 packets transmitted, 2 packets received, 0% packet loss - round-trip min/avg/max/stddev = 2.244/3.556/4.867/1.312 ms
// // ./ping -v 3232235777
// /*
//     PING 3232235777 (192.168.1.1): 56 data bytes, id 0x81e7 = 33255
//     64 bytes from 192.168.1.1: icmp_seq=0 ttl=255 time=4.263 ms
//     64 bytes from 192.168.1.1: icmp_seq=1 ttl=0 time=8.486 ms
//     ^C--- 3232235777 ping statistics ---
//     2 packets transmitted, 2 packets received, 0% packet loss
//     round-trip min/avg/max/stddev = 4.263/6.375/8.486/2.112 ms
// */
// // ./ping 3232235777
// /*
//     PING 3232235777 (192.168.1.1): 56 data bytes
//     64 bytes from 192.168.1.1: icmp_seq=0 ttl=2 time=4.867 ms
//     64 bytes from 192.168.1.1: icmp_seq=1 ttl=3 time=2.244 ms
//     ^C--- 3232235777 ping statistics ---
//     2 packets transmitted, 2 packets received, 0% packet loss
//     round-trip min/avg/max/stddev = 2.244/3.556/4.867/1.312 ms
// */
// // ./ping -v 192.168.1.99 //! bad address -> stats no round-trip min/avg etc
// /*
//     PING 192.168.1.99 (192.168.1.99): 56 data bytes, id 0x857d = 34173
//     ^C--- 192.168.1.99 ping statistics ---
//     5 packets transmitted, 0 packets received, 100% packet loss
// */

uint8_t stop = 1;

struct s_ping_stats
{
    long        sum;
    long long   sum_sq;
    long        min;
    long        max;
    long        packets_sent;
    long        packets_lost;
};

struct s_argv_flags
{
    // flags
    unsigned int verbose : 1; // 0 or 1
    
    // bonus flags
    double interval; // > 0.2
    uint8_t ttl; // 256 > ttl > 0
    unsigned int count; // >= 0
    unsigned int quiet : 1; // 0 or 1
    unsigned int recv_timeout; // > 0
};

struct s_ping
{
    int sockfd;
    uint16_t id;
    char *ip_argv;
    char ip_address[INET_ADDRSTRLEN];
    struct sockaddr_in dest_addr;

    // flags
    struct s_argv_flags flags;

    // stats
    struct s_ping_stats stats;
};

#define ICMP_ECHOREPLY		0
#define ICMP_DEST_UNREACH	3	/* Destination Unreachable	*/
#define ICMP_REDIRECT		5	/* Redirect (change route)	*/
#define ICMP_ECHO           8
#define ICMP_TIME_EXCEEDED	11	/* Time Exceeded		*/
#define ICMP_PARAMETERPROB	12	/* Parameter Problem		*/

/* Codes for UNREACH. */
#define ICMP_NET_UNREACH	0	/* Network Unreachable		*/
#define ICMP_HOST_UNREACH	1	/* Host Unreachable		*/
#define ICMP_PROT_UNREACH	2	/* Protocol Unreachable		*/
#define ICMP_PORT_UNREACH	3	/* Port Unreachable		*/
#define ICMP_FRAG_NEEDED	4	/* Fragmentation Needed/DF set	*/
#define ICMP_SR_FAILED		5	/* Source Route failed		*/
#define ICMP_NET_UNKNOWN	6
#define ICMP_HOST_UNKNOWN	7
#define ICMP_HOST_ISOLATED	8
#define ICMP_NET_ANO		9
#define ICMP_HOST_ANO		10
#define ICMP_NET_UNR_TOS	11
#define ICMP_HOST_UNR_TOS	12
#define ICMP_PKT_FILTERED	13	/* Packet filtered */
#define ICMP_PREC_VIOLATION	14	/* Precedence violation */
#define ICMP_PREC_CUTOFF	15	/* Precedence cut off */
#define NR_ICMP_UNREACH		15	/* instead of hardcoding immediate value */

/* Codes for REDIRECT. */
#define ICMP_REDIR_NET		0	/* Redirect Net			*/
#define ICMP_REDIR_HOST		1	/* Redirect Host		*/
#define ICMP_REDIR_NETTOS	2	/* Redirect Net for TOS		*/
#define ICMP_REDIR_HOSTTOS	3	/* Redirect Host for TOS	*/

/* Codes for TIME_EXCEEDED. */
#define ICMP_EXC_TTL		0	/* TTL count exceeded		*/
#define ICMP_EXC_FRAGTIME	1	/* Fragment Reass time exceeded	*/

#define LONG_MAX    9223372036854775807
#define INT_MAX     2147483647

struct icmp
{
    // header 20 bytes
    uint8_t     icmp_type;
    uint8_t     icmp_code;
    uint16_t    icmp_cksum; // padding
    uint16_t    icmp_id;
    uint16_t    icmp_seq;
    uint32_t    icmp_otime;
    uint32_t    icmp_rtime; // padding
    uint32_t    icmp_ttime;

    // padding 4 bytes
    uint32_t    padding;

    // data 40 bytes
    uint8_t     data[40];
};

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

//? code this better!
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
    struct s_ping ping_data = {0};
    set_signal_action();
    init_ping(&ping_data, argc, argv);
    send_ping(&ping_data);
    print_stats(&ping_data);
    close(ping_data.sockfd);
    return 0;
}