#include <stdlib.h>  /* malloc() */
#include <string.h>  /* memcpy() */
#include "lockfree_fifo.h"

/* single reader, single writer */

int lockfree_fifo_init(struct lockfree_fifo *fifo,
                       unsigned elem_size,
                       unsigned capacity)
{
        fifo->buf = malloc(elem_size * capacity);
        if (fifo->buf == NULL)
                return -1;
        fifo->elem_size = elem_size;
        fifo->capacity = capacity;
        fifo->first = 0;
        fifo->last = 0;
        fifo->fill = 0;
        return 0;
}

void lockfree_fifo_exit(struct lockfree_fifo *fifo)
{
        free(fifo->buf);
}

int lockfree_fifo_enqueue(struct lockfree_fifo *fifo, const void *elem)
{
        if (fifo->fill == fifo->capacity)
                return -1;
        void *dst = &((char *)fifo->buf)[fifo->elem_size * fifo->last];
        memcpy(dst, elem, fifo->elem_size);
        __sync_synchronize();
        fifo->last++;
        if (fifo->last == fifo->capacity)
                fifo->last = 0;
        __sync_fetch_and_add(&fifo->fill, 1);
        return 0;
}

int lockfree_fifo_dequeue(struct lockfree_fifo *fifo, void *elem)
{
        if (fifo->fill == 0)
                return -1;
        void *src = &((char *)fifo->buf)[fifo->elem_size * fifo->first];
        memcpy(elem, src, fifo->elem_size);
        __sync_synchronize();
        fifo->first++;
        if (fifo->first == fifo->capacity)
                fifo->first = 0;
        __sync_fetch_and_add(&fifo->fill, -1);
        return 0;
}
