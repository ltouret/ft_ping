#ifndef PING_H
#define PING_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

uint8_t stop;

struct s_ping_stats
{
    long sum;
    long long sum_sq;
    long min;
    long max;
    long packets_sent;
    long packets_lost;
};

struct s_argv_flags
{
    // flags
    unsigned int verbose : 1; // 0 or 1

    // bonus flags
    double interval;           // > 0.2
    uint8_t ttl;               // 256 > ttl > 0
    unsigned int count;        // >= 0
    unsigned int quiet : 1;    // 0 or 1
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

struct icmp
{
    // header 20 bytes
    uint8_t icmp_type;
    uint8_t icmp_code;
    uint16_t icmp_cksum; // padding
    uint16_t icmp_id;
    uint16_t icmp_seq;
    uint32_t icmp_otime;
    uint32_t icmp_rtime; // padding
    uint32_t icmp_ttime;

    // padding 4 bytes
    uint32_t padding;

    // data 40 bytes
    uint8_t data[40];
};

/* Types for ICMP. */
#define ICMP_ECHOREPLY 0
#define ICMP_DEST_UNREACH 3 /* Destination Unreachable	*/
#define ICMP_REDIRECT 5     /* Redirect (change route)	*/
#define ICMP_ECHO 8
#define ICMP_TIME_EXCEEDED 11 /* Time Exceeded		*/
#define ICMP_PARAMETERPROB 12 /* Parameter Problem		*/

/* Codes for UNREACH. */
#define ICMP_NET_UNREACH 0  /* Network Unreachable		*/
#define ICMP_HOST_UNREACH 1 /* Host Unreachable		*/
#define ICMP_PROT_UNREACH 2 /* Protocol Unreachable		*/
#define ICMP_PORT_UNREACH 3 /* Port Unreachable		*/
#define ICMP_FRAG_NEEDED 4  /* Fragmentation Needed/DF set	*/
#define ICMP_SR_FAILED 5    /* Source Route failed		*/
#define ICMP_NET_UNKNOWN 6
#define ICMP_HOST_UNKNOWN 7
#define ICMP_HOST_ISOLATED 8
#define ICMP_NET_ANO 9
#define ICMP_HOST_ANO 10
#define ICMP_NET_UNR_TOS 11
#define ICMP_HOST_UNR_TOS 12
#define ICMP_PKT_FILTERED 13   /* Packet filtered */
#define ICMP_PREC_VIOLATION 14 /* Precedence violation */
#define ICMP_PREC_CUTOFF 15    /* Precedence cut off */
#define NR_ICMP_UNREACH 15     /* instead of hardcoding immediate value */

/* Codes for REDIRECT. */
#define ICMP_REDIR_NET 0     /* Redirect Net			*/
#define ICMP_REDIR_HOST 1    /* Redirect Host		*/
#define ICMP_REDIR_NETTOS 2  /* Redirect Net for TOS		*/
#define ICMP_REDIR_HOSTTOS 3 /* Redirect Host for TOS	*/

/* Codes for TIME_EXCEEDED. */
#define ICMP_EXC_TTL 0      /* TTL count exceeded		*/
#define ICMP_EXC_FRAGTIME 1 /* Fragment Reass time exceeded	*/

#define LONG_MAX 9223372036854775807
#define INT_MAX 2147483647

#endif