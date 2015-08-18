#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>  /* sleep() */
#include "lockfree_fifo.h"

void *test_writer(void *fifop)
{
        int i;
        long misses = 0;
        struct lockfree_fifo *fifo = fifop;

        /* sync with reader, without increasing "misses" */
        while (lockfree_fifo_enqueue(fifo, &i) == -1) {
        }
        lockfree_fifo_block_until_writeable(fifo);

        for (i = 0; i < 14000000; i++) {
                while (lockfree_fifo_enqueue(fifo, &i) == -1)
                        misses++;
        }
        printf("writer had %ld misses\n", misses);
        return NULL;
}

void *test_reader(void *fifop)
{
        int i;
        int out;
        long misses = 0;
        struct lockfree_fifo *fifo = fifop;

        /* sync with writer, without increasing "misses" */
        lockfree_fifo_block_until_readable(fifo);
        while (lockfree_fifo_dequeue(fifo, &out) == -1) {
        }

        for (i = 0; i < 14000000; i++) {
                while (lockfree_fifo_dequeue(fifo, &out) == -1)
                        misses++;
                /*printf("read %d\n", out);
                 */
        }
        printf("reader had %ld misses\n", misses);
        return NULL;
}

void test_fifo(unsigned fifo_size)
{
        int r;
        pthread_t pt;
        struct lockfree_fifo *fifo;

        r = lockfree_fifo_init(&fifo, sizeof (int), fifo_size);
        if (r == -1) {
                fprintf(stderr, "failed to initialize fifo\n");
                return;
        }

        r = pthread_create(&pt, NULL, test_writer, fifo);
        if (r != 0) {
                fprintf(stderr, "pthread_create failed!\n");
                lockfree_fifo_exit(fifo);
                return;
        }

        test_reader(fifo);

        r = pthread_join(pt, NULL);
        if (r != 0)
                fprintf(stderr, "Warning: pthread_join failed\n");

        lockfree_fifo_exit(fifo);
}

int main(int argc, const char *argv[])
{
        int fifo_size = 1;
        if (argc == 2)
                fifo_size = strtol(argv[1], NULL, 10);
        test_fifo(fifo_size);
        return 0;
}
