#include "ping.h"

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