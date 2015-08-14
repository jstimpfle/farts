#include <stdio.h>
#include <time.h>

static unsigned long timespec_diff(struct timespec *t1, struct timespec *t2)
{
        return (t1->tv_sec - t2->tv_sec) * 1000000
                + (t1->tv_nsec - t2->tv_nsec) / 1000;
}

void print_time(void)
{
        static struct timespec last;
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        fprintf(stderr, "TIME %9ld %9ld\n",
                timespec_diff(&ts, &last), (long)ts.tv_nsec);
        last = ts;
}
