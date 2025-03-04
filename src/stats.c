#include "ft_ping.h"

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

void print_stats(struct s_ping *ping_data)
{
    int loss_percent = 0;
    struct s_ping_stats *stats = &ping_data->stats;

    printf("--- %s ping statistics ---\n", ping_data->ip_argv);
    if (stats->packets_sent != 0)
    {
        loss_percent = (int)(((float)stats->packets_lost / stats->packets_sent) * 100.0);
    }
    printf("%ld packets transmitted, %ld packets received, %d%% packet loss\n",
        stats->packets_sent, stats->packets_sent - stats->packets_lost, loss_percent);
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