#include <stdio.h>
// #include <sys/socket.h>
// #include <netinet/in.h> //?
#include <stdlib.h>
#include <unistd.h> //? this needed now
#include <arpa/inet.h>
#include <string.h>
// #include <netinet/ip_icmp.h>
// #include <netinet/ip.h> //?
#include <netdb.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
// #include <float.h>
// #include <limits.h> //! add my own limits with define

//! TODO
//! add real printf of usage of ping! printf("print usage info of pingu\n");
// // code panic funtion for check_argv
//! code my own sqrt
//! put all printfs possible into send_ping or print stats, no weird mix between main send_ping etc
//! change t_ping_stats and s_ping to normal struct and merge them
//! its 3 diff unique things, and seq goes ++ each time
    //! recvmsg needs to check if id == id, icmp_seq ==, echo_reply i sent
// // use EXIT_FAILURE and EXIT_SUCCESS in exit
// clean all, cut into smaller functions
// // if (bytes_sent == 0 or it block what do i do, just pack_lost++ and continue, this can lead to an infinite loop
// // add control + c signal catch
// add argv parsing
    // add argv -? -v and bonus -ttl -c -i for intervals.
    // Maybe add -W (SO_RCVTIMEO) its the timeout time in recvmsg, -w is the max time ping before exiting, or if -c is done - no idea how to implement that
    // if -W 2 ping will wait 2 seconds if the packet doesnt arrive back before sending the next interval (-c or 1 second if no option)
    // recv from socket with timeout? //! for now if we dont receive the info it just stays locked there in the recv line -> ping waits 10 seconds then says packet lost? ineutils says it waits forever... need to check
    // maybe add -q? seems ez

// // find a way to know if i need to do a dns lookup or not. add dns_lookup -> maybe parse if ip is 255.255.255.255 (char.char.char.char)
// find a way to simulate errors and try them with ./ping ping and ft_ping
// // icmp->icmp_ttime = 631043; //! add real time here
// add header file
    // make struct with all the data, int sockfd, const char *dest, icmp(?), the argv and what else?
    // add icmp struct int the header to make it work with c99.
    // make my own typedef with icmp + padding char[36] -> with data? or use icmphdr thats 8 bytes bc i dont really use much of the 28 of icmp  
    // easter egg in data padding of 64 bytes?
// // calculate time between sendto and recvfrom and print it
// // retrieve ttl from ip packet, how??
//? ping 3232235777 should work for now doesnt --> output -v PING 3232235777 (192.168.1.1): 56 data bytes (add the ip)
//! for now if ping 127.0.0.1 does work i dont receive the same message i sent. check later
//// uint16_t seq = 0; //? for ./ping work froms 0 check in real debian - local machine starts from 1...
//! what do i do with sendto, does it block if wrong ip? if it does find a way for it to have a timeout and not block, same for recvmsg

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

uint8_t stop = 1; // char for the lulz?

typedef struct s_ping_stats
{
    long        sum;
    long long   sum_sq;
    long        min;
    long        max;
    long        packets_sent;
    long        packets_lost;
} t_ping_stats;

struct argv_flags
{
    // flags
    unsigned int verbose : 1; // 0 or 1
    
    // bonus flags
    double interval; // > 0.2
    uint8_t ttl; // 256 > ttl > 0 ! max 255 so char
    unsigned int count; // >= 0 // 0 infinite
    unsigned int quiet : 1; // 0 or 1
    unsigned int recv_timeout; // > 0
};

//! add icmp or not?
typedef struct s_ping
{
    // int icmp_data;
    uint16_t id; //! add random id at init
    int sockfd;
    char *ip_argv;
    char ip_address[INET_ADDRSTRLEN];
    struct sockaddr_in dest_addr;

    struct argv_flags flags;
    // // flags
    // unsigned int verbose : 1; // 0 or 1
    
    // // bonus flags
    // unsigned int quiet : 1; // 0 or 1
    // uint8_t ttl; // 256 > ttl > 0 ! max 255 so char
    // unsigned int count; // >= 0 // 0 infinite
    // double interval; // > 0.2
    // unsigned int recv_timeout; // > 0
} t_ping;

#define ICMP_ECHO	8
#define LONG_MAX    9223372036854775807
#define INT_MAX     2147483647

struct icmp
{
    //? header 22 bytes
    uint8_t     icmp_type;
    uint8_t     icmp_code;
    uint16_t    icmp_cksum; // padding
    uint16_t    icmp_id;
    uint16_t    icmp_seq;
    uint32_t    icmp_otime;
    uint32_t    icmp_rtime; // padding
    uint32_t    icmp_ttime;

    //? padding
    uint32_t    padding; //? this or data 44 and skip 4 first

    //? data 40 bytes
    uint8_t     data[40];
};

void update_stats(t_ping_stats *stats, long elapsed_micros) {
    stats->sum += elapsed_micros;
    stats->sum_sq += elapsed_micros * elapsed_micros;
    
    if (elapsed_micros < stats->min) {
        stats->min = elapsed_micros;
    }
    if (elapsed_micros > stats->max) {
        stats->max = elapsed_micros;
    }
    stats->packets_sent++;
}

void set_timeval(struct timeval *tval, double time_in_seconds)
{
    tval->tv_sec = (int)time_in_seconds;
    tval->tv_usec = (int)((time_in_seconds - tval->tv_sec) * 1000000);
}

//! code my own sqrt
//? code this better!
double square_root(double val)
{
	double ans = 1, sqr = 1, i = 1;
	while (sqr <= val)	//checking if squares of the numbers from 1 till given value is smaller than the  number
	{
		i++;
		sqr = i * i;
	}
	ans = i - 1;
	return ans;
}

void print_stats(t_ping_stats *stats) {
    //? PING 192.168.1.99 (192.168.1.99): 56 data bytes, id 0x857d = 34173
    //! IF ITS -v the i still need to add id 0x857d = 34173
    //! 0 packets transmitted, 0 packets received, -2147483648% packet loss  
    //! if 0 breaks if i dont add packets_sent > 0
    //? do print if packets_sent == 0?
    int loss_percent = 0;
    if (stats->packets_sent != 0)
    {
        loss_percent = (int)(((float)stats->packets_lost / stats->packets_sent) * 100.0);
    }
    printf("%ld packets transmitted, %ld packets received, %d%% packet loss\n", stats->packets_sent, stats->packets_sent - stats->packets_lost, loss_percent);
    if (stats->packets_sent > 0 && stats->packets_sent - stats->packets_lost > 0) {
        // printf("%ld packets transmitted, %ld packets received, %d%% packet loss\n", stats->packets_sent, stats->packets_sent - stats->packets_lost, (int)(((float)stats->packets_lost / stats->packets_sent) * 100.0));
        long total = stats->packets_sent - stats->packets_lost;

        long avg = stats->sum / total;
        long long variance;

        if (stats->sum < INT_MAX)
            variance = (stats->sum_sq - ((stats->sum * stats->sum) / total)) / total;
        else
            variance = (stats->sum_sq / total) - (avg * avg);

        long stddev = square_root(variance);

        printf("round-trip min/avg/max/stddev = %ld.%03ld/%ld.%03ld/%ld.%03ld/%ld.%03ld ms\n",
            stats->min / 1000, stats->min % 1000,
            avg / 1000, avg % 1000,
            stats->max / 1000, stats->max % 1000,
            stddev / 1000, stddev % 1000);
    }
}

void send_ping(t_ping *ping_data) {
    //? move to init
    t_ping_stats stats = {0};
    stats.min = LONG_MAX;


    // ! stats - var to track lost packets -> add counter for total packets and for -c, reset seq to 0 after max uint16_t 65k
    //? maybe add all of this in struct stats?

    uint8_t ttl; //? is there a way i can remove this? or add to stats struct?
    unsigned int counter = 0;

    //? easter egg with data
    //! make my own typedef with icmp + padding char[40] -> with random or easter egg data?
    struct icmp icmp = {
        .data = {
            // ? padding
            // 0x00, 0x00, 0x00, 0x00,
            //? debug data
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
            0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
            0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
            0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27
        }
    };
    uint16_t seq = 0; //? for ./ping work froms 0 check in real debian
    icmp.icmp_type = ICMP_ECHO;
    icmp.icmp_code = 0;
    icmp.icmp_id = ping_data->id; // done by the kernel so idc, adding for the !lulz //! 4242
    // icmp->icmp_seq = htons(++seq); //? change this

    // printf("id = %ld\n", sizeof(icmp)); //! -v id 0x4242 = 16962

    //! all the msg shit doesnt need to be in the loop, only created once!
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

    // struct timespec start, end; //? move this later

    struct timeval  start, end, interval;
    // wait.tv_sec = 0;
    // wait.tv_usec = 500000;
    // set_timeval(&interval, ping_data->interval);

    //! while stop = 1 or if -c --> i < c or -w close when time is over
    while (stop && (ping_data->flags.count == 0 || counter < ping_data->flags.count)) {
        //! seq is wrong order for some reason its 01 00 instead of 00 01 bytes
        icmp.icmp_seq = htons(++seq); //? change this

        // clock_gettime(CLOCK_REALTIME, &ts);
        // struct timeval start1, end1;
        // clock_gettime(CLOCK_REALTIME, &start);
        gettimeofday(&start, NULL);

        //! do i need to typecast?
        icmp.icmp_otime = (uint32_t)start.tv_sec;
        icmp.icmp_ttime = (uint32_t)start.tv_usec;

        // printf("%d %d\n", icmp->icmp_otime, icmp->icmp_ttime);

        // clock_gettime(CLOCK_MONOTONIC, &start);
        gettimeofday(&start, NULL);
        //! what do i do with sendto, does it block if wrong ip? if it does find a way for it to have a timeout and not block, same for recvmsg
        ssize_t bytes_sent = sendto(ping_data->sockfd, &icmp, sizeof(icmp), 0,
                                (struct sockaddr*)&ping_data->dest_addr, sizeof(ping_data->dest_addr));
        // stats.packets_sent++; //? what order do i add this?
        //? packets_lost++? what do i do here? just quit bc its broken? -> for now if it breaks its an infinite loop, maybe quit
        if (bytes_sent < 0) {
            // perror("sendto failed");
            stats.packets_sent++; //? what order do i add this?
            stats.packets_lost++;
            continue;
            // close(sockfd);
            // exit(EXIT_FAILURE);
        }

        //! what do i do with sendto, does it block if wrong ip? if it does find a way for it to have a timeout and not block, same for recvmsg
        ssize_t bytes_received = recvmsg(ping_data->sockfd, &msg, 0); //? MSG_DONTWAIT -> no timeout?
        // clock_gettime(CLOCK_MONOTONIC, &end);
        gettimeofday(&end, NULL);

        // long seconds = end.tv_sec - start.tv_sec;
        // long microseconds = end.tv_usec - start.tv_usec;
    
        // printf("Elapsed time: %ld secs %ld elap %ld\n", seconds, microseconds, elapsed_micros);
        //? if timeout and i dont receive the bytes the program exits it should just print Host unreachable, packets_lost++ and what else?
        //! and no usleep + packet_sent++! thats bad
        // long elapsed_micros = (end.tv_sec - start.tv_sec) * 1000000LL + end.tv_usec - start.tv_usec;
        // printf("elapsed_micros %ld\n", elapsed_micros);
        if (bytes_received < 0) {
            // perror("recvmsg"); //? do i remove this?
            stats.packets_sent++; //? what order do i add this?
            stats.packets_lost++;
            continue;
            // close(sockfd);
            // exit(EXIT_FAILURE);
        }

        // to print data in buffers
        // for (int i = 0; i < 64; i++) {
        //     printf("i: %d -- buff %x | control %x\n", i, (unsigned char) buffer[i], (unsigned char) control[i]);
        // }

        //? get icmp data in case of error (code 8, type with error).
        for (struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
            if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_TTL) {
                ttl = *(uint8_t *)CMSG_DATA(cmsg);
                // break; //? shoould i break here?
                // printf("Received TTL: %d\n", ttl);
            }
        }

        //! this is for error codes later!
        // struct icmp *rec_icmp = (struct icmp *) &buffer;
        // printf("type %d code %d millis %f ms -- %d %d %d\n", rec_icmp->icmp_type, rec_icmp->icmp_code, elapsed_time, rec_icmp->icmp_dun.id_ts.its_otime, rec_icmp->icmp_dun.id_ts.its_rtime, rec_icmp->icmp_dun.id_ts.its_ttime);

        // long elapsed_micros = (end.tv_sec - start.tv_sec) * 1000000LL + 
        //              (end.tv_nsec - start.tv_nsec) / 1000;
        long elapsed_micros = (end.tv_sec - start.tv_sec) * 1000000L + end.tv_usec - start.tv_usec;
        // printf("elapsed_micros %ld\n", elapsed_micros);
        update_stats(&stats, elapsed_micros);
        if (!ping_data->flags.quiet) // -> print next line, ezzz
        {
            printf("64 bytes from %s: icmp_seq=%d ttl=%u time=%ld.%03ld ms\n", ping_data->ip_address, seq, ttl, elapsed_micros / 1000, elapsed_micros % 1000);
        }
        counter++;
        // usleep(1000000); //? here can change to add bonus of -i and -w //! min is -i 0.2 less print error!
        //! use select instead of usleep if c99
        set_timeval(&interval, ping_data->flags.interval);
        select(0, NULL, NULL, NULL, &interval);
    }

    //! move this to print_stats
    printf("--- %s ping statistics ---\n", ping_data->ip_argv); //! add received string here, if example.com then example.com if 3232235777 then 3232235777, not the number ip
    print_stats(&stats);
    close(ping_data->sockfd);
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

//! code panic funtion for check_argv
void panic_argv(const char *format, const char *var)
{
    fprintf(stderr, format, var);
    fprintf(stderr, "Try 'ft_ping --help' or 'ping --usage' for more information.\n");
    exit(EXIT_FAILURE);
}

double check_bonus_argv_double(int *i, int argc, char *argv[])
{
    if (*i + 1 < argc)
    {
        char *endptr;
        char *str = argv[*i + 1];
        double num = strtod(str, &endptr);

        if (endptr == str || *endptr != '\0')
        {
            fprintf(stderr, "ft_ping: invalid value (`%s' near `%s')\n", str, endptr);
            exit(EXIT_FAILURE);
        }
        (*i)++;
        return num;
    }
    if (argv[*i][1] == '-')
    {
        panic_argv("ft_ping: option '%s' requires an argument\n", argv[*i]);
    }
    panic_argv("ft_ping: option requires an argument -- '%s'\n", (char[]){argv[*i][1], '\0'});
    exit(EXIT_FAILURE);
}

int check_bonus_argv_int(int *i, int argc, char *argv[])
{
    if (*i + 1 < argc)
    {
        char *endptr;
        char *str = argv[*i + 1];
        int num = (int)strtol(str, &endptr, 10);

        if (endptr == str || *endptr != '\0')
        {
            fprintf(stderr, "ft_ping: invalid value (`%s' near `%s')\n", str, endptr);
            exit(EXIT_FAILURE);
        }
        (*i)++;
        return num;
    }
    if (argv[*i][1] == '-')
    {
        panic_argv("ft_ping: option '%s' requires an argument\n", argv[*i]);
    }
    panic_argv("ft_ping: option requires an argument -- '%s'\n", (char[]){argv[*i][1], '\0'});
    exit(EXIT_FAILURE);
}

//! change return type etc
//? use EXIT_FAILURE
// fix bonus arg values :
    // int ttl; // 256 > ttl > 0
    // int count; // >= 0
    // int interval; // > 0.2 -> ./ping: option value too small: 0.1
    // int recv_timeout; // > 0
void check_argv(t_ping *ping_data, int argc, char *argv[])
{
    //! -c10 -W1 -i1 this doesnt work...
    double interval = 0.0;
    int flag_val = 0;
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (!strcmp(argv[i], "-?") || !strcmp(argv[i], "--help") || !strcmp(argv[i], "--usage"))
            {
                printf("print usage info of pingu\n");
                exit(EXIT_FAILURE);
            }
            else if (!strcmp(argv[i], "-v"))
            {
                // add id 0x81e7 = 33255
                ping_data->flags.verbose = 1;
            }
            else if (!strcmp(argv[i], "-q"))
            {
                // remove print 64 bytes from 192.168.1.1: icmp_seq=0 ttl=2 time=4.867 ms
                ping_data->flags.quiet = 1;
            }
            else if (!strcmp(argv[i], "--ttl")) // && i + 1 < argc)
            {
                // change IP_RECVTTL
                flag_val = check_bonus_argv_int(&i, argc, argv);
                if (flag_val <= 0 || flag_val >= 256)
                {
                    fprintf(stderr, "ft_ping: invalid value must be more than 0 and less than 256\n");
                    exit(EXIT_FAILURE);
                }
                ping_data->flags.ttl = flag_val;
            }
            else if (!strcmp(argv[i], "-c"))
            {
                // count till -c then exit
                flag_val = check_bonus_argv_int(&i, argc, argv);
                if (flag_val < 0)
                {
                    fprintf(stderr, "ft_ping: invalid value must be non-negative\n");
                    exit(EXIT_FAILURE);
                }
                ping_data->flags.count = (uint8_t)flag_val;
            }
            else if (!strcmp(argv[i], "-i"))
            {
                // change usleep
                interval = check_bonus_argv_double(&i, argc, argv);
                if (interval < 0.2)
                {
                    fprintf(stderr, "ft_ping: invalid value must be more than 0.2\n");
                    exit(EXIT_FAILURE);
                }
                ping_data->flags.interval = interval;
            }
            else if (!strcmp(argv[i], "-W"))
            {
                // change SO_RCVTIMEO
                flag_val = check_bonus_argv_int(&i, argc, argv);
                if (flag_val < 0)
                {
                    fprintf(stderr, "ft_ping: invalid value must be more than 0\n");
                    exit(EXIT_FAILURE);
                }
                ping_data->flags.recv_timeout = flag_val;
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
        //! else: if is ip add ip to struct, ip can be a whole number | 255.255.255.255 | domain.com else print wrong flag
        //! for now if its any other data other than ip and my args its an error, normal ping ignores data after ip if its not -
        else
        {
            //! ping: missing host operand
            if (!ping_data->ip_argv)
            {
                ping_data->ip_argv = argv[i];
                // printf("received ip: %s\n", ping_data->ip_argv); //! remove me
            }
        }
    }
    if (!ping_data->ip_argv)
    {
        panic_argv("ft_ping: missing host operand\n", "");
    }
}


int main(int argc, char *argv[])
{
    //! argv into its own function, check if argv are correct!
    // move this to init()
    t_ping ping_data = {0};
    //! if -v this?
    // if (ping_data.verbose)
    {
        srand(time(NULL));
        ping_data.id =  rand() % 65535;
    }
    ping_data.flags.ttl = 64;
    // ping_data.count = 0;
    ping_data.flags.interval = 1.0;
    ping_data.flags.recv_timeout = 1;
    check_argv(&ping_data, argc, argv);
    printf("received argv v %d ttl %d c %d i %0.3f W %d\n", ping_data.flags.verbose, ping_data.flags.ttl, ping_data.flags.count, ping_data.flags.interval, ping_data.flags.recv_timeout);
    set_signal_action();

    //? change this into its own function, like socket_init
    // this to init_socket
    int sockfd;
    // int ttl_val = 64;
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    if (sockfd < 0)
    {
        // printf("\nSocket file descriptor not received!\n");
        // return 1;
        fprintf(stderr, "ft_ping: Socket file descriptor not received\n");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(sockfd, SOL_IP, IP_TTL, &ping_data.flags.ttl, sizeof(ping_data.flags.ttl)) != 0)
    {
        // printf("\nSetting socket options to TTL failed!\n");
        // return 1;
        fprintf(stderr, "ft_ping: Setting socket options of TTL failed\n");
        exit(EXIT_FAILURE);

    }
    int optval = 1; //? can i change this?
    if (setsockopt(sockfd, IPPROTO_IP, IP_RECVTTL, &optval, sizeof(optval)) < 0)
    {
        // printf("\nSetting socket options of linger failed\n");
        // return 1;
        fprintf(stderr, "ft_ping: Setting socket options to receive TTL failed\n");
        exit(EXIT_FAILURE);
    }

    //? timeout bonus -W?
    struct timeval tv = {.tv_sec = ping_data.flags.recv_timeout};
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0)
    {
        // printf("\nSetting socket options of linger failed\n");
        // return 1;
        fprintf(stderr, "ft_ping: Setting socket options of linger failed\n");
        exit(EXIT_FAILURE);
    }
    ping_data.sockfd = sockfd;


    //! dns check
    struct hostent *host = gethostbyname(ping_data.ip_argv); // Resolve hostname
    if (host == NULL) {
        fprintf(stderr, "ft_ping: Unknown host %s\n", ping_data.ip_argv);
        exit(EXIT_FAILURE);
        // return (1); //or exit failure? its same but more explicit...
    }

    ping_data.dest_addr.sin_family = AF_INET;
    memcpy(&ping_data.dest_addr.sin_addr, host->h_addr_list[0], host->h_length);

    inet_ntop(AF_INET, &ping_data.dest_addr.sin_addr, ping_data.ip_address, sizeof(ping_data.ip_address));

    // printf("Resolved IP: %s\n", ping_data.ip_address);

    // PING 192.168.1.99 (192.168.1.99): 56 data bytes //? this should be printed like this! and add id if its -v
    printf("PING %s (%s): 56 data bytes", ping_data.ip_argv, ping_data.ip_address); //! add id = 0x4242 if option -v
    if (ping_data.flags.verbose)
    {
        printf (", id 0x%04x = %u", ping_data.id, ping_data.id);
    }
    printf("\n");
    // send_ping(&ping_data);

    //! erase from here 

    // char *str = "-c 10";
    // char *str = "-c10";
    // "-c" // wrong
    // "-caaaa" // wrong
    // "-c 10" // correct
    // "-c10" // correct
    // "-c" "10" // correct
    const char *strs[] = {
        "-z", // wrong
        "-c",     // wrong
        "-caaaa", // wrong
        "-c 10",  // correct
        "-c10",   // correct
         "-c ", "10" // correct
    };
    for (size_t i = 0; i < 5; i++)
    {
        char *result = strstr(strs[i], "-c");
        if (result != NULL) {
            int position = result - strs[i];
            int substringLength = strlen(strs[i]) - position;
            printf("i %ld pos %d sub %d %s\n", i, position, substringLength, strs[i]);
        }
    }
    //! to test case 6 - 7 later
    // {
    //     char *result = strstr(strs[i], "-c");
    //     int position = result - strs[i];
    //     int substringLength = strlen(strs[i]) - position;
    //     printf("pos %d sub %d %s\n", position, substringLength, strs[i]);
    // }
    return 0;
}