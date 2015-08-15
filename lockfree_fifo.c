/* single reader, single writer fixed-width (ringbuffer) FIFO queue */

#include <stdio.h>   /* fprintf() */
#include <stdlib.h>  /* malloc() */
#include <string.h>  /* memcpy() */
#include "lockfree_fifo.h"

struct lockfree_fifo {
        void *buf;
        unsigned elem_size;
        unsigned mask;
        unsigned first;
        unsigned last;
};

#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* on x86/x64 */
#define LOADLOAD_BARRIER() COMPILER_BARRIER()
#define LOADSTORE_BARRIER() COMPILER_BARRIER()
#define STORESTORE_BARRIER() COMPILER_BARRIER()

static unsigned next_pow2(unsigned x)
{
        if ((x & (x-1)) == 0)
                return x;
        while ((x & (x-1)) != 0)
                x = x & (x-1);
        return x << 1;
}

int lockfree_fifo_init(struct lockfree_fifo **fifop,
                       unsigned elem_size,
                       unsigned capacity)
{
        struct lockfree_fifo *fifo;
        capacity = next_pow2(capacity);

        fifo = *fifop = malloc(sizeof *fifo);
        if (fifo == NULL)
                return -1;

        fifo->buf = malloc(elem_size * capacity);
        if (fifo->buf == NULL) {
                free(fifo);
                return -1;
        }

        fifo->elem_size = elem_size;
        fifo->mask = capacity - 1;
        fifo->first = 0;
        fifo->last = 0;
        return 0;
}

void lockfree_fifo_exit(struct lockfree_fifo *fifo)
{
        free((void *)fifo->buf);
        free(fifo);
}

int lockfree_fifo_enqueue(struct lockfree_fifo *fifo, const void *elem)
{
        unsigned next = (fifo->last + 1) & fifo->mask;
        if (next == fifo->first)
                /* fifo full */
                return -1;
        void *dst = &((char *)fifo->buf)[fifo->elem_size * fifo->last];
        unsigned size = fifo->elem_size;
        /* store dst only _after_ asserting that that location can be written
         * to (i.e. next != fifo->first) */
        LOADSTORE_BARRIER();
        memcpy(dst, elem, size);
        /* externally visible transaction commit (increase last) only after all
         * is written */
        STORESTORE_BARRIER();
        fifo->last = next;
        return 0;
}

int lockfree_fifo_dequeue(struct lockfree_fifo *fifo, void *elem)
{
        /* last may be modified from the writer thread. But once (fifo->first !=
         * fifo->last), only the reader can change that */
        if (fifo->first == fifo->last)
                return -1;
        unsigned next = (fifo->first + 1) & fifo->mask;
        void *src = &((char *)fifo->buf)[fifo->elem_size * fifo->first];
        unsigned size = fifo->elem_size;
        /* load src only _after_ asserting that that location was valid (i.e.
         * fifo->first != fifo->last) */
        LOADLOAD_BARRIER();
        memcpy(elem, src, size);
        /* externally visible transaction commit (increase first) only after
         * all is read */
        LOADSTORE_BARRIER();
        fifo->first = next;
        return 0;
}

void lockfree_fifo_block_until_writeable(const struct lockfree_fifo *fifo)
{
        const volatile unsigned *first = &fifo->first;
        const volatile unsigned *last = &fifo->last;
        unsigned mask = fifo->mask;
        while (((*first + 1) & mask) != *last)
                continue;
}

void lockfree_fifo_block_until_readable(const struct lockfree_fifo *fifo)
{
        const volatile unsigned *first = &fifo->first;
        const volatile unsigned *last = &fifo->last;
        while (*first == *last)
                continue;
}
