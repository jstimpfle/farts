#ifndef LOCKFREE_FIFO_H_
#define LOCKFREE_FIFO_H_

/* single reader, single writer */

struct lockfree_fifo {
        void *buf;
        unsigned elem_size;
        unsigned capacity;
        unsigned first;
        unsigned last;
        unsigned volatile fill;
};

int lockfree_fifo_init(struct lockfree_fifo *fifo,
                       unsigned elem_size,
                       unsigned capacity);
void lockfree_fifo_exit(struct lockfree_fifo *fifo);

int lockfree_fifo_enqueue(struct lockfree_fifo *fifo, const void *elem);
int lockfree_fifo_dequeue(struct lockfree_fifo *fifo, void *elem);

#endif
