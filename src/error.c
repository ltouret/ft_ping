#include "ft_ping.h"

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