#include "ping.h"

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